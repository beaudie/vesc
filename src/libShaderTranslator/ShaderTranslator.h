#pragma once

struct NameHashingMap {
    char** user_names;
    char** compiled_names;
    int count;
};

// Shader variables

enum st_variable_datatype
{
    ST_VARIABLE_UNIFORM,
    ST_VARIABLE_ATTRIBUTE,
    ST_VARIABLE_OUTPUT,
    ST_VARIABLE_VARYING,
    ST_VARIABLE_INTERFACE_BLOCK_FIELD,
};

// Varying interpolation qualifier, see section 4.3.9 of the ESSL 3.00.4 spec
enum st_interpolation_qualifier
{
    ST_INTERPOLATION_SMOOTH,
    ST_INTERPOLATION_CENTROID,
    ST_INTERPOLATION_FLAT
};

// Uniform block layout qualifier, see section 4.3.8.3 of the ESSL 3.00.4 spec
enum st_block_layout_type
{
    ST_BLOCKLAYOUT_STANDARD,
    ST_BLOCKLAYOUT_STD140 = ST_BLOCKLAYOUT_STANDARD,
    ST_BLOCKLAYOUT_STD430,  // Shader storage block layout qualifier
    ST_BLOCKLAYOUT_PACKED,
    ST_BLOCKLAYOUT_SHARED
};

// Interface Blocks, see section 4.3.9 of the ESSL 3.10 spec
enum st_block_type
{
    ST_BLOCK_UNIFORM,
    ST_BLOCK_BUFFER,

    // Required in OpenGL ES 3.1 extension GL_OES_shader_io_blocks.
    // TODO(jiawei.shao@intel.com): add BLOCK_OUT.
    ST_BLOCK_IN
};

struct st_shader_variable {
    st_variable_datatype st_var_type;

    unsigned int type;
    unsigned int precision;
    int location;
    int binding;
    int offset;
    bool readonly;
    bool writeonly;
    bool is_row_major_layout;
    st_interpolation_qualifier interpolation;
    bool is_invariant;

    char* name;
    char* mapped_name;
    char* struct_name;

    unsigned int* array_sizes;
    int array_sizes_count;

    unsigned int flattened_offset_in_parent_arrays;

    bool static_use;
    bool active;

    st_shader_variable* fields;
    int fields_count;
};

struct st_interface_block {
    char* name;
    char* mapped_name;
    char* instance_name;
    unsigned int array_size;
    st_block_layout_type layout;

    bool is_row_major_layout;

    int binding;
    bool static_use;
    bool active;
    st_block_type block_type;

    st_shader_variable* fields;
    int fields_count;
};

struct st_work_group_size {
    int local_size_qualifiers[3];
};
