# Debugging Tips

Despite our best efforts ANGLE still has bugs so here are a couple of debugging tips specific to ANGLE

## Running ANGLE under apitrace on Linux

[Apitrace](http://apitrace.github.io/) allows capturing traces of OpenGL commands for later analysis, allowing us to see how ANGLE translated OpenGL ES commands. In order to capture the trace, it inserts a driver shim using `LD_PRELOAD` that records the command and then forwards it to the OpenGL driver.

The problem with ANGLE is that it exposes the same symbols as ANGLE so the usual flow `sample -> ANGLE -> OpenGL driver` becomes `sample -> apitrace shim -> OpenGL driver` and bypasses ANGLE completely. In order to have the correct flow `sample -> ANGLE -> apitrace shim -> OpenGL driver` we must do the following:
 # Compile ANGLE as a static library so that it doesn't get shadowed by apitrace's shim using the `-D angle_gl_library_type=static_library` gyp flag.
 # Ask apitrace to explicitely load the driver instead of using a dlsym on the current module otherwise apitrace will use ANGLE's symbols as the OpenGL driver entrypoint (causing an infinite recursion). To do this `export TRACE_LIBGL=/usr/lib/libGL.so.1`.
 # Link ANGLE against libGL instead of dlsyming the symbols at runtime, otherwise ANGLE doesn't use the replaced driver entrypoints. This can be done with adding the `-D angle_link_glx=1`.

Using all this apitrace works correctly up to a few minor bugs like not being able to figure out what the default framebuffer size if there is no glViewport command.

Here is how to trace a run of `hello_triangle` assuming you are using the ninja gyp generator and that the apitrace executables are in the path:
 - `./build/gyp_angle -D angle_link_glx=1 -D angle_gl_library_type=static_library`
 - `ninja -C out/Debug`
 - `export TRACE_LIBGL="/usr/lib/libGL.so.1"`
 - `apitrace trace -o mytrace ./out/Debug/hello_triangle`
 - `qapitrace mytrace` to analyze the trace using apitrace's GUI
