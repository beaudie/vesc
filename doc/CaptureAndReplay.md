# ANGLE OpenGL Frame Capture and Replay

ANGLE currently supports a limited OpenGL capture and replay framework.

Limitations:

 * Many OpenGL ES functions are not yet implemented.
 * EGL capture and replay is not yet supported.
 * Mid-execution is not yet implemented.
 * Capture only tested on desktop platforms currently.
 * The only replay format current is via CPP files.

## Capturing and replaying an application

To build ANGLE with capture and replay enabled update your GN args:

```
angle_with_capture_by_default = true
```

Once built ANGLE will capture the OpenGL ES calls to a CPP replay. By default the replay will be
stored in the current working directory. The capture files will be named according to the pattern
`angle_capture_context{id}_frame{n}.cpp`. ANGLE will additionally write out data binary blobs for
Texture or Buffer contexts to `angle_capture_context{id}_frame{n}.angledata`.

To run a CPP replay you can use a template located in
[samples/capture_and_replay](../samples/capture_and_replay). Update
[samples/BUILD.gn](../samples/BUILD.gn) to adapt the `capture_replay` sample to include your replay.

## Controlling Frame Capture

Some simple environment variables control frame capture:

 * `ANGLE_CAPTURE_ENABLED`:
   Can be set to "0" to disable capture entirely.
 * `ANGLE_CAPTURE_OUT_DIR`:
   Can specify an alternate replay output directory than the CWD.
 * `ANGLE_CAPTURE_FRAME_END`:
   By default ANGLE will capture the first ten frames. This variable can override the default.

A good way to test out the capture is to use environment variables in conjunction with the sample
template. For example:

```
$ ANGLE_CAPTURE_FRAME_END=4 ANGLE_CAPTURE_OUT_DIR=samples/capture_replay out/Debug/simple_texture_2d
```
