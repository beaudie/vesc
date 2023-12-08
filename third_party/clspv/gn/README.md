# CLSPV GN Build

The clspv official build system is CMake and as such have no GN build infra. The
files in here enable building `clspv` using the ANGLE GN build infra.

The top-level build file is `BUILD.gn` with all the auxiliary build files
located in the `gn` folder. The `clspv` build is heavily dependent on the LLVM
build. The LLVM settings and targets needed for `clspv` are captured in the
`gn/llvm` location. These utilize the LLVM experimental GN build infra [1].

## Build Instructions

The GN build in here is setup to function within the ANGLE GN build
infrastructure and as such follows the same setup as of ANGLE project. Please
refer top level ANGLE readme file.

Add the following to `args.gn` file

```
angle_enable_cl = true
# llvm_have_mallinfo2 = false # For system that doesn't have mallinfo2
```

Note: Only the `linux/x86{,_64}` and `android/arm{64}` combination of `os/cpu`
are setup for now.

## References

[1]: https://github.com/llvm/llvm-project/blob/main/llvm/utils/gn/README.rst
