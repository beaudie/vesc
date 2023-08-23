#!/bin/bash
autoninja -C out/Android angle_end2end_tests || exit 1

out/Android/angle_end2end_tests --gtest_filter=*ImageTestES3.AHBUploadDataColorspace/ES3_Vulkan --verbose

# using mali
# out/Android/angle_end2end_tests --gtest_filter=*ImageTestES3.AHBUploadDataColorspace/ES3_OpenGLES --verbose
