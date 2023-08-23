#!/bin/bash
autoninja -C out/Android angle_end2end_tests || exit 1

out/Android/angle_end2end_tests --gtest_filter=*ImageTestES3.RGBXAHBUploadDataColorspace_Clone/ES3_Vulkan --verbose

# using mali
# out/Android/angle_end2end_tests --gtest_filter=*ImageTestES3.RGBXAHBUploadDataColorspace_Clone/ES3_OpenGLES --verbose
# out/Android/angle_end2end_tests --gtest_filter=*ImageTestES3.RGBXAHBUploadDataColorspace/* --verbose
# out/Android/angle_end2end_tests --gtest_filter=*ImageTestES3.AHBUploadDataColorspace/* --verbose
