//
// Copyright (c) 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ErrorStrings.h: Contains mapping of commonly used error messages

#define ERRMSG(name, message) static constexpr char *g_kError##name = ##message
#define GET_ERROR_MESSAGE(name) g_kError##name

namespace gl
{
ERRMSG(InvalidDrawMode, "Invalid draw mode.");
ERRMSG(ShaderAttachmentHasShader, "Shader attachment already has a shader.");
ERRMSG(UniformSizeDoesNotMatchMethod, "Uniform size does not match uniform method.");
ERRMSG(InvalidBlendFunction, "Invalid blend function.");
ERRMSG(InvalidBlendEquation, "Invalid blend equation.");
ERRMSG(ResourceMaxTextureSize, "Desired resource size is greater than max texture size.");
ERRMSG(ViewportNegativeSize, "Viewport size cannot be negative.");
ERRMSG(NegativeCount, "Negative count.");
ERRMSG(NegativeOffset, "Negative offset.");
ERRMSG(NegativeLength, "Negative length.");
ERRMSG(MustHaveElementArrayBinding, "Must have element array buffer binding.");
ERRMSG(InvalidBufferTypes, "Invalid buffer target enum.");
ERRMSG(InvalidBufferUsage, "Invalid buffer usage enum.");
ERRMSG(InvalidShaderType, "Invalid shader type.");
ERRMSG(ClearInvalidMask, "Invalid mask bits.");
ERRMSG(NegativeSize, "Cannot have negative height or width.");
ERRMSG(InsufficientBufferSize, "Insufficient buffer size.");
ERRMSG(NegativeStart, "Cannot have negative start.");
ERRMSG(OutsideOfBounds, "Parameter outside of bounds.");
ERRMSG(ProgramNotLinked, "Program not linked.");
ERRMSG(MismatchedVariableProgram, "Variable is not part of the current program.");
ERRMSG(DepthRange, "Near value cannot be greater than far.");
ERRMSG(ActiveTextureRange, "Cannot be less than 0 or greater than maximum number of textures.");
ERRMSG(TextureTarget, "Invalid or unsupported texture target.");
ERRMSG(TextureNotBound, "A texture must be bound.");
ERRMSG(InvalidFormat, "Invalid format.");
ERRMSG(InvalidType, "Invalid type.");
ERRMSG(MismatchedFormat, "Format must match internal format.");
ERRMSG(MismatchedTypeAndFormat, "Invalid format and type combination.");
ERRMSG(InvalidMipLevel, "Level of detail outside of range.");
ERRMSG(InvalidInternalFormat, "Invalid internal format.");
ERRMSG(InvalidBorder, "Border must be 0.");
ERRMSG(NegativeStride, "Cannot have negative stride.");
ERRMSG(VertexAttrSize, "Vertex attribute size must be 1, 2, 3, or 4.");
ERRMSG(BufferNotBound, "A buffer must be bound.");
ERRMSG(InvalidPrecision, "Invalid or unsupported precision type.");
ERRMSG(ParamOverflow, "The provided parameters overflow with the provided buffer.");
ERRMSG(InvalidShaderName, "Shader object expected.");
ERRMSG(InvalidProgramName, "Program object expected.");
ERRMSG(InvalidTextureFilterParam, "Texture filter not recognized.");
ERRMSG(InvalidCullMode, "Cull mode not recognized.");
ERRMSG(InvalidRenderbufferTextureParameter, "Invalid parameter name for renderbuffer attachment.");
ERRMSG(InvalidRenderbufferTarget, "Invalid renderbuffer target.");
ERRMSG(InvalidFramebufferTarget, "Invalid framebuffer target.");
ERRMSG(RenderbufferNotBound, "A renderbuffer must be bound.");
ERRMSG(InvalidFramebufferTextureLevel, "Mipmap level must be 0 when attaching a texture.");
ERRMSG(InvalidRenderbufferWidthHeight,
       "Renderbuffer width and height cannot be negative and cannot exceed maximum texture size.");
ERRMSG(FramebufferIncompleteAttachment,
       "Attachment type must be compatible with attachment object.");
ERRMSG(InvalidRenderbufferInternalFormat, "Invalid renderbuffer internalformat.");
ERRMSG(EnumNotSupported, "Enum is not currently supported.");
ERRMSG(TextureNotPow2, "The texture is a non-power-of-two texture.");
ERRMSG(SubtextureDimensions, "The specified dimensions are outside of the bounds of the texture.");
ERRMSG(ObjectNotGenerated, "Object cannot be used because it has not been generated.");
ERRMSG(NameBeginsWithGL, "Attributes that begin with 'gl_' are not allowed.");
ERRMSG(InvalidUnpackAlignment, "Unpack alignment must be 1, 2, 4, or 8.");
ERRMSG(TypeMustMatchOriginalType,
       "Passed in texture target and format must match the one originally used to define the "
       "texture.");
ERRMSG(CubemapFacesEqualDimensions, "Each cubemap face must have equal width and height.");
ERRMSG(CubemapComplete,
       "Texture is not cubemap complete. All cubemaps faces must be defined and be the same size.");
ERRMSG(ShaderToDetachMustBeAttached,
       "Shader to be detached must be currently attached to the program.");
ERRMSG(GenerateMipmapNotAllowed, "Compressed textures do not support mipmap generation.");
ERRMSG(BufferDataNull, "bufferData must be passed an array. Null is not valid.");
ERRMSG(BufferSubDataNull, "bufferSubData expects an array. Null is not a valid value.");
ERRMSG(OffsetMustBeMultipleOfType, "Offset must be a multiple of the passed in datatype.");
ERRMSG(StrideMustBeMultipleOfType, "Stride must be a multiple of the passed in datatype.");
ERRMSG(StencilReferenceMaskOrMismatch,
       "Stencil reference and mask values must be the same for front facing and back facing "
       "triangles.");
ERRMSG(
    SubtextureFormatType,
    "Passed in texture format and type must match the one originally used to define the texture.");
ERRMSG(WebglNameLengthLimitExceeded, "Location lengths must not be greater than 256 characters.");
ERRMSG(WebglBindAttribLocationReservedPrefix,
       "Attributes that begin with 'gl_', 'webgl_', or '_webgl_' are not allowed.");
ERRMSG(InvalidConstantColor,
       "CONSTANT_COLOR (or ONE_MINUS_CONSTANT_COLOR) and CONSTANT_ALPHA (or "
       "ONE_MINUS_CONSTANT_ALPHA) cannot be used together as source and destination factors in the "
       "blend function.");
ERRMSG(TypeNotUnsignedShortByte, "Only UNSIGNED_SHORT and UNSIGNED_BYTE types are supported.");
ERRMSG(InvalidStencil, "Invalid stencil.");
ERRMSG(ExceedsMaxElementErrorMessage, "Element value exceeds maximum element index.");
ERRMSG(ExtensionNotEnabled, "Extension is not enabled.");
ERRMSG(UnknownPname, "Unknown pname.");
ERRMSG(MissingReadAttachment, "Missing read attachment.");
ERRMSG(NameInvalidCharacters, "Name contains invalid characters.");
ERRMSG(ShaderSourceInvalidCharacters, "Shader source contains invalid characters.");
ERRMSG(IndexExceedsMax, "Index exceeds MAX_VERTEX_ATTRIBS");
ERRMSG(IntegerOverflow, "Integer overflow.");
ERRMSG(ExpectedShaderName, "Expected a shader name, but found a program name.");
ERRMSG(ExpectedProgramName, "Expected a program name, but found a shader name.");
}
