Take mat3x2(column_major) for an example, the uniform buffer’ memory layout is as below.

| index | A                       | B                          |
| ----  | -----                   | -----                      |
| data | 1 2 x x 3 4 x x 5 6 x x | 7 8 x x 9 10 x x 11 12 x x |

hello
|         GLSLTYPETYPE       |     TRANSLATED HLSL TYPE      |
|         :------            |          :------              |
|   vec4/ivec4/uvec4/bvec4   |     float4/int4/uint4/bool4   |

