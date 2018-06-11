#pragma once

#if defined(_WIN32)
#        define ANGLE_EXPORT __declspec(dllexport)
#elif defined(__GNUC__)
#        define ANGLE_EXPORT __attribute__((visibility("default")))
#else
#    define ANGLE_EXPORT
#endif

extern "C" {

// GLenum alias
typedef unsigned int GLenum;

// Varying interpolation qualifier, see section 4.3.9 of the ESSL 3.00.4 spec
enum ST_InterpolationType {
    INTERPOLATION_SMOOTH,
    INTERPOLATION_CENTROID,
    INTERPOLATION_FLAT,
    INTERPOLATION_NOPERSPECTIVE
};

// Uniform block layout qualifier, see section 4.3.8.3 of the ESSL 3.00.4 spec
enum ST_BlockLayoutType
{
    BLOCKLAYOUT_STANDARD,
    BLOCKLAYOUT_STD140 = BLOCKLAYOUT_STANDARD,
    BLOCKLAYOUT_STD430,  // Shader storage block layout qualifier
    BLOCKLAYOUT_PACKED,
    BLOCKLAYOUT_SHARED
};

// Interface Blocks, see section 4.3.9 of the ESSL 3.10 spec
enum class ST_BlockType
{
    BLOCK_UNIFORM,
    BLOCK_BUFFER,

    // Required in OpenGL ES 3.1 extension GL_OES_shader_io_blocks.
    // TODO(jiawei.shao@intel.com): add BLOCK_OUT.
    // Also used in GLSL
    BLOCK_IN
};

// Base class for all variables defined in shaders, including Varyings, Uniforms, etc
// Note: we must override the copy constructor and assignment operator so we can
// work around excessive GCC binary bloating:
// See https://code.google.com/p/angleproject/issues/detail?id=697
struct ST_ShaderVariable {
    GLenum type;
    GLenum precision;
    const char* name;
    const char* mappedName;

    unsigned int arraySizeCount;
    unsigned int* pArraySizes;

    unsigned int staticUse;
    unsigned int active;

    unsigned int fieldsCount;
    ST_ShaderVariable* pFields;

    const char* structName;

    // Only applies to interface block fields. Kept here for simplicity.
    unsigned int isRowMajorLayout;

    // VariableWithLocation
    int location;

    // Uniform
    int binding;

    GLenum imageUnitFormat;
    int offset;
    unsigned int readonly;
    unsigned int writeonly;

    // OutputVariable
    // From EXT_blend_func_extended.
    int index;

    // Varying
    ST_InterpolationType interpolation;
    unsigned int isInvariant;

    int flattenedOffsetInParentArrays;
};

struct ST_InterfaceBlock {
    const char* name;
    const char* mappedName;
    const char* instanceName;
    unsigned int arraySize;
    ST_BlockLayoutType layout;

    // Deprecated. Matrix packing should only be queried from individual fields of the block.
    // TODO(oetuaho): Remove this once it is no longer used in Chromium.
    unsigned int isRowMajorLayout;

    int binding;
    unsigned int staticUse;
    unsigned int active;
    ST_BlockType blockType;

    unsigned int fieldsCount;
    ST_ShaderVariable* pFields;
};

struct ST_WorkGroupSize {
    int localSizeQualifiers[3];
};

struct ST_NameHashingMap {
    unsigned int entryCount;
    const char* const* ppUserNames;
    const char* const* ppCompiledNames;
};

ANGLE_EXPORT void STInitialize(void);
ANGLE_EXPORT void STFinalize(void);

ANGLE_EXPORT ShBuiltInResources *STGenerateResources(
    ShBuiltInResources *resources);
ANGLE_EXPORT ShHandle STConstructCompiler(
    unsigned int type,
    ShShaderSpec spec,
    ShShaderOutput output,
    ShBuiltInResources *resources);

// A one-stop shop for getting all shader info in one call
// Everything is written and allocated by the library, transferring ownership
// to the consumer. The consumer will need to call STFreeShaderResolveState()
// to free the memory.
struct ST_ShaderResolveState {
    GLenum type;
    int version;

    const char* originalSource;
    const char* translatedSource;
    const char* infoLog;

    ST_NameHashingMap nameHashingMap;

    ST_WorkGroupSize workGroupSize;

    unsigned int inputVaryingsCount;
    const ST_ShaderVariable* pInputVaryings;

    unsigned int outputVaryingsCount;
    const ST_ShaderVariable* pOutputVaryings;

    unsigned int uniformsCount;
    const ST_ShaderVariable* pUniforms;

    unsigned int uniformBlocksCount;
    const ST_InterfaceBlock* pUniformBlocks;

    unsigned int shaderStorageBlocksCount;
    const ST_InterfaceBlock* pShaderStorageBlocks;

    unsigned int allAttributesCount;
    const ST_ShaderVariable* pActiveAttributes;

    unsigned int activeOutputVariablesCount;
    const ST_ShaderVariable* pActiveOutputVariables;

    bool earlyFragmentTestsOptimization;
    int numViews;

    // Geometry shader
    unsigned int geometryShaderInputPrimitiveType;
    unsigned int geometryShaderOutputPrimitiveType;
    int geometryShaderMaxInvocations;
    int geometryShaderInvocations;

    bool compileStatus;
};

ANGLE_EXPORT void STCompileAndResolve(
    const ShHandle handle, 
    const char *const *shaderStrings,
    size_t numStrings,
    ST_ShaderResolveState** state_out);

void STFreeShaderResolveState(
    ST_ShaderResolveState* state);


} // extern "C"
