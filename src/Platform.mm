#import "Platform.h"
#import <Cocoa/Cocoa.h>
#import <MetalKit/MetalKit.h>
#import <QuartzCore/QuartzCore.h>

// Use fully qualified names in Objective-C++ interface
using PixelEngine::PlatformCallbacks;
using PixelEngine::InputState;
using PixelEngine::MaterialID;

// Objective-C app delegate (must be outside namespace)
@interface AppDelegate : NSObject <NSApplicationDelegate, MTKViewDelegate>
@property (nonatomic) PlatformCallbacks callbacks;
@property (nonatomic) InputState* inputState;
@property (nonatomic) CFTimeInterval lastFrameTime;
@property (nonatomic) BOOL shouldClose;
@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification*)notification {
    (void)notification;
    _lastFrameTime = CACurrentMediaTime();
    _shouldClose = NO;
}

- (BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication*)sender {
    (void)sender;
    return YES;
}

- (void)applicationWillTerminate:(NSNotification*)notification {
    (void)notification;
    _shouldClose = YES;
}

// MTKViewDelegate methods
- (void)mtkView:(MTKView*)view drawableSizeWillChange:(CGSize)size {
    (void)view;
    (void)size;
}

- (void)drawInMTKView:(MTKView*)view {
    (void)view;

    // Calculate delta time
    CFTimeInterval currentTime = CACurrentMediaTime();
    float deltaTime = static_cast<float>(currentTime - _lastFrameTime);
    _lastFrameTime = currentTime;

    // Call update callback
    if (_callbacks.on_update) {
        _callbacks.on_update(deltaTime);
    }

    // Call render callback
    if (_callbacks.on_render) {
        _callbacks.on_render();
    }
}

@end

// Objective-C view for handling input
@interface InputView : MTKView
@property (nonatomic) InputState* inputState;
@end

@implementation InputView

- (void)mouseDown:(NSEvent*)event {
    _inputState->mouse_left_down = true;
    [self updateMousePosition:event];
}

- (void)mouseUp:(NSEvent*)event {
    _inputState->mouse_left_down = false;
    [self updateMousePosition:event];
}

- (void)mouseDragged:(NSEvent*)event {
    [self updateMousePosition:event];
}

- (void)mouseMoved:(NSEvent*)event {
    [self updateMousePosition:event];
}

- (void)rightMouseDown:(NSEvent*)event {
    _inputState->mouse_right_down = true;
    [self updateMousePosition:event];
}

- (void)rightMouseUp:(NSEvent*)event {
    _inputState->mouse_right_down = false;
    [self updateMousePosition:event];
}

- (void)rightMouseDragged:(NSEvent*)event {
    [self updateMousePosition:event];
}

- (void)updateMousePosition:(NSEvent*)event {
    NSPoint locationInView = [self convertPoint:event.locationInWindow fromView:nil];

    // Get current view size
    CGFloat viewWidth = self.bounds.size.width;
    CGFloat viewHeight = self.bounds.size.height;

    // Update view size in input state
    _inputState->view_width = static_cast<int32_t>(viewWidth);
    _inputState->view_height = static_cast<int32_t>(viewHeight);

    // Scale mouse coordinates from view space to world space
    // This maps the view coordinates to WORLD_WIDTH x WORLD_HEIGHT
    CGFloat scaleX = static_cast<CGFloat>(PixelEngine::WORLD_WIDTH) / viewWidth;
    CGFloat scaleY = static_cast<CGFloat>(PixelEngine::WORLD_HEIGHT) / viewHeight;

    // Flip Y coordinate (Cocoa origin is bottom-left, we want top-left) and scale
    _inputState->mouse_x = static_cast<int32_t>(locationInView.x * scaleX);
    _inputState->mouse_y = static_cast<int32_t>((viewHeight - locationInView.y) * scaleY);

    // Clamp to world bounds
    if (_inputState->mouse_x < 0) _inputState->mouse_x = 0;
    if (_inputState->mouse_x >= PixelEngine::WORLD_WIDTH) _inputState->mouse_x = PixelEngine::WORLD_WIDTH - 1;
    if (_inputState->mouse_y < 0) _inputState->mouse_y = 0;
    if (_inputState->mouse_y >= PixelEngine::WORLD_HEIGHT) _inputState->mouse_y = PixelEngine::WORLD_HEIGHT - 1;
}

- (void)keyDown:(NSEvent*)event {
    unichar key = [[event charactersIgnoringModifiers] characterAtIndex:0];

    // Switch materials with number keys
    switch (key) {
        case '1':
            _inputState->selected_material = MaterialID::Sand;
            NSLog(@"Selected: Sand");
            break;
        case '2':
            _inputState->selected_material = MaterialID::Water;
            NSLog(@"Selected: Water");
            break;
        case '3':
            _inputState->selected_material = MaterialID::Stone;
            NSLog(@"Selected: Stone");
            break;
        case '4':
            _inputState->selected_material = MaterialID::Steam;
            NSLog(@"Selected: Steam");
            break;
        case '5':
            _inputState->selected_material = MaterialID::Oil;
            NSLog(@"Selected: Oil");
            break;
        case '6':
            _inputState->selected_material = MaterialID::Fire;
            NSLog(@"Selected: Fire");
            break;
        case '7':
            _inputState->selected_material = MaterialID::Wood;
            NSLog(@"Selected: Wood");
            break;
        case '8':
            _inputState->selected_material = MaterialID::Acid;
            NSLog(@"Selected: Acid");
            break;
        case '9':
            _inputState->selected_material = MaterialID::Lava;
            NSLog(@"Selected: Lava");
            break;
        case '0':
            _inputState->selected_material = MaterialID::Ash;
            NSLog(@"Selected: Ash");
            break;
        case 'g':
        case 'G':
            _inputState->selected_material = MaterialID::Grass;
            NSLog(@"Selected: Grass");
            break;
        case 's':
        case 'S':
            _inputState->selected_material = MaterialID::Smoke;
            NSLog(@"Selected: Smoke");
            break;
        case 'p':
        case 'P':
            _inputState->selected_material = MaterialID::Person;
            NSLog(@"Selected: Person");
            break;
        case '[':
            // Decrease brush size
            if (_inputState->brush_radius > 1) {
                _inputState->brush_radius--;
                NSLog(@"Brush size: %d", _inputState->brush_radius);
            }
            break;
        case ']':
            // Increase brush size
            if (_inputState->brush_radius < 20) {
                _inputState->brush_radius++;
                NSLog(@"Brush size: %d", _inputState->brush_radius);
            }
            break;
        case 'b':
        case 'B':
            // Toggle brush shape
            if (_inputState->brush_shape == PixelEngine::BrushShape::Circle) {
                _inputState->brush_shape = PixelEngine::BrushShape::Square;
                NSLog(@"Brush shape: Square");
            } else {
                _inputState->brush_shape = PixelEngine::BrushShape::Circle;
                NSLog(@"Brush shape: Circle");
            }
            break;
        case 'c':
        case 'C':
            // Clear world
            _inputState->clear_world = true;
            NSLog(@"Clearing world...");
            break;
        case '\t':  // Tab key
            // Toggle debug GUI
            _inputState->show_debug_gui = !_inputState->show_debug_gui;
            NSLog(@"Debug GUI: %s", _inputState->show_debug_gui ? "ON" : "OFF");
            break;
        case 'q':
        case 'Q':
            // Quit
            [NSApp terminate:nil];
            break;
        case '=':
        case '+':
            // Increase bloom intensity
            _inputState->increase_bloom = true;
            break;
        case '-':
        case '_':
            // Decrease bloom intensity
            _inputState->decrease_bloom = true;
            break;
        case ',':
        case '<':
            // Previous page
            _inputState->prev_page = true;
            break;
        case '.':
        case '>':
            // Next page
            _inputState->next_page = true;
            break;
        case 'l':
        case 'L':
            // Line tool
            _inputState->tool_mode = PixelEngine::ToolMode::Line;
            _inputState->shape_drawing = false;
            NSLog(@"Tool mode: Line");
            break;
        case 'r':
        case 'R':
            // Rectangle tool
            _inputState->tool_mode = PixelEngine::ToolMode::Rectangle;
            _inputState->shape_drawing = false;
            NSLog(@"Tool mode: Rectangle");
            break;
        case 'o':
        case 'O':
            // Circle/Oval tool
            _inputState->tool_mode = PixelEngine::ToolMode::Circle;
            _inputState->shape_drawing = false;
            NSLog(@"Tool mode: Circle");
            break;
        case 'e':
        case 'E':
            // Fill tool (E for fill, since F is already Fire)
            _inputState->tool_mode = PixelEngine::ToolMode::Fill;
            _inputState->shape_drawing = false;
            NSLog(@"Tool mode: Fill");
            break;
        case 'd':
        case 'D':
            // Back to brush (D for Draw)
            _inputState->tool_mode = PixelEngine::ToolMode::Brush;
            _inputState->shape_drawing = false;
            NSLog(@"Tool mode: Brush");
            break;
        case 'x':
        case 'X':
            // Toggle filled vs outline shapes
            _inputState->filled_shapes = !_inputState->filled_shapes;
            NSLog(@"Shapes: %s", _inputState->filled_shapes ? "Filled" : "Outline");
            break;
        default:
            break;
    }

    // Handle function keys (special key codes)
    unsigned short keyCode = [event keyCode];
    switch (keyCode) {
        case 122:  // F1
            _inputState->toggle_post_processing = true;
            NSLog(@"Toggle post-processing");
            break;
        case 120:  // F2
            _inputState->toggle_bloom = true;
            NSLog(@"Toggle bloom");
            break;
        case 99:   // F3
            _inputState->toggle_color = true;
            NSLog(@"Toggle color grading");
            break;
        case 118:  // F4
            _inputState->toggle_vignette = true;
            NSLog(@"Toggle vignette");
            break;
        default:
            break;
    }
}

- (BOOL)acceptsFirstResponder {
    return YES;
}

@end

// Platform implementation
namespace PixelEngine {

Platform::Platform()
    : initialized_(false)
    , running_(false)
    , input_state_()
    , window_(nullptr)
    , metal_view_(nullptr)
    , app_delegate_(nullptr) {
}

Platform::~Platform() {
    if (app_delegate_) {
        AppDelegate* delegate = (__bridge_transfer AppDelegate*)app_delegate_;
        delegate = nil;
    }
    if (metal_view_) {
        InputView* view = (__bridge_transfer InputView*)metal_view_;
        view = nil;
    }
    if (window_) {
        NSWindow* window = (__bridge_transfer NSWindow*)window_;
        window = nil;
    }
}

bool Platform::initialize(int32_t window_width, int32_t window_height, const char* title) {
    @autoreleasepool {
        // Initialize Cocoa application
        [NSApplication sharedApplication];
        [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

        // Create app delegate
        AppDelegate* appDelegate = [[AppDelegate alloc] init];
        appDelegate.inputState = &input_state_;
        [NSApp setDelegate:appDelegate];
        app_delegate_ = (__bridge_retained void*)appDelegate;

        // Create window
        NSRect windowRect = NSMakeRect(0, 0, window_width, window_height);
        NSWindowStyleMask styleMask = NSWindowStyleMaskTitled |
                                      NSWindowStyleMaskClosable |
                                      NSWindowStyleMaskMiniaturizable |
                                      NSWindowStyleMaskResizable;

        NSWindow* window = [[NSWindow alloc] initWithContentRect:windowRect
                                                        styleMask:styleMask
                                                          backing:NSBackingStoreBuffered
                                                            defer:NO];

        [window setTitle:[NSString stringWithUTF8String:title]];
        [window center];

        // Set minimum size to prevent window from being too small
        [window setMinSize:NSMakeSize(400, 300)];

        window_ = (__bridge_retained void*)window;

        // Create Metal device
        id<MTLDevice> device = MTLCreateSystemDefaultDevice();
        if (!device) {
            NSLog(@"Metal is not supported on this device");
            return false;
        }

        // Create Metal view
        InputView* metalView = [[InputView alloc] initWithFrame:windowRect device:device];
        metalView.inputState = &input_state_;
        metalView.delegate = appDelegate;
        metalView.clearColor = MTLClearColorMake(0.0, 0.0, 0.0, 1.0);
        metalView.framebufferOnly = YES;
        metalView.preferredFramesPerSecond = 60;

        // Enable mouse tracking
        NSTrackingArea* trackingArea = [[NSTrackingArea alloc]
            initWithRect:metalView.bounds
                 options:NSTrackingMouseMoved | NSTrackingActiveInKeyWindow | NSTrackingInVisibleRect
                   owner:metalView
                userInfo:nil];
        [metalView addTrackingArea:trackingArea];

        metal_view_ = (__bridge_retained void*)metalView;

        // Set view as window content
        [window setContentView:metalView];
        [window makeFirstResponder:metalView];

        initialized_ = true;
        return true;
    }
}

void Platform::run(const PlatformCallbacks& callbacks) {
    if (!initialized_) {
        NSLog(@"Platform not initialized");
        return;
    }

    @autoreleasepool {
        AppDelegate* appDelegate = (__bridge AppDelegate*)app_delegate_;
        appDelegate.callbacks = callbacks;

        NSWindow* window = (__bridge NSWindow*)window_;
        [window makeKeyAndOrderFront:nil];

        [NSApp activateIgnoringOtherApps:YES];

        running_ = true;
        [NSApp run];
        running_ = false;
    }
}

void* Platform::get_metal_view() {
    return metal_view_;
}

} // namespace PixelEngine
