#include <gtest/gtest.h>
#if !defined(__APPLE__)
#include <CL/cl.h>
#endif
#include <stdlib.h>

#include "angle_oclcts_gtest.h"

int *CtsSetup::globalArgC;
char **CtsSetup::globalArgV;

void CtsSetup::Init(int *argc, char **argv) {
    globalArgC = argc;
    globalArgV = argv;
}

int main(int argc, char **argv) {
    printf("In main\n");
    CtsSetup::Init(&argc, argv);
    printf("Post cts init\n");
    ::testing::InitGoogleTest(&argc, argv); 
    return RUN_ALL_TESTS();
}
