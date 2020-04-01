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
    INVARIANT                 = 258,
    PRECISE                   = 259,
    HIGH_PRECISION            = 260,
    MEDIUM_PRECISION          = 261,
    LOW_PRECISION             = 262,
    PRECISION                 = 263,
    ATTRIBUTE                 = 264,
    CONST_QUAL                = 265,
    BOOL_TYPE                 = 266,
    FLOAT_TYPE                = 267,
    INT_TYPE                  = 268,
    UINT_TYPE                 = 269,
    BREAK                     = 270,
    CONTINUE                  = 271,
    DO                        = 272,
    ELSE                      = 273,
    FOR                       = 274,
    IF                        = 275,
    DISCARD                   = 276,
    RETURN                    = 277,
    SWITCH                    = 278,
    CASE                      = 279,
    DEFAULT                   = 280,
    BVEC2                     = 281,
    BVEC3                     = 282,
    BVEC4                     = 283,
    IVEC2                     = 284,
    IVEC3                     = 285,
    IVEC4                     = 286,
    VEC2                      = 287,
    VEC3                      = 288,
    VEC4                      = 289,
    UVEC2                     = 290,
    UVEC3                     = 291,
    UVEC4                     = 292,
    MATRIX2                   = 293,
    MATRIX3                   = 294,
    MATRIX4                   = 295,
    IN_QUAL                   = 296,
    OUT_QUAL                  = 297,
    INOUT_QUAL                = 298,
    UNIFORM                   = 299,
    BUFFER                    = 300,
    VARYING                   = 301,
    MATRIX2x3                 = 302,
    MATRIX3x2                 = 303,
    MATRIX2x4                 = 304,
    MATRIX4x2                 = 305,
    MATRIX3x4                 = 306,
    MATRIX4x3                 = 307,
    CENTROID                  = 308,
    FLAT                      = 309,
    SMOOTH                    = 310,
    NOPERSPECTIVE             = 311,
    READONLY                  = 312,
    WRITEONLY                 = 313,
    COHERENT                  = 314,
    RESTRICT                  = 315,
    VOLATILE                  = 316,
    SHARED                    = 317,
    STRUCT                    = 318,
    VOID_TYPE                 = 319,
    WHILE                     = 320,
    SAMPLER2D                 = 321,
    SAMPLERCUBE               = 322,
    SAMPLER_EXTERNAL_OES      = 323,
    SAMPLER2DRECT             = 324,
    SAMPLER2DARRAY            = 325,
    ISAMPLER2D                = 326,
    ISAMPLER3D                = 327,
    ISAMPLERCUBE              = 328,
    ISAMPLER2DARRAY           = 329,
    USAMPLER2D                = 330,
    USAMPLER3D                = 331,
    USAMPLERCUBE              = 332,
    USAMPLER2DARRAY           = 333,
    SAMPLER2DMS               = 334,
    ISAMPLER2DMS              = 335,
    USAMPLER2DMS              = 336,
    SAMPLER2DMSARRAY          = 337,
    ISAMPLER2DMSARRAY         = 338,
    USAMPLER2DMSARRAY         = 339,
    SAMPLER3D                 = 340,
    SAMPLER3DRECT             = 341,
    SAMPLER2DSHADOW           = 342,
    SAMPLERCUBESHADOW         = 343,
    SAMPLER2DARRAYSHADOW      = 344,
    SAMPLERVIDEOWEBGL         = 345,
    SAMPLERCUBEARRAY          = 346,
    SAMPLERCUBEARRAYSHADOW    = 347,
    ISAMPLERCUBEARRAY         = 348,
    USAMPLERCUBEARRAY         = 349,
    SAMPLEREXTERNAL2DY2YEXT   = 350,
    IMAGE2D                   = 351,
    IIMAGE2D                  = 352,
    UIMAGE2D                  = 353,
    IMAGE3D                   = 354,
    IIMAGE3D                  = 355,
    UIMAGE3D                  = 356,
    IMAGE2DARRAY              = 357,
    IIMAGE2DARRAY             = 358,
    UIMAGE2DARRAY             = 359,
    IMAGECUBE                 = 360,
    IIMAGECUBE                = 361,
    UIMAGECUBE                = 362,
    IMAGECUBEARRAY            = 363,
    IIMAGECUBEARRAY           = 364,
    UIMAGECUBEARRAY           = 365,
    ATOMICUINT                = 366,
    LAYOUT                    = 367,
    YUVCSCSTANDARDEXT         = 368,
    YUVCSCSTANDARDEXTCONSTANT = 369,
    IDENTIFIER                = 370,
    TYPE_NAME                 = 371,
    FLOATCONSTANT             = 372,
    INTCONSTANT               = 373,
    UINTCONSTANT              = 374,
    BOOLCONSTANT              = 375,
    FIELD_SELECTION           = 376,
    LEFT_OP                   = 377,
    RIGHT_OP                  = 378,
    INC_OP                    = 379,
    DEC_OP                    = 380,
    LE_OP                     = 381,
    GE_OP                     = 382,
    EQ_OP                     = 383,
    NE_OP                     = 384,
    AND_OP                    = 385,
    OR_OP                     = 386,
    XOR_OP                    = 387,
    MUL_ASSIGN                = 388,
    DIV_ASSIGN                = 389,
    ADD_ASSIGN                = 390,
    MOD_ASSIGN                = 391,
    LEFT_ASSIGN               = 392,
    RIGHT_ASSIGN              = 393,
    AND_ASSIGN                = 394,
    XOR_ASSIGN                = 395,
    OR_ASSIGN                 = 396,
    SUB_ASSIGN                = 397,
    LEFT_PAREN                = 398,
    RIGHT_PAREN               = 399,
    LEFT_BRACKET              = 400,
    RIGHT_BRACKET             = 401,
    LEFT_BRACE                = 402,
    RIGHT_BRACE               = 403,
    DOT                       = 404,
    COMMA                     = 405,
    COLON                     = 406,
    EQUAL                     = 407,
    SEMICOLON                 = 408,
    BANG                      = 409,
    DASH                      = 410,
    TILDE                     = 411,
    PLUS                      = 412,
    STAR                      = 413,
    SLASH                     = 414,
    PERCENT                   = 415,
    LEFT_ANGLE                = 416,
    RIGHT_ANGLE               = 417,
    VERTICAL_BAR              = 418,
    CARET                     = 419,
    AMPERSAND                 = 420,
    QUESTION                  = 421
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
