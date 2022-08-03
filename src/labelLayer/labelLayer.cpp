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

#include <android/log.h>
#include <string.h>
#include <string>
#include "angle_gl.h"

#define xstr(a) str(a)
#define str(a) #a

#define LAYERNAME "liblabel_layer.so"
#define LOG_TAG "glesLayer" xstr(LAYERNAME)

#define ALOGI(msg, ...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, (msg), __VA_ARGS__)

namespace
{

typedef void *(*PFNEGLGETNEXTLAYERPROCADDRESS)(void *, const char *);

PFNEGLGETNEXTLAYERPROCADDRESS g_next_layer_proc_addr = nullptr;
void *g_layer_id                                     = nullptr;

}  // anonymous namespace

extern "C" {

typedef void (*PFNBINDTEXTUREPROCADDRESS)(GLenum, GLuint);
typedef void (*PFNLABELOBJECTEXTPROCADDRESS)(GLenum, GLuint, GLsizei, const GLchar *);

PFNBINDTEXTUREPROCADDRESS nextBindTexture       = nullptr;
PFNLABELOBJECTEXTPROCADDRESS nextLabelObjectEXT = nullptr;

void hookedBindTexture(GLenum target, GLuint texture)
{
    nextBindTexture(target, texture);
    if (nextLabelObjectEXT && (texture != 0))
    {
        // Build string for texture label
        std::string label = "Texture " + std::to_string(texture);
        ALOGI("%s%d%s%s%s%llu%s",
              "Inside hookedBindTexture -- now calling nextLabelObject function with these args: (",
              texture, " -- ", label.c_str(), " -- ", (unsigned long long)nextLabelObjectEXT, ")");
        nextLabelObjectEXT(GL_TEXTURE, texture, 0, label.c_str());
    }
};

__attribute__((visibility("default"))) void AndroidGLESLayer_Initialize(
    void *layer_id,
    PFNEGLGETNEXTLAYERPROCADDRESS get_next_layer_proc_address)
{

    ALOGI("%s%llu%s%llu", "AndroidGLESLayer_Initialize called with layer_id (",
          (unsigned long long)layer_id, ") get_next_layer_proc_address (",
          (unsigned long long)get_next_layer_proc_address);

    const char *func   = "glLabelObjectEXT";
    void *gpa          = get_next_layer_proc_address(layer_id, func);
    nextLabelObjectEXT = reinterpret_cast<PFNLABELOBJECTEXTPROCADDRESS>(gpa);

    g_layer_id             = layer_id;
    g_next_layer_proc_addr = get_next_layer_proc_address;
}

__attribute__((visibility("default"))) void *AndroidGLESLayer_GetProcAddress(const char *name,
                                                                             void *next)
{

    ALOGI("%s%s%s%llu%s", "Setting up glesLayer version of ", name, " calling down with: next (",
          (unsigned long long)next, ")");

    if (strcmp(name, "glBindTexture") == 0)
    {
        ALOGI("%s%llu%s",
              "Function is glBindTexture, setting up our hook -- calling down with: next (",
              (unsigned long long)hookedBindTexture, ")");
        nextBindTexture = reinterpret_cast<PFNBINDTEXTUREPROCADDRESS>(next);
        return (void *)hookedBindTexture;
    }
    return next;
}

}  // extern "C"
