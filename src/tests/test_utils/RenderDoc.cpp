//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RenderDoc:
//   Connection to renderdoc for capturing tests through its API.
//

#include "RenderDoc.h"

#include "common/angleutils.h"
#include "third_party/renderdoc/src/renderdoc_app.h"

#if defined(ANGLE_PLATFORM_WINDOWS)
constexpr char kRenderDocModuleName[] = "renderdoc";
#elif defined(ANGLE_PLATFORM_ANDROID)
constexpr char kRenderDocModuleName[] = "libVkLayer_GLES_RenderDoc";
#else
constexpr char kRenderDocModuleName[] = "librenderdoc";
#endif

RenderDoc::RenderDoc() : mRenderDocModule(nullptr), mApi(nullptr) {}

RenderDoc::~RenderDoc()
{
    SafeDelete(mRenderDocModule);
}

void RenderDoc::attach()
{
    mRenderDocModule = OpenSharedLibrary(kRenderDocModuleName, angle::SearchType::AlreadyLoaded);
    if (mRenderDocModule == nullptr || mRenderDocModule->getNative() == nullptr)
    {
        return;
    }
    void *getApi = mRenderDocModule->getSymbol("RENDERDOC_GetAPI");
    if (getApi == nullptr)
    {
        return;
    }

    int result = reinterpret_cast<pRENDERDOC_GetAPI>(getApi)(eRENDERDOC_API_Version_1_1_2, &mApi);
    if (result != 1)
    {
        printf("RenderDoc module is present but API 1.1.2 is unavailable\n");
        mApi = nullptr;
    }
}

void RenderDoc::startFrame()
{
    if (mApi)
    {
        static_cast<RENDERDOC_API_1_1_2 *>(mApi)->StartFrameCapture(nullptr, nullptr);
    }
}

void RenderDoc::endFrame()
{
    if (mApi)
    {
        static_cast<RENDERDOC_API_1_1_2 *>(mApi)->EndFrameCapture(nullptr, nullptr);
    }
}
