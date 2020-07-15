# Introduction

This page describes how to add new extensions to ANGLE.

# Adding EGL extensions

Note: see also [anglebug.com/2621](http://anglebug.com/2621), linked
from the [starter project](Starter-Projects.md) doc, to simplify some
of these steps.

For extensions requiring new entry points:

* Add the extension xml to
  [scripts/egl_angle_ext.xml](https://source.chromium.org/chromium/chromium/src/+/master:third_party/angle/scripts/egl_angle_ext.xml)
  .

* Note the prototypes for the new entry points must be added to the
  top of the file, and the functions themselves grouped under the
  extension name to the bottom of the file.

* Modify
  [scripts/registry_xml.py](https://source.chromium.org/chromium/chromium/src/+/master:third_party/angle/scripts/registry_xml.py)
  to add the new extension as needed.

* Run
  [scripts/run_code_generation.py](https://source.chromium.org/chromium/chromium/src/+/master:third_party/angle/scripts/run_code_generation.py)
  .

* The entry point itself goes in
  [entry_points_egl_ext.h](https://source.chromium.org/chromium/chromium/src/+/master:third_party/angle/src/libGLESv2/entry_points_egl_ext.h)
  and [entry_points_egl_ext.cpp](https://source.chromium.org/chromium/chromium/src/+/master:third_party/angle/src/libGLESv2/entry_points_egl_ext.cpp) .

* Add the new function to
  [libEGL.cpp](https://source.chromium.org/chromium/chromium/src/+/master:third_party/angle/src/libEGL/libEGL.cpp)
  and [libEGL.def](https://source.chromium.org/chromium/chromium/src/+/master:third_party/angle/src/libEGL/libEGL.def) .

* Update
  [eglext_angle.h](https://source.chromium.org/chromium/chromium/src/+/master:third_party/angle/include/EGL/eglext_angle.h)
  with the new entry points and/or enums.

* Add members to the appropriate Extensions struct in
  [Caps.h](https://source.chromium.org/chromium/chromium/src/+/master:third_party/angle/src/libANGLE/Caps.h)
  and [Caps.cpp](https://source.chromium.org/chromium/chromium/src/+/master:third_party/angle/src/libANGLE/Caps.cpp) .

* Initialize extension availability in the `Display` subclass's
  `generateExtensions` method for displays that can support the
  extension; for example,
  [DisplayCGL](https://source.chromium.org/chromium/angle/angle.git/+/483ee3fa2315298aba6cbaf6f62d515f7214c0f7:src/libANGLE/renderer/gl/cgl/DisplayCGL.mm;l=324?originalUrl=https:%2F%2Fcs.chromium.org%2Fchromium%2Fsrc%2Fthird_party%2Fangle%2Fsrc%2FlibANGLE%2Frenderer%2Fgl%2Fcgl%2FDisplayCGL.mm).
