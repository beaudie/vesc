//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// dispatch.cpp: Implements a function to fetch the ANGLE OpenCL dispatch table.

#include "libOpenCL/dispatch.h"

#include "anglebase/no_destructor.h"
#include "common/debug.h"
#include "common/system_utils.h"

#include <memory>

#ifdef _WIN32
#    include <windows.h>
#endif

namespace cl
{

namespace
{

std::unique_ptr<angle::Library> &EntryPointsLib()
{
    static angle::base::NoDestructor<std::unique_ptr<angle::Library>> sEntryPointsLib;
    return *sEntryPointsLib;
}

IcdDispatch CreateDispatch()
{
    const cl_icd_dispatch *clIcdDispatch = nullptr;
    const char *error                    = nullptr;

    INFO() << "Module directory: " << angle::GetModuleDirectory();

    // Try to find ANGLE's GLESv2 library in the consistent way
    EntryPointsLib().reset(
        angle::OpenSharedLibrary(ANGLE_GLESV2_LIBRARY_NAME, angle::SearchType::ApplicationDir));
    if (EntryPointsLib() && EntryPointsLib()->getNative() != nullptr)
    {
        EntryPointsLib()->getAs("gCLIcdDispatchTable", &clIcdDispatch);
        if (clIcdDispatch == nullptr)
        {
            INFO() << "Found system's instead of ANGLE's GLESv2 library";
        }
    }
    else
    {
        error = "Not able to find GLESv2 library";
    }

#ifdef _WIN32
    // If not found try to find ANGLE's GLESv2 library in build path
    if (clIcdDispatch == nullptr)
    {
        const char path[] = ANGLE_GLESV2_LIBRARY_PATH "\\" ANGLE_GLESV2_LIBRARY_NAME ".dll";
        HMODULE handle    = LoadLibraryExA(path, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
        if (handle != nullptr)
        {
            clIcdDispatch = reinterpret_cast<const cl_icd_dispatch *>(
                GetProcAddress(handle, "gCLIcdDispatchTable"));
            if (clIcdDispatch == nullptr)
            {
                error = "Error loading CL dispatch table.";
            }
            else
            {
                INFO() << "Found ANGLE's GLESv2 library in " << path;
            }
        }
        else
        {
            INFO() << "Could not open ANGLE's GLESv2 library in " << path;
        }
    }
#endif

    IcdDispatch dispatch;
    if (clIcdDispatch != nullptr)
    {
        static_cast<cl_icd_dispatch &>(dispatch) = *clIcdDispatch;
        dispatch.clIcdGetPlatformIDsKHR          = reinterpret_cast<clIcdGetPlatformIDsKHR_fn>(
            clIcdDispatch->clGetExtensionFunctionAddress("clIcdGetPlatformIDsKHR"));
    }
    else if (error != nullptr)
    {
        ERR() << error;
    }
    return dispatch;
}

}  // anonymous namespace

const IcdDispatch &GetDispatch()
{
    static const IcdDispatch sDispatch(CreateDispatch());
    return sDispatch;
}

}  // namespace cl
