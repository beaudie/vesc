//
// Copyright (c) 2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// validationES.h: Validation functions for generic OpenGL ES entry point parameters

#ifndef LIBGLESV2_VALIDATION_ES_H
#define LIBGLESV2_VALIDATION_ES_H

#include "common/mathutil.h"
#include "libGLESv2/Error.h"

#include <GLES2/gl2.h>
#include <GLES3/gl3.h>

namespace gl
{

class Context;
class State;
struct Caps;
class TextureCapsMap;
struct Extensions;
struct TextureCaps;

struct ValidateResult
{
    ValidateResult(const Error &errorIn, bool callHasNoSideEffectsIn)
        : error(errorIn),
          callHasNoSideEffects(callHasNoSideEffectsIn)
    {}

    // This test indicates the call did not produce a validation error, and the
    // validator determined it may have side effects (may not be a no-op).
    bool shouldRunAPICall() const { return !error.isError() && !callHasNoSideEffects; }
    bool hasError() { return error.isError(); }

    Error error;
    bool callHasNoSideEffects;
};

class Validator
{
  public:
    explicit Validator(const Context *context);
    Validator(int clientVersion, const State &state, const Caps &caps,
              const TextureCapsMap &textureCaps, const Extensions &extensions);

    ValidateResult getResult();

  protected:
    // TODO(jmadill): varargs
    void error(GLenum value) { mError = Error(value); }
    bool hasError() { return mError.isError(); }

    int mClientVersion;
    const State &mState;
    const Caps &mCaps;
    const TextureCapsMap &mTextureCaps;
    const Extensions &mExtensions;
    Error mError;

    // The validator sets this flag to true when it knows the API call will have
    // no side effects, and we may skip any further work after validation.
    bool mCallHasNoSideEffects;
};

class ES2Validator : public Validator
{
  public:
    explicit ES2Validator(const Context *context);
    ES2Validator(int clientVersion, const State &state, const Caps &caps,
                 const TextureCapsMap &textureCaps, const Extensions &extensions);

    // Entry-point validation.
    // When implementing an entry point validation stub, make sure you call getResult()
    // to return the current ValidateResult to the API call. You should call getResult as
    // the last step before returning to the API.
    ValidateResult RenderbufferStorageMultisample(GLenum target, GLsizei samples, GLenum internalformat,
                                                  GLsizei width, GLsizei height);
    ValidateResult RenderbufferStorageMultisampleANGLE(GLenum target, GLsizei samples, GLenum internalformat,
                                                       GLsizei width, GLsizei height);

  private:
    void RenderbufferStorageMultisampleBase(GLenum target, GLsizei samples, GLenum internalformat,
                                            GLsizei width, GLsizei height);

    // cached queries
    const TextureCaps *mFormatCaps;
};

ES2Validator ValidateES2(const Context *context);

bool ValidCap(const Context *context, GLenum cap);
bool ValidTextureTarget(const Context *context, GLenum target);
bool ValidTexture2DDestinationTarget(const Context *context, GLenum target);
bool ValidFramebufferTarget(GLenum target);
bool ValidBufferTarget(const Context *context, GLenum target);
bool ValidBufferParameter(const Context *context, GLenum pname);
bool ValidMipLevel(const Context *context, GLenum target, GLint level);
bool ValidImageSize(const Context *context, GLenum target, GLint level, GLsizei width, GLsizei height, GLsizei depth);
bool ValidCompressedImageSize(const Context *context, GLenum internalFormat, GLsizei width, GLsizei height);
bool ValidQueryType(const Context *context, GLenum queryType);
bool ValidProgram(Context *context, GLuint id);

bool ValidateAttachmentTarget(Context *context, GLenum attachment);
bool ValidateFramebufferRenderbufferParameters(Context *context, GLenum target, GLenum attachment,
                                               GLenum renderbuffertarget, GLuint renderbuffer);

bool ValidateBlitFramebufferParameters(Context *context, GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1,
                                       GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask,
                                       GLenum filter, bool fromAngleExtension);

bool ValidateGetVertexAttribParameters(Context *context, GLenum pname);

bool ValidateTexParamParameters(Context *context, GLenum pname, GLint param);

bool ValidateSamplerObjectParameter(Context *context, GLenum pname);

bool ValidateReadPixelsParameters(Context *context, GLint x, GLint y, GLsizei width, GLsizei height,
                                  GLenum format, GLenum type, GLsizei *bufSize, GLvoid *pixels);

bool ValidateBeginQuery(Context *context, GLenum target, GLuint id);
bool ValidateEndQuery(Context *context, GLenum target);

bool ValidateUniform(Context *context, GLenum uniformType, GLint location, GLsizei count);
bool ValidateUniformMatrix(Context *context, GLenum matrixType, GLint location, GLsizei count,
                           GLboolean transpose);

bool ValidateStateQuery(Context *context, GLenum pname, GLenum *nativeType, unsigned int *numParams);

bool ValidateCopyTexImageParametersBase(Context* context, GLenum target, GLint level, GLenum internalformat, bool isSubImage,
                                        GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height,
                                        GLint border, GLenum *textureInternalFormatOut);

bool ValidateDrawArrays(Context *context, GLenum mode, GLint first, GLsizei count, GLsizei primcount);
bool ValidateDrawArraysInstanced(Context *context, GLenum mode, GLint first, GLsizei count, GLsizei primcount);
bool ValidateDrawArraysInstancedANGLE(Context *context, GLenum mode, GLint first, GLsizei count, GLsizei primcount);

bool ValidateDrawElements(Context *context, GLenum mode, GLsizei count, GLenum type,
                          const GLvoid* indices, GLsizei primcount, rx::RangeUI *indexRangeOut);

bool ValidateDrawElementsInstanced(Context *context, GLenum mode, GLsizei count, GLenum type,
                                   const GLvoid *indices, GLsizei primcount, rx::RangeUI *indexRangeOut);
bool ValidateDrawElementsInstancedANGLE(Context *context, GLenum mode, GLsizei count, GLenum type,
                                        const GLvoid *indices, GLsizei primcount, rx::RangeUI *indexRangeOut);

bool ValidateFramebufferTextureBase(Context *context, GLenum target, GLenum attachment,
                                    GLuint texture, GLint level);
bool ValidateFramebufferTexture2D(Context *context, GLenum target, GLenum attachment,
                                  GLenum textarget, GLuint texture, GLint level);

bool ValidateGetUniformBase(Context *context, GLuint program, GLint location);
bool ValidateGetUniformfv(Context *context, GLuint program, GLint location, GLfloat* params);
bool ValidateGetUniformiv(Context *context, GLuint program, GLint location, GLint* params);
bool ValidateGetnUniformfvEXT(Context *context, GLuint program, GLint location, GLsizei bufSize, GLfloat* params);
bool ValidateGetnUniformivEXT(Context *context, GLuint program, GLint location, GLsizei bufSize, GLint* params);

}

#endif // LIBGLESV2_VALIDATION_ES_H
