#if !defined(__APPLE__)
#    include <CL/cl.h>
#endif
#include "harness/testHarness.h"

#include <gtest/gtest.h>

#define TEST_SUCCESS 1
#define TEST_FAIL 0

#define MAX_ARG_LENGTH 1024

#define MAX_LINE_LENGTH 3000

class CtsSetup
{
  public:
    static void Init(int *argc, char **argv);
    static int *getArgC() { return globalArgC; }
    static char **getArgV() { return globalArgV; }

  private:
    static int *globalArgC;
    static char **globalArgV;
};

void runTest(::testing::UnitTest *instance);
