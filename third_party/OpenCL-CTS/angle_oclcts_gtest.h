#if !defined(__APPLE__)
#include <CL/cl.h>
#endif
#include "harness/testHarness.h"
#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#define DEFAULT_NUM_ELEMENTS 0x4000

#define MAXLEN 1024

#ifdef _WIN32
#define LOAD_LIB(libpath) LoadLibrary(libpath)
#define CLOSE_LIB(handle) FreeLibrary(handle)
#define LOAD_SYM(handle, fname) GetProcAddress(handle, fname)
#define LIB_HANDLE HINSTANCE
#else // *nix
#define LOAD_LIB(libpath) dlopen(libpath, RTLD_LAZY)
#define CLOSE_LIB(handle) dlclose(handle)
#define LOAD_SYM(handle, fname) dlsym(handle, fname)
#define LIB_HANDLE void *
#endif

#define CALL_METHOD() {\
          char **originalArgV = CtsSetup::getArgV();\
          int argc = (*CtsSetup::getArgC());\
          char **argv = (char**) alloca(((argc + 1) * sizeof(char*)) + 1);\
          for(int index = 0; index < (argc + 1); index++){\
            argv[index] = (char *) alloca((MAXLEN * sizeof(char)) + 1);\
          }\
          for(int index = 0; index < *CtsSetup::getArgC(); index++)\
          {\
            if(index == 0){\
              const char *testSuiteName = ::testing::UnitTest::GetInstance()->current_test_suite()->name();\
              char invokedSuite[strlen(testSuiteName) + 3];\
              strcpy(invokedSuite, "./");\
              strcat(invokedSuite, testSuiteName);\
              strcpy(argv[index], invokedSuite);\
            }\
            else{\
              strcpy(argv[index], originalArgV[index]);\
            }\
          }\
          strcpy(argv[argc], ::testing::UnitTest::GetInstance()->current_test_info()->name());\
          argc += 1;\
          int result = test_method(argc, (const char **) argv);\
          EXPECT_EQ(result, EXIT_SUCCESS);\
          }

#define RUN_TEST() {\
          const char *testSuiteName = ::testing::UnitTest::GetInstance()->current_test_suite()->name();\
          char libraryFileName[strlen(testSuiteName) + strlen("lib.so") + 1];\
          strcpy(libraryFileName, "lib");\
          strcat(libraryFileName, testSuiteName);\
          strcat(libraryFileName, ".so");\
          LIB_HANDLE ret = LOAD_LIB(libraryFileName);\
          if (NULL == ret)\
          {\
            GTEST_FAIL();\
          }\
          int (*test_method)(int argc, const char *argv[]) = (int(*)(int argc, const char *argv[]))LOAD_SYM(ret, "main");\
          if (test_method == NULL){\
            printf("%s\n", dlerror());\
            CLOSE_LIB(ret);\
            GTEST_FAIL();\
          }\
          else{\
            CALL_METHOD();\
            CLOSE_LIB(ret);\
          }\
        }

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
