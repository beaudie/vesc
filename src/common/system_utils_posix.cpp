//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// system_utils_posix.cpp: Implementation of POSIX OS-specific functions.

#include "system_utils.h"

#include <array>

#include <link.h>
#include <dlfcn.h>
#include <unistd.h>

namespace angle
{
Optional<std::string> GetCWD()
{
    std::array<char, 4096> pathBuf;
    char *result = getcwd(pathBuf.data(), pathBuf.size());
    if (result == nullptr)
    {
        return Optional<std::string>::Invalid();
    }
    return std::string(pathBuf.data());
}

bool SetCWD(const char *dirName)
{
    return (chdir(dirName) == 0);
}

bool UnsetEnvironmentVar(const char *variableName)
{
    return (unsetenv(variableName) == 0);
}

bool SetEnvironmentVar(const char *variableName, const char *value)
{
    return (setenv(variableName, value, 1) == 0);
}

std::string GetEnvironmentVar(const char *variableName)
{
    const char *value = getenv(variableName);
    return (value == nullptr ? std::string() : std::string(value));
}

const char *GetPathSeparator()
{
    return ":";
}

class PosixLibrary : public Library
{
  public:
    PosixLibrary(const char *libraryName)
    {
        char buffer[1000];
        int ret = snprintf(buffer, 1000, "%s.%s", libraryName, GetSharedLibraryExtension());
        if (ret > 0 && ret < 1000)
        {
            mModule = dlopen(buffer, RTLD_NOW);
            fprintf(stderr, "Opening shared lib %s returned %p\n", buffer, mModule);
            link_map *linkmap = nullptr;
            int ret = dlinfo(mModule, RTLD_DI_LINKMAP, &linkmap);
            if (ret == 0)
                fprintf(stderr, "  - %s\n", linkmap->l_name);
#if 1
           Dl_serinfo serinfo;
           Dl_serinfo *sip;

           if (dlinfo(mModule, RTLD_DI_SERINFOSIZE, &serinfo) == -1) {
               fprintf(stderr, "RTLD_DI_SERINFOSIZE failed: %s\n", dlerror());
               exit(EXIT_FAILURE);
           }

           /* Allocate the buffer for use with RTLD_DI_SERINFO */

           sip = (Dl_serinfo *)malloc(serinfo.dls_size);
           if (sip == NULL) {
               perror("malloc");
               exit(EXIT_FAILURE);
           }

           /* Initialize the 'dls_size' and 'dls_cnt' fields in the newly
              allocated buffer */

           if (dlinfo(mModule, RTLD_DI_SERINFOSIZE, sip) == -1) {
               fprintf(stderr, "RTLD_DI_SERINFOSIZE failed: %s\n", dlerror());
               exit(EXIT_FAILURE);
           }

           /* Fetch and print library search list */

           if (dlinfo(mModule, RTLD_DI_SERINFO, sip) == -1) {
               fprintf(stderr, "RTLD_DI_SERINFO failed: %s\n", dlerror());
               exit(EXIT_FAILURE);
           }

           for (unsigned int j = 0; j < serinfo.dls_cnt; j++)
               printf("dls_serpath[%d].dls_name = %s\n",
                       j, sip->dls_serpath[j].dls_name);
#endif
        }
    }

    ~PosixLibrary() override
    {
        if (mModule)
        {
            dlclose(mModule);
        }
    }

    void *getSymbol(const char *symbolName) override
    {
        if (!mModule)
        {
            return nullptr;
        }

        return dlsym(mModule, symbolName);
    }

    void *getNative() const override { return mModule; }

  private:
    void *mModule = nullptr;
};

Library *OpenSharedLibrary(const char *libraryName)
{
    return new PosixLibrary(libraryName);
}
}  // namespace angle
