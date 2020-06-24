//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ProgramImpl_mock.h:
//   Defines a mock of the ProgramImpl class.
//

#ifndef LIBANGLE_RENDERER_PROGRAMIMPLMOCK_H_
#define LIBANGLE_RENDERER_PROGRAMIMPLMOCK_H_

#include "gmock/gmock.h"

#include "libANGLE/ProgramLinkedResources.h"
#include "libANGLE/renderer/ProgramImpl.h"

namespace rx
{

class MockProgramImpl : public rx::ProgramImpl
{
  public:
    MockProgramImpl() : ProgramImpl(gl::ProgramState()) {}
    virtual ~MockProgramImpl() { destructor(); }

    MOCK_METHOD(angle::Result,
                load,
                (const gl::Context *, gl::InfoLog &, gl::BinaryInputStream *),
                (override));
    MOCK_METHOD(void, save, (const gl::Context *, gl::BinaryOutputStream *), (override));
    MOCK_METHOD(void, setBinaryRetrievableHint, (bool), (override));
    MOCK_METHOD(void, setSeparable, (bool), (override));

    MOCK_METHOD(std::unique_ptr<LinkEvent>,
                link,
                (const gl::Context *, const gl::ProgramLinkedResources &, gl::InfoLog &),
                (override));
    MOCK_METHOD(GLboolean, validate, (const gl::Caps &, gl::InfoLog *), (override));

    MOCK_METHOD(void, setUniform1fv, (GLint, GLsizei, const GLfloat *), (override));
    MOCK_METHOD(void, setUniform2fv, (GLint, GLsizei, const GLfloat *), (override));
    MOCK_METHOD(void, setUniform3fv, (GLint, GLsizei, const GLfloat *), (override));
    MOCK_METHOD(void, setUniform4fv, (GLint, GLsizei, const GLfloat *), (override));
    MOCK_METHOD(void, setUniform1iv, (GLint, GLsizei, const GLint *), (override));
    MOCK_METHOD(void, setUniform2iv, (GLint, GLsizei, const GLint *), (override));
    MOCK_METHOD(void, setUniform3iv, (GLint, GLsizei, const GLint *), (override));
    MOCK_METHOD(void, setUniform4iv, (GLint, GLsizei, const GLint *), (override));
    MOCK_METHOD(void, setUniform1uiv, (GLint, GLsizei, const GLuint *), (override));
    MOCK_METHOD(void, setUniform2uiv, (GLint, GLsizei, const GLuint *), (override));
    MOCK_METHOD(void, setUniform3uiv, (GLint, GLsizei, const GLuint *), (override));
    MOCK_METHOD(void, setUniform4uiv, (GLint, GLsizei, const GLuint *), (override));

    MOCK_METHOD(void,
                setUniformMatrix2fv,
                (GLint, GLsizei, GLboolean, const GLfloat *),
                (override));
    MOCK_METHOD(void,
                setUniformMatrix3fv,
                (GLint, GLsizei, GLboolean, const GLfloat *),
                (override));
    MOCK_METHOD(void,
                setUniformMatrix4fv,
                (GLint, GLsizei, GLboolean, const GLfloat *),
                (override));
    MOCK_METHOD(void,
                setUniformMatrix2x3fv,
                (GLint, GLsizei, GLboolean, const GLfloat *),
                (override));
    MOCK_METHOD(void,
                setUniformMatrix3x2fv,
                (GLint, GLsizei, GLboolean, const GLfloat *),
                (override));
    MOCK_METHOD(void,
                setUniformMatrix2x4fv,
                (GLint, GLsizei, GLboolean, const GLfloat *),
                (override));
    MOCK_METHOD(void,
                setUniformMatrix4x2fv,
                (GLint, GLsizei, GLboolean, const GLfloat *),
                (override));
    MOCK_METHOD(void,
                setUniformMatrix3x4fv,
                (GLint, GLsizei, GLboolean, const GLfloat *),
                (override));
    MOCK_METHOD(void,
                setUniformMatrix4x3fv,
                (GLint, GLsizei, GLboolean, const GLfloat *),
                (override));

    MOCK_METHOD(void, getUniformfv, (const gl::Context *, GLint, GLfloat *), (const override));
    MOCK_METHOD(void, getUniformiv, (const gl::Context *, GLint, GLint *), (const override));
    MOCK_METHOD(void, getUniformuiv, (const gl::Context *, GLint, GLuint *), (const override));

    MOCK_METHOD(void,
                setPathFragmentInputGen,
                (const std::string &, GLenum, GLint, const GLfloat *),
                (override));

    MOCK_METHOD(void, destructor, ());
};

inline ::testing::NiceMock<MockProgramImpl> *MakeProgramMock()
{
    ::testing::NiceMock<MockProgramImpl> *programImpl = new ::testing::NiceMock<MockProgramImpl>();
    // TODO(jmadill): add ON_CALLS for returning methods
    // We must mock the destructor since NiceMock doesn't work for destructors.
    EXPECT_CALL(*programImpl, destructor()).Times(1).RetiresOnSaturation();

    return programImpl;
}

}  // namespace rx

#endif  // LIBANGLE_RENDERER_PROGRAMIMPLMOCK_H_
