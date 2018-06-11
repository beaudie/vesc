
#include "ShaderTranslator.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sstream>
#include <vector>

extern "C" {

static bool sInitialized = false;

ANGLE_EXPORT void STInitialize(void)
{
    if (!sInitialized) {
        sh::Initialize();
    }
}

ANGLE_EXPORT void STFinalize(void)
{
    if (sInitialized) {
        sh::Initialize();
        sInitialized = false;
    }
}

ANGLE_EXPORT ShBuiltInResources *STGenerateResources(ShBuiltInResources *resources)
{
    sh::InitBuiltInResources(resources);
    return resources;
}

ANGLE_EXPORT ShHandle STConstructCompiler(GLenum type,
                                          ShShaderSpec spec,
                                          ShShaderOutput output,
                                          ShBuiltInResources *resources)
{
    return sh::ConstructCompiler(type, spec, output, resources);
}

ANGLE_EXPORT bool STCompile(ShHandle compiler,
                            const char *const *shaderStrings,
                            size_t numStrings,
                            ShCompileOptions compileOptions)
{
    return sh::Compile(compiler, shaderStrings, numStrings, compileOptions);
}

ANGLE_EXPORT void STClearResults(const ShHandle handle)
{
    sh::ClearResults(handle);
}

ANGLE_EXPORT char *STGetInfoLog(const ShHandle handle)
{
    std::string res = sh::GetInfoLog(handle);
    char *result    = (char *)malloc(res.size() + 1);
    memset(result, 0, res.size() + 1);
    memcpy(result, res.data(), res.size());
    return result;
}

ANGLE_EXPORT char *STGetObjectCode(const ShHandle handle)
{
    std::string res = sh::GetObjectCode(handle);
    char *result    = (char *)malloc(res.size() + 1);
    memset(result, 0, res.size() + 1);
    memcpy(result, res.data(), res.size());
    return result;
}

ANGLE_EXPORT st_name_hashing_map STGetNameHashingMap(const ShHandle handle)
{

    st_name_hashing_map res;

    const std::map<std::string, std::string> *nameMap = sh::GetNameHashingMap(handle);

    res.count          = nameMap->size();
    res.user_names     = new char *[res.count];
    res.compiled_names = new char *[res.count];

    int i = 0;
    for (auto elt : *nameMap)
    {
        res.user_names[i] = new char[elt.first.size() + 1];
        memset(res.user_names[i], 0, elt.first.size() + 1);
        memcpy(res.user_names[i], elt.first.data(), elt.first.size());

        res.compiled_names[i] = new char[elt.second.size() + 1];
        memset(res.compiled_names[i], 0, elt.second.size() + 1);
        memcpy(res.compiled_names[i], elt.second.data(), elt.second.size());

        i++;
    }

    return res;
}

static char *to_alloced_buffer(const std::string &str)
{
    char *res = (char *)malloc(str.size() + 1);
    memset(res, 0, str.size() + 1);
    memcpy(res, str.data(), str.size());
    return res;
}

static void copy_var(const sh::ShaderVariable &var, struct st_shader_variable *out_var)
{
    out_var->type      = var.type;
    out_var->precision = var.precision;

    out_var->name        = to_alloced_buffer(var.name);
    out_var->mapped_name = to_alloced_buffer(var.mappedName);

    out_var->array_sizes_count = var.arraySizes.size();
    out_var->array_sizes       = 0;
    if (out_var->array_sizes_count)
    {
        out_var->array_sizes =
            (unsigned int *)malloc(out_var->array_sizes_count * sizeof(unsigned int));
        memcpy(out_var->array_sizes, var.arraySizes.data(),
               (out_var->array_sizes_count * sizeof(unsigned int)));
    }

    out_var->flattened_offset_in_parent_arrays = var.flattenedOffsetInParentArrays;
    out_var->static_use                        = var.staticUse;
    out_var->active                            = var.active;

    out_var->fields_count = var.fields.size();
    out_var->fields       = 0;
    if (out_var->fields_count)
    {
        out_var->fields = (struct st_shader_variable *)malloc(out_var->fields_count *
                                                              sizeof(struct st_shader_variable));
        for (int i = 0; i < out_var->fields_count; i++)
        {
            copy_var(var.fields[i], out_var->fields + i);
        }
    }

    out_var->struct_name = to_alloced_buffer(var.structName);
}

static void copy_var_with_location(const sh::VariableWithLocation &var,
                                   struct st_shader_variable *out_var)
{
    copy_var(var, out_var);
    out_var->location = var.location;
}

static void copy_uniform(const sh::Uniform &uniform, struct st_shader_variable *out_var)
{
    copy_var_with_location(uniform, out_var);

    out_var->binding   = uniform.binding;
    out_var->offset    = uniform.offset;
    out_var->readonly  = uniform.readonly;
    out_var->writeonly = uniform.writeonly;

    out_var->st_var_type = ST_VARIABLE_UNIFORM;
}

static void copy_attribute(const sh::Attribute &var, struct st_shader_variable *out_var)
{
    copy_var_with_location(var, out_var);
    out_var->st_var_type = ST_VARIABLE_ATTRIBUTE;
}

static void copy_output_variable(const sh::OutputVariable &var, struct st_shader_variable *out_var)
{
    copy_var_with_location(var, out_var);
    out_var->st_var_type = ST_VARIABLE_OUTPUT;
}

static void copy_interface_block_field(const sh::InterfaceBlockField &var,
                                       struct st_shader_variable *out_var)
{
    copy_var(var, out_var);
    out_var->is_row_major_layout = var.isRowMajorLayout;
    out_var->st_var_type         = ST_VARIABLE_INTERFACE_BLOCK_FIELD;
}

static void copy_varying(const sh::Varying &var, struct st_shader_variable *out_var)
{
    copy_var_with_location(var, out_var);
    out_var->interpolation = (st_interpolation_qualifier)var.interpolation;
    out_var->is_invariant  = var.isInvariant;
    out_var->is_built_in = var.isBuiltIn();
    out_var->st_var_type   = ST_VARIABLE_VARYING;
}

static void copy_interface_block(const sh::InterfaceBlock &ib, struct st_interface_block *out_ib)
{
    out_ib->name          = to_alloced_buffer(ib.name);
    out_ib->mapped_name   = to_alloced_buffer(ib.mappedName);
    out_ib->instance_name = to_alloced_buffer(ib.instanceName);
    out_ib->array_size    = ib.arraySize;

    out_ib->layout              = (st_block_layout_type)ib.layout;
    out_ib->is_row_major_layout = ib.isRowMajorLayout;

    out_ib->binding    = ib.binding;
    out_ib->static_use = ib.staticUse;
    out_ib->active     = ib.active;

    out_ib->block_type = (st_block_type)ib.blockType;

    out_ib->fields_count = ib.fields.size();
    out_ib->fields       = 0;
    if (out_ib->fields_count)
    {
        out_ib->fields = (struct st_shader_variable *)malloc(out_ib->fields_count *
                                                             sizeof(struct st_shader_variable));
        for (int i = 0; i < out_ib->fields_count; i++)
        {
            copy_interface_block_field(ib.fields[i], out_ib->fields + i);
        }
    }
}

static void copy_work_group_size(const sh::WorkGroupSize &wg, struct st_work_group_size *out_wg)
{
    for (int i = 0; i < (int)wg.localSizeQualifiers.size(); i++)
    {
        out_wg->local_size_qualifiers[i] = wg.localSizeQualifiers[i];
    }
}

ANGLE_EXPORT void STGetUniforms(const ShHandle handle,
                                int *out_count,
                                struct st_shader_variable *out_vars)
{
    const std::vector<sh::Uniform> *vars = sh::GetUniforms(handle);
    *out_count                           = vars->size();

    if (!out_vars)
        return;

    for (int i = 0; i < *out_count; i++)
    {
        copy_uniform((*vars)[i], out_vars + i);
    }
}

ANGLE_EXPORT void STGetVaryings(const ShHandle handle,
                                int *out_count,
                                struct st_shader_variable *out_vars)
{
    const std::vector<sh::Varying> *vars = sh::GetVaryings(handle);
    *out_count                           = vars->size();

    if (!out_vars)
        return;

    for (int i = 0; i < *out_count; i++)
    {
        copy_varying((*vars)[i], out_vars + i);
    }
}

ANGLE_EXPORT void STGetInputVaryings(const ShHandle handle,
                                     int *out_count,
                                     struct st_shader_variable *out_vars)
{
    const std::vector<sh::Varying> *vars = sh::GetInputVaryings(handle);
    *out_count                           = vars->size();

    if (!out_vars)
        return;

    for (int i = 0; i < *out_count; i++)
    {
        copy_varying((*vars)[i], out_vars + i);
    }
}

ANGLE_EXPORT void STGetOutputVaryings(const ShHandle handle,
                                      int *out_count,
                                      struct st_shader_variable *out_vars)
{
    const std::vector<sh::Varying> *vars = sh::GetOutputVaryings(handle);
    *out_count                           = vars->size();

    if (!out_vars)
        return;

    for (int i = 0; i < *out_count; i++)
    {
        copy_varying((*vars)[i], out_vars + i);
    }
}

ANGLE_EXPORT void STGetAttributes(const ShHandle handle,
                                  int *out_count,
                                  struct st_shader_variable *out_vars)
{
    const std::vector<sh::Attribute> *vars = sh::GetAttributes(handle);
    *out_count                             = vars->size();

    if (!out_vars)
        return;

    for (int i = 0; i < *out_count; i++)
    {
        copy_attribute((*vars)[i], out_vars + i);
    }
}

ANGLE_EXPORT void STGetOutputVariables(const ShHandle handle,
                                       int *out_count,
                                       struct st_shader_variable *out_vars)
{
    const std::vector<sh::OutputVariable> *vars = sh::GetOutputVariables(handle);
    *out_count                                  = vars->size();

    if (!out_vars)
        return;

    for (int i = 0; i < *out_count; i++)
    {
        copy_output_variable((*vars)[i], out_vars + i);
    }
}

ANGLE_EXPORT void STGetInterfaceBlocks(const ShHandle handle,
                                       int *out_count,
                                       struct st_interface_block *out_ibs)
{
    const std::vector<sh::InterfaceBlock> *ibs = sh::GetInterfaceBlocks(handle);
    *out_count                                 = ibs->size();

    if (!out_ibs)
        return;

    for (int i = 0; i < *out_count; i++)
    {
        copy_interface_block((*ibs)[i], out_ibs + i);
    }
}

ANGLE_EXPORT void STGetUniformBlocks(const ShHandle handle,
                                     int *out_count,
                                     struct st_interface_block *out_ibs)
{
    const std::vector<sh::InterfaceBlock> *ibs = sh::GetUniformBlocks(handle);
    *out_count                                 = ibs->size();

    if (!out_ibs)
        return;

    for (int i = 0; i < *out_count; i++)
    {
        copy_interface_block((*ibs)[i], out_ibs + i);
    }
}

ANGLE_EXPORT void STGetShaderStorageBlocks(const ShHandle handle,
                                           int *out_count,
                                           struct st_interface_block *out_ibs)
{
    const std::vector<sh::InterfaceBlock> *ibs = sh::GetShaderStorageBlocks(handle);
    *out_count                                 = ibs->size();

    if (!out_ibs)
        return;

    for (int i = 0; i < *out_count; i++)
    {
        copy_interface_block((*ibs)[i], out_ibs + i);
    }
}

ANGLE_EXPORT void STGetComputeShaderLocalGroupSize(const ShHandle handle,
                                                   struct st_work_group_size *out_wg)
{
    sh::WorkGroupSize wg = sh::GetComputeShaderLocalGroupSize(handle);
    copy_work_group_size(wg, out_wg);
}

ANGLE_EXPORT int STGetVertexShaderNumViews(const ShHandle handle)
{
    return sh::GetVertexShaderNumViews(handle);
}
}
