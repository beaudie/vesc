//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test_utils/shader_library.h"

namespace angle
{

namespace shader_library
{

const char *positionAttribName()
{
    return "a_position";
}

namespace essl1
{

namespace vs
{

// A shader that sets gl_Position to zero.
const char *zero()
{
    return R"(void main()
    {
        gl_Position = vec4(0);
    })";
}

// A shader that sets gl_Position to attribute a_position.
const char *simple()
{
    return R"(precision highp float;
    attribute vec4 a_position;

    void main()
    {
        gl_Position = a_position;
    })";
}

// A shader that simply passes through attribute a_position, setting it to gl_Position and varying
// pos.
const char *passthrough()
{
    return R"(precision highp float;
    attribute vec4 a_position;
    varying vec4 v_position;

    void main()
    {
        gl_Position = a_position;
        v_position = a_position;
    })";
}

}  // namespace vs

namespace fs
{

// A shader that renders a simple checker pattern of red and green. X axis and y axis separate the
// different colors. Needs varying v_position.
const char *checkered()
{
    return R"(precision highp float;
    varying vec4 v_position;

    void main()
    {
        if (v_position.x * v_position.y > 0.0)
        {
            gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
        }
        else
        {
            gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);
        }
    })";
}

// A shader that fills with color taken from uniform named "color".
const char *uniformColor()
{
    return R"(uniform highp vec4 color;
    void main(void)
    {
        gl_FragColor = color;
    })";
}

// A shader that fills with 100% opaque red.
const char *red()
{
    return R"(precision highp float;

    void main()
    {
        gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
    })";
}

// A shader that fills with 100% opaque blue.
const char *blue()
{
    return R"(precision highp float;

    void main()
    {
        gl_FragColor = vec4(0.0, 0.0, 1.0, 1.0);
    })";
}

}  // namespace fs

}  // namespace essl1

namespace essl3
{

namespace vs
{

// A shader that sets gl_Position to zero.
const char *zero()
{
    return R"(#version 300 es
    void main()
    {
        gl_Position = vec4(0);
    })";
}

// A shader that sets gl_Position to attribute a_position.
const char *simple()
{
    return R"(#version 300 es
    in vec4 a_position;
    void main()
    {
        gl_Position = a_position;
    })";
}

}  // namespace vs

}  // namespace essl3

namespace essl31
{

namespace vs
{

// A shader that sets gl_Position to zero.
const char *zero()
{
    return R"(#version 310 es
    void main()
    {
        gl_Position = vec4(0);
    })";
}

// A shader that sets gl_Position to attribute a_position.
const char *simple()
{
    return R"(#version 310 es
    in vec4 a_position;
    void main()
    {
        gl_Position = a_position;
    })";
}

}  // namespace vs

}  // namespace essl31

}  // namespace shader_library

}  // namespace angle
