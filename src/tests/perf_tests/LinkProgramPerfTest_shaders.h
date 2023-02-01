struct ProgramInfo
{
    const char *name;
    std::map<GLenum, const char *> shaders;
};
static const ProgramInfo kPrograms[] = {
    {
        "10281214085255716636",
        {
            {GL_VERTEX_SHADER,
             R"(#version 300 es

precision mediump float;
precision mediump sampler2D;
uniform highp vec4 sk_RTAdjust;
in highp vec4 radii_selector;
in highp vec4 corner_and_radius_outsets;
in highp vec4 aa_bloat_and_coverage;
in highp vec4 radii_x;
in highp vec4 radii_y;
in highp vec4 skew;
in highp vec2 translate_and_localrotate;
in mediump vec4 color;
out mediump vec4 vcolor_S0;
out highp vec2 varccoord_S0;
void main() {
    vcolor_S0 = color;
    highp float aa_bloat_multiplier = 1.0;
    highp vec2 corner = corner_and_radius_outsets.xy;
    highp vec2 radius_outset = corner_and_radius_outsets.zw;
    highp vec2 aa_bloat_direction = aa_bloat_and_coverage.xy;
    highp float is_linear_coverage = aa_bloat_and_coverage.w;
    highp vec2 pixellength = inversesqrt(vec2(dot(skew.xz, skew.xz), dot(skew.yw, skew.yw)));
    highp vec4 normalized_axis_dirs = skew * pixellength.xyxy;
    highp vec2 axiswidths = abs(normalized_axis_dirs.xy) + abs(normalized_axis_dirs.zw);
    highp vec2 aa_bloatradius = (axiswidths * pixellength) * 0.5;
    highp vec4 radii_and_neighbors = radii_selector * mat4(radii_x, radii_y, radii_x.yxwz, radii_y.wzyx);
    highp vec2 radii = radii_and_neighbors.xy;
    highp vec2 neighbor_radii = radii_and_neighbors.zw;
    highp float coverage_multiplier = 1.0;
    if (any(greaterThan(aa_bloatradius, vec2(1.0)))) {
        corner = max(abs(corner), aa_bloatradius) * sign(corner);
        coverage_multiplier = 1.0 / (max(aa_bloatradius.x, 1.0) * max(aa_bloatradius.y, 1.0));
        radii = vec2(0.0);
    }
    highp float coverage = aa_bloat_and_coverage.z;
    if (any(lessThan(radii, aa_bloatradius * 1.5))) {
        radii = vec2(0.0);
        aa_bloat_direction = sign(corner);
        if (coverage > 0.5) {
            aa_bloat_direction = -aa_bloat_direction;
        }
        is_linear_coverage = 1.0;
    } else {
        radii = clamp(radii, pixellength * 1.5, 2.0 - pixellength * 1.5);
        neighbor_radii = clamp(neighbor_radii, pixellength * 1.5, 2.0 - pixellength * 1.5);
        highp vec2 spacing = (2.0 - radii) - neighbor_radii;
        highp vec2 extra_pad = max(pixellength * 0.0625 - spacing, vec2(0.0));
        radii -= extra_pad * 0.5;
    }
    highp vec2 aa_outset = (aa_bloat_direction * aa_bloatradius) * aa_bloat_multiplier;
    highp vec2 vertexpos = (corner + radius_outset * radii) + aa_outset;
    if (coverage > 0.5) {
        if (aa_bloat_direction.x != 0.0 ? vertexpos.x * corner.x < 0.0 : false) {
            highp float backset = abs(vertexpos.x);
            vertexpos.x = 0.0;
            vertexpos.y += ((backset * sign(corner.y)) * pixellength.y) / pixellength.x;
            coverage = ((coverage - 0.5) * abs(corner.x)) / (abs(corner.x) + backset) + 0.5;
        }
        if (aa_bloat_direction.y != 0.0 ? vertexpos.y * corner.y < 0.0 : false) {
            highp float backset = abs(vertexpos.y);
            vertexpos.y = 0.0;
            vertexpos.x += ((backset * sign(corner.x)) * pixellength.x) / pixellength.y;
            coverage = ((coverage - 0.5) * abs(corner.y)) / (abs(corner.y) + backset) + 0.5;
        }
    }
    highp mat2 skewmatrix = mat2(skew.xy, skew.zw);
    highp vec2 devcoord = vertexpos * skewmatrix + translate_and_localrotate;
    if (0.0 != is_linear_coverage) {
        varccoord_S0 = vec2(0.0, coverage * coverage_multiplier);
    } else {
        highp vec2 arccoord = (1.0 - abs(radius_outset)) + (aa_outset / radii) * corner;
        varccoord_S0 = vec2(arccoord.x + 1.0, arccoord.y);
    }
    gl_Position = vec4(devcoord, 0.0, 1.0);
    gl_Position = vec4(gl_Position.xy * sk_RTAdjust.xz + gl_Position.ww * sk_RTAdjust.yw, 0.0, gl_Position.w);
}
)"},
            {GL_FRAGMENT_SHADER,
             R"(#version 300 es

precision mediump float;
precision mediump sampler2D;
out mediump vec4 sk_FragColor;
in mediump vec4 vcolor_S0;
in highp vec2 varccoord_S0;
void main() {
    mediump vec4 outputColor_S0;
    outputColor_S0 = vcolor_S0;
    highp float x_plus_1 = varccoord_S0.x;
    highp float y = varccoord_S0.y;
    mediump float coverage;
    if (0.0 == x_plus_1) {
        coverage = y;
    } else {
        highp float fn = x_plus_1 * (x_plus_1 - 2.0);
        fn = ((y) * (y) + (fn));
        highp float fnwidth = fwidth(fn);
        coverage = 0.5 - fn / fnwidth;
        coverage = clamp(coverage, 0.0, 1.0);
    }
    mediump vec4 outputCoverage_S0 = vec4(coverage);
    {
        sk_FragColor = outputColor_S0 * outputCoverage_S0;
    }
}
)"},
        },
    },
    {
        "10447697909465823707",
        {
            {GL_VERTEX_SHADER,
             R"(#version 300 es

precision mediump float;
precision mediump sampler2D;
uniform highp vec4 sk_RTAdjust;
in highp vec2 position;
in mediump vec4 color;
out mediump vec4 vcolor_S0;
void main() {
    vcolor_S0 = color;
    gl_Position = vec4(position, 0.0, 1.0);
    gl_Position = vec4(gl_Position.xy * sk_RTAdjust.xz + gl_Position.ww * sk_RTAdjust.yw, 0.0, gl_Position.w);
}
)"},
            {GL_FRAGMENT_SHADER,
             R"(#version 300 es

precision mediump float;
precision mediump sampler2D;
out mediump vec4 sk_FragColor;
in mediump vec4 vcolor_S0;
void main() {
    {
        sk_FragColor = vec4(0.0);
    }
}
)"},
        },
    },
    {
        "11120690127791227782",
        {
            {GL_VERTEX_SHADER,
             R"(#version 300 es

precision mediump float;
precision mediump sampler2D;
uniform highp vec4 sk_RTAdjust;
uniform highp vec4 ulocalMatrix_S0;
in highp vec2 inPosition;
in mediump vec4 inColor;
in highp vec4 inCircleEdge;
out highp vec4 vinCircleEdge_S0;
out mediump vec4 vinColor_S0;
void main() {
    vinCircleEdge_S0 = inCircleEdge;
    vinColor_S0 = inColor;
    highp vec2 _tmp_0_inPosition = inPosition;
    gl_Position = vec4(_tmp_0_inPosition, 0.0, 1.0);
    gl_Position = vec4(gl_Position.xy * sk_RTAdjust.xz + gl_Position.ww * sk_RTAdjust.yw, 0.0, gl_Position.w);
}
)"},
            {GL_FRAGMENT_SHADER,
             R"(#version 300 es

precision mediump float;
precision mediump sampler2D;
out mediump vec4 sk_FragColor;
in highp vec4 vinCircleEdge_S0;
in mediump vec4 vinColor_S0;
void main() {
    highp vec4 circleEdge;
    circleEdge = vinCircleEdge_S0;
    highp float d = length(circleEdge.xy);
    mediump float distanceToOuterEdge = circleEdge.z * (1.0 - d);
    mediump float edgeAlpha = clamp(distanceToOuterEdge, 0.0, 1.0);
    mediump vec4 outputCoverage_S0 = vec4(edgeAlpha);
    {
        sk_FragColor = outputCoverage_S0;
        sk_FragColor = vec4(sk_FragColor.w, 0.0, 0.0, 0.0);
    }
}
)"},
        },
    },
    {
        "11695560673984095862",
        {
            {GL_VERTEX_SHADER,
             R"(#version 300 es

precision mediump float;
precision mediump sampler2D;
uniform highp vec4 sk_RTAdjust;
uniform highp mat3 umatrix_S1_c0;
in highp vec2 position;
in mediump vec4 color;
in highp vec2 localCoord;
out mediump vec4 vcolor_S0;
out highp vec2 vTransformedCoords_3_S0;
void main() {
    vcolor_S0 = color;
    gl_Position = vec4(position, 0.0, 1.0);
    {
        vTransformedCoords_3_S0 = mat3x2(umatrix_S1_c0) * vec3(localCoord, 1.0);
    }
    gl_Position = vec4(gl_Position.xy * sk_RTAdjust.xz + gl_Position.ww * sk_RTAdjust.yw, 0.0, gl_Position.w);
}
)"},
            {GL_FRAGMENT_SHADER,
             R"(#version 300 es

precision mediump float;
precision mediump sampler2D;
out mediump vec4 sk_FragColor;
uniform highp vec4 uclamp_S1_c0_c0;
uniform highp mat3 umatrix_S1_c0;
uniform sampler2D uTextureSampler_0_S1;
in mediump vec4 vcolor_S0;
in highp vec2 vTransformedCoords_3_S0;
void main() {
    mediump vec4 outputColor_S0;
    outputColor_S0 = vcolor_S0;
    mediump vec4 output_S1;
    mediump vec4 _8_input = outputColor_S0;
    highp vec2 _9_inCoord = vTransformedCoords_3_S0;
    highp vec2 _10_subsetCoord;
    _10_subsetCoord.x = _9_inCoord.x;
    _10_subsetCoord.y = _9_inCoord.y;
    highp vec2 _11_clampedCoord;
    _11_clampedCoord.x = _10_subsetCoord.x;
    _11_clampedCoord.y = clamp(_10_subsetCoord.y, uclamp_S1_c0_c0.y, uclamp_S1_c0_c0.w);
    mediump vec4 _12_textureColor = texture(uTextureSampler_0_S1, _11_clampedCoord, -0.5);
    _8_input = _12_textureColor;
    output_S1 = _8_input;
    {
        sk_FragColor = output_S1;
    }
}
)"},
        },
    },
    {
        "1258425782167183689",
        {
            {GL_VERTEX_SHADER,
             R"(#version 300 es

precision mediump float;
precision mediump sampler2D;
uniform highp vec4 sk_RTAdjust;
in highp vec2 position;
in highp float coverage;
in highp vec2 localCoord;
in highp vec4 texSubset;
out highp vec2 vlocalCoord_S0;
out highp vec4 vtexSubset_S0;
out highp float vcoverage_S0;
void main() {
    highp vec2 position = position;
    vlocalCoord_S0 = localCoord;
    vtexSubset_S0 = texSubset;
    vcoverage_S0 = coverage;
    gl_Position = vec4(position, 0.0, 1.0);
    gl_Position = vec4(gl_Position.xy * sk_RTAdjust.xz + gl_Position.ww * sk_RTAdjust.yw, 0.0, gl_Position.w);
}
)"},
            {GL_FRAGMENT_SHADER,
             R"(#version 300 es

precision mediump float;
precision mediump sampler2D;
out mediump vec4 sk_FragColor;
uniform sampler2D uTextureSampler_0_S0;
in highp vec2 vlocalCoord_S0;
in highp vec4 vtexSubset_S0;
in highp float vcoverage_S0;
void main() {
    mediump vec4 outputColor_S0 = vec4(1.0);
    highp vec2 texCoord;
    texCoord = vlocalCoord_S0;
    highp vec4 subset;
    subset = vtexSubset_S0;
    texCoord = clamp(texCoord, subset.xy, subset.zw);
    outputColor_S0 = texture(uTextureSampler_0_S0, texCoord, -0.5);
    highp float coverage = vcoverage_S0;
    mediump vec4 outputCoverage_S0 = vec4(coverage);
    {
        sk_FragColor = outputColor_S0 * outputCoverage_S0;
    }
}
)"},
        },
    },
    {
        "13167178478044706769",
        {
            {GL_VERTEX_SHADER,
             R"(#version 300 es

precision mediump float;
precision mediump sampler2D;
uniform highp vec4 sk_RTAdjust;
in highp vec2 position;
in mediump vec4 color;
out mediump vec4 vcolor_S0;
void main() {
    vcolor_S0 = color;
    gl_Position = vec4(position, 0.0, 1.0);
    gl_Position = vec4(gl_Position.xy * sk_RTAdjust.xz + gl_Position.ww * sk_RTAdjust.yw, 0.0, gl_Position.w);
}
)"},
            {GL_FRAGMENT_SHADER,
             R"(#version 300 es

uniform highp vec2 u_skRTFlip;
precision mediump float;
precision mediump sampler2D;
out mediump vec4 sk_FragColor;
uniform highp mat3 umatrix_S1_c0_c0_c0;
uniform highp vec4 urect_S1_c0_c0;
uniform sampler2D uTextureSampler_0_S1;
in mediump vec4 vcolor_S0;
void main() {
highp     vec4 sk_FragCoord = vec4(gl_FragCoord.x, u_skRTFlip.x + u_skRTFlip.y * gl_FragCoord.y, gl_FragCoord.z, gl_FragCoord.w);
    mediump vec4 outputColor_S0;
    outputColor_S0 = vcolor_S0;
    mediump vec4 output_S1;
    highp vec2 _11_tmp_1_coords = sk_FragCoord.xy;
    mediump float _12_xCoverage;
    mediump float _13_yCoverage;
    {
        mediump vec2 _14_xy = max(urect_S1_c0_c0.xy - _11_tmp_1_coords, _11_tmp_1_coords - urect_S1_c0_c0.zw);
        _12_xCoverage = texture(uTextureSampler_0_S1, mat3x2(umatrix_S1_c0_c0_c0) * vec3(vec2(_14_xy.x, 0.5), 1.0), -0.5).x;
        _13_yCoverage = texture(uTextureSampler_0_S1, mat3x2(umatrix_S1_c0_c0_c0) * vec3(vec2(_14_xy.y, 0.5), 1.0), -0.5).x;
    }
    output_S1 = vec4(_12_xCoverage * _13_yCoverage);
    {
        sk_FragColor = outputColor_S0 * output_S1;
    }
}
)"},
        },
    },
    {
        "14028004723287305347",
        {
            {GL_VERTEX_SHADER,
             R"(#version 300 es

precision mediump float;
precision mediump sampler2D;
uniform highp vec4 sk_RTAdjust;
uniform highp mat3 umatrix_S1;
in highp vec2 position;
in highp vec2 localCoord;
out highp vec2 vTransformedCoords_2_S0;
void main() {
    gl_Position = vec4(position, 0.0, 1.0);
    {
        vTransformedCoords_2_S0 = mat3x2(umatrix_S1) * vec3(localCoord, 1.0);
    }
    gl_Position = vec4(gl_Position.xy * sk_RTAdjust.xz + gl_Position.ww * sk_RTAdjust.yw, 0.0, gl_Position.w);
}
)"},
            {GL_FRAGMENT_SHADER,
             R"(#version 300 es

precision mediump float;
precision mediump sampler2D;
out mediump vec4 sk_FragColor;
uniform highp vec4 uclamp_S1_c0_c0_c0;
uniform highp mat3 umatrix_S1_c0_c0;
uniform mediump vec2 uIncrement_S1_c0;
uniform mediump vec2 uOffsetsAndKernel_S1_c0[4];
uniform highp mat3 umatrix_S1;
uniform sampler2D uTextureSampler_0_S1;
in highp vec2 vTransformedCoords_2_S0;
void main() {
    mediump vec4 output_S1;
    mediump vec4 _21_color = vec4(0.0);
    highp vec2 _22_coord = vTransformedCoords_2_S0;
    for (highp int _23_i = 0;_23_i < 4; ++_23_i) {
        mediump vec2 _24_offsetAndKernel = uOffsetsAndKernel_S1_c0[_23_i];
        highp vec2 _25_inCoord = mat3x2(umatrix_S1_c0_c0) * vec3(_22_coord + _24_offsetAndKernel.x * uIncrement_S1_c0, 1.0);
        highp vec2 _26_subsetCoord;
        _26_subsetCoord.x = _25_inCoord.x;
        _26_subsetCoord.y = _25_inCoord.y;
        highp vec2 _27_clampedCoord;
        _27_clampedCoord.x = clamp(_26_subsetCoord.x, uclamp_S1_c0_c0_c0.x, uclamp_S1_c0_c0_c0.z);
        _27_clampedCoord.y = _26_subsetCoord.y;
        mediump vec4 _28_textureColor = vec4(0.0, 0.0, 0.0, texture(uTextureSampler_0_S1, _27_clampedCoord, -0.5).x);
        _21_color += _28_textureColor * _24_offsetAndKernel.y;
    }
    output_S1 = _21_color;
    {
        sk_FragColor = output_S1;
        sk_FragColor = vec4(sk_FragColor.w, 0.0, 0.0, 0.0);
    }
}
)"},
        },
    },
    {
        "14821118696134815087",
        {
            {GL_VERTEX_SHADER,
             R"(#version 300 es

precision mediump float;
precision mediump sampler2D;
uniform highp vec4 sk_RTAdjust;
in highp vec2 position;
in mediump vec4 color;
out mediump vec4 vcolor_S0;
void main() {
    vcolor_S0 = color;
    gl_Position = vec4(position, 0.0, 1.0);
    gl_Position = vec4(gl_Position.xy * sk_RTAdjust.xz + gl_Position.ww * sk_RTAdjust.yw, 0.0, gl_Position.w);
}
)"},
            {GL_FRAGMENT_SHADER,
             R"(#version 300 es

uniform highp vec2 u_skRTFlip;
precision mediump float;
precision mediump sampler2D;
out mediump vec4 sk_FragColor;
uniform highp mat3 umatrix_S1_c0;
uniform mediump vec2 ucoverageInvert_S1;
uniform sampler2D uTextureSampler_0_S1;
in mediump vec4 vcolor_S0;
void main() {
highp     vec4 sk_FragCoord = vec4(gl_FragCoord.x, u_skRTFlip.x + u_skRTFlip.y * gl_FragCoord.y, gl_FragCoord.z, gl_FragCoord.w);
    mediump vec4 outputColor_S0;
    outputColor_S0 = vcolor_S0;
    mediump vec4 output_S1;
    mediump float _0_coverage = 0.0;
    {
        _0_coverage = texture(uTextureSampler_0_S1, mat3x2(umatrix_S1_c0) * vec3(sk_FragCoord.xy, 1.0), -0.5).x;
    }
    _0_coverage = _0_coverage * ucoverageInvert_S1.x + ucoverageInvert_S1.y;
    output_S1 = vec4(_0_coverage);
    {
        sk_FragColor = outputColor_S0 * output_S1;
    }
}
)"},
        },
    },
    {
        "14886662037881130532",
        {
            {GL_VERTEX_SHADER,
             R"(#version 300 es

precision mediump float;
precision mediump sampler2D;
uniform highp vec4 sk_RTAdjust;
uniform highp mat3 umatrix_S1;
in highp vec2 position;
in highp vec2 localCoord;
out highp vec2 vTransformedCoords_2_S0;
void main() {
    gl_Position = vec4(position, 0.0, 1.0);
    {
        vTransformedCoords_2_S0 = mat3x2(umatrix_S1) * vec3(localCoord, 1.0);
    }
    gl_Position = vec4(gl_Position.xy * sk_RTAdjust.xz + gl_Position.ww * sk_RTAdjust.yw, 0.0, gl_Position.w);
}
)"},
            {GL_FRAGMENT_SHADER,
             R"(#version 300 es

precision mediump float;
precision mediump sampler2D;
out mediump vec4 sk_FragColor;
uniform highp vec4 uclamp_S1_c0_c0_c0;
uniform highp mat3 umatrix_S1_c0_c0;
uniform mediump vec2 uIncrement_S1_c0;
uniform mediump vec2 uOffsetsAndKernel_S1_c0[10];
uniform highp mat3 umatrix_S1;
uniform sampler2D uTextureSampler_0_S1;
in highp vec2 vTransformedCoords_2_S0;
void main() {
    mediump vec4 output_S1;
    mediump vec4 _21_color = vec4(0.0);
    highp vec2 _22_coord = vTransformedCoords_2_S0;
    for (highp int _23_i = 0;_23_i < 10; ++_23_i) {
        mediump vec2 _24_offsetAndKernel = uOffsetsAndKernel_S1_c0[_23_i];
        highp vec2 _25_inCoord = mat3x2(umatrix_S1_c0_c0) * vec3(_22_coord + _24_offsetAndKernel.x * uIncrement_S1_c0, 1.0);
        highp vec2 _26_subsetCoord;
        _26_subsetCoord.x = _25_inCoord.x;
        _26_subsetCoord.y = _25_inCoord.y;
        highp vec2 _27_clampedCoord;
        _27_clampedCoord.x = _26_subsetCoord.x;
        _27_clampedCoord.y = clamp(_26_subsetCoord.y, uclamp_S1_c0_c0_c0.y, uclamp_S1_c0_c0_c0.w);
        mediump vec4 _28_textureColor = vec4(0.0, 0.0, 0.0, texture(uTextureSampler_0_S1, _27_clampedCoord, -0.5).x);
        _21_color += _28_textureColor * _24_offsetAndKernel.y;
    }
    output_S1 = _21_color;
    {
        sk_FragColor = output_S1;
        sk_FragColor = vec4(sk_FragColor.w, 0.0, 0.0, 0.0);
    }
}
)"},
        },
    },
    {
        "15018417907435518591",
        {
            {GL_VERTEX_SHADER,
             R"(#version 300 es

precision mediump float;
precision mediump sampler2D;
const highp float PRECISION = 4.0;
const highp float MAX_FIXED_RESOLVE_LEVEL = 5.0;
const highp float MAX_FIXED_SEGMENTS = 32.0;
uniform highp vec4 sk_RTAdjust;
uniform highp vec4 uaffineMatrix_S0;
uniform highp vec2 utranslate_S0;
in highp vec2 resolveLevel_and_idx;
in highp vec4 p01;
in highp vec4 p23;
in highp vec2 fanPointAttrib;
highp float wangs_formula_max_fdiff_p2_ff2f2f2f2f22(highp vec2 p0, highp vec2 p1, highp vec2 p2, highp vec2 p3, highp mat2 matrix) {
    highp vec2 d0 = matrix * (((vec2(-2.0)) * (p1) + (p2)) + p0);
    highp vec2 d1 = matrix * (((vec2(-2.0)) * (p2) + (p3)) + p1);
    return max(dot(d0, d0), dot(d1, d1));
}
highp float wangs_formula_conic_p2_fff2f2f2f(highp float _precision_, highp vec2 p0, highp vec2 p1, highp vec2 p2, highp float w) {
    highp vec2 C = (min(min(p0, p1), p2) + max(max(p0, p1), p2)) * 0.5;
    p0 -= C;
    p1 -= C;
    p2 -= C;
    highp float m = sqrt(max(max(dot(p0, p0), dot(p1, p1)), dot(p2, p2)));
    highp vec2 dp = ((vec2(-2.0 * w)) * (p1) + (p0)) + p2;
    highp float dw = abs(((-2.0) * (w) + (2.0)));
    highp float rp_minus_1 = max(0.0, ((m) * (_precision_) + (-1.0)));
    highp float numer = length(dp) * _precision_ + rp_minus_1 * dw;
    highp float denom = 4.0 * min(w, 1.0);
    return numer / denom;
}
void main() {
    highp mat2 AFFINE_MATRIX = mat2(uaffineMatrix_S0.xy, uaffineMatrix_S0.zw);
    highp vec2 TRANSLATE = utranslate_S0;
    highp float resolveLevel = resolveLevel_and_idx.x;
    highp float idxInResolveLevel = resolveLevel_and_idx.y;
    highp vec2 localcoord;
    if (resolveLevel < 0.0) {
        localcoord = fanPointAttrib;
    } else {
        if (isinf(p23.z)) {
            localcoord = resolveLevel != 0.0 ? p01.zw : (idxInResolveLevel != 0.0 ? p23.xy : p01.xy);
        } else {
            highp vec2 p0 = p01.xy;
            highp vec2 p1 = p01.zw;
            highp vec2 p2 = p23.xy;
            highp vec2 p3 = p23.zw;
            highp float w = -1.0;
            highp float maxResolveLevel;
            if (isinf(p23.w)) {
                w = p3.x;
                highp float _0_n2 = wangs_formula_conic_p2_fff2f2f2f(PRECISION, AFFINE_MATRIX * p0, AFFINE_MATRIX * p1, AFFINE_MATRIX * p2, w);
                maxResolveLevel = ceil(log2(max(_0_n2, 1.0)) * 0.5);
                p1 *= w;
                p3 = p2;
            } else {
                highp float _1_m = wangs_formula_max_fdiff_p2_ff2f2f2f2f22(p0, p1, p2, p3, AFFINE_MATRIX);
                maxResolveLevel = ceil(log2(max(9.0 * _1_m, 1.0)) * 0.25);
            }
            if (resolveLevel > maxResolveLevel) {
                idxInResolveLevel = floor(idxInResolveLevel * exp2(maxResolveLevel - resolveLevel));
                resolveLevel = maxResolveLevel;
            }
            highp float fixedVertexID = floor(0.5 + idxInResolveLevel * exp2(MAX_FIXED_RESOLVE_LEVEL - resolveLevel));
            if (0.0 < fixedVertexID ? fixedVertexID < MAX_FIXED_SEGMENTS : false) {
                highp float T = fixedVertexID * 0.03125;
                highp vec2 ab = mix(p0, p1, T);
                highp vec2 bc = mix(p1, p2, T);
                highp vec2 cd = mix(p2, p3, T);
                highp vec2 abc = mix(ab, bc, T);
                highp vec2 bcd = mix(bc, cd, T);
                highp vec2 abcd = mix(abc, bcd, T);
                highp float u = mix(1.0, w, T);
                highp float v = (w + 1.0) - u;
                highp float uv = mix(u, v, T);
                localcoord = w < 0.0 ? abcd : abc / uv;
            } else {
                localcoord = fixedVertexID == 0.0 ? p0 : p3;
            }
        }
    }
    highp vec2 vertexpos = AFFINE_MATRIX * localcoord + TRANSLATE;
    gl_Position = vec4(vertexpos, 0.0, 1.0);
    gl_Position = vec4(gl_Position.xy * sk_RTAdjust.xz + gl_Position.ww * sk_RTAdjust.yw, 0.0, gl_Position.w);
}
)"},
            {GL_FRAGMENT_SHADER,
             R"(#version 300 es

precision mediump float;
precision mediump sampler2D;
uniform mediump vec4 ucolor_S0;
void main() {
}
)"},
        },
    },
    {
        "15043701738123506499",
        {
            {GL_VERTEX_SHADER,
             R"(#version 300 es

precision mediump float;
precision mediump sampler2D;
uniform highp vec4 sk_RTAdjust;
uniform highp mat3 umatrix_S1;
in highp vec2 position;
in mediump vec4 color;
in highp vec2 localCoord;
out mediump vec4 vcolor_S0;
out highp vec2 vTransformedCoords_2_S0;
void main() {
    vcolor_S0 = color;
    gl_Position = vec4(position, 0.0, 1.0);
    {
        vTransformedCoords_2_S0 = mat3x2(umatrix_S1) * vec3(localCoord, 1.0);
    }
    gl_Position = vec4(gl_Position.xy * sk_RTAdjust.xz + gl_Position.ww * sk_RTAdjust.yw, 0.0, gl_Position.w);
}
)"},
            {GL_FRAGMENT_SHADER,
             R"(#version 300 es

precision mediump float;
precision mediump sampler2D;
out mediump vec4 sk_FragColor;
uniform highp mat3 umatrix_S1;
uniform sampler2D uTextureSampler_0_S1;
in mediump vec4 vcolor_S0;
in highp vec2 vTransformedCoords_2_S0;
void main() {
    mediump vec4 outputColor_S0;
    outputColor_S0 = vcolor_S0;
    mediump vec4 output_S1;
    output_S1 = texture(uTextureSampler_0_S1, vTransformedCoords_2_S0, -0.5).xxxx;
    {
        sk_FragColor = outputColor_S0 * output_S1;
    }
}
)"},
        },
    },
    {
        "15519165880928771938",
        {
            {GL_VERTEX_SHADER,
             R"(#version 300 es

precision mediump float;
precision mediump sampler2D;
uniform highp vec4 sk_RTAdjust;
uniform highp vec4 ulocalMatrix_S0;
in highp vec2 inPosition;
in mediump vec4 inColor;
in highp vec4 inCircleEdge;
out highp vec4 vinCircleEdge_S0;
out mediump vec4 vinColor_S0;
void main() {
    vinCircleEdge_S0 = inCircleEdge;
    vinColor_S0 = inColor;
    highp vec2 _tmp_0_inPosition = inPosition;
    gl_Position = vec4(_tmp_0_inPosition, 0.0, 1.0);
    gl_Position = vec4(gl_Position.xy * sk_RTAdjust.xz + gl_Position.ww * sk_RTAdjust.yw, 0.0, gl_Position.w);
}
)"},
            {GL_FRAGMENT_SHADER,
             R"(#version 300 es

precision mediump float;
precision mediump sampler2D;
out mediump vec4 sk_FragColor;
in highp vec4 vinCircleEdge_S0;
in mediump vec4 vinColor_S0;
void main() {
    highp vec4 circleEdge;
    circleEdge = vinCircleEdge_S0;
    mediump vec4 outputColor_S0;
    outputColor_S0 = vinColor_S0;
    highp float d = length(circleEdge.xy);
    mediump float distanceToOuterEdge = circleEdge.z * (1.0 - d);
    mediump float edgeAlpha = clamp(distanceToOuterEdge, 0.0, 1.0);
    mediump vec4 outputCoverage_S0 = vec4(edgeAlpha);
    {
        sk_FragColor = outputColor_S0 * outputCoverage_S0;
    }
}
)"},
        },
    },
    {
        "1605918475496372484",
        {
            {GL_VERTEX_SHADER,
             R"(#version 300 es

precision mediump float;
precision mediump sampler2D;
uniform highp vec4 sk_RTAdjust;
in highp vec2 position;
in mediump vec4 color;
out mediump vec4 vcolor_S0;
void main() {
    vcolor_S0 = color;
    gl_Position = vec4(position, 0.0, 1.0);
    gl_Position = vec4(gl_Position.xy * sk_RTAdjust.xz + gl_Position.ww * sk_RTAdjust.yw, 0.0, gl_Position.w);
}
)"},
            {GL_FRAGMENT_SHADER,
             R"(#version 300 es

precision mediump float;
precision mediump sampler2D;
out mediump vec4 sk_FragColor;
in mediump vec4 vcolor_S0;
void main() {
    mediump vec4 outputColor_S0;
    outputColor_S0 = vcolor_S0;
    {
        sk_FragColor = outputColor_S0;
        sk_FragColor = vec4(sk_FragColor.w, 0.0, 0.0, 0.0);
    }
}
)"},
        },
    },
    {
        "16403000803783882409",
        {
            {GL_VERTEX_SHADER,
             R"(#version 300 es

precision mediump float;
precision mediump sampler2D;
uniform highp vec4 sk_RTAdjust;
in highp vec2 position;
in mediump vec4 color;
out mediump vec4 vcolor_S0;
void main() {
    vcolor_S0 = color;
    gl_Position = vec4(position, 0.0, 1.0);
    gl_Position = vec4(gl_Position.xy * sk_RTAdjust.xz + gl_Position.ww * sk_RTAdjust.yw, 0.0, gl_Position.w);
}
)"},
            {GL_FRAGMENT_SHADER,
             R"(#version 300 es

precision mediump float;
precision mediump sampler2D;
out mediump vec4 sk_FragColor;
in mediump vec4 vcolor_S0;
void main() {
    mediump vec4 outputColor_S0;
    outputColor_S0 = vcolor_S0;
    {
        sk_FragColor = outputColor_S0;
    }
}
)"},
        },
    },
    {
        "16779675656727705765",
        {
            {GL_VERTEX_SHADER,
             R"(#version 300 es

precision mediump float;
precision mediump sampler2D;
uniform highp vec4 sk_RTAdjust;
uniform highp vec2 uatlas_adjust_S0;
in highp vec4 fillBounds;
in mediump vec4 color;
in highp vec4 locations;
out highp vec2 vatlasCoord_S0;
out mediump vec4 vcolor_S0;
void main() {
    highp vec2 unitCoord = vec2(float(gl_VertexID & 1), float(gl_VertexID >> 1));
    highp vec2 devCoord = mix(fillBounds.xy, fillBounds.zw, unitCoord);
    highp vec2 atlasTopLeft = vec2(abs(locations.x) - 1.0, locations.y);
    highp vec2 devTopLeft = locations.zw;
    bool transposed = locations.x < 0.0;
    highp vec2 atlasCoord = devCoord - devTopLeft;
    if (transposed) {
        atlasCoord = atlasCoord.yx;
    }
    atlasCoord += atlasTopLeft;
    vatlasCoord_S0 = atlasCoord * uatlas_adjust_S0;
    vcolor_S0 = color;
    gl_Position = vec4(devCoord, 0.0, 1.0);
    gl_Position = vec4(gl_Position.xy * sk_RTAdjust.xz + gl_Position.ww * sk_RTAdjust.yw, 0.0, gl_Position.w);
}
)"},
            {GL_FRAGMENT_SHADER,
             R"(#version 300 es

precision mediump float;
precision mediump sampler2D;
out mediump vec4 sk_FragColor;
uniform sampler2D uTextureSampler_0_S0;
in highp vec2 vatlasCoord_S0;
in mediump vec4 vcolor_S0;
void main() {
    mediump vec4 outputCoverage_S0 = vec4(1.0);
    mediump float atlasCoverage = texture(uTextureSampler_0_S0, vatlasCoord_S0, -0.5).x;
    outputCoverage_S0 *= atlasCoverage;
    mediump vec4 outputColor_S0;
    outputColor_S0 = vcolor_S0;
    {
        sk_FragColor = outputColor_S0 * outputCoverage_S0;
    }
}
)"},
        },
    },
    {
        "16996936725532664887",
        {
            {GL_VERTEX_SHADER,
             R"(#version 300 es

precision mediump float;
precision mediump sampler2D;
uniform highp vec4 sk_RTAdjust;
uniform highp vec2 uAtlasSizeInv_S0;
in highp vec2 inPosition;
in mediump vec4 inColor;
in mediump uvec2 inTextureCoords;
out highp vec2 vTextureCoords_S0;
out highp float vTexIndex_S0;
out mediump vec4 vinColor_S0;
void main() {
    highp int texIdx = 0;
    highp vec2 unormTexCoords = vec2(float(inTextureCoords.x), float(inTextureCoords.y));
    vTextureCoords_S0 = unormTexCoords * uAtlasSizeInv_S0;
    vTexIndex_S0 = float(texIdx);
    vinColor_S0 = inColor;
    gl_Position = vec4(inPosition, 0.0, 1.0);
    gl_Position = vec4(gl_Position.xy * sk_RTAdjust.xz + gl_Position.ww * sk_RTAdjust.yw, 0.0, gl_Position.w);
}
)"},
            {GL_FRAGMENT_SHADER,
             R"(#version 300 es

precision mediump float;
precision mediump sampler2D;
out mediump vec4 sk_FragColor;
uniform sampler2D uTextureSampler_0_S0;
in highp vec2 vTextureCoords_S0;
in highp float vTexIndex_S0;
in mediump vec4 vinColor_S0;
void main() {
    mediump vec4 outputColor_S0;
    outputColor_S0 = vinColor_S0;
    mediump vec4 texColor;
    {
        texColor = texture(uTextureSampler_0_S0, vTextureCoords_S0, -0.5).xxxx;
    }
    mediump vec4 outputCoverage_S0 = texColor;
    {
        sk_FragColor = outputColor_S0 * outputCoverage_S0;
    }
}
)"},
        },
    },
    {
        "1798731529094074495",
        {
            {GL_VERTEX_SHADER,
             R"(#version 300 es

precision mediump float;
precision mediump sampler2D;
uniform highp vec4 sk_RTAdjust;
uniform highp mat3 umatrix_S1;
in highp vec2 position;
in highp vec2 localCoord;
out highp vec2 vTransformedCoords_2_S0;
void main() {
    gl_Position = vec4(position, 0.0, 1.0);
    {
        vTransformedCoords_2_S0 = mat3x2(umatrix_S1) * vec3(localCoord, 1.0);
    }
    gl_Position = vec4(gl_Position.xy * sk_RTAdjust.xz + gl_Position.ww * sk_RTAdjust.yw, 0.0, gl_Position.w);
}
)"},
            {GL_FRAGMENT_SHADER,
             R"(#version 300 es

precision mediump float;
precision mediump sampler2D;
out mediump vec4 sk_FragColor;
uniform highp vec4 uclamp_S1_c0_c0_c0;
uniform highp mat3 umatrix_S1_c0_c0;
uniform mediump vec2 uIncrement_S1_c0;
uniform mediump vec2 uOffsetsAndKernel_S1_c0[4];
uniform highp mat3 umatrix_S1;
uniform sampler2D uTextureSampler_0_S1;
in highp vec2 vTransformedCoords_2_S0;
void main() {
    mediump vec4 output_S1;
    mediump vec4 _21_color = vec4(0.0);
    highp vec2 _22_coord = vTransformedCoords_2_S0;
    for (highp int _23_i = 0;_23_i < 4; ++_23_i) {
        mediump vec2 _24_offsetAndKernel = uOffsetsAndKernel_S1_c0[_23_i];
        highp vec2 _25_inCoord = mat3x2(umatrix_S1_c0_c0) * vec3(_22_coord + _24_offsetAndKernel.x * uIncrement_S1_c0, 1.0);
        highp vec2 _26_subsetCoord;
        _26_subsetCoord.x = _25_inCoord.x;
        _26_subsetCoord.y = _25_inCoord.y;
        highp vec2 _27_clampedCoord;
        _27_clampedCoord.x = _26_subsetCoord.x;
        _27_clampedCoord.y = clamp(_26_subsetCoord.y, uclamp_S1_c0_c0_c0.y, uclamp_S1_c0_c0_c0.w);
        mediump vec4 _28_textureColor = vec4(0.0, 0.0, 0.0, texture(uTextureSampler_0_S1, _27_clampedCoord, -0.5).x);
        _21_color += _28_textureColor * _24_offsetAndKernel.y;
    }
    output_S1 = _21_color;
    {
        sk_FragColor = output_S1;
        sk_FragColor = vec4(sk_FragColor.w, 0.0, 0.0, 0.0);
    }
}
)"},
        },
    },
    {
        "18426746490946486420",
        {
            {GL_VERTEX_SHADER,
             R"(#version 300 es

precision mediump float;
precision mediump sampler2D;
uniform highp vec4 sk_RTAdjust;
in highp vec2 inPosition;
in mediump vec4 inColor;
in highp vec4 inCircleEdge;
out highp vec4 vinCircleEdge_S0;
out mediump vec4 vinColor_S0;
void main() {
    vinCircleEdge_S0 = inCircleEdge;
    vinColor_S0 = inColor;
    highp vec2 _tmp_0_inPosition = inPosition;
    gl_Position = vec4(_tmp_0_inPosition, 0.0, 1.0);
    gl_Position = vec4(gl_Position.xy * sk_RTAdjust.xz + gl_Position.ww * sk_RTAdjust.yw, 0.0, gl_Position.w);
}
)"},
            {GL_FRAGMENT_SHADER,
             R"(#version 300 es

precision mediump float;
precision mediump sampler2D;
out mediump vec4 sk_FragColor;
in highp vec4 vinCircleEdge_S0;
in mediump vec4 vinColor_S0;
void main() {
    highp vec4 circleEdge;
    circleEdge = vinCircleEdge_S0;
    mediump vec4 outputColor_S0;
    outputColor_S0 = vinColor_S0;
    highp float d = length(circleEdge.xy);
    mediump float distanceToOuterEdge = circleEdge.z * (1.0 - d);
    mediump float edgeAlpha = clamp(distanceToOuterEdge, 0.0, 1.0);
    mediump float distanceToInnerEdge = circleEdge.z * (d - circleEdge.w);
    mediump float innerAlpha = clamp(distanceToInnerEdge, 0.0, 1.0);
    edgeAlpha *= innerAlpha;
    mediump vec4 outputCoverage_S0 = vec4(edgeAlpha);
    {
        sk_FragColor = outputColor_S0 * outputCoverage_S0;
    }
}
)"},
        },
    },
    {
        "2018606693716023504",
        {
            {GL_VERTEX_SHADER,
             R"(#version 300 es

precision mediump float;
precision mediump sampler2D;
uniform highp vec4 sk_RTAdjust;
uniform highp mat3 umatrix_S1_c0;
in highp vec2 position;
in mediump vec4 color;
in highp vec2 localCoord;
out mediump vec4 vcolor_S0;
out highp vec2 vTransformedCoords_3_S0;
void main() {
    vcolor_S0 = color;
    gl_Position = vec4(position, 0.0, 1.0);
    {
        vTransformedCoords_3_S0 = mat3x2(umatrix_S1_c0) * vec3(localCoord, 1.0);
    }
    gl_Position = vec4(gl_Position.xy * sk_RTAdjust.xz + gl_Position.ww * sk_RTAdjust.yw, 0.0, gl_Position.w);
}
)"},
            {GL_FRAGMENT_SHADER,
             R"(#version 300 es

precision mediump float;
precision mediump sampler2D;
out mediump vec4 sk_FragColor;
uniform highp vec4 uclamp_S1_c0_c0;
uniform highp mat3 umatrix_S1_c0;
uniform sampler2D uTextureSampler_0_S1;
in mediump vec4 vcolor_S0;
in highp vec2 vTransformedCoords_3_S0;
void main() {
    mediump vec4 outputColor_S0;
    outputColor_S0 = vcolor_S0;
    mediump vec4 output_S1;
    mediump vec4 _8_input = outputColor_S0;
    highp vec2 _9_inCoord = vTransformedCoords_3_S0;
    highp vec2 _10_subsetCoord;
    _10_subsetCoord.x = _9_inCoord.x;
    _10_subsetCoord.y = _9_inCoord.y;
    highp vec2 _11_clampedCoord;
    _11_clampedCoord.x = clamp(_10_subsetCoord.x, uclamp_S1_c0_c0.x, uclamp_S1_c0_c0.z);
    _11_clampedCoord.y = _10_subsetCoord.y;
    mediump vec4 _12_textureColor = texture(uTextureSampler_0_S1, _11_clampedCoord, -0.5);
    _8_input = _12_textureColor;
    output_S1 = _8_input;
    {
        sk_FragColor = output_S1;
    }
}
)"},
        },
    },
    {
        "208898923065600589",
        {
            {GL_VERTEX_SHADER,
             R"(#version 300 es

precision mediump float;
precision mediump sampler2D;
uniform highp vec4 sk_RTAdjust;
uniform highp mat3 umatrix_S1_c0;
in highp vec2 position;
in mediump vec4 color;
in highp vec2 localCoord;
out mediump vec4 vcolor_S0;
out highp vec2 vTransformedCoords_3_S0;
void main() {
    vcolor_S0 = color;
    gl_Position = vec4(position, 0.0, 1.0);
    {
        vTransformedCoords_3_S0 = mat3x2(umatrix_S1_c0) * vec3(localCoord, 1.0);
    }
    gl_Position = vec4(gl_Position.xy * sk_RTAdjust.xz + gl_Position.ww * sk_RTAdjust.yw, 0.0, gl_Position.w);
}
)"},
            {GL_FRAGMENT_SHADER,
             R"(#version 300 es

uniform highp vec2 u_skRTFlip;
precision mediump float;
precision mediump sampler2D;
out mediump vec4 sk_FragColor;
uniform highp mat3 umatrix_S1_c0;
uniform highp vec4 ucircle_S2_c0;
uniform sampler2D uTextureSampler_0_S1;
in mediump vec4 vcolor_S0;
in highp vec2 vTransformedCoords_3_S0;
void main() {
highp     vec4 sk_FragCoord = vec4(gl_FragCoord.x, u_skRTFlip.x + u_skRTFlip.y * gl_FragCoord.y, gl_FragCoord.z, gl_FragCoord.w);
    mediump vec4 outputColor_S0;
    outputColor_S0 = vcolor_S0;
    mediump vec4 output_S1;
    mediump vec4 _2_input = outputColor_S0;
    _2_input = texture(uTextureSampler_0_S1, vTransformedCoords_3_S0, -0.5);
    output_S1 = _2_input;
    mediump vec4 output_S2;
    mediump float _5_d;
    {
        _5_d = (1.0 - length((ucircle_S2_c0.xy - sk_FragCoord.xy) * ucircle_S2_c0.w)) * ucircle_S2_c0.z;
    }
    output_S2 = vec4(clamp(_5_d, 0.0, 1.0));
    {
        sk_FragColor = output_S1 * output_S2;
    }
}
)"},
        },
    },
    {
        "3232265438052441951",
        {
            {GL_VERTEX_SHADER,
             R"(#version 300 es

precision mediump float;
precision mediump sampler2D;
in mediump vec2 a_vertex;
uniform mediump vec4 u_texCoordXform;
uniform mediump vec4 u_posXform;
out mediump vec2 v_texCoord;
void main() {
    v_texCoord = a_vertex * u_texCoordXform.xy + u_texCoordXform.zw;
    gl_Position.xy = a_vertex * u_posXform.xy + u_posXform.zw;
    gl_Position.zw = vec2(0.0, 1.0);
}
)"},
            {GL_FRAGMENT_SHADER,
             R"(#version 300 es

precision mediump float;
precision mediump sampler2D;
out mediump vec4 sk_FragColor;
in mediump vec2 v_texCoord;
uniform sampler2D u_texture;
void main() {
    sk_FragColor = texture(u_texture, v_texCoord);
}
)"},
        },
    },
    {
        "3647734197196143142",
        {
            {GL_VERTEX_SHADER,
             R"(#version 300 es

precision mediump float;
precision mediump sampler2D;
uniform highp vec4 sk_RTAdjust;
uniform highp mat3 umatrix_S1;
in highp vec2 position;
in mediump vec4 color;
in highp vec2 localCoord;
out mediump vec4 vcolor_S0;
out highp vec2 vTransformedCoords_2_S0;
void main() {
    vcolor_S0 = color;
    gl_Position = vec4(position, 0.0, 1.0);
    {
        vTransformedCoords_2_S0 = mat3x2(umatrix_S1) * vec3(localCoord, 1.0);
    }
    gl_Position = vec4(gl_Position.xy * sk_RTAdjust.xz + gl_Position.ww * sk_RTAdjust.yw, 0.0, gl_Position.w);
}
)"},
            {GL_FRAGMENT_SHADER,
             R"(#version 300 es

uniform highp vec2 u_skRTFlip;
precision mediump float;
precision mediump sampler2D;
out mediump vec4 sk_FragColor;
uniform highp mat3 umatrix_S1;
uniform highp vec4 uinnerRect_S2;
uniform mediump vec2 uradiusPlusHalf_S2;
uniform sampler2D uTextureSampler_0_S1;
in mediump vec4 vcolor_S0;
in highp vec2 vTransformedCoords_2_S0;
void main() {
highp     vec4 sk_FragCoord = vec4(gl_FragCoord.x, u_skRTFlip.x + u_skRTFlip.y * gl_FragCoord.y, gl_FragCoord.z, gl_FragCoord.w);
    mediump vec4 outputColor_S0;
    outputColor_S0 = vcolor_S0;
    mediump vec4 output_S1;
    output_S1 = texture(uTextureSampler_0_S1, vTransformedCoords_2_S0, -0.5).xxxx;
    mediump vec4 output_S2;
    highp vec2 _0_dxy0 = uinnerRect_S2.xy - sk_FragCoord.xy;
    highp vec2 _1_dxy1 = sk_FragCoord.xy - uinnerRect_S2.zw;
    highp vec2 _2_dxy = max(max(_0_dxy0, _1_dxy1), 0.0);
    mediump float _3_alpha = clamp(uradiusPlusHalf_S2.x - length(_2_dxy), 0.0, 1.0);
    _3_alpha = 1.0 - _3_alpha;
    output_S2 = output_S1 * _3_alpha;
    {
        sk_FragColor = outputColor_S0 * output_S2;
    }
}
)"},
        },
    },
    {
        "5215762711036580956",
        {
            {GL_VERTEX_SHADER,
             R"(#version 300 es

precision mediump float;
precision mediump sampler2D;
uniform highp vec4 sk_RTAdjust;
in highp vec2 position;
in highp vec2 localCoord;
out highp vec2 vlocalCoord_S0;
void main() {
    vlocalCoord_S0 = localCoord;
    gl_Position = vec4(position, 0.0, 1.0);
    gl_Position = vec4(gl_Position.xy * sk_RTAdjust.xz + gl_Position.ww * sk_RTAdjust.yw, 0.0, gl_Position.w);
}
)"},
            {GL_FRAGMENT_SHADER,
             R"(#version 300 es

uniform highp vec2 u_skRTFlip;
precision mediump float;
precision mediump sampler2D;
out mediump vec4 sk_FragColor;
uniform highp vec4 ucircle_S1_c0;
uniform sampler2D uTextureSampler_0_S0;
in highp vec2 vlocalCoord_S0;
void main() {
highp     vec4 sk_FragCoord = vec4(gl_FragCoord.x, u_skRTFlip.x + u_skRTFlip.y * gl_FragCoord.y, gl_FragCoord.z, gl_FragCoord.w);
    mediump vec4 outputColor_S0 = vec4(1.0);
    highp vec2 texCoord;
    texCoord = vlocalCoord_S0;
    outputColor_S0 = texture(uTextureSampler_0_S0, texCoord, -0.5);
    mediump vec4 output_S1;
    mediump float _3_d;
    {
        _3_d = (1.0 - length((ucircle_S1_c0.xy - sk_FragCoord.xy) * ucircle_S1_c0.w)) * ucircle_S1_c0.z;
    }
    output_S1 = vec4(clamp(_3_d, 0.0, 1.0));
    {
        sk_FragColor = outputColor_S0 * output_S1;
    }
}
)"},
        },
    },
    {
        "52812126979314805",
        {
            {GL_VERTEX_SHADER,
             R"(#version 300 es

precision mediump float;
precision mediump sampler2D;
uniform highp vec4 sk_RTAdjust;
uniform highp mat3 umatrix_S1;
in highp vec2 position;
in highp vec2 localCoord;
out highp vec2 vTransformedCoords_2_S0;
void main() {
    gl_Position = vec4(position, 0.0, 1.0);
    {
        vTransformedCoords_2_S0 = mat3x2(umatrix_S1) * vec3(localCoord, 1.0);
    }
    gl_Position = vec4(gl_Position.xy * sk_RTAdjust.xz + gl_Position.ww * sk_RTAdjust.yw, 0.0, gl_Position.w);
}
)"},
            {GL_FRAGMENT_SHADER,
             R"(#version 300 es

precision mediump float;
precision mediump sampler2D;
out mediump vec4 sk_FragColor;
uniform highp mat3 umatrix_S1_c0_c0;
uniform mediump vec2 uIncrement_S1_c0;
uniform mediump vec2 uOffsetsAndKernel_S1_c0[10];
uniform highp mat3 umatrix_S1;
uniform sampler2D uTextureSampler_0_S1;
in highp vec2 vTransformedCoords_2_S0;
void main() {
    mediump vec4 output_S1;
    mediump vec4 _5_color = vec4(0.0);
    highp vec2 _6_coord = vTransformedCoords_2_S0;
    for (highp int _7_i = 0;_7_i < 10; ++_7_i) {
        mediump vec2 _8_offsetAndKernel = uOffsetsAndKernel_S1_c0[_7_i];
        _5_color += vec4(0.0, 0.0, 0.0, texture(uTextureSampler_0_S1, mat3x2(umatrix_S1_c0_c0) * vec3(_6_coord + _8_offsetAndKernel.x * uIncrement_S1_c0, 1.0), -0.5).x) * _8_offsetAndKernel.y;
    }
    output_S1 = _5_color;
    {
        sk_FragColor = output_S1;
        sk_FragColor = vec4(sk_FragColor.w, 0.0, 0.0, 0.0);
    }
}
)"},
        },
    },
    {
        "5654438683513672207",
        {
            {GL_VERTEX_SHADER,
             R"(#version 300 es

precision mediump float;
precision mediump sampler2D;
uniform highp vec4 sk_RTAdjust;
in highp vec2 position;
in highp vec2 localCoord;
out highp vec2 vlocalCoord_S0;
void main() {
    vlocalCoord_S0 = localCoord;
    gl_Position = vec4(position, 0.0, 1.0);
    gl_Position = vec4(gl_Position.xy * sk_RTAdjust.xz + gl_Position.ww * sk_RTAdjust.yw, 0.0, gl_Position.w);
}
)"},
            {GL_FRAGMENT_SHADER,
             R"(#version 300 es

precision mediump float;
precision mediump sampler2D;
out mediump vec4 sk_FragColor;
uniform sampler2D uTextureSampler_0_S0;
in highp vec2 vlocalCoord_S0;
void main() {
    mediump vec4 outputColor_S0 = vec4(1.0);
    highp vec2 texCoord;
    texCoord = vlocalCoord_S0;
    outputColor_S0 = texture(uTextureSampler_0_S0, texCoord, -0.5);
    {
        sk_FragColor = outputColor_S0;
    }
}
)"},
        },
    },
    {
        "6097412585987124949",
        {
            {GL_VERTEX_SHADER,
             R"(#version 300 es

precision mediump float;
precision mediump sampler2D;
uniform highp vec4 sk_RTAdjust;
uniform highp mat3 umatrix_S1_c0;
in highp vec2 position;
in mediump vec4 color;
in highp vec2 localCoord;
out mediump vec4 vcolor_S0;
out highp vec2 vTransformedCoords_3_S0;
void main() {
    vcolor_S0 = color;
    gl_Position = vec4(position, 0.0, 1.0);
    {
        vTransformedCoords_3_S0 = mat3x2(umatrix_S1_c0) * vec3(localCoord, 1.0);
    }
    gl_Position = vec4(gl_Position.xy * sk_RTAdjust.xz + gl_Position.ww * sk_RTAdjust.yw, 0.0, gl_Position.w);
}
)"},
            {GL_FRAGMENT_SHADER,
             R"(#version 300 es

precision mediump float;
precision mediump sampler2D;
out mediump vec4 sk_FragColor;
uniform highp mat3 umatrix_S1_c0;
uniform sampler2D uTextureSampler_0_S1;
in mediump vec4 vcolor_S0;
in highp vec2 vTransformedCoords_3_S0;
void main() {
    mediump vec4 outputColor_S0;
    outputColor_S0 = vcolor_S0;
    mediump vec4 output_S1;
    mediump vec4 _0_input = outputColor_S0;
    _0_input = texture(uTextureSampler_0_S1, vTransformedCoords_3_S0, -0.5);
    output_S1 = _0_input;
    {
        sk_FragColor = output_S1;
    }
}
)"},
        },
    },
    {
        "6570254861767933680",
        {
            {GL_VERTEX_SHADER,
             R"(#version 300 es

precision mediump float;
precision mediump sampler2D;
uniform highp vec4 sk_RTAdjust;
in highp vec2 inPosition;
in mediump vec4 inColor;
in highp vec4 inQuadEdge;
out highp vec4 vQuadEdge_S0;
out mediump vec4 vinColor_S0;
void main() {
    vQuadEdge_S0 = inQuadEdge;
    vinColor_S0 = inColor;
    highp vec2 _tmp_0_inPosition = inPosition;
    gl_Position = vec4(_tmp_0_inPosition, 0.0, 1.0);
    gl_Position = vec4(gl_Position.xy * sk_RTAdjust.xz + gl_Position.ww * sk_RTAdjust.yw, 0.0, gl_Position.w);
}
)"},
            {GL_FRAGMENT_SHADER,
             R"(#version 300 es

uniform highp vec2 u_skRTFlip;
precision mediump float;
precision mediump sampler2D;
out mediump vec4 sk_FragColor;
in highp vec4 vQuadEdge_S0;
in mediump vec4 vinColor_S0;
void main() {
    mediump vec4 outputColor_S0;
    outputColor_S0 = vinColor_S0;
    mediump float edgeAlpha;
    mediump vec2 duvdx = dFdx(vQuadEdge_S0.xy);
    mediump vec2 duvdy = (u_skRTFlip.y * dFdy(vQuadEdge_S0.xy));
    if (vQuadEdge_S0.z > 0.0 ? vQuadEdge_S0.w > 0.0 : false) {
        edgeAlpha = min(min(vQuadEdge_S0.z, vQuadEdge_S0.w) + 0.5, 1.0);
    } else {
        mediump vec2 gF = vec2((2.0 * vQuadEdge_S0.x) * duvdx.x - duvdx.y, (2.0 * vQuadEdge_S0.x) * duvdy.x - duvdy.y);
        edgeAlpha = vQuadEdge_S0.x * vQuadEdge_S0.x - vQuadEdge_S0.y;
        edgeAlpha = clamp(0.5 - edgeAlpha / length(gF), 0.0, 1.0);
    }
    mediump vec4 outputCoverage_S0 = vec4(edgeAlpha);
    {
        sk_FragColor = outputColor_S0 * outputCoverage_S0;
    }
}
)"},
        },
    },
    {
        "6999673272543125767",
        {
            {GL_VERTEX_SHADER,
             R"(#version 300 es

precision mediump float;
precision mediump sampler2D;
uniform highp vec4 sk_RTAdjust;
uniform highp mat3 umatrix_S1;
in highp vec2 position;
in highp vec2 localCoord;
out highp vec2 vTransformedCoords_2_S0;
void main() {
    gl_Position = vec4(position, 0.0, 1.0);
    {
        vTransformedCoords_2_S0 = mat3x2(umatrix_S1) * vec3(localCoord, 1.0);
    }
    gl_Position = vec4(gl_Position.xy * sk_RTAdjust.xz + gl_Position.ww * sk_RTAdjust.yw, 0.0, gl_Position.w);
}
)"},
            {GL_FRAGMENT_SHADER,
             R"(#version 300 es

precision mediump float;
precision mediump sampler2D;
out mediump vec4 sk_FragColor;
uniform highp vec4 uclamp_S1_c0_c0_c0;
uniform highp mat3 umatrix_S1_c0_c0;
uniform mediump vec2 uIncrement_S1_c0;
uniform mediump vec2 uOffsetsAndKernel_S1_c0[10];
uniform highp mat3 umatrix_S1;
uniform sampler2D uTextureSampler_0_S1;
in highp vec2 vTransformedCoords_2_S0;
void main() {
    mediump vec4 output_S1;
    mediump vec4 _21_color = vec4(0.0);
    highp vec2 _22_coord = vTransformedCoords_2_S0;
    for (highp int _23_i = 0;_23_i < 10; ++_23_i) {
        mediump vec2 _24_offsetAndKernel = uOffsetsAndKernel_S1_c0[_23_i];
        highp vec2 _25_inCoord = mat3x2(umatrix_S1_c0_c0) * vec3(_22_coord + _24_offsetAndKernel.x * uIncrement_S1_c0, 1.0);
        highp vec2 _26_subsetCoord;
        _26_subsetCoord.x = _25_inCoord.x;
        _26_subsetCoord.y = _25_inCoord.y;
        highp vec2 _27_clampedCoord;
        _27_clampedCoord.x = clamp(_26_subsetCoord.x, uclamp_S1_c0_c0_c0.x, uclamp_S1_c0_c0_c0.z);
        _27_clampedCoord.y = _26_subsetCoord.y;
        mediump vec4 _28_textureColor = vec4(0.0, 0.0, 0.0, texture(uTextureSampler_0_S1, _27_clampedCoord, -0.5).x);
        _21_color += _28_textureColor * _24_offsetAndKernel.y;
    }
    output_S1 = _21_color;
    {
        sk_FragColor = output_S1;
        sk_FragColor = vec4(sk_FragColor.w, 0.0, 0.0, 0.0);
    }
}
)"},
        },
    },
    {
        "7558431755647152417",
        {
            {GL_VERTEX_SHADER,
             R"(#version 300 es

precision mediump float;
precision mediump sampler2D;
uniform highp vec4 sk_RTAdjust;
uniform highp mat3 umatrix_S1_c0;
in highp vec2 position;
in highp vec2 localCoord;
out highp vec2 vTransformedCoords_3_S0;
void main() {
    gl_Position = vec4(position, 0.0, 1.0);
    {
        vTransformedCoords_3_S0 = mat3x2(umatrix_S1_c0) * vec3(localCoord, 1.0);
    }
    gl_Position = vec4(gl_Position.xy * sk_RTAdjust.xz + gl_Position.ww * sk_RTAdjust.yw, 0.0, gl_Position.w);
}
)"},
            {GL_FRAGMENT_SHADER,
             R"(#version 300 es

precision mediump float;
precision mediump sampler2D;
out mediump vec4 sk_FragColor;
uniform highp mat3 umatrix_S1_c0;
uniform mediump float uSrcTF_S1[7];
uniform mediump mat3 uColorXform_S1;
uniform mediump float uDstTF_S1[7];
uniform sampler2D uTextureSampler_0_S1;
in highp vec2 vTransformedCoords_3_S0;
mediump float src_tf_S1_hh(mediump float x) {
    mediump float G = uSrcTF_S1[0];
    mediump float A = uSrcTF_S1[1];
    mediump float B = uSrcTF_S1[2];
    mediump float C = uSrcTF_S1[3];
    mediump float D = uSrcTF_S1[4];
    mediump float E = uSrcTF_S1[5];
    mediump float F = uSrcTF_S1[6];
    mediump float s = sign(x);
    x = abs(x);
    x = x < D ? C * x + F : pow(A * x + B, G) + E;
    return s * x;
}
mediump float dst_tf_S1_hh(mediump float x) {
    mediump float G = uDstTF_S1[0];
    mediump float A = uDstTF_S1[1];
    mediump float B = uDstTF_S1[2];
    mediump float C = uDstTF_S1[3];
    mediump float D = uDstTF_S1[4];
    mediump float E = uDstTF_S1[5];
    mediump float F = uDstTF_S1[6];
    mediump float s = sign(x);
    x = abs(x);
    x = x < D ? C * x + F : pow(A * x + B, G) + E;
    return s * x;
}
void main() {
    mediump vec4 output_S1;
    mediump vec4 _3_color = texture(uTextureSampler_0_S1, vTransformedCoords_3_S0, -0.5);
    _3_color = vec4(_3_color.xyz / max(_3_color.w, 0.0001), _3_color.w);
    _3_color.x = src_tf_S1_hh(_3_color.x);
    _3_color.y = src_tf_S1_hh(_3_color.y);
    _3_color.z = src_tf_S1_hh(_3_color.z);
    mediump vec4 _4_color = _3_color;
    _4_color.xyz = uColorXform_S1 * _4_color.xyz;
    _3_color = _4_color;
    _3_color.x = dst_tf_S1_hh(_3_color.x);
    _3_color.y = dst_tf_S1_hh(_3_color.y);
    _3_color.z = dst_tf_S1_hh(_3_color.z);
    _3_color.xyz *= _3_color.w;
    output_S1 = _3_color;
    {
        sk_FragColor = output_S1;
    }
}
)"},
        },
    },
    {
        "7895935357070818931",
        {
            {GL_VERTEX_SHADER,
             R"(#version 300 es

precision mediump float;
precision mediump sampler2D;
uniform highp vec4 sk_RTAdjust;
in highp vec2 position;
in highp float coverage;
in highp vec2 localCoord;
out highp vec2 vlocalCoord_S0;
out highp float vcoverage_S0;
void main() {
    highp vec2 position = position;
    vlocalCoord_S0 = localCoord;
    vcoverage_S0 = coverage;
    gl_Position = vec4(position, 0.0, 1.0);
    gl_Position = vec4(gl_Position.xy * sk_RTAdjust.xz + gl_Position.ww * sk_RTAdjust.yw, 0.0, gl_Position.w);
}
)"},
            {GL_FRAGMENT_SHADER,
             R"(#version 300 es

precision mediump float;
precision mediump sampler2D;
out mediump vec4 sk_FragColor;
uniform sampler2D uTextureSampler_0_S0;
in highp vec2 vlocalCoord_S0;
in highp float vcoverage_S0;
void main() {
    mediump vec4 outputColor_S0 = vec4(1.0);
    highp vec2 texCoord;
    texCoord = vlocalCoord_S0;
    outputColor_S0 = texture(uTextureSampler_0_S0, texCoord, -0.5);
    highp float coverage = vcoverage_S0;
    mediump vec4 outputCoverage_S0 = vec4(coverage);
    {
        sk_FragColor = outputColor_S0 * outputCoverage_S0;
    }
}
)"},
        },
    },
    {
        "9227781959051929065",
        {
            {GL_VERTEX_SHADER,
             R"(#version 300 es

precision mediump float;
precision mediump sampler2D;
uniform highp vec4 sk_RTAdjust;
uniform highp mat3 umatrix_S1_c0;
in highp vec2 position;
in mediump vec4 color;
in highp vec2 localCoord;
out mediump vec4 vcolor_S0;
out highp vec2 vTransformedCoords_3_S0;
void main() {
    vcolor_S0 = color;
    gl_Position = vec4(position, 0.0, 1.0);
    {
        vTransformedCoords_3_S0 = mat3x2(umatrix_S1_c0) * vec3(localCoord, 1.0);
    }
    gl_Position = vec4(gl_Position.xy * sk_RTAdjust.xz + gl_Position.ww * sk_RTAdjust.yw, 0.0, gl_Position.w);
}
)"},
            {GL_FRAGMENT_SHADER,
             R"(#version 300 es

precision mediump float;
precision mediump sampler2D;
out mediump vec4 sk_FragColor;
uniform highp vec4 uclamp_S1_c0_c0;
uniform highp mat3 umatrix_S1_c0;
uniform sampler2D uTextureSampler_0_S1;
in mediump vec4 vcolor_S0;
in highp vec2 vTransformedCoords_3_S0;
void main() {
    mediump vec4 outputColor_S0;
    outputColor_S0 = vcolor_S0;
    mediump vec4 output_S1;
    mediump vec4 _8_input = outputColor_S0;
    highp vec2 _9_inCoord = vTransformedCoords_3_S0;
    highp vec2 _10_subsetCoord;
    _10_subsetCoord.x = _9_inCoord.x;
    _10_subsetCoord.y = _9_inCoord.y;
    highp vec2 _11_clampedCoord;
    _11_clampedCoord = clamp(_10_subsetCoord, uclamp_S1_c0_c0.xy, uclamp_S1_c0_c0.zw);
    mediump vec4 _12_textureColor = texture(uTextureSampler_0_S1, _11_clampedCoord, -0.5);
    _8_input = _12_textureColor;
    output_S1 = _8_input;
    {
        sk_FragColor = output_S1;
    }
}
)"},
        },
    },
    {
        "9508166712714518371",
        {
            {GL_VERTEX_SHADER,
             R"(#version 300 es

precision mediump float;
precision mediump sampler2D;
uniform highp vec4 sk_RTAdjust;
in highp vec2 inPosition;
in mediump vec4 inColor;
out mediump vec4 vcolor_S0;
void main() {
    mediump vec4 color = inColor;
    vcolor_S0 = color;
    highp vec2 _tmp_1_inPosition = inPosition;
    gl_Position = vec4(_tmp_1_inPosition, 0.0, 1.0);
    gl_Position = vec4(gl_Position.xy * sk_RTAdjust.xz + gl_Position.ww * sk_RTAdjust.yw, 0.0, gl_Position.w);
}
)"},
            {GL_FRAGMENT_SHADER,
             R"(#version 300 es

precision mediump float;
precision mediump sampler2D;
out mediump vec4 sk_FragColor;
in mediump vec4 vcolor_S0;
void main() {
    mediump vec4 outputColor_S0;
    outputColor_S0 = vcolor_S0;
    {
        sk_FragColor = outputColor_S0;
    }
}
)"},
        },
    },
    {
        "9862848659140010561",
        {
            {GL_VERTEX_SHADER,
             R"(#version 300 es

precision mediump float;
precision mediump sampler2D;
uniform highp vec4 sk_RTAdjust;
in highp vec2 inPosition;
in mediump vec4 inColor;
in highp vec4 inCircleEdge;
out highp vec4 vinCircleEdge_S0;
out mediump vec4 vinColor_S0;
void main() {
    vinCircleEdge_S0 = inCircleEdge;
    vinColor_S0 = inColor;
    highp vec2 _tmp_0_inPosition = inPosition;
    gl_Position = vec4(_tmp_0_inPosition, 0.0, 1.0);
    gl_Position = vec4(gl_Position.xy * sk_RTAdjust.xz + gl_Position.ww * sk_RTAdjust.yw, 0.0, gl_Position.w);
}
)"},
            {GL_FRAGMENT_SHADER,
             R"(#version 300 es

precision mediump float;
precision mediump sampler2D;
out mediump vec4 sk_FragColor;
in highp vec4 vinCircleEdge_S0;
in mediump vec4 vinColor_S0;
void main() {
    highp vec4 circleEdge;
    circleEdge = vinCircleEdge_S0;
    mediump vec4 outputColor_S0;
    outputColor_S0 = vinColor_S0;
    highp float d = length(circleEdge.xy);
    mediump float distanceToOuterEdge = circleEdge.z * (1.0 - d);
    mediump float edgeAlpha = clamp(distanceToOuterEdge, 0.0, 1.0);
    mediump vec4 outputCoverage_S0 = vec4(edgeAlpha);
    {
        sk_FragColor = outputColor_S0 * outputCoverage_S0;
    }
}
)"},
        },
    },
};
