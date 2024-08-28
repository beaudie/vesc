#include <gtest/gtest.h>
#if !defined(__APPLE__)
#    include <CL/cl.h>
#endif
#include <stdlib.h>

#include "angle_oclcts_gtest.h"

int *CtsSetup::globalArgC;
char **CtsSetup::globalArgV;

void CtsSetup::Init(int *argc, char **argv)
{
    globalArgC = argc;
    globalArgV = argv;
}

int main(int argc, char **argv)
{
    CtsSetup::Init(&argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

void runTest(::testing::UnitTest* instance)
{
    char path[MAX_LINE_LENGTH];
    const char *testSuite = instance->current_test_suite()->name();
    const char *testName = instance->current_test_info()->name();
    char **originalArgV = CtsSetup::getArgV();
    int argc            = (*CtsSetup::getArgC());
    char **argv         = (char **)alloca(((argc + 1) * sizeof(char *)) + 1);
    for (int index = 0; index < (argc + 1); index++)
    {
        argv[index] = (char *)alloca((MAX_LINE_LENGTH * sizeof(char)) + 1);
    }
    char command[MAX_LINE_LENGTH];
    for (int index = 0; index < *CtsSetup::getArgC(); index++)
    {
        if (index == 0)
        {
            strcpy(command, "./");
            strcat(command, testSuite);
        }
        else
        {
            strcat(command, " ");
            strcat(command, originalArgV[index]);
        }
    }
    strcat(command, " ");
    strcat(command, testName);
    FILE *fp = popen(command, "r");
    int result = TEST_FAIL;
    if (fp != NULL){
        while (fgets(path, 300, fp) != NULL){
            printf("%s", path);
            if(strstr(path, "PASSED test.")){
                result = TEST_SUCCESS;
                break;
            }
            else if(strstr(path, "FAILED test.")){
                break;
            }
        }
        pclose(fp);
    }
    if(result == TEST_SUCCESS){
        GTEST_SUCCEED();
    }
    else{
        GTEST_FAIL();
    }
}
