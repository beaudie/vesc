//
// Copyright (c) 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ErrorStrings.h: Contains mapping of commonly used error messages

namespace gl
{
static constexpr char *g_kErrorInvalidDrawMode = "Invalid draw mode.";
static constexpr char *g_kErrorShaderAttachmentHasShader =
    "Shader attachment already has a shader.";
static constexpr char *g_kErrorUniformSizeDoesNotMatchMethod =
    "Uniform size does not match uniform method.";
static constexpr char *g_kErrorInvalidBlendFunction = "Invalid blend function.";
static constexpr char *g_kErrorInvalidBlendEquation = "Invalid blend equation.";
static constexpr char *g_kErrorResourceMaxTextureSize =
    "Desired resource size is greater than max texture size.";
static constexpr char *g_kErrorViewportNegativeSize = "Viewport size cannot be negative.";
static constexpr char *g_kErrorNegativeCount        = "Negative count.";
static constexpr char *g_kErrorNegativeOffset       = "Negative offset.";
static constexpr char *g_kErrorNegativeLength       = "Negative length.";
static constexpr char *g_kErrorMustHaveElementArrayBinding =
    "Must have element array buffer binding.";
static constexpr char *g_kErrorInvalidBufferTypes     = "Invalid buffer target enum.";
static constexpr char *g_kErrorInvalidBufferUsage     = "Invalid buffer usage enum.";
static constexpr char *g_kErrorInvalidShaderType      = "Invalid shader type.";
static constexpr char *g_kErrorClearInvalidMask       = "Invalid mask bits.";
static constexpr char *g_kErrorNegativeSize           = "Cannot have negative height or width.";
static constexpr char *g_kErrorInsufficientBufferSize = "Insufficient buffer size.";
static constexpr char *g_kErrorNegativeStart          = "Cannot have negative start.";
static constexpr char *g_kErrorOutsideOfBounds        = "Parameter outside of bounds.";
static constexpr char *g_kErrorProgramNotLinked       = "Program not linked.";
static constexpr char *g_kErrorMismatchedVariableProgram =
    "Variable is not part of the current program.";
static constexpr char *g_kErrorDepthRange = "Near value cannot be greater than far.";
static constexpr char *g_kErrorActiveTextureRange =
    "Cannot be less than 0 or greater than maximum number of textures.";
static constexpr char *g_kErrorTextureTarget           = "Invalid or unsupported texture target.";
static constexpr char *g_kErrorTextureNotBound         = "A texture must be bound.";
static constexpr char *g_kErrorInvalidFormat           = "Invalid format.";
static constexpr char *g_kErrorInvalidType             = "Invalid type.";
static constexpr char *g_kErrorMismatchedFormat        = "Format must match internal format.";
static constexpr char *g_kErrorMismatchedTypeAndFormat = "Invalid format and type combination.";
static constexpr char *g_kErrorInvalidMipLevel         = "Level of detail outside of range.";
static constexpr char *g_kErrorInvalidInternalFormat   = "Invalid internal format.";
static constexpr char *g_kErrorInvalidBorder           = "Border must be 0.";
static constexpr char *g_kErrorNegativeStride          = "Cannot have negative stride.";
static constexpr char *g_kErrorVertexAttrSize   = "Vertex attribute size must be 1, 2, 3, or 4.";
static constexpr char *g_kErrorBufferNotBound   = "A buffer must be bound.";
static constexpr char *g_kErrorInvalidPrecision = "Invalid or unsupported precision type.";
static constexpr char *g_kErrorParamOverflow =
    "The provided parameters overflow with the provided buffer.";
static constexpr char *g_kErrorInvalidShaderName         = "Shader object expected.";
static constexpr char *g_kErrorInvalidProgramName        = "Program object expected.";
static constexpr char *g_kErrorInvalidTextureFilterParam = "Texture filter not recognized.";
static constexpr char *g_kErrorInvalidCullMode           = "Cull mode not recognized.";
static constexpr char *g_kErrorInvalidRenderbufferTextureParameter =
    "Invalid parameter name for renderbuffer attachment.";
static constexpr char *g_kErrorInvalidRenderbufferTarget = "Invalid renderbuffer target.";
static constexpr char *g_kErrorInvalidFramebufferTarget  = "Invalid framebuffer target.";
static constexpr char *g_kErrorRenderbufferNotBound      = "A renderbuffer must be bound.";
static constexpr char *g_kErrorInvalidFramebufferTextureLevel =
    "Mipmap level must be 0 when attaching a texture.";
static constexpr char *g_kErrorInvalidRenderbufferWidthHeight =
    "Renderbuffer width and height cannot be negative and cannot exceed maximum texture size.";
static constexpr char *g_kErrorFramebufferIncompleteAttachment =
    "Attachment type must be compatible with attachment object.";
static constexpr char *g_kErrorInvalidRenderbufferInternalFormat =
    "Invalid renderbuffer internalformat.";
static constexpr char *g_kErrorEnumNotSupported = "Enum is not currently supported.";
static constexpr char *g_kErrorTextureNotPow2   = "The texture is a non-power-of-two texture.";
static constexpr char *g_kErrorSubtextureDimensions =
    "The specified dimensions are outside of the bounds of the texture.";
static constexpr char *g_kErrorObjectNotGenerated =
    "Object cannot be used because it has not been generated.";
static constexpr char *g_kErrorNameBeginsWithGL =
    "Attributes that begin with 'gl_' are not allowed.";
static constexpr char *g_kErrorInvalidUnpackAlignment = "Unpack alignment must be 1, 2, 4, or 8.";
static constexpr char *g_kErrorTypeMustMatchOriginalType =
    "Passed in texture target and format must match the one originally used to define the texture.";
static constexpr char *g_kErrorCubemapFacesEqualDimensions =
    "Each cubemap face must have equal width and height.";
static constexpr char *g_kErrorCubemapComplete =
    "Texture is not cubemap complete. All cubemaps faces must be defined and be the same size.";
static constexpr char *g_kErrorShaderToDetachMustBeAttached =
    "Shader to be detached must be currently attached to the program.";
static constexpr char *g_kErrorGenerateMipmapNotAllowed =
    "Compressed textures do not support mipmap generation.";
static constexpr char *g_kErrorBufferDataNull =
    "bufferData must be passed an array. Null is not valid.";
static constexpr char *g_kErrorBufferSubDataNull =
    "bufferSubData expects an array. Null is not a valid value.";
static constexpr char *g_kErrorOffsetMustBeMultipleOfType =
    "Offset must be a multiple of the passed in datatype.";
static constexpr char *g_kErrorStrideMustBeMultipleOfType =
    "Stride must be a multiple of the passed in datatype.";
static constexpr char *g_kErrorStencilReferenceMaskOrMismatch =
    "Stencil reference and mask values must be the same for front facing and back facing "
    "triangles.";
static constexpr char *g_kErrorSubtextureFormatType =
    "Passed in texture format and type must match the one originally used to define the texture.";
static constexpr char *g_kErrorWebglNameLengthLimitExceeded =
    "Location lengths must not be greater than 256 characters.";
static constexpr char *g_kErrorWebglBindAttribLocationReservedPrefix =
    "Attributes that begin with 'gl_', 'webgl_', or '_webgl_' are not allowed.";
static constexpr char *g_kErrorInvalidConstantColor =
    "CONSTANT_COLOR (or ONE_MINUS_CONSTANT_COLOR) and CONSTANT_ALPHA (or ONE_MINUS_CONSTANT_ALPHA) "
    "cannot be used together as source and destination factors in the blend function.";
static constexpr char *g_kErrorTypeNotUnsignedShortByte =
    "Only UNSIGNED_SHORT and UNSIGNED_BYTE types are supported.";
static constexpr char *g_kErrorInvalidStencil = "Invalid stencil.";
static constexpr char *g_kErrorExceedsMaxElementErrorMessage =
    "Element value exceeds maximum element index.";
static constexpr char *g_kErrorExtensionNotEnabled   = "Extension is not enabled.";
static constexpr char *g_kErrorUnknownPname          = "Unknown pname.";
static constexpr char *g_kErrorMissingReadAttachment = "Missing read attachment.";
static constexpr char *g_kErrorNameInvalidCharacters = "Name contains invalid characters.";
static constexpr char *g_kErrorShaderSourceInvalidCharacters =
    "Shader source contains invalid characters.";
static constexpr char *g_kErrorIndexExceedsMax = "Index exceeds MAX_VERTEX_ATTRIBS";
static constexpr char *g_kErrorIntegerOverflow = "Integer overflow.";
}
