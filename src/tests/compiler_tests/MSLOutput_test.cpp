//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// MSLOutput_test.cpp:
//   Tests for MSL output.
//

#include <regex>
#include "GLSLANG/ShaderLang.h"
#include "angle_gl.h"
#include "gtest/gtest.h"
#include "tests/test_utils/compiler_test.h"

using namespace sh;

class MSLVertexOutputTest : public MatchOutputCodeTest
{
  public:
    MSLVertexOutputTest() : MatchOutputCodeTest(GL_VERTEX_SHADER, 0, SH_MSL_METAL_OUTPUT) {}
};

class MSLOutputTest : public MatchOutputCodeTest
{
  public:
    MSLOutputTest() : MatchOutputCodeTest(GL_FRAGMENT_SHADER, 0, SH_MSL_METAL_OUTPUT) {}
};

// Test that having dynamic indexing of a vector inside the right hand side of logical or doesn't
// trigger asserts in MSL output.
TEST_F(MSLOutputTest, DynamicIndexingOfVectorOnRightSideOfLogicalOr)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision highp float;\n"
        "out vec4 my_FragColor;\n"
        "uniform int u1;\n"
        "void main() {\n"
        "   bvec4 v = bvec4(true, true, true, false);\n"
        "   my_FragColor = vec4(v[u1 + 1] || v[u1]);\n"
        "}\n";
    compile(shaderString, SH_VARIABLES);
}

// Test that having an array constructor as a statement doesn't trigger an assert in MSL output.
TEST_F(MSLOutputTest, ArrayConstructorStatement)
{
    const std::string &shaderString =
        R"(#version 300 es
        precision mediump float;
        out vec4 outColor;
        void main()
        {
            outColor = vec4(0.0, 0.0, 0.0, 1.0);
            float[1](outColor[1]++);
        })";
    compile(shaderString, SH_VARIABLES);
}

// Test an array of arrays constructor as a statement.
TEST_F(MSLOutputTest, ArrayOfArraysStatement)
{
    const std::string &shaderString =
        R"(#version 310 es
        precision mediump float;
        out vec4 outColor;
        void main()
        {
            outColor = vec4(0.0, 0.0, 0.0, 1.0);
            float[2][2](float[2](outColor[1]++, 0.0), float[2](1.0, 2.0));
        })";
    compile(shaderString, SH_VARIABLES);
}

// Test dynamic indexing of a vector. This makes sure that helper functions added for dynamic
// indexing have correct data that subsequent traversal steps rely on.
TEST_F(MSLOutputTest, VectorDynamicIndexing)
{
    const std::string &shaderString =
        R"(#version 300 es
        precision mediump float;
        out vec4 outColor;
        uniform int i;
        void main()
        {
            vec4 foo = vec4(0.0, 0.0, 0.0, 1.0);
            foo[i] = foo[i + 1];
            outColor = foo;
        })";
    compile(shaderString, SH_VARIABLES);
}

// Test returning an array from a user-defined function. This makes sure that function symbols are
// changed consistently when the user-defined function is changed to have an array out parameter.
TEST_F(MSLOutputTest, ArrayReturnValue)
{
    const std::string &shaderString =
        R"(#version 300 es
        precision mediump float;
        uniform float u;
        out vec4 outColor;

        float[2] getArray(float f)
        {
            return float[2](f, f + 1.0);
        }

        void main()
        {
            float[2] arr = getArray(u);
            outColor = vec4(arr[0], arr[1], 0.0, 1.0);
        })";
    compile(shaderString, SH_VARIABLES);
}

// Test that writing parameters without a name doesn't assert.
TEST_F(MSLOutputTest, ParameterWithNoName)
{
    const std::string &shaderString =
        R"(precision mediump float;

        uniform vec4 v;

        vec4 s(vec4)
        {
            return v;
        }
        void main()
        {
            gl_FragColor = s(v);
        })";
    compile(shaderString, SH_VARIABLES);
}

TEST_F(MSLOutputTest, Macro)
{
    const std::string &shaderString =
        R"(#version 300 es
        precision highp float;

        #define FOO vec4

        out vec4 outColor;

        void main()
        {
            outColor = FOO(1.0, 2.0, 3.0, 4.0);
        })";
    compile(shaderString, SH_VARIABLES);
}

TEST_F(MSLOutputTest, UniformSimple)
{
    const std::string &shaderString =
        R"(#version 300 es
        precision highp float;

        out vec4 outColor;
        uniform float x;

        void main()
        {
            outColor = vec4(x, x, x, x);
        })";
    compile(shaderString, SH_VARIABLES);
}

TEST_F(MSLOutputTest, FragmentOutSimple)
{
    const std::string &shaderString =
        R"(#version 300 es
        precision highp float;

        out vec4 outColor;

        void main()
        {
            outColor = vec4(1.0, 2.0, 3.0, 4.0);
        })";
    compile(shaderString, SH_VARIABLES);
}

TEST_F(MSLOutputTest, FragmentOutIndirect1)
{
    const std::string &shaderString =
        R"(#version 300 es
        precision highp float;

        out vec4 outColor;

        void foo()
        {
            outColor = vec4(1.0, 2.0, 3.0, 4.0);
        }

        void bar()
        {
            foo();
        }

        void main()
        {
            bar();
        })";
    compile(shaderString, SH_VARIABLES);
}

TEST_F(MSLOutputTest, FragmentOutIndirect2)
{
    const std::string &shaderString =
        R"(#version 300 es
        precision highp float;

        out vec4 outColor;

        void foo();

        void bar()
        {
            foo();
        }

        void foo()
        {
            outColor = vec4(1.0, 2.0, 3.0, 4.0);
        }

        void main()
        {
            bar();
        })";
    compile(shaderString, SH_VARIABLES);
}

TEST_F(MSLOutputTest, FragmentOutIndirect3)
{
    const std::string &shaderString =
        R"(#version 300 es
        precision highp float;

        out vec4 outColor;

        float foo(float x, float y)
        {
            outColor = vec4(x, y, 3.0, 4.0);
            return 7.0;
        }

        float bar(float x)
        {
            return foo(x, 2.0);
        }

        float baz()
        {
            return 13.0;
        }

        float identity(float x)
        {
            return x;
        }

        void main()
        {
            identity(bar(baz()));
        })";
    compile(shaderString, SH_VARIABLES);
}

TEST_F(MSLOutputTest, VertexInOut)
{
    const std::string &shaderString =
        R"(#version 300 es
        precision highp float;
        in float in0;
        out float out0;
        void main()
        {
            out0 = in0;
        })";
    compile(shaderString, SH_VARIABLES);
}

TEST_F(MSLOutputTest, SymbolSharing)
{
    const std::string &shaderString =
        R"(#version 300 es
        precision highp float;

        out vec4 outColor;

        struct Foo {
            float x;
            float y;
        };

        void doFoo(Foo foo, float zw);

        void doFoo(Foo foo, float zw)
        {
            foo.x = foo.y;
            outColor = vec4(foo.x, foo.y, zw, zw);
        }

        void main()
        {
            Foo foo;
            foo.x = 2.0;
            foo.y = 2.0;
            doFoo(foo, 3.0);
        })";
    compile(shaderString, SH_VARIABLES);
}

TEST_F(MSLOutputTest, StructDecl)
{
    const std::string &shaderString =
        R"(#version 300 es
        precision highp float;

        out float out0;

        struct Foo {
            float value;
        };

        void main()
        {
            Foo foo;
            out0 = foo.value;
        }
        )";
    compile(shaderString, SH_VARIABLES);
}

TEST_F(MSLOutputTest, Structs)
{
    const std::string &shaderString =
        R"(#version 300 es
        precision highp float;

        struct Foo {
            float value;
        };

        out vec4 out0;

        struct Bar {
            Foo foo;
        };

        void go();

        uniform UniInstance {
            Bar bar;
            float instance;
        } uniInstance;

        uniform UniGlobal {
            Foo foo;
            float global;
        };

        void main()
        {
            go();
        }

        struct Baz {
            Bar bar;
        } baz;

        void go()
        {
            out0.x = baz.bar.foo.value;
            out0.y = global;
            out0.z = uniInstance.instance;
            out0.w = 0.0;
        }

        )";
    compile(shaderString, SH_VARIABLES);
}

TEST_F(MSLOutputTest, KeywordConflict)
{
    const std::string &shaderString =
        R"(#version 300 es
            precision highp float;

        struct fragment {
            float kernel;
        } device;

        struct Foo {
            fragment frag;
        } foo;

        out float vertex;
        float kernel;

        float stage_in(float x)
        {
            return x;
        }

        void metal(float metal, float fragment);
        void metal(float metal, float fragment)
        {
            vertex = metal * fragment * foo.frag.kernel;
        }

        void main()
        {
            metal(stage_in(stage_in(kernel * device.kernel)), foo.frag.kernel);
        })";
    compile(shaderString, SH_VARIABLES);
}

TEST_F(MSLVertexOutputTest, Vertex)
{
    const std::string &shaderString =
        R"(#version 300 es
        precision highp float;
        void main()
        {
            gl_Position = vec4(1.0,1.0,1.0,1.0);
        })";
    compile(shaderString, SH_VARIABLES);
}

TEST_F(MSLVertexOutputTest, LastReturn)
{
    const std::string &shaderString =
        R"(#version 300 es
        in highp vec4 a_position;
        in highp vec4 a_coords;
        out highp vec4 v_color;

        void main (void)
        {
            gl_Position = a_position;
            v_color = vec4(a_coords.xyz, 1.0);
            return;
        })";
    compile(shaderString, SH_VARIABLES);
}

TEST_F(MSLOutputTest, LastReturn)
{
    const std::string &shaderString =
        R"(#version 300 es
        in mediump vec4 v_coords;
        layout(location = 0) out mediump vec4 o_color;

        void main (void)
        {
            o_color = vec4(v_coords.xyz, 1.0);
            return;
        })";
    compile(shaderString, SH_VARIABLES);
}

TEST_F(MSLOutputTest, FragColor)
{
    const std::string &shaderString = R"(
        void main ()
        {
            gl_FragColor = vec4(1.0, 0.0, 1.0, 1.0);
        })";
    compile(shaderString, SH_VARIABLES);
}

TEST_F(MSLOutputTest, MatrixIn)
{
    const std::string &shaderString =
        R"(#version 300 es
        precision highp float;

        in mat4 mat;
        out float out0;

        void main()
        {
            out0 = mat[0][0];
        }
        )";
    compile(shaderString, SH_VARIABLES);
}

TEST_F(MSLOutputTest, WhileTrue)
{
    const std::string &shaderString =
        R"(#version 300 es
        precision mediump float;

        uniform float uf;
        out vec4 my_FragColor;

        void main()
        {
            my_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
            while (true)
            {
                break;
            }
        })";
    compile(shaderString, SH_VARIABLES);
}

TEST_F(MSLOutputTest, ForTrue)
{
    const std::string &shaderString =
        R"(#version 300 es
        precision mediump float;

        uniform float uf;
        out vec4 my_FragColor;

        void main()
        {
            my_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
            for (;true;)
            {
                break;
            }
        })";
    compile(shaderString, SH_VARIABLES);
}

TEST_F(MSLOutputTest, ForEmpty)
{
    const std::string &shaderString =
        R"(#version 300 es
        precision mediump float;

        uniform float uf;
        out vec4 my_FragColor;

        void main()
        {
            my_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
            for (;;)
            {
                break;
            }
        })";
    compile(shaderString, SH_VARIABLES);
}

TEST_F(MSLOutputTest, ForComplex)
{
    const std::string &shaderString =
        R"(#version 300 es
        precision mediump float;

        uniform float uf;
        out vec4 my_FragColor;

        void main()
        {
            my_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
            for (int i = 0, j = 2; i < j; ++i) {
                if (i == 0) continue;
                if (i == 42) break;
                my_FragColor.x += float(i);
            }
        })";
    compile(shaderString, SH_VARIABLES);
}

TEST_F(MSLOutputTest, ForSymbol)
{
    const std::string &shaderString =
        R"(#version 300 es
        precision mediump float;

        uniform float uf;
        out vec4 my_FragColor;

        void main()
        {
            bool cond = true;
            for (;cond;)
            {
                my_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
                cond = false;
            }
        })";
    compile(shaderString, SH_VARIABLES);
}

TEST_F(MSLOutputTest, DoWhileSymbol)
{
    const std::string &shaderString =
        R"(#version 300 es
        precision mediump float;

        uniform float uf;
        out vec4 my_FragColor;

        void main()
        {
            bool cond = false;
            do
            {
                my_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
            } while (cond);
        })";
    compile(shaderString, SH_VARIABLES);
}

TEST_F(MSLOutputTest, AnonymousStruct)
{
    const std::string &shaderString =
        R"(
        precision mediump float;
        struct { vec4 v; } anonStruct;
        void main() {
            anonStruct.v = vec4(0.0,1.0,0.0,1.0);
            gl_FragColor = anonStruct.v;
        })";
    compile(shaderString, SH_VARIABLES);
    // TODO(anglebug.com/6395): This success condition is expected to fail now.
    // When WebKit build is able to run the tests, this should be changed to something else.
    //    ASSERT_TRUE(foundInCode(SH_MSL_METAL_OUTPUT, "__unnamed"));
}

TEST_F(MSLOutputTest, CrashesWhenCompiled)
{
    // Source:
    // https://gkjohnson.github.io/three-gpu-pathtracer/example/bundle/materialBall.html#transmission
    // Issue: A while loop's expression, and a branch condition with eOpContinue were being deep
    // copied as part of
    // monomorphize functions, causing a crash, as they were not null-checked.
    const std::string &shaderString =
        R"(#version 300 es
#define varying in
out highp vec4 pc_fragColor;
#define gl_FragColor pc_fragColor
#define gl_FragDepthEXT gl_FragDepth
#define texture2D texture
#define textureCube texture
#define texture2DProj textureProj
#define texture2DLodEXT textureLod
#define texture2DProjLodEXT textureProjLod
#define textureCubeLodEXT textureLod
#define texture2DGradEXT textureGrad
#define texture2DProjGradEXT textureProjGrad
#define textureCubeGradEXT textureGrad
precision highp float;
precision highp int;
#define HIGH_PRECISION
#define SHADER_NAME ShaderMaterial
#define BOUNCES 5
#define GAMMA_FACTOR 2
uniform mat4 viewMatrix;
uniform vec3 cameraPosition;
uniform bool isOrthographic;

vec4 LinearToLinear( in vec4 value ) {
    return value;
}
vec4 GammaToLinear( in vec4 value, in float gammaFactor ) {
    return vec4( pow( value.rgb, vec3( gammaFactor ) ), value.a );
}
vec4 LinearToGamma( in vec4 value, in float gammaFactor ) {
    return vec4( pow( value.rgb, vec3( 1.0 / gammaFactor ) ), value.a );
}
vec4 sRGBToLinear( in vec4 value ) {
    return vec4( mix( pow( value.rgb * 0.9478672986 + vec3( 0.0521327014 ), vec3( 2.4 ) ), value.rgb * 0.0773993808, vec3( lessThanEqual( value.rgb, vec3( 0.04045 ) ) ) ), value.a );
}
vec4 LinearTosRGB( in vec4 value ) {
    return vec4( mix( pow( value.rgb, vec3( 0.41666 ) ) * 1.055 - vec3( 0.055 ), value.rgb * 12.92, vec3( lessThanEqual( value.rgb, vec3( 0.0031308 ) ) ) ), value.a );
}
vec4 RGBEToLinear( in vec4 value ) {
    return vec4( value.rgb * exp2( value.a * 255.0 - 128.0 ), 1.0 );
}
vec4 LinearToRGBE( in vec4 value ) {
    float maxComponent = max( max( value.r, value.g ), value.b );
    float fExp = clamp( ceil( log2( maxComponent ) ), -128.0, 127.0 );
    return vec4( value.rgb / exp2( fExp ), ( fExp + 128.0 ) / 255.0 );
}
vec4 RGBMToLinear( in vec4 value, in float maxRange ) {
    return vec4( value.rgb * value.a * maxRange, 1.0 );
}
vec4 LinearToRGBM( in vec4 value, in float maxRange ) {
    float maxRGB = max( value.r, max( value.g, value.b ) );
    float M = clamp( maxRGB / maxRange, 0.0, 1.0 );
    M = ceil( M * 255.0 ) / 255.0;
    return vec4( value.rgb / ( M * maxRange ), M );
}
vec4 RGBDToLinear( in vec4 value, in float maxRange ) {
    return vec4( value.rgb * ( ( maxRange / 255.0 ) / value.a ), 1.0 );
}
vec4 LinearToRGBD( in vec4 value, in float maxRange ) {
    float maxRGB = max( value.r, max( value.g, value.b ) );
    float D = max( maxRange / maxRGB, 1.0 );
    D = clamp( floor( D ) / 255.0, 0.0, 1.0 );
    return vec4( value.rgb * ( D * ( 255.0 / maxRange ) ), D );
}
const mat3 cLogLuvM = mat3( 0.2209, 0.3390, 0.4184, 0.1138, 0.6780, 0.7319, 0.0102, 0.1130, 0.2969 );
vec4 LinearToLogLuv( in vec4 value ) {
    vec3 Xp_Y_XYZp = cLogLuvM * value.rgb;
    Xp_Y_XYZp = max( Xp_Y_XYZp, vec3( 1e-6, 1e-6, 1e-6 ) );
    vec4 vResult;
    vResult.xy = Xp_Y_XYZp.xy / Xp_Y_XYZp.z;
    float Le = 2.0 * log2(Xp_Y_XYZp.y) + 127.0;
    vResult.w = fract( Le );
    vResult.z = ( Le - ( floor( vResult.w * 255.0 ) ) / 255.0 ) / 255.0;
    return vResult;
}
const mat3 cLogLuvInverseM = mat3( 6.0014, -2.7008, -1.7996, -1.3320, 3.1029, -5.7721, 0.3008, -1.0882, 5.6268 );
vec4 LogLuvToLinear( in vec4 value ) {
    float Le = value.z * 255.0 + value.w;
    vec3 Xp_Y_XYZp;
    Xp_Y_XYZp.y = exp2( ( Le - 127.0 ) / 2.0 );
    Xp_Y_XYZp.z = Xp_Y_XYZp.y / value.y;
    Xp_Y_XYZp.x = value.x * Xp_Y_XYZp.z;
    vec3 vRGB = cLogLuvInverseM * Xp_Y_XYZp.rgb;
    return vec4( max( vRGB, 0.0 ), 1.0 );
}
vec4 linearToOutputTexel( vec4 value ) { return LinearToLinear( value ); }


            #define RAY_OFFSET 1e-5

            precision highp isampler2D;
            precision highp usampler2D;
            
#ifndef TRI_INTERSECT_EPSILON
#define TRI_INTERSECT_EPSILON 1e-5
#endif

#ifndef INFINITY
#define INFINITY 1e20
#endif

struct BVH {

    usampler2D index;
    sampler2D position;

    sampler2D bvhBounds;
    usampler2D bvhContents;

};

// Note that a struct cannot be used for the hit record including faceIndices, faceNormal, barycoord,
// side, and dist because on some mobile GPUS (such as Adreno) numbers are afforded less precision specifically
// when in a struct leading to inaccurate hit results. See KhronosGroup/WebGL#3351 for more details.

            

uvec4 uTexelFetch1D( usampler2D tex, uint index ) {

    uint width = uint( textureSize( tex, 0 ).x );
    uvec2 uv;
    uv.x = index % width;
    uv.y = index / width;

    return texelFetch( tex, ivec2( uv ), 0 );

}

ivec4 iTexelFetch1D( isampler2D tex, uint index ) {

    uint width = uint( textureSize( tex, 0 ).x );
    uvec2 uv;
    uv.x = index % width;
    uv.y = index / width;

    return texelFetch( tex, ivec2( uv ), 0 );

}

vec4 texelFetch1D( sampler2D tex, uint index ) {

    uint width = uint( textureSize( tex, 0 ).x );
    uvec2 uv;
    uv.x = index % width;
    uv.y = index / width;

    return texelFetch( tex, ivec2( uv ), 0 );

}

vec4 textureSampleBarycoord( sampler2D tex, vec3 barycoord, uvec3 faceIndices ) {

    return
        barycoord.x * texelFetch1D( tex, faceIndices.x ) +
        barycoord.y * texelFetch1D( tex, faceIndices.y ) +
        barycoord.z * texelFetch1D( tex, faceIndices.z );

}

void ndcToCameraRay(
    vec2 coord, mat4 cameraWorld, mat4 invProjectionMatrix,
    out vec3 rayOrigin, out vec3 rayDirection
) {

    // get camera look direction and near plane for camera clipping
    vec4 lookDirection = cameraWorld * vec4( 0.0, 0.0, - 1.0, 0.0 );
    vec4 nearVector = invProjectionMatrix * vec4( 0.0, 0.0, - 1.0, 1.0 );
    float near = abs( nearVector.z / nearVector.w );

    // get the camera direction and position from camera matrices
    vec4 origin = cameraWorld * vec4( 0.0, 0.0, 0.0, 1.0 );
    vec4 direction = invProjectionMatrix * vec4( coord, 0.5, 1.0 );
    direction /= direction.w;
    direction = cameraWorld * direction - origin;

    // slide the origin along the ray until it sits at the near clip plane position
    origin.xyz += direction.xyz * near / dot( direction, lookDirection );

    rayOrigin = origin.xyz;
    rayDirection = direction.xyz;

}

float intersectsBounds( vec3 rayOrigin, vec3 rayDirection, vec3 boundsMin, vec3 boundsMax ) {

    // https://www.reddit.com/r/opengl/comments/8ntzz5/fast_glsl_ray_box_intersection/
    // https://tavianator.com/2011/ray_box.html
    vec3 invDir = 1.0 / rayDirection;

    // find intersection distances for each plane
    vec3 tMinPlane = invDir * ( boundsMin - rayOrigin );
    vec3 tMaxPlane = invDir * ( boundsMax - rayOrigin );

    // get the min and max distances from each intersection
    vec3 tMinHit = min( tMaxPlane, tMinPlane );
    vec3 tMaxHit = max( tMaxPlane, tMinPlane );

    // get the furthest hit distance
    vec2 t = max( tMinHit.xx, tMinHit.yz );
    float t0 = max( t.x, t.y );

    // get the minimum hit distance
    t = min( tMaxHit.xx, tMaxHit.yz );
    float t1 = min( t.x, t.y );

    // set distance to 0.0 if the ray starts inside the box
    float dist = max( t0, 0.0 );

    return t1 >= dist ? dist : INFINITY;

}

bool intersectsTriangle(
    vec3 rayOrigin, vec3 rayDirection, vec3 a, vec3 b, vec3 c,
    out vec3 barycoord, out vec3 norm, out float dist, out float side
) {

    // https://stackoverflow.com/questions/42740765/intersection-between-line-and-triangle-in-3d
    vec3 edge1 = b - a;
    vec3 edge2 = c - a;
    norm = cross( edge1, edge2 );

    float det = - dot( rayDirection, norm );
    float invdet = 1.0 / det;

    vec3 AO = rayOrigin - a;
    vec3 DAO = cross( AO, rayDirection );

    vec4 uvt;
    uvt.x = dot( edge2, DAO ) * invdet;
    uvt.y = - dot( edge1, DAO ) * invdet;
    uvt.z = dot( AO, norm ) * invdet;
    uvt.w = 1.0 - uvt.x - uvt.y;

    // set the hit information
    barycoord = uvt.wxy; // arranged in A, B, C order
    dist = uvt.z;
    side = sign( det );
    norm = side * normalize( norm );

    // add an epsilon to avoid misses between triangles
    uvt += vec4( TRI_INTERSECT_EPSILON );

    return all( greaterThanEqual( uvt, vec4( 0.0 ) ) );

}

bool intersectTriangles(
    BVH bvh, vec3 rayOrigin, vec3 rayDirection, uint offset, uint count,
    inout float minDistance,

    // output variables
    out uvec4 faceIndices, out vec3 faceNormal, out vec3 barycoord,
    out float side, out float dist
) {

    bool found = false;
    vec3 localBarycoord, localNormal;
    float localDist, localSide;
    for ( uint i = offset, l = offset + count; i < l; i ++ ) {

        uvec3 indices = uTexelFetch1D( bvh.index, i ).xyz;
        vec3 a = texelFetch1D( bvh.position, indices.x ).rgb;
        vec3 b = texelFetch1D( bvh.position, indices.y ).rgb;
        vec3 c = texelFetch1D( bvh.position, indices.z ).rgb;

        if (
            intersectsTriangle( rayOrigin, rayDirection, a, b, c, localBarycoord, localNormal, localDist, localSide )
            && localDist < minDistance
        ) {

            found = true;
            minDistance = localDist;

            faceIndices = uvec4( indices.xyz, i );
            faceNormal = localNormal;

            side = localSide;
            barycoord = localBarycoord;
            dist = localDist;

        }

    }

    return found;

}

float intersectsBVHNodeBounds( vec3 rayOrigin, vec3 rayDirection, BVH bvh, uint currNodeIndex ) {

    vec3 boundsMin = texelFetch1D( bvh.bvhBounds, currNodeIndex * 2u + 0u ).xyz;
    vec3 boundsMax = texelFetch1D( bvh.bvhBounds, currNodeIndex * 2u + 1u ).xyz;
    return intersectsBounds( rayOrigin, rayDirection, boundsMin, boundsMax );

}

bool bvhIntersectFirstHit(
    BVH bvh, vec3 rayOrigin, vec3 rayDirection,

    // output variables
    out uvec4 faceIndices, out vec3 faceNormal, out vec3 barycoord,
    out float side, out float dist
) {

    // stack needs to be twice as long as the deepest tree we expect because
    // we push both the left and right child onto the stack every traversal
    int ptr = 0;
    uint stack[ 60 ];
    stack[ 0 ] = 0u;

    float triangleDistance = 1e20;
    bool found = false;
    while ( ptr > - 1 && ptr < 60 ) {

        uint currNodeIndex = stack[ ptr ];
        ptr --;

        // check if we intersect the current bounds
        float boundsHitDistance = intersectsBVHNodeBounds( rayOrigin, rayDirection, bvh, currNodeIndex );
        if ( boundsHitDistance == INFINITY || boundsHitDistance > triangleDistance ) {

            continue;

        }

        uvec2 boundsInfo = uTexelFetch1D( bvh.bvhContents, currNodeIndex ).xy;
        bool isLeaf = bool( boundsInfo.x & 0xffff0000u );

        if ( isLeaf ) {

            uint count = boundsInfo.x & 0x0000ffffu;
            uint offset = boundsInfo.y;

            found = intersectTriangles(
                bvh, rayOrigin, rayDirection, offset, count, triangleDistance,
                faceIndices, faceNormal, barycoord, side, dist
            ) || found;

        } else {

            uint leftIndex = currNodeIndex + 1u;
            uint splitAxis = boundsInfo.x & 0x0000ffffu;
            uint rightIndex = boundsInfo.y;

            bool leftToRight = rayDirection[ splitAxis ] >= 0.0;
            uint c1 = leftToRight ? leftIndex : rightIndex;
            uint c2 = leftToRight ? rightIndex : leftIndex;

            // set c2 in the stack so we traverse it later. We need to keep track of a pointer in
            // the stack while we traverse. The second pointer added is the one that will be
            // traversed first
            ptr ++;
            stack[ ptr ] = c2;

            ptr ++;
            stack[ ptr ] = c1;

        }

    }

    return found;

}


#define PI 3.141592653589793
#define PI2 6.283185307179586
#define PI_HALF 1.5707963267948966
#define RECIPROCAL_PI 0.3183098861837907
#define RECIPROCAL_PI2 0.15915494309189535
#define EPSILON 1e-6
#ifndef saturate
#define saturate( a ) clamp( a, 0.0, 1.0 )
#endif
#define whiteComplement( a ) ( 1.0 - saturate( a ) )
float pow2( const in float x ) { return x*x; }
float pow3( const in float x ) { return x*x*x; }
float pow4( const in float x ) { float x2 = x*x; return x2*x2; }
float max3( const in vec3 v ) { return max( max( v.x, v.y ), v.z ); }
float average( const in vec3 color ) { return dot( color, vec3( 0.3333 ) ); }
highp float rand( const in vec2 uv ) {
    const highp float a = 12.9898, b = 78.233, c = 43758.5453;
    highp float dt = dot( uv.xy, vec2( a,b ) ), sn = mod( dt, PI );
    return fract( sin( sn ) * c );
}
#ifdef HIGH_PRECISION
    float precisionSafeLength( vec3 v ) { return length( v ); }
#else
    float precisionSafeLength( vec3 v ) {
        float maxComponent = max3( abs( v ) );
        return length( v / maxComponent ) * maxComponent;
    }
#endif
struct IncidentLight {
    vec3 color;
    vec3 direction;
    bool visible;
};
struct ReflectedLight {
    vec3 directDiffuse;
    vec3 directSpecular;
    vec3 indirectDiffuse;
    vec3 indirectSpecular;
};
struct GeometricContext {
    vec3 position;
    vec3 normal;
    vec3 viewDir;
#ifdef USE_CLEARCOAT
    vec3 clearcoatNormal;
#endif
};
vec3 transformDirection( in vec3 dir, in mat4 matrix ) {
    return normalize( ( matrix * vec4( dir, 0.0 ) ).xyz );
}
vec3 inverseTransformDirection( in vec3 dir, in mat4 matrix ) {
    return normalize( ( vec4( dir, 0.0 ) * matrix ).xyz );
}
mat3 transposeMat3( const in mat3 m ) {
    mat3 tmp;
    tmp[ 0 ] = vec3( m[ 0 ].x, m[ 1 ].x, m[ 2 ].x );
    tmp[ 1 ] = vec3( m[ 0 ].y, m[ 1 ].y, m[ 2 ].y );
    tmp[ 2 ] = vec3( m[ 0 ].z, m[ 1 ].z, m[ 2 ].z );
    return tmp;
}
float linearToRelativeLuminance( const in vec3 color ) {
    vec3 weights = vec3( 0.2126, 0.7152, 0.0722 );
    return dot( weights, color.rgb );
}
bool isPerspectiveMatrix( mat4 m ) {
    return m[ 2 ][ 3 ] == - 1.0;
}
vec2 equirectUv( in vec3 dir ) {
    float u = atan( dir.z, dir.x ) * RECIPROCAL_PI2 + 0.5;
    float v = asin( clamp( dir.y, - 1.0, 1.0 ) ) * RECIPROCAL_PI + 0.5;
    return vec2( u, v );
}

            uniform mat4 cameraWorldMatrix;
            uniform mat4 invProjectionMatrix;
            uniform sampler2D normalAttribute;
            uniform BVH bvh;
            uniform float seed;
            uniform float opacity;
            varying vec2 vUv;

            void main() {

                // get [-1, 1] normalized device coordinates
                vec2 ndc = 2.0 * vUv - vec2( 1.0 );
                vec3 rayOrigin, rayDirection;
                ndcToCameraRay( ndc, cameraWorldMatrix, invProjectionMatrix, rayOrigin, rayDirection );

                // Lambertian render
                gl_FragColor = vec4( 0.0 );

                vec3 throughputColor = vec3( 1.0 );
                vec3 randomPoint = vec3( .0 );

                // hit results
                uvec4 faceIndices = uvec4( 0u );
                vec3 faceNormal = vec3( 0.0, 0.0, 1.0 );
                vec3 barycoord = vec3( 0.0 );
                float side = 1.0;
                float dist = 0.0;

                for ( int i = 0; i < BOUNCES; i ++ ) {

                    if ( ! bvhIntersectFirstHit( bvh, rayOrigin, rayDirection, faceIndices, faceNormal, barycoord, side, dist ) ) {

                        float value = ( rayDirection.y + 0.5 ) / 1.5;
                        vec3 skyColor = mix( vec3( 1.0 ), vec3( 0.75, 0.85, 1.0 ), value );

                        gl_FragColor = vec4( skyColor * throughputColor * 2.0, 1.0 );

                        break;

                    }

                    // 1 / PI attenuation for physically correct lambert model
                    // https://www.rorydriscoll.com/2009/01/25/energy-conservation-in-games/
                    throughputColor *= 1.0 / PI;

                    randomPoint = vec3(
                        rand( vUv + float( i + 1 ) + vec2( seed, seed ) ),
                        rand( - vUv * seed + float( i ) - seed ),
                        rand( - vUv * float( i + 1 ) - vec2( seed, - seed ) )
                    );
                    randomPoint -= 0.5;
                    randomPoint *= 2.0;

                    // ensure the random vector is not 0,0,0 and that it won't exactly negate
                    // the surface normal

                    float pointLength = max( length( randomPoint ), 1e-4 );
                    randomPoint /= pointLength;
                    randomPoint *= 0.999;

                    // fetch the interpolated smooth normal
                    vec3 normal =
                        side *
                        textureSampleBarycoord(
                            normalAttribute,
                            barycoord,
                            faceIndices.xyz
                        ).xyz;

                    // adjust the hit point by the surface normal by a factor of some offset and the
                    // maximum component-wise value of the current point to accommodate floating point
                    // error as values increase.
                    vec3 point = rayOrigin + rayDirection * dist;
                    vec3 absPoint = abs( point );
                    float maxPoint = max( absPoint.x, max( absPoint.y, absPoint.z ) );
                    rayOrigin = point + faceNormal * ( maxPoint + 1.0 ) * RAY_OFFSET;
                    rayDirection = normalize( normal + randomPoint );

                }

                gl_FragColor.a = opacity;

            }
)";
    compile(shaderString, SH_VARIABLES);
}
