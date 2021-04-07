//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// OutputSPIRV: Generate SPIR-V from the AST.
//

#include "compiler/translator/OutputSPIRV.h"

#include "angle_gl.h"
#include "common/debug.h"
#include "common/hash_utils.h"
#include "common/mathutil.h"
#include "common/spirv/spirv_instruction_builder_autogen.h"
#include "compiler/translator/Compiler.h"
#include "compiler/translator/ValidateVaryingLocations.h"
#include "compiler/translator/tree_util/IntermTraverse.h"
#include "compiler/translator/util.h"

#include <cfloat>

// SPIR-V tools include for disassembly
#include <spirv-tools/libspirv.hpp>

// Enable this for debug logging of pre-transform SPIR-V:
#if !defined(ANGLE_DEBUG_SPIRV_TRANSFORMER)
#    define ANGLE_DEBUG_SPIRV_TRANSFORMER 1
#endif  // !defined(ANGLE_DEBUG_SPIRV_TRANSFORMER)

namespace spirv = angle::spirv;

namespace sh
{
namespace
{

// Helper classes to map types to ids
struct SpirvType
{
    // If struct or interface block, the type is identified by the pointer.  Note that both
    // TStructure and TInterfaceBlock inherit from TFieldListCollection, and their difference is
    // irrelevant as far as SPIR-V type is concerned.
    const TFieldListCollection *block = nullptr;

    // If a structure is used in two interface blocks with different layouts, it would have
    // to generate two SPIR-V types, as its fields' Offset decorations could be different.
    // For non-block types, when used in an interface block as an array, they could generate
    // different ArrayStride decorations.  As such, the block storage is part of the SPIR-V type
    // except for non-block non-array types.
    TLayoutBlockStorage blockStorage = EbsUnspecified;

    // Otherwise, it's a basic type + column, row and array dimensions, or it's an image
    // declaration.
    //
    // Notes:
    //
    // - `precision` turns into a RelaxedPrecision decoration on the variable and instructions.
    // - `precise` turns into a NoContraction decoration on the instructions.  Probably need to fix
    //   the "promote" code to propagate `precise`.
    // - `readonly`, `writeonly`, `coherent`, `volatile` and `restrict` only apply to memory object
    //    declarations
    // - `invariant` only applies to variable or members of a block
    // - `matrixPacking` only applies to members of a struct
    TBasicType type = EbtFloat;

    unsigned char primarySize          = 1;
    unsigned char secondarySize        = 1;
    TLayoutMatrixPacking matrixPacking = EmpColumnMajor;

    TSpan<const unsigned int> arraySizes;

    // Only useful for image types.
    TLayoutImageInternalFormat imageInternalFormat = EiifUnspecified;

    // For sampled images (i.e. GLSL samplers), there are two type ids; one is the OpTypeImage that
    // declares the image itself, and one OpTypeSampledImage.  `imageOnly` distinguishes between
    // these two types.  Note that for the former, the basic type is still Ebt*Sampler* to
    // distinguish it from storage images (which have a basic type of Ebt*Image*).
    bool isSamplerBaseImage = false;
};

bool operator==(const SpirvType &a, const SpirvType &b)
{
    if (a.block != b.block)
    {
        return false;
    }

    if (a.arraySizes != b.arraySizes)
    {
        return false;
    }

    // If structure or interface block, they should match by pointer (i.e. be the same block).  The
    // AST transformations are expected to keep the AST consistent by using the same structure and
    // interface block pointer between declarations and usages.  This is validated by
    // ValidateASTOptions::validateVariableReferences.
    if (a.block != nullptr)
    {
        return a.block == b.block && a.blockStorage == b.blockStorage;
    }

    // Otherwise, match by the type contents.  The AST transformations sometimes recreate types that
    // are already defined, so we can't rely on pointers being unique.
    return a.type == b.type && a.primarySize == b.primarySize &&
           a.secondarySize == b.secondarySize && a.matrixPacking == b.matrixPacking &&
           a.imageInternalFormat == b.imageInternalFormat &&
           a.isSamplerBaseImage == b.isSamplerBaseImage &&
           (a.arraySizes.empty() || a.blockStorage == b.blockStorage);
}
}  // anonymous namespace
}  // namespace sh

namespace std
{
template <>
struct hash<sh::SpirvType>
{
    size_t operator()(const sh::SpirvType &type) const
    {
        // Block storage must only affect the type if it's a block type or array type (in a block).
        ASSERT(type.blockStorage == sh::EbsUnspecified || type.block != nullptr ||
               !type.arraySizes.empty());

        size_t result = 0;

        if (!type.arraySizes.empty())
            result = angle::ComputeGenericHash(type.arraySizes.data(),
                                               type.arraySizes.size() * sizeof(type.arraySizes[0]));

        if (type.block != nullptr)
        {
            return result ^ angle::ComputeGenericHash(&type.block, sizeof(type.block)) ^
                   type.blockStorage;
        }

        static_assert(sh::EbtLast < 256, "Basic type doesn't fit in uint8_t");
        static_assert(sh::EbsLast < 8, "Block storage doesn't fit in 3 bits");
        static_assert(sh::EiifLast < 32, "Image format doesn't fit in 5 bits");
        static_assert(sh::EmpLast < 4, "Matrix packing doesn't fit in 2 bits");
        ASSERT(type.primarySize > 0 && type.primarySize <= 4);
        ASSERT(type.secondarySize > 0 && type.secondarySize <= 4);

        const uint8_t properties[4] = {
            type.type,
            (type.primarySize - 1) | (type.secondarySize - 1) << 2 | type.isSamplerBaseImage << 4 |
                type.matrixPacking << 5,
            type.blockStorage | type.imageInternalFormat << 3,
        };

        return result ^ angle::ComputeGenericHash(properties, sizeof(properties));
    }
};

template <>
struct hash<std::pair<spirv::IdRef, spirv::IdRefList>>
{
    size_t operator()(const std::pair<spirv::IdRef, spirv::IdRefList> &key) const
    {
        return angle::ComputeGenericHash(key.second.data(),
                                         key.second.size() & sizeof(key.second[0])) ^
               key.first;
    }
};

template <>
struct hash<std::pair<spirv::IdRef, spv::StorageClass>>
{
    size_t operator()(const std::pair<spirv::IdRef, spv::StorageClass> &key) const
    {
        ASSERT(key.second < 16);
        return key.second | key.first << 4;
    }
};
}  // namespace std

namespace sh
{
namespace
{
// Data tracked per SPIR-V type (keyed by SpirvType).
struct SpirvTypeData
{
    // The SPIR-V id corresponding to the type.
    spirv::IdRef id;
    // The base alignment and size of the type based on the storage block it's used in (if
    // applicable)
    uint32_t baseAlignment;
    uint32_t sizeInStorageBlock;
};

// Helper class to construct SPIR-V
class SPIRVBuilder : angle::NonCopyable
{
  public:
    SPIRVBuilder(gl::ShaderType shaderType, ShHashFunction64 hashFunction, NameMap &nameMap)
        : mShaderType(shaderType),
          mNextAvailableId(1),
          mHashFunction(hashFunction),
          mNameMap(nameMap),
          mNextUnusedBinding(0),
          mNextUnusedInputLocation(0),
          mNextUnusedOutputLocation(0)
    {}

    spirv::IdRef getNewId();
    const SpirvTypeData &getTypeData(const TType &type, TLayoutBlockStorage blockStorage);
    const SpirvTypeData &getSpirvTypeData(const SpirvType &type, const char *blockName);
    spirv::IdRef getTypePointerId(spirv::IdRef typeId, spv::StorageClass storageClass);
    spirv::IdRef getFunctionTypeId(spirv::IdRef returnTypeId, const spirv::IdRefList &paramTypeIds);

    spirv::Blob *getSpirvExecutionModes() { return &mSpirvExecutionModes; }
    spirv::Blob *getSpirvDebug() { return &mSpirvDebug; }
    spirv::Blob *getSpirvDecorations() { return &mSpirvDecorations; }
    spirv::Blob *getSpirvVariableDecls() { return &mSpirvVariableDecls; }
    spirv::Blob *getSpirvFunctions() { return &mSpirvFunctions; }

    void addCapability(spv::Capability capability);
    void addExecutionMode(spv::ExecutionMode executionMode);
    void setEntryPointId(spirv::IdRef id);
    void addEntryPointInterfaceVariableId(spirv::IdRef id);
    void writeInterfaceVariableDecorations(const TType &type, spirv::IdRef variableId);

    uint32_t calculateBaseAlignmentAndSize(const SpirvType &type, uint32_t *sizeInStorageBlockOut);
    uint32_t calculateSizeAndWriteOffsetDecorations(const SpirvType &type, spirv::IdRef typeId);

    // TODO: remove name hashing once translation through glslang is removed.  That is necessary to
    // avoid name collision between ANGLE's internal symbols and user-defined ones when compiling
    // the generated GLSL, but is irrelevant when generating SPIR-V directly.  Currently, the SPIR-V
    // transformer relies on the "mapped" names, which should also be changed when this hashing is
    // removed.
    ImmutableString hashName(const TSymbol *symbol);
    ImmutableString hashTypeName(const TType &type);
    ImmutableString hashFieldName(const TField *field);
    ImmutableString hashFunctionNameIfNeeded(const TFunction *func);

    spirv::Blob getSpirv();

  private:
    SpirvTypeData declareType(const SpirvType &type, const char *blockName);

    // Helpers for type declaration.
    void getImageTypeParameters(TBasicType type,
                                spirv::IdRef *sampledTypeOut,
                                spv::Dim *dimOut,
                                spirv::LiteralInteger *depthOut,
                                spirv::LiteralInteger *arrayedOut,
                                spirv::LiteralInteger *multisampledOut,
                                spirv::LiteralInteger *sampledOut);
    spv::ImageFormat getImageFormat(TLayoutImageInternalFormat imageInternalFormat);

    spirv::IdRef getBoolConstant(bool value);
    spirv::IdRef getBasicConstantHelper(uint32_t value,
                                        TBasicType type,
                                        angle::HashMap<uint32_t, spirv::IdRef> *constants);
    spirv::IdRef getUintConstant(uint32_t value);
    spirv::IdRef getIntConstant(int32_t value);
    spirv::IdRef getFloatConstant(float value);
    spirv::IdRef getCompositeConstant(spirv::IdRef typeId, const spirv::IdRefList &values);

    uint32_t nextUnusedBinding();
    uint32_t nextUnusedInputLocation(uint32_t consumedCount);
    uint32_t nextUnusedOutputLocation(uint32_t consumedCount);

    gl::ShaderType mShaderType;

    // Capabilities the shader is using.  Accumulated as the instructions are generated.  The Shader
    // capability is unconditionally generated, so it's not tracked.
    std::set<spv::Capability> mCapabilities;

    // Execution modes the shader is enabling.  Accumulated as the instructions are generated.
    // Execution mode instructions that require a parameter are written to mSpirvExecutionModes as
    // instructions; they are always generated once so don't benefit from being in a std::set.
    std::set<spv::ExecutionMode> mExecutionModes;

    // The list of interface variables and the id of main() populated as the instructions are
    // generated.  Used for the OpEntryPoint instruction.
    spirv::IdRefList mEntryPointInterfaceList;
    spirv::IdRef mEntryPointId;

    // Current ID bound, used to allocate new ids.
    spirv::IdRef mNextAvailableId;

    // A map from the AST type to the corresponding SPIR-V ID and associated data.  Note that TType
    // includes a lot of information that pertains to the variable that has the type, not the type
    // itself.  SpirvType instead contains only information that can identify the type itself.
    angle::HashMap<SpirvType, SpirvTypeData> mTypeMap;

    // Various sections of SPIR-V.  Each section grows as SPIR-V is generated, and the final result
    // is obtained by stiching the sections together.  This puts the instructions in the order
    // required by the spec.
    spirv::Blob mSpirvExecutionModes;
    spirv::Blob mSpirvDebug;
    spirv::Blob mSpirvDecorations;
    spirv::Blob mSpirvTypeAndConstantDecls;
    spirv::Blob mSpirvTypePointerDecls;
    spirv::Blob mSpirvVariableDecls;
    spirv::Blob mSpirvFunctions;

#if 0
    // A map of SPIR-V id to instruction.  Useful when some property of the instruction needs to be
    // looked up.
    // TODO: probably necessary, but not yet
    struct InstructionLocation
    {
        const spirv::Blob *blob;
        size_t offset;
    }
    std::vector<InstructionLocation> mInstructionsById;
#endif

    // List of constants that are already defined (for reuse).
    spirv::IdRef mBoolConstants[2];
    angle::HashMap<uint32_t, spirv::IdRef> mUintConstants;
    angle::HashMap<uint32_t, spirv::IdRef> mIntConstants;
    angle::HashMap<uint32_t, spirv::IdRef> mFloatConstants;
    angle::HashMap<std::pair<spirv::IdRef, spirv::IdRefList>, spirv::IdRef> mCompositeConstants;
    // TODO: Use null constants as optimization for when complex types are initialized with all
    // zeros.  http://anglebug.com/4889

    // List of type pointers that are already defined.
    // TODO: if all users call getTypeData(), move to SpirvTypeData.  http://anglebug.com/4889
    angle::HashMap<std::pair<spirv::IdRef, spv::StorageClass>, spirv::IdRef> mTypePointerIdMap;

    // List of function types that are already defined.
    angle::HashMap<std::pair<spirv::IdRef, spirv::IdRefList>, spirv::IdRef> mFunctionTypeIdMap;

    // name hashing.
    ShHashFunction64 mHashFunction;
    NameMap &mNameMap;

    // Every resource that requires set & binding layout qualifiers is assigned set 0 and an
    // arbitrary binding.  Every input/output that requires a location layout qualifiers is assigned
    // an arbitrary location as well.
    //
    // The link-time SPIR-V transformer modifies set, binding and location decorations in SPIR-V
    // directly.
    uint32_t mNextUnusedBinding;
    uint32_t mNextUnusedInputLocation;
    uint32_t mNextUnusedOutputLocation;
};

spirv::IdRef SPIRVBuilder::getNewId()
{
    spirv::IdRef newId = mNextAvailableId;
    mNextAvailableId   = spirv::IdRef(mNextAvailableId + 1);
    return newId;
}

const SpirvTypeData &SPIRVBuilder::getTypeData(const TType &type, TLayoutBlockStorage blockStorage)
{
    SpirvType spirvType;
    spirvType.type                = type.getBasicType();
    spirvType.primarySize         = type.getNominalSize();
    spirvType.secondarySize       = type.getSecondarySize();
    spirvType.matrixPacking       = type.getLayoutQualifier().matrixPacking;
    spirvType.arraySizes          = type.getArraySizes();
    spirvType.imageInternalFormat = type.getLayoutQualifier().imageInternalFormat;
    spirvType.blockStorage        = blockStorage;

    // Turn unspecifed matrix packing into column-major.
    if (spirvType.matrixPacking == EmpUnspecified)
    {
        spirvType.matrixPacking = EmpColumnMajor;
    }

    const char *blockName = "";
    if (type.getStruct() != nullptr)
    {
        spirvType.block = type.getStruct();
        blockName       = type.getStruct()->name().data();
    }
    else if (type.isInterfaceBlock())
    {
        spirvType.block = type.getInterfaceBlock();
        blockName       = type.getInterfaceBlock()->name().data();

        // Calculate the block storage from the interface block automatically.  The fields inherit
        // from this.  Default to std140.
        ASSERT(spirvType.blockStorage == EbsUnspecified);
        spirvType.blockStorage = type.getLayoutQualifier().blockStorage;
        if (spirvType.blockStorage != EbsStd430)
        {
            spirvType.blockStorage = EbsStd140;
        }
    }
    else if (spirvType.arraySizes.empty())
    {
        // No difference in type for non-block non-array types in std140 and std430 block storage.
        spirvType.blockStorage = EbsUnspecified;
    }

    return getSpirvTypeData(spirvType, blockName);
}

const SpirvTypeData &SPIRVBuilder::getSpirvTypeData(const SpirvType &type, const char *blockName)
{
    auto iter = mTypeMap.find(type);
    if (iter == mTypeMap.end())
    {
        SpirvTypeData newTypeData = declareType(type, blockName);

        iter = mTypeMap.insert({type, newTypeData}).first;
    }

    return iter->second;
}

spirv::IdRef SPIRVBuilder::getTypePointerId(spirv::IdRef typeId, spv::StorageClass storageClass)
{
    std::pair<spirv::IdRef, spv::StorageClass> key{typeId, storageClass};

    auto iter = mTypePointerIdMap.find(key);
    if (iter == mTypePointerIdMap.end())
    {
        const spirv::IdRef typePointerId = getNewId();

        spirv::WriteTypePointer(&mSpirvTypePointerDecls, typePointerId, storageClass, typeId);

        iter = mTypePointerIdMap.insert({key, typePointerId}).first;
    }

    return iter->second;
}

spirv::IdRef SPIRVBuilder::getFunctionTypeId(spirv::IdRef returnTypeId,
                                             const spirv::IdRefList &paramTypeIds)
{
    std::pair<spirv::IdRef, spirv::IdRefList> key{returnTypeId, paramTypeIds};

    auto iter = mFunctionTypeIdMap.find(key);
    if (iter == mFunctionTypeIdMap.end())
    {
        const spirv::IdRef functionTypeId = getNewId();

        spirv::WriteTypeFunction(&mSpirvTypeAndConstantDecls, functionTypeId, returnTypeId,
                                 paramTypeIds);

        iter = mFunctionTypeIdMap.insert({key, functionTypeId}).first;
    }

    return iter->second;
}

SpirvTypeData SPIRVBuilder::declareType(const SpirvType &type, const char *blockName)
{
    // Recursively declare the type.  Type id is allocated afterwards purely for better id order in
    // output.
    spirv::IdRef typeId;

    if (!type.arraySizes.empty())
    {
        // Declaring an array.  First, declare the type without the outermost array size, then
        // declare a new array type based on that.

        SpirvType subType  = type;
        subType.arraySizes = type.arraySizes.first(type.arraySizes.size() - 1);
        if (subType.arraySizes.empty() && subType.block == nullptr)
        {
            subType.blockStorage = EbsUnspecified;
        }

        const spirv::IdRef subTypeId = getSpirvTypeData(subType, "").id;

        const unsigned int length   = type.arraySizes.back();
        const spirv::IdRef lengthId = getUintConstant(length);

        typeId = getNewId();
        spirv::WriteTypeArray(&mSpirvTypeAndConstantDecls, typeId, subTypeId, lengthId);
    }
    else if (type.block != nullptr)
    {
        // Declaring a block.  First, declare all the fields, then declare a struct based on the
        // list of field types.

        spirv::IdRefList fieldTypeIds;
        for (const TField *field : type.block->fields())
        {
            spirv::IdRef fieldTypeId = getTypeData(*field->type(), type.blockStorage).id;
            fieldTypeIds.push_back(fieldTypeId);
        }

        typeId = getNewId();
        spirv::WriteTypeStruct(&mSpirvTypeAndConstantDecls, typeId, fieldTypeIds);
    }
    else if (IsSampler(type.type) && !type.isSamplerBaseImage)
    {
        // Declaring a sampler.  First, declare the non-sampled image and then a combined
        // image-sampler.

        SpirvType imageType          = type;
        imageType.isSamplerBaseImage = true;
        imageType.blockStorage       = EbsUnspecified;

        const spirv::IdRef nonSampledId = getSpirvTypeData(imageType, "").id;

        typeId = getNewId();
        spirv::WriteTypeSampledImage(&mSpirvTypeAndConstantDecls, typeId, nonSampledId);
    }
    else if (IsImage(type.type) || type.isSamplerBaseImage)
    {
        // Declaring an image.

        spirv::IdRef sampledType;
        spv::Dim dim;
        spirv::LiteralInteger depth;
        spirv::LiteralInteger arrayed;
        spirv::LiteralInteger multisampled;
        spirv::LiteralInteger sampled;

        getImageTypeParameters(type.type, &sampledType, &dim, &depth, &arrayed, &multisampled,
                               &sampled);
        spv::ImageFormat imageFormat = getImageFormat(type.imageInternalFormat);

        typeId = getNewId();
        spirv::WriteTypeImage(&mSpirvTypeAndConstantDecls, typeId, sampledType, dim, depth, arrayed,
                              multisampled, sampled, imageFormat, nullptr);
    }
    else if (IsSubpassInputType(type.type))
    {
        // TODO: add support for framebuffer fetch. http://anglebug.com/4889
        UNIMPLEMENTED();
    }
    else if (type.secondarySize > 1)
    {
        // Declaring a matrix.  Declare the column type first, then create a matrix out of it.

        SpirvType columnType     = type;
        columnType.secondarySize = 1;
        columnType.blockStorage  = EbsUnspecified;

        const spirv::IdRef columnTypeId = getSpirvTypeData(columnType, "").id;

        typeId = getNewId();
        spirv::WriteTypeMatrix(&mSpirvTypeAndConstantDecls, typeId, columnTypeId,
                               spirv::LiteralInteger(type.secondarySize));
    }
    else if (type.primarySize > 1)
    {
        // Declaring a vector.  Declare the component type first, then create a vector out of it.

        SpirvType componentType    = type;
        componentType.primarySize  = 1;
        componentType.blockStorage = EbsUnspecified;

        const spirv::IdRef componentTypeId = getSpirvTypeData(componentType, "").id;

        typeId = getNewId();
        spirv::WriteTypeVector(&mSpirvTypeAndConstantDecls, typeId, componentTypeId,
                               spirv::LiteralInteger(type.primarySize));
    }
    else
    {
        typeId = getNewId();

        // Declaring a basic type.  There's a different instruction for each.
        switch (type.type)
        {
            case EbtVoid:
                spirv::WriteTypeVoid(&mSpirvTypeAndConstantDecls, typeId);
                break;
            case EbtFloat:
                spirv::WriteTypeFloat(&mSpirvTypeAndConstantDecls, typeId,
                                      spirv::LiteralInteger(32));
                break;
            case EbtDouble:
                // TODO: support desktop GLSL.  http://anglebug.com/4889
                UNIMPLEMENTED();
                break;
            case EbtInt:
                spirv::WriteTypeInt(&mSpirvTypeAndConstantDecls, typeId, spirv::LiteralInteger(32),
                                    spirv::LiteralInteger(1));
                break;
            case EbtUInt:
                spirv::WriteTypeInt(&mSpirvTypeAndConstantDecls, typeId, spirv::LiteralInteger(32),
                                    spirv::LiteralInteger(0));
                break;
            case EbtBool:
                spirv::WriteTypeBool(&mSpirvTypeAndConstantDecls, typeId);
                break;
            default:
                UNREACHABLE();
        }
    }

    // If this was a block declaration, add debug information for its type and field names.
    //
    // TODO: make this conditional to a compiler flag.  Instead of outputting the debug info
    // unconditionally and having the SPIR-V transformer remove them, it's better to avoid
    // generating them in the first place.  This both simplifies the transformer and reduces SPIR-V
    // binary size that gets written to disk cache.  http://anglebug.com/4889
    if (type.block != nullptr)
    {
        spirv::WriteName(&mSpirvDebug, typeId, blockName);

        uint32_t fieldIndex = 0;
        for (const TField *field : type.block->fields())
        {
            spirv::WriteMemberName(&mSpirvDebug, typeId, spirv::LiteralInteger(fieldIndex++),
                                   field->name().data());
        }
    }

    uint32_t baseAlignment      = 4;
    uint32_t sizeInStorageBlock = 0;

    // Calculate base alignment and sizes for types.  Size for blocks are not calculated, as they
    // are done later at the same time Offset decorations are written.
    const bool isOpaqueType = IsOpaqueType(type.type);
    if (!isOpaqueType)
    {
        baseAlignment = calculateBaseAlignmentAndSize(type, &sizeInStorageBlock);
    }

    // Write decorations for interface block fields.
    if (type.blockStorage != EbsUnspecified)
    {
        if (!isOpaqueType && !type.arraySizes.empty())
        {
            // Write the ArrayStride decoration for arrays inside interface blocks.
            spirv::WriteDecorate(&mSpirvDecorations, typeId, spv::DecorationArrayStride,
                                 {spirv::LiteralInteger(sizeInStorageBlock)});
        }
        else if (type.arraySizes.empty() && type.block != nullptr)
        {
            // Write the Offset decoration for interface blocks and structs in them.
            sizeInStorageBlock = calculateSizeAndWriteOffsetDecorations(type, typeId);
        }

        // TODO: write the MatrixStride decoration.  http://anglebug.com/4889.
    }

    // TODO: handle row-major matrixes.  http://anglebug.com/4889.
    // TODO: handle RelaxedPrecision types.  http://anglebug.com/4889.

    return {typeId, baseAlignment, sizeInStorageBlock};
}

void SPIRVBuilder::getImageTypeParameters(TBasicType type,
                                          spirv::IdRef *sampledTypeOut,
                                          spv::Dim *dimOut,
                                          spirv::LiteralInteger *depthOut,
                                          spirv::LiteralInteger *arrayedOut,
                                          spirv::LiteralInteger *multisampledOut,
                                          spirv::LiteralInteger *sampledOut)
{
    TBasicType sampledType = EbtFloat;
    *dimOut                = spv::Dim2D;
    bool isDepth           = false;
    bool isArrayed         = false;
    bool isMultisampled    = false;

    // Decompose the basic type into image properties
    switch (type)
    {
        // Float 2D Images
        case EbtSampler2D:
        case EbtImage2D:
        case EbtSamplerExternalOES:
        case EbtSamplerExternal2DY2YEXT:
        case EbtSamplerVideoWEBGL:
            break;
        case EbtSampler2DArray:
        case EbtImage2DArray:
            isArrayed = true;
            break;
        case EbtSampler2DMS:
        case EbtImage2DMS:
            isMultisampled = true;
            break;
        case EbtSampler2DMSArray:
        case EbtImage2DMSArray:
            isArrayed      = true;
            isMultisampled = true;
            break;
        case EbtSampler2DShadow:
            isDepth = true;
            break;
        case EbtSampler2DArrayShadow:
            isDepth   = true;
            isArrayed = true;
            break;

        // Integer 2D images
        case EbtISampler2D:
        case EbtIImage2D:
            sampledType = EbtInt;
            break;
        case EbtISampler2DArray:
        case EbtIImage2DArray:
            sampledType = EbtInt;
            isArrayed   = true;
            break;
        case EbtISampler2DMS:
        case EbtIImage2DMS:
            sampledType    = EbtInt;
            isMultisampled = true;
            break;
        case EbtISampler2DMSArray:
        case EbtIImage2DMSArray:
            sampledType    = EbtInt;
            isArrayed      = true;
            isMultisampled = true;
            break;

        // Unsinged integer 2D images
        case EbtUSampler2D:
        case EbtUImage2D:
            sampledType = EbtUInt;
            break;
        case EbtUSampler2DArray:
        case EbtUImage2DArray:
            sampledType = EbtUInt;
            isArrayed   = true;
            break;
        case EbtUSampler2DMS:
        case EbtUImage2DMS:
            sampledType    = EbtUInt;
            isMultisampled = true;
            break;
        case EbtUSampler2DMSArray:
        case EbtUImage2DMSArray:
            sampledType    = EbtUInt;
            isArrayed      = true;
            isMultisampled = true;
            break;

        // 3D images
        case EbtSampler3D:
        case EbtImage3D:
            *dimOut = spv::Dim3D;
            break;
        case EbtISampler3D:
        case EbtIImage3D:
            sampledType = EbtInt;
            *dimOut     = spv::Dim3D;
            break;
        case EbtUSampler3D:
        case EbtUImage3D:
            sampledType = EbtUInt;
            *dimOut     = spv::Dim3D;
            break;

        // Float cube images
        case EbtSamplerCube:
        case EbtImageCube:
            *dimOut = spv::DimCube;
            break;
        case EbtSamplerCubeArray:
        case EbtImageCubeArray:
            *dimOut   = spv::DimCube;
            isArrayed = true;
            break;
        case EbtSamplerCubeArrayShadow:
            *dimOut   = spv::DimCube;
            isDepth   = true;
            isArrayed = true;
            break;
        case EbtSamplerCubeShadow:
            *dimOut = spv::DimCube;
            isDepth = true;
            break;

        // Integer cube images
        case EbtISamplerCube:
        case EbtIImageCube:
            sampledType = EbtInt;
            *dimOut     = spv::DimCube;
            break;
        case EbtISamplerCubeArray:
        case EbtIImageCubeArray:
            sampledType = EbtInt;
            *dimOut     = spv::DimCube;
            isArrayed   = true;
            break;

        // Unsigned integer cube images
        case EbtUSamplerCube:
        case EbtUImageCube:
            sampledType = EbtUInt;
            *dimOut     = spv::DimCube;
            break;
        case EbtUSamplerCubeArray:
        case EbtUImageCubeArray:
            sampledType = EbtUInt;
            *dimOut     = spv::DimCube;
            isArrayed   = true;
            break;

        // Float 1D images
        case EbtSampler1D:
        case EbtImage1D:
            *dimOut = spv::Dim1D;
            break;
        case EbtSampler1DArray:
        case EbtImage1DArray:
            *dimOut   = spv::Dim1D;
            isArrayed = true;
            break;
        case EbtSampler1DShadow:
            *dimOut = spv::Dim1D;
            isDepth = true;
            break;
        case EbtSampler1DArrayShadow:
            *dimOut   = spv::Dim1D;
            isDepth   = true;
            isArrayed = true;
            break;

        // Integer 1D images
        case EbtISampler1D:
        case EbtIImage1D:
            sampledType = EbtInt;
            *dimOut     = spv::Dim1D;
            break;
        case EbtISampler1DArray:
        case EbtIImage1DArray:
            sampledType = EbtInt;
            *dimOut     = spv::Dim1D;
            isArrayed   = true;
            break;

        // Unsigned integer 1D images
        case EbtUSampler1D:
        case EbtUImage1D:
            sampledType = EbtUInt;
            *dimOut     = spv::Dim1D;
            break;
        case EbtUSampler1DArray:
        case EbtUImage1DArray:
            sampledType = EbtUInt;
            *dimOut     = spv::Dim1D;
            isArrayed   = true;
            break;

        // Rect images
        case EbtSampler2DRect:
        case EbtImageRect:
            *dimOut = spv::DimRect;
            break;
        case EbtSampler2DRectShadow:
            *dimOut = spv::DimRect;
            isDepth = true;
            break;
        case EbtISampler2DRect:
        case EbtIImageRect:
            sampledType = EbtInt;
            *dimOut     = spv::DimRect;
            break;
        case EbtUSampler2DRect:
        case EbtUImageRect:
            sampledType = EbtUInt;
            *dimOut     = spv::DimRect;
            break;

        // Image buffers
        case EbtSamplerBuffer:
        case EbtImageBuffer:
            *dimOut = spv::DimBuffer;
            break;
        case EbtISamplerBuffer:
        case EbtIImageBuffer:
            sampledType = EbtInt;
            *dimOut     = spv::DimBuffer;
            break;
        case EbtUSamplerBuffer:
        case EbtUImageBuffer:
            sampledType = EbtUInt;
            *dimOut     = spv::DimBuffer;
            break;
        default:
            // TODO: support framebuffer fetch.  http://anglebug.com/4889
            UNREACHABLE();
    }

    // Get id of the component type of the image
    SpirvType sampledSpirvType;
    sampledSpirvType.type = sampledType;

    *sampledTypeOut = getSpirvTypeData(sampledSpirvType, "").id;

    const bool isSampledImage = IsSampler(type);

    // Set flags based on SPIR-V required values.  See OpTypeImage:
    //
    // - For depth:        0 = non-depth,      1 = depth
    // - For arrayed:      0 = non-arrayed,    1 = arrayed
    // - For multisampled: 0 = single-sampled, 1 = multisampled
    // - For sampled:      1 = sampled,        2 = storage
    //
    *depthOut        = spirv::LiteralInteger(isDepth ? 1 : 0);
    *arrayedOut      = spirv::LiteralInteger(isArrayed ? 1 : 0);
    *multisampledOut = spirv::LiteralInteger(isMultisampled ? 1 : 0);
    *sampledOut      = spirv::LiteralInteger(isSampledImage ? 1 : 2);

    // Add the necessary capability based on parameters.  The SPIR-V spec section 3.8 Dim specfies
    // the required capabilities:
    //
    //     Dim          Sampled         Storage            Storage Array
    //     --------------------------------------------------------------
    //     1D           Sampled1D       Image1D
    //     2D           Shader                             ImageMSArray
    //     3D
    //     Cube         Shader                             ImageCubeArray
    //     Rect         SampledRect     ImageRect
    //     Buffer       SampledBuffer   ImageBuffer
    //
    // Note that the Shader capability is always unconditionally added.
    //
    switch (*dimOut)
    {
        case spv::Dim1D:
            addCapability(isSampledImage ? spv::CapabilitySampled1D : spv::CapabilityImage1D);
            break;
        case spv::Dim2D:
            if (!isSampledImage && isArrayed && isMultisampled)
            {
                addCapability(spv::CapabilityImageMSArray);
            }
            break;
        case spv::Dim3D:
            break;
        case spv::DimCube:
            if (!isSampledImage && isArrayed && isMultisampled)
            {
                addCapability(spv::CapabilityImageCubeArray);
            }
            break;
        case spv::DimRect:
            addCapability(isSampledImage ? spv::CapabilitySampledRect : spv::CapabilityImageRect);
            break;
        case spv::DimBuffer:
            addCapability(isSampledImage ? spv::CapabilitySampledBuffer
                                         : spv::CapabilityImageBuffer);
            break;
        default:
            // TODO: support framebuffer fetch.  http://anglebug.com/4889
            UNREACHABLE();
    }
}

spv::ImageFormat SPIRVBuilder::getImageFormat(TLayoutImageInternalFormat imageInternalFormat)
{
    switch (imageInternalFormat)
    {
        case EiifUnspecified:
            return spv::ImageFormatUnknown;
        case EiifRGBA32F:
            return spv::ImageFormatRgba32f;
        case EiifRGBA16F:
            return spv::ImageFormatRgba16f;
        case EiifR32F:
            return spv::ImageFormatR32f;
        case EiifRGBA32UI:
            return spv::ImageFormatRgba32ui;
        case EiifRGBA16UI:
            return spv::ImageFormatRgba16ui;
        case EiifRGBA8UI:
            return spv::ImageFormatRgba8ui;
        case EiifR32UI:
            return spv::ImageFormatR32ui;
        case EiifRGBA32I:
            return spv::ImageFormatRgba32i;
        case EiifRGBA16I:
            return spv::ImageFormatRgba16i;
        case EiifRGBA8I:
            return spv::ImageFormatRgba8i;
        case EiifR32I:
            return spv::ImageFormatR32i;
        case EiifRGBA8:
            return spv::ImageFormatRgba8;
        case EiifRGBA8_SNORM:
            return spv::ImageFormatRgba8Snorm;
        default:
            UNREACHABLE();
            return spv::ImageFormatUnknown;
    }
}

spirv::IdRef SPIRVBuilder::getBoolConstant(bool value)
{
    uint32_t asInt = static_cast<uint32_t>(value);

    spirv::IdRef constantId = mBoolConstants[asInt];

    if (!constantId.valid())
    {
        SpirvType boolType;
        boolType.type = EbtBool;

        const spirv::IdRef boolTypeId = getSpirvTypeData(boolType, "").id;

        mBoolConstants[asInt] = constantId = getNewId();
        if (value)
        {
            spirv::WriteConstantTrue(&mSpirvTypeAndConstantDecls, boolTypeId, constantId);
        }
        else
        {
            spirv::WriteConstantFalse(&mSpirvTypeAndConstantDecls, boolTypeId, constantId);
        }
    }

    return constantId;
}

spirv::IdRef SPIRVBuilder::getBasicConstantHelper(uint32_t value,
                                                  TBasicType type,
                                                  angle::HashMap<uint32_t, spirv::IdRef> *constants)
{
    auto iter = constants->find(value);
    if (iter == constants->end())
    {
        SpirvType spirvType;
        spirvType.type = type;

        const spirv::IdRef typeId     = getSpirvTypeData(spirvType, "").id;
        const spirv::IdRef constantId = getNewId();

        spirv::WriteConstant(&mSpirvTypeAndConstantDecls, typeId, constantId,
                             spirv::LiteralContextDependentNumber(value));

        iter = constants->insert({value, constantId}).first;
    }

    return iter->second;
}

spirv::IdRef SPIRVBuilder::getUintConstant(uint32_t value)
{
    return getBasicConstantHelper(value, EbtUInt, &mUintConstants);
}

spirv::IdRef SPIRVBuilder::getIntConstant(int32_t value)
{
    uint32_t asUint = static_cast<uint32_t>(value);
    return getBasicConstantHelper(asUint, EbtInt, &mIntConstants);
}

spirv::IdRef SPIRVBuilder::getFloatConstant(float value)
{
    union
    {
        float f;
        uint32_t u;
    } asUint;
    asUint.f = value;
    return getBasicConstantHelper(asUint.u, EbtFloat, &mFloatConstants);
}

spirv::IdRef SPIRVBuilder::getCompositeConstant(spirv::IdRef typeId, const spirv::IdRefList &values)
{
    std::pair<spirv::IdRef, spirv::IdRefList> key{typeId, values};

    auto iter = mCompositeConstants.find(key);
    if (iter == mCompositeConstants.end())
    {
        const spirv::IdRef constantId = getNewId();

        spirv::WriteConstantComposite(&mSpirvTypeAndConstantDecls, typeId, constantId, values);

        iter = mCompositeConstants.insert({key, constantId}).first;
    }

    return iter->second;
}

uint32_t SPIRVBuilder::nextUnusedBinding()
{
    return mNextUnusedBinding++;
}
uint32_t SPIRVBuilder::nextUnusedInputLocation(uint32_t consumedCount)
{
    uint32_t nextUnused = mNextUnusedInputLocation;
    mNextUnusedInputLocation += consumedCount;
    return nextUnused;
}
uint32_t SPIRVBuilder::nextUnusedOutputLocation(uint32_t consumedCount)
{
    uint32_t nextUnused = mNextUnusedOutputLocation;
    mNextUnusedOutputLocation += consumedCount;
    return nextUnused;
}

void SPIRVBuilder::addCapability(spv::Capability capability)
{
    mCapabilities.insert(capability);
}

void SPIRVBuilder::addExecutionMode(spv::ExecutionMode executionMode)
{
    mExecutionModes.insert(executionMode);
}

void SPIRVBuilder::setEntryPointId(spirv::IdRef id)
{
    ASSERT(!mEntryPointId.valid());
    mEntryPointId = id;
}

void SPIRVBuilder::addEntryPointInterfaceVariableId(spirv::IdRef id)
{
    mEntryPointInterfaceList.push_back(id);
}

void SPIRVBuilder::writeInterfaceVariableDecorations(const TType &type, spirv::IdRef variableId)
{
    const TLayoutQualifier &layoutQualifier = type.getLayoutQualifier();

    const bool needsSetBinding =
        IsSampler(type.getBasicType()) ||
        (type.isInterfaceBlock() &&
         (type.getQualifier() == EvqUniform || type.getQualifier() == EvqBuffer)) ||
        IsImage(type.getBasicType()) || IsSubpassInputType(type.getBasicType());
    const bool needsLocation =
        type.getQualifier() == EvqAttribute || type.getQualifier() == EvqVertexIn ||
        type.getQualifier() == EvqFragmentOut || IsVarying(type.getQualifier());
    const bool needsInputAttachmentIndex = IsSubpassInputType(type.getBasicType());
    const bool needsBlendIndex =
        type.getQualifier() == EvqFragmentOut && layoutQualifier.index >= 0;

    // TODO: handle row-major matrixes.  http://anglebug.com/4889.
    // TODO: handle invariant (spv::DecorationInvariant).

    // If the resource declaration requires set & binding, add the DescriptorSet and Binding
    // decorations.
    if (needsSetBinding)
    {
        spirv::WriteDecorate(&mSpirvDecorations, variableId, spv::DecorationDescriptorSet,
                             {spirv::LiteralInteger(0)});
        spirv::WriteDecorate(&mSpirvDecorations, variableId, spv::DecorationBinding,
                             {spirv::LiteralInteger(nextUnusedBinding())});
    }

    if (needsLocation)
    {
        const unsigned int locationCount =
            CalculateVaryingLocationCount(type, gl::ToGLenum(mShaderType));
        const uint32_t location = IsShaderIn(type.getQualifier())
                                      ? nextUnusedInputLocation(locationCount)
                                      : nextUnusedOutputLocation(locationCount);

        spirv::WriteDecorate(&mSpirvDecorations, variableId, spv::DecorationLocation,
                             {spirv::LiteralInteger(location)});
    }

    // If the resource declaration is an input attachment, add the InputAttachmentIndex decoration.
    if (needsInputAttachmentIndex)
    {
        spirv::WriteDecorate(&mSpirvDecorations, variableId, spv::DecorationInputAttachmentIndex,
                             {spirv::LiteralInteger(layoutQualifier.inputAttachmentIndex)});
    }

    if (needsBlendIndex)
    {
        spirv::WriteDecorate(&mSpirvDecorations, variableId, spv::DecorationIndex,
                             {spirv::LiteralInteger(layoutQualifier.index)});
    }
}

uint32_t SPIRVBuilder::calculateBaseAlignmentAndSize(const SpirvType &type,
                                                     uint32_t *sizeInStorageBlockOut)
{
    // Calculate the base alignment of a type according to the rules of std140 and std430 packing.
    //
    // See GLES3.2 Section 7.6.2.2 Standard Uniform Block Layout.

    if (!type.arraySizes.empty())
    {
        // > Rule 4. If the member is an array of scalars or vectors, the base alignment and array
        // > stride are set to match the base alignment of a single array element, according to
        // > rules (1), (2), and (3), ...
        //
        // > Rule 10. If the member is an array of S structures, the S elements of the array are
        // > laid out in order, according to rule (9).
        SpirvType baseType  = type;
        baseType.arraySizes = {};

        const SpirvTypeData &baseTypeData = getSpirvTypeData(baseType, "");
        uint32_t baseAlignment            = baseTypeData.baseAlignment;

        // For std140 only:
        // > Rule 4. ... and rounded up to the base alignment of a vec4.
        // > Rule 9. ... If none of the structure members are larger than a vec4, the base alignment
        // of the structure is vec4.
        if (type.blockStorage != EbsStd430)
        {
            baseAlignment = std::max(baseAlignment, 16u);
        }

        // Note that matrix arrays follow a similar rule (rules 6 and 8).  The matrix base alignment
        // is the same as its column or row base alignment, and arrays of that matrix don't change
        // the base alignment.

        // The size occupied by the array is simply the size of each element (which is already
        // aligned to baseAlignment) multiplied by the number of elements.
        uint32_t arraySizeProduct = 1;
        for (uint32_t arraySize : type.arraySizes)
        {
            arraySizeProduct *= arraySize;
        }
        *sizeInStorageBlockOut = baseTypeData.sizeInStorageBlock * arraySizeProduct;

        return baseAlignment;
    }

    if (type.block != nullptr)
    {
        // > Rule 9. If the member is a structure, the base alignment of the structure is N, where N
        // > is the largest base alignment value of any of its members, and rounded up to the base
        // > alignment of a vec4.

        uint32_t baseAlignment = 4;
        for (const TField *field : type.block->fields())
        {
            const SpirvTypeData &fieldTypeData = getTypeData(*field->type(), type.blockStorage);
            baseAlignment = std::max(baseAlignment, fieldTypeData.baseAlignment);
        }

        // For std140 only:
        // > If none of the structure members are larger than a vec4, the base alignment of the
        // structure is vec4.
        if (type.blockStorage != EbsStd430)
        {
            baseAlignment = std::max(baseAlignment, 16u);
        }

        // Note: sizeInStorageBlockOut is not calculated here, it's done in
        // calculateSizeAndWriteOffsetDecorations at the same time offsets are calculated.
        *sizeInStorageBlockOut = 0;

        return baseAlignment;
    }

    if (type.secondarySize > 1)
    {
        SpirvType vectorType = type;

        // > Rule 5. If the member is a column-major matrix with C columns and R rows, the matrix is
        // > stored identically to an array of C column vectors with R components each, according to
        // > rule (4).
        //
        // > Rule 7. If the member is a row-major matrix with C columns and R rows, the matrix is
        // > stored identically to an array of R row vectors with C components each, according to
        // > rule (4).
        //
        // For example, given a mat3x4 (3 columns, 4 rows), the base alignment is the same as the
        // base alignment of a vec4 (secondary size) if column-major, and a vec3 (primary size) if
        // row-major.
        //
        // TODO: verify that ANGLE's primary size is 3 in the example above.
        if (type.matrixPacking != EmpRowMajor)
        {
            vectorType.primarySize = vectorType.secondarySize;
        }
        vectorType.secondarySize = 1;

        const SpirvTypeData &vectorTypeData = getSpirvTypeData(vectorType, "");
        uint32_t baseAlignment              = vectorTypeData.baseAlignment;

        // For std140 only:
        // > Rule 4. ... and rounded up to the base alignment of a vec4.
        if (type.blockStorage != EbsStd430)
        {
            baseAlignment = std::max(baseAlignment, 16u);
        }

        // The size occupied by the matrix is the size of each vector multiplied by the number of
        // vectors.
        *sizeInStorageBlockOut = vectorTypeData.sizeInStorageBlock * type.primarySize *
                                 type.secondarySize / vectorType.primarySize;

        return baseAlignment;
    }

    if (type.primarySize > 1)
    {
        // > Rule 2. If the member is a two- or four-component vector with components consuming N
        // > basic machine units, the base alignment is 2N or 4N, respectively.
        //
        // > Rule 3. If the member is a three-component vector with components consuming N basic
        // > machine units, the base alignment is 4N.

        SpirvType baseType   = type;
        baseType.primarySize = 1;

        const SpirvTypeData &baseTypeData = getSpirvTypeData(baseType, "");
        uint32_t baseAlignment            = baseTypeData.baseAlignment;

        uint32_t multiplier = type.primarySize != 3 ? type.primarySize : 4;
        baseAlignment *= multiplier;

        // The size occupied by the vector is the same as its alignment.
        *sizeInStorageBlockOut = baseAlignment;

        return baseAlignment;
    }

    // TODO: support desktop GLSL.  http://anglebug.com/4889.  Except for double (desktop GLSL),
    // every other type occupies 4 bytes.
    constexpr uint32_t kBasicAlignment = 4;
    *sizeInStorageBlockOut             = kBasicAlignment;
    return kBasicAlignment;
}

uint32_t SPIRVBuilder::calculateSizeAndWriteOffsetDecorations(const SpirvType &type,
                                                              spirv::IdRef typeId)
{
    ASSERT(type.block != nullptr);

    uint32_t fieldIndex = 0;
    uint32_t nextOffset = 0;

    // Get the storage size for each field, align them based on block storage rules, and sum them
    // up.  In the process, write Offset decorations for the block.
    //
    // See GLES3.2 Section 7.6.2.2 Standard Uniform Block Layout.

    for (const TField *field : type.block->fields())
    {
        // Round the offset up to the field's alignment.  The spec says:
        //
        // > A structure and each structure member have a base offset and a base alignment, from
        // > which an aligned offset is computed by rounding the base offset up to a multiple of the
        // > base alignment.
        const SpirvTypeData &fieldTypeData = getTypeData(*field->type(), type.blockStorage);
        nextOffset                         = rx::roundUp(nextOffset, fieldTypeData.baseAlignment);

        // Write the Offset decoration.
        spirv::WriteMemberDecorate(&mSpirvDecorations, typeId, spirv::LiteralInteger(fieldIndex++),
                                   spv::DecorationOffset, {spirv::LiteralInteger(nextOffset)});

        // Calculate the next offset.  The next offset is the current offset plus the size of the
        // field, aligned to its base alignment.
        //
        // > Rule 4. ... the base offset of the member following the array is rounded up to the next
        // > multiple of the base alignment.
        //
        // > Rule 9. ... the base offset of the member following the sub-structure is rounded up to
        // > the next multiple of the base alignment of the structure.
        nextOffset = nextOffset + fieldTypeData.sizeInStorageBlock;
        nextOffset = rx::roundUp(nextOffset, fieldTypeData.baseAlignment);
    }

    return nextOffset;
}

ImmutableString SPIRVBuilder::hashName(const TSymbol *symbol)
{
    return HashName(symbol, mHashFunction, &mNameMap);
}

ImmutableString SPIRVBuilder::hashTypeName(const TType &type)
{
    return GetTypeName(type, mHashFunction, &mNameMap);
}

ImmutableString SPIRVBuilder::hashFieldName(const TField *field)
{
    ASSERT(field->symbolType() != SymbolType::Empty);
    if (field->symbolType() == SymbolType::UserDefined)
    {
        return HashName(field->name(), mHashFunction, &mNameMap);
    }

    return field->name();
}

ImmutableString SPIRVBuilder::hashFunctionNameIfNeeded(const TFunction *func)
{
    if (func->isMain())
    {
        return func->name();
    }
    else
    {
        return hashName(func);
    }
}

spirv::Blob SPIRVBuilder::getSpirv()
{
    spirv::Blob result;

    // Reserve a minimum amount of memory.
    //
    //   5 for header +
    //   a number of capabilities +
    //   a number of execution modes +
    //   size of already generated instructions.
    //
    // The actual size is larger due to other metadata instructions such as extensions,
    // OpExtInstImport, OpEntryPoint etc.
    result.reserve(5 + mCapabilities.size() * 2 + mExecutionModes.size() * 3 + mSpirvDebug.size() +
                   mSpirvDecorations.size() + mSpirvTypeAndConstantDecls.size() +
                   mSpirvTypePointerDecls.size() + mSpirvVariableDecls.size() +
                   mSpirvFunctions.size());

    // Generate any necessary id before writing the id bound in header.
    const spirv::IdRef extInstImportId = getNewId();

    // Generate the SPIR-V header.
    spirv::WriteSpirvHeader(&result, mNextAvailableId);

    // Generate metadata in the following order:
    //
    // - OpCapability instructions.  The Shader capability is always defined.
    spirv::WriteCapability(&result, spv::CapabilityShader);
    for (spv::Capability capability : mCapabilities)
    {
        spirv::WriteCapability(&result, capability);
    }

    // - OpExtension instructions (TODO: http://anglebug.com/4889)

    // - OpExtInstImport
    spirv::WriteExtInstImport(&result, extInstImportId, "GLSL.std.450");

    // - OpMemoryModel
    spirv::WriteMemoryModel(&result, spv::AddressingModelLogical, spv::MemoryModelGLSL450);

    // - OpEntryPoint
    constexpr gl::ShaderMap<spv::ExecutionModel> kExecutionModels = {
        {gl::ShaderType::Vertex, spv::ExecutionModelVertex},
        {gl::ShaderType::TessControl, spv::ExecutionModelTessellationControl},
        {gl::ShaderType::TessEvaluation, spv::ExecutionModelTessellationEvaluation},
        {gl::ShaderType::Geometry, spv::ExecutionModelGeometry},
        {gl::ShaderType::Fragment, spv::ExecutionModelFragment},
        {gl::ShaderType::Compute, spv::ExecutionModelGLCompute},
    };
    spirv::WriteEntryPoint(&result, kExecutionModels[mShaderType], mEntryPointId, "main",
                           mEntryPointInterfaceList);

    // - OpExecutionMode instructions
    for (spv::ExecutionMode executionMode : mExecutionModes)
    {
        spirv::WriteExecutionMode(&result, mEntryPointId, executionMode);
    }
    result.insert(result.end(), mSpirvExecutionModes.begin(), mSpirvExecutionModes.end());

    // Append the already generated sections in order
    result.insert(result.end(), mSpirvDebug.begin(), mSpirvDebug.end());
    result.insert(result.end(), mSpirvDecorations.begin(), mSpirvDecorations.end());
    result.insert(result.end(), mSpirvTypeAndConstantDecls.begin(),
                  mSpirvTypeAndConstantDecls.end());
    result.insert(result.end(), mSpirvTypePointerDecls.begin(), mSpirvTypePointerDecls.end());
    result.insert(result.end(), mSpirvVariableDecls.begin(), mSpirvVariableDecls.end());
    result.insert(result.end(), mSpirvFunctions.begin(), mSpirvFunctions.end());

    result.shrink_to_fit();
    return result;
}

class OutputSPIRVTraverser : public TIntermTraverser
{
  public:
    OutputSPIRVTraverser(TCompiler *compiler, ShCompileOptions compileOptions);

    spirv::Blob getSpirv();

  protected:
    void visitSymbol(TIntermSymbol *node) override;
    void visitConstantUnion(TIntermConstantUnion *node) override;
    bool visitSwizzle(Visit visit, TIntermSwizzle *node) override;
    bool visitBinary(Visit visit, TIntermBinary *node) override;
    bool visitUnary(Visit visit, TIntermUnary *node) override;
    bool visitTernary(Visit visit, TIntermTernary *node) override;
    bool visitIfElse(Visit visit, TIntermIfElse *node) override;
    bool visitSwitch(Visit visit, TIntermSwitch *node) override;
    bool visitCase(Visit visit, TIntermCase *node) override;
    void visitFunctionPrototype(TIntermFunctionPrototype *node) override;
    bool visitFunctionDefinition(Visit visit, TIntermFunctionDefinition *node) override;
    bool visitAggregate(Visit visit, TIntermAggregate *node) override;
    bool visitBlock(Visit visit, TIntermBlock *node) override;
    bool visitGlobalQualifierDeclaration(Visit visit,
                                         TIntermGlobalQualifierDeclaration *node) override;
    bool visitDeclaration(Visit visit, TIntermDeclaration *node) override;
    bool visitLoop(Visit visit, TIntermLoop *node) override;
    bool visitBranch(Visit visit, TIntermBranch *node) override;
    void visitPreprocessorDirective(TIntermPreprocessorDirective *node) override;

  private:
    ANGLE_MAYBE_UNUSED TCompiler *mCompiler;
    ANGLE_MAYBE_UNUSED ShCompileOptions mCompileOptions;

    SPIRVBuilder mBuilder;
};

spv::StorageClass GetStorageClass(const TType &type)
{
    // Opaque uniforms (samplers and images) have the UniformConstant storage class
    if (type.isSampler() || type.isImage())
    {
        return spv::StorageClassUniformConstant;
    }

    // Input varying and IO blocks have the Input storage class
    if (IsShaderIn(type.getQualifier()))
    {
        return spv::StorageClassInput;
    }

    // Output varying and IO blocks have the Input storage class
    if (IsShaderOut(type.getQualifier()))
    {
        return spv::StorageClassOutput;
    }

    // Uniform and storage buffers have the Uniform storage class
    if (type.isInterfaceBlock())
    {
        // I/O blocks must have already been classified as input or output above.
        ASSERT(!IsShaderIoBlock(type.getQualifier()));
        return spv::StorageClassUniform;
    }

    // Compute shader shared memory has the Workgroup storage class
    if (type.getQualifier() == EvqShared)
    {
        return spv::StorageClassWorkgroup;
    }

    // All other variables are either Private or Function, based on whether they are global or
    // function-local.
    if (type.getQualifier() == EvqGlobal)
    {
        return spv::StorageClassPrivate;
    }

    ASSERT(type.getQualifier() == EvqTemporary);
    return spv::StorageClassFunction;
}

OutputSPIRVTraverser::OutputSPIRVTraverser(TCompiler *compiler, ShCompileOptions compileOptions)
    : TIntermTraverser(true, true, true, &compiler->getSymbolTable()),
      mCompiler(compiler),
      mCompileOptions(compileOptions),
      mBuilder(gl::FromGLenum<gl::ShaderType>(compiler->getShaderType()),
               compiler->getHashFunction(),
               compiler->getNameMap())
{}

void OutputSPIRVTraverser::visitSymbol(TIntermSymbol *node)
{
    // TODO: http://anglebug.com/4889
    UNIMPLEMENTED();
}

void OutputSPIRVTraverser::visitConstantUnion(TIntermConstantUnion *node)
{
    // TODO: http://anglebug.com/4889
    UNIMPLEMENTED();
}

bool OutputSPIRVTraverser::visitSwizzle(Visit visit, TIntermSwizzle *node)
{
    // TODO: http://anglebug.com/4889
    UNIMPLEMENTED();

    return true;
}

bool OutputSPIRVTraverser::visitBinary(Visit visit, TIntermBinary *node)
{
    // TODO: http://anglebug.com/4889
    UNIMPLEMENTED();

    return true;
}

bool OutputSPIRVTraverser::visitUnary(Visit visit, TIntermUnary *node)
{
    // TODO: http://anglebug.com/4889
    UNIMPLEMENTED();

    return true;
}

bool OutputSPIRVTraverser::visitTernary(Visit visit, TIntermTernary *node)
{
    // TODO: http://anglebug.com/4889
    UNIMPLEMENTED();

    return true;
}

bool OutputSPIRVTraverser::visitIfElse(Visit visit, TIntermIfElse *node)
{
    // TODO: http://anglebug.com/4889
    UNIMPLEMENTED();

    return true;
}

bool OutputSPIRVTraverser::visitSwitch(Visit visit, TIntermSwitch *node)
{
    // TODO: http://anglebug.com/4889
    UNIMPLEMENTED();

    return true;
}

bool OutputSPIRVTraverser::visitCase(Visit visit, TIntermCase *node)
{
    // TODO: http://anglebug.com/4889
    UNIMPLEMENTED();

    return false;
}

bool OutputSPIRVTraverser::visitBlock(Visit visit, TIntermBlock *node)
{
    // If global block, nothing to generate.
    if (getCurrentTraversalDepth() == 0)
    {
        return true;
    }

    if (visit == PreVisit)
    {
        const spirv::IdRef blockLabelId = mBuilder.getNewId();
        spirv::WriteLabel(mBuilder.getSpirvFunctions(), blockLabelId);
    }

    // TODO: http://anglebug.com/4889
    UNIMPLEMENTED();

    return false;
}

bool OutputSPIRVTraverser::visitFunctionDefinition(Visit visit, TIntermFunctionDefinition *node)
{
    if (visit == PreVisit)
    {
        const TFunction *function = node->getFunction();

        // Declare the function type
        const spirv::IdRef returnTypeId =
            mBuilder.getTypeData(function->getReturnType(), EbsUnspecified).id;

        spirv::IdRefList paramTypeIds;
        for (size_t paramIndex = 0; paramIndex < function->getParamCount(); ++paramIndex)
        {
            paramTypeIds.push_back(
                mBuilder.getTypeData(function->getParam(paramIndex)->getType(), EbsUnspecified).id);
        }

        const spirv::IdRef functionTypeId = mBuilder.getFunctionTypeId(returnTypeId, paramTypeIds);

        // Declare the function itself
        const spirv::IdRef functionId = mBuilder.getNewId();
        spirv::WriteFunction(mBuilder.getSpirvFunctions(), returnTypeId, functionId,
                             spv::FunctionControlMaskNone, functionTypeId);

        for (size_t paramIndex = 0; paramIndex < function->getParamCount(); ++paramIndex)
        {
            const spirv::IdRef paramId = mBuilder.getNewId();
            spirv::WriteFunctionParameter(mBuilder.getSpirvFunctions(), paramTypeIds[paramIndex],
                                          paramId);

            // TODO: Add to TVariable to variableId map so references to this variable can discover
            // the ID.  http://anglebug.com/4889
        }

        // Remember the ID of main() for the sake of OpEntryPoint.
        if (function->isMain())
        {
            mBuilder.setEntryPointId(functionId);
        }

        return true;
    }

    if (visit == PostVisit)
    {
        // TODO: if the function returns void, the AST may not have an explicit OpReturn node, so
        // generate one at the end if not already.  For testing, unconditionally add it.
        // http://anglebug.com/4889
        if (node->getFunction()->getReturnType().getBasicType() == EbtVoid)
        {
            spirv::WriteReturn(mBuilder.getSpirvFunctions());
        }

        // End the function
        spirv::WriteFunctionEnd(mBuilder.getSpirvFunctions());
    }

    return true;
}

bool OutputSPIRVTraverser::visitGlobalQualifierDeclaration(Visit visit,
                                                           TIntermGlobalQualifierDeclaration *node)
{
    // TODO: http://anglebug.com/4889
    UNIMPLEMENTED();

    return true;
}

void OutputSPIRVTraverser::visitFunctionPrototype(TIntermFunctionPrototype *node)
{
    // Nothing to do.  The function type is declared together with its definition.
}

bool OutputSPIRVTraverser::visitAggregate(Visit visit, TIntermAggregate *node)
{
    // TODO: http://anglebug.com/4889
    UNIMPLEMENTED();

    return false;
}

bool OutputSPIRVTraverser::visitDeclaration(Visit visit, TIntermDeclaration *node)
{
    if (visit != PreVisit)
    {
        return true;
    }

    const TIntermSequence &sequence = *node->getSequence();

    // Enforced by ValidateASTOptions::validateMultiDeclarations.
    ASSERT(sequence.size() == 1);

    TIntermTyped *declVariable = sequence.front()->getAsTyped();
    const TType &type          = declVariable->getType();
    TIntermSymbol *symbol      = declVariable->getAsSymbolNode();
    ASSERT(symbol != nullptr);

    // If this is just a struct declaration (and not a variable declaration), don't declare the
    // struct up-front and let it be lazily defined.  If the struct is only used inside an interface
    // block for example, this avoids it being doubly defined (once with the unspecified block
    // storage and once with interface block's).
    if (type.isStructSpecifier() && symbol->variable().symbolType() == SymbolType::Empty)
    {
        return false;
    }

    const spirv::IdRef typeId = mBuilder.getTypeData(type, EbsUnspecified).id;

    // TODO: handle constant declarations.  http://anglebug.com/4889

    spv::StorageClass storageClass   = GetStorageClass(type);
    const spirv::IdRef typePointerId = mBuilder.getTypePointerId(typeId, storageClass);

    spirv::Blob *spirvSection = storageClass == spv::StorageClassFunction
                                    ? mBuilder.getSpirvFunctions()
                                    : mBuilder.getSpirvVariableDecls();

    const spirv::IdRef variableId = mBuilder.getNewId();
    // TODO: handle initializers.  http://anglebug.com/4889
    spirv::WriteVariable(spirvSection, typePointerId, variableId, storageClass, nullptr);

    // TODO: create a TVariable to variableId map so references to this variable can discover the
    // ID.  http://anglebug.com/4889

    if (IsShaderIn(type.getQualifier()) || IsShaderOut(type.getQualifier()))
    {
        // Add in and out variables to the list of interface variables.
        mBuilder.addEntryPointInterfaceVariableId(variableId);
    }
    else if (type.getBasicType() == EbtInterfaceBlock)
    {
        // For uniform and buffer variables, add Block and BufferBlock decorations respectively.
        const spv::Decoration decoration =
            type.getQualifier() == EvqUniform ? spv::DecorationBlock : spv::DecorationBufferBlock;
        spirv::WriteDecorate(mBuilder.getSpirvDecorations(), typeId, decoration, {});
    }

    // Write DescriptorSet, Binding, Location etc decorations if necessary.
    mBuilder.writeInterfaceVariableDecorations(type, variableId);

    // Output debug information.
    spirv::WriteName(mBuilder.getSpirvDebug(), variableId,
                     mBuilder.hashName(&symbol->variable()).data());

    return false;
}

bool OutputSPIRVTraverser::visitLoop(Visit visit, TIntermLoop *node)
{
    // TODO: http://anglebug.com/4889
    UNIMPLEMENTED();

    return true;
}

bool OutputSPIRVTraverser::visitBranch(Visit visit, TIntermBranch *node)
{
    // TODO: http://anglebug.com/4889
    UNIMPLEMENTED();

    return true;
}

void OutputSPIRVTraverser::visitPreprocessorDirective(TIntermPreprocessorDirective *node)
{
    // No preprocessor directives expected at this point.
    UNREACHABLE();
}

spirv::Blob OutputSPIRVTraverser::getSpirv()
{
    spirv::Blob result = mBuilder.getSpirv();

    // Validate that correct SPIR-V was generated
    ASSERT(spirv::Validate(result));

#if ANGLE_DEBUG_SPIRV_TRANSFORMER
    // Disassemble and log the generated SPIR-V for debugging.
    spvtools::SpirvTools spirvTools(SPV_ENV_VULKAN_1_1);
    std::string readableSpirv;
    spirvTools.Disassemble(result, &readableSpirv, 0);
    fprintf(stderr, "%s\n", readableSpirv.c_str());
#endif  // ANGLE_DEBUG_SPIRV_TRANSFORMER

    return result;
}
}  // anonymous namespace

bool OutputSPIRV(TCompiler *compiler, TIntermBlock *root, ShCompileOptions compileOptions)
{
    // Traverse the tree and generate SPIR-V instructions
    OutputSPIRVTraverser traverser(compiler, compileOptions);
    root->traverse(&traverser);

    // Generate the final SPIR-V and store in the sink
    spirv::Blob spirvBlob = traverser.getSpirv();
    compiler->getInfoSink().obj.setBinary(std::move(spirvBlob));

    return true;
}
}  // namespace sh
