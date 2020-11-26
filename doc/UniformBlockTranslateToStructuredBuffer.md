# Translate Uniform Block to StructuredBuffer

## Background
In Angle Direct3D backend, we will generally translate GLSL uniform block to HLSL cbuffer. But there is a slow fxc compile performance issue with dynamic cbuffer indexing, [anglebug.com/3682](https://bugs.chromium.org/p/angleproject/issues/detail?id=3682)

## Solution
We plan to translate a uniform block into StructuredBuffer only when all the following three conditions are satisfied:
* The uniform block has only one array member, and the array size is larger or equal to 50;
* In the shader, all the accesses of the member are through indexing operator;
* The type of the array member must be any of the following,
  * The type is one of scalars or vectors;
  * The type is one of matrices, it must be mat2x4, mat3x4 and mat4x4 for column_major layout qualifier, and it must be mat4x2, mat4x3and mat4x4 for row_major qualifier;
  * The type is a structure, and its fields' types must satisfy the above two items, and any one of fields cannot be an array or a structure type.

## Analysis
Normally, we will translate GLSL uniform block to HLSL StructuredBuffer when there is only one large array member in uniform block to partially resolve the above issue. The translation is like below,
```
// GLSL code
uniform buffer {
    TYPE buf [NUMBER];
};
```
Will be translated into
```
// HLSL code
StructuredBuffer <TYPETRANSLATED>  bufTranslated: register(tN);
```

NUMBER: is a large number, is larger than or equal to 50.

However, even with the above limitation, there are still many situations that cannot be easily translated into StructuredBuffer. They are divided into two classes. One is to operate a uniform block array member as the whole entity. The other is caused by the std140 layout.

### Operate uniform block array member as whole entity
According to ESSL spec 3.0, 5.7 Structure and Array Operations,  the following operators are allowed to operate on arrays as whole entities,

|  Operator NameName         |    Operator    |
|  :----------------         |    :------:    |
|  field or method selector  |        .       |
|  assignment                |     ==  !=     |
|  Ternary operator          |       ?:       |
|  Sequence operator         |       ,        |
|  indexing                  |       []       |

However, after translating to StructuredBuffer, the uniform array member cannot be used as a whole entity since its type has been changed into a structuredBuffer not an array.
so we only support  indexing operator (most frequent usage), other operators and usage are not supported, some examples are as below,

|    Operator On the Uniform Array Member    |         examples          |
|              :----------------             |         :------           |
|    indexing                                |   TYPE var = buf[INDEX];  |
|    method selector                         |   buf.length();   // Angle don’t support it, too. |
|    equality == !=                          |   TYPE var[NUMBER] = {…}; <br> if (var == buf);   |
|    assignment =                            |   TYPE var[NUMBER] = {…}; <br> var = buf;         |
|    Ternary operator ?:                     |   // Angle don’t support it, too.                 |
|    Sequence operator ,                     |   TYPE var1[NUMBER] = {…}; <br> TYPE var2[NUMBER] = (var1, buf);      |
|    Function arguments                      |   void func(TYPE a[NUMBER]); <br> func(buf);                          |
|    Function return type                    |   TYPE[NUMBER] func() { return buf;}  <br> TYPE var[NUMBER] = func(); |

### Std140 limitation
Uniform block follows st140 layout rules, but StructuredBuffer has no these rules. So we may need to explicitly pad the type `TYPETRANSLATED` to follow std140 rules. Or we can just simply only support those types which don't need to be padded.
The types which can be directly supported are shown as below and we have already supported them.

|         GLSL TYPE          |     TRANSLATED HLSL TYPE      |
|         :------            |          :------              |
|   vec4/ivec4/uvec4/bvec4   |     float4/int4/uint4/bool4   |
|   mat2x4 (column_major)    |     float2x4 (row_major)      |
|   mat3x4 (column_major)    |     float3x4 (row_major)      |
|   mat4x4 (column_major)    |     float4x4 (row_major)      |
|   mat4x2 (row_major)       |     float4x3 (column_major)   |
|   mat4x3 (row_major)       |     float4x3 (column_major)   |
|   mat4x4 (row_major)       |     float4x4 (column_major)   |

The types which are not consistent with TYPETRANSLATED, but the implementation is easy and straightforward are shown as below and we have already supported them.

|         GLSL TYPE          |     TRANSLATED HLSL TYPE      |     examples        |
|         :------            |          :------              |     :------         |
|   float/int/uint/bool      |     float4/int4/uint4/bool4   | GLSL: float var = buf[0]; <br> HLSL: float var = buf[0].x;  |
|   vec2/ivec2/uvec2/bvec2   |     float4/int4/uint4/bool4   | GLSL: vec2 var = buf[0]; <br> HLSL: float2 var = buf[0].xy; |
|   vec3/ivec3/uvec3/bvec3   |     float4/int4/uint4/bool4   | GLSL: vec3 var = buf[0]; <br> HLSL: float3 var = buf[0].xyz;|

The types which are not consistent between TYPE and TYPETRANSLATED, and the implementation is difficult, and may exist side effects and be performance loss, we have not supported them now.

|         GLSL TYPE          |     TRANSLATED HLSL TYPE      |
|         :------            |          :------              |
|   mat2x2 (column_major)    |     float2x4 (row_major)      |
|   mat2x3 (column_major)    |     float2x4 (row_major)      |
|   mat3x2 (column_major)    |     float3x4 (row_major)      |
|   mat3x3 (column_major)    |     float3x4 (row_major)      |
|   mat4x2 (column_major)    |     float4x4 (row_major)      |
|   mat4x3 (column_major)    |     float4x4 (row_major)      |
|   mat2x2 (row_major)       |     float4x2 (column_major)   |
|   mat2x3 (row_major)       |     float4x3 (column_major)   |
|   mat2x4 (row_major)       |     float4x4 (column_major)   |
|   mat3x2 (row_major)       |     float4x2 (column_major)   |
|   mat3x3 (row_major)       |     float4x3 (column_major)   |
|   mat3x4 (row_major)       |     float4x4 (column_major)   |


Take mat3x2(column_major) for an example, the uniform buffer's memory layout is as below.

| index | 0 |   |   |   |   |   |   |   |   |   |   |   | 1 |   |   |   |   |   |   |   |   |   |   |   | 2 |   |
| :- | :- | :- | :- | :- | :- | :- | :- | :- | :- | :- | :- | :- | :- | :- | :- | :- | :- | :- | :- | :- | :- | :- | :- | :- | :- | :- |
| data | 1 | 2 | x | x | 3 | 4 | x | x | 5 | 6 | x | x | 7 | 8 | x | x | 9 | 10 | x | x | 11 | 12 | x | x | 13 | 14 |


And the declaration of uniform block in vertex shader may be as below.
```
layout(std140) uniform buffer {
    mat3x2 buf [100];
};

void main(void) {
    ...
    vec2 var = buf[0][2]
    ...
}
```
Will be translated to

```
#pragma pack_matrix(row_major)
StructuredBuffer<float3x4> bufTranslated: register(t0);

float3x2 GetFloat3x2FromFloat3x4Rowmajor(float3x4 mat)
{
    float3x2 res = { 0.0 };
    res[0] = mat[0].xy;
    res[1] = mat[1].xy;
    res[2] = mat[2].xy;
    return res;
}

VS_OUTPUT main(VS_INPUT input) {
    ...
    float3x2 var = GetFloat3x2FromFloat3x4Rowmajor(bufTranslated[0])
}
```

When accessing the element of the `buf` variable, we need to extract a float3x2 from a float3x4 for every element.