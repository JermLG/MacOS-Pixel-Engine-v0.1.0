#import "MetalRenderer.h"
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#import <simd/simd.h>

namespace PixelEngine {

// Vertex structure for the fullscreen quad
struct Vertex {
    simd_float2 position;
    simd_float2 texCoord;
};

MetalRenderer::MetalRenderer()
    : initialized_(false)
    , post_processing_enabled_(true)
    , world_width_(0)
    , world_height_(0)
    , device_(nullptr)
    , command_queue_(nullptr)
    , vertex_buffer_(nullptr)
    , params_buffer_(nullptr)
    , view_(nullptr)
    , scene_texture_(nullptr)
    , bloom_texture_a_(nullptr)
    , bloom_texture_b_(nullptr)
    , pipeline_main_(nullptr)
    , pipeline_bloom_extract_(nullptr)
    , pipeline_blur_h_(nullptr)
    , pipeline_blur_v_(nullptr)
    , pipeline_composite_(nullptr) {
}

MetalRenderer::~MetalRenderer() {
    // Release Metal objects
    if (device_) {
        id<MTLDevice> device = (__bridge_transfer id<MTLDevice>)device_;
        device = nil;
    }
    if (command_queue_) {
        id<MTLCommandQueue> queue = (__bridge_transfer id<MTLCommandQueue>)command_queue_;
        queue = nil;
    }
    if (vertex_buffer_) {
        id<MTLBuffer> buffer = (__bridge_transfer id<MTLBuffer>)vertex_buffer_;
        buffer = nil;
    }
    if (params_buffer_) {
        id<MTLBuffer> buffer = (__bridge_transfer id<MTLBuffer>)params_buffer_;
        buffer = nil;
    }
    if (scene_texture_) {
        id<MTLTexture> tex = (__bridge_transfer id<MTLTexture>)scene_texture_;
        tex = nil;
    }
    if (bloom_texture_a_) {
        id<MTLTexture> tex = (__bridge_transfer id<MTLTexture>)bloom_texture_a_;
        tex = nil;
    }
    if (bloom_texture_b_) {
        id<MTLTexture> tex = (__bridge_transfer id<MTLTexture>)bloom_texture_b_;
        tex = nil;
    }
    if (pipeline_main_) {
        id<MTLRenderPipelineState> p = (__bridge_transfer id<MTLRenderPipelineState>)pipeline_main_;
        p = nil;
    }
    if (pipeline_bloom_extract_) {
        id<MTLRenderPipelineState> p = (__bridge_transfer id<MTLRenderPipelineState>)pipeline_bloom_extract_;
        p = nil;
    }
    if (pipeline_blur_h_) {
        id<MTLRenderPipelineState> p = (__bridge_transfer id<MTLRenderPipelineState>)pipeline_blur_h_;
        p = nil;
    }
    if (pipeline_blur_v_) {
        id<MTLRenderPipelineState> p = (__bridge_transfer id<MTLRenderPipelineState>)pipeline_blur_v_;
        p = nil;
    }
    if (pipeline_composite_) {
        id<MTLRenderPipelineState> p = (__bridge_transfer id<MTLRenderPipelineState>)pipeline_composite_;
        p = nil;
    }
}

bool MetalRenderer::initialize(void* mtkView, int32_t world_width, int32_t world_height) {
    MTKView* view = (__bridge MTKView*)mtkView;
    view_ = mtkView;

    world_width_ = world_width;
    world_height_ = world_height;

    // Get Metal device
    id<MTLDevice> device = view.device;
    if (!device) {
        NSLog(@"Failed to get Metal device");
        return false;
    }
    device_ = (__bridge_retained void*)device;

    // Create command queue
    id<MTLCommandQueue> commandQueue = [device newCommandQueue];
    if (!commandQueue) {
        NSLog(@"Failed to create command queue");
        return false;
    }
    command_queue_ = (__bridge_retained void*)commandQueue;

    // Create all pipelines
    if (!create_pipelines()) {
        return false;
    }

    // Create vertex buffer
    if (!create_vertex_buffer()) {
        return false;
    }

    // Create textures
    if (!create_textures()) {
        return false;
    }

    // Create params buffer
    if (!create_params_buffer()) {
        return false;
    }

    initialized_ = true;
    return true;
}

bool MetalRenderer::create_pipelines() {
    id<MTLDevice> device = (__bridge id<MTLDevice>)device_;

    // Load shader library
    NSError* error = nil;

    // Try to load compiled shader library first
    NSString* shaderPath = [[NSBundle mainBundle] pathForResource:@"shaders/shader" ofType:@"metallib"];
    id<MTLLibrary> library = nil;

    if (shaderPath) {
        library = [device newLibraryWithFile:shaderPath error:&error];
    }

    // If not found, compile from source
    if (!library) {
        NSString* sourcePath = [[NSBundle mainBundle] pathForResource:@"shaders/shader" ofType:@"metal"];
        if (!sourcePath) {
            sourcePath = @"shaders/shader.metal";
        }

        NSString* shaderSource = [NSString stringWithContentsOfFile:sourcePath
                                                           encoding:NSUTF8StringEncoding
                                                              error:&error];
        if (!shaderSource) {
            NSLog(@"Failed to load shader source: %@", error);
            return false;
        }

        library = [device newLibraryWithSource:shaderSource options:nil error:&error];
        if (!library) {
            NSLog(@"Failed to compile shader: %@", error);
            return false;
        }
    }

    // Get shader functions
    id<MTLFunction> vertexFunction = [library newFunctionWithName:@"vertex_main"];
    id<MTLFunction> fragmentMain = [library newFunctionWithName:@"fragment_main"];
    id<MTLFunction> fragmentBloomExtract = [library newFunctionWithName:@"fragment_bloom_extract"];
    id<MTLFunction> fragmentBlurH = [library newFunctionWithName:@"fragment_blur_horizontal"];
    id<MTLFunction> fragmentBlurV = [library newFunctionWithName:@"fragment_blur_vertical"];
    id<MTLFunction> fragmentComposite = [library newFunctionWithName:@"fragment_composite"];

    if (!vertexFunction || !fragmentMain) {
        NSLog(@"Failed to get required shader functions");
        return false;
    }

    // Create vertex descriptor
    MTLVertexDescriptor* vertexDescriptor = [[MTLVertexDescriptor alloc] init];
    vertexDescriptor.attributes[0].format = MTLVertexFormatFloat2;
    vertexDescriptor.attributes[0].offset = 0;
    vertexDescriptor.attributes[0].bufferIndex = 0;
    vertexDescriptor.attributes[1].format = MTLVertexFormatFloat2;
    vertexDescriptor.attributes[1].offset = sizeof(simd_float2);
    vertexDescriptor.attributes[1].bufferIndex = 0;
    vertexDescriptor.layouts[0].stride = sizeof(Vertex);
    vertexDescriptor.layouts[0].stepRate = 1;
    vertexDescriptor.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;

    // Helper to create pipeline
    auto createPipeline = [&](id<MTLFunction> fragment, MTLPixelFormat format) -> id<MTLRenderPipelineState> {
        MTLRenderPipelineDescriptor* desc = [[MTLRenderPipelineDescriptor alloc] init];
        desc.vertexFunction = vertexFunction;
        desc.fragmentFunction = fragment;
        desc.vertexDescriptor = vertexDescriptor;
        desc.colorAttachments[0].pixelFormat = format;

        NSError* err = nil;
        id<MTLRenderPipelineState> state = [device newRenderPipelineStateWithDescriptor:desc error:&err];
        if (!state) {
            NSLog(@"Failed to create pipeline: %@", err);
        }
        return state;
    };

    // Main pipeline (renders to screen - BGRA8Unorm)
    id<MTLRenderPipelineState> mainPipeline = createPipeline(fragmentMain, MTLPixelFormatBGRA8Unorm);
    if (!mainPipeline) return false;
    pipeline_main_ = (__bridge_retained void*)mainPipeline;

    // Bloom extract pipeline (renders to RGBA16Float for HDR)
    if (fragmentBloomExtract) {
        id<MTLRenderPipelineState> bloomExtractPipeline = createPipeline(fragmentBloomExtract, MTLPixelFormatRGBA16Float);
        if (bloomExtractPipeline) {
            pipeline_bloom_extract_ = (__bridge_retained void*)bloomExtractPipeline;
        }
    }

    // Blur pipelines
    if (fragmentBlurH) {
        id<MTLRenderPipelineState> blurHPipeline = createPipeline(fragmentBlurH, MTLPixelFormatRGBA16Float);
        if (blurHPipeline) {
            pipeline_blur_h_ = (__bridge_retained void*)blurHPipeline;
        }
    }

    if (fragmentBlurV) {
        id<MTLRenderPipelineState> blurVPipeline = createPipeline(fragmentBlurV, MTLPixelFormatRGBA16Float);
        if (blurVPipeline) {
            pipeline_blur_v_ = (__bridge_retained void*)blurVPipeline;
        }
    }

    // Composite pipeline (renders to screen)
    if (fragmentComposite) {
        id<MTLRenderPipelineState> compositePipeline = createPipeline(fragmentComposite, MTLPixelFormatBGRA8Unorm);
        if (compositePipeline) {
            pipeline_composite_ = (__bridge_retained void*)compositePipeline;
        }
    }

    return true;
}

bool MetalRenderer::create_vertex_buffer() {
    id<MTLDevice> device = (__bridge id<MTLDevice>)device_;

    // Fullscreen quad vertices
    Vertex vertices[] = {
        {{-1.0f, -1.0f}, {0.0f, 1.0f}},
        {{ 1.0f, -1.0f}, {1.0f, 1.0f}},
        {{-1.0f,  1.0f}, {0.0f, 0.0f}},
        {{ 1.0f, -1.0f}, {1.0f, 1.0f}},
        {{ 1.0f,  1.0f}, {1.0f, 0.0f}},
        {{-1.0f,  1.0f}, {0.0f, 0.0f}},
    };

    id<MTLBuffer> vertexBuffer = [device newBufferWithBytes:vertices
                                                     length:sizeof(vertices)
                                                    options:MTLResourceStorageModeShared];
    if (!vertexBuffer) {
        NSLog(@"Failed to create vertex buffer");
        return false;
    }

    vertex_buffer_ = (__bridge_retained void*)vertexBuffer;
    return true;
}

bool MetalRenderer::create_textures() {
    id<MTLDevice> device = (__bridge id<MTLDevice>)device_;

    // Scene texture (full resolution, CPU-uploaded)
    MTLTextureDescriptor* sceneDesc = [MTLTextureDescriptor
        texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm
                                     width:world_width_
                                    height:world_height_
                                 mipmapped:NO];
    sceneDesc.usage = MTLTextureUsageShaderRead;
    sceneDesc.storageMode = MTLStorageModeShared;

    id<MTLTexture> sceneTexture = [device newTextureWithDescriptor:sceneDesc];
    if (!sceneTexture) {
        NSLog(@"Failed to create scene texture");
        return false;
    }
    scene_texture_ = (__bridge_retained void*)sceneTexture;

    // Bloom textures (half resolution for performance, HDR format)
    int bloomWidth = world_width_ / 2;
    int bloomHeight = world_height_ / 2;

    MTLTextureDescriptor* bloomDesc = [MTLTextureDescriptor
        texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA16Float
                                     width:bloomWidth
                                    height:bloomHeight
                                 mipmapped:NO];
    bloomDesc.usage = MTLTextureUsageShaderRead | MTLTextureUsageRenderTarget;
    bloomDesc.storageMode = MTLStorageModePrivate;  // GPU-only for performance

    id<MTLTexture> bloomA = [device newTextureWithDescriptor:bloomDesc];
    id<MTLTexture> bloomB = [device newTextureWithDescriptor:bloomDesc];

    if (!bloomA || !bloomB) {
        NSLog(@"Failed to create bloom textures");
        return false;
    }

    bloom_texture_a_ = (__bridge_retained void*)bloomA;
    bloom_texture_b_ = (__bridge_retained void*)bloomB;

    return true;
}

bool MetalRenderer::create_params_buffer() {
    id<MTLDevice> device = (__bridge id<MTLDevice>)device_;

    id<MTLBuffer> paramsBuffer = [device newBufferWithLength:sizeof(PostProcessParams)
                                                     options:MTLResourceStorageModeShared];
    if (!paramsBuffer) {
        NSLog(@"Failed to create params buffer");
        return false;
    }

    params_buffer_ = (__bridge_retained void*)paramsBuffer;

    // Initialize with default values
    update_params_buffer();

    return true;
}

void MetalRenderer::update_params_buffer() {
    if (!params_buffer_) return;

    id<MTLBuffer> buffer = (__bridge id<MTLBuffer>)params_buffer_;
    memcpy([buffer contents], &params_, sizeof(PostProcessParams));
}

void MetalRenderer::update_texture(const uint32_t* pixel_data) {
    if (!initialized_ || !scene_texture_) {
        return;
    }

    id<MTLTexture> texture = (__bridge id<MTLTexture>)scene_texture_;

    MTLRegion region = MTLRegionMake2D(0, 0, world_width_, world_height_);
    NSUInteger bytesPerRow = world_width_ * sizeof(uint32_t);

    [texture replaceRegion:region
               mipmapLevel:0
                 withBytes:pixel_data
               bytesPerRow:bytesPerRow];
}

void MetalRenderer::toggle_effect(PostProcessEffects effect) {
    params_.effects_enabled ^= effect;
}

bool MetalRenderer::is_effect_enabled(PostProcessEffects effect) const {
    return (params_.effects_enabled & effect) != 0;
}

void MetalRenderer::render() {
    if (!initialized_) {
        return;
    }

    // Update time for animated effects
    static float time = 0.0f;
    time += 1.0f / 60.0f;
    params_.time = time;

    if (post_processing_enabled_ && pipeline_composite_ && pipeline_bloom_extract_) {
        render_with_post_processing();
    } else {
        render_without_post_processing();
    }
}

void MetalRenderer::render_without_post_processing() {
    MTKView* view = (__bridge MTKView*)view_;
    id<MTLCommandQueue> commandQueue = (__bridge id<MTLCommandQueue>)command_queue_;
    id<MTLRenderPipelineState> pipelineState = (__bridge id<MTLRenderPipelineState>)pipeline_main_;
    id<MTLBuffer> vertexBuffer = (__bridge id<MTLBuffer>)vertex_buffer_;
    id<MTLTexture> texture = (__bridge id<MTLTexture>)scene_texture_;

    MTLRenderPassDescriptor* renderPassDescriptor = view.currentRenderPassDescriptor;
    if (!renderPassDescriptor) return;

    id<CAMetalDrawable> drawable = view.currentDrawable;
    if (!drawable) return;

    id<MTLCommandBuffer> commandBuffer = [commandQueue commandBuffer];
    id<MTLRenderCommandEncoder> encoder = [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];

    [encoder setRenderPipelineState:pipelineState];
    [encoder setVertexBuffer:vertexBuffer offset:0 atIndex:0];
    [encoder setFragmentTexture:texture atIndex:0];
    [encoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:6];
    [encoder endEncoding];

    [commandBuffer presentDrawable:drawable];
    [commandBuffer commit];
}

void MetalRenderer::render_with_post_processing() {
    MTKView* view = (__bridge MTKView*)view_;
    id<MTLCommandQueue> commandQueue = (__bridge id<MTLCommandQueue>)command_queue_;
    id<MTLBuffer> vertexBuffer = (__bridge id<MTLBuffer>)vertex_buffer_;
    id<MTLTexture> sceneTexture = (__bridge id<MTLTexture>)scene_texture_;
    id<MTLTexture> bloomA = (__bridge id<MTLTexture>)bloom_texture_a_;
    id<MTLTexture> bloomB = (__bridge id<MTLTexture>)bloom_texture_b_;

    id<MTLRenderPipelineState> bloomExtract = (__bridge id<MTLRenderPipelineState>)pipeline_bloom_extract_;
    id<MTLRenderPipelineState> blurH = (__bridge id<MTLRenderPipelineState>)pipeline_blur_h_;
    id<MTLRenderPipelineState> blurV = (__bridge id<MTLRenderPipelineState>)pipeline_blur_v_;
    id<MTLRenderPipelineState> composite = (__bridge id<MTLRenderPipelineState>)pipeline_composite_;

    id<CAMetalDrawable> drawable = view.currentDrawable;
    if (!drawable) return;

    // Update params buffer
    update_params_buffer();
    id<MTLBuffer> paramsBuffer = (__bridge id<MTLBuffer>)params_buffer_;

    id<MTLCommandBuffer> commandBuffer = [commandQueue commandBuffer];

    // =====================================
    // PASS 1: BLOOM EXTRACT (scene → bloomA)
    // =====================================
    {
        MTLRenderPassDescriptor* passDesc = [[MTLRenderPassDescriptor alloc] init];
        passDesc.colorAttachments[0].texture = bloomA;
        passDesc.colorAttachments[0].loadAction = MTLLoadActionDontCare;
        passDesc.colorAttachments[0].storeAction = MTLStoreActionStore;

        id<MTLRenderCommandEncoder> encoder = [commandBuffer renderCommandEncoderWithDescriptor:passDesc];
        [encoder setRenderPipelineState:bloomExtract];
        [encoder setVertexBuffer:vertexBuffer offset:0 atIndex:0];
        [encoder setFragmentTexture:sceneTexture atIndex:0];
        [encoder setFragmentBuffer:paramsBuffer offset:0 atIndex:0];
        [encoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:6];
        [encoder endEncoding];
    }

    // =====================================
    // PASS 2: HORIZONTAL BLUR (bloomA → bloomB)
    // =====================================
    {
        MTLRenderPassDescriptor* passDesc = [[MTLRenderPassDescriptor alloc] init];
        passDesc.colorAttachments[0].texture = bloomB;
        passDesc.colorAttachments[0].loadAction = MTLLoadActionDontCare;
        passDesc.colorAttachments[0].storeAction = MTLStoreActionStore;

        id<MTLRenderCommandEncoder> encoder = [commandBuffer renderCommandEncoderWithDescriptor:passDesc];
        [encoder setRenderPipelineState:blurH];
        [encoder setVertexBuffer:vertexBuffer offset:0 atIndex:0];
        [encoder setFragmentTexture:bloomA atIndex:0];
        [encoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:6];
        [encoder endEncoding];
    }

    // =====================================
    // PASS 3: VERTICAL BLUR (bloomB → bloomA)
    // =====================================
    {
        MTLRenderPassDescriptor* passDesc = [[MTLRenderPassDescriptor alloc] init];
        passDesc.colorAttachments[0].texture = bloomA;
        passDesc.colorAttachments[0].loadAction = MTLLoadActionDontCare;
        passDesc.colorAttachments[0].storeAction = MTLStoreActionStore;

        id<MTLRenderCommandEncoder> encoder = [commandBuffer renderCommandEncoderWithDescriptor:passDesc];
        [encoder setRenderPipelineState:blurV];
        [encoder setVertexBuffer:vertexBuffer offset:0 atIndex:0];
        [encoder setFragmentTexture:bloomB atIndex:0];
        [encoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:6];
        [encoder endEncoding];
    }

    // =====================================
    // PASS 4: SECOND BLUR PASS (wider bloom)
    // =====================================
    {
        // Horizontal (bloomA → bloomB)
        MTLRenderPassDescriptor* passDescH = [[MTLRenderPassDescriptor alloc] init];
        passDescH.colorAttachments[0].texture = bloomB;
        passDescH.colorAttachments[0].loadAction = MTLLoadActionDontCare;
        passDescH.colorAttachments[0].storeAction = MTLStoreActionStore;

        id<MTLRenderCommandEncoder> encoderH = [commandBuffer renderCommandEncoderWithDescriptor:passDescH];
        [encoderH setRenderPipelineState:blurH];
        [encoderH setVertexBuffer:vertexBuffer offset:0 atIndex:0];
        [encoderH setFragmentTexture:bloomA atIndex:0];
        [encoderH drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:6];
        [encoderH endEncoding];

        // Vertical (bloomB → bloomA)
        MTLRenderPassDescriptor* passDescV = [[MTLRenderPassDescriptor alloc] init];
        passDescV.colorAttachments[0].texture = bloomA;
        passDescV.colorAttachments[0].loadAction = MTLLoadActionDontCare;
        passDescV.colorAttachments[0].storeAction = MTLStoreActionStore;

        id<MTLRenderCommandEncoder> encoderV = [commandBuffer renderCommandEncoderWithDescriptor:passDescV];
        [encoderV setRenderPipelineState:blurV];
        [encoderV setVertexBuffer:vertexBuffer offset:0 atIndex:0];
        [encoderV setFragmentTexture:bloomB atIndex:0];
        [encoderV drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:6];
        [encoderV endEncoding];
    }

    // =====================================
    // PASS 5: FINAL COMPOSITE (scene + bloom → screen)
    // =====================================
    {
        MTLRenderPassDescriptor* renderPassDescriptor = view.currentRenderPassDescriptor;
        if (!renderPassDescriptor) {
            [commandBuffer commit];
            return;
        }

        id<MTLRenderCommandEncoder> encoder = [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
        [encoder setRenderPipelineState:composite];
        [encoder setVertexBuffer:vertexBuffer offset:0 atIndex:0];
        [encoder setFragmentTexture:sceneTexture atIndex:0];
        [encoder setFragmentTexture:bloomA atIndex:1];
        [encoder setFragmentBuffer:paramsBuffer offset:0 atIndex:0];
        [encoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:6];
        [encoder endEncoding];
    }

    [commandBuffer presentDrawable:drawable];
    [commandBuffer commit];
}

} // namespace PixelEngine
