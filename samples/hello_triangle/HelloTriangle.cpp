//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

//            Based on Hello_Triangle.c from
// Book:      OpenGL(R) ES 2.0 Programming Guide
// Authors:   Aaftab Munshi, Dan Ginsburg, Dave Shreiner
// ISBN-10:   0321502795
// ISBN-13:   9780321502797
// Publisher: Addison-Wesley Professional
// URLs:      http://safari.informit.com/9780321563835
//            http://www.opengles-book.com

#include "SampleApplication.h"

#include "util/shader_utils.h"

#include <chrono>  // std::chrono::seconds
#include <thread>  // std::this_thread::sleep_for

class HelloTriangleSample : public SampleApplication
{
  public:
    HelloTriangleSample(int argc, char **argv)
        : SampleApplication("HelloTriangle", argc, argv, ClientType::ES3_0, 640, 640)
    {}

    bool initialize() override
    {

        std::cout << glGetString(GL_RENDERER) << std::endl;

        static constexpr const char *vs = R"STR(#version 300 es
    // an attribute is an input (in) to a vertex shader.
    // It will receive data from a buffer
    in vec4 a_position;

    // all shaders have a main function
    void main() {

      // gl_Position is a special variable a vertex shader
      // is responsible for setting
      gl_Position = a_position;
    })STR";

        static constexpr const char *fs = R"STR(#version 300 es
    precision highp float;

    uniform vec2 iResolution;
    uniform vec2 iMouse;
    uniform float iTime;

    // we need to declare an output for the fragment shader
    out vec4 outColor;

// Protean clouds by nimitz (twitter: @stormoid)
// https://www.shadertoy.com/view/3l23Rh
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License
// Contact the author for other licensing options

/*
    Technical details:

    The main volume noise is generated from a deformed periodic grid, which can produce
    a large range of noise-like patterns at very cheap evalutation cost. Allowing for multiple
    fetches of volume gradient computation for improved lighting.

    To further accelerate marching, since the volume is smooth, more than half the the density
    information isn't used to rendering or shading but only as an underlying volume distance to
    determine dynamic step size, by carefully selecting an equation (polynomial for speed) to
    step as a function of overall density (not necessarialy rendered) the visual results can be
    the same as a naive implementation with ~40% increase in rendering performance.

    Since the dynamic marching step size is even less uniform due to steps not being rendered at all
    the fog is evaluated as the difference of the fog integral at each rendered step.

*/

mat2 rot(in float a){float c = cos(a), s = sin(a);return mat2(c,s,-s,c);}
const mat3 m3 = mat3(0.33338, 0.56034, -0.71817, -0.87887, 0.32651, -0.15323, 0.15162, 0.69596, 0.61339)*1.93;
float mag2(vec2 p){return dot(p,p);}
float linstep(in float mn, in float mx, in float x){ return clamp((x - mn)/(mx - mn), 0., 1.); }
float prm1 = 0.;
vec2 bsMo = vec2(0);

vec2 disp(float t){ return vec2(sin(t*0.22)*1., cos(t*0.175)*1.)*2.; }

vec2 map(vec3 p)
{
    vec3 p2 = p;
    p2.xy -= disp(p.z).xy;
    p.xy *= rot(sin(p.z+iTime)*(0.1 + prm1*0.05) + iTime*0.09);
    float cl = mag2(p2.xy);
    float d = 0.;
    p *= .61;
    float z = 1.;
    float trk = 1.;
    float dspAmp = 0.1 + prm1*0.2;
    for(int i = 0; i < 5; i++)
    {
        p += sin(p.zxy*0.75*trk + iTime*trk*.8)*dspAmp;
        d -= abs(dot(cos(p), sin(p.yzx))*z);
        z *= 0.57;
        trk *= 1.4;
        p = p*m3;
    }
    d = abs(d + prm1*3.)+ prm1*.3 - 2.5 + bsMo.y;
    return vec2(d + cl*.2 + 0.25, cl);
}

vec4 render( in vec3 ro, in vec3 rd, float time )
{
    vec4 rez = vec4(0);
    const float ldst = 8.;
    vec3 lpos = vec3(disp(time + ldst)*0.5, time + ldst);
    float t = 1.5;
    float fogT = 0.;
    for(int i=0; i<50; i++)
    {
        if(rez.a > 0.99)break;

        vec3 pos = ro + t*rd;
        vec2 mpv = map(pos);
        float den = clamp(mpv.x-0.3,0.,1.)*1.12;
        float dn = clamp((mpv.x + 2.),0.,3.);

        vec4 col = vec4(0);
        if (mpv.x > 0.6)
        {

            col = vec4(sin(vec3(5.,0.4,0.2) + mpv.y*0.1 +sin(pos.z*0.4)*0.5 + 1.8)*0.5 + 0.5,0.08);
            col *= den*den*den;
            col.rgb *= linstep(4.,-2.5, mpv.x)*2.3;
            float dif =  clamp((den - map(pos+.8).x)/9., 0.001, 1. );
            dif += clamp((den - map(pos+.35).x)/2.5, 0.001, 1. );
            col.xyz *= den*(vec3(0.005,.045,.075) + 1.5*vec3(0.033,0.07,0.03)*dif);
        }

        float fogC = exp(t*0.2 - 2.2);
        col.rgba += vec4(0.06,0.11,0.11, 0.1)*clamp(fogC-fogT, 0., 1.);
        fogT = fogC;
        rez = rez + col*(1. - rez.a);
        t += clamp(0.5 - dn*dn*.05, 0.09, 0.3);
    }
    return clamp(rez, 0.0, 1.0);
}

float getsat(vec3 c)
{
    float mi = min(min(c.x, c.y), c.z);
    float ma = max(max(c.x, c.y), c.z);
    return (ma - mi)/(ma+ 1e-7);
}

//from my "Will it blend" shader (https://www.shadertoy.com/view/lsdGzN)
vec3 iLerp(in vec3 a, in vec3 b, in float x)
{
    vec3 ic = mix(a, b, x) + vec3(1e-6,0.,0.);
    float sd = abs(getsat(ic) - mix(getsat(a), getsat(b), x));
    vec3 dir = normalize(vec3(2.*ic.x - ic.y - ic.z, 2.*ic.y - ic.x - ic.z, 2.*ic.z - ic.y - ic.x));
    float lgt = dot(vec3(1.0), ic);
    float ff = dot(dir, normalize(ic));
    ic += 1.5*dir*sd*ff*lgt;
    return clamp(ic,0.,1.);
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    vec2 q = fragCoord.xy/iResolution.xy;
    vec2 p = (gl_FragCoord.xy - 0.5*iResolution.xy)/iResolution.y;
    bsMo = (iMouse.xy - 0.5*iResolution.xy)/iResolution.y;

    float time = iTime*3.;
    vec3 ro = vec3(0,0,time);

    ro += vec3(sin(iTime)*0.5,sin(iTime*1.)*0.,0);

    float dspAmp = .85;
    ro.xy += disp(ro.z)*dspAmp;
    float tgtDst = 3.5;

    vec3 target = normalize(ro - vec3(disp(time + tgtDst)*dspAmp, time + tgtDst));
    ro.x -= bsMo.x*2.;
    vec3 rightdir = normalize(cross(target, vec3(0,1,0)));
    vec3 updir = normalize(cross(rightdir, target));
    rightdir = normalize(cross(updir, target));
    vec3 rd=normalize((p.x*rightdir + p.y*updir)*1. - target);
    rd.xy *= rot(-disp(time + 3.5).x*0.2 + bsMo.x);
    prm1 = smoothstep(-0.4, 0.4,sin(iTime*0.3));
    vec4 scn = render(ro, rd, time);

    vec3 col = scn.rgb;
    col = iLerp(col.bgr, col.rgb, clamp(1.-prm1,0.05,1.));

    col = pow(col, vec3(.55,0.65,0.6))*vec3(1.,.97,.9);

    col *= pow( 16.0*q.x*q.y*(1.0-q.x)*(1.0-q.y), 0.12)*0.7+0.3; //Vign

    fragColor = vec4( col, 1.0 );
}

    void main() {
      mainImage(outColor, gl_FragCoord.xy);
    })STR";

        auto createShader = [](GLenum type, const char *src) {
            GLuint shader = glCreateShader(type);
            glShaderSource(shader, 1, &src, nullptr);
            glCompileShader(shader);
            return shader;
        };

        GLuint vShader = createShader(GL_VERTEX_SHADER, vs);
        GLuint fShader = createShader(GL_FRAGMENT_SHADER, fs);

        mProgram = glCreateProgram();
        glAttachShader(mProgram, vShader);
        glAttachShader(mProgram, fShader);
        glLinkProgram(mProgram);

        GLint linkStatus;
        glGetProgramiv(mProgram, GL_LINK_STATUS, &linkStatus);
        if (linkStatus != EGL_TRUE)
        {
            return false;
        }

        glUseProgram(mProgram);

        // look up where the vertex data needs to go.
        GLint positionAttributeLocation = glGetAttribLocation(mProgram, "a_position");

        // look up uniform locations
        resolutionLocation = glGetUniformLocation(mProgram, "iResolution");
        mouseLocation      = glGetUniformLocation(mProgram, "iMouse");
        timeLocation       = glGetUniformLocation(mProgram, "iTime");

        // Create a vertex array object (attribute state)
        GLuint vao = 0;
        glGenVertexArrays(1, &vao);

        // and make it the one we're currently working with
        glBindVertexArray(vao);

        // Create a buffer to put three 2d clip space points in
        GLuint positionBuffer;
        glGenBuffers(1, &positionBuffer);

        // Bind it to ARRAY_BUFFER (think of it as ARRAY_BUFFER = positionBuffer)
        glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);

        // fill it with a 2 triangles that cover clip space
        std::vector<float> bufferData = {
            -1, -1,                // first triangle
            1,  -1, -1, 1, -1, 1,  // second triangle
            1,  -1, 1,  1,
        };
        glBufferData(GL_ARRAY_BUFFER, bufferData.size() * sizeof(bufferData[0]), bufferData.data(),
                     GL_STATIC_DRAW);

        // Turn on the attribute
        glEnableVertexAttribArray(positionAttributeLocation);

        // Tell the attribute how to get data out of positionBuffer (ARRAY_BUFFER)
        glVertexAttribPointer(
            positionAttributeLocation,
            2,         // 2 components per iteration
            GL_FLOAT,  // the data is 32bit floats
            false,     // don't normalize the data
            0,  // 0 = move forward size * sizeof(type) each iteration to get the next position
            0   // start at the beginning of the buffer
        );

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

        return true;
    }

    void destroy() override { glDeleteProgram(mProgram); }

    void step(float dt, double totalTime) override { time += dt; }

    void draw() override
    {
        glUseProgram(mProgram);
        glUniform2f(resolutionLocation, getWindow()->getWidth(), getWindow()->getHeight());
        glUniform2f(mouseLocation, 0, 0);
        glUniform1f(timeLocation, time);

        // Set the viewport
        glViewport(0, 0, getWindow()->getWidth(), getWindow()->getHeight());
        glClear(GL_COLOR_BUFFER_BIT);

        glDrawArrays(GL_TRIANGLES,
                     0,  // offset
                     6   // num vertices to process
        );

        // std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

  private:
    GLuint mProgram;
    GLint resolutionLocation;
    GLint mouseLocation;
    GLint timeLocation;
    float time = 0;
};

int main(int argc, char **argv)
{
    HelloTriangleSample app(argc, argv);
    return app.run();
}
