//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// cl_dispatch_table_autogen.cpp: Dispatch table for CL ICD Loader.

#include "libGLESv2/cl_dispatch_table.h"

#include "libGLESv2/entry_points_cl_autogen.h"

// clang-format off

// The correct order is required, not reasonable to autogenerate from cl.xml.
cl_icd_dispatch gCLIcdDispatchTable = {

    // OpenCL 1.0
    CL_GetPlatformIDs,
    CL_GetPlatformInfo,
    CL_GetDeviceIDs,
    CL_GetDeviceInfo,
    CL_CreateContext,
    CL_CreateContextFromType,
    CL_RetainContext,
    CL_ReleaseContext,
    CL_GetContextInfo,
    CL_CreateCommandQueue,
    CL_RetainCommandQueue,
    CL_ReleaseCommandQueue,
    CL_GetCommandQueueInfo,
    CL_SetCommandQueueProperty,
    CL_CreateBuffer,
    CL_CreateImage2D,
    CL_CreateImage3D,
    CL_RetainMemObject,
    CL_ReleaseMemObject,
    CL_GetSupportedImageFormats,
    CL_GetMemObjectInfo,
    CL_GetImageInfo,
    CL_CreateSampler,
    CL_RetainSampler,
    CL_ReleaseSampler,
    CL_GetSamplerInfo,
    CL_CreateProgramWithSource,
    CL_CreateProgramWithBinary,
    CL_RetainProgram,
    CL_ReleaseProgram,
    CL_BuildProgram,
    CL_UnloadCompiler,
    CL_GetProgramInfo,
    CL_GetProgramBuildInfo,
    CL_CreateKernel,
    CL_CreateKernelsInProgram,
    CL_RetainKernel,
    CL_ReleaseKernel,
    CL_SetKernelArg,
    CL_GetKernelInfo,
    CL_GetKernelWorkGroupInfo,
    CL_WaitForEvents,
    CL_GetEventInfo,
    CL_RetainEvent,
    CL_ReleaseEvent,
    CL_GetEventProfilingInfo,
    CL_Flush,
    CL_Finish,
    CL_EnqueueReadBuffer,
    CL_EnqueueWriteBuffer,
    CL_EnqueueCopyBuffer,
    CL_EnqueueReadImage,
    CL_EnqueueWriteImage,
    CL_EnqueueCopyImage,
    CL_EnqueueCopyImageToBuffer,
    CL_EnqueueCopyBufferToImage,
    CL_EnqueueMapBuffer,
    CL_EnqueueMapImage,
    CL_EnqueueUnmapMemObject,
    CL_EnqueueNDRangeKernel,
    CL_EnqueueTask,
    CL_EnqueueNativeKernel,
    CL_EnqueueMarker,
    CL_EnqueueWaitForEvents,
    CL_EnqueueBarrier,
    CL_GetExtensionFunctionAddress,
    nullptr, // CL_CreateFromGLBuffer,
    nullptr, // CL_CreateFromGLTexture2D,
    nullptr, // CL_CreateFromGLTexture3D,
    nullptr, // CL_CreateFromGLRenderbuffer,
    nullptr, // CL_GetGLObjectInfo,
    nullptr, // CL_GetGLTextureInfo,
    nullptr, // CL_EnqueueAcquireGLObjects,
    nullptr, // CL_EnqueueReleaseGLObjects,
    nullptr, // CL_GetGLContextInfoKHR,

    // cl_khr_d3d10_sharing
    nullptr, // CL_GetDeviceIDsFromD3D10KHR,
    nullptr, // CL_CreateFromD3D10BufferKHR,
    nullptr, // CL_CreateFromD3D10Texture2DKHR,
    nullptr, // CL_CreateFromD3D10Texture3DKHR,
    nullptr, // CL_EnqueueAcquireD3D10ObjectsKHR,
    nullptr, // CL_EnqueueReleaseD3D10ObjectsKHR,

    // OpenCL 1.1
    CL_SetEventCallback,
    CL_CreateSubBuffer,
    CL_SetMemObjectDestructorCallback,
    CL_CreateUserEvent,
    CL_SetUserEventStatus,
    CL_EnqueueReadBufferRect,
    CL_EnqueueWriteBufferRect,
    CL_EnqueueCopyBufferRect,

    // cl_ext_device_fission
    nullptr, // CL_CreateSubDevicesEXT,
    nullptr, // CL_RetainDeviceEXT,
    nullptr, // CL_ReleaseDeviceEXT,

    // cl_khr_gl_event
    nullptr, // CL_CreateEventFromGLsyncKHR,

    // OpenCL 1.2
    CL_CreateSubDevices,
    CL_RetainDevice,
    CL_ReleaseDevice,
    CL_CreateImage,
    CL_CreateProgramWithBuiltInKernels,
    CL_CompileProgram,
    CL_LinkProgram,
    CL_UnloadPlatformCompiler,
    CL_GetKernelArgInfo,
    CL_EnqueueFillBuffer,
    CL_EnqueueFillImage,
    CL_EnqueueMigrateMemObjects,
    CL_EnqueueMarkerWithWaitList,
    CL_EnqueueBarrierWithWaitList,
    CL_GetExtensionFunctionAddressForPlatform,
    nullptr, // CL_CreateFromGLTexture,

    // cl_khr_d3d11_sharing
    nullptr, // CL_GetDeviceIDsFromD3D11KHR,
    nullptr, // CL_CreateFromD3D11BufferKHR,
    nullptr, // CL_CreateFromD3D11Texture2DKHR,
    nullptr, // CL_CreateFromD3D11Texture3DKHR,
    nullptr, // CL_CreateFromDX9MediaSurfaceKHR,
    nullptr, // CL_EnqueueAcquireD3D11ObjectsKHR,
    nullptr, // CL_EnqueueReleaseD3D11ObjectsKHR,

    // cl_khr_dx9_media_sharing
    nullptr, // CL_GetDeviceIDsFromDX9MediaAdapterKHR,
    nullptr, // CL_EnqueueAcquireDX9MediaSurfacesKHR,
    nullptr, // CL_EnqueueReleaseDX9MediaSurfacesKHR,

    // cl_khr_egl_image
    nullptr, // CL_CreateFromEGLImageKHR,
    nullptr, // CL_EnqueueAcquireEGLObjectsKHR,
    nullptr, // CL_EnqueueReleaseEGLObjectsKHR,

    // cl_khr_egl_event
    nullptr, // CL_CreateEventFromEGLSyncKHR,

    // OpenCL 2.0
    CL_CreateCommandQueueWithProperties,
    CL_CreatePipe,
    CL_GetPipeInfo,
    CL_SVMAlloc,
    CL_SVMFree,
    CL_EnqueueSVMFree,
    CL_EnqueueSVMMemcpy,
    CL_EnqueueSVMMemFill,
    CL_EnqueueSVMMap,
    CL_EnqueueSVMUnmap,
    CL_CreateSamplerWithProperties,
    CL_SetKernelArgSVMPointer,
    CL_SetKernelExecInfo,

    // cl_khr_sub_groups
    nullptr, // CL_GetKernelSubGroupInfoKHR,

    // OpenCL 2.1
    CL_CloneKernel,
    CL_CreateProgramWithIL,
    CL_EnqueueSVMMigrateMem,
    CL_GetDeviceAndHostTimer,
    CL_GetHostTimer,
    CL_GetKernelSubGroupInfo,
    CL_SetDefaultDeviceCommandQueue,

    // OpenCL 2.2
    CL_SetProgramReleaseCallback,
    CL_SetProgramSpecializationConstant,

    // OpenCL 3.0
    CL_CreateBufferWithProperties,
    CL_CreateImageWithProperties,
    CL_SetContextDestructorCallback
};

// clang-format on
