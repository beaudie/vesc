# GPU Memory Reporting and Analysis

[MemRptExt]: https://www.khronos.org/registry/vulkan/specs/1.2-extensions/man/html/VK_EXT_device_memory_report.html

GPU memory usage data can be reported when using the Vulkan back-end with drivers that support the
[VK_EXT_device_memory_report][MemRptExt] extension.  When enabled, ANGLE will produce log messages
based on every allocation, free, import, unimport, and failed allocation of GPU memory.

By default, both general logging and memory report logging are disabled.  ANGLE's general logging
is enabled at compilation time, when the following GN arg is set:

```
angle_enable_trace = true
```

## GPU Memory Reporting

ANGLE registers a callback function with the Vulkan driver for
[VK_EXT_device_memory_report][MemRptExt] extension.  The Vulkan driver calls this callback for the
each of the following GPU memory events:

- Allocation of GPU memory dedicated to ANGLE
- Free of GPU memory dedicated to ANGLE
- Import of GPU memory provided by another process (e.g. Android SurfaceFlinger)
- Unimport of GPU memory provided by another process
- Failed allocation

The callback provides additional information about each event such as the size, the VkObjectType,
and the address (see the extension documentation for more details).  ANGLE caches this information,
and logs messages based on this information.  ANGLE keeps track of how much of each type of memory
is allocated and imported.  For example, if a GLES command causes ANGLE five 4 KB descriptor-set
allocations, ANGLE will add 20 KB to the total of allocated descriptor-set memory.

ANGLE supports two types of memory reporting, both of which are enabled
via feature flags:

* `logMemoryReportStats` provides summary statistics at each eglSwapBuffers() command
* `logMemoryReportCallbacks` provides per-callback information at the time of the callback

Both feature flags can be enabled at the same time.  A simple way to enable a feature flag is by
editing the `RendererVk.cpp` file.

Note: At this time, GPU memory reporting has only been used on Android.  The logged information can
be viewed with the `logcat` command.

## GPU Memory Analysis

GPU memory reporting can be combined with other forms of debugging in order to do analysis.  For
example, for a GLES application/test that properly shuts down, the total size of each type of
allocated and imported memory should be zero bytes at the end of the application/test.  If not, a memory leak exists, and the log can be used to determine where the leak occurs.

If an application seems to be using too much GPU memory, enabling memory reporting can reveal which
type of memory is being excessively used.

Complex forms of analysis can be done by enabling logging of every GLES and EGL API command.  This
can be enabled on Android at compilation time, when the following GN args are set:
```
angle_enable_trace = true
angle_enable_android_api_logcat = true
```

Combining that with enabling the `logMemoryReportCallbacks` feature flag will allow each memory
allocation and import to be correlated with the GLES/EGL commands that caused it.  This can also be
combined with the use of a graphics debugger such as Android GPU Inspector (AGI) or RenderDoc in
order to gain insight into the cause of excessive or leaked allocations and/or imports.
