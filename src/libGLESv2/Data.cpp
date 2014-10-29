//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Data.cpp: Container class for all GL relevant state, caps and objects

#include "libGLESv2/Data.h"
#include "libGLESv2/ResourceManager.h"

namespace gl
{

Data::Data()
    : clientVersion(2),
      resourceManager(NULL)
{}

Data::~Data()
{
    for (auto &zeroTexture : zeroTextures)
    {
        zeroTexture.second.set(NULL);
    }
    zeroTextures.clear();

    if (resourceManager)
    {
        resourceManager->release();
    }
}

Texture *Data::getSamplerTexture(unsigned int sampler, GLenum type) const
{
    if (state.getSamplerTextureId(sampler, type) == 0)
    {
        return zeroTextures.at(type).get();
    }
    else
    {
        return state.getSamplerTexture(sampler, type);
    }
}

}
