const char kGLES1DrawTexVShader[] = R"(#version 300 es
precision highp float;
layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 texcoord;
out vec2 texcoord_varying;
void main() {
    gl_Position = vec4(pos.x, pos.y, pos.z, 1.0);
    texcoord_varying = texcoord;
}
)";

const char kGLES1DrawTexFShader[] = R"(#version 300 es
precision highp float;
uniform sampler2D tex_sampler;
in vec2 texcoord_varying;
out vec4 frag_color;
void main() {
    frag_color = texture(tex_sampler, texcoord_varying);
}
)";

// version, flat,
const char kGLES1DrawVShader[] = R"(#version 300 es
precision highp float;

layout(location = 0) in vec4 pos;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec4 color;
layout(location = 3) in float pointsize;
layout(location = 4) in vec4 texcoord0;
layout(location = 5) in vec4 texcoord1;
layout(location = 6) in vec4 texcoord2;
layout(location = 7) in vec4 texcoord3;

uniform mat4 projection;
uniform mat4 modelview;
uniform mat4 modelview_invtr;
uniform mat4 texture_matrix;

uniform bool enable_rescale_normal;
uniform bool enable_normalize;

out vec4 pos_varying;
out vec3 normal_varying;
out vec4 color_varying;
flat out vec4 color_varying_flat;
out float pointsize_varying;
out vec4 texcoord0_varying;
out vec4 texcoord1_varying;
out vec4 texcoord2_varying;
out vec4 texcoord3_varying;

void main() {

    pos_varying = modelview * pos;
    mat3 mvInvTr3 = mat3(modelview_invtr);
    normal_varying = mvInvTr3 * normal;

    if (enable_rescale_normal) {
        float rescale = 1.0;
        vec3 rescaleVec = vec3(mvInvTr3[2]);
        float len = length(rescaleVec);
        if (len > 0.0) {
            rescale = 1.0 / len;
        }
        normal_varying *= rescale;
    }

    if (enable_normalize) {
        normal_varying = normalize(normal_varying);
    }

    color_varying = color;
    color_varying_flat = color;
    pointsize_varying = pointsize;
    texcoord0_varying = texture_matrix * texcoord0;
    texcoord1_varying = texture_matrix * texcoord1;
    texcoord2_varying = texture_matrix * texcoord2;
    texcoord3_varying = texture_matrix * texcoord3;

    gl_Position = projection * modelview * pos;
    gl_PointSize = 1.0;
}
)";

// version, flat,
const char kGLES1DrawFShader[] = R"(#version 300 es
precision highp float;
// Defines
#define kMaxLights                           8
#define kMaxTexUnits                         4

#define kModulate                       0x2100
#define kDecal                          0x2101
#define kCombine                        0x8570
#define kReplace                        0x1E01

#define kAlpha                          0x1906
#define kRGB                            0x1907
#define kRGBA                           0x1908
#define kLuminance                      0x1909
#define kLuminanceAlpha                 0x190A

#define kLinear                         0x2601
#define kExp                            0x0800
#define kExp2                           0x0801

#define kNever                          0x0200
#define kLess                           0x0201
#define kEqual                          0x0202
#define kLequal                         0x0203
#define kGreater                        0x0204
#define kNotequal                       0x0205
#define kGequal                         0x0206
#define kAlways                         0x0207
#define kZero                              0x0
#define kOne                               0x1

#define kClear                          0x1500
#define kAnd                            0x1501
#define kAnd_reverse                    0x1502
#define kCopy                           0x1503
#define kAnd_inverted                   0x1504
#define kNoop                           0x1505
#define kXor                            0x1506
#define kOr                             0x1507
#define kNor                            0x1508
#define kEquiv                          0x1509
#define kInvert                         0x150A
#define kOr_reverse                     0x150B
#define kCopy_inverted                  0x150C
#define kOr_inverted                    0x150D
#define kNand                           0x150E
#define kSet                            0x150F


precision highp float;
uniform sampler2D tex_sampler;
uniform samplerCube tex_cube_sampler;
uniform bool shade_model_flat;
uniform bool enable_texture_2d;
uniform bool enable_texture_cube_map;
uniform bool enable_lighting;
uniform bool enable_color_material;
uniform bool enable_fog;
uniform bool enable_reflection_map;
uniform bool enable_alpha_test;
uniform bool enable_logic_op;

uniform int alpha_func;
uniform float alpha_test_ref;

uniform int logic_op;

uniform int texture_format;
uniform int texture_env_mode;
uniform int combine_rgb;
uniform int combine_alpha;
uniform int src0_rgb;
uniform int src0_alpha;
uniform int src1_rgb;
uniform int src1_alpha;
uniform int src2_rgb;
uniform int src2_alpha;
uniform int op0_rgb;
uniform int op0_alpha;
uniform int op1_rgb;
uniform int op1_alpha;
uniform int op2_rgb;
uniform int op2_alpha;
uniform vec4 texture_env_color;
uniform float texture_env_rgb_scale;
uniform float texture_env_alpha_scale;
uniform bool texture_env_point_sprite_coord_replace;

// material (front+back)
uniform vec4 material_ambient;
uniform vec4 material_diffuse;
uniform vec4 material_specular;
uniform vec4 material_emissive;
uniform float material_specular_exponent;

// lights
uniform vec4 light_model_scene_ambient;
uniform bool light_model_two_sided;

uniform bool light_enables[kMaxLights];
uniform vec4 light_ambients[kMaxLights];
uniform vec4 light_diffuses[kMaxLights];
uniform vec4 light_speculars[kMaxLights];
uniform vec4 light_positions[kMaxLights];
uniform vec3 light_directions[kMaxLights];
uniform float light_spotlight_exponents[kMaxLights];
uniform float light_spotlight_cutoff_angles[kMaxLights];
uniform float light_attenuation_consts[kMaxLights];
uniform float light_attenuation_linears[kMaxLights];
uniform float light_attenuation_quadratics[kMaxLights];

// fog
uniform int fog_mode;
uniform float fog_density;
uniform float fog_start;
uniform float fog_end;
uniform vec4 fog_color;

in vec4 pos_varying;
in vec3 normal_varying;
in vec4 color_varying;
flat in vec4 color_varying_flat;
in float pointsize_varying;
in vec4 texcoord0_varying;
in vec4 texcoord1_varying;
in vec4 texcoord2_varying;
in vec4 texcoord3_varying;

out vec4 frag_color;

float posDot(vec3 a, vec3 b) {
    return max(dot(a, b), 0.0);
}

void main() {
    vec4 currentFragment;

    vec4 vertex_color;
    if (shade_model_flat) {
        vertex_color = color_varying_flat;
    } else {
        vertex_color = color_varying;
    }

    if (enable_texture_2d ||
        enable_texture_cube_map) {
        vec4 textureColor;
        if (enable_reflection_map) {
            textureColor = texture(tex_cube_sampler, reflect(pos_varying.xyz, normalize(normal_varying)));
            currentFragment = textureColor;
        } else {

            if (enable_texture_2d) {
                textureColor = texture(tex_sampler, texcoord0_varying.xy);
            } else {
                textureColor = texture(tex_cube_sampler, texcoord0_varying.xyz);
            }

            switch (texture_env_mode) {
            case kReplace:
                switch (texture_format) {
                case kAlpha:
                    currentFragment.rgb = vertex_color.rgb;
                    currentFragment.a = textureColor.a;
                    break;
                case kRGBA:
                case kLuminanceAlpha:
                    currentFragment.rgba = textureColor.rgba;
                    break;
                case kRGB:
                case kLuminance:
                default:
                    currentFragment.rgb = textureColor.rgb;
                    currentFragment.a = vertex_color.a;
                    break;
                }
                break;
            case kModulate:
                switch (texture_format) {
                case kAlpha:
                    currentFragment.rgb = vertex_color.rgb;
                    currentFragment.a = vertex_color.a * textureColor.a;
                    break;
                case kRGBA:
                case kLuminanceAlpha:
                    currentFragment.rgba = vertex_color.rgba * textureColor.rgba;
                    break;
                case kRGB:
                case kLuminance:
                default:
                    currentFragment.rgb = vertex_color.rgb * textureColor.rgb;
                    currentFragment.a = vertex_color.a;
                    break;
                }
            case kDecal:
                switch (texture_format) {
                case kRGB:
                    currentFragment.rgb = textureColor.rgb;
                    currentFragment.a = vertex_color.a;
                    break;
                case kRGBA:
                    currentFragment.rgb = vertex_color.rgb * (1.0 - textureColor.a) + textureColor.rgb * textureColor.a;
                    currentFragment.a = vertex_color.a;
                    break;
                case kAlpha:
                case kLuminance:
                case kLuminanceAlpha:
                default:
                    currentFragment.rgb = vertex_color.rgb * textureColor.rgb;
                    currentFragment.a = vertex_color.a;
                    break;
                }
            case kCombine:
            default:
                switch (texture_format) {
                case kAlpha:
                    currentFragment.rgb = vertex_color.rgb;
                    currentFragment.a = vertex_color.a * textureColor.a;
                    break;
                case kRGBA:
                case kLuminanceAlpha:
                    currentFragment.rgba = vertex_color.rgba * textureColor.rgba;
                    break;
                case kRGB:
                case kLuminance:
                default:
                    currentFragment.rgb = vertex_color.rgb * textureColor.rgb;
                    currentFragment.a = vertex_color.a;
                    break;
                }
                break;
            }
        }
    } else {
        currentFragment = vertex_color;
    }

    if (enable_lighting) {

    vec4 materialAmbientActual = material_ambient;
    vec4 materialDiffuseActual = material_diffuse;

    if (enable_color_material || enable_texture_2d || enable_texture_cube_map) {
        materialAmbientActual = currentFragment;
        materialDiffuseActual = currentFragment;
    }

    vec4 lit = material_emissive +
               materialAmbientActual * light_model_scene_ambient;

    for (int i = 0; i < kMaxLights; i++) {

        if (!light_enables[i]) continue;

        vec4 lightAmbient = light_ambients[i];
        vec4 lightDiffuse = light_diffuses[i];
        vec4 lightSpecular = light_speculars[i];
        vec4 lightPos = light_positions[i];
        vec3 lightDir = light_directions[i];
        float attConst = light_attenuation_consts[i];
        float attLinear = light_attenuation_linears[i];
        float attQuadratic = light_attenuation_quadratics[i];
        float spotAngle = light_spotlight_cutoff_angles[i];
        float spotExponent = light_spotlight_exponents[i];

        vec3 toLight;
        if (lightPos.w == 0.0) {
            toLight = lightPos.xyz;
        } else {
            toLight = (lightPos.xyz / lightPos.w - pos_varying.xyz);
        }

        float lightDist = length(toLight);
        vec3 h = normalize(toLight) + vec3(0.0, 0.0, 1.0);
        float ndotL = posDot(normal_varying, normalize(toLight));
        float ndoth = posDot(normal_varying, normalize(h));

        float specAtt;

        if (ndotL != 0.0) {
            specAtt = 1.0;
        } else {
            specAtt = 0.0;
        }

        float att;

        if (lightPos.w != 0.0) {
            float attDenom = (attConst + attLinear * lightDist +
                              attQuadratic * lightDist * lightDist);
            att = 1.0 / attDenom;
        } else {
            att = 1.0;
        }

        float spot;

        float spotAngleCos = cos(radians(spotAngle));
        vec3 toSurfaceDir = -normalize(toLight);
        float spotDot = posDot(toSurfaceDir, normalize(lightDir));

        if (spotAngle == 180.0 || lightPos.w == 0.0) {
            spot = 1.0;
        } else {
            if (spotDot < spotAngleCos) {
                spot = 0.0;
            } else {
                spot = pow(spotDot, spotExponent);
            }
        }

        vec4 contrib = materialAmbientActual * lightAmbient;
        contrib += ndotL * materialDiffuseActual * lightDiffuse;
        if (ndoth > 0.0 && material_specular_exponent > 0.0) {
            contrib += specAtt * pow(ndoth, material_specular_exponent) *
                                 material_specular * lightSpecular;
        } else {
            if (ndoth > 0.0) {
                contrib += specAtt * material_specular * lightSpecular;
            }
        }
        contrib *= att * spot;
        lit += contrib;
    }

    currentFragment = lit;

    }

    if (enable_fog) {

    float eyeDist = -pos_varying.z / pos_varying.w;
    float f = 1.0;
    switch (fog_mode) {
        case kExp:
            f = exp(-fog_density * eyeDist);
            break;
        case kExp2:
            f = exp(-(pow(fog_density * eyeDist, 2.0)));
            break;
        case kLinear:
            f = (fog_end - eyeDist) / (fog_end - fog_start);
            break;
        default:
            break;
    }

    currentFragment = f * currentFragment + (1.0 - f) * fog_color;

    }

    bool shouldPass = true;

    if (enable_alpha_test) {
        bool shouldPassAlpha = false;
        float incAlpha = currentFragment.a;
        switch (alpha_func) {
            case kNever:
                shouldPassAlpha = false;
                break;
            case kLess:
                shouldPassAlpha = incAlpha < alpha_test_ref;
                break;
            case kLequal:
                shouldPassAlpha = incAlpha <= alpha_test_ref;
                break;
            case kEqual:
                shouldPassAlpha = incAlpha == alpha_test_ref;
                break;
            case kGequal:
                shouldPassAlpha = incAlpha >= alpha_test_ref;
                break;
            case kGreater:
                shouldPassAlpha = incAlpha > alpha_test_ref;
                break;
            case kNotequal:
                shouldPassAlpha = incAlpha != alpha_test_ref;
                break;
            case kAlways:
            default:
                shouldPassAlpha = true;
                break;
        }

        shouldPass = shouldPass && shouldPassAlpha;
    }

    if (shouldPass) {
        if (enable_logic_op) {

            // TODO: figure out the best way to read
            // current attachment color
            vec4 read_color = vec4(1.0, 1.0, 1.0, 1.0);

            highp uint currentRG = packUnorm2x16(currentFragment.rg);
            highp uint currentBA = packUnorm2x16(currentFragment.ba);

            highp uint prevRG = packUnorm2x16(read_color.rg);
            highp uint prevBA = packUnorm2x16(read_color.ba);

            switch (logic_op) {
                case kClear:
                    currentRG = uint(0);
                    currentBA = uint(0);
                    break;
                case kAnd:
                    currentRG = currentRG & prevRG;
                    currentBA = currentBA & prevBA;
                    break;
                case kAnd_reverse:
                    currentRG = currentRG & (~prevRG);
                    currentBA = currentBA & (~prevBA);
                    break;
                case kCopy:
                    currentRG = currentRG;
                    currentBA = currentBA;
                    break;
                case kAnd_inverted:
                    currentRG = (~currentRG);
                    currentBA = (~currentBA);
                    break;
                case kNoop:
                    currentRG = prevRG;
                    currentBA = prevBA;
                    break;
                case kXor:
                    currentRG = currentRG ^ prevRG;
                    currentBA = currentBA ^ prevBA;
                    break;
                case kOr:
                    currentRG = currentRG | prevRG;
                    currentBA = currentBA | prevBA;
                    break;
                case kNor:
                    currentRG = ~(currentRG | prevRG);
                    currentBA = ~(currentBA | prevBA);
                    break;
                case kEquiv:
                    currentRG = ~(currentRG ^ prevRG);
                    currentBA = ~(currentBA ^ prevBA);
                    break;
                case kInvert:
                    currentRG = ~prevRG;
                    currentBA = ~prevBA;
                    break;
                case kOr_reverse:
                    currentRG = currentRG | (~prevRG);
                    currentBA = currentBA | (~prevBA);
                    break;
                case kCopy_inverted:
                    currentRG = ~currentRG;
                    currentBA = ~currentBA;
                    break;
                case kOr_inverted:
                    currentRG = (~currentRG) | prevRG;
                    currentBA = (~currentBA) | prevBA;
                    break;
                case kNand:
                    currentRG = ~(currentRG & prevRG);
                    currentBA = ~(currentBA & prevBA);
                    break;
                case kSet:
                    currentRG = uint(0xffffffff);
                    currentBA = uint(0xffffffff);
                    break;
                default:
                    break;
            }

            currentFragment = vec4(unpackUnorm2x16(currentRG),
                                   unpackUnorm2x16(currentBA));
        }

        frag_color = currentFragment;
    } else {
        discard;
    }
}
)";
