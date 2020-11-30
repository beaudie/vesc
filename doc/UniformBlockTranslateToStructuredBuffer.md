Take mat3x2(column_major) for an example, the uniform buffer’ memory layout is as below.

| index | A                       | B                          | C   |
| ----  | -----                   | -----                      | --  |
| data |     float4/int4/uint4/bool4   | asdb | asdf |

hello
|         GLSL TYPE          |     TRANSLATED HLSL TYPE      |
|         :------            |          :------              |
|   vec4/ivec4/uvec4/bvec4   |     float4/int4/uint4/bool4   |
|   mat2x4 (column_major)    |     float2x4 (row_major)      |
|   mat3x4 (column_major)    |     float3x4 (row_major)      |
|   mat4x4 (column_major)    |     float4x4 (row_major)      |
|   mat4x2 (row_major)       |     float4x3 (column_major)   |
|   mat4x3 (row_major)       |     float4x3 (column_major)   |
|   mat4x4 (row_major)       |     float4x4 (column_major)   |
