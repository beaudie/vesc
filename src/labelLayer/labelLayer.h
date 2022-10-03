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
#include <inttypes.h>
#include <string.h>
#include <string>
#include <unordered_map>
#include "angle_gl.h"

#define LOG_TAG "glesLayer(liblabel_layer.so)"
#define ALOGI(msg, ...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, (msg), __VA_ARGS__)

namespace agi_layer
{
extern std::unordered_map<std::string, void *> funcMap;
extern std::unordered_map<std::string, void *> hookMap;
extern PFNGLDEBUGMESSAGEINSERTKHRPROC g_debug_message_insert;
extern PFNGLPUSHDEBUGGROUPKHRPROC g_push_debug_group;
extern PFNGLPOPDEBUGGROUPKHRPROC g_pop_debug_group;

}  //  namespace agi_layer
