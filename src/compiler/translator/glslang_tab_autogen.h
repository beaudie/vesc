/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
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
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

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

/* Token kinds.  */
#ifndef YYTOKENTYPE
#    define YYTOKENTYPE
enum yytokentype
{
    YYEMPTY                   = -2,
    YYEOF                     = 0,   /* "end of file"  */
    YYerror                   = 256, /* error  */
    YYUNDEF                   = 257, /* "invalid token"  */
    INVARIANT                 = 258, /* INVARIANT  */
    PRECISE                   = 259, /* PRECISE  */
    HIGH_PRECISION            = 260, /* HIGH_PRECISION  */
    MEDIUM_PRECISION          = 261, /* MEDIUM_PRECISION  */
    LOW_PRECISION             = 262, /* LOW_PRECISION  */
    PRECISION                 = 263, /* PRECISION  */
    ATTRIBUTE                 = 264, /* ATTRIBUTE  */
    CONST_QUAL                = 265, /* CONST_QUAL  */
    BOOL_TYPE                 = 266, /* BOOL_TYPE  */
    FLOAT_TYPE                = 267, /* FLOAT_TYPE  */
    INT_TYPE                  = 268, /* INT_TYPE  */
    UINT_TYPE                 = 269, /* UINT_TYPE  */
    BREAK                     = 270, /* BREAK  */
    CONTINUE                  = 271, /* CONTINUE  */
    DO                        = 272, /* DO  */
    ELSE                      = 273, /* ELSE  */
    FOR                       = 274, /* FOR  */
    IF                        = 275, /* IF  */
    DISCARD                   = 276, /* DISCARD  */
    RETURN                    = 277, /* RETURN  */
    SWITCH                    = 278, /* SWITCH  */
    CASE                      = 279, /* CASE  */
    DEFAULT                   = 280, /* DEFAULT  */
    BVEC2                     = 281, /* BVEC2  */
    BVEC3                     = 282, /* BVEC3  */
    BVEC4                     = 283, /* BVEC4  */
    IVEC2                     = 284, /* IVEC2  */
    IVEC3                     = 285, /* IVEC3  */
    IVEC4                     = 286, /* IVEC4  */
    VEC2                      = 287, /* VEC2  */
    VEC3                      = 288, /* VEC3  */
    VEC4                      = 289, /* VEC4  */
    UVEC2                     = 290, /* UVEC2  */
    UVEC3                     = 291, /* UVEC3  */
    UVEC4                     = 292, /* UVEC4  */
    MATRIX2                   = 293, /* MATRIX2  */
    MATRIX3                   = 294, /* MATRIX3  */
    MATRIX4                   = 295, /* MATRIX4  */
    IN_QUAL                   = 296, /* IN_QUAL  */
    OUT_QUAL                  = 297, /* OUT_QUAL  */
    INOUT_QUAL                = 298, /* INOUT_QUAL  */
    UNIFORM                   = 299, /* UNIFORM  */
    BUFFER                    = 300, /* BUFFER  */
    VARYING                   = 301, /* VARYING  */
    MATRIX2x3                 = 302, /* MATRIX2x3  */
    MATRIX3x2                 = 303, /* MATRIX3x2  */
    MATRIX2x4                 = 304, /* MATRIX2x4  */
    MATRIX4x2                 = 305, /* MATRIX4x2  */
    MATRIX3x4                 = 306, /* MATRIX3x4  */
    MATRIX4x3                 = 307, /* MATRIX4x3  */
    SAMPLE                    = 308, /* SAMPLE  */
    CENTROID                  = 309, /* CENTROID  */
    FLAT                      = 310, /* FLAT  */
    SMOOTH                    = 311, /* SMOOTH  */
    NOPERSPECTIVE             = 312, /* NOPERSPECTIVE  */
    PATCHOES                  = 313, /* PATCHOES  */
    PATCHEXT                  = 314, /* PATCHEXT  */
    READONLY                  = 315, /* READONLY  */
    WRITEONLY                 = 316, /* WRITEONLY  */
    COHERENT                  = 317, /* COHERENT  */
    RESTRICT                  = 318, /* RESTRICT  */
    VOLATILE                  = 319, /* VOLATILE  */
    SHARED                    = 320, /* SHARED  */
    STRUCT                    = 321, /* STRUCT  */
    VOID_TYPE                 = 322, /* VOID_TYPE  */
    WHILE                     = 323, /* WHILE  */
    SAMPLER2D                 = 324, /* SAMPLER2D  */
    SAMPLERCUBE               = 325, /* SAMPLERCUBE  */
    SAMPLER_EXTERNAL_OES      = 326, /* SAMPLER_EXTERNAL_OES  */
    SAMPLER2DRECT             = 327, /* SAMPLER2DRECT  */
    SAMPLER2DARRAY            = 328, /* SAMPLER2DARRAY  */
    ISAMPLER2D                = 329, /* ISAMPLER2D  */
    ISAMPLER3D                = 330, /* ISAMPLER3D  */
    ISAMPLERCUBE              = 331, /* ISAMPLERCUBE  */
    ISAMPLER2DARRAY           = 332, /* ISAMPLER2DARRAY  */
    USAMPLER2D                = 333, /* USAMPLER2D  */
    USAMPLER3D                = 334, /* USAMPLER3D  */
    USAMPLERCUBE              = 335, /* USAMPLERCUBE  */
    USAMPLER2DARRAY           = 336, /* USAMPLER2DARRAY  */
    SAMPLER2DMS               = 337, /* SAMPLER2DMS  */
    ISAMPLER2DMS              = 338, /* ISAMPLER2DMS  */
    USAMPLER2DMS              = 339, /* USAMPLER2DMS  */
    SAMPLER2DMSARRAY          = 340, /* SAMPLER2DMSARRAY  */
    ISAMPLER2DMSARRAY         = 341, /* ISAMPLER2DMSARRAY  */
    USAMPLER2DMSARRAY         = 342, /* USAMPLER2DMSARRAY  */
    SAMPLER3D                 = 343, /* SAMPLER3D  */
    SAMPLER3DRECT             = 344, /* SAMPLER3DRECT  */
    SAMPLER2DSHADOW           = 345, /* SAMPLER2DSHADOW  */
    SAMPLERCUBESHADOW         = 346, /* SAMPLERCUBESHADOW  */
    SAMPLER2DARRAYSHADOW      = 347, /* SAMPLER2DARRAYSHADOW  */
    SAMPLERVIDEOWEBGL         = 348, /* SAMPLERVIDEOWEBGL  */
    SAMPLERCUBEARRAYOES       = 349, /* SAMPLERCUBEARRAYOES  */
    SAMPLERCUBEARRAYSHADOWOES = 350, /* SAMPLERCUBEARRAYSHADOWOES  */
    ISAMPLERCUBEARRAYOES      = 351, /* ISAMPLERCUBEARRAYOES  */
    USAMPLERCUBEARRAYOES      = 352, /* USAMPLERCUBEARRAYOES  */
    SAMPLERCUBEARRAYEXT       = 353, /* SAMPLERCUBEARRAYEXT  */
    SAMPLERCUBEARRAYSHADOWEXT = 354, /* SAMPLERCUBEARRAYSHADOWEXT  */
    ISAMPLERCUBEARRAYEXT      = 355, /* ISAMPLERCUBEARRAYEXT  */
    USAMPLERCUBEARRAYEXT      = 356, /* USAMPLERCUBEARRAYEXT  */
    SAMPLERBUFFER             = 357, /* SAMPLERBUFFER  */
    ISAMPLERBUFFER            = 358, /* ISAMPLERBUFFER  */
    USAMPLERBUFFER            = 359, /* USAMPLERBUFFER  */
    SAMPLEREXTERNAL2DY2YEXT   = 360, /* SAMPLEREXTERNAL2DY2YEXT  */
    IMAGE2D                   = 361, /* IMAGE2D  */
    IIMAGE2D                  = 362, /* IIMAGE2D  */
    UIMAGE2D                  = 363, /* UIMAGE2D  */
    IMAGE3D                   = 364, /* IMAGE3D  */
    IIMAGE3D                  = 365, /* IIMAGE3D  */
    UIMAGE3D                  = 366, /* UIMAGE3D  */
    IMAGE2DARRAY              = 367, /* IMAGE2DARRAY  */
    IIMAGE2DARRAY             = 368, /* IIMAGE2DARRAY  */
    UIMAGE2DARRAY             = 369, /* UIMAGE2DARRAY  */
    IMAGECUBE                 = 370, /* IMAGECUBE  */
    IIMAGECUBE                = 371, /* IIMAGECUBE  */
    UIMAGECUBE                = 372, /* UIMAGECUBE  */
    IMAGECUBEARRAYOES         = 373, /* IMAGECUBEARRAYOES  */
    IIMAGECUBEARRAYOES        = 374, /* IIMAGECUBEARRAYOES  */
    UIMAGECUBEARRAYOES        = 375, /* UIMAGECUBEARRAYOES  */
    IMAGECUBEARRAYEXT         = 376, /* IMAGECUBEARRAYEXT  */
    IIMAGECUBEARRAYEXT        = 377, /* IIMAGECUBEARRAYEXT  */
    UIMAGECUBEARRAYEXT        = 378, /* UIMAGECUBEARRAYEXT  */
    IMAGEBUFFER               = 379, /* IMAGEBUFFER  */
    IIMAGEBUFFER              = 380, /* IIMAGEBUFFER  */
    UIMAGEBUFFER              = 381, /* UIMAGEBUFFER  */
    ATOMICUINT                = 382, /* ATOMICUINT  */
    PIXELLOCALANGLE           = 383, /* PIXELLOCALANGLE  */
    IPIXELLOCALANGLE          = 384, /* IPIXELLOCALANGLE  */
    UPIXELLOCALANGLE          = 385, /* UPIXELLOCALANGLE  */
    LAYOUT                    = 386, /* LAYOUT  */
    YUVCSCSTANDARDEXT         = 387, /* YUVCSCSTANDARDEXT  */
    YUVCSCSTANDARDEXTCONSTANT = 388, /* YUVCSCSTANDARDEXTCONSTANT  */
    IDENTIFIER                = 389, /* IDENTIFIER  */
    TYPE_NAME                 = 390, /* TYPE_NAME  */
    FLOATCONSTANT             = 391, /* FLOATCONSTANT  */
    INTCONSTANT               = 392, /* INTCONSTANT  */
    UINTCONSTANT              = 393, /* UINTCONSTANT  */
    BOOLCONSTANT              = 394, /* BOOLCONSTANT  */
    FIELD_SELECTION           = 395, /* FIELD_SELECTION  */
    LEFT_OP                   = 396, /* LEFT_OP  */
    RIGHT_OP                  = 397, /* RIGHT_OP  */
    INC_OP                    = 398, /* INC_OP  */
    DEC_OP                    = 399, /* DEC_OP  */
    LE_OP                     = 400, /* LE_OP  */
    GE_OP                     = 401, /* GE_OP  */
    EQ_OP                     = 402, /* EQ_OP  */
    NE_OP                     = 403, /* NE_OP  */
    AND_OP                    = 404, /* AND_OP  */
    OR_OP                     = 405, /* OR_OP  */
    XOR_OP                    = 406, /* XOR_OP  */
    MUL_ASSIGN                = 407, /* MUL_ASSIGN  */
    DIV_ASSIGN                = 408, /* DIV_ASSIGN  */
    ADD_ASSIGN                = 409, /* ADD_ASSIGN  */
    MOD_ASSIGN                = 410, /* MOD_ASSIGN  */
    LEFT_ASSIGN               = 411, /* LEFT_ASSIGN  */
    RIGHT_ASSIGN              = 412, /* RIGHT_ASSIGN  */
    AND_ASSIGN                = 413, /* AND_ASSIGN  */
    XOR_ASSIGN                = 414, /* XOR_ASSIGN  */
    OR_ASSIGN                 = 415, /* OR_ASSIGN  */
    SUB_ASSIGN                = 416, /* SUB_ASSIGN  */
    LEFT_PAREN                = 417, /* LEFT_PAREN  */
    RIGHT_PAREN               = 418, /* RIGHT_PAREN  */
    LEFT_BRACKET              = 419, /* LEFT_BRACKET  */
    RIGHT_BRACKET             = 420, /* RIGHT_BRACKET  */
    LEFT_BRACE                = 421, /* LEFT_BRACE  */
    RIGHT_BRACE               = 422, /* RIGHT_BRACE  */
    DOT                       = 423, /* DOT  */
    COMMA                     = 424, /* COMMA  */
    COLON                     = 425, /* COLON  */
    EQUAL                     = 426, /* EQUAL  */
    SEMICOLON                 = 427, /* SEMICOLON  */
    BANG                      = 428, /* BANG  */
    DASH                      = 429, /* DASH  */
    TILDE                     = 430, /* TILDE  */
    PLUS                      = 431, /* PLUS  */
    STAR                      = 432, /* STAR  */
    SLASH                     = 433, /* SLASH  */
    PERCENT                   = 434, /* PERCENT  */
    LEFT_ANGLE                = 435, /* LEFT_ANGLE  */
    RIGHT_ANGLE               = 436, /* RIGHT_ANGLE  */
    VERTICAL_BAR              = 437, /* VERTICAL_BAR  */
    CARET                     = 438, /* CARET  */
    AMPERSAND                 = 439, /* AMPERSAND  */
    QUESTION                  = 440  /* QUESTION  */
};
typedef enum yytokentype yytoken_kind_t;
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
