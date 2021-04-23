//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// validationCL.cpp: Validation functions for generic CL entry point parameters

#include "libANGLE/validationCL_autogen.h"

namespace cl
{
// CL 1.0
bool ValidateGetPlatformIDs(cl_uint num_entries,
                            Platform *const *platformsPacked,
                            const cl_uint *num_platforms)
{
    return true;
}

bool ValidateGetPlatformInfo(const Platform *platformPacked,
                             PlatformInfo param_namePacked,
                             size_t param_value_size,
                             const void *param_value,
                             const size_t *param_value_size_ret)
{
    return true;
}

bool ValidateGetDeviceIDs(const Platform *platformPacked,
                          cl_device_type device_type,
                          cl_uint num_entries,
                          Device *const *devicesPacked,
                          const cl_uint *num_devices)
{
    return true;
}

bool ValidateGetDeviceInfo(const Device *devicePacked,
                           DeviceInfo param_namePacked,
                           size_t param_value_size,
                           const void *param_value,
                           const size_t *param_value_size_ret)
{
    return true;
}

bool ValidateCreateContext(const cl_context_properties *properties,
                           cl_uint num_devices,
                           Device *const *devicesPacked,
                           void(CL_CALLBACK *pfn_notify)(const char *errinfo,
                                                         const void *private_info,
                                                         size_t cb,
                                                         void *user_data),
                           const void *user_data,
                           const cl_int *errcode_ret)
{
    return true;
}

bool ValidateCreateContextFromType(const cl_context_properties *properties,
                                   cl_device_type device_type,
                                   void(CL_CALLBACK *pfn_notify)(const char *errinfo,
                                                                 const void *private_info,
                                                                 size_t cb,
                                                                 void *user_data),
                                   const void *user_data,
                                   const cl_int *errcode_ret)
{
    return true;
}

bool ValidateRetainContext(const Context *contextPacked)
{
    return true;
}

bool ValidateReleaseContext(const Context *contextPacked)
{
    return true;
}

bool ValidateGetContextInfo(const Context *contextPacked,
                            ContextInfo param_namePacked,
                            size_t param_value_size,
                            const void *param_value,
                            const size_t *param_value_size_ret)
{
    return true;
}

bool ValidateRetainCommandQueue(const CommandQueue *command_queuePacked)
{
    return true;
}

bool ValidateReleaseCommandQueue(const CommandQueue *command_queuePacked)
{
    return true;
}

bool ValidateGetCommandQueueInfo(const CommandQueue *command_queuePacked,
                                 CommandQueueInfo param_namePacked,
                                 size_t param_value_size,
                                 const void *param_value,
                                 const size_t *param_value_size_ret)
{
    return true;
}

bool ValidateCreateBuffer(const Context *contextPacked,
                          cl_mem_flags flags,
                          size_t size,
                          const void *host_ptr,
                          const cl_int *errcode_ret)
{
    return true;
}

bool ValidateRetainMemObject(const Memory *memobjPacked)
{
    return true;
}

bool ValidateReleaseMemObject(const Memory *memobjPacked)
{
    return true;
}

bool ValidateGetSupportedImageFormats(const Context *contextPacked,
                                      cl_mem_flags flags,
                                      MemObjectType image_typePacked,
                                      cl_uint num_entries,
                                      const cl_image_format *image_formats,
                                      const cl_uint *num_image_formats)
{
    return true;
}

bool ValidateGetMemObjectInfo(const Memory *memobjPacked,
                              MemInfo param_namePacked,
                              size_t param_value_size,
                              const void *param_value,
                              const size_t *param_value_size_ret)
{
    return true;
}

bool ValidateGetImageInfo(const Memory *imagePacked,
                          ImageInfo param_namePacked,
                          size_t param_value_size,
                          const void *param_value,
                          const size_t *param_value_size_ret)
{
    return true;
}

bool ValidateRetainSampler(const Sampler *samplerPacked)
{
    return true;
}

bool ValidateReleaseSampler(const Sampler *samplerPacked)
{
    return true;
}

bool ValidateGetSamplerInfo(const Sampler *samplerPacked,
                            SamplerInfo param_namePacked,
                            size_t param_value_size,
                            const void *param_value,
                            const size_t *param_value_size_ret)
{
    return true;
}

bool ValidateCreateProgramWithSource(const Context *contextPacked,
                                     cl_uint count,
                                     const char **strings,
                                     const size_t *lengths,
                                     const cl_int *errcode_ret)
{
    return true;
}

bool ValidateCreateProgramWithBinary(const Context *contextPacked,
                                     cl_uint num_devices,
                                     Device *const *device_listPacked,
                                     const size_t *lengths,
                                     const unsigned char **binaries,
                                     const cl_int *binary_status,
                                     const cl_int *errcode_ret)
{
    return true;
}

bool ValidateRetainProgram(const Program *programPacked)
{
    return true;
}

bool ValidateReleaseProgram(const Program *programPacked)
{
    return true;
}

bool ValidateBuildProgram(const Program *programPacked,
                          cl_uint num_devices,
                          Device *const *device_listPacked,
                          const char *options,
                          void(CL_CALLBACK *pfn_notify)(cl_program program, void *user_data),
                          const void *user_data)
{
    return true;
}

bool ValidateGetProgramInfo(const Program *programPacked,
                            ProgramInfo param_namePacked,
                            size_t param_value_size,
                            const void *param_value,
                            const size_t *param_value_size_ret)
{
    return true;
}

bool ValidateGetProgramBuildInfo(const Program *programPacked,
                                 const Device *devicePacked,
                                 ProgramBuildInfo param_namePacked,
                                 size_t param_value_size,
                                 const void *param_value,
                                 const size_t *param_value_size_ret)
{
    return true;
}

bool ValidateCreateKernel(const Program *programPacked,
                          const char *kernel_name,
                          const cl_int *errcode_ret)
{
    return true;
}

bool ValidateCreateKernelsInProgram(const Program *programPacked,
                                    cl_uint num_kernels,
                                    Kernel *const *kernelsPacked,
                                    const cl_uint *num_kernels_ret)
{
    return true;
}

bool ValidateRetainKernel(const Kernel *kernelPacked)
{
    return true;
}

bool ValidateReleaseKernel(const Kernel *kernelPacked)
{
    return true;
}

bool ValidateSetKernelArg(const Kernel *kernelPacked,
                          cl_uint arg_index,
                          size_t arg_size,
                          const void *arg_value)
{
    return true;
}

bool ValidateGetKernelInfo(const Kernel *kernelPacked,
                           KernelInfo param_namePacked,
                           size_t param_value_size,
                           const void *param_value,
                           const size_t *param_value_size_ret)
{
    return true;
}

bool ValidateGetKernelWorkGroupInfo(const Kernel *kernelPacked,
                                    const Device *devicePacked,
                                    KernelWorkGroupInfo param_namePacked,
                                    size_t param_value_size,
                                    const void *param_value,
                                    const size_t *param_value_size_ret)
{
    return true;
}

bool ValidateWaitForEvents(cl_uint num_events, Event *const *event_listPacked)
{
    return true;
}

bool ValidateGetEventInfo(const Event *eventPacked,
                          EventInfo param_namePacked,
                          size_t param_value_size,
                          const void *param_value,
                          const size_t *param_value_size_ret)
{
    return true;
}

bool ValidateRetainEvent(const Event *eventPacked)
{
    return true;
}

bool ValidateReleaseEvent(const Event *eventPacked)
{
    return true;
}

bool ValidateGetEventProfilingInfo(const Event *eventPacked,
                                   ProfilingInfo param_namePacked,
                                   size_t param_value_size,
                                   const void *param_value,
                                   const size_t *param_value_size_ret)
{
    return true;
}

bool ValidateFlush(const CommandQueue *command_queuePacked)
{
    return true;
}

bool ValidateFinish(const CommandQueue *command_queuePacked)
{
    return true;
}

bool ValidateEnqueueReadBuffer(const CommandQueue *command_queuePacked,
                               const Memory *bufferPacked,
                               cl_bool blocking_read,
                               size_t offset,
                               size_t size,
                               const void *ptr,
                               cl_uint num_events_in_wait_list,
                               Event *const *event_wait_listPacked,
                               Event *const *eventPacked)
{
    return true;
}

bool ValidateEnqueueWriteBuffer(const CommandQueue *command_queuePacked,
                                const Memory *bufferPacked,
                                cl_bool blocking_write,
                                size_t offset,
                                size_t size,
                                const void *ptr,
                                cl_uint num_events_in_wait_list,
                                Event *const *event_wait_listPacked,
                                Event *const *eventPacked)
{
    return true;
}

bool ValidateEnqueueCopyBuffer(const CommandQueue *command_queuePacked,
                               const Memory *src_bufferPacked,
                               const Memory *dst_bufferPacked,
                               size_t src_offset,
                               size_t dst_offset,
                               size_t size,
                               cl_uint num_events_in_wait_list,
                               Event *const *event_wait_listPacked,
                               Event *const *eventPacked)
{
    return true;
}

bool ValidateEnqueueReadImage(const CommandQueue *command_queuePacked,
                              const Memory *imagePacked,
                              cl_bool blocking_read,
                              const size_t *origin,
                              const size_t *region,
                              size_t row_pitch,
                              size_t slice_pitch,
                              const void *ptr,
                              cl_uint num_events_in_wait_list,
                              Event *const *event_wait_listPacked,
                              Event *const *eventPacked)
{
    return true;
}

bool ValidateEnqueueWriteImage(const CommandQueue *command_queuePacked,
                               const Memory *imagePacked,
                               cl_bool blocking_write,
                               const size_t *origin,
                               const size_t *region,
                               size_t input_row_pitch,
                               size_t input_slice_pitch,
                               const void *ptr,
                               cl_uint num_events_in_wait_list,
                               Event *const *event_wait_listPacked,
                               Event *const *eventPacked)
{
    return true;
}

bool ValidateEnqueueCopyImage(const CommandQueue *command_queuePacked,
                              const Memory *src_imagePacked,
                              const Memory *dst_imagePacked,
                              const size_t *src_origin,
                              const size_t *dst_origin,
                              const size_t *region,
                              cl_uint num_events_in_wait_list,
                              Event *const *event_wait_listPacked,
                              Event *const *eventPacked)
{
    return true;
}

bool ValidateEnqueueCopyImageToBuffer(const CommandQueue *command_queuePacked,
                                      const Memory *src_imagePacked,
                                      const Memory *dst_bufferPacked,
                                      const size_t *src_origin,
                                      const size_t *region,
                                      size_t dst_offset,
                                      cl_uint num_events_in_wait_list,
                                      Event *const *event_wait_listPacked,
                                      Event *const *eventPacked)
{
    return true;
}

bool ValidateEnqueueCopyBufferToImage(const CommandQueue *command_queuePacked,
                                      const Memory *src_bufferPacked,
                                      const Memory *dst_imagePacked,
                                      size_t src_offset,
                                      const size_t *dst_origin,
                                      const size_t *region,
                                      cl_uint num_events_in_wait_list,
                                      Event *const *event_wait_listPacked,
                                      Event *const *eventPacked)
{
    return true;
}

bool ValidateEnqueueMapBuffer(const CommandQueue *command_queuePacked,
                              const Memory *bufferPacked,
                              cl_bool blocking_map,
                              cl_map_flags map_flags,
                              size_t offset,
                              size_t size,
                              cl_uint num_events_in_wait_list,
                              Event *const *event_wait_listPacked,
                              Event *const *eventPacked,
                              const cl_int *errcode_ret)
{
    return true;
}

bool ValidateEnqueueMapImage(const CommandQueue *command_queuePacked,
                             const Memory *imagePacked,
                             cl_bool blocking_map,
                             cl_map_flags map_flags,
                             const size_t *origin,
                             const size_t *region,
                             const size_t *image_row_pitch,
                             const size_t *image_slice_pitch,
                             cl_uint num_events_in_wait_list,
                             Event *const *event_wait_listPacked,
                             Event *const *eventPacked,
                             const cl_int *errcode_ret)
{
    return true;
}

bool ValidateEnqueueUnmapMemObject(const CommandQueue *command_queuePacked,
                                   const Memory *memobjPacked,
                                   const void *mapped_ptr,
                                   cl_uint num_events_in_wait_list,
                                   Event *const *event_wait_listPacked,
                                   Event *const *eventPacked)
{
    return true;
}

bool ValidateEnqueueNDRangeKernel(const CommandQueue *command_queuePacked,
                                  const Kernel *kernelPacked,
                                  cl_uint work_dim,
                                  const size_t *global_work_offset,
                                  const size_t *global_work_size,
                                  const size_t *local_work_size,
                                  cl_uint num_events_in_wait_list,
                                  Event *const *event_wait_listPacked,
                                  Event *const *eventPacked)
{
    return true;
}

bool ValidateEnqueueNativeKernel(const CommandQueue *command_queuePacked,
                                 void(CL_CALLBACK *user_func)(void *),
                                 const void *args,
                                 size_t cb_args,
                                 cl_uint num_mem_objects,
                                 Memory *const *mem_listPacked,
                                 const void **args_mem_loc,
                                 cl_uint num_events_in_wait_list,
                                 Event *const *event_wait_listPacked,
                                 Event *const *eventPacked)
{
    return true;
}

bool ValidateSetCommandQueueProperty(const CommandQueue *command_queuePacked,
                                     cl_command_queue_properties properties,
                                     cl_bool enable,
                                     const cl_command_queue_properties *old_properties)
{
    return true;
}

bool ValidateCreateImage2D(const Context *contextPacked,
                           cl_mem_flags flags,
                           const cl_image_format *image_format,
                           size_t image_width,
                           size_t image_height,
                           size_t image_row_pitch,
                           const void *host_ptr,
                           const cl_int *errcode_ret)
{
    return true;
}

bool ValidateCreateImage3D(const Context *contextPacked,
                           cl_mem_flags flags,
                           const cl_image_format *image_format,
                           size_t image_width,
                           size_t image_height,
                           size_t image_depth,
                           size_t image_row_pitch,
                           size_t image_slice_pitch,
                           const void *host_ptr,
                           const cl_int *errcode_ret)
{
    return true;
}

bool ValidateEnqueueMarker(const CommandQueue *command_queuePacked, Event *const *eventPacked)
{
    return true;
}

bool ValidateEnqueueWaitForEvents(const CommandQueue *command_queuePacked,
                                  cl_uint num_events,
                                  Event *const *event_listPacked)
{
    return true;
}

bool ValidateEnqueueBarrier(const CommandQueue *command_queuePacked)
{
    return true;
}

bool ValidateUnloadCompiler()
{
    return true;
}

bool ValidateGetExtensionFunctionAddress(const char *func_name)
{
    return true;
}

bool ValidateCreateCommandQueue(const Context *contextPacked,
                                const Device *devicePacked,
                                cl_command_queue_properties properties,
                                const cl_int *errcode_ret)
{
    return true;
}

bool ValidateCreateSampler(const Context *contextPacked,
                           cl_bool normalized_coords,
                           AddressingMode addressing_modePacked,
                           FilterMode filter_modePacked,
                           const cl_int *errcode_ret)
{
    return true;
}

bool ValidateEnqueueTask(const CommandQueue *command_queuePacked,
                         const Kernel *kernelPacked,
                         cl_uint num_events_in_wait_list,
                         Event *const *event_wait_listPacked,
                         Event *const *eventPacked)
{
    return true;
}

// CL 1.1
bool ValidateCreateSubBuffer(const Memory *bufferPacked,
                             cl_mem_flags flags,
                             cl_buffer_create_type buffer_create_type,
                             const void *buffer_create_info,
                             const cl_int *errcode_ret)
{
    return true;
}

bool ValidateSetMemObjectDestructorCallback(const Memory *memobjPacked,
                                            void(CL_CALLBACK *pfn_notify)(cl_mem memobj,
                                                                          void *user_data),
                                            const void *user_data)
{
    return true;
}

bool ValidateCreateUserEvent(const Context *contextPacked, const cl_int *errcode_ret)
{
    return true;
}

bool ValidateSetUserEventStatus(const Event *eventPacked, cl_int execution_status)
{
    return true;
}

bool ValidateSetEventCallback(const Event *eventPacked,
                              cl_int command_exec_callback_type,
                              void(CL_CALLBACK *pfn_notify)(cl_event event,
                                                            cl_int event_command_status,
                                                            void *user_data),
                              const void *user_data)
{
    return true;
}

bool ValidateEnqueueReadBufferRect(const CommandQueue *command_queuePacked,
                                   const Memory *bufferPacked,
                                   cl_bool blocking_read,
                                   const size_t *buffer_origin,
                                   const size_t *host_origin,
                                   const size_t *region,
                                   size_t buffer_row_pitch,
                                   size_t buffer_slice_pitch,
                                   size_t host_row_pitch,
                                   size_t host_slice_pitch,
                                   const void *ptr,
                                   cl_uint num_events_in_wait_list,
                                   Event *const *event_wait_listPacked,
                                   Event *const *eventPacked)
{
    return true;
}

bool ValidateEnqueueWriteBufferRect(const CommandQueue *command_queuePacked,
                                    const Memory *bufferPacked,
                                    cl_bool blocking_write,
                                    const size_t *buffer_origin,
                                    const size_t *host_origin,
                                    const size_t *region,
                                    size_t buffer_row_pitch,
                                    size_t buffer_slice_pitch,
                                    size_t host_row_pitch,
                                    size_t host_slice_pitch,
                                    const void *ptr,
                                    cl_uint num_events_in_wait_list,
                                    Event *const *event_wait_listPacked,
                                    Event *const *eventPacked)
{
    return true;
}

bool ValidateEnqueueCopyBufferRect(const CommandQueue *command_queuePacked,
                                   const Memory *src_bufferPacked,
                                   const Memory *dst_bufferPacked,
                                   const size_t *src_origin,
                                   const size_t *dst_origin,
                                   const size_t *region,
                                   size_t src_row_pitch,
                                   size_t src_slice_pitch,
                                   size_t dst_row_pitch,
                                   size_t dst_slice_pitch,
                                   cl_uint num_events_in_wait_list,
                                   Event *const *event_wait_listPacked,
                                   Event *const *eventPacked)
{
    return true;
}

// CL 1.2
bool ValidateCreateSubDevices(const Device *in_devicePacked,
                              const cl_device_partition_property *properties,
                              cl_uint num_devices,
                              Device *const *out_devicesPacked,
                              const cl_uint *num_devices_ret)
{
    return true;
}

bool ValidateRetainDevice(const Device *devicePacked)
{
    return true;
}

bool ValidateReleaseDevice(const Device *devicePacked)
{
    return true;
}

bool ValidateCreateImage(const Context *contextPacked,
                         cl_mem_flags flags,
                         const cl_image_format *image_format,
                         const cl_image_desc *image_desc,
                         const void *host_ptr,
                         const cl_int *errcode_ret)
{
    return true;
}

bool ValidateCreateProgramWithBuiltInKernels(const Context *contextPacked,
                                             cl_uint num_devices,
                                             Device *const *device_listPacked,
                                             const char *kernel_names,
                                             const cl_int *errcode_ret)
{
    return true;
}

bool ValidateCompileProgram(const Program *programPacked,
                            cl_uint num_devices,
                            Device *const *device_listPacked,
                            const char *options,
                            cl_uint num_input_headers,
                            Program *const *input_headersPacked,
                            const char **header_include_names,
                            void(CL_CALLBACK *pfn_notify)(cl_program program, void *user_data),
                            const void *user_data)
{
    return true;
}

bool ValidateLinkProgram(const Context *contextPacked,
                         cl_uint num_devices,
                         Device *const *device_listPacked,
                         const char *options,
                         cl_uint num_input_programs,
                         Program *const *input_programsPacked,
                         void(CL_CALLBACK *pfn_notify)(cl_program program, void *user_data),
                         const void *user_data,
                         const cl_int *errcode_ret)
{
    return true;
}

bool ValidateUnloadPlatformCompiler(const Platform *platformPacked)
{
    return true;
}

bool ValidateGetKernelArgInfo(const Kernel *kernelPacked,
                              cl_uint arg_index,
                              KernelArgInfo param_namePacked,
                              size_t param_value_size,
                              const void *param_value,
                              const size_t *param_value_size_ret)
{
    return true;
}

bool ValidateEnqueueFillBuffer(const CommandQueue *command_queuePacked,
                               const Memory *bufferPacked,
                               const void *pattern,
                               size_t pattern_size,
                               size_t offset,
                               size_t size,
                               cl_uint num_events_in_wait_list,
                               Event *const *event_wait_listPacked,
                               Event *const *eventPacked)
{
    return true;
}

bool ValidateEnqueueFillImage(const CommandQueue *command_queuePacked,
                              const Memory *imagePacked,
                              const void *fill_color,
                              const size_t *origin,
                              const size_t *region,
                              cl_uint num_events_in_wait_list,
                              Event *const *event_wait_listPacked,
                              Event *const *eventPacked)
{
    return true;
}

bool ValidateEnqueueMigrateMemObjects(const CommandQueue *command_queuePacked,
                                      cl_uint num_mem_objects,
                                      Memory *const *mem_objectsPacked,
                                      cl_mem_migration_flags flags,
                                      cl_uint num_events_in_wait_list,
                                      Event *const *event_wait_listPacked,
                                      Event *const *eventPacked)
{
    return true;
}

bool ValidateEnqueueMarkerWithWaitList(const CommandQueue *command_queuePacked,
                                       cl_uint num_events_in_wait_list,
                                       Event *const *event_wait_listPacked,
                                       Event *const *eventPacked)
{
    return true;
}

bool ValidateEnqueueBarrierWithWaitList(const CommandQueue *command_queuePacked,
                                        cl_uint num_events_in_wait_list,
                                        Event *const *event_wait_listPacked,
                                        Event *const *eventPacked)
{
    return true;
}

bool ValidateGetExtensionFunctionAddressForPlatform(const Platform *platformPacked,
                                                    const char *func_name)
{
    return true;
}

// CL 2.0
bool ValidateCreateCommandQueueWithProperties(const Context *contextPacked,
                                              const Device *devicePacked,
                                              const cl_queue_properties *properties,
                                              const cl_int *errcode_ret)
{
    return true;
}

bool ValidateCreatePipe(const Context *contextPacked,
                        cl_mem_flags flags,
                        cl_uint pipe_packet_size,
                        cl_uint pipe_max_packets,
                        const cl_pipe_properties *properties,
                        const cl_int *errcode_ret)
{
    return true;
}

bool ValidateGetPipeInfo(const Memory *pipePacked,
                         PipeInfo param_namePacked,
                         size_t param_value_size,
                         const void *param_value,
                         const size_t *param_value_size_ret)
{
    return true;
}

bool ValidateSVMAlloc(const Context *contextPacked,
                      cl_svm_mem_flags flags,
                      size_t size,
                      cl_uint alignment)
{
    return true;
}

bool ValidateSVMFree(const Context *contextPacked, const void *svm_pointer)
{
    return true;
}

bool ValidateCreateSamplerWithProperties(const Context *contextPacked,
                                         const cl_sampler_properties *sampler_properties,
                                         const cl_int *errcode_ret)
{
    return true;
}

bool ValidateSetKernelArgSVMPointer(const Kernel *kernelPacked,
                                    cl_uint arg_index,
                                    const void *arg_value)
{
    return true;
}

bool ValidateSetKernelExecInfo(const Kernel *kernelPacked,
                               KernelExecInfo param_namePacked,
                               size_t param_value_size,
                               const void *param_value)
{
    return true;
}

bool ValidateEnqueueSVMFree(const CommandQueue *command_queuePacked,
                            cl_uint num_svm_pointers,
                            void *const svm_pointers[],
                            void(CL_CALLBACK *pfn_free_func)(cl_command_queue queue,
                                                             cl_uint num_svm_pointers,
                                                             void *svm_pointers[],
                                                             void *user_data),
                            const void *user_data,
                            cl_uint num_events_in_wait_list,
                            Event *const *event_wait_listPacked,
                            Event *const *eventPacked)
{
    return true;
}

bool ValidateEnqueueSVMMemcpy(const CommandQueue *command_queuePacked,
                              cl_bool blocking_copy,
                              const void *dst_ptr,
                              const void *src_ptr,
                              size_t size,
                              cl_uint num_events_in_wait_list,
                              Event *const *event_wait_listPacked,
                              Event *const *eventPacked)
{
    return true;
}

bool ValidateEnqueueSVMMemFill(const CommandQueue *command_queuePacked,
                               const void *svm_ptr,
                               const void *pattern,
                               size_t pattern_size,
                               size_t size,
                               cl_uint num_events_in_wait_list,
                               Event *const *event_wait_listPacked,
                               Event *const *eventPacked)
{
    return true;
}

bool ValidateEnqueueSVMMap(const CommandQueue *command_queuePacked,
                           cl_bool blocking_map,
                           cl_map_flags flags,
                           const void *svm_ptr,
                           size_t size,
                           cl_uint num_events_in_wait_list,
                           Event *const *event_wait_listPacked,
                           Event *const *eventPacked)
{
    return true;
}

bool ValidateEnqueueSVMUnmap(const CommandQueue *command_queuePacked,
                             const void *svm_ptr,
                             cl_uint num_events_in_wait_list,
                             Event *const *event_wait_listPacked,
                             Event *const *eventPacked)
{
    return true;
}

// CL 2.1
bool ValidateSetDefaultDeviceCommandQueue(const Context *contextPacked,
                                          const Device *devicePacked,
                                          const CommandQueue *command_queuePacked)
{
    return true;
}

bool ValidateGetDeviceAndHostTimer(const Device *devicePacked,
                                   const cl_ulong *device_timestamp,
                                   const cl_ulong *host_timestamp)
{
    return true;
}

bool ValidateGetHostTimer(const Device *devicePacked, const cl_ulong *host_timestamp)
{
    return true;
}

bool ValidateCreateProgramWithIL(const Context *contextPacked,
                                 const void *il,
                                 size_t length,
                                 const cl_int *errcode_ret)
{
    return true;
}

bool ValidateCloneKernel(const Kernel *source_kernelPacked, const cl_int *errcode_ret)
{
    return true;
}

bool ValidateGetKernelSubGroupInfo(const Kernel *kernelPacked,
                                   const Device *devicePacked,
                                   KernelSubGroupInfo param_namePacked,
                                   size_t input_value_size,
                                   const void *input_value,
                                   size_t param_value_size,
                                   const void *param_value,
                                   const size_t *param_value_size_ret)
{
    return true;
}

bool ValidateEnqueueSVMMigrateMem(const CommandQueue *command_queuePacked,
                                  cl_uint num_svm_pointers,
                                  const void **svm_pointers,
                                  const size_t *sizes,
                                  cl_mem_migration_flags flags,
                                  cl_uint num_events_in_wait_list,
                                  Event *const *event_wait_listPacked,
                                  Event *const *eventPacked)
{
    return true;
}

// CL 2.2
bool ValidateSetProgramReleaseCallback(const Program *programPacked,
                                       void(CL_CALLBACK *pfn_notify)(cl_program program,
                                                                     void *user_data),
                                       const void *user_data)
{
    return true;
}

bool ValidateSetProgramSpecializationConstant(const Program *programPacked,
                                              cl_uint spec_id,
                                              size_t spec_size,
                                              const void *spec_value)
{
    return true;
}

// CL 3.0
bool ValidateSetContextDestructorCallback(const Context *contextPacked,
                                          void(CL_CALLBACK *pfn_notify)(cl_context context,
                                                                        void *user_data),
                                          const void *user_data)
{
    return true;
}

bool ValidateCreateBufferWithProperties(const Context *contextPacked,
                                        const cl_mem_properties *properties,
                                        cl_mem_flags flags,
                                        size_t size,
                                        const void *host_ptr,
                                        const cl_int *errcode_ret)
{
    return true;
}

bool ValidateCreateImageWithProperties(const Context *contextPacked,
                                       const cl_mem_properties *properties,
                                       cl_mem_flags flags,
                                       const cl_image_format *image_format,
                                       const cl_image_desc *image_desc,
                                       const void *host_ptr,
                                       const cl_int *errcode_ret)
{
    return true;
}
}  // namespace cl
