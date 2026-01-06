#include <metal_stdlib>
using namespace metal;

// ============================================
// VERTEX STRUCTURES
// ============================================

struct VertexIn {
    float2 position [[attribute(0)]];
    float2 texCoord [[attribute(1)]];
};

struct VertexOut {
    float4 position [[position]];
    float2 texCoord;
};

// ============================================
// POST-PROCESSING PARAMETERS
// ============================================

struct PostProcessParams {
    float bloom_intensity;      // 0.0 - 1.0, default 0.3
    float bloom_threshold;      // 0.0 - 1.0, default 0.6
    float contrast;             // 0.8 - 1.5, default 1.1
    float saturation;           // 0.5 - 1.5, default 1.15
    float vignette_intensity;   // 0.0 - 1.0, default 0.3
    float gamma;                // 0.8 - 2.2, default 1.0
    uint  effects_enabled;      // Bitflags: 1=bloom, 2=color, 4=vignette
    float time;                 // For animated effects
};

// ============================================
// SHARED VERTEX SHADER
// ============================================

vertex VertexOut vertex_main(VertexIn in [[stage_in]]) {
    VertexOut out;
    out.position = float4(in.position, 0.0, 1.0);
    out.texCoord = in.texCoord;
    return out;
}

// ============================================
// PASS 1: SCENE RENDER (original, unchanged)
// ============================================

fragment float4 fragment_main(VertexOut in [[stage_in]],
                              texture2d<float> colorTexture [[texture(0)]]) {
    constexpr sampler textureSampler(mag_filter::nearest, min_filter::nearest);
    return colorTexture.sample(textureSampler, in.texCoord);
}

// ============================================
// PASS 2: BLOOM THRESHOLD + DOWNSAMPLE
// Extracts bright pixels for bloom effect
// ============================================

fragment float4 fragment_bloom_extract(VertexOut in [[stage_in]],
                                       texture2d<float> sceneTexture [[texture(0)]],
                                       constant PostProcessParams& params [[buffer(0)]]) {
    constexpr sampler textureSampler(mag_filter::linear, min_filter::linear);

    float4 color = sceneTexture.sample(textureSampler, in.texCoord);

    // Calculate luminance
    float luminance = dot(color.rgb, float3(0.299, 0.587, 0.114));

    // Use threshold directly - don't halve it
    float threshold = params.bloom_threshold;
    float brightness = max(0.0, luminance - threshold);

    // Soft knee for smoother transition
    float softness = 0.3;
    brightness = brightness / (brightness + softness);

    // FIRE/LAVA BOOST: Boost warm colors (red/orange/yellow)
    // Fire is typically high R, medium G, low B
    float warmth = color.r - max(color.g, color.b);  // More conservative warmth calc
    warmth = max(0.0, warmth);

    // Gentler glow multiplier for warm colors
    float glowMultiplier = 1.0 + warmth * 1.5;

    // Combine luminance-based and warmth-based extraction
    float bloomStrength = brightness * glowMultiplier;
    bloomStrength = clamp(bloomStrength, 0.0, 1.0);  // Clamp to 1.0 to prevent runaway

    return float4(color.rgb * bloomStrength, 1.0);
}

// ============================================
// PASS 3: HORIZONTAL BLUR
// Separable Gaussian blur - horizontal pass
// ============================================

fragment float4 fragment_blur_horizontal(VertexOut in [[stage_in]],
                                         texture2d<float> bloomTexture [[texture(0)]]) {
    constexpr sampler textureSampler(mag_filter::linear, min_filter::linear,
                                     address::clamp_to_edge);

    float2 texelSize = 1.0 / float2(bloomTexture.get_width(), bloomTexture.get_height());

    // Normalized Gaussian weights that sum to 1.0
    // Center weight + 2*(sum of other weights) = 1.0
    const float weights[5] = { 0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216 };
    const float spread = 2.0;  // Spread multiplier for wider blur

    float3 result = bloomTexture.sample(textureSampler, in.texCoord).rgb * weights[0];

    for (int i = 1; i < 5; i++) {
        float2 offset = float2(texelSize.x * float(i) * spread, 0.0);
        result += bloomTexture.sample(textureSampler, in.texCoord + offset).rgb * weights[i];
        result += bloomTexture.sample(textureSampler, in.texCoord - offset).rgb * weights[i];
    }

    return float4(result, 1.0);
}

// ============================================
// PASS 4: VERTICAL BLUR
// Separable Gaussian blur - vertical pass
// ============================================

fragment float4 fragment_blur_vertical(VertexOut in [[stage_in]],
                                       texture2d<float> bloomTexture [[texture(0)]]) {
    constexpr sampler textureSampler(mag_filter::linear, min_filter::linear,
                                     address::clamp_to_edge);

    float2 texelSize = 1.0 / float2(bloomTexture.get_width(), bloomTexture.get_height());

    // Normalized Gaussian weights that sum to 1.0
    // Center weight + 2*(sum of other weights) = 1.0
    const float weights[5] = { 0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216 };
    const float spread = 2.0;  // Spread multiplier for wider blur

    float3 result = bloomTexture.sample(textureSampler, in.texCoord).rgb * weights[0];

    for (int i = 1; i < 5; i++) {
        float2 offset = float2(0.0, texelSize.y * float(i) * spread);
        result += bloomTexture.sample(textureSampler, in.texCoord + offset).rgb * weights[i];
        result += bloomTexture.sample(textureSampler, in.texCoord - offset).rgb * weights[i];
    }

    return float4(result, 1.0);
}

// ============================================
// PASS 5: FINAL COMPOSITE
// Combines scene + bloom + color grading + vignette
// ============================================

fragment float4 fragment_composite(VertexOut in [[stage_in]],
                                   texture2d<float> sceneTexture [[texture(0)]],
                                   texture2d<float> bloomTexture [[texture(1)]],
                                   constant PostProcessParams& params [[buffer(0)]]) {
    constexpr sampler sceneSampler(mag_filter::nearest, min_filter::nearest);
    constexpr sampler bloomSampler(mag_filter::linear, min_filter::linear);

    // Sample textures
    float4 scene = sceneTexture.sample(sceneSampler, in.texCoord);
    float4 bloom = bloomTexture.sample(bloomSampler, in.texCoord);

    float3 color = scene.rgb;

    // ---- BLOOM ----
    if (params.effects_enabled & 1u) {
        // Additive bloom with intensity control
        color += bloom.rgb * params.bloom_intensity;
    }

    // ---- COLOR ENHANCEMENT ----
    if (params.effects_enabled & 2u) {
        // Contrast adjustment (centered around 0.5)
        color = (color - 0.5) * params.contrast + 0.5;

        // Saturation adjustment
        float gray = dot(color, float3(0.299, 0.587, 0.114));
        color = mix(float3(gray), color, params.saturation);

        // Gamma correction
        color = pow(max(color, 0.0), float3(1.0 / params.gamma));
    }

    // ---- VIGNETTE ----
    if (params.effects_enabled & 4u) {
        float2 uv = in.texCoord * 2.0 - 1.0;  // -1 to 1
        float dist = length(uv * float2(0.8, 1.0));  // Elliptical
        float vignette = 1.0 - smoothstep(0.5, 1.4, dist) * params.vignette_intensity;
        color *= vignette;
    }

    // Clamp to valid range
    color = clamp(color, 0.0, 1.0);

    return float4(color, scene.a);
}

// ============================================
// OPTIONAL: NO POST-PROCESSING (passthrough)
// Direct render without effects
// ============================================

fragment float4 fragment_passthrough(VertexOut in [[stage_in]],
                                     texture2d<float> colorTexture [[texture(0)]]) {
    constexpr sampler textureSampler(mag_filter::nearest, min_filter::nearest);
    return colorTexture.sample(textureSampler, in.texCoord);
}
