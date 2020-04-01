/* A Bison parser, made by GNU Bison 3.3.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2019 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* Undocumented macros, especially those whose name start with YY_,
   are private implementation details.  Do not rely on them.  */

#ifndef YY_YY_GLSLANG_TAB_AUTOGEN_H_INCLUDED
#define YY_YY_GLSLANG_TAB_AUTOGEN_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
#    define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif
/* "%code requires" blocks.  */

#define YYLTYPE TSourceLoc
#define YYLTYPE_IS_DECLARED 1
#define YYLTYPE_IS_TRIVIAL 1

/* Token type.  */
#ifndef YYTOKENTYPE
#    define YYTOKENTYPE
enum yytokentype
{
    INVARIANT                    = 258,
    PRECISE                      = 259,
    HIGH_PRECISION               = 260,
    MEDIUM_PRECISION             = 261,
    LOW_PRECISION                = 262,
    PRECISION                    = 263,
    ATTRIBUTE                    = 264,
    CONST_QUAL                   = 265,
    BOOL_TYPE                    = 266,
    FLOAT_TYPE                   = 267,
    INT_TYPE                     = 268,
    UINT_TYPE                    = 269,
    BREAK                        = 270,
    CONTINUE                     = 271,
    DO                           = 272,
    ELSE                         = 273,
    FOR                          = 274,
    IF                           = 275,
    DISCARD                      = 276,
    RETURN                       = 277,
    SWITCH                       = 278,
    CASE                         = 279,
    DEFAULT                      = 280,
    BVEC2                        = 281,
    BVEC3                        = 282,
    BVEC4                        = 283,
    IVEC2                        = 284,
    IVEC3                        = 285,
    IVEC4                        = 286,
    VEC2                         = 287,
    VEC3                         = 288,
    VEC4                         = 289,
    UVEC2                        = 290,
    UVEC3                        = 291,
    UVEC4                        = 292,
    MATRIX2                      = 293,
    MATRIX3                      = 294,
    MATRIX4                      = 295,
    IN_QUAL                      = 296,
    OUT_QUAL                     = 297,
    INOUT_QUAL                   = 298,
    UNIFORM                      = 299,
    BUFFER                       = 300,
    VARYING                      = 301,
    MATRIX2x3                    = 302,
    MATRIX3x2                    = 303,
    MATRIX2x4                    = 304,
    MATRIX4x2                    = 305,
    MATRIX3x4                    = 306,
    MATRIX4x3                    = 307,
    CENTROID                     = 308,
    FLAT                         = 309,
    SMOOTH                       = 310,
    READONLY                     = 311,
    WRITEONLY                    = 312,
    COHERENT                     = 313,
    RESTRICT                     = 314,
    VOLATILE                     = 315,
    SHARED                       = 316,
    STRUCT                       = 317,
    VOID_TYPE                    = 318,
    WHILE                        = 319,
    SAMPLER2D                    = 320,
    SAMPLERCUBE                  = 321,
    SAMPLER_EXTERNAL_OES         = 322,
    SAMPLER2DRECT                = 323,
    SAMPLER2DARRAY               = 324,
    ISAMPLER2D                   = 325,
    ISAMPLER3D                   = 326,
    ISAMPLERCUBE                 = 327,
    ISAMPLER2DARRAY              = 328,
    USAMPLER2D                   = 329,
    USAMPLER3D                   = 330,
    USAMPLERCUBE                 = 331,
    USAMPLER2DARRAY              = 332,
    SAMPLER2DMS                  = 333,
    ISAMPLER2DMS                 = 334,
    USAMPLER2DMS                 = 335,
    SAMPLER2DMSARRAY             = 336,
    ISAMPLER2DMSARRAY            = 337,
    USAMPLER2DMSARRAY            = 338,
    SAMPLER3D                    = 339,
    SAMPLER3DRECT                = 340,
    SAMPLER2DSHADOW              = 341,
    SAMPLERCUBESHADOW            = 342,
    SAMPLER2DARRAYSHADOW         = 343,
    SAMPLERVIDEOWEBGL            = 344,
    SAMPLERCUBEMAPARRAYOES       = 345,
    SAMPLERCUBEMAPARRAYSHADOWOES = 346,
    ISAMPLERCUBEMAPARRAYOES      = 347,
    USAMPLERCUBEMAPARRAYOES      = 348,
    SAMPLEREXTERNAL2DY2YEXT      = 349,
    IMAGE2D                      = 350,
    IIMAGE2D                     = 351,
    UIMAGE2D                     = 352,
    IMAGE3D                      = 353,
    IIMAGE3D                     = 354,
    UIMAGE3D                     = 355,
    IMAGE2DARRAY                 = 356,
    IIMAGE2DARRAY                = 357,
    UIMAGE2DARRAY                = 358,
    IMAGECUBE                    = 359,
    IIMAGECUBE                   = 360,
    UIMAGECUBE                   = 361,
    IMAGECUBEMAPARRAYOES         = 362,
    IIMAGECUBEMAPARRAYOES        = 363,
    UIMAGECUBEMAPARRAYOES        = 364,
    ATOMICUINT                   = 365,
    LAYOUT                       = 366,
    YUVCSCSTANDARDEXT            = 367,
    YUVCSCSTANDARDEXTCONSTANT    = 368,
    IDENTIFIER                   = 369,
    TYPE_NAME                    = 370,
    FLOATCONSTANT                = 371,
    INTCONSTANT                  = 372,
    UINTCONSTANT                 = 373,
    BOOLCONSTANT                 = 374,
    FIELD_SELECTION              = 375,
    LEFT_OP                      = 376,
    RIGHT_OP                     = 377,
    INC_OP                       = 378,
    DEC_OP                       = 379,
    LE_OP                        = 380,
    GE_OP                        = 381,
    EQ_OP                        = 382,
    NE_OP                        = 383,
    AND_OP                       = 384,
    OR_OP                        = 385,
    XOR_OP                       = 386,
    MUL_ASSIGN                   = 387,
    DIV_ASSIGN                   = 388,
    ADD_ASSIGN                   = 389,
    MOD_ASSIGN                   = 390,
    LEFT_ASSIGN                  = 391,
    RIGHT_ASSIGN                 = 392,
    AND_ASSIGN                   = 393,
    XOR_ASSIGN                   = 394,
    OR_ASSIGN                    = 395,
    SUB_ASSIGN                   = 396,
    LEFT_PAREN                   = 397,
    RIGHT_PAREN                  = 398,
    LEFT_BRACKET                 = 399,
    RIGHT_BRACKET                = 400,
    LEFT_BRACE                   = 401,
    RIGHT_BRACE                  = 402,
    DOT                          = 403,
    COMMA                        = 404,
    COLON                        = 405,
    EQUAL                        = 406,
    SEMICOLON                    = 407,
    BANG                         = 408,
    DASH                         = 409,
    TILDE                        = 410,
    PLUS                         = 411,
    STAR                         = 412,
    SLASH                        = 413,
    PERCENT                      = 414,
    LEFT_ANGLE                   = 415,
    RIGHT_ANGLE                  = 416,
    VERTICAL_BAR                 = 417,
    CARET                        = 418,
    AMPERSAND                    = 419,
    QUESTION                     = 420
};
#endif

/* Value type.  */
#if !defined YYSTYPE && !defined YYSTYPE_IS_DECLARED

union YYSTYPE
{

    struct
    {
        union
        {
            const char *string;  // pool allocated.
            float f;
            int i;
            unsigned int u;
            bool b;
        };
        const TSymbol *symbol;
    } lex;
    struct
    {
        TOperator op;
        union
        {
            TIntermNode *intermNode;
            TIntermNodePair nodePair;
            TIntermTyped *intermTypedNode;
            TIntermAggregate *intermAggregate;
            TIntermBlock *intermBlock;
            TIntermDeclaration *intermDeclaration;
            TIntermFunctionPrototype *intermFunctionPrototype;
            TIntermSwitch *intermSwitch;
            TIntermCase *intermCase;
        };
        union
        {
            TVector<unsigned int> *arraySizes;
            TTypeSpecifierNonArray typeSpecifierNonArray;
            TPublicType type;
            TPrecision precision;
            TLayoutQualifier layoutQualifier;
            TQualifier qualifier;
            TFunction *function;
            TFunctionLookup *functionLookup;
            TParameter param;
            TDeclarator *declarator;
            TDeclaratorList *declaratorList;
            TFieldList *fieldList;
            TQualifierWrapperBase *qualifierWrapper;
            TTypeQualifierBuilder *typeQualifierBuilder;
        };
    } interm;
};

typedef union YYSTYPE YYSTYPE;
#    define YYSTYPE_IS_TRIVIAL 1
#    define YYSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if !defined YYLTYPE && !defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE YYLTYPE;
struct YYLTYPE
{
    int first_line;
    int first_column;
    int last_line;
    int last_column;
};
#    define YYLTYPE_IS_DECLARED 1
#    define YYLTYPE_IS_TRIVIAL 1
#endif

int yyparse(TParseContext *context, void *scanner);

#endif /* !YY_YY_GLSLANG_TAB_AUTOGEN_H_INCLUDED  */
