//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// validationEGL.h: Validation functions for generic EGL entry point parameters

#ifndef LIBANGLE_VALIDATIONEGL_H_
#define LIBANGLE_VALIDATIONEGL_H_

#include "common/PackedEnums.h"
#include "libANGLE/Error.h"
#include "libANGLE/Texture.h"

#include <EGL/egl.h>
#include <EGL/eglext.h>

namespace gl
{
class Context;
}

namespace egl
{

class AttributeMap;
struct ClientExtensions;
struct Config;
class Device;
class Display;
class Image;
class Stream;
class Surface;
class Sync;
class Thread;
class LabeledObject;

struct ValidationContext
{
    ValidationContext(Thread *threadIn, const char *entryPointIn, const LabeledObject *objectIn)
        : eglThread(threadIn), entryPoint(entryPointIn), labeledObject(objectIn)
    {}

    // We should remove the message-less overload once we have messages for all EGL errors.
    void setError(EGLint error);
    void setError(EGLint error, const char *message...);

    Thread *eglThread;
    const char *entryPoint;
    const LabeledObject *labeledObject;
};

// Object validation
bool ValidateDisplay(ValidationContext *val, const Display *display);
bool ValidateSurface(ValidationContext *val, const Display *display, const Surface *surface);
bool ValidateConfig(ValidationContext *val, const Display *display, const Config *config);
bool ValidateContext(ValidationContext *val, const Display *display, const gl::Context *context);
bool ValidateImage(ValidationContext *val, const Display *display, const Image *image);
bool ValidateDevice(ValidationContext *val, const Device *device);
bool ValidateSync(ValidationContext *val, const Display *display, const Sync *sync);

// Return the requested object only if it is valid (otherwise nullptr)
const Thread *GetThreadIfValid(const Thread *thread);
const Display *GetDisplayIfValid(const Display *display);
const Surface *GetSurfaceIfValid(const Display *display, const Surface *surface);
const Image *GetImageIfValid(const Display *display, const Image *image);
const Stream *GetStreamIfValid(const Display *display, const Stream *stream);
const gl::Context *GetContextIfValid(const Display *display, const gl::Context *context);
const Device *GetDeviceIfValid(const Device *device);
const Sync *GetSyncIfValid(const Display *display, const Sync *sync);
LabeledObject *GetLabeledObjectIfValid(Thread *thread,
                                       const Display *display,
                                       ObjectType objectType,
                                       EGLObjectKHR object);

// Entry point validation
bool ValidateInitialize(ValidationContext *val, const Display *display);

bool ValidateTerminate(ValidationContext *val, const Display *display);

bool ValidateCreateContext(ValidationContext *val,
                           Display *display,
                           Config *configuration,
                           gl::Context *shareContext,
                           const AttributeMap &attributes);

bool ValidateCreateWindowSurface(ValidationContext *val,
                                 Display *display,
                                 Config *config,
                                 EGLNativeWindowType window,
                                 const AttributeMap &attributes);

bool ValidateCreatePbufferSurface(ValidationContext *val,
                                  Display *display,
                                  Config *config,
                                  const AttributeMap &attributes);
bool ValidateCreatePbufferFromClientBuffer(ValidationContext *val,
                                           Display *display,
                                           EGLenum buftype,
                                           EGLClientBuffer buffer,
                                           Config *config,
                                           const AttributeMap &attributes);

bool ValidateCreatePixmapSurface(ValidationContext *val,
                                 Display *display,
                                 Config *config,
                                 EGLNativePixmapType pixmap,
                                 const AttributeMap &attributes);

bool ValidateMakeCurrent(ValidationContext *val,
                         Display *display,
                         Surface *draw,
                         Surface *read,
                         gl::Context *context);

bool ValidateCreateImage(ValidationContext *val,
                         const Display *display,
                         gl::Context *context,
                         EGLenum target,
                         EGLClientBuffer buffer,
                         const AttributeMap &attributes);
bool ValidateDestroyImage(ValidationContext *val, const Display *display, const Image *image);
bool ValidateCreateImageKHR(ValidationContext *val,
                            const Display *display,
                            gl::Context *context,
                            EGLenum target,
                            EGLClientBuffer buffer,
                            const AttributeMap &attributes);
bool ValidateDestroyImageKHR(ValidationContext *val, const Display *display, const Image *image);

bool ValidateCreateDeviceANGLE(ValidationContext *val,
                               EGLint device_type,
                               void *native_device,
                               const EGLAttrib *attrib_list);
bool ValidateReleaseDeviceANGLE(ValidationContext *val, Device *device);

bool ValidateCreateSyncBase(ValidationContext *val,
                            const Display *display,
                            EGLenum type,
                            const AttributeMap &attribs,
                            const Display *currentDisplay,
                            const gl::Context *currentContext,
                            bool isExt);
bool ValidateGetSyncAttribBase(ValidationContext *val,
                               const Display *display,
                               const Sync *sync,
                               EGLint attribute);

bool ValidateCreateSyncKHR(ValidationContext *val,
                           const Display *display,
                           EGLenum type,
                           const AttributeMap &attribs,
                           const Display *currentDisplay,
                           const gl::Context *currentContext);
bool ValidateCreateSync(ValidationContext *val,
                        const Display *display,
                        EGLenum type,
                        const AttributeMap &attribs,
                        const Display *currentDisplay,
                        const gl::Context *currentContext);
bool ValidateDestroySync(ValidationContext *val, const Display *display, const Sync *sync);
bool ValidateClientWaitSync(ValidationContext *val,
                            const Display *display,
                            const Sync *sync,
                            EGLint flags,
                            EGLTime timeout);
bool ValidateWaitSync(ValidationContext *val,
                      const Display *display,
                      const gl::Context *context,
                      const Sync *sync,
                      EGLint flags);
bool ValidateGetSyncAttribKHR(ValidationContext *val,
                              const Display *display,
                              const Sync *sync,
                              EGLint attribute,
                              EGLint *value);
bool ValidateGetSyncAttrib(ValidationContext *val,
                           const Display *display,
                           const Sync *sync,
                           EGLint attribute,
                           EGLAttrib *value);

bool ValidateCreateStreamKHR(ValidationContext *val,
                             const Display *display,
                             const AttributeMap &attributes);
bool ValidateDestroyStreamKHR(ValidationContext *val, const Display *display, const Stream *stream);
bool ValidateStreamAttribKHR(ValidationContext *val,
                             const Display *display,
                             const Stream *stream,
                             EGLint attribute,
                             EGLint value);
bool ValidateQueryStreamKHR(ValidationContext *val,
                            const Display *display,
                            const Stream *stream,
                            EGLenum attribute,
                            EGLint *value);
bool ValidateQueryStreamu64KHR(ValidationContext *val,
                               const Display *display,
                               const Stream *stream,
                               EGLenum attribute,
                               EGLuint64KHR *value);
bool ValidateStreamConsumerGLTextureExternalKHR(ValidationContext *val,
                                                const Display *display,
                                                gl::Context *context,
                                                const Stream *stream);
bool ValidateStreamConsumerAcquireKHR(ValidationContext *val,
                                      const Display *display,
                                      gl::Context *context,
                                      const Stream *stream);
bool ValidateStreamConsumerReleaseKHR(ValidationContext *val,
                                      const Display *display,
                                      gl::Context *context,
                                      const Stream *stream);
bool ValidateStreamConsumerGLTextureExternalAttribsNV(ValidationContext *val,
                                                      const Display *display,
                                                      gl::Context *context,
                                                      const Stream *stream,
                                                      const AttributeMap &attribs);
bool ValidateCreateStreamProducerD3DTextureANGLE(ValidationContext *val,
                                                 const Display *display,
                                                 const Stream *stream,
                                                 const AttributeMap &attribs);
bool ValidateStreamPostD3DTextureANGLE(ValidationContext *val,
                                       const Display *display,
                                       const Stream *stream,
                                       void *texture,
                                       const AttributeMap &attribs);

bool ValidateGetMscRateANGLE(ValidationContext *val,
                             const Display *display,
                             const Surface *surface,
                             const EGLint *numerator,
                             const EGLint *denominator);
bool ValidateGetSyncValuesCHROMIUM(ValidationContext *val,
                                   const Display *display,
                                   const Surface *surface,
                                   const EGLuint64KHR *ust,
                                   const EGLuint64KHR *msc,
                                   const EGLuint64KHR *sbc);

bool ValidateDestroySurface(ValidationContext *val,
                            const Display *display,
                            const Surface *surface,
                            const EGLSurface eglSurface);

bool ValidateDestroyContext(ValidationContext *val,
                            const Display *display,
                            const gl::Context *glCtx,
                            const EGLContext eglCtx);

bool ValidateSwapBuffers(ValidationContext *val,
                         Thread *thread,
                         const Display *display,
                         const Surface *surface);

bool ValidateWaitNative(ValidationContext *val, const Display *display, const EGLint engine);

bool ValidateCopyBuffers(ValidationContext *val, Display *display, const Surface *surface);

bool ValidateSwapBuffersWithDamageKHR(ValidationContext *val,
                                      const Display *display,
                                      const Surface *surface,
                                      EGLint *rects,
                                      EGLint n_rects);

bool ValidateBindTexImage(ValidationContext *val,
                          const Display *display,
                          const Surface *surface,
                          const EGLSurface eglSurface,
                          const EGLint buffer,
                          const gl::Context *context,
                          gl::Texture **textureObject);

bool ValidateReleaseTexImage(ValidationContext *val,
                             const Display *display,
                             const Surface *surface,
                             const EGLSurface eglSurface,
                             const EGLint buffer);

bool ValidateSwapInterval(ValidationContext *val,
                          const Display *display,
                          const Surface *draw_surface,
                          const gl::Context *context);

bool ValidateBindAPI(ValidationContext *val, const EGLenum api);

bool ValidatePresentationTimeANDROID(ValidationContext *val,
                                     const Display *display,
                                     const Surface *surface,
                                     EGLnsecsANDROID time);

bool ValidateSetBlobCacheANDROID(ValidationContext *val,
                                 const Display *display,
                                 EGLSetBlobFuncANDROID set,
                                 EGLGetBlobFuncANDROID get);

bool ValidateGetConfigAttrib(ValidationContext *val,
                             const Display *display,
                             const Config *config,
                             EGLint attribute);
bool ValidateChooseConfig(ValidationContext *val,
                          const Display *display,
                          const AttributeMap &attribs,
                          EGLint configSize,
                          EGLint *numConfig);
bool ValidateGetConfigs(ValidationContext *val,
                        const Display *display,
                        EGLint configSize,
                        EGLint *numConfig);

// Other validation
bool ValidateCompatibleSurface(ValidationContext *val,
                               const Display *display,
                               gl::Context *context,
                               const Surface *surface);

bool ValidateGetPlatformDisplay(ValidationContext *val,
                                EGLenum platform,
                                void *native_display,
                                const EGLAttrib *attrib_list);
bool ValidateGetPlatformDisplayEXT(ValidationContext *val,
                                   EGLenum platform,
                                   void *native_display,
                                   const EGLint *attrib_list);
bool ValidateCreatePlatformWindowSurfaceEXT(ValidationContext *val,
                                            const Display *display,
                                            const Config *configuration,
                                            void *nativeWindow,
                                            const AttributeMap &attributes);
bool ValidateCreatePlatformPixmapSurfaceEXT(ValidationContext *val,
                                            const Display *display,
                                            const Config *configuration,
                                            void *nativePixmap,
                                            const AttributeMap &attributes);

bool ValidateProgramCacheGetAttribANGLE(ValidationContext *val,
                                        const Display *display,
                                        EGLenum attrib);

bool ValidateProgramCacheQueryANGLE(ValidationContext *val,
                                    const Display *display,
                                    EGLint index,
                                    void *key,
                                    EGLint *keysize,
                                    void *binary,
                                    EGLint *binarysize);

bool ValidateProgramCachePopulateANGLE(ValidationContext *val,
                                       const Display *display,
                                       const void *key,
                                       EGLint keysize,
                                       const void *binary,
                                       EGLint binarysize);

bool ValidateProgramCacheResizeANGLE(ValidationContext *val,
                                     const Display *display,
                                     EGLint limit,
                                     EGLenum mode);

bool ValidateSurfaceAttrib(ValidationContext *val,
                           const Display *display,
                           const Surface *surface,
                           EGLint attribute,
                           EGLint value);
bool ValidateQuerySurface(ValidationContext *val,
                          const Display *display,
                          const Surface *surface,
                          EGLint attribute,
                          EGLint *value);
bool ValidateQueryContext(ValidationContext *val,
                          const Display *display,
                          const gl::Context *context,
                          EGLint attribute,
                          EGLint *value);

// EGL_KHR_debug
bool ValidateDebugMessageControlKHR(ValidationContext *val,
                                    EGLDEBUGPROCKHR callback,
                                    const AttributeMap &attribs);

bool ValidateQueryDebugKHR(ValidationContext *val, EGLint attribute, EGLAttrib *value);

bool ValidateLabelObjectKHR(ValidationContext *val,
                            Thread *thread,
                            const Display *display,
                            ObjectType objectType,
                            EGLObjectKHR object,
                            EGLLabelKHR label);

// ANDROID_get_frame_timestamps
bool ValidateGetCompositorTimingSupportedANDROID(ValidationContext *val,
                                                 const Display *display,
                                                 const Surface *surface,
                                                 CompositorTiming name);

bool ValidateGetCompositorTimingANDROID(ValidationContext *val,
                                        const Display *display,
                                        const Surface *surface,
                                        EGLint numTimestamps,
                                        const EGLint *names,
                                        EGLnsecsANDROID *values);

bool ValidateGetNextFrameIdANDROID(ValidationContext *val,
                                   const Display *display,
                                   const Surface *surface,
                                   EGLuint64KHR *frameId);

bool ValidateGetFrameTimestampSupportedANDROID(ValidationContext *val,
                                               const Display *display,
                                               const Surface *surface,
                                               Timestamp timestamp);

bool ValidateGetFrameTimestampsANDROID(ValidationContext *val,
                                       const Display *display,
                                       const Surface *surface,
                                       EGLuint64KHR frameId,
                                       EGLint numTimestamps,
                                       const EGLint *timestamps,
                                       EGLnsecsANDROID *values);

bool ValidateQueryStringiANGLE(ValidationContext *val,
                               const Display *display,
                               EGLint name,
                               EGLint index);

bool ValidateQueryDisplayAttribEXT(ValidationContext *val,
                                   const Display *display,
                                   const EGLint attribute);
bool ValidateQueryDisplayAttribANGLE(ValidationContext *val,
                                     const Display *display,
                                     const EGLint attribute);

// EGL_ANDROID_get_native_client_buffer
bool ValidateGetNativeClientBufferANDROID(ValidationContext *val,
                                          const struct AHardwareBuffer *buffer);

// EGL_ANDROID_create_native_client_buffer
bool ValidateCreateNativeClientBufferANDROID(ValidationContext *val,
                                             const egl::AttributeMap &attribMap);

// EGL_ANDROID_native_fence_sync
bool ValidateDupNativeFenceFDANDROID(ValidationContext *val,
                                     const Display *display,
                                     const Sync *sync);

// EGL_ANGLE_swap_with_frame_token
bool ValidateSwapBuffersWithFrameTokenANGLE(ValidationContext *val,
                                            const Display *display,
                                            const Surface *surface,
                                            EGLFrameTokenANGLE frametoken);

// EGL_KHR_reusable_sync
bool ValidateSignalSyncKHR(ValidationContext *val,
                           const Display *display,
                           const Sync *sync,
                           EGLint mode);

// EGL_ANGLE_query_surface_pointer
bool ValidateQuerySurfacePointerANGLE(ValidationContext *val,
                                      const Display *display,
                                      Surface *eglSurface,
                                      EGLint attribute,
                                      void **value);

// EGL_NV_post_sub_buffer
bool ValidatePostSubBufferNV(ValidationContext *val,
                             Display *display,
                             Surface *eglSurface,
                             EGLint x,
                             EGLint y,
                             EGLint width,
                             EGLint height);

// EGL_EXT_device_query
bool ValidateQueryDeviceAttribEXT(ValidationContext *val,
                                  Device *device,
                                  EGLint attribute,
                                  EGLAttrib *value);
bool ValidateQueryDeviceStringEXT(ValidationContext *val, Device *device, EGLint name);

bool ValidateReleaseHighPowerGPUANGLE(ValidationContext *val,
                                      Display *display,
                                      gl::Context *context);
bool ValidateReacquireHighPowerGPUANGLE(ValidationContext *val,
                                        Display *display,
                                        gl::Context *context);
bool ValidateHandleGPUSwitchANGLE(ValidationContext *val, Display *display);

}  // namespace egl

#define ANGLE_EGL_VALIDATE(THREAD, EP, OBJ, RETVAL, ...)           \
    do                                                             \
    {                                                              \
        const char *epname = "egl" #EP;                            \
        ValidationContext vctx(THREAD, epname, OBJ);               \
        auto ANGLE_LOCAL_VAR = (Validate##EP(&vctx, __VA_ARGS__)); \
        if (!ANGLE_LOCAL_VAR)                                      \
        {                                                          \
            return RETVAL;                                         \
        }                                                          \
    } while (0)

#define ANGLE_EGL_VALIDATE_VOID(THREAD, EP, OBJ, ...)              \
    do                                                             \
    {                                                              \
        const char *epname = "egl" #EP;                            \
        ValidationContext vctx(THREAD, epname, OBJ);               \
        auto ANGLE_LOCAL_VAR = (Validate##EP(&vctx, __VA_ARGS__)); \
        if (!ANGLE_LOCAL_VAR)                                      \
        {                                                          \
            return;                                                \
        }                                                          \
    } while (0)

#define ANGLE_EGL_TRY(THREAD, EXPR, FUNCNAME, LABELOBJECT)                   \
    do                                                                       \
    {                                                                        \
        auto ANGLE_LOCAL_VAR = (EXPR);                                       \
        if (ANGLE_LOCAL_VAR.isError())                                       \
            return THREAD->setError(ANGLE_LOCAL_VAR, FUNCNAME, LABELOBJECT); \
    } while (0)

#define ANGLE_EGL_TRY_RETURN(THREAD, EXPR, FUNCNAME, LABELOBJECT, RETVAL) \
    do                                                                    \
    {                                                                     \
        auto ANGLE_LOCAL_VAR = (EXPR);                                    \
        if (ANGLE_LOCAL_VAR.isError())                                    \
        {                                                                 \
            THREAD->setError(ANGLE_LOCAL_VAR, FUNCNAME, LABELOBJECT);     \
            return RETVAL;                                                \
        }                                                                 \
    } while (0)

#endif  // LIBANGLE_VALIDATIONEGL_H_
