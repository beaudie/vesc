//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// MultiDrawTest: Tests of GL_ANGLE_multi_draw

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

using namespace angle;

namespace
{

// Create a kWidth * kHeight canvas equally split into kCountX * kCountY tiles
// each containing a quad partially coverting each tile
constexpr uint32_t kWidth                  = 256;
constexpr uint32_t kHeight                 = 256;
constexpr uint32_t kCountX                 = 8;
constexpr uint32_t kCountY                 = 8;
constexpr uint32_t kQuadCount              = kCountX * kCountY;
constexpr uint32_t kTriCount               = kQuadCount * 2;
constexpr std::array<GLfloat, 2> kTileSize = {
    1.f / static_cast<GLfloat>(kCountX),
    1.f / static_cast<GLfloat>(kCountY),
};
constexpr std::array<uint32_t, 2> kTilePixelSize  = {kWidth / kCountX, kHeight / kCountY};
constexpr std::array<GLfloat, 2> kQuadRadius      = {0.25f * kTileSize[0], 0.25f * kTileSize[1]};
constexpr std::array<uint32_t, 2> kPixelCheckSize = {
    static_cast<uint32_t>(kQuadRadius[0] * kWidth),
    static_cast<uint32_t>(kQuadRadius[1] * kHeight)};

constexpr std::array<GLfloat, 2> getTileCenter(uint32_t x, uint32_t y)
{
    return {
        kTileSize[0] * (0.5f + static_cast<GLfloat>(x)),
        kTileSize[1] * (0.5f + static_cast<GLfloat>(y)),
    };
}
constexpr std::array<std::array<GLfloat, 3>, 4> getQuadVertices(uint32_t x, uint32_t y)
{
    const auto center = getTileCenter(x, y);
    return {
        std::array<GLfloat, 3>{center[0] - kQuadRadius[0], center[1] - kQuadRadius[1], 0.0f},
        std::array<GLfloat, 3>{center[0] + kQuadRadius[0], center[1] - kQuadRadius[1], 0.0f},
        std::array<GLfloat, 3>{center[0] + kQuadRadius[0], center[1] + kQuadRadius[1], 0.0f},
        std::array<GLfloat, 3>{center[0] - kQuadRadius[0], center[1] + kQuadRadius[1], 0.0f},
    };
}

enum class DrawIDOption
{
    NoDrawID,
    UseDrawID,
};

enum class InstancingOption
{
    NoInstancing,
    UseInstancing,
};

using MultiDrawTestParams = std::tuple<angle::PlatformParameters, DrawIDOption, InstancingOption>;

struct PrintToStringParamName
{
    std::string operator()(const ::testing::TestParamInfo<MultiDrawTestParams> &info) const
    {
        ::std::stringstream ss;
        ss << (std::get<2>(info.param) == InstancingOption::UseInstancing ? "Instanced_" : "")
           << (std::get<1>(info.param) == DrawIDOption::UseDrawID ? "DrawID_" : "")
           << std::get<0>(info.param);
        return ss.str();
    }
};

// These tests check correctness of the ANGLE_multi_draw extension.
// An array of quads is drawn across the screen.
// gl_DrawID is checked by using it to select the color of the draw.
// MultiDraw*Instanced entrypoints use the existing instancing APIs which are
// more fully tested in InstancingTest.cpp.
// Correct interaction with the instancing APIs is tested here by using scaling
// and then instancing the array of quads over four quadrants on the screen.
class MultiDrawTest : public ANGLETestBase, public ::testing::TestWithParam<MultiDrawTestParams>
{
  protected:
    MultiDrawTest()
        : ANGLETestBase(std::get<0>(GetParam())),
          mNonIndexedVertexBuffer(0u),
          mVertexBuffer(0u),
          mIndexBuffer(0u),
          mInstanceBuffer(0u),
          mProgram(0u)
    {
        setWindowWidth(kWidth);
        setWindowHeight(kHeight);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    void SetUp() override { ANGLETestBase::ANGLETestSetUp(); }

    bool IsDrawIDTest() const { return std::get<1>(GetParam()) == DrawIDOption::UseDrawID; }

    bool IsInstancedTest() const
    {
        return std::get<2>(GetParam()) == InstancingOption::UseInstancing;
    }

    std::string VertexShaderSource()
    {

        std::stringstream shader;
        shader << (IsDrawIDTest() ? "#extension GL_ANGLE_multi_draw : require\n" : "")
               << (IsInstancedTest() ? "attribute float vInstance;" : "") << R"(
attribute vec2 vPosition;
varying vec4 color;
void main()
{
    int id = )" << (IsDrawIDTest() ? "gl_DrawID" : "0")
               << ";"
               << R"(
    float quad_id = float(id / 2);
    float color_id = quad_id - (3.0 * floor(quad_id / 3.0));
    if (color_id == 0.0) {
      color = vec4(1, 0, 0, 1);
    } else if (color_id == 1.0) {
      color = vec4(0, 1, 0, 1);
    } else {
      color = vec4(0, 0, 1, 1);
    }

    mat3 transform = mat3(1.0);
)"
               << (IsInstancedTest() ? R"(
    transform[0][0] = 0.5;
    transform[1][1] = 0.5;
    if (vInstance == 0.0) {

    } else if (vInstance == 1.0) {
        transform[2][0] = 0.5;
    } else if (vInstance == 2.0) {
        transform[2][1] = 0.5;
    } else if (vInstance == 3.0) {
        transform[2][0] = 0.5;
        transform[2][1] = 0.5;
    }
)"
                                     : "")
               << R"(
    gl_Position = vec4(transform * vec3(vPosition, 1.0) * 2.0 - 1.0, 1);
})";

        return shader.str();
    }

    std::string FragmentShaderSource()
    {
        return
            R"(precision mediump float;
            varying vec4 color;
            void main()
            {
                gl_FragColor = color;
            })";
    }

    void SetupProgram()
    {
        mProgram = CompileProgram(VertexShaderSource().c_str(), FragmentShaderSource().c_str());
        EXPECT_GL_NO_ERROR();
        ASSERT_GE(mProgram, 1u);
        glUseProgram(mProgram);
        mPositionLoc = glGetAttribLocation(mProgram, "vPosition");
        mInstanceLoc = glGetAttribLocation(mProgram, "vInstance");
    }

    void SetupBuffers()
    {
        for (uint32_t y = 0; y < kCountY; ++y)
        {
            for (uint32_t x = 0; x < kCountX; ++x)
            {
                // v3 ---- v2
                // |       |
                // |       |
                // v0 ---- v1
                uint32_t quadIndex         = y * kCountX + x;
                GLushort starting_index    = static_cast<GLushort>(4 * quadIndex);
                std::array<GLushort, 6> is = {0, 1, 2, 0, 2, 3};
                const auto vs              = getQuadVertices(x, y);
                for (GLushort i : is)
                {
                    mIndices.push_back(starting_index + i);
                }

                for (const auto &v : vs)
                {
                    mVertices.insert(mVertices.end(), v.begin(), v.end());
                }

                for (GLushort i : is)
                {
                    mNonIndexedVertices.insert(mNonIndexedVertices.end(), vs[i].begin(),
                                               vs[i].end());
                }
            }
        }

        std::array<GLfloat, 4> instances{0, 1, 2, 3};

        glGenBuffers(1, &mNonIndexedVertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, mNonIndexedVertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * mNonIndexedVertices.size(),
                     mNonIndexedVertices.data(), GL_STATIC_DRAW);

        glGenBuffers(1, &mVertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * mVertices.size(), mVertices.data(),
                     GL_STATIC_DRAW);

        glGenBuffers(1, &mIndexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * mIndices.size(), mIndices.data(),
                     GL_STATIC_DRAW);

        glGenBuffers(1, &mInstanceBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, mInstanceBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * instances.size(), instances.data(),
                     GL_STATIC_DRAW);

        ASSERT_GL_NO_ERROR();
    }

    void DoVertexAttribDivisor(GLint location, GLuint divisor)
    {
        if (getClientMajorVersion() <= 2)
        {
            ASSERT_TRUE(IsGLExtensionEnabled("GL_ANGLE_instanced_arrays"));
            glVertexAttribDivisorANGLE(location, divisor);
        }
        else
        {
            glVertexAttribDivisor(location, divisor);
        }
    }

    void DoDrawArrays()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glBindBuffer(GL_ARRAY_BUFFER, mNonIndexedVertexBuffer);
        glEnableVertexAttribArray(mPositionLoc);
        glVertexAttribPointer(mPositionLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);

        std::vector<GLint> firsts(kTriCount);
        std::vector<GLsizei> counts(kTriCount, 3);
        for (uint32_t i = 0; i < kTriCount; ++i)
            firsts[i] = i * 3;

        if (IsInstancedTest())
        {
            glBindBuffer(GL_ARRAY_BUFFER, mInstanceBuffer);
            glEnableVertexAttribArray(mInstanceLoc);
            glVertexAttribPointer(mInstanceLoc, 1, GL_FLOAT, GL_FALSE, 0, 0);
            DoVertexAttribDivisor(mInstanceLoc, 1);
            std::vector<GLsizei> instanceCounts(kTriCount, 4);
            glMultiDrawArraysInstancedANGLE(GL_TRIANGLES, firsts.data(), counts.data(),
                                            instanceCounts.data(), kTriCount);
        }
        else
        {
            glMultiDrawArraysANGLE(GL_TRIANGLES, firsts.data(), counts.data(), kTriCount);
        }
    }

    void DoDrawElements()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
        glEnableVertexAttribArray(mPositionLoc);
        glVertexAttribPointer(mPositionLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);

        std::vector<GLsizei> counts(kTriCount, 3);
        std::vector<const GLvoid *> indices(kTriCount);
        for (uint32_t i = 0; i < kTriCount; ++i)
            indices[i] = reinterpret_cast<GLvoid *>(static_cast<uintptr_t>(i * 3 * 2));

        if (IsInstancedTest())
        {
            glBindBuffer(GL_ARRAY_BUFFER, mInstanceBuffer);
            glEnableVertexAttribArray(mInstanceLoc);
            glVertexAttribPointer(mInstanceLoc, 1, GL_FLOAT, GL_FALSE, 0, 0);
            DoVertexAttribDivisor(mInstanceLoc, 1);
            std::vector<GLsizei> instanceCounts(kTriCount, 4);
            glMultiDrawElementsInstancedANGLE(GL_TRIANGLES, counts.data(), GL_UNSIGNED_SHORT,
                                              indices.data(), instanceCounts.data(), kTriCount);
        }
        else
        {
            glMultiDrawElementsANGLE(GL_TRIANGLES, counts.data(), GL_UNSIGNED_SHORT, indices.data(),
                                     kTriCount);
        }
    }

    void CheckDrawResult()
    {
        for (uint32_t y = 0; y < kCountY; ++y)
        {
            for (uint32_t x = 0; x < kCountX; ++x)
            {
                uint32_t center_x             = x * kTilePixelSize[0] + kTilePixelSize[0] / 2;
                uint32_t center_y             = y * kTilePixelSize[1] + kTilePixelSize[1] / 2;
                uint32_t quadID               = IsDrawIDTest() ? y * kCountX + x : 0;
                uint32_t colorID              = quadID % 3u;
                std::array<GLColor, 3> colors = {GLColor(255, 0, 0, 255), GLColor(0, 255, 0, 255),
                                                 GLColor(0, 0, 255, 255)};
                GLColor expected              = colors[colorID];

                if (IsInstancedTest())
                {
                    EXPECT_PIXEL_RECT_EQ(center_x / 2 - kPixelCheckSize[0] / 4,
                                         center_y / 2 - kPixelCheckSize[1] / 4,
                                         kPixelCheckSize[0] / 2, kPixelCheckSize[1] / 2, expected);
                    EXPECT_PIXEL_RECT_EQ(center_x / 2 - kPixelCheckSize[0] / 4 + kWidth / 2,
                                         center_y / 2 - kPixelCheckSize[1] / 4,
                                         kPixelCheckSize[0] / 2, kPixelCheckSize[1] / 2, expected);
                    EXPECT_PIXEL_RECT_EQ(center_x / 2 - kPixelCheckSize[0] / 4,
                                         center_y / 2 - kPixelCheckSize[1] / 4 + kHeight / 2,
                                         kPixelCheckSize[0] / 2, kPixelCheckSize[1] / 2, expected);
                    EXPECT_PIXEL_RECT_EQ(center_x / 2 - kPixelCheckSize[0] / 4 + kWidth / 2,
                                         center_y / 2 - kPixelCheckSize[1] / 4 + kHeight / 2,
                                         kPixelCheckSize[0] / 2, kPixelCheckSize[1] / 2, expected);
                }
                else
                {
                    EXPECT_PIXEL_RECT_EQ(center_x - kPixelCheckSize[0] / 2,
                                         center_y - kPixelCheckSize[1] / 2, kPixelCheckSize[0],
                                         kPixelCheckSize[1], expected);
                }
            }
        }
    }

    void TearDown() override
    {
        if (mNonIndexedVertexBuffer != 0u)
        {
            glDeleteBuffers(1, &mNonIndexedVertexBuffer);
        }
        if (mVertexBuffer != 0u)
        {
            glDeleteBuffers(1, &mVertexBuffer);
        }
        if (mIndexBuffer != 0u)
        {
            glDeleteBuffers(1, &mIndexBuffer);
        }
        if (mInstanceBuffer != 0u)
        {
            glDeleteBuffers(1, &mInstanceBuffer);
        }
        if (mProgram != 0)
        {
            glDeleteProgram(mProgram);
        }
        ANGLETestBase::ANGLETestTearDown();
    }

    bool requestMultiDrawExtension()
    {
        if (IsGLExtensionRequestable("GL_ANGLE_multi_draw"))
        {
            glRequestExtensionANGLE("GL_ANGLE_multi_draw");
        }

        if (!IsGLExtensionEnabled("GL_ANGLE_multi_draw"))
        {
            return false;
        }

        return true;
    }

    bool requestInstancedExtension()
    {
        if (IsGLExtensionRequestable("GL_ANGLE_instanced_arrays"))
        {
            glRequestExtensionANGLE("GL_ANGLE_instanced_arrays");
        }

        if (!IsGLExtensionEnabled("GL_ANGLE_instanced_arrays"))
        {
            return false;
        }

        return true;
    }

    bool requestExtensions()
    {
        if (IsInstancedTest() && getClientMajorVersion() <= 2)
        {
            if (!requestInstancedExtension())
            {
                return false;
            }
        }
        return requestMultiDrawExtension();
    }

    std::vector<GLushort> mIndices;
    std::vector<GLfloat> mVertices;
    std::vector<GLfloat> mNonIndexedVertices;
    GLuint mNonIndexedVertexBuffer;
    GLuint mVertexBuffer;
    GLuint mIndexBuffer;
    GLuint mInstanceBuffer;
    GLuint mProgram;
    GLint mPositionLoc;
    GLint mInstanceLoc;
};

class MultiDrawNoInstancingSupportTest : public MultiDrawTest
{
    void SetUp() override
    {
        ASSERT_LE(getClientMajorVersion(), 2);
        ASSERT_TRUE(IsInstancedTest());
        MultiDrawTest::SetUp();
    }
};

// glMultiDraw*ANGLE are emulated and should always be available
TEST_P(MultiDrawTest, RequestExtension)
{
    EXPECT_TRUE(requestMultiDrawExtension());
}

// Test that compile a program with the extension succeeds
TEST_P(MultiDrawTest, CanCompile)
{
    ANGLE_SKIP_TEST_IF(!requestExtensions());
    SetupProgram();
}

// Tests basic functionality of glMultiDrawArraysANGLE
TEST_P(MultiDrawTest, MultiDrawArrays)
{
    ANGLE_SKIP_TEST_IF(!requestExtensions());
    SetupBuffers();
    SetupProgram();
    DoDrawArrays();
    EXPECT_GL_NO_ERROR();
    CheckDrawResult();
}

// Tests basic functionality of glMultiDrawElementsANGLE
TEST_P(MultiDrawTest, MultiDrawElements)
{
    ANGLE_SKIP_TEST_IF(!requestExtensions());
    SetupBuffers();
    SetupProgram();
    DoDrawElements();
    EXPECT_GL_NO_ERROR();
    CheckDrawResult();
}

// Tests basic functionality of glMultiDrawElementsANGLE
TEST_P(MultiDrawTest, DrawElementsUBYTEDrawElementsUSHORT)
{
    ANGLE_SKIP_TEST_IF(!requestExtensions());
    SetupBuffers();
    SetupProgram();
    DoDrawElements();
    EXPECT_GL_NO_ERROR();
    CheckDrawResult();
}

// Check that glMultiDraw*Instanced without instancing support results in GL_INVALID_OPERATION
TEST_P(MultiDrawNoInstancingSupportTest, InvalidOperation)
{
    ANGLE_SKIP_TEST_IF(IsGLExtensionEnabled("GL_ANGLE_instanced_arrays"));
    requestMultiDrawExtension();
    SetupBuffers();
    SetupProgram();

    GLint first       = 0;
    GLsizei count     = 3;
    GLvoid *indices   = 0;
    GLsizei instances = 1;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindBuffer(GL_ARRAY_BUFFER, mNonIndexedVertexBuffer);
    glEnableVertexAttribArray(mPositionLoc);
    glVertexAttribPointer(mPositionLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glMultiDrawArraysInstancedANGLE(GL_TRIANGLES, &first, &count, &instances, 1);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
    glEnableVertexAttribArray(mPositionLoc);
    glVertexAttribPointer(mPositionLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glMultiDrawElementsInstancedANGLE(GL_TRIANGLES, &count, GL_UNSIGNED_SHORT, &indices, &instances,
                                      1);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

#if 0
class MultiDrawTestWebGLPort : public ANGLETestBase, public ::testing::TestWithParam<MultiDrawTestParams>
{
protected:
	MultiDrawTestWebGLPort()
		: ANGLETestBase(std::get<0>(GetParam())),
		// mIndices({ 0, 1, 2, 0 }),
		// mVertices({ 0.2, 0.2, 0.8, 0.2, 0.5, 0.8 }),
		// mInstances({ 0, 1, 2, 3 }),
		mNonIndexedVertexBuffer(0u),
		mVertexBuffer(0u),
		mIndexBuffer(0u),
		mInstanceBuffer(0u),
		mProgram(0u)
	{
		setWindowWidth(kWidth);
		setWindowHeight(kHeight);
		setConfigRedBits(8);
		setConfigGreenBits(8);
		setConfigBlueBits(8);
		setConfigAlphaBits(8);
	}

	void SetUp() override { ANGLETestBase::ANGLETestSetUp(); }

	void SetupProgram()
	{
		constexpr char kVShaderNoDrawID[] = R"(
attribute vec2 vPosition;
attribute float vInstance;
varying vec4 color;
void main()
{
  color = vec4(1.0, 0.0, 0.0, 1.0);
  mat3 transform = mat3(1.0);
  // vInstance starts at 1.0 on instanced calls
  if (vInstance >= 1.0) {
    transform[0][0] = 0.5;
    transform[1][1] = 0.5;
  }
  if (vInstance == 1.0) {
  } else if (vInstance == 2.0) {
      transform[2][0] = 0.5;
  } else if (vInstance == 3.0) {
      transform[2][1] = 0.5;
  } else if (vInstance == 4.0) {
      transform[2][0] = 0.5;
      transform[2][1] = 0.5;
  }
  gl_Position = vec4(transform * vec3(vPosition, 1.0) * 2.0 - 1.0, 1);
}
)";

		constexpr char kFS[] = R"(
precision mediump float;
varying vec4 color;
void main() {
  gl_FragColor = color;
}
)";
		// const program = wtu.setupProgram(gl, ["vshaderNoDrawID", "fshader"], ["vPosition", "vInstance"], [0, 1]);
		mProgram = CompileProgram(kVShaderNoDrawID, kFS);
		EXPECT_GL_NO_ERROR();
		ASSERT_GE(mProgram, 1u);
		glUseProgram(mProgram);
		mPositionLoc = glGetAttribLocation(mProgram, "vPosition");
		mInstanceLoc = glGetAttribLocation(mProgram, "vInstance");
	}

	void setupDrawArrays() {
		glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, false, 0, 0);
	}

	void setupDrawElements() {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, false, 0, 0);
	}

	void setupInstanced() {
		glBindBuffer(GL_ARRAY_BUFFER, mInstanceBuffer);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 1, GL_FLOAT, false, 0, 0);
		glVertexAttribDivisor(1, 1);
	}

	void setupDrawArraysInstanced() {
		setupDrawArrays();
		setupInstanced();
	}

	void setupDrawElementsInstanced() {
		setupDrawElements();
		setupInstanced();
	}

	void SetupBuffers()
	{
		for (uint32_t y = 0; y < kCountY; ++y)
		{
			for (uint32_t x = 0; x < kCountX; ++x)
			{
				// v3 ---- v2
				// |       |
				// |       |
				// v0 ---- v1
				uint32_t quadIndex = y * kCountX + x;
				GLushort starting_index = static_cast<GLushort>(4 * quadIndex);
				std::array<GLushort, 6> is = { 0, 1, 2, 0, 2, 3 };
				const auto vs = getQuadVertices(x, y);
				for (GLushort i : is)
				{
					mIndices.push_back(starting_index + i);
				}

				for (const auto &v : vs)
				{
					mVertices.insert(mVertices.end(), v.begin(), v.end());
				}

				for (GLushort i : is)
				{
					mNonIndexedVertices.insert(mNonIndexedVertices.end(), vs[i].begin(),
						vs[i].end());
				}
			}
		}

		std::array<GLfloat, 4> instances{ 0, 1, 2, 3 };

		glGenBuffers(1, &mNonIndexedVertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, mNonIndexedVertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * mNonIndexedVertices.size(),
			mNonIndexedVertices.data(), GL_STATIC_DRAW);

		glGenBuffers(1, &mVertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * mVertices.size(), mVertices.data(),
			GL_STATIC_DRAW);

		glGenBuffers(1, &mIndexBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * mIndices.size(), mIndices.data(),
			GL_STATIC_DRAW);

		glGenBuffers(1, &mInstanceBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, mInstanceBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * instances.size(), instances.data(),
			GL_STATIC_DRAW);

		ASSERT_GL_NO_ERROR();
	}

	void runValidationTests(bool instanced) {
		SetupBuffers(); // Double check that this is correct
		SetupProgram();

		glUseProgram(mProgram);

		// Check that drawing a single triangle works
		if (!instanced) {
			setupDrawElements();
			// According to the WebGL spec, multiDrawElementsWEBGL takes 7 arguments but it's supposed to map directly to
			// glMultiDrawElementsANGLE, though that has only 5 arguments.
			// I'm pretty sure that's a Java -> native mapping issue. extra two parameters are offsets into the arrays.
			std::vector<GLsizei> counts(1, 3);
			std::vector<const GLvoid *> indices(1, 0);
			glMultiDrawElementsANGLE(GL_TRIANGLES, counts.data(), GL_UNSIGNED_BYTE, indices.data(), 1);
			EXPECT_GL_NO_ERROR();
			glDisableVertexAttribArray(0);
			glDisableVertexAttribArray(1);
		}
		else {
			setupDrawElementsInstanced();
			std::vector<GLsizei> counts(1, 3);
			std::vector<const GLvoid *> indices(1, 0);
			std::vector<GLsizei> instanceCounts(1, 1);
			glMultiDrawElementsInstancedANGLE(GL_TRIANGLES, counts.data(), GL_UNSIGNED_BYTE, indices.data(), instanceCounts.data(), 1);
			EXPECT_GL_NO_ERROR();
			glDisableVertexAttribArray(0);
			glDisableVertexAttribArray(1);
		}
	}

#    if 0
	function checkResult(config, msg) {
		const rects = [];
		const expected = [
			[255, 0, 0, 255],
				[0, 255, 0, 255],
				[0, 0, 255, 255],
		];
		for (let y = 0; y < 1; ++y) {
			for (let x = 0; x < x_count; ++x) {
				const center_x = x * tilePixelSize[0] + Math.floor(tilePixelSize[0] / 2);
				const center_y = y * tilePixelSize[1] + Math.floor(tilePixelSize[1] / 2);
				const quadID = y * x_count + x;
				const colorID = config.drawID ? quadID % 3 : 0;
				if (config.instanced) {
					rects.push(wtu.makeCheckRect(
						center_x / 2 - Math.floor(pixelCheckSize[0] / 4),
						center_y / 2 - Math.floor(pixelCheckSize[1] / 4),
						pixelCheckSize[0] / 2,
						pixelCheckSize[1] / 2,
						expected[colorID],
						msg + " (" + x + "," + y + ")", 0));
					rects.push(wtu.makeCheckRect(
						center_x / 2 - Math.floor(pixelCheckSize[0] / 4) + width / 2,
						center_y / 2 - Math.floor(pixelCheckSize[1] / 4),
						pixelCheckSize[0] / 2,
						pixelCheckSize[1] / 2,
						expected[colorID],
						msg + " (" + x + "," + y + ")", 0));
					rects.push(wtu.makeCheckRect(
						center_x / 2 - Math.floor(pixelCheckSize[0] / 4),
						center_y / 2 - Math.floor(pixelCheckSize[1] / 4) + height / 2,
						pixelCheckSize[0] / 2,
						pixelCheckSize[1] / 2,
						expected[colorID],
						msg + " (" + x + "," + y + ")", 0));
					rects.push(wtu.makeCheckRect(
						center_x / 2 - Math.floor(pixelCheckSize[0] / 4) + width / 2,
						center_y / 2 - Math.floor(pixelCheckSize[1] / 4) + height / 2,
						pixelCheckSize[0] / 2,
						pixelCheckSize[1] / 2,
						expected[colorID],
						msg + " (" + x + "," + y + ")", 0));
				}
				else {
					rects.push(wtu.makeCheckRect(
						center_x - Math.floor(pixelCheckSize[0] / 2),
						center_y - Math.floor(pixelCheckSize[1] / 2),
						pixelCheckSize[0],
						pixelCheckSize[1],
						expected[colorID],
						msg + " (" + x + "," + y + ")", 0));
				}
			}
		}
		wtu.checkCanvasRects(gl, rects);
	}
#    endif

	void DoVertexAttribDivisor(GLint location, GLuint divisor)
	{
		if (getClientMajorVersion() <= 2)
		{
			ASSERT_TRUE(IsGLExtensionEnabled("GL_ANGLE_instanced_arrays"));
			glVertexAttribDivisorANGLE(location, divisor);
		}
		else
		{
			glVertexAttribDivisor(location, divisor);
		}
	}

	void runPixelTests() {
		// An array of quads is tiled across the screen.
		// gl_DrawID is checked by using it to select the color of the draw.
		// Instanced entrypoints are tested here scaling and then instancing the
		// array of quads over four quadrants on the screen.

		// These tests also include "manyDraw" tests which emulate a multiDraw with
		// a Javascript for-loop and gl_DrawID with a uniform constiable. They are
		// included to ensure the test is written correctly.

		SetupBuffers();

		std::vector<GLint> firsts(kTriCount);
		std::vector<GLsizei> counts(kTriCount, 3);
		for (uint32_t i = 0; i < kTriCount; ++i)
			firsts[i] = i * 3;

		std::vector<GLsizei> offsets(kTriCount);
		std::vector<GLsizei> instances(kTriCount, 4);

		for (uint32_t i = 0; i < kTriCount; ++i) offsets[i] = i * 3 * 2;

		const GLsizei firstsOffset = 47;
		const GLsizei countsOffset = firstsOffset + firsts.size();
		const GLsizei offsetsOffset = countsOffset + counts.size();
		const GLsizei instancesOffset = offsetsOffset + instances.size();

		std::vector<GLuint> buffer(firstsOffset + firsts.size() + counts.size() + offsets.size() + instances.size());
		std::copy(firsts.data(), &firsts[kTriCount], buffer.begin() + firstsOffset);
		std::copy(counts.data(), &counts[kTriCount], buffer.begin() + countsOffset);
		std::copy(offsets.data(), &offsets[kTriCount], buffer.begin() + offsetsOffset);
		std::copy(instances.data(), &instances[kTriCount], buffer.begin() + instancesOffset);

		const multiDrawElementsInstanced = function() {
			ext.multiDrawElementsInstancedWEBGL(GL_TRIANGLES, counts, 0, gl.UNSIGNED_SHORT, offsets, 0, instances, 0, kTriCount);
		}

#    if 0
		function checkDraw(config) {
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			if (config.indexed) {
				glBindBuffer(gl.ELEMENT_ARRAY_BUFFER, indexBuffer);
				glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
				glEnableVertexAttribArray(0);
				glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
			}
			else {
				glBindBuffer(GL_ARRAY_BUFFER, nonIndexedVertexBuffer);
				glEnableVertexAttribArray(0);
				glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
			}

			if (config.instanced) {
				glBindBuffer(GL_ARRAY_BUFFER, instanceBuffer);
				glEnableVertexAttribArray(1);
				glVertexAttribPointer(1, 1, GL_FLOAT, false, 0, 0);
				DoVertexAttribDivisor(1, 1);
			}

			config.drawFunc();
			ASSERT_GL_NO_ERROR();

			checkResult(config, config.drawFunc.name + (
				config.instanced ? ' instanced' : ''
				) + (
					config.drawID ? ' with gl_DrawID' : ''
					));

			glDisableVertexAttribArray(0);
			glDisableVertexAttribArray(1);
		}
#    endif

		const noDrawIDProgram = wtu.setupProgram(gl, ["vshaderNoDrawID", "fshader"], ["vPosition", "vInstance"], [0, 1]);
		expectTrue(noDrawIDProgram != null, "can compile simple program");
		if (noDrawIDProgram) {
			gl.useProgram(noDrawIDProgram);

			if (!instanced) {
			}
			else {
				// Required
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


				glBindBuffer(gl.ELEMENT_ARRAY_BUFFER, indexBuffer);
				glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
				glEnableVertexAttribArray(0);
				glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);

				glBindBuffer(GL_ARRAY_BUFFER, instanceBuffer);
				glEnableVertexAttribArray(1);
				glVertexAttribPointer(1, 1, GL_FLOAT, false, 0, 0);
				DoVertexAttribDivisor(1, 1);

				glMultiDrawElementsInstancedANGLE(GL_TRIANGLES, counts.data(), GL_UNSIGNED_SHORT, offsets.data(), instances.data(), kTriCount);
				// multiDrawElementsInstancedWEBGL(GL_TRIANGLES, counts, 0, gl.UNSIGNED_SHORT, offsets, 0, instances, 0, kTriCount);
				ASSERT_GL_NO_ERROR();

				checkResult(config, config.drawFunc.name + (
					config.instanced ? ' instanced' : ''
					) + (
						config.drawID ? ' with gl_DrawID' : ''
						));

				glDisableVertexAttribArray(0);
				glDisableVertexAttribArray(1);

				checkDraw({
				drawFunc: multiDrawElementsInstanced,
						  indexed : true,
						  drawID : false,
						  instanced : true,
					});
			}
		}
	}

	std::vector<GLubyte> mIndices;
	std::vector<GLfloat> mVertices;
	std::vector<GLfloat> mInstances;
	GLuint mNonIndexedVertexBuffer;
	GLuint mVertexBuffer;
	GLuint mIndexBuffer;
	GLuint mInstanceBuffer;
	GLuint mProgram;
	GLint mPositionLoc;
	GLint mInstanceLoc;
};



void doTest(const char *extensionName, bool instanced) {
	const ext = gl.getExtension(extensionName);

	// TODO: Needs GLES 3.0



	// runValidationTests required for error
	runValidationTests();
	// runShaderTests();
	runPixelTests();
}

// Implement test subset from WebGL/sdk/tests/conformance/extensions/webgl-multi-draw.html
TEST_P(MultiDraw, WebGL)
{
	GL_ANGLE_multi_draw
	doTest('WEBGL_multi_draw', false);
	doTest('WEBGL_multi_draw_instanced', true);
}

#endif

const angle::PlatformParameters platforms[] = {
    ES2_D3D9(),  ES2_OPENGL(), ES2_OPENGLES(), ES2_VULKAN(),
    ES3_D3D11(), ES3_OPENGL(), ES3_OPENGLES(),
};

const angle::PlatformParameters es2_platforms[] = {
    ES2_D3D9(),
    ES2_OPENGL(),
    ES2_OPENGLES(),
    ES2_VULKAN(),
};

INSTANTIATE_TEST_SUITE_P(
    ,
    MultiDrawTest,
    testing::Combine(testing::ValuesIn(::angle::FilterTestParams(platforms, ArraySize(platforms))),
                     testing::Values(DrawIDOption::NoDrawID, DrawIDOption::UseDrawID),
                     testing::Values(InstancingOption::NoInstancing,
                                     InstancingOption::UseInstancing)),
    PrintToStringParamName());

INSTANTIATE_TEST_SUITE_P(
    ,
    MultiDrawNoInstancingSupportTest,
    testing::Combine(testing::ValuesIn(::angle::FilterTestParams(es2_platforms,
                                                                 ArraySize(es2_platforms))),
                     testing::Values(DrawIDOption::NoDrawID, DrawIDOption::UseDrawID),
                     testing::Values(InstancingOption::UseInstancing)),
    PrintToStringParamName());

}  // namespace
