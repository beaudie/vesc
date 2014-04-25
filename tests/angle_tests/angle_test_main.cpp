//
// Copyright (c) 2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "gtest/gtest.h"
#include "ANGLETest.h"

#include <map>
#include <string>

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);

    typedef std::pair<std::string, ANGLETest::Config> NameConfigPair;
    typedef std::map<std::string, ANGLETest::Config> ConfigMap;
    ConfigMap allConfigs;
    allConfigs["es2_d3d9"] = ANGLETest::Config(2, EGL_DEFAULT_DISPLAY);
    allConfigs["es2_d3d11"] = ANGLETest::Config(2, EGL_D3D11_ELSE_D3D9_DISPLAY_ANGLE);
    allConfigs["es3_d3d11"] = ANGLETest::Config(3, EGL_D3D11_ELSE_D3D9_DISPLAY_ANGLE);

    // Iterate through the command line arguments and check if they are config names
    std::vector<NameConfigPair> requestedConfigs;
    for (size_t i = 1; i < static_cast<size_t>(argc); i++)
    {
        ConfigMap::const_iterator iter = allConfigs.find(argv[i]);
        if (iter != allConfigs.end())
        {
            requestedConfigs.push_back(*iter);
        }
    }

    // If no configs were requested, run them all
    if (requestedConfigs.empty())
    {
        for (ConfigMap::const_iterator i = allConfigs.begin(); i != allConfigs.end(); i++)
        {
            requestedConfigs.push_back(*i);
        }
    }

    // Run each requested config
    int rt = 0;
    for (size_t i = 0; i < requestedConfigs.size(); i++)
    {
        ANGLETest::Init(requestedConfigs[i].second);

        std::cout << "Running test configuration \"" << requestedConfigs[i].first << "\".\n";

        rt |= RUN_ALL_TESTS();

        ANGLETest::Destroy();
    }

    return rt;
}
