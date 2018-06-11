#include "GLSLANG/ShaderLang.h"

#include "angle_gl.h"
#include "ShaderTranslator.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sstream>
#include <vector>
#include <map>
#include <string>

namespace {

unsigned int boolToUint(bool b) {
    return b ? 1 : 0;
}

template<class S, class T>
static T* toAllocedCArray(const S& xs) {
    size_t bytes = sizeof(T) * xs.size();
    T* res = (T *)malloc(bytes);
    memset(res, 0, bytes);
    memcpy(res, xs.data(), bytes);
    return res;
}

static const char* toAllocedCString(const std::string& str) {
    return toAllocedCArray<std::string, char>(str);
}

static ST_NameHashingMap* createNameHashingMap(const std::map<std::string, std::string>& inputMap);
static ST_ShaderVariable* createShaderVariableArray(const std::vector<sh::ShaderVariable>& inputVars);
static ST_InterfaceBlock* createInterfaceBlockArray(const std::vector<sh::InterfaceBlock>& inputBlocks);

static void freeNameHashingMap(ST_NameHashingMap* nameHashingMap);
static void destroyShaderVariable(ST_ShaderVariable* var);
static void freeShaderVariableArray(uint32_t count, ST_ShaderVariable* vars);
static void destroyInterfaceBlock(ST_InterfaceBlock* ib);
static void freeInterfaceBlockArray(uint32_t count, ST_InterfaceBlock* ibs);

static ST_NameHashingMap* createNameHashingMap(const std::map<std::string, std::string>& inputMap) {
(void)createNameHashingMap;
(void)createShaderVariableArray;
(void)createInterfaceBlockArray;

    size_t elements = inputMap.size();

    const char** ppUserNames = (const char**)malloc(elements * sizeof(const char*));
    const char** ppCompiledNames = (const char**)malloc(elements * sizeof(const char* const*));
    size_t i = 0;

    for (auto it : inputMap) {
        ppUserNames[i] = toAllocedCString(it.first);
        ppCompiledNames[i] = toAllocedCString(it.second);
        ++i;
    }

    ST_NameHashingMap* res = (ST_NameHashingMap*)(malloc(sizeof(ST_NameHashingMap)));

    res->entryCount = (uint32_t)elements;
    res->ppUserNames = ppUserNames;
    res->ppCompiledNames = ppCompiledNames;

    return res;
}

static void fillShaderVariable(const sh::ShaderVariable& var, ST_ShaderVariable* out) {
    out->type = var.type;
    out->precision = var.precision;

    out->name = toAllocedCString(var.name);
    out->mappedName = toAllocedCString(var.mappedName);

    out->arraySizeCount = (uint32_t)var.arraySizes.size();
    out->pArraySizes = toAllocedCArray<std::vector<unsigned int>, unsigned int>(var.arraySizes);

    out->staticUse = boolToUint(var.staticUse);
    out->active = boolToUint(var.active);

    out->fieldsCount = (uint32_t)var.fields.size();
    out->pFields = (ST_ShaderVariable*)(malloc(out->fieldsCount * sizeof(ST_ShaderVariable)));
    for (uint32_t i = 0; i < out->fieldsCount; ++i) {
        fillShaderVariable(var.fields[i], out->pFields + i);
    }

    out->structName = toAllocedCString(var.structName);

    out->isRowMajorLayout = boolToUint(var.isRowMajorLayout);

    out->location = var.location;

    out->binding = var.binding;

    out->imageUnitFormat = var.imageUnitFormat;
    out->offset = var.offset;
    out->readonly = boolToUint(var.readonly);
    out->writeonly = boolToUint(var.writeonly);

    out->index = var.index;

    out->interpolation = (ST_InterpolationType)(var.interpolation);
    out->isInvariant = boolToUint(var.isInvariant);

    out->flattenedOffsetInParentArrays = var.parentArrayIndex();
}

static ST_ShaderVariable* createShaderVariableArray(const std::vector<sh::ShaderVariable>& inputVars) {

    return 0;
}

static ST_InterfaceBlock* createInterfaceBlockArray(const std::vector<sh::InterfaceBlock>& inputBlocks) {
    return 0;
}

static void freeShaderVariableArray(uint32_t count, ST_ShaderVariable* vars) {
    for (uint32_t i = 0; i < count; ++i) {
        destroyShaderVariable(vars + i);
    }
    free(vars);
}

static void freeInterfaceBlockArray(uint32_t count, ST_InterfaceBlock* ibs) {
    for (uint32_t i = 0; i < count; ++i) {
        destroyInterfaceBlock(ibs + i);
    }
    free(ibs);
}

static void freeNameHashingMap(ST_NameHashingMap* nameHashingMap) {
    for (uint32_t i = 0; i < nameHashingMap->entryCount; ++i) {
        free((void*)nameHashingMap->ppUserNames[i]);
        free((void*)nameHashingMap->ppCompiledNames[i]);
    }

    free((void*)nameHashingMap->ppUserNames);
    free((void*)nameHashingMap->ppCompiledNames);
}

static void destroyShaderVariable(ST_ShaderVariable* var) {
    free((void*)var->name);
    free((void*)var->mappedName);

    free((void*)var->pArraySizes);
    freeShaderVariableArray(var->fieldsCount, var->pFields);

    free((void*)var->structName);
}

static void destroyInterfaceBlock(ST_InterfaceBlock* ib) {
    free((void*)ib->name);
    free((void*)ib->mappedName);
    free((void*)ib->instanceName);

    freeShaderVariableArray(ib->fieldsCount, ib->pFields);
}

} // namespace

extern "C" {

ANGLE_EXPORT void STInitialize() {
    sh::Initialize();
}

ANGLE_EXPORT void STFinalize() {
    sh::Finalize();
}

ANGLE_EXPORT ST_BuiltInResources*
STGenerateResources(ST_BuiltInResources *pResources) {
    static_assert(sizeof(ST_BuiltInResources) == sizeof(ShBuiltInResources),
        "mismatch in ST_BuiltInResources versus ShBuiltInResources");
    sh::InitBuiltInResources(reinterpret_cast<ShBuiltInResources*>(pResources));
    return pResources;
}

ANGLE_EXPORT ST_Handle STConstructCompiler(
    GLenum type,
    ST_ShaderSpec spec,
    ST_ShaderOutput output,
    ST_BuiltInResources *resources) {
    return static_cast<ST_Handle>(
        sh::ConstructCompiler(
            type,
            static_cast<ShShaderSpec>(spec),
            static_cast<ShShaderOutput>(output),
            reinterpret_cast<ShBuiltInResources*>(resources)));
}

ANGLE_EXPORT void STCompileAndResolve(
    const ST_Handle handle, 
    const char *const *shaderStrings,
    size_t numStrings,
    ST_CompileOptions compileOptions,
    ST_ShaderResolveState** state_out) {

    bool compileResult =
        sh::Compile(
            static_cast<ShHandle>(handle),
            shaderStrings, numStrings,
            static_cast<ST_CompileOptions>(compileOptions));

    ST_ShaderResolveState* res =
        (ST_ShaderResolveState*)(malloc(sizeof(ST_ShaderResolveState)));

    res->compileStatus = compileResult ? 1 : 0;


}

ANGLE_EXPORT void STFreeShaderResolveState(
    ST_ShaderResolveState* state) {

    free((void*)state->originalSource);
    free((void*)state->translatedSource);
    free((void*)state->infoLog);

    freeNameHashingMap(state->nameHashingMap);

    freeShaderVariableArray(state->inputVaryingsCount, (ST_ShaderVariable*)state->pInputVaryings);
    freeShaderVariableArray(state->outputVaryingsCount, (ST_ShaderVariable*)state->pOutputVaryings);
    freeShaderVariableArray(state->uniformsCount, (ST_ShaderVariable*)state->pUniforms);

    freeInterfaceBlockArray(state->uniformBlocksCount, (ST_InterfaceBlock*)state->pUniformBlocks);
    freeInterfaceBlockArray(state->shaderStorageBlocksCount, (ST_InterfaceBlock*)state->pShaderStorageBlocks);

    freeShaderVariableArray(state->allAttributesCount, (ST_ShaderVariable*)state->pActiveAttributes);
    freeShaderVariableArray(state->activeOutputVariablesCount, (ST_ShaderVariable*)state->pActiveOutputVariables);
}


// static bool sInitialized = false;
// 
// ANGLE_EXPORT void STInitialize(void)
// {
//     if (!sInitialized) {
//         sh::Initialize();
//     }
// }
// 
// ANGLE_EXPORT void STFinalize(void)
// {
//     if (sInitialized) {
//         sh::Finalize();
//         sInitialized = false;
//     }
// }
// 
// ANGLE_EXPORT ShBuiltInResources *STGenerateResources(ShBuiltInResources *resources)
// {
//     sh::InitBuiltInResources(resources);
//     return resources;
// }
// 
// ANGLE_EXPORT ShHandle STConstructCompiler(unsigned int type,
//                                           ShShaderSpec spec,
//                                           ShShaderOutput output,
//                                           ShBuiltInResources *resources)
// {
//     return sh::ConstructCompiler(type, spec, output, resources);
// }
// 
// ANGLE_EXPORT bool STCompile(ShHandle compiler,
//                             const char *const *shaderStrings,
//                             size_t numStrings,
//                             ShCompileOptions compileOptions)
// {
//     return sh::Compile(compiler, shaderStrings, numStrings, compileOptions);
// }
// 
// ANGLE_EXPORT void STClearResults(const ShHandle handle)
// {
//     sh::ClearResults(handle);
// }
// 
// ANGLE_EXPORT char *STGetInfoLog(const ShHandle handle)
// {
//     std::string res = sh::GetInfoLog(handle);
//     char *result    = (char *)malloc(res.size() + 1);
//     memset(result, 0, res.size() + 1);
//     memcpy(result, res.data(), res.size());
//     return result;
// }
// 
// ANGLE_EXPORT char *STGetObjectCode(const ShHandle handle)
// {
//     std::string res = sh::GetObjectCode(handle);
//     char *result    = (char *)malloc(res.size() + 1);
//     memset(result, 0, res.size() + 1);
//     memcpy(result, res.data(), res.size());
//     return result;
// }
// 
// ANGLE_EXPORT st_name_hashing_map STGetNameHashingMap(const ShHandle handle)
// {
// 
//     st_name_hashing_map res;
// 
//     const std::map<std::string, std::string> *nameMap = sh::GetNameHashingMap(handle);
// 
//     res.count          = nameMap->size();
//     res.user_names     = new char *[res.count];
//     res.compiled_names = new char *[res.count];
// 
//     int i = 0;
//     for (auto elt : *nameMap)
//     {
//         res.user_names[i] = new char[elt.first.size() + 1];
//         memset(res.user_names[i], 0, elt.first.size() + 1);
//         memcpy(res.user_names[i], elt.first.data(), elt.first.size());
// 
//         res.compiled_names[i] = new char[elt.second.size() + 1];
//         memset(res.compiled_names[i], 0, elt.second.size() + 1);
//         memcpy(res.compiled_names[i], elt.second.data(), elt.second.size());
// 
//         i++;
//     }
// 
//     return res;
// }
// 
// static char *to_alloced_buffer(const std::string &str)
// {
//     char *res = (char *)malloc(str.size() + 1);
//     memset(res, 0, str.size() + 1);
//     memcpy(res, str.data(), str.size());
//     return res;
// }
// 
// static void copy_var(const sh::ShaderVariable &var, struct st_shader_variable *out_var)
// {
//     out_var->type      = var.type;
//     out_var->precision = var.precision;
// 
//     out_var->name        = to_alloced_buffer(var.name);
//     out_var->mapped_name = to_alloced_buffer(var.mappedName);
// 
//     out_var->array_sizes_count = var.arraySizes.size();
//     out_var->array_sizes       = 0;
//     if (out_var->array_sizes_count)
//     {
//         out_var->array_sizes =
//             (unsigned int *)malloc(out_var->array_sizes_count * sizeof(unsigned int));
//         memcpy(out_var->array_sizes, var.arraySizes.data(),
//                (out_var->array_sizes_count * sizeof(unsigned int)));
//     }
// 
//     out_var->flattened_offset_in_parent_arrays = var.parentArrayIndex();
//     out_var->static_use                        = var.staticUse;
//     out_var->active                            = var.active;
// 
//     out_var->fields_count = var.fields.size();
//     out_var->fields       = 0;
//     if (out_var->fields_count)
//     {
//         out_var->fields = (struct st_shader_variable *)malloc(out_var->fields_count *
//                                                               sizeof(struct st_shader_variable));
//         for (int i = 0; i < out_var->fields_count; i++)
//         {
//             copy_var(var.fields[i], out_var->fields + i);
//         }
//     }
// 
//     out_var->struct_name = to_alloced_buffer(var.structName);
// }
// 
// static void copy_uniform(const sh::Uniform &uniform, struct st_shader_variable *out_var)
// {
//     out_var->location = uniform.location;
// 
//     out_var->binding   = uniform.binding;
//     out_var->offset    = uniform.offset;
//     out_var->readonly  = uniform.readonly;
//     out_var->writeonly = uniform.writeonly;
// 
//     out_var->st_var_type = ST_VARIABLE_UNIFORM;
// }
// 
// static void copy_attribute(const sh::Attribute &var, struct st_shader_variable *out_var)
// {
//     out_var->location = var.location;
// 
//     out_var->st_var_type = ST_VARIABLE_ATTRIBUTE;
// }
// 
// static void copy_output_variable(const sh::OutputVariable &var, struct st_shader_variable *out_var)
// {
//     out_var->location = var.location;
//     out_var->st_var_type = ST_VARIABLE_OUTPUT;
// }
// 
// static void copy_interface_block_field(const sh::InterfaceBlockField &var,
//                                        struct st_shader_variable *out_var)
// {
//     copy_var(var, out_var);
//     out_var->is_row_major_layout = var.isRowMajorLayout;
//     out_var->st_var_type         = ST_VARIABLE_INTERFACE_BLOCK_FIELD;
// }
// 
// static void copy_varying(const sh::Varying &var, struct st_shader_variable *out_var)
// {
//     out_var->location = var.location;
//     out_var->interpolation = (st_interpolation_qualifier)var.interpolation;
//     out_var->is_invariant  = var.isInvariant;
//     out_var->is_built_in = var.isBuiltIn();
//     out_var->st_var_type   = ST_VARIABLE_VARYING;
// }
// 
// static void copy_interface_block(const sh::InterfaceBlock &ib, struct st_interface_block *out_ib)
// {
//     out_ib->name          = to_alloced_buffer(ib.name);
//     out_ib->mapped_name   = to_alloced_buffer(ib.mappedName);
//     out_ib->instance_name = to_alloced_buffer(ib.instanceName);
//     out_ib->array_size    = ib.arraySize;
// 
//     out_ib->layout              = (st_block_layout_type)ib.layout;
//     out_ib->is_row_major_layout = ib.isRowMajorLayout;
// 
//     out_ib->binding    = ib.binding;
//     out_ib->static_use = ib.staticUse;
//     out_ib->active     = ib.active;
// 
//     out_ib->block_type = (st_block_type)ib.blockType;
// 
//     out_ib->fields_count = ib.fields.size();
//     out_ib->fields       = 0;
//     if (out_ib->fields_count)
//     {
//         out_ib->fields = (struct st_shader_variable *)malloc(out_ib->fields_count *
//                                                              sizeof(struct st_shader_variable));
//         for (int i = 0; i < out_ib->fields_count; i++)
//         {
//             copy_interface_block_field(ib.fields[i], out_ib->fields + i);
//         }
//     }
// }
// 
// static void copy_work_group_size(const sh::WorkGroupSize &wg, struct st_work_group_size *out_wg)
// {
//     for (int i = 0; i < 3; ++i) {
//         out_wg->local_size_qualifiers[i] = wg.localSizeQualifiers[i];
//     }
// }
// 
// ANGLE_EXPORT void STGetUniforms(const ShHandle handle,
//                                 int *out_count,
//                                 struct st_shader_variable *out_vars)
// {
//     const std::vector<sh::Uniform> *vars = sh::GetUniforms(handle);
//     *out_count                           = vars->size();
// 
//     if (!out_vars)
//         return;
// 
//     for (int i = 0; i < *out_count; i++)
//     {
//         copy_uniform((*vars)[i], out_vars + i);
//     }
// }
// 
// ANGLE_EXPORT void STGetVaryings(const ShHandle handle,
//                                 int *out_count,
//                                 struct st_shader_variable *out_vars)
// {
//     const std::vector<sh::Varying> *vars = sh::GetVaryings(handle);
//     *out_count                           = vars->size();
// 
//     if (!out_vars)
//         return;
// 
//     for (int i = 0; i < *out_count; i++)
//     {
//         copy_varying((*vars)[i], out_vars + i);
//     }
// }
// 
// ANGLE_EXPORT void STGetInputVaryings(const ShHandle handle,
//                                      int *out_count,
//                                      struct st_shader_variable *out_vars)
// {
//     const std::vector<sh::Varying> *vars = sh::GetInputVaryings(handle);
//     *out_count                           = vars->size();
// 
//     if (!out_vars)
//         return;
// 
//     for (int i = 0; i < *out_count; i++)
//     {
//         copy_varying((*vars)[i], out_vars + i);
//     }
// }
// 
// ANGLE_EXPORT void STGetOutputVaryings(const ShHandle handle,
//                                       int *out_count,
//                                       struct st_shader_variable *out_vars)
// {
//     const std::vector<sh::Varying> *vars = sh::GetOutputVaryings(handle);
//     *out_count                           = vars->size();
// 
//     if (!out_vars)
//         return;
// 
//     for (int i = 0; i < *out_count; i++)
//     {
//         copy_varying((*vars)[i], out_vars + i);
//     }
// }
// 
// ANGLE_EXPORT void STGetAttributes(const ShHandle handle,
//                                   int *out_count,
//                                   struct st_shader_variable *out_vars)
// {
//     const std::vector<sh::Attribute> *vars = sh::GetAttributes(handle);
//     *out_count                             = vars->size();
// 
//     if (!out_vars)
//         return;
// 
//     for (int i = 0; i < *out_count; i++)
//     {
//         copy_attribute((*vars)[i], out_vars + i);
//     }
// }
// 
// ANGLE_EXPORT void STGetOutputVariables(const ShHandle handle,
//                                        int *out_count,
//                                        struct st_shader_variable *out_vars)
// {
//     const std::vector<sh::OutputVariable> *vars = sh::GetOutputVariables(handle);
//     *out_count                                  = vars->size();
// 
//     if (!out_vars)
//         return;
// 
//     for (int i = 0; i < *out_count; i++)
//     {
//         copy_output_variable((*vars)[i], out_vars + i);
//     }
// }
// 
// ANGLE_EXPORT void STGetInterfaceBlocks(const ShHandle handle,
//                                        int *out_count,
//                                        struct st_interface_block *out_ibs)
// {
//     const std::vector<sh::InterfaceBlock> *ibs = sh::GetInterfaceBlocks(handle);
//     *out_count                                 = ibs->size();
// 
//     if (!out_ibs)
//         return;
// 
//     for (int i = 0; i < *out_count; i++)
//     {
//         copy_interface_block((*ibs)[i], out_ibs + i);
//     }
// }
// 
// ANGLE_EXPORT void STGetUniformBlocks(const ShHandle handle,
//                                      int *out_count,
//                                      struct st_interface_block *out_ibs)
// {
//     const std::vector<sh::InterfaceBlock> *ibs = sh::GetUniformBlocks(handle);
//     *out_count                                 = ibs->size();
// 
//     if (!out_ibs)
//         return;
// 
//     for (int i = 0; i < *out_count; i++)
//     {
//         copy_interface_block((*ibs)[i], out_ibs + i);
//     }
// }
// 
// ANGLE_EXPORT void STGetShaderStorageBlocks(const ShHandle handle,
//                                            int *out_count,
//                                            struct st_interface_block *out_ibs)
// {
//     const std::vector<sh::InterfaceBlock> *ibs = sh::GetShaderStorageBlocks(handle);
//     *out_count                                 = ibs->size();
// 
//     if (!out_ibs)
//         return;
// 
//     for (int i = 0; i < *out_count; i++)
//     {
//         copy_interface_block((*ibs)[i], out_ibs + i);
//     }
// }
// 
// ANGLE_EXPORT void STGetComputeShaderLocalGroupSize(const ShHandle handle,
//                                                    struct st_work_group_size *out_wg)
// {
//     sh::WorkGroupSize wg = sh::GetComputeShaderLocalGroupSize(handle);
//     copy_work_group_size(wg, out_wg);
// }
// 
// ANGLE_EXPORT int STGetVertexShaderNumViews(const ShHandle handle)
// {
//     return sh::GetVertexShaderNumViews(handle);
// }
// }
// 
// ANGLE_EXPORT bool STCompileAndGenerateResolveState(
//     const ShHandle handle, 
//     const char *const *shaderStrings,
//     size_t numStrings,
//     st_shader_resolve_state** state_out) {
// 
//     if (!mState.compilePending())
//     {
//         return;
//     }
// 
//     ASSERT(mCompilingState.get());
// 
//     mCompilingState->compileEvent->wait();
// 
//     mInfoLog += mCompilingState->compileEvent->getInfoLog();
// 
//     ScopedExit exit([this]() {
//         mBoundCompiler->putInstance(std::move(mCompilingState->shCompilerInstance));
//         mCompilingState->compileEvent.reset();
//         mCompilingState.reset();
//     });
// 
//     ShHandle compilerHandle = mCompilingState->shCompilerInstance.getHandle();
//     if (!mCompilingState->compileEvent->getResult())
//     {
//         mInfoLog += sh::GetInfoLog(compilerHandle);
//         WARN() << std::endl << mInfoLog;
//         mState.mCompileStatus = CompileStatus::NOT_COMPILED;
//         return;
//     }
// 
//     mState.mTranslatedSource = sh::GetObjectCode(compilerHandle);
// 
// #if !defined(NDEBUG)
//     // Prefix translated shader with commented out un-translated shader.
//     // Useful in diagnostics tools which capture the shader source.
//     std::ostringstream shaderStream;
//     shaderStream << "// GLSL\n";
//     shaderStream << "//\n";
// 
//     std::istringstream inputSourceStream(mState.mSource);
//     std::string line;
//     while (std::getline(inputSourceStream, line))
//     {
//         // Remove null characters from the source line
//         line.erase(std::remove(line.begin(), line.end(), '\0'), line.end());
// 
//         shaderStream << "// " << line;
// 
//         // glslang complains if a comment ends with backslash
//         if (!line.empty() && line.back() == '\\')
//         {
//             shaderStream << "\\";
//         }
// 
//         shaderStream << std::endl;
//     }
//     shaderStream << "\n\n";
//     shaderStream << mState.mTranslatedSource;
//     mState.mTranslatedSource = shaderStream.str();
// #endif  // !defined(NDEBUG)
// 
//     // Gather the shader information
//     mState.mShaderVersion = sh::GetShaderVersion(compilerHandle);
// 
//     mState.mUniforms            = GetShaderVariables(sh::GetUniforms(compilerHandle));
//     mState.mUniformBlocks       = GetShaderVariables(sh::GetUniformBlocks(compilerHandle));
//     mState.mShaderStorageBlocks = GetShaderVariables(sh::GetShaderStorageBlocks(compilerHandle));
// 
//     switch (mState.mShaderType)
//     {
//         case ShaderType::Compute:
//         {
//             mState.mAllAttributes    = GetShaderVariables(sh::GetAttributes(compilerHandle));
//             mState.mActiveAttributes = GetActiveShaderVariables(&mState.mAllAttributes);
//             mState.mLocalSize        = sh::GetComputeShaderLocalGroupSize(compilerHandle);
//             if (mState.mLocalSize.isDeclared())
//             {
//                 angle::CheckedNumeric<uint32_t> checked_local_size_product(mState.mLocalSize[0]);
//                 checked_local_size_product *= mState.mLocalSize[1];
//                 checked_local_size_product *= mState.mLocalSize[2];
// 
//                 if (!checked_local_size_product.IsValid())
//                 {
//                     WARN() << std::endl
//                            << "Integer overflow when computing the product of local_size_x, "
//                            << "local_size_y and local_size_z.";
//                     mState.mCompileStatus = CompileStatus::NOT_COMPILED;
//                     return;
//                 }
//                 if (checked_local_size_product.ValueOrDie() >
//                     mCurrentMaxComputeWorkGroupInvocations)
//                 {
//                     WARN() << std::endl
//                            << "The total number of invocations within a work group exceeds "
//                            << "MAX_COMPUTE_WORK_GROUP_INVOCATIONS.";
//                     mState.mCompileStatus = CompileStatus::NOT_COMPILED;
//                     return;
//                 }
//             }
// 
//             unsigned int sharedMemSize = sh::GetShaderSharedMemorySize(compilerHandle);
//             if (sharedMemSize > mMaxComputeSharedMemory)
//             {
//                 WARN() << std::endl << "Exceeded maximum shared memory size";
//                 mState.mCompileStatus = CompileStatus::NOT_COMPILED;
//                 return;
//             }
//             break;
//         }
//         case ShaderType::Vertex:
//         {
//             mState.mOutputVaryings   = GetShaderVariables(sh::GetOutputVaryings(compilerHandle));
//             mState.mAllAttributes    = GetShaderVariables(sh::GetAttributes(compilerHandle));
//             mState.mActiveAttributes = GetActiveShaderVariables(&mState.mAllAttributes);
//             mState.mNumViews         = sh::GetVertexShaderNumViews(compilerHandle);
//             break;
//         }
//         case ShaderType::Fragment:
//         {
//             mState.mAllAttributes    = GetShaderVariables(sh::GetAttributes(compilerHandle));
//             mState.mActiveAttributes = GetActiveShaderVariables(&mState.mAllAttributes);
//             mState.mInputVaryings    = GetShaderVariables(sh::GetInputVaryings(compilerHandle));
//             // TODO(jmadill): Figure out why we only sort in the FS, and if we need to.
//             std::sort(mState.mInputVaryings.begin(), mState.mInputVaryings.end(), CompareShaderVar);
//             mState.mActiveOutputVariables =
//                 GetActiveShaderVariables(sh::GetOutputVariables(compilerHandle));
//             mState.mEarlyFragmentTestsOptimization =
//                 sh::HasEarlyFragmentTestsOptimization(compilerHandle);
//             break;
//         }
//         case ShaderType::Geometry:
//         {
//             mState.mInputVaryings  = GetShaderVariables(sh::GetInputVaryings(compilerHandle));
//             mState.mOutputVaryings = GetShaderVariables(sh::GetOutputVaryings(compilerHandle));
// 
//             if (sh::HasValidGeometryShaderInputPrimitiveType(compilerHandle))
//             {
//                 mState.mGeometryShaderInputPrimitiveType = FromGLenum<PrimitiveMode>(
//                     sh::GetGeometryShaderInputPrimitiveType(compilerHandle));
//             }
//             if (sh::HasValidGeometryShaderOutputPrimitiveType(compilerHandle))
//             {
//                 mState.mGeometryShaderOutputPrimitiveType = FromGLenum<PrimitiveMode>(
//                     sh::GetGeometryShaderOutputPrimitiveType(compilerHandle));
//             }
//             if (sh::HasValidGeometryShaderMaxVertices(compilerHandle))
//             {
//                 mState.mGeometryShaderMaxVertices =
//                     sh::GetGeometryShaderMaxVertices(compilerHandle);
//             }
//             mState.mGeometryShaderInvocations = sh::GetGeometryShaderInvocations(compilerHandle);
//             break;
//         }
//         default:
//             UNREACHABLE();
//     }
// 
//     ASSERT(!mState.mTranslatedSource.empty());
// 
//     bool success          = mCompilingState->compileEvent->postTranslate(&mInfoLog);
//     mState.mCompileStatus = success ? CompileStatus::COMPILED : CompileStatus::NOT_COMPILED;
// }


} // extern "C"
