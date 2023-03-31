# Introduction

This page describes how to add new extensions to ANGLE.

# Adding EGL extensions

Note: see also [anglebug.com/2621](http://anglebug.com/2621), linked
from the [starter project](Starter-Projects.md) doc, to simplify some
of these steps.

For extensions requiring new entry points:

* Add the extension xml to
  [scripts/egl_angle_ext.xml](../scripts/egl_angle_ext.xml) .

* Note the prototypes for the new entry points must be added to the
  top of the file, and the functions themselves grouped under the
  extension name to the bottom of the file.

* Modify [scripts/registry_xml.py](../scripts/registry_xml.py) to add
  the new extension as needed.

* Run
  [scripts/run_code_generation.py](../scripts/run_code_generation.py)
  .

* The entry point itself goes in
  [entry_points_egl_ext_autogen.h](../src/libGLESv2/entry_points_egl_ext_autogen.h)
  and
  [entry_points_egl_ext_autogen.cpp](../src/libGLESv2/entry_points_egl_ext_autogen.cpp)
  .

* Add the new function to [libEGL.cpp](../src/libEGL/libEGL.cpp) and
  [libEGL.def](../src/libEGL/libEGL.def) .

* Update [eglext_angle.h](../include/EGL/eglext_angle.h) with the new
  entry points and/or enums.

* Add members to the appropriate Extensions struct in
  [Caps.h](../src/libANGLE/Caps.h) and
  [Caps.cpp](../src/libANGLE/Caps.cpp) .

* Initialize extension availability in the `Display` subclass's
  `generateExtensions` method for displays that can support the
  extension; for example,
  [DisplayCGL](../src/libANGLE/renderer/gl/cgl/DisplayCGL.mm).

# Adding GL extensions

For extensions requiring new entry points:

* Add the extension xml to
  [scripts/gl_angle_ext.xml](../scripts/gl_angle_ext.xml) .

* Note the prototypes for the new entry points must be added to the
  top of the file, and the functions themselves grouped under the
  extension name to the bottom of the file.

* Modify [scripts/registry_xml.py](../scripts/registry_xml.py) to add
  the new extension as needed.

* Run
  [scripts/run_code_generation.py](../scripts/run_code_generation.py)
  .

* The entry point itself goes in
  [entry_points_gles_ext_autogen.h](../src/libGLESv2/entry_points_gles_ext_autogen.h)
  and
  [entry_points_gles_ext_autogen.cpp](../src/libGLESv2/entry_points_gles_ext_autogen.cpp)
  .

* Update [gl2ext_angle.h](../include/EGL/gl2ext_angle.h) with the new
  entry points and/or enums.

* Add members to the appropriate Extensions struct in the appropriate cap
  file to enable it. 
  * Vulkan: 
    [vk_caps_utils.cpp](../src/libANGLE/renderer/vulkan/vk_caps_utils.cpp)
  * GL: 
    [renderergl_utils.cpp](../src/libANGLE/renderer/gl/renderergl_utils.cpp)
  * Metal: [DisplayMtl.mm](../src/libANGLE/renderer/metal/DisplayMtl.mm)
  * D3D11: [renderer11_utils.cpp](../src/libANGLE/renderer/d3d/d3d11/renderer11_utils.cpp)

* Add the function in the base class in [ContextImpl.h](../src/libANGLE/renderer/ContextImpl.h)
  as virtual. Then add the function to the necessary context files related
  to the backend to override it.

* Update the validation functions if necessary.

* Run 
  [scripts/run_code_generation.py](../scripts/run_code_generation.py)
  to make sure no more changes are required.
