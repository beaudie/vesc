//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "libANGLE/ResourceManager.h"
#include "libANGLE/renderer/ImplFactory.h"

using namespace rx;
using namespace gl;

namespace
{

class MockFactory : public NullFactory
{
  public:
    MOCK_METHOD0(createBuffer, BufferImpl*());
    MOCK_METHOD1(createTexture, TextureImpl*(GLenum));
    MOCK_METHOD0(createRenderbuffer, RenderbufferImpl*());
};

}

class ResourceManagerTest : public testing::Test
{
  protected:
    void SetUp() override
    {
        mResourceManager = new ResourceManager(&mMockFactory);
    }

    void TearDown() override
    {
        SafeDelete(mResourceManager);
    }

    MockFactory mMockFactory;
    ResourceManager *mResourceManager;
};

TEST_F(ResourceManagerTest, ReallocateBoundTexture)
{
    EXPECT_CALL(mMockFactory, createTexture(GL_TEXTURE_2D)).Times(1).RetiresOnSaturation();

    mResourceManager->checkTextureAllocation(1, GL_TEXTURE_2D);
    GLuint newTexture = mResourceManager->createTexture();
    EXPECT_NE(1u, newTexture);
}

TEST_F(ResourceManagerTest, ReallocateBoundBuffer)
{
    EXPECT_CALL(mMockFactory, createBuffer()).Times(1).RetiresOnSaturation();

    mResourceManager->checkBufferAllocation(1);
    GLuint newBuffer = mResourceManager->createBuffer();
    EXPECT_NE(1u, newBuffer);
}

TEST_F(ResourceManagerTest, ReallocateBoundRenderbuffer)
{
    EXPECT_CALL(mMockFactory, createRenderbuffer()).Times(1).RetiresOnSaturation();

    mResourceManager->checkRenderbufferAllocation(1);
    GLuint newRenderbuffer = mResourceManager->createRenderbuffer();
    EXPECT_NE(1u, newRenderbuffer);
}

class HandleAllocatorTest : public testing::Test
{
  protected:
    HandleAllocator mAllocator;
};


TEST_F(HandleAllocatorTest, ReservationsAndAllocations)
{
    std::set<GLuint> allocationList;
    for (GLuint id = 2; id < 50; id += 2)
    {
        allocationList.insert(id);
    }

    for (GLuint id : allocationList)
    {
        mAllocator.reserve(id);
    }

    std::set<GLuint> allocatedList;
    for (size_t allocationNum = 0; allocationNum < allocationList.size() * 2; ++allocationNum)
    {
        GLuint handle = mAllocator.allocate();
        EXPECT_EQ(0, allocationList.count(handle));
        EXPECT_EQ(0, allocatedList.count(handle));
        allocatedList.insert(handle);
    }
}
