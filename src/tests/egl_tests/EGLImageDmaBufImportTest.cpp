//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// EGLImageDmaBufImportTest.cpp:
//   Unit tests for the EGL_EXT_image_dma_buf_import and EGL_EXT_image_dma_buf_import_modifiers
//   extensions.
//

#include <gtest/gtest.h>

#include "common/debug.h"
#include "linux-dmabuf-unstable-v1-client-protocol.h"
#include "test_utils/ANGLETest.h"
#include "xdg-shell-client-protocol.h"

#include <fcntl.h>
#include <gbm.h>
#include <libdrm/drm_fourcc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <wayland-client.h>
#include <xf86drm.h>

using namespace angle;

namespace
{
constexpr char kDrmRenderNode[] = "/dev/dr/renderD128";
constexpr uint32_t kDrmFormat   = DRM_FORMAT_XRGB8888;
constexpr uint32_t kWidth       = 16;
constexpr uint32_t kHeight      = 8;

struct PlaneInfo
{
    int fd          = -1;
    uint32_t stride = 0;
    uint32_t offset = 0;
};

struct WaylandData
{
    // Wayland display
    wl_display *wlDisplay       = nullptr;
    wl_registry *wlRegistry     = nullptr;
    wl_compositor *wlCompositor = nullptr;
    xdg_wm_base *xdgWmBase      = nullptr;
    // zwp_fullscreen_shell_v1 *wpShell = nullptr;
    zwp_linux_dmabuf_v1 *wpDmabuf = nullptr;
    bool formatSupported          = false;
    std::vector<uint64_t> modifiers;

    // Gbm
    int drmFd             = -1;
    gbm_device *gbmDevice = nullptr;

    // Wayland window
    wl_surface *wlSurface     = nullptr;
    xdg_surface *xdgSurface   = nullptr;
    xdg_toplevel *xdgToplevel = nullptr;
    wl_callback *wlCallback   = nullptr;

    // DMA buf
    wl_buffer *wlBuffer;
    gbm_bo *gbmBo;
    std::vector<PlaneInfo> bufferPlanes;
    uint64_t bufferModifier = 0;
    // zwp_linux_buffer_release_v1 *wpBufferRelease = nullptr;
};

class EGLImageDmaBufImportTest : public ANGLETest
{
  protected:
    EGLImageDmaBufImportTest() {}

    void testSetUp() override;
    void testTearDown() override;

    WaylandData mWl;

    // EGL
    EGLDisplay mDisplay       = EGL_NO_DISPLAY;
    EGLSurface mWindowSurface = EGL_NO_SURFACE;
    EGLContext mContext       = EGL_NO_CONTEXT;
    EGLConfig mConfig         = nullptr;
};

void DmabufModifiers(void *data,
                     zwp_linux_dmabuf_v1 *zwp_linux_dmabuf,
                     uint32_t format,
                     uint32_t modifier_hi,
                     uint32_t modifier_lo)
{
    WaylandData *wl   = static_cast<WaylandData *>(data);
    uint64_t modifier = static_cast<uint64_t>(modifier_hi) << 32 | modifier_lo;

    if (format != kDrmFormat)
    {
        return;
    }

    wl->formatSupported = true;

    if (modifier != DRM_FORMAT_MOD_INVALID)
    {
        wl->modifiers.push_back(modifier);
    }
}

void DmabufFormat(void *data, zwp_linux_dmabuf_v1 *zwp_linux_dmabuf, uint32_t format) {}

constexpr zwp_linux_dmabuf_v1_listener kDmabufListener = {DmabufFormat, DmabufModifiers};

void XdgWmBasePing(void *data, xdg_wm_base *wmBase, uint32_t serial)
{
    xdg_wm_base_pong(wmBase, serial);
}

constexpr xdg_wm_base_listener kXdgWmBaseListener = {
    XdgWmBasePing,
};

void RegistryHandleGlobal(void *data,
                          wl_registry *registry,
                          uint32_t id,
                          const char *interface,
                          uint32_t version)
{
    WaylandData *wl = static_cast<WaylandData *>(data);

    if (strcmp(interface, "wl_compositor") == 0)
    {
        wl->wlCompositor = static_cast<wl_compositor *>(
            wl_registry_bind(registry, id, &wl_compositor_interface, 1));
    }
    else if (strcmp(interface, "xdg_wm_base") == 0)
    {
        wl->xdgWmBase =
            static_cast<xdg_wm_base *>(wl_registry_bind(registry, id, &xdg_wm_base_interface, 1));
        xdg_wm_base_add_listener(wl->xdgWmBase, &kXdgWmBaseListener, wl);
    }
#if 0
    else if (strcmp(interface, "zwp_fullscreen_shell_v1") == 0)
    {
        wl->wpShell = wl_registry_bind(registry, id, &zwp_fullscreen_shell_v1_interface, 1);
    }
#endif
    else if (strcmp(interface, "zwp_linux_dmabuf_v1") == 0)
    {
        if (version < 3)
            return;
        wl->wpDmabuf = static_cast<zwp_linux_dmabuf_v1 *>(
            wl_registry_bind(registry, id, &zwp_linux_dmabuf_v1_interface, 3));
        zwp_linux_dmabuf_v1_add_listener(wl->wpDmabuf, &kDmabufListener, wl);
    }
#if 0
    else if (strcmp(interface, "zwp_linux_explicit_synchronization_v1") == 0)
    {
        wl->explicit_sync = wl_registry_bind(
                registry, id,
                &zwp_linux_explicit_synchronization_v1_interface, 1);
    }
    else if (strcmp(interface, "weston_direct_display_v1") == 0)
    {
        wl->direct_display = wl_registry_bind(registry,
                id, &weston_direct_display_v1_interface, 1);
    }
#endif
}

void RegistryHandleGlobalRemove(void *data, wl_registry *registry, uint32_t name) {}

constexpr wl_registry_listener kRegistryListener = {
    RegistryHandleGlobal,
    RegistryHandleGlobalRemove,
};

void XdgSurfaceHandleConfigure(void *data, xdg_surface *surface, uint32_t serial)
{
    // WaylandData *wl = static_cast<WaylandData *>(data);

    xdg_surface_ack_configure(surface, serial);

#if 0
    if (window->initialized && window->wait_for_configure)
        redraw(window, NULL, 0);
    window->wait_for_configure = false;
#endif
}

constexpr xdg_surface_listener kXdgSurfaceListener = {
    XdgSurfaceHandleConfigure,
};

void XdgToplevelHandleConfigure(void *data,
                                xdg_toplevel *toplevel,
                                int32_t width,
                                int32_t height,
                                wl_array *states)
{}

void XdgToplevelHandleClose(void *data, xdg_toplevel *toplevel) {}

constexpr xdg_toplevel_listener kXdgToplevelListener = {
    XdgToplevelHandleConfigure,
    XdgToplevelHandleClose,
};

void BufferRelease(void *data, wl_buffer *buffer)
{
    // WaylandData *wl = static_cast<WaylandData *>(data);

    // wl->bufferBusy = false;
}

constexpr wl_buffer_listener kBufferListener = {
    BufferRelease,
};

void CreateSucceeded(void *data, zwp_linux_buffer_params_v1 *params, wl_buffer *new_buffer)
{
    WaylandData *wl = static_cast<WaylandData *>(data);

    wl->wlBuffer = new_buffer;
    wl_buffer_add_listener(wl->wlBuffer, &kBufferListener, wl);

    zwp_linux_buffer_params_v1_destroy(params);
}

void CreateFailed(void *data, zwp_linux_buffer_params_v1 *params)
{
    WaylandData *wl = static_cast<WaylandData *>(data);
    wl->wlBuffer    = NULL;
    zwp_linux_buffer_params_v1_destroy(params);

    UNREACHABLE();
}

constexpr zwp_linux_buffer_params_v1_listener kParamsListener = {
    CreateSucceeded,
    CreateFailed,
};

void EGLImageDmaBufImportTest::testSetUp()
{
    // Set up Wayland display.
    mWl.wlDisplay = wl_display_connect(nullptr);
    if (mWl.wlDisplay == nullptr)
    {
        return;
    }

    mWl.wlRegistry = wl_display_get_registry(mWl.wlDisplay);
    ASSERT(mWl.wlRegistry != nullptr);

    wl_registry_add_listener(mWl.wlRegistry, &kRegistryListener, &mWl);
    wl_display_roundtrip(mWl.wlDisplay);
    ASSERT(mWl.wpDmabuf != nullptr);

    if (!mWl.formatSupported)
    {
        return;
    }

    // Set up GBM display.
    mWl.drmFd = open(kDrmRenderNode, O_RDWR);
    if (mWl.drmFd < 0)
    {
        return;
    }

    mWl.gbmDevice = gbm_create_device(mWl.drmFd);
    ASSERT(mWl.gbmDevice != NULL);

    // Set up window
    mWl.wlSurface = wl_compositor_create_surface(mWl.wlCompositor);

    if (mWl.xdgWmBase)
    {
        mWl.xdgSurface = xdg_wm_base_get_xdg_surface(mWl.xdgWmBase, mWl.wlSurface);
        ASSERT(mWl.xdgSurface != nullptr);

        xdg_surface_add_listener(mWl.xdgSurface, &kXdgSurfaceListener, &mWl);

        mWl.xdgToplevel = xdg_surface_get_toplevel(mWl.xdgSurface);
        ASSERT(mWl.xdgToplevel);

        xdg_toplevel_add_listener(mWl.xdgToplevel, &kXdgToplevelListener, &mWl);

        xdg_toplevel_set_title(mWl.xdgToplevel, "Dma_buf test");

        // window->wait_for_configure = true;
        wl_surface_commit(mWl.wlSurface);
    }
#if 0
    else if (mWl.wpShell)
    {
        // TODO: don't need to support fullscreen
        zwp_fullscreen_shell_v1_present_surface(
            mWl.wpShell, mWl.wlSurface, ZWP_FULLSCREEN_SHELL_V1_PRESENT_METHOD_DEFAULT, nullptr);
    }
#endif
    else
    {
        ASSERT(false);
    }

    // Create a buffer object
    if (!mWl.modifiers.empty())
    {
        mWl.gbmBo = gbm_bo_create_with_modifiers(mWl.gbmDevice, kWidth, kHeight, kDrmFormat,
                                                 mWl.modifiers.data(), mWl.modifiers.size());
        if (mWl.gbmBo)
            mWl.bufferModifier = gbm_bo_get_modifier(mWl.gbmBo);
    }

    if (mWl.gbmBo == nullptr)
    {
        mWl.gbmBo = gbm_bo_create(mWl.gbmDevice, kWidth, kHeight, kDrmFormat, GBM_BO_USE_RENDERING);
        mWl.bufferModifier = DRM_FORMAT_MOD_INVALID;
    }

    ASSERT(mWl.gbmBo != nullptr);

    zwp_linux_buffer_params_v1 *params = zwp_linux_dmabuf_v1_create_params(mWl.wpDmabuf);

    // Get plane data from the buffer
    const uint32_t planeCount = gbm_bo_get_plane_count(mWl.gbmBo);
    mWl.bufferPlanes.resize(planeCount);
    for (uint32_t plane = 0; plane < planeCount; ++plane)
    {
        gbm_bo_handle handle = gbm_bo_get_handle_for_plane(mWl.gbmBo, plane);
        ASSERT(handle.s32 != -1);

        int ret = drmPrimeHandleToFD(mWl.drmFd, handle.u32, 0, &mWl.bufferPlanes[plane].fd);
        ASSERT(ret >= 0 && mWl.bufferPlanes[plane].fd >= 0);

        mWl.bufferPlanes[plane].stride = gbm_bo_get_stride_for_plane(mWl.gbmBo, plane);
        mWl.bufferPlanes[plane].offset = gbm_bo_get_offset(mWl.gbmBo, plane);

        zwp_linux_buffer_params_v1_add(params, mWl.bufferPlanes[plane].fd, plane,
                                       mWl.bufferPlanes[plane].offset,
                                       mWl.bufferPlanes[plane].stride, mWl.bufferModifier >> 32,
                                       mWl.bufferModifier & 0xFFFFFFFF);
    }

    zwp_linux_buffer_params_v1_add_listener(params, &kParamsListener, &mWl);

    zwp_linux_buffer_params_v1_create(params, kWidth, kHeight, kDrmFormat, 0);

    // TODO: from here: set up EGL display
}

void EGLImageDmaBufImportTest::testTearDown()
{
    // TODO
}

// Verify config matching is working.
TEST_P(EGLImageDmaBufImportTest, MatchFramebufferTargetConfigs)
{
    ANGLE_SKIP_TEST_IF(!IsEGLDisplayExtensionEnabled(mDisplay, "EGL_EXT_image_dma_buf_import"));
    ANGLE_SKIP_TEST_IF(
        !IsEGLDisplayExtensionEnabled(mDisplay, "EGL_EXT_image_dma_buf_import_modifiers"));
    ANGLE_SKIP_TEST_IF(mWl.wlDisplay == nullptr);

    // TODO
}
}  // anonymous namespace

ANGLE_INSTANTIATE_TEST(EGLImageDmaBufImportTest,
                       WithNoFixture(ES3_OPENGL()),
                       WithNoFixture(ES3_VULKAN()));
