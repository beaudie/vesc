//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "SimpleBenchmark.h"
#include "BufferSubData.h"
#include "TexSubImage.h"
#include "PointSprites.h"

#include <iostream>
#include <rapidjson/document.h>
#include <rapidjson/filestream.h>

namespace
{

template <class T>
struct Optional
{
    bool valid;
    T value;

    Optional(bool validIn, T valueIn)
        : valid(validIn),
          value(valueIn)
    {}

    Optional(T valueIn)
        : valid(true),
          value(valueIn)
    {}

    static Optional None()
    {
        return Optional(false, static_cast<T>(0));
    }
};

Optional<const char *> GetStringMember(const rapidjson::Document &document, const char *name)
{
    auto typeIt = document.FindMember(name);
    if (typeIt == document.MemberEnd() || !typeIt->value.IsString())
    {
        std::cerr << "JSON has missing or bad string member '" << name << "'" << std::endl;
        return Optional<const char *>::None();
    }

    return Optional<const char *>(typeIt->value.GetString());
}

Optional<bool> GetBoolMember(const rapidjson::Document &document, const char *name)
{
    auto typeIt = document.FindMember(name);
    if (typeIt == document.MemberEnd() || !typeIt->value.IsBool())
    {
        std::cerr << "JSON has missing or bad bool member '" << name << "'" << std::endl;
        return Optional<bool>::None();
    }

    return Optional<bool>(typeIt->value.GetBool());
}

Optional<int> GetIntMember(const rapidjson::Document &document, const char *name)
{
    auto typeIt = document.FindMember(name);
    if (typeIt == document.MemberEnd() || !typeIt->value.IsInt())
    {
        std::cerr << "JSON has missing or bad int member '" << name << "'" << std::endl;
        return Optional<int>::None();
    }

    return Optional<int>(typeIt->value.GetInt());
}

Optional<unsigned int> GetUintMember(const rapidjson::Document &document, const char *name)
{
    auto typeIt = document.FindMember(name);
    if (typeIt == document.MemberEnd() || !typeIt->value.IsUint())
    {
        std::cerr << "JSON has missing or bad uint member '" << name << "'" << std::endl;
        return Optional<unsigned int>::None();
    }

    return Optional<unsigned int>(typeIt->value.GetUint());
}

EGLint ParseRendererType(const char *value)
{
    if (strcmp(value, "d3d11") == 0)
    {
        return EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE;
    }
    else if (strcmp(value, "d3d9") == 0)
    {
        return EGL_PLATFORM_ANGLE_TYPE_D3D9_ANGLE;
    }
    else if (strcmp(value, "warp") == 0)
    {
        return EGL_PLATFORM_ANGLE_TYPE_D3D11_WARP_ANGLE;
    }
    else if (strcmp(value, "default") == 0)
    {
        return EGL_PLATFORM_ANGLE_TYPE_DEFAULT_ANGLE;
    }
    else
    {
        return EGL_NONE;
    }
}

GLenum ParseAttribType(const char *value)
{
    if (strcmp(value, "float") == 0)
    {
        return GL_FLOAT;
    }
    else if (strcmp(value, "int") == 0)
    {
        return GL_INT;
    }
    else if (strcmp(value, "uint") == 0)
    {
        return GL_UNSIGNED_INT;
    }
    else if (strcmp(value, "short") == 0)
    {
        return GL_SHORT;
    }
    else if (strcmp(value, "ushort") == 0)
    {
        return GL_UNSIGNED_SHORT;
    }
    else if (strcmp(value, "byte") == 0)
    {
        return GL_BYTE;
    }
    else if (strcmp(value, "ubyte") == 0)
    {
        return GL_UNSIGNED_BYTE;
    }
    else
    {
        return GL_NONE;
    }
}

bool ParseBenchmarkParams(const rapidjson::Document &document, BufferSubDataParams *params)
{
    // Validate params
    auto type = GetStringMember(document, "type");
    auto components = GetUintMember(document, "components");
    auto normalized = GetBoolMember(document, "normalized");
    auto updateSize = GetUintMember(document, "update_size");
    auto bufferSize = GetUintMember(document, "buffer_size");
    auto iterations = GetUintMember(document, "iterations");
    auto updateRate = GetUintMember(document, "update_rate");

    if (!type.valid || !components.valid || !normalized.valid || !updateSize.valid ||
        !bufferSize.valid || !iterations.valid || !updateRate.valid)
    {
        return false;
    }

    GLenum vertexType = ParseAttribType(type.value);
    if (vertexType == GL_NONE)
    {
        std::cerr << "Invalid attribute type: " << type.value << std::endl;
        return false;
    }

    if (components.value < 1 || components.value > 4)
    {
        std::cerr << "Invalid component count: " << components.value << std::endl;
        return false;
    }

    if (normalized.value && vertexType == GL_FLOAT)
    {
        std::cerr << "Normalized float is not a valid vertex type." << std::endl;
        return false;
    }

    if (bufferSize.value == 0)
    {
        std::cerr << "Zero buffer size is not valid." << std::endl;
        return false;
    }

    if (iterations.value == 0)
    {
        std::cerr << "Zero iterations not valid." << std::endl;
        return false;
    }

    params->vertexType = vertexType;
    params->vertexComponentCount = components.value;
    params->vertexNormalized = normalized.value;
    params->updateSize = updateSize.value;
    params->bufferSize = bufferSize.value;
    params->iterations = iterations.value;
    params->updateRate = updateRate.value;

    return true;
}

bool ParseBenchmarkParams(const rapidjson::Document &document, TexSubImageParams *params)
{
    // TODO(jmadll): Parse and validate parameters

    params->imageWidth = 1024;
    params->imageHeight = 1024;
    params->subImageHeight = 64;
    params->subImageWidth = 64;
    params->iterations = 10;

    return true;
}

bool ParseBenchmarkParams(const rapidjson::Document &document, PointSpritesParams *params)
{
    // TODO(jmadll): Parse and validate parameters

    params->iterations = 10;
    params->count = 10;
    params->size = 3.0f;
    params->numVaryings = 3;

    return true;
}

template <class BenchT>
int ParseAndRunBenchmark(EGLint rendererType, const rapidjson::Document &document)
{
    BenchT::Params params;
    if (!ParseBenchmarkParams(document, &params))
    {
        // Parse or validation error
        return 1;
    }

    params.requestedRenderer = rendererType;

    BenchT benchmark(params);

    // Run the benchmark
    return benchmark.run();
}

}

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        std::cerr << "Must specify a renderer and source json file." << std::endl;
        return 1;
    }

    EGLint rendererType = ParseRendererType(argv[1]);
    if (rendererType == EGL_NONE)
    {
        std::cerr << "Invalid renderer type: " << argv[1] << std::endl;
        return 1;
    }

    FILE *fp = fopen(argv[2], "rt");
    if (fp == NULL)
    {
        std::cerr << "Cannot open " << argv[2] << std::endl;
        return 1;
    }

    rapidjson::FileStream fileStream(fp);
    rapidjson::Document document;

    if (document.ParseStream(fileStream).HasParseError())
    {
        std::cerr << "JSON Parse error code " << document.GetParseError() << "." << std::endl;
        return 1;
    }

    fclose(fp);

    Optional<const char *> testName = GetStringMember(document, "test");

    if (!testName.valid)
    {
        return 1;
    }

    if (strcmp(testName.value, "BufferSubData") == 0)
    {
        return ParseAndRunBenchmark<BufferSubDataBenchmark>(rendererType, document);
    }
    else if (strcmp(testName.value, "TexSubImage") == 0)
    {
        return ParseAndRunBenchmark<TexSubImageBenchmark>(rendererType, document);
    }
    else if (strcmp(testName.value, "PointSprites") == 0)
    {
        return ParseAndRunBenchmark<PointSpritesBenchmark>(rendererType, document);
    }
    else
    {
        std::cerr << "Unknown test: " << testName.value << std::endl;
        return 1;
    }
}
