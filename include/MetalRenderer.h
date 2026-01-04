#pragma once

#include "Types.h"
#include <cstdint>

// Forward declarations for Objective-C types
#ifdef __OBJC__
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
@class MTKView;
#else
typedef struct objc_object MTKView;
#endif

namespace PixelEngine {

// Post-processing parameters (must match shader struct)
struct PostProcessParams {
    float bloom_intensity = 0.7f;    // Much stronger bloom (was 0.35)
    float bloom_threshold = 0.3f;    // Lower threshold to catch more (was 0.55)
    float contrast = 1.1f;
    float saturation = 1.15f;
    float vignette_intensity = 0.25f;
    float gamma = 1.0f;
    uint32_t effects_enabled = 7;    // All effects on by default (1+2+4)
    float time = 0.0f;
};

// Effect flags for effects_enabled
enum PostProcessEffects : uint32_t {
    EFFECT_BLOOM    = 1 << 0,  // 1
    EFFECT_COLOR    = 1 << 1,  // 2
    EFFECT_VIGNETTE = 1 << 2,  // 4
    EFFECT_ALL      = 7
};

// Metal renderer - handles all GPU rendering with post-processing
class MetalRenderer {
public:
    MetalRenderer();
    ~MetalRenderer();

    // Initialize with MTKView
    bool initialize(void* mtkView, int32_t world_width, int32_t world_height);

    // Update texture from CPU buffer
    void update_texture(const uint32_t* pixel_data);

    // Render frame (with or without post-processing)
    void render();

    // Check if initialized
    bool is_initialized() const { return initialized_; }

    // Post-processing controls
    void set_post_processing_enabled(bool enabled) { post_processing_enabled_ = enabled; }
    bool is_post_processing_enabled() const { return post_processing_enabled_; }

    void toggle_effect(PostProcessEffects effect);
    bool is_effect_enabled(PostProcessEffects effect) const;

    // Direct access to parameters for fine-tuning
    PostProcessParams& params() { return params_; }
    const PostProcessParams& params() const { return params_; }

    // Convenience methods
    void set_bloom_intensity(float intensity) { params_.bloom_intensity = intensity; }
    void set_bloom_threshold(float threshold) { params_.bloom_threshold = threshold; }
    void set_contrast(float contrast) { params_.contrast = contrast; }
    void set_saturation(float saturation) { params_.saturation = saturation; }
    void set_vignette_intensity(float intensity) { params_.vignette_intensity = intensity; }

private:
    bool initialized_;
    bool post_processing_enabled_;
    int32_t world_width_;
    int32_t world_height_;

    PostProcessParams params_;

    // Metal objects (stored as void* to avoid Objective-C in header)
    void* device_;              // id<MTLDevice>
    void* command_queue_;       // id<MTLCommandQueue>
    void* vertex_buffer_;       // id<MTLBuffer>
    void* params_buffer_;       // id<MTLBuffer> for PostProcessParams
    void* view_;                // MTKView*

    // Main scene texture (CPU-uploaded simulation)
    void* scene_texture_;       // id<MTLTexture>

    // Post-processing render targets (half resolution for bloom)
    void* bloom_texture_a_;     // id<MTLTexture> - bloom extract / blur ping
    void* bloom_texture_b_;     // id<MTLTexture> - blur pong

    // Render pipelines
    void* pipeline_main_;           // id<MTLRenderPipelineState> - direct render
    void* pipeline_bloom_extract_;  // id<MTLRenderPipelineState>
    void* pipeline_blur_h_;         // id<MTLRenderPipelineState>
    void* pipeline_blur_v_;         // id<MTLRenderPipelineState>
    void* pipeline_composite_;      // id<MTLRenderPipelineState>

    bool create_pipelines();
    bool create_vertex_buffer();
    bool create_textures();
    bool create_params_buffer();

    void render_without_post_processing();
    void render_with_post_processing();
    void update_params_buffer();
};

} // namespace PixelEngine
