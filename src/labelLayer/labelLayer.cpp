/*
 * Copyright (C) 2022 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "../src/labelLayer/labelLayer.h"
#include <unordered_set>
#include "libGLESv2/layer_entry_points_cl_autogen.h"
#include "libGLESv2/layer_entry_points_egl_autogen.h"
#include "libGLESv2/layer_entry_points_egl_ext_autogen.h"
#include "libGLESv2/layer_entry_points_gles_1_0_autogen.h"
#include "libGLESv2/layer_entry_points_gles_2_0_autogen.h"
#include "libGLESv2/layer_entry_points_gles_3_0_autogen.h"
#include "libGLESv2/layer_entry_points_gles_3_1_autogen.h"
#include "libGLESv2/layer_entry_points_gles_3_2_autogen.h"
#include "libGLESv2/layer_entry_points_gles_ext_autogen.h"

namespace agi_layer
{
std::unordered_map<std::string, void *> funcMap;
std::unordered_map<std::string, void *> hookMap;

typedef void *(*PFNEGLGETNEXTLAYERPROCADDRESS)(void *, const char *);
PFNEGLGETNEXTLAYERPROCADDRESS g_next_layer_proc_addr  = nullptr;
PFNGLPUSHDEBUGGROUPKHRPROC g_push_debug_group         = nullptr;
PFNGLPOPDEBUGGROUPKHRPROC g_pop_debug_group           = nullptr;
PFNGLDEBUGMESSAGEINSERTKHRPROC g_debug_message_insert = nullptr;
}  // namespace agi_layer

extern "C" {

using namespace agi_layer;

__attribute__((visibility("default"))) void AndroidGLESLayer_Initialize(
    void *layer_id,
    PFNEGLGETNEXTLAYERPROCADDRESS get_next_layer_proc_address)
{
    const char *func            = "glLabelObjectEXT";
    void *gpa                   = get_next_layer_proc_address(layer_id, func);
    funcMap["glLabelObjectEXT"] = gpa;

    func                        = "glObjectLabelKHR";
    gpa                         = get_next_layer_proc_address(layer_id, func);
    funcMap["glObjectLabelKHR"] = gpa;

    func               = "glPushDebugGroupKHR";
    gpa                = get_next_layer_proc_address(layer_id, func);
    g_push_debug_group = reinterpret_cast<PFNGLPUSHDEBUGGROUPKHRPROC>(gpa);
    ALOGI("%s%p", "DDDDD Function is glPushDebugGroupKHR, next ptr is ", (void *)gpa);

    func                   = "glDebugMessageInsertKHR";
    gpa                    = get_next_layer_proc_address(layer_id, func);
    g_debug_message_insert = reinterpret_cast<PFNGLDEBUGMESSAGEINSERTKHRPROC>(gpa);
    ALOGI("%s%p", "DDDDD Function is glDebugMessageInsertKHR, next ptr is ", (void *)gpa);

    func              = "glPopDebugGroupKHR";
    gpa               = get_next_layer_proc_address(layer_id, func);
    g_pop_debug_group = reinterpret_cast<PFNGLPOPDEBUGGROUPKHRPROC>(gpa);
    ALOGI("%s%p", "DDDDD Function is glPopDebugGroupKHR, next ptr is ", (void *)gpa);

    g_next_layer_proc_addr = get_next_layer_proc_address;

    // Set up layer hooks
    // TODO: Examine need for adding hooks on-the-fly. Each EP can check to see if the fcn ptr is
    // null,
    //       use it directly if not else call GPA and add it to the map.
    setupCLHooks();
    setupEGLHooks();
    setupEGL_EXTHooks();
    setupGLES_1_0Hooks();
    setupGLES_2_0Hooks();
    setupGLES_3_0Hooks();
    setupGLES_3_1Hooks();
    setupGLES_3_2Hooks();
    setupGLES_EXTHooks();
}

__attribute__((visibility("default"))) void *AndroidGLESLayer_GetProcAddress(const char *name,
                                                                             void *next)
{
    void *entry   = hookMap[name];
    funcMap[name] = next;

    if (entry)
    {
        return entry;
    }
    return next;
}

}  // extern "C"
