//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef TESTS_TESTUTILS_SHADERLIBRARY_H_
#define TESTS_TESTUTILS_SHADERLIBRARY_H_

#include <string>

namespace angle
{

namespace shader_library
{

const char *positionAttribName();

namespace essl1
{

namespace vs
{

// A shader that sets gl_Position to zero.
const char *zero();

// A shader that sets gl_Position to attribute a_position.
const char *simple();

// A shader that passes through attribute a_position, setting it to gl_Position and varying
// v_position.
const char *passthrough();

}  // namespace vs

namespace fs
{

// A shader that renders a simple checker pattern of red and green. X axis and y axis separate the
// different colors. Needs varying v_position.
const char *checkered();

// A shader that fills with color taken from uniform named "color".
const char *uniformColor();

// A shader that fills with 100% opaque red.
const char *red();

// A shader that fills with 100% opaque blue.
const char *blue();

}  // namespace fs

}  // namespace essl1

namespace essl3
{

namespace vs
{

// A shader that sets gl_Position to zero.
const char *zero();

// A shader that sets gl_Position to attribute a_position.
const char *simple();

}  // namespace vs

namespace fs
{

// A shader that fills with 100% opaque red.
const char *red();

}  // namespace fs

}  // namespace essl3

namespace essl31
{

namespace vs
{

// A shader that sets gl_Position to zero.
const char *zero();

// A shader that sets gl_Position to attribute a_position.
const char *simple();

}  // namespace vs

namespace fs
{

// A shader that fills with 100% opaque red.
const char *red();

}  // namespace fs

}  // namespace essl31

}  // namespace shader_library

}  // namespace angle

#endif  // TESTS_TESTUTILS_SHADERLIBRARY_H_
