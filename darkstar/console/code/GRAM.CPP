/* yacc -p CMD -D inc\gram.h -o code\gram.cpp code\gram.y */
#ifdef YYTRACE
#define YYDEBUG 1
#else
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#endif
/*
 * Portable way of defining ANSI C prototypes
 */
#ifndef YY_ARGS
#ifdef __STDC__
#define YY_ARGS(x)	x
#else
#define YY_ARGS(x)	()
#endif
#endif

#ifdef YACC_WINDOWS

#include <windows.h>

/*
 * the following is the handle to the current
 * instance of a windows program. The user
 * program calling CMDparse must supply this!
 */

#ifdef STRICT
extern HINSTANCE hInst;	
#else
extern HANDLE hInst;	
#endif

#endif	/* YACC_WINDOWS */

#if YYDEBUG
typedef struct yyNamedType_tag {	/* Tokens */
	char	* name;		/* printable name */
	short	token;		/* token # */
	short	type;		/* token type */
} yyNamedType;
typedef struct yyTypedRules_tag {	/* Typed rule table */
	char	* name;		/* compressed rule string */
	short	type;		/* rule result type */
} yyTypedRules;

#endif

#line 1 "code\gram.y"

#include "console.h"
#include "ast.h"
#include <stdlib.h>
#include "stdio.h"
#include "consoleInternal.h"

#ifndef YYDEBUG
#define YYDEBUG
#endif

#define YYSSIZE 350

int outtext(char *fmt, ...);
extern int serrors;
#define nil 0
#undef YY_ARGS
#define YY_ARGS(x)	x
#line 20 "code\gram.y"

        /* Reserved Word Definitions */
#define rwDEFINE	257
#define rwENDDEF	258
#define rwDECLARE	259
#define rwBREAK	260
#define rwCLASS	261
#define rwELSE	262
#define rwCONTINUE	263
#define rwGLOBAL	264
#define rwIF	265
#define rwNIL	266
#define rwRETURN	267
#define rwWHILE	268
#define rwENDIF	269
#define rwENDWHILE	270
#define rwENDFOR	271
#define rwFOR	272
#define ILLEGAL_TOKEN	273
#line 29 "code\gram.y"

        /* Constants and Identifier Definitions */
#define CHRCONST	274
#define INTCONST	275
#define TTAG	276
#define VAR	277
#define IDENT	278
#define STRATOM	279
#define FLTCONST	280
#line 40 "code\gram.y"

        /* Operator Definitions */
#define opMINUSMINUS	281
#define opPLUSPLUS	282
#define STMT_SEP	283
#define opSHL	284
#define opSHR	285
#define opPLASN	286
#define opMIASN	287
#define opMLASN	288
#define opDVASN	289
#define opMODASN	290
#define opANDASN	291
#define opXORASN	292
#define opORASN	293
#define opSLASN	294
#define opSRASN	295
#define opCAT	296
#define opEQ	297
#define opNE	298
#define opGE	299
#define opLE	300
#define opAND	301
#define opOR	302
#define opCOLONCOLON	303
typedef union {
	char c;
	int i;
	const char *s;
   char *str;
	double f;
	ASTNode *decl;
	StmtNode *stmt;
	ExprNode *expr;
   SlotAssignListNode *slist;
   VarNode *var;
   SlotDecl slot;
   ObjectBlockDecl odcl;
   AssignDecl asn;
} YYSTYPE;
#define opMDASN	304
#define opNDASN	305
#define opNTASN	306
#define UNARY	307
extern int CMDchar, yyerrflag;
extern YYSTYPE CMDlval;
#if YYDEBUG
enum YY_Types { YY_t_NoneDefined, YY_t_i, YY_t_c, YY_t_s, YY_t_str, YY_t_f, YY_t_decl, YY_t_stmt, YY_t_expr, YY_t_odcl, YY_t_slist, YY_t_slot, YY_t_var, YY_t_asn
};
#endif
#if YYDEBUG
yyTypedRules yyRules[] = {
	{ "&00: %24 &00",  0},
	{ "%24: %01",  0},
	{ "%01:",  6},
	{ "%01: %01 %02",  6},
	{ "%02: %05",  6},
	{ "%04:",  7},
	{ "%04: %04 %05",  7},
	{ "%05: %10",  7},
	{ "%05: %11",  7},
	{ "%05: %12",  7},
	{ "%05: %03",  7},
	{ "%05: %14",  7},
	{ "%05: &05 &41",  7},
	{ "%05: &08 &41",  7},
	{ "%05: &12 &41",  7},
	{ "%05: &12 %17 &41",  7},
	{ "%05: %21 &41",  7},
	{ "%05: &23 &23 &42 %18 &43 &41",  7},
	{ "%05: &23 &42 %18 &43 &41",  7},
	{ "%05: &23 &23 &41",  7},
	{ "%05: &21 &32 %17 &41",  7},
	{ "%05: &21 &32 %17 &39 %17 &41",  7},
	{ "%03: &02 &23 &37 &38 &42 %04 &43",  7},
	{ "%03: &02 &23 &37 %22 &38 &42 %04 &43",  7},
	{ "%22: &22",  12},
	{ "%22: %22 &39 &22",  12},
	{ "%14: &04 &23 %17 &42 %16 &43 &41",  7},
	{ "%14: &04 &23 &42 %16 &43 &41",  7},
	{ "%14: &04 &23 %17 &41",  7},
	{ "%14: &04 &23 &41",  7},
	{ "%16: %18",  9},
	{ "%16: %15",  9},
	{ "%16: %18 %15",  9},
	{ "%15: %14",  7},
	{ "%15: %15 %14",  7},
	{ "%13: &42 %04 &43",  7},
	{ "%13: %05",  7},
	{ "%10: &10 &37 %17 &38 %13",  7},
	{ "%10: &10 &37 %17 &38 %13 &07 %13",  7},
	{ "%11: &13 &37 %17 &38 %13",  7},
	{ "%12: &17 &37 %17 &41 %17 &41 %17 &38 %13",  7},
	{ "%21: %09",  7},
	{ "%17: %09",  8},
	{ "%17: &37 %17 &38",  8},
	{ "%17: %17 &44 %17",  8},
	{ "%17: %17 &36 %17",  8},
	{ "%17: %17 &35 %17",  8},
	{ "%17: %17 &34 %17",  8},
	{ "%17: %17 &26 %17",  8},
	{ "%17: %17 &27 %17",  8},
	{ "%17: %17 &28 %17",  8},
	{ "%17: %17 &29 %17",  8},
	{ "%17: &27 %17",  8},
	{ "%17: &28 %17",  8},
	{ "%17: &21",  8},
	{ "%17: %17 &30 %17",  8},
	{ "%17: %17 &31 %17",  8},
	{ "%17: %17 &66 %17",  8},
	{ "%17: %17 &67 %17",  8},
	{ "%17: %17 &64 %17",  8},
	{ "%17: %17 &65 %17",  8},
	{ "%17: %17 &69 %17",  8},
	{ "%17: %17 &51 %17",  8},
	{ "%17: %17 &52 %17",  8},
	{ "%17: %17 &68 %17",  8},
	{ "%17: %17 &63 %17",  8},
	{ "%17: %17 &47 %17",  8},
	{ "%17: &46 %17",  8},
	{ "%17: &45 %17",  8},
	{ "%17: &22",  8},
	{ "%17: &22 &71 %07 &76",  8},
	{ "%17: &24",  8},
	{ "%17: &25",  8},
	{ "%17: &20",  8},
	{ "%17: &23",  8},
	{ "%17: &05",  8},
	{ "%17: %20",  8},
	{ "%20: %17 &33 &23",  11},
	{ "%20: %17 &33 &23 &71 %07 &76",  11},
	{ "%20: %17 &33 &23 &71 %07 &76 &71 %17 &76",  11},
	{ "%23: &49",  13},
	{ "%23: &48",  13},
	{ "%23: &53 %17",  13},
	{ "%23: &54 %17",  13},
	{ "%23: &55 %17",  13},
	{ "%23: &56 %17",  13},
	{ "%23: &57 %17",  13},
	{ "%23: &58 %17",  13},
	{ "%23: &59 %17",  13},
	{ "%23: &60 %17",  13},
	{ "%23: &61 %17",  13},
	{ "%23: &62 %17",  13},
	{ "%09: %08",  8},
	{ "%09: &22 &32 %17",  8},
	{ "%09: &22 &71 %07 &76 &32 %17",  8},
	{ "%09: &22 %23",  8},
	{ "%09: &22 &71 %07 &76 %23",  8},
	{ "%09: %20 %23",  8},
	{ "%09: %20 &32 %17",  8},
	{ "%09: %20 &32 &42 %06 &43",  8},
	{ "%08: &23 &37 &38",  8},
	{ "%08: &23 &37 %06 &38",  8},
	{ "%08: %17 &33 &23 &37 &38",  8},
	{ "%08: %17 &33 &23 &37 %06 &38",  8},
	{ "%06: %17",  8},
	{ "%06: %06 &39 %17",  8},
	{ "%18: %19",  10},
	{ "%18: %18 %19",  10},
	{ "%19: &23 &32 %17 &41",  10},
	{ "%19: &23 &71 %07 &76 &32 %17 &41",  10},
	{ "%19: &23 &71 %07 &76 &71 %17 &76 &32 %17 &41",  10},
	{ "%19: &23 &32 &42 %06 &43 &41",  10},
	{ "%19: &23 &71 %07 &76 &32 &42 %06 &43 &41",  10},
	{ "%07: %17",  8},
	{ "%07: %07 &39 %17",  8},
{ "$accept",  0},{ "error",  0}
};
yyNamedType yyTokenTypes[] = {
	{ "$end",  0,  0},
	{ "error",  256,  0},
	{ "rwDEFINE",  257,  1},
	{ "rwENDDEF",  258,  1},
	{ "rwDECLARE",  259,  1},
	{ "rwBREAK",  260,  1},
	{ "rwCLASS",  261,  1},
	{ "rwELSE",  262,  1},
	{ "rwCONTINUE",  263,  1},
	{ "rwGLOBAL",  264,  1},
	{ "rwIF",  265,  1},
	{ "rwNIL",  266,  1},
	{ "rwRETURN",  267,  1},
	{ "rwWHILE",  268,  1},
	{ "rwENDIF",  269,  1},
	{ "rwENDWHILE",  270,  1},
	{ "rwENDFOR",  271,  1},
	{ "rwFOR",  272,  1},
	{ "ILLEGAL_TOKEN",  273,  0},
	{ "CHRCONST",  274,  2},
	{ "INTCONST",  275,  1},
	{ "TTAG",  276,  3},
	{ "VAR",  277,  3},
	{ "IDENT",  278,  3},
	{ "STRATOM",  279,  4},
	{ "FLTCONST",  280,  5},
	{ "'+'",  43,  1},
	{ "'-'",  45,  1},
	{ "'*'",  42,  1},
	{ "'/'",  47,  1},
	{ "'<'",  60,  1},
	{ "'>'",  62,  1},
	{ "'='",  61,  1},
	{ "'.'",  46,  1},
	{ "'|'",  124,  1},
	{ "'&'",  38,  1},
	{ "'%'",  37,  1},
	{ "'('",  40,  1},
	{ "')'",  41,  1},
	{ "','",  44,  1},
	{ "':'",  58,  1},
	{ "';'",  59,  1},
	{ "'{'",  123,  1},
	{ "'}'",  125,  1},
	{ "'^'",  94,  1},
	{ "'~'",  126,  1},
	{ "'!'",  33,  1},
	{ "'@'",  64,  1},
	{ "opMINUSMINUS",  281,  1},
	{ "opPLUSPLUS",  282,  1},
	{ "STMT_SEP",  283,  1},
	{ "opSHL",  284,  1},
	{ "opSHR",  285,  1},
	{ "opPLASN",  286,  1},
	{ "opMIASN",  287,  1},
	{ "opMLASN",  288,  1},
	{ "opDVASN",  289,  1},
	{ "opMODASN",  290,  1},
	{ "opANDASN",  291,  1},
	{ "opXORASN",  292,  1},
	{ "opORASN",  293,  1},
	{ "opSLASN",  294,  1},
	{ "opSRASN",  295,  1},
	{ "opCAT",  296,  1},
	{ "opEQ",  297,  1},
	{ "opNE",  298,  1},
	{ "opGE",  299,  1},
	{ "opLE",  300,  1},
	{ "opAND",  301,  1},
	{ "opOR",  302,  1},
	{ "opCOLONCOLON",  303,  1},
	{ "'['",  91,  0},
	{ "opMDASN",  304,  0},
	{ "opNDASN",  305,  0},
	{ "opNTASN",  306,  0},
	{ "UNARY",  307,  0},
	{ "']'",  93,  0}

};
#endif
static short yydef[] = {

	 116, 65535,   35,   32, 65531,   78,   34,   74,   34,   31, 
	  30,   17,   16,   50,   47,   46,   45,   44,   43,   42, 
	  41,   40,   39,   38,   48,   53,   36,   29,   28,   27, 
	  26,   25,   24,   23,   22,   21,   20,   19,   18,   15, 
	  14,   13,   12,   11,   10,    9,    8,   51,   33,    4, 
	   3,   54,    6,    5,  107,   52,   49,   37,  107
};
static short yyex[] = {

	   0,    0, 65535,    1,   59,    7, 65535,   80
};
static short yyact[] = {

	65353, 65357, 65354, 65355, 65352, 65362, 65361, 65528, 65365, 65360, 
	65364, 65359, 65358, 65331, 65530, 65532, 65529, 65329, 65330,  280, 
	 279,  278,  277,  276,  275,  272,  268,  267,  265,  263, 
	 260,  259,  257,  126,   45,   42,   40,   33, 65366, 65335, 
	65334, 65376, 65375, 65374, 65373, 65372, 65371, 65370, 65369, 65368, 
	65367,  295,  294,  293,  292,  291,  290,  289,  288,  287, 
	 286,  282,  281,   61, 65377, 65378, 65335, 65334, 65376, 65375, 
	65374, 65373, 65372, 65371, 65370, 65369, 65368, 65367,  295,  294, 
	 293,  292,  291,  290,  289,  288,  287,  286,  282,  281, 
	  91,   61, 65353, 65357, 65354, 65355, 65352, 65332, 65331, 65328, 
	65532, 65527, 65329, 65330,  280,  279,  278,  277,  276,  275, 
	 260,  126,   45,   42,   40,   33, 65398, 65397, 65393, 65395, 
	65394, 65379, 65392, 65391, 65390, 65380, 65399, 65396, 65384, 65383, 
	65381, 65387, 65386, 65389, 65388, 65382, 65385,  302,  301,  300, 
	 299,  298,  297,  296,  285,  284,  124,   94,   64,   62, 
	  60,   47,   46,   45,   43,   42,   38,   37, 65401,   40, 
	65402,   40, 65403,   40, 65404,  278, 65405,  278, 65406,   61, 
	65407, 65408, 65409,  278,  123,   40, 65305,   59, 65353, 65357, 
	65354, 65355, 65303, 65352, 65332, 65331, 65328, 65532, 65527, 65329, 
	65330,  280,  279,  278,  277,  276,  275,  260,  126,   59, 
	  45,   42,   40,   33, 65302,   59, 65301,   59, 65353, 65357, 
	65354, 65355, 65411, 65352, 65332, 65331, 65328, 65532, 65527, 65329, 
	65330,  280,  279,  278,  277,  276,  275,  260,  126,  123, 
	  45,   42,   40,   33, 65407,   40, 65379,   46, 65509,  278, 
	65398, 65397, 65327, 65393, 65395, 65394, 65379, 65392, 65391, 65390, 
	65380, 65399, 65396, 65384, 65383, 65381, 65387, 65386, 65389, 65388, 
	65382, 65385,  302,  301,  300,  299,  298,  297,  296,  285, 
	 284,  124,   94,   64,   62,   60,   47,   46,   45,   43, 
	  42,   41,   38,   37, 65353, 65357, 65354, 65355, 65318, 65416, 
	65352, 65332, 65331, 65328, 65532, 65527, 65329, 65330,  280,  279, 
	 278,  277,  276,  275,  260,  126,  123,   59,   45,   42, 
	  40,   33, 65418,   40, 65353, 65357, 65341, 65354, 65355, 65352, 
	65332, 65331, 65328, 65532, 65527, 65329, 65330,  280,  279,  278, 
	 277,  276,  275,  260,  126,   45,   42,   41,   40,   33, 
	65421,  278, 65308, 65423,  123,   59, 65398, 65397, 65393, 65395, 
	65394, 65379, 65392, 65304, 65391, 65390, 65380, 65399, 65396, 65384, 
	65383, 65381, 65387, 65386, 65389, 65388, 65382, 65385,  302,  301, 
	 300,  299,  298,  297,  296,  285,  284,  124,   94,   64, 
	  62,   60,   59,   47,   46,   45,   43,   42,   38,   37, 
	65425, 65487,   93,   44, 65426, 65427,   91,   40, 65398, 65393, 
	65395, 65394, 65379, 65392, 65384, 65383,  285,  284,   47,   46, 
	  45,   43,   42,   37, 65398, 65397, 65393, 65395, 65394, 65379, 
	65392, 65391, 65390, 65380, 65399, 65396, 65384, 65383, 65381, 65387, 
	65386, 65389, 65388,  300,  299,  298,  297,  296,  285,  284, 
	 124,   94,   64,   62,   60,   47,   46,   45,   43,   42, 
	  38,   37, 65398, 65393, 65395, 65394, 65379, 65392,   47,   46, 
	  45,   43,   42,   37, 65398, 65397, 65393, 65395, 65394, 65379, 
	65392, 65391, 65390, 65380, 65399, 65396, 65384, 65383, 65381, 65387, 
	65386, 65389, 65388, 65382,  301,  300,  299,  298,  297,  296, 
	 285,  284,  124,   94,   64,   62,   60,   47,   46,   45, 
	  43,   42,   38,   37, 65398, 65393, 65395, 65394, 65379, 65392, 
	65391, 65390, 65380, 65384, 65383, 65381, 65389, 65388,  300,  299, 
	 296,  285,  284,   64,   62,   60,   47,   46,   45,   43, 
	  42,   37, 65398, 65393, 65395, 65394, 65379, 65392, 65380, 65384, 
	65383, 65381,  296,  285,  284,   64,   47,   46,   45,   43, 
	  42,   37, 65398, 65393, 65379, 65392,   47,   46,   42,   37, 
	65398, 65397, 65393, 65395, 65394, 65379, 65392, 65391, 65390, 65380, 
	65399, 65384, 65383, 65381, 65387, 65386, 65389, 65388,  300,  299, 
	 298,  297,  296,  285,  284,   94,   64,   62,   60,   47, 
	  46,   45,   43,   42,   38,   37, 65398, 65393, 65395, 65394, 
	65379, 65392, 65391, 65390, 65380, 65384, 65383, 65381, 65387, 65386, 
	65389, 65388,  300,  299,  298,  297,  296,  285,  284,   64, 
	  62,   60,   47,   46,   45,   43,   42,   37, 65398, 65397, 
	65393, 65395, 65394, 65379, 65392, 65391, 65390, 65380, 65384, 65383, 
	65381, 65387, 65386, 65389, 65388,  300,  299,  298,  297,  296, 
	 285,  284,   64,   62,   60,   47,   46,   45,   43,   42, 
	  38,   37, 65398, 65397, 65393, 65395, 65394, 65379, 65392, 65428, 
	65391, 65390, 65380, 65399, 65396, 65384, 65383, 65381, 65387, 65386, 
	65389, 65388, 65382, 65385,  302,  301,  300,  299,  298,  297, 
	 296,  285,  284,  124,   94,   64,   62,   60,   59,   47, 
	  46,   45,   43,   42,   38,   37, 65398, 65397, 65429, 65393, 
	65395, 65394, 65379, 65392, 65391, 65390, 65380, 65399, 65396, 65384, 
	65383, 65381, 65387, 65386, 65389, 65388, 65382, 65385,  302,  301, 
	 300,  299,  298,  297,  296,  285,  284,  124,   94,   64, 
	  62,   60,   47,   46,   45,   43,   42,   41,   38,   37, 
	65398, 65397, 65430, 65393, 65395, 65394, 65379, 65392, 65391, 65390, 
	65380, 65399, 65396, 65384, 65383, 65381, 65387, 65386, 65389, 65388, 
	65382, 65385,  302,  301,  300,  299,  298,  297,  296,  285, 
	 284,  124,   94,   64,   62,   60,   47,   46,   45,   43, 
	  42,   41,   38,   37, 65361, 65421,  278,  259, 65398, 65397, 
	65393, 65395, 65394, 65379, 65392, 65317, 65391, 65390, 65380, 65399, 
	65432, 65396, 65384, 65383, 65381, 65387, 65386, 65389, 65388, 65382, 
	65385,  302,  301,  300,  299,  298,  297,  296,  285,  284, 
	 124,  123,   94,   64,   62,   60,   59,   47,   46,   45, 
	  43,   42,   38,   37, 65434, 65313,  277,   41, 65398, 65397, 
	65393, 65395, 65435, 65394, 65379, 65392, 65309, 65391, 65390, 65380, 
	65399, 65396, 65384, 65383, 65381, 65387, 65386, 65389, 65388, 65382, 
	65385,  302,  301,  300,  299,  298,  297,  296,  285,  284, 
	 124,   94,   64,   62,   60,   59,   47,   46,   45,   44, 
	  43,   42,   38,   37, 65342, 65436,   44,   41, 65438, 65437, 
	  91,   61, 65439, 65421,  278,  125, 65436, 65340,  125,   44, 
	65441, 65335, 65334, 65376, 65375, 65374, 65373, 65372, 65371, 65370, 
	65369, 65368, 65367,  295,  294,  293,  292,  291,  290,  289, 
	 288,  287,  286,  282,  281,   61, 65353, 65357, 65343, 65354, 
	65355, 65352, 65332, 65331, 65328, 65532, 65527, 65329, 65330,  280, 
	 279,  278,  277,  276,  275,  260,  126,   45,   42,   41, 
	  40,   33, 65353, 65357, 65354, 65355, 65299, 65352, 65362, 65361, 
	65528, 65365, 65360, 65364, 65359, 65358, 65331, 65530, 65532, 65529, 
	65329, 65330,  280,  279,  278,  277,  276,  275,  272,  268, 
	 267,  265,  263,  260,  259,  257,  126,  123,   45,   42, 
	  40,   33, 65361,  259, 65445,  125, 65448, 65447,   44,   41, 
	65481,  123, 65353, 65357, 65354, 65355, 65451, 65352, 65332, 65331, 
	65328, 65532, 65527, 65329, 65330,  280,  279,  278,  277,  276, 
	 275,  260,  126,  123,   45,   42,   40,   33, 65307,   59, 
	65453, 65421,  278,  125, 65344, 65436,   44,   41, 65425, 65478, 
	  93,   44, 65398, 65397, 65393, 65395, 65394, 65379, 65392, 65454, 
	65391, 65390, 65380, 65399, 65396, 65384, 65383, 65381, 65387, 65386, 
	65389, 65388, 65382, 65385,  302,  301,  300,  299,  298,  297, 
	 296,  285,  284,  124,   94,   64,   62,   60,   59,   47, 
	  46,   45,   43,   42,   38,   37, 65456,  262, 65316,   59, 
	65457,  125, 65314,  277, 65477,  123, 65398, 65397, 65393, 65395, 
	65394, 65379, 65392, 65310, 65391, 65390, 65380, 65399, 65396, 65384, 
	65383, 65381, 65387, 65386, 65389, 65388, 65382, 65385,  302,  301, 
	 300,  299,  298,  297,  296,  285,  284,  124,   94,   64, 
	  62,   60,   59,   47,   46,   45,   43,   42,   38,   37, 
	65425, 65459,   93,   44, 65398, 65397, 65393, 65395, 65394, 65379, 
	65392, 65347, 65391, 65390, 65380, 65399, 65396, 65384, 65383, 65381, 
	65387, 65386, 65389, 65388, 65382, 65385,  302,  301,  300,  299, 
	 298,  297,  296,  285,  284,  124,   94,   64,   62,   60, 
	  59,   47,   46,   45,   43,   42,   38,   37, 65306,   59, 
	65461,   91, 65353, 65357, 65354, 65355, 65321, 65352, 65362, 65361, 
	65528, 65365, 65360, 65364, 65359, 65358, 65331, 65530, 65532, 65529, 
	65329, 65330,  280,  279,  278,  277,  276,  275,  272,  268, 
	 267,  265,  263,  260,  259,  257,  126,  125,   45,   42, 
	  40,   33, 65315,   59, 65353, 65357, 65354, 65355, 65311, 65352, 
	65362, 65361, 65528, 65365, 65360, 65364, 65359, 65358, 65331, 65530, 
	65532, 65529, 65329, 65330,  280,  279,  278,  277,  276,  275, 
	 272,  268,  267,  265,  263,  260,  259,  257,  126,  125, 
	  45,   42,   40,   33, 65465, 65464,   91,   61, 65436, 65466, 
	 125,   44, 65398, 65397, 65468, 65393, 65395, 65394, 65379, 65392, 
	65391, 65390, 65380, 65399, 65396, 65384, 65383, 65381, 65387, 65386, 
	65389, 65388, 65382, 65385,  302,  301,  300,  299,  298,  297, 
	 296,  285,  284,  124,   94,   64,   62,   60,   47,   46, 
	  45,   43,   42,   41,   38,   37, 65353, 65357, 65354, 65355, 
	65312, 65352, 65362, 65361, 65528, 65365, 65360, 65364, 65359, 65358, 
	65331, 65530, 65532, 65529, 65329, 65330,  280,  279,  278,  277, 
	 276,  275,  272,  268,  267,  265,  263,  260,  259,  257, 
	 126,  125,   45,   42,   40,   33, 65353, 65357, 65354, 65355, 
	65470, 65352, 65332, 65331, 65328, 65532, 65527, 65329, 65330,  280, 
	 279,  278,  277,  276,  275,  260,  126,  123,   45,   42, 
	  40,   33, 65350,   59, 65398, 65397, 65393, 65395, 65394, 65379, 
	65392, 65391, 65390, 65380, 65333, 65399, 65396, 65384, 65383, 65381, 
	65387, 65386, 65389, 65388, 65382, 65385,  302,  301,  300,  299, 
	 298,  297,  296,  285,  284,  124,   94,   93,   64,   62, 
	  60,   47,   46,   45,   43,   42,   38,   37, 65398, 65397, 
	65393, 65395, 65394, 65379, 65392, 65391, 65390, 65380, 65472, 65399, 
	65396, 65384, 65383, 65381, 65387, 65386, 65389, 65388, 65382, 65385, 
	 302,  301,  300,  299,  298,  297,  296,  285,  284,  124, 
	  94,   93,   64,   62,   60,   47,   46,   45,   43,   42, 
	  38,   37, 65398, 65397, 65393, 65395, 65394, 65379, 65392, 65348, 
	65391, 65390, 65380, 65399, 65396, 65384, 65383, 65381, 65387, 65386, 
	65389, 65388, 65382, 65385,  302,  301,  300,  299,  298,  297, 
	 296,  285,  284,  124,   94,   64,   62,   60,   59,   47, 
	  46,   45,   43,   42,   38,   37, 65474,   61, 65436, 65475, 
	 125,   44, 65351,   59, 65398, 65397, 65393, 65395, 65394, 65379, 
	65392, 65349, 65391, 65390, 65380, 65399, 65396, 65384, 65383, 65381, 
	65387, 65386, 65389, 65388, 65382, 65385,  302,  301,  300,  299, 
	 298,  297,  296,  285,  284,  124,   94,   64,   62,   60, 
	  59,   47,   46,   45,   43,   42,   38,   37,   -1
};
static short yypact[] = {

	  19,   38,   51,   78,  158,  169,  173,  207,  235,  237, 
	 237,  237,  237,  137,  137,  137,  137,  137,  137,  137, 
	 137,  137,  137,  137,  137,  137,  396,  406,  406,  433, 
	 458,  458,  484,  518,  518,  542,  542,  542,  542,  237, 
	 237,  556,  556,  578,  612,  237,  645,  137,  923, 1003, 
	 796,  137, 1097, 1003, 1106,  137,  137, 1201, 1244, 1566, 
	1543,  104, 1540, 1537, 1514,  104, 1470,  982, 1426, 1403, 
	1389,  104, 1356, 1314,  104, 1290, 1286, 1264, 1243,  982, 
	1222,  104, 1199, 1176,  104, 1152, 1128, 1105, 1103, 1101, 
	1099, 1074, 1050, 1046,  104, 1042, 1039, 1025,  104,  104, 
	 104, 1011, 1008,  796, 1005,  982,  982,  104,  104,  949, 
	 104,  908,  341,  904,  900,  896,  871,  846,  821,  796, 
	 772,  728,  684,  392,  104,  368,  344,  341,  327,  104, 
	 313,  298,  104,  104,  104,  262,  104,  104,  104,  104, 
	 104,  104,  104,  104,  104,  104,  104,  104,  104,  104, 
	 104,  104,  104,  104,  104,  104,  239,  104,  104,  104, 
	 104,  104,  104,  104,  104,  104,  104,  104,  104,  221, 
	 205,  191,  177,  167,  165,  163,  161,  159,  104,  137, 
	 104,  104,  104,  104
};
static short yygo[] = {

	65535, 65296, 65292, 65458, 65463, 65455,   58,   54, 65298, 65300, 
	65300, 65300, 65322,   80,   77,   72,    0, 65473, 65460, 65442, 
	65424, 65420,  124,  109,   84,   65, 65450, 65443, 65412,  108, 
	  98, 65336, 65531, 65531, 65531, 65531, 65531, 65531, 65531, 65531, 
	65326,  106,  105,   80,   79,   77,   72,   67,    0, 65295, 
	65294, 65293, 65325, 65323, 65483, 65324,  105,   79,   67, 65320, 
	65319, 65320, 65319, 65319, 65291,  119,  103,   53,   50,   49, 
	65482, 65486,   50, 65446, 65431,  103, 65476, 65488, 65471, 65469, 
	65467, 65462, 65488, 65479, 65452, 65510, 65480, 65449, 65444, 65510, 
	65488, 65484, 65488, 65488, 65419, 65417, 65415, 65414, 65413, 65489, 
	65490, 65491, 65492, 65493, 65494, 65495, 65496, 65497, 65498, 65499, 
	65500, 65501, 65502, 65503, 65504, 65505, 65506, 65507, 65508, 65510, 
	65511, 65512, 65513, 65514, 65515, 65516, 65517, 65518, 65519, 65520, 
	65521, 65522, 65410, 65400, 65523, 65524, 65525, 65526, 65356,  183, 
	 182,  181,  180,  178,  171,  169,  168,  167,  166,  165, 
	 164,  163,  162,  161,  160,  159,  158,  157,  155,  154, 
	 153,  152,  151,  150,  149,  148,  147,  146,  145,  144, 
	 143,  142,  141,  140,  139,  138,  137,  136,  134,  133, 
	 132,  131,  129,  128,  124,  110,  109,  108,  107,  100, 
	  99,   98,   97,   94,   84,   81,   74,   71,   70,   65, 
	  61, 65440, 65422, 65485,  127,  112, 65346, 65346, 65346, 65345, 
	 113,   95,   50, 65533, 65363, 65433, 65337, 65338, 65339,   48, 
	   3, 65534,   -1
};
static short yypgo[] = {

	   0,    0,    0,   74,   74,   74,   49,  214,  138,  138, 
	 138,  138,  138,  138,  138,  138,  138,  138,  138,  138, 
	 138,  138,  138,  138,  138,  138,  138,  138,  138,  138, 
	 138,  138,  138,  138,  138,  138,  213,  213,  218,  218, 
	 218,  218,  218,  218,  218,  218,  218,  218,   40,   40, 
	  40,   21,   21,   28,   28,  209,  209,  209,  209,  209, 
	 203,  203,   31,   31,   31,   31,   40,   40,   40,   40, 
	  40,  218,  218,  213,  138,  138,  138,  138,  138,  138, 
	 138,   51,   50,   49,   55,   55,   71,   71,   64,   64, 
	  64,   64,  215,  215,    2,    2,   12,   12,   12,   12, 
	  12,   12,   12,   12,   12,   12,    5,    5,    1,    0, 
	   0,   12,   12,   12,   12,   12,  221,    0
};
static short yyrlen[] = {

	   0,    0,    0,    1,    1,    2,    5,    1,    3,    3, 
	   3,    3,    3,    3,    3,    3,    2,    2,    3,    3, 
	   3,    3,    3,    3,    3,    3,    3,    3,    3,    3, 
	   2,    2,    1,    4,    1,    1,    3,    6,    2,    2, 
	   2,    2,    2,    2,    2,    2,    2,    2,    3,    6, 
	   3,    1,    3,    1,    3,    9,    6,   10,    7,    4, 
	   2,    1,    6,    5,    4,    3,    5,    2,    5,    2, 
	   1,    1,    1,    9,    1,    1,    1,    1,    1,    3, 
	   1,    9,    5,    7,    1,    3,    2,    1,    3,    4, 
	   6,    7,    3,    1,    8,    7,    6,    4,    3,    5, 
	   6,    2,    3,    2,    2,    2,    2,    0,    1,    0, 
	   2,    1,    1,    1,    1,    1,    1,    2
};
#define YYS0	238
#define YYDELTA	129
#define YYNPACT	184
#define YYNDEF	59

#define YYr115	0
#define YYr116	1
#define YYr117	2
#define YYr30	3
#define YYr31	4
#define YYr32	5
#define YYr37	6
#define YYr41	7
#define YYr44	8
#define YYr45	9
#define YYr46	10
#define YYr47	11
#define YYr48	12
#define YYr49	13
#define YYr50	14
#define YYr51	15
#define YYr52	16
#define YYr53	17
#define YYr55	18
#define YYr56	19
#define YYr57	20
#define YYr58	21
#define YYr59	22
#define YYr60	23
#define YYr61	24
#define YYr62	25
#define YYr63	26
#define YYr64	27
#define YYr65	28
#define YYr66	29
#define YYr67	30
#define YYr68	31
#define YYr69	32
#define YYr70	33
#define YYr74	34
#define YYr76	35
#define YYr77	36
#define YYr78	37
#define YYr82	38
#define YYr83	39
#define YYr84	40
#define YYr85	41
#define YYr86	42
#define YYr87	43
#define YYr88	44
#define YYr89	45
#define YYr90	46
#define YYr91	47
#define YYr93	48
#define YYr94	49
#define YYr98	50
#define YYr104	51
#define YYr105	52
#define YYr113	53
#define YYr114	54
#define YYr112	55
#define YYr111	56
#define YYr110	57
#define YYr109	58
#define YYr108	59
#define YYr107	60
#define YYr106	61
#define YYr103	62
#define YYr102	63
#define YYr101	64
#define YYr100	65
#define YYr99	66
#define YYr97	67
#define YYr96	68
#define YYr95	69
#define YYr92	70
#define YYr81	71
#define YYr80	72
#define YYr79	73
#define YYr75	74
#define YYr73	75
#define YYr72	76
#define YYr71	77
#define YYr54	78
#define YYr43	79
#define YYr42	80
#define YYr40	81
#define YYr39	82
#define YYr38	83
#define YYr36	84
#define YYr35	85
#define YYr34	86
#define YYr33	87
#define YYr29	88
#define YYr28	89
#define YYr27	90
#define YYr26	91
#define YYr25	92
#define YYr24	93
#define YYr23	94
#define YYr22	95
#define YYr21	96
#define YYr20	97
#define YYr19	98
#define YYr18	99
#define YYr17	100
#define YYr16	101
#define YYr15	102
#define YYr14	103
#define YYr13	104
#define YYr12	105
#define YYr6	106
#define YYr5	107
#define YYr4	108
#define YYr2	109
#define YYrACCEPT	YYr115
#define YYrERROR	YYr116
#define YYrLR2	YYr117
#if YYDEBUG
char * yysvar[] = {
	"$accept",
	"program",
	"decl",
	"fn_decl_stmt",
	"statement_list",
	"stmt",
	"expr_list",
	"aidx_expr",
	"funcall_expr",
	"stmt_expr",
	"if_stmt",
	"while_stmt",
	"for_stmt",
	"stmt_block",
	"object_decl",
	"object_decl_list",
	"declare_block",
	"expr",
	"slot_assign_list",
	"slot_assign",
	"slot_acc",
	"expression_stmt",
	"var_list",
	"assign_op_struct",
	"start",
	0
};
short yyrmap[] = {

	 115,  116,  117,   30,   31,   32,   37,   41,   44,   45, 
	  46,   47,   48,   49,   50,   51,   52,   53,   55,   56, 
	  57,   58,   59,   60,   61,   62,   63,   64,   65,   66, 
	  67,   68,   69,   70,   74,   76,   77,   78,   82,   83, 
	  84,   85,   86,   87,   88,   89,   90,   91,   93,   94, 
	  98,  104,  105,  113,  114,  112,  111,  110,  109,  108, 
	 107,  106,  103,  102,  101,  100,   99,   97,   96,   95, 
	  92,   81,   80,   79,   75,   73,   72,   71,   54,   43, 
	  42,   40,   39,   38,   36,   35,   34,   33,   29,   28, 
	  27,   26,   25,   24,   23,   22,   21,   20,   19,   18, 
	  17,   16,   15,   14,   13,   12,    6,    5,    4,    2, 
	   3,    7,    8,    9,   10,   11,    1,    0
};
short yysmap[] = {

	   1,    2,    4,    8,   15,   21,   22,   26,   52,   54, 
	  56,   57,   58,   96,   97,   98,   99,  100,  101,  102, 
	 103,  104,  105,  106,  107,  109,  110,  111,  112,  113, 
	 114,  115,  116,  117,  118,  119,  120,  121,  122,  123, 
	 124,  125,  126,  127,  128,  129,  130,  142,  151,  158, 
	 159,  176,  186,  188,  193,  195,  201,  203,  210,  242, 
	 241,  240,  238,  237,  233,  232,  231,  229,  228,  227, 
	 226,  225,  223,  218,  217,  214,  213,  211,  208,  206, 
	 205,  204,  200,  198,  197,  196,  194,  192,  191,  190, 
	 189,  182,  181,  179,  178,  174,  173,  171,  170,  168, 
	 166,  165,  164,  162,  160,  156,  155,  154,  153,  152, 
	 150,  149,  147,  145,  143,  140,  139,  138,  137,  136, 
	 134,  133,  132,  108,   95,   91,   89,   88,   87,   86, 
	  85,   84,   83,   82,   81,   80,   79,   78,   77,   76, 
	  75,   74,   73,   72,   71,   70,   69,   68,   67,   66, 
	  65,   64,   63,   62,   61,   60,   59,   50,   49,   45, 
	  44,   43,   42,   41,   40,   39,   38,   37,   36,   34, 
	  25,   24,   23,   20,   19,   18,   17,   16,   14,   13, 
	  12,   11,   10,    9,  243,  234,  244,  239,  215,  172, 
	 144,  202,  180,  169,  141,  175,   35,  177,   48,    3, 
	  46,   47,  235,   51,    5,    6,    7,   53,  131,   55, 
	 236,  183,  221,  184,  219,  187,  157,  135,  161,  207, 
	 222,  209,  163,  230,  224,  212,  167,  146,  199,  216, 
	  90,  148,   92,   93,   94,  220,  185,   32,    0,   33, 
	  31,   30,   29,   28,   27
};
int yyntoken = 77;
int yynvar = 25;
int yynstate = 245;
int yynrule = 118;
#endif

#if YYDEBUG
/*
 * Package up YACC context for tracing
 */
typedef struct yyTraceItems_tag {
	int	state, lookahead, errflag, done;
	int	rule, npop;
	short	* states;
	int	nstates;
	YYSTYPE * values;
	int	nvalues;
	short	* types;
} yyTraceItems;
#endif

#line 2 "c:/etc/yyparse.c"

/*
 * Copyright 1985, 1990 by Mortice Kern Systems Inc.  All rights reserved.
 * 
 * Automaton to interpret LALR(1) tables.
 *
 * Macros:
 *	yyclearin - clear the lookahead token.
 *	yyerrok - forgive a pending error
 *	YYERROR - simulate an error
 *	YYACCEPT - halt and return 0
 *	YYABORT - halt and return 1
 *	YYRETURN(value) - halt and return value.  You should use this
 *		instead of return(value).
 *	YYREAD - ensure CMDchar contains a lookahead token by reading
 *		one if it does not.  See also YYSYNC.
 *	YYRECOVERING - 1 if syntax error detected and not recovered
 *		yet; otherwise, 0.
 *
 * Preprocessor flags:
 *	YYDEBUG - includes debug code if 1.  The parser will print
 *		 a travelogue of the parse if this is defined as 1
 *		 and CMDdebug is non-zero.
 *		yacc -t sets YYDEBUG to 1, but not CMDdebug.
 *	YYTRACE - turn on YYDEBUG, and undefine default trace functions
 *		so that the interactive functions in 'ytrack.c' will
 *		be used.
 *	YYSSIZE - size of state and value stacks (default 150).
 *	YYSTATIC - By default, the state stack is an automatic array.
 *		If this is defined, the stack will be static.
 *		In either case, the value stack is static.
 *	YYALLOC - Dynamically allocate both the state and value stacks
 *		by calling malloc() and free().
 *	YYDYNAMIC - Dynamically allocate (and reallocate, if necessary)
 *		both the state and value stacks by calling malloc(),
 *		realloc(), and free().
 *	YYSYNC - if defined, yacc guarantees to fetch a lookahead token
 *		before any action, even if it doesnt need it for a decision.
 *		If YYSYNC is defined, YYREAD will never be necessary unless
 *		the user explicitly sets CMDchar = -1
 *
 * Copyright (c) 1983, by the University of Waterloo
 */
/*
 * Prototypes
 */

extern int CMDlex YY_ARGS((void));
extern void CMDerror YY_ARGS((char *, ...));

#if YYDEBUG

#include <stdlib.h>		/* common prototypes */
#include <string.h>

extern char *	yyValue YY_ARGS((YYSTYPE, int));	/* print CMDlval */
extern void yyShowState YY_ARGS((yyTraceItems *));
extern void yyShowReduce YY_ARGS((yyTraceItems *));
extern void yyShowGoto YY_ARGS((yyTraceItems *));
extern void yyShowShift YY_ARGS((yyTraceItems *));
extern void yyShowErrRecovery YY_ARGS((yyTraceItems *));
extern void yyShowErrDiscard YY_ARGS((yyTraceItems *));

extern void yyShowRead YY_ARGS((int));
#endif

/*
 * If YYDEBUG defined and CMDdebug set,
 * tracing functions will be called at appropriate times in CMDparse()
 * Pass state of YACC parse, as filled into yyTraceItems yyx
 * If yyx.done is set by the tracing function, CMDparse() will terminate
 * with a return value of -1
 */
#define YY_TRACE(fn) { \
	yyx.state = yystate; yyx.lookahead = CMDchar; yyx.errflag =yyerrflag; \
	yyx.states = yys+1; yyx.nstates = yyps-yys; \
	yyx.values = yyv+1; yyx.nvalues = yypv-yyv; \
	yyx.types = yytypev+1; yyx.done = 0; \
	yyx.rule = yyi; yyx.npop = yyj; \
	fn(&yyx); \
	if (yyx.done) YYRETURN(-1); }

#ifndef I18N
#define m_textmsg(id, str, cls)	(str)
#else /*I18N*/
#include <m_nls.h>
#endif/*I18N*/

#ifndef YYSSIZE
# define YYSSIZE	150
#endif

#ifdef YYDYNAMIC
#define YYALLOC
char *getenv();
int atoi();
int yysinc = -1; /* stack size increment, <0 = double, 0 = none, >0 = fixed */
#endif

#ifdef YYALLOC
int yyssize = YYSSIZE;
#endif

#define YYERROR		goto yyerrlabel
#define yyerrok		yyerrflag = 0
#if YYDEBUG
#define yyclearin	{ if (CMDdebug) yyShowRead(-1); CMDchar = -1; }
#else
#define yyclearin	CMDchar = -1
#endif
#define YYACCEPT	YYRETURN(0)
#define YYABORT		YYRETURN(1)
#define YYRECOVERING()	(yyerrflag != 0)
#ifdef YYALLOC
#define YYRETURN(val)	{ retval = (val); goto yyReturn; }
#else
#define YYRETURN(val)	return(val);
#endif
#if YYDEBUG
/* The if..else makes this macro behave exactly like a statement */
# define YYREAD	if (CMDchar < 0) {					\
			if ((CMDchar = CMDlex()) < 0)	{		\
				if (CMDchar == -2) YYABORT; \
				CMDchar = 0;				\
			}	/* endif */			\
			if (CMDdebug)					\
				yyShowRead(CMDchar);			\
		} else
#else
# define YYREAD	if (CMDchar < 0) {					\
			if ((CMDchar = CMDlex()) < 0) {			\
				if (CMDchar == -2) YYABORT; \
				CMDchar = 0;				\
			}	/* endif */			\
		} else
#endif

#define YYERRCODE	256		/* value of `error' */
#define YYTOKEN_BASE	256
#define	YYQYYP	yyq[yyq-yyp]

/*
 * Simulate bitwise negation as if was done on a two's complement machine.
 * This makes the generated code portable to machines with different
 * representations of integers (ie. signed magnitude).
 */
#define	yyneg(s)	(-((s)+1))

YYSTYPE	yyval;				/* $ */
YYSTYPE	*yypvt;				/* $n */
YYSTYPE	CMDlval;				/* CMDlex() sets this */

int	CMDchar,				/* current token */
	yyerrflag,			/* error flag */
	yynerrs;			/* error count */

#if YYDEBUG
int CMDdebug = 0;		/* debug if this flag is set */
extern char	*yysvar[];	/* table of non-terminals (aka 'variables') */
extern yyNamedType yyTokenTypes[];	/* table of terminals & their types */
extern short	yyrmap[], yysmap[];	/* map internal rule/states */
extern int	yynstate, yynvar, yyntoken, yynrule;

extern int	yyGetType YY_ARGS((int));	/* token type */
extern char	*yyptok YY_ARGS((int));	/* printable token string */
extern int	yyExpandName YY_ARGS((int, int, char *, int));
				  /* expand yyRules[] or yyStates[] */
static char *	yygetState YY_ARGS((int));

#define yyassert(condition, msg, arg) \
	if (!(condition)) { \
		printf(m_textmsg(2824, "\nyacc bug: ", "E")); \
		printf(msg, arg); \
		YYABORT; }
#else /* !YYDEBUG */
#define yyassert(condition, msg, arg)
#endif



#ifdef YACC_WINDOWS

/*
 * the following is the CMDparse() function that will be
 * callable by a windows type program. It in turn will
 * load all needed resources, obtain pointers to these
 * resources, and call a statically defined function
 * win_yyparse(), which is the original CMDparse() fn
 * When win_yyparse() is complete, it will return a
 * value to the new CMDparse(), where it will be stored
 * away temporarily, all resources will be freed, and
 * that return value will be given back to the caller
 * CMDparse(), as expected.
 */

static int win_yyparse();			/* prototype */

CMDparse() 
{
	int wReturnValue;
	HANDLE hRes_table;		/* handle of resource after loading */
	short far *old_yydef;		/* the following are used for saving */
	short far *old_yyex;		/* the current pointers */
	short far *old_yyact;
	short far *old_yypact;
	short far *old_yygo;
	short far *old_yypgo;
	short far *old_yyrlen;

	/*
	 * the following code will load the required
	 * resources for a Windows based parser.
	 */

	hRes_table = LoadResource (hInst, 
		FindResource (hInst, "UD_RES_yyYACC", "yyYACCTBL"));
	
	/*
	 * return an error code if any
	 * of the resources did not load
	 */

	if (hRes_table == NULL)
		return (1);
	
	/*
	 * the following code will lock the resources
	 * into fixed memory locations for the parser
	 * (also, save the current pointer values first)
	 */

	old_yydef = yydef;
	old_yyex = yyex;
	old_yyact = yyact;
	old_yypact = yypact;
	old_yygo = yygo;
	old_yypgo = yypgo;
	old_yyrlen = yyrlen;

	yydef = (short far *)LockResource (hRes_table);
	yyex = (short far *)(yydef + Sizeof_yydef);
	yyact = (short far *)(yyex + Sizeof_yyex);
	yypact = (short far *)(yyact + Sizeof_yyact);
	yygo = (short far *)(yypact + Sizeof_yypact);
	yypgo = (short far *)(yygo + Sizeof_yygo);
	yyrlen = (short far *)(yypgo + Sizeof_yypgo);

	/*
	 * call the official CMDparse() function
	 */

	wReturnValue = win_yyparse();

	/*
	 * unlock the resources
	 */

	UnlockResource (hRes_table);

	/*
	 * and now free the resource
	 */

	FreeResource (hRes_table);

	/*
	 * restore previous pointer values
	 */

	yydef = old_yydef;
	yyex = old_yyex;
	yyact = old_yyact;
	yypact = old_yypact;
	yygo = old_yygo;
	yypgo = old_yypgo;
	yyrlen = old_yyrlen;

	return (wReturnValue);
}	/* end CMDparse */

static int win_yyparse() 

#else /* YACC_WINDOWS */

/*
 * we are not compiling a windows resource
 * based parser, so call CMDparse() the old
 * standard way.
 */

CMDparse() 

#endif /* YACC_WINDOWS */

{
#ifdef YACC_WINDOWS
	register short far	*yyp;	/* for table lookup */
	register short far	*yyq;
#else
	register short		*yyp;	/* for table lookup */
	register short		*yyq;
#endif	/* YACC_WINDOWS */
	register short		yyi;
	register short		*yyps;		/* top of state stack */
	register short		yystate;	/* current state */
	register YYSTYPE	*yypv;		/* top of value stack */
	register int		yyj;
#if YYDEBUG
	yyTraceItems	yyx;			/* trace block */
	short	* yytp;
	int	yyruletype = 0;
#endif
#ifdef YYSTATIC
	static short	yys[YYSSIZE + 1];
	static YYSTYPE	yyv[YYSSIZE + 1];
#if YYDEBUG
	static short	yytypev[YYSSIZE+1];	/* type assignments */
#endif
#else /* ! YYSTATIC */
#ifdef YYALLOC
	YYSTYPE *yyv;
	short	*yys;
#if YYDEBUG
	short	*yytypev;
#endif
	YYSTYPE save_yylval;
	YYSTYPE save_yyval;
	YYSTYPE *save_yypvt;
	int save_yychar, save_yyerrflag, save_yynerrs;
	int retval; 			/* return value holder */
#else
	short		yys[YYSSIZE + 1];
	static YYSTYPE	yyv[YYSSIZE + 1];	/* historically static */
#if YYDEBUG
	short	yytypev[YYSSIZE+1];		/* mirror type table */
#endif
#endif /* ! YYALLOC */
#endif /* ! YYSTATIC */
#ifdef YYDYNAMIC
	char *envp;
#endif


#ifdef YYDYNAMIC
	if ((envp = getenv("YYSTACKSIZE")) != (char *)0) {
		yyssize = atoi(envp);
		if (yyssize <= 0)
			yyssize = YYSSIZE;
	}
	if ((envp = getenv("YYSTACKINC")) != (char *)0)
		yysinc = atoi(envp);
#endif
#ifdef YYALLOC
	yys = (short *) malloc((yyssize + 1) * sizeof(short));
	yyv = (YYSTYPE *) malloc((yyssize + 1) * sizeof(YYSTYPE));
#if YYDEBUG
	yytypev = (short *) malloc((yyssize + 1) * sizeof(short));
#endif
	if (yys == (short *)0 || yyv == (YYSTYPE *)0
#if YYDEBUG
		|| yytypev == (short *) 0
#endif
	) {
		CMDerror(m_textmsg(4967, "Not enough space for parser stacks",
				  "E"));
		return 1;
	}
	save_yylval = CMDlval;
	save_yyval = yyval;
	save_yypvt = yypvt;
	save_yychar = CMDchar;
	save_yyerrflag = yyerrflag;
	save_yynerrs = yynerrs;
#endif

	yynerrs = 0;
	yyerrflag = 0;
	yyclearin;
	yyps = yys;
	yypv = yyv;
	*yyps = yystate = YYS0;		/* start state */
#if YYDEBUG
	yytp = yytypev;
	yyi = yyj = 0;			/* silence compiler warnings */
#endif

yyStack:
	yyassert((unsigned)yystate < yynstate, m_textmsg(587, "state %d\n", ""), yystate);
#ifdef YYDYNAMIC
	if (++yyps > &yys[yyssize]) {
		int yynewsize;
		int yysindex = yyps - yys;
		int yyvindex = yypv - yyv;
#if YYDEBUG
		int yytindex = yytp - yytypev;
#endif
		if (yysinc == 0) {		/* no increment */
			CMDerror(m_textmsg(4968, "Parser stack overflow", "E"));
			YYABORT;
		} else if (yysinc < 0)		/* binary-exponential */
			yynewsize = yyssize * 2;
		else				/* fixed increment */
			yynewsize = yyssize + yysinc;
		if (yynewsize < yyssize) {
			CMDerror(m_textmsg(4967,
					  "Not enough space for parser stacks",
					  "E"));
			YYABORT;
		}
		yyssize = yynewsize;
		yys = (short *) realloc(yys, (yyssize + 1) * sizeof(short));
		yyps = yys + yysindex;
		yyv = (YYSTYPE *) realloc(yyv, (yyssize + 1) * sizeof(YYSTYPE));
		yypv = yyv + yyvindex;
#if YYDEBUG
		yytypev = (short *)realloc(yytypev,(yyssize + 1)*sizeof(short));
		yytp = yytypev + yytindex;
#endif
		if (yys == (short *)0 || yyv == (YYSTYPE *)0
#if YYDEBUG
			|| yytypev == (short *) 0
#endif
		) {
			CMDerror(m_textmsg(4967, 
					  "Not enough space for parser stacks",
					  "E"));
			YYABORT;
		}
	}
#else
	if (++yyps > &yys[YYSSIZE]) {
		CMDerror(m_textmsg(4968, "Parser stack overflow", "E"));
		YYABORT;
	}
#endif /* !YYDYNAMIC */
	*yyps = yystate;	/* stack current state */
	*++yypv = yyval;	/* ... and value */
#if YYDEBUG
	*++yytp = yyruletype;	/* ... and type */

	if (CMDdebug)
		YY_TRACE(yyShowState)
#endif

	/*
	 *	Look up next action in action table.
	 */
yyEncore:
#ifdef YYSYNC
	YYREAD;
#endif

#ifdef YACC_WINDOWS
	if (yystate >= Sizeof_yypact) 	/* simple state */
#else /* YACC_WINDOWS */
	if (yystate >= sizeof yypact/sizeof yypact[0]) 	/* simple state */
#endif /* YACC_WINDOWS */
		yyi = yystate - YYDELTA;	/* reduce in any case */
	else {
		if(*(yyp = &yyact[yypact[yystate]]) >= 0) {
			/* Look for a shift on CMDchar */
#ifndef YYSYNC
			YYREAD;
#endif
			yyq = yyp;
			yyi = CMDchar;
			while (yyi < *yyp++)
				;
			if (yyi == yyp[-1]) {
				yystate = yyneg(YYQYYP);
#if YYDEBUG
				if (CMDdebug) {
					yyruletype = yyGetType(CMDchar);
					YY_TRACE(yyShowShift)
				}
#endif
				yyval = CMDlval;	/* stack what CMDlex() set */
				yyclearin;		/* clear token */
				if (yyerrflag)
					yyerrflag--;	/* successful shift */
				goto yyStack;
			}
		}

		/*
	 	 *	Fell through - take default action
	 	 */

#ifdef YACC_WINDOWS
		if (yystate >= Sizeof_yydef)
#else /* YACC_WINDOWS */
		if (yystate >= sizeof yydef /sizeof yydef[0])
#endif /* YACC_WINDOWS */
			goto yyError;
		if ((yyi = yydef[yystate]) < 0)	 { /* default == reduce? */
			/* Search exception table */
#ifdef YACC_WINDOWS
			yyassert((unsigned)yyneg(yyi) < Sizeof_yyex,
				m_textmsg(2825, "exception %d\n", "I num"), yystate);
#else /* YACC_WINDOWS */
			yyassert((unsigned)yyneg(yyi) < sizeof yyex/sizeof yyex[0],
				m_textmsg(2825, "exception %d\n", "I num"), yystate);
#endif /* YACC_WINDOWS */
			yyp = &yyex[yyneg(yyi)];
#ifndef YYSYNC
			YYREAD;
#endif
			while((yyi = *yyp) >= 0 && yyi != CMDchar)
				yyp += 2;
			yyi = yyp[1];
			yyassert(yyi >= 0,
				 m_textmsg(2826, "Ex table not reduce %d\n", "I num"), yyi);
		}
	}

	yyassert((unsigned)yyi < yynrule, m_textmsg(2827, "reduce %d\n", "I num"), yyi);
	yyj = yyrlen[yyi];
#if YYDEBUG
	if (CMDdebug)
		YY_TRACE(yyShowReduce)
	yytp -= yyj;
#endif
	yyps -= yyj;		/* pop stacks */
	yypvt = yypv;		/* save top */
	yypv -= yyj;
	yyval = yypv[1];	/* default action $ = $1 */
#if YYDEBUG
	yyruletype = yyRules[yyrmap[yyi]].type;
#endif

	switch (yyi) {		/* perform semantic action */
		
case YYr2: {	/* program :  */
#line 116 "code\gram.y"
 yyval.decl = nil; 
} break;

case YYr4: {	/* decl :  stmt */
#line 122 "code\gram.y"
 if(!statementList) { statementList = yypvt[0].stmt; } else { statementList->append(yypvt[0].stmt); } yyval.decl = nil; 
} break;

case YYr5: {	/* statement_list :  */
#line 127 "code\gram.y"
 yyval.stmt = nil; 
} break;

case YYr6: {	/* statement_list :  statement_list stmt */
#line 129 "code\gram.y"
 if(!yypvt[-1].stmt) { yyval.stmt = yypvt[0].stmt; } else { (yypvt[-1].stmt)->append(yypvt[0].stmt); yyval.stmt = yypvt[-1].stmt; } 
} break;

case YYr12: {	/* stmt :  rwBREAK ';' */
#line 139 "code\gram.y"
 yyval.stmt = new BreakStmtNode(); 
} break;

case YYr13: {	/* stmt :  rwCONTINUE ';' */
#line 141 "code\gram.y"
 yyval.stmt = new ContinueStmtNode(); 
} break;

case YYr14: {	/* stmt :  rwRETURN ';' */
#line 143 "code\gram.y"
 yyval.stmt = new ReturnStmtNode(); 
} break;

case YYr15: {	/* stmt :  rwRETURN expr ';' */
#line 145 "code\gram.y"
 yyval.stmt = new ReturnStmtNode(yypvt[-1].expr); 
} break;

case YYr16: {	/* stmt :  expression_stmt ';' */
#line 147 "code\gram.y"
 yyval.stmt = yypvt[-1].stmt; 
} break;

case YYr17: {	/* stmt :  IDENT IDENT '{' slot_assign_list '}' ';' */
#line 149 "code\gram.y"
 yyval.stmt = new StructDeclNode(yypvt[-5].s, yypvt[-4].s, yypvt[-2].slist); 
} break;

case YYr18: {	/* stmt :  IDENT '{' slot_assign_list '}' ';' */
#line 151 "code\gram.y"
 yyval.stmt = new StructDeclNode(yypvt[-4].s, stringTable.insert(""), yypvt[-2].slist); 
} break;

case YYr19: {	/* stmt :  IDENT IDENT ';' */
#line 153 "code\gram.y"
 yyval.stmt = new StructDeclNode(yypvt[-2].s, yypvt[-1].s, NULL); 
} break;

case YYr20: {	/* stmt :  TTAG '=' expr ';' */
#line 155 "code\gram.y"
 yyval.stmt = new TTagSetStmtNode(yypvt[-3].s, yypvt[-1].expr, NULL); 
} break;

case YYr21: {	/* stmt :  TTAG '=' expr ',' expr ';' */
#line 157 "code\gram.y"
 yyval.stmt = new TTagSetStmtNode(yypvt[-5].s, yypvt[-3].expr, yypvt[-1].expr); 
} break;

case YYr22: {	/* fn_decl_stmt :  rwDEFINE IDENT '(' ')' '{' statement_list '}' */
#line 162 "code\gram.y"
 yyval.stmt = new FunctionDeclStmtNode(yypvt[-5].s, NULL, yypvt[-1].stmt); 
} break;

case YYr23: {	/* fn_decl_stmt :  rwDEFINE IDENT '(' var_list ')' '{' statement_list '}' */
#line 164 "code\gram.y"
 yyval.stmt = new FunctionDeclStmtNode(yypvt[-6].s, yypvt[-4].var, yypvt[-1].stmt); 
} break;

case YYr24: {	/* var_list :  VAR */
#line 169 "code\gram.y"
 yyval.var = new VarNode(yypvt[0].s); 
} break;

case YYr25: {	/* var_list :  var_list ',' VAR */
#line 171 "code\gram.y"
 yyval.var = yypvt[-2].var; (yypvt[-2].var)->append(new VarNode(yypvt[0].s)); 
} break;

case YYr26: {	/* object_decl :  rwDECLARE IDENT expr '{' declare_block '}' ';' */
#line 176 "code\gram.y"
 yyval.stmt = new ObjectDeclNode(yypvt[-5].s, yypvt[-4].expr, yypvt[-2].odcl.slots, yypvt[-2].odcl.decls); 
} break;

case YYr27: {	/* object_decl :  rwDECLARE IDENT '{' declare_block '}' ';' */
#line 178 "code\gram.y"
 yyval.stmt = new ObjectDeclNode(yypvt[-4].s, NULL, yypvt[-2].odcl.slots, yypvt[-2].odcl.decls); 
} break;

case YYr28: {	/* object_decl :  rwDECLARE IDENT expr ';' */
#line 180 "code\gram.y"
 yyval.stmt = new ObjectDeclNode(yypvt[-2].s, yypvt[-1].expr, NULL, NULL); 
} break;

case YYr29: {	/* object_decl :  rwDECLARE IDENT ';' */
#line 182 "code\gram.y"
 yyval.stmt = new ObjectDeclNode(yypvt[-1].s, NULL, NULL, NULL); 
} break;

case YYr30: {	/* declare_block :  slot_assign_list */
#line 187 "code\gram.y"
 yyval.odcl.slots = yypvt[0].slist; yyval.odcl.decls = NULL; 
} break;

case YYr31: {	/* declare_block :  object_decl_list */
#line 189 "code\gram.y"
 yyval.odcl.slots = NULL; yyval.odcl.decls = yypvt[0].stmt; 
} break;

case YYr32: {	/* declare_block :  slot_assign_list object_decl_list */
#line 191 "code\gram.y"
 yyval.odcl.slots = yypvt[-1].slist; yyval.odcl.decls = yypvt[0].stmt; 
} break;

case YYr33: {	/* object_decl_list :  object_decl */
#line 196 "code\gram.y"
 yyval.stmt = yypvt[0].stmt; 
} break;

case YYr34: {	/* object_decl_list :  object_decl_list object_decl */
#line 198 "code\gram.y"
 yypvt[-1].stmt->append(yypvt[0].stmt); yyval.stmt = yypvt[-1].stmt; 
} break;

case YYr35: {	/* stmt_block :  '{' statement_list '}' */
#line 203 "code\gram.y"
 yyval.stmt = yypvt[-1].stmt; 
} break;

case YYr36: {	/* stmt_block :  stmt */
#line 205 "code\gram.y"
 yyval.stmt = yypvt[0].stmt; 
} break;

case YYr37: {	/* if_stmt :  rwIF '(' expr ')' stmt_block */
#line 210 "code\gram.y"
 yyval.stmt = new IfStmtNode(yypvt[-2].expr, yypvt[0].stmt); 
} break;

case YYr38: {	/* if_stmt :  rwIF '(' expr ')' stmt_block rwELSE stmt_block */
#line 212 "code\gram.y"
 yyval.stmt = new IfStmtNode(yypvt[-4].expr, yypvt[-2].stmt, yypvt[0].stmt); 
} break;

case YYr39: {	/* while_stmt :  rwWHILE '(' expr ')' stmt_block */
#line 217 "code\gram.y"
 yyval.stmt = new LoopStmtNode(nil, yypvt[-2].expr, nil, yypvt[0].stmt); 
} break;

case YYr40: {	/* for_stmt :  rwFOR '(' expr ';' expr ';' expr ')' stmt_block */
#line 222 "code\gram.y"
 yyval.stmt = new LoopStmtNode(yypvt[-6].expr, yypvt[-4].expr, yypvt[-2].expr, yypvt[0].stmt); 
} break;

case YYr41: {	/* expression_stmt :  stmt_expr */
#line 227 "code\gram.y"
 yyval.stmt = yypvt[0].expr; 
} break;

case YYr42: {	/* expr :  stmt_expr */
#line 232 "code\gram.y"
 yyval.expr = yypvt[0].expr; 
} break;

case YYr43: {	/* expr :  '(' expr ')' */
#line 234 "code\gram.y"
 yyval.expr = yypvt[-1].expr; 
} break;

case YYr44: {	/* expr :  expr '^' expr */
#line 236 "code\gram.y"
 yyval.expr = new IntBinaryExprNode(yypvt[-1].i, yypvt[-2].expr, yypvt[0].expr); 
} break;

case YYr45: {	/* expr :  expr '%' expr */
#line 238 "code\gram.y"
 yyval.expr = new IntBinaryExprNode(yypvt[-1].i, yypvt[-2].expr, yypvt[0].expr); 
} break;

case YYr46: {	/* expr :  expr '&' expr */
#line 240 "code\gram.y"
 yyval.expr = new IntBinaryExprNode(yypvt[-1].i, yypvt[-2].expr, yypvt[0].expr); 
} break;

case YYr47: {	/* expr :  expr '|' expr */
#line 242 "code\gram.y"
 yyval.expr = new IntBinaryExprNode(yypvt[-1].i, yypvt[-2].expr, yypvt[0].expr); 
} break;

case YYr48: {	/* expr :  expr '+' expr */
#line 244 "code\gram.y"
 yyval.expr = new FloatBinaryExprNode(yypvt[-1].i, yypvt[-2].expr, yypvt[0].expr); 
} break;

case YYr49: {	/* expr :  expr '-' expr */
#line 246 "code\gram.y"
 yyval.expr = new FloatBinaryExprNode(yypvt[-1].i, yypvt[-2].expr, yypvt[0].expr); 
} break;

case YYr50: {	/* expr :  expr '*' expr */
#line 248 "code\gram.y"
 yyval.expr = new FloatBinaryExprNode(yypvt[-1].i, yypvt[-2].expr, yypvt[0].expr); 
} break;

case YYr51: {	/* expr :  expr '/' expr */
#line 250 "code\gram.y"
 yyval.expr = new FloatBinaryExprNode(yypvt[-1].i, yypvt[-2].expr, yypvt[0].expr); 
} break;

case YYr52: {	/* expr :  '-' expr */
#line 252 "code\gram.y"
 yyval.expr = new FloatUnaryExprNode(yypvt[-1].i, yypvt[0].expr); 
} break;

case YYr53: {	/* expr :  '*' expr */
#line 254 "code\gram.y"
 yyval.expr = new TTagDerefNode(yypvt[0].expr); 
} break;

case YYr54: {	/* expr :  TTAG */
#line 256 "code\gram.y"
 yyval.expr = new TTagExprNode(yypvt[0].s); 
} break;

case YYr55: {	/* expr :  expr '<' expr */
#line 258 "code\gram.y"
 yyval.expr = new IntBinaryExprNode(yypvt[-1].i, yypvt[-2].expr, yypvt[0].expr); 
} break;

case YYr56: {	/* expr :  expr '>' expr */
#line 260 "code\gram.y"
 yyval.expr = new IntBinaryExprNode(yypvt[-1].i, yypvt[-2].expr, yypvt[0].expr); 
} break;

case YYr57: {	/* expr :  expr opGE expr */
#line 262 "code\gram.y"
 yyval.expr = new IntBinaryExprNode(yypvt[-1].i, yypvt[-2].expr, yypvt[0].expr); 
} break;

case YYr58: {	/* expr :  expr opLE expr */
#line 264 "code\gram.y"
 yyval.expr = new IntBinaryExprNode(yypvt[-1].i, yypvt[-2].expr, yypvt[0].expr); 
} break;

case YYr59: {	/* expr :  expr opEQ expr */
#line 266 "code\gram.y"
 yyval.expr = new IntBinaryExprNode(yypvt[-1].i, yypvt[-2].expr, yypvt[0].expr); 
} break;

case YYr60: {	/* expr :  expr opNE expr */
#line 268 "code\gram.y"
 yyval.expr = new IntBinaryExprNode(yypvt[-1].i, yypvt[-2].expr, yypvt[0].expr); 
} break;

case YYr61: {	/* expr :  expr opOR expr */
#line 270 "code\gram.y"
 yyval.expr = new IntBinaryExprNode(yypvt[-1].i, yypvt[-2].expr, yypvt[0].expr); 
} break;

case YYr62: {	/* expr :  expr opSHL expr */
#line 272 "code\gram.y"
 yyval.expr = new IntBinaryExprNode(yypvt[-1].i, yypvt[-2].expr, yypvt[0].expr); 
} break;

case YYr63: {	/* expr :  expr opSHR expr */
#line 274 "code\gram.y"
 yyval.expr = new IntBinaryExprNode(yypvt[-1].i, yypvt[-2].expr, yypvt[0].expr); 
} break;

case YYr64: {	/* expr :  expr opAND expr */
#line 276 "code\gram.y"
 yyval.expr = new IntBinaryExprNode(yypvt[-1].i, yypvt[-2].expr, yypvt[0].expr); 
} break;

case YYr65: {	/* expr :  expr opCAT expr */
#line 278 "code\gram.y"
 yyval.expr = new StrcatExprNode(yypvt[-1].i, yypvt[-2].expr, yypvt[0].expr); 
} break;

case YYr66: {	/* expr :  expr '@' expr */
#line 280 "code\gram.y"
 yyval.expr = new StrcatExprNode(opCAT, yypvt[-2].expr, yypvt[0].expr); 
} break;

case YYr67: {	/* expr :  '!' expr */
#line 282 "code\gram.y"
 yyval.expr = new IntUnaryExprNode(yypvt[-1].i, yypvt[0].expr); 
} break;

case YYr68: {	/* expr :  '~' expr */
#line 284 "code\gram.y"
 yyval.expr = new IntUnaryExprNode(yypvt[-1].i, yypvt[0].expr); 
} break;

case YYr69: {	/* expr :  VAR */
#line 286 "code\gram.y"
 yyval.expr = new VarNode(yypvt[0].s); 
} break;

case YYr70: {	/* expr :  VAR '[' aidx_expr ']' */
#line 288 "code\gram.y"
 yyval.expr = new VarNode(yypvt[-3].s, yypvt[-1].expr); 
} break;

case YYr71: {	/* expr :  STRATOM */
#line 290 "code\gram.y"
 yyval.expr = new StrConstNode(yypvt[0].str); 
} break;

case YYr72: {	/* expr :  FLTCONST */
#line 292 "code\gram.y"
 yyval.expr = new FloatNode(yypvt[0].f); 
} break;

case YYr73: {	/* expr :  INTCONST */
#line 294 "code\gram.y"
 yyval.expr = new IntNode(yypvt[0].i); 
} break;

case YYr74: {	/* expr :  IDENT */
#line 296 "code\gram.y"
 yyval.expr = new ConstantNode(yypvt[0].s); 
} break;

case YYr75: {	/* expr :  rwBREAK */
#line 298 "code\gram.y"
 yyval.expr = new ConstantNode(stringTable.insert("break")); 
} break;

case YYr76: {	/* expr :  slot_acc */
#line 300 "code\gram.y"
 yyval.expr = new SlotAccessNode(yypvt[0].slot.object, yypvt[0].slot.slotName, yypvt[0].slot.array1, yypvt[0].slot.array2); 
} break;

case YYr77: {	/* slot_acc :  expr '.' IDENT */
#line 305 "code\gram.y"
 yyval.slot.object = yypvt[-2].expr; yyval.slot.slotName = yypvt[0].s; yyval.slot.array1 = NULL; yyval.slot.array2 = NULL; 
} break;

case YYr78: {	/* slot_acc :  expr '.' IDENT '[' aidx_expr ']' */
#line 307 "code\gram.y"
 yyval.slot.object = yypvt[-5].expr; yyval.slot.slotName = yypvt[-3].s; yyval.slot.array1 = yypvt[-1].expr; yyval.slot.array2 = NULL; 
} break;

case YYr79: {	/* slot_acc :  expr '.' IDENT '[' aidx_expr ']' '[' expr ']' */
#line 309 "code\gram.y"
 yyval.slot.object = yypvt[-8].expr; yyval.slot.slotName = yypvt[-6].s; yyval.slot.array1 = yypvt[-1].expr; yyval.slot.array2 = yypvt[-4].expr; 
} break;

case YYr80: {	/* assign_op_struct :  opPLUSPLUS */
#line 314 "code\gram.y"
 yyval.asn.token = '+'; yyval.asn.expr = new FloatNode(1); yyval.asn.integer = false; 
} break;

case YYr81: {	/* assign_op_struct :  opMINUSMINUS */
#line 316 "code\gram.y"
 yyval.asn.token = '-'; yyval.asn.expr = new FloatNode(1); yyval.asn.integer = false; 
} break;

case YYr82: {	/* assign_op_struct :  opPLASN expr */
#line 318 "code\gram.y"
 yyval.asn.token = '+'; yyval.asn.expr = yypvt[0].expr; yyval.asn.integer = false; 
} break;

case YYr83: {	/* assign_op_struct :  opMIASN expr */
#line 320 "code\gram.y"
 yyval.asn.token = '-'; yyval.asn.expr = yypvt[0].expr; yyval.asn.integer = false; 
} break;

case YYr84: {	/* assign_op_struct :  opMLASN expr */
#line 322 "code\gram.y"
 yyval.asn.token = '*'; yyval.asn.expr = yypvt[0].expr; yyval.asn.integer = false; 
} break;

case YYr85: {	/* assign_op_struct :  opDVASN expr */
#line 324 "code\gram.y"
 yyval.asn.token = '/'; yyval.asn.expr = yypvt[0].expr; yyval.asn.integer = false; 
} break;

case YYr86: {	/* assign_op_struct :  opMODASN expr */
#line 326 "code\gram.y"
 yyval.asn.token = '%'; yyval.asn.expr = yypvt[0].expr; yyval.asn.integer = true; 
} break;

case YYr87: {	/* assign_op_struct :  opANDASN expr */
#line 328 "code\gram.y"
 yyval.asn.token = '&'; yyval.asn.expr = yypvt[0].expr; yyval.asn.integer = true; 
} break;

case YYr88: {	/* assign_op_struct :  opXORASN expr */
#line 330 "code\gram.y"
 yyval.asn.token = '^'; yyval.asn.expr = yypvt[0].expr; yyval.asn.integer = true; 
} break;

case YYr89: {	/* assign_op_struct :  opORASN expr */
#line 332 "code\gram.y"
 yyval.asn.token = '|'; yyval.asn.expr = yypvt[0].expr; yyval.asn.integer = true; 
} break;

case YYr90: {	/* assign_op_struct :  opSLASN expr */
#line 334 "code\gram.y"
 yyval.asn.token = opSHL; yyval.asn.expr = yypvt[0].expr; yyval.asn.integer = true; 
} break;

case YYr91: {	/* assign_op_struct :  opSRASN expr */
#line 336 "code\gram.y"
 yyval.asn.token = opSHR; yyval.asn.expr = yypvt[0].expr; yyval.asn.integer = true; 
} break;

case YYr92: {	/* stmt_expr :  funcall_expr */
#line 341 "code\gram.y"
 yyval.expr = yypvt[0].expr; 
} break;

case YYr93: {	/* stmt_expr :  VAR '=' expr */
#line 343 "code\gram.y"
 yyval.expr = new AssignExprNode(yypvt[-2].s, yypvt[0].expr); 
} break;

case YYr94: {	/* stmt_expr :  VAR '[' aidx_expr ']' '=' expr */
#line 345 "code\gram.y"
 yyval.expr = new AssignExprNode(yypvt[-5].s, yypvt[-3].expr, yypvt[0].expr); 
} break;

case YYr95: {	/* stmt_expr :  VAR assign_op_struct */
#line 347 "code\gram.y"
 yyval.expr = new AssignOpExprNode(yypvt[-1].s, NULL, yypvt[0].asn.token, yypvt[0].asn.expr, yypvt[0].asn.integer); 
} break;

case YYr96: {	/* stmt_expr :  VAR '[' aidx_expr ']' assign_op_struct */
#line 349 "code\gram.y"
 yyval.expr = new AssignOpExprNode(yypvt[-4].s, yypvt[-2].expr, yypvt[0].asn.token, yypvt[0].asn.expr, yypvt[0].asn.integer); 
} break;

case YYr97: {	/* stmt_expr :  slot_acc assign_op_struct */
#line 351 "code\gram.y"
 yyval.expr = new SlotAssignOpNode(yypvt[-1].slot.object, yypvt[-1].slot.slotName, yypvt[-1].slot.array1, yypvt[-1].slot.array2, yypvt[0].asn.token, yypvt[0].asn.expr, yypvt[0].asn.integer); 
} break;

case YYr98: {	/* stmt_expr :  slot_acc '=' expr */
#line 353 "code\gram.y"
 yyval.expr = new SlotAssignNode(yypvt[-2].slot.object, yypvt[-2].slot.slotName, yypvt[-2].slot.array1, yypvt[-2].slot.array2, yypvt[0].expr); 
} break;

case YYr99: {	/* stmt_expr :  slot_acc '=' '{' expr_list '}' */
#line 355 "code\gram.y"
 yyval.expr = new SlotAssignNode(yypvt[-4].slot.object, yypvt[-4].slot.slotName, yypvt[-4].slot.array1, yypvt[-4].slot.array2, yypvt[-1].expr); 
} break;

case YYr100: {	/* funcall_expr :  IDENT '(' ')' */
#line 360 "code\gram.y"
 yyval.expr = new FuncCallExprNode(yypvt[-2].s); 
} break;

case YYr101: {	/* funcall_expr :  IDENT '(' expr_list ')' */
#line 362 "code\gram.y"
 yyval.expr = new FuncCallExprNode(yypvt[-3].s, yypvt[-1].expr); 
} break;

case YYr102: {	/* funcall_expr :  expr '.' IDENT '(' ')' */
#line 364 "code\gram.y"
 yyval.expr = new FuncCallExprNode(yypvt[-2].s, NULL, yypvt[-4].expr); 
} break;

case YYr103: {	/* funcall_expr :  expr '.' IDENT '(' expr_list ')' */
#line 366 "code\gram.y"
 yyval.expr = new FuncCallExprNode(yypvt[-3].s, yypvt[-1].expr, yypvt[-5].expr); 
} break;

case YYr104: {	/* expr_list :  expr */
#line 371 "code\gram.y"
 yyval.expr = yypvt[0].expr; 
} break;

case YYr105: {	/* expr_list :  expr_list ',' expr */
#line 373 "code\gram.y"
 (yypvt[-2].expr)->append(yypvt[0].expr); yyval.expr = yypvt[-2].expr; 
} break;

case YYr106: {	/* slot_assign_list :  slot_assign */
#line 378 "code\gram.y"
 yyval.slist = yypvt[0].slist; 
} break;

case YYr107: {	/* slot_assign_list :  slot_assign_list slot_assign */
#line 380 "code\gram.y"
 yypvt[-1].slist->append(yypvt[0].slist); yyval.slist = yypvt[-1].slist; 
} break;

case YYr108: {	/* slot_assign :  IDENT '=' expr ';' */
#line 384 "code\gram.y"
 yyval.slist = new SlotAssignListNode(yypvt[-3].s, NULL, NULL, yypvt[-1].expr); 
} break;

case YYr109: {	/* slot_assign :  IDENT '[' aidx_expr ']' '=' expr ';' */
#line 386 "code\gram.y"
 yyval.slist = new SlotAssignListNode(yypvt[-6].s, yypvt[-4].expr, NULL, yypvt[-1].expr); 
} break;

case YYr110: {	/* slot_assign :  IDENT '[' aidx_expr ']' '[' expr ']' '=' expr ';' */
#line 388 "code\gram.y"
 yyval.slist = new SlotAssignListNode(yypvt[-9].s, yypvt[-7].expr, yypvt[-4].expr, yypvt[-1].expr); 
} break;

case YYr111: {	/* slot_assign :  IDENT '=' '{' expr_list '}' ';' */
#line 390 "code\gram.y"
 yyval.slist = new SlotAssignListNode(yypvt[-5].s, NULL, NULL, yypvt[-2].expr); 
} break;

case YYr112: {	/* slot_assign :  IDENT '[' aidx_expr ']' '=' '{' expr_list '}' ';' */
#line 392 "code\gram.y"
 yyval.slist = new SlotAssignListNode(yypvt[-8].s, yypvt[-6].expr, NULL, yypvt[-2].expr); 
} break;

case YYr113: {	/* aidx_expr :  expr */
#line 397 "code\gram.y"
 yyval.expr = yypvt[0].expr; 
} break;

case YYr114: {	/* aidx_expr :  aidx_expr ',' expr */
#line 399 "code\gram.y"
 yyval.expr = new CommaCatExprNode(yypvt[-2].expr, yypvt[0].expr); 
} break;
#line 314 "c:/etc/yyparse.c"
	case YYrACCEPT:
		YYACCEPT;
	case YYrERROR:
		goto yyError;
	}

	/*
	 *	Look up next state in goto table.
	 */

	yyp = &yygo[yypgo[yyi]];
	yyq = yyp++;
	yyi = *yyps;
	while (yyi < *yyp++)
		;

	yystate = yyneg(yyi == *--yyp? YYQYYP: *yyq);
#if YYDEBUG
	if (CMDdebug)
		YY_TRACE(yyShowGoto)
#endif
	goto yyStack;

yyerrlabel:	;		/* come here from YYERROR	*/
/*
#pragma used yyerrlabel
 */
	yyerrflag = 1;
	if (yyi == YYrERROR) {
		yyps--;
		yypv--;
#if YYDEBUG
		yytp--;
#endif
	}

yyError:
	switch (yyerrflag) {

	case 0:		/* new error */
		yynerrs++;
		yyi = CMDchar;
		CMDerror(m_textmsg(4969, "Syntax error", "E"));
		if (yyi != CMDchar) {
			/* user has changed the current token */
			/* try again */
			yyerrflag++;	/* avoid loops */
			goto yyEncore;
		}

	case 1:		/* partially recovered */
	case 2:
		yyerrflag = 3;	/* need 3 valid shifts to recover */
			
		/*
		 *	Pop states, looking for a
		 *	shift on `error'.
		 */

		for ( ; yyps > yys; yyps--, yypv--
#if YYDEBUG
					, yytp--
#endif
		) {
#ifdef YACC_WINDOWS
			if (*yyps >= Sizeof_yypact)
#else /* YACC_WINDOWS */
			if (*yyps >= sizeof yypact/sizeof yypact[0])
#endif /* YACC_WINDOWS */
				continue;
			yyp = &yyact[yypact[*yyps]];
			yyq = yyp;
			do {
				if (YYERRCODE == *yyp) {
					yyp++;
					yystate = yyneg(YYQYYP);
					goto yyStack;
				}
			} while (*yyp++ > YYTOKEN_BASE);
		
			/* no shift in this state */
#if YYDEBUG
			if (CMDdebug && yyps > yys+1)
				YY_TRACE(yyShowErrRecovery)
#endif
			/* pop stacks; try again */
		}
		/* no shift on error - abort */
		break;

	case 3:
		/*
		 *	Erroneous token after
		 *	an error - discard it.
		 */

		if (CMDchar == 0)  /* but not EOF */
			break;
#if YYDEBUG
		if (CMDdebug)
			YY_TRACE(yyShowErrDiscard)
#endif
		yyclearin;
		goto yyEncore;	/* try again in same state */
	}
	YYABORT;

#ifdef YYALLOC
yyReturn:
	CMDlval = save_yylval;
	yyval = save_yyval;
	yypvt = save_yypvt;
	CMDchar = save_yychar;
	yyerrflag = save_yyerrflag;
	yynerrs = save_yynerrs;
	free((char *)yys);
	free((char *)yyv);
#if YYDEBUG
	free((char *)yytypev);
#endif
	return(retval);
#endif
}

		
#if YYDEBUG
/*
 * Return type of token
 */
int
yyGetType(tok)
int tok;
{
	yyNamedType * tp;
	for (tp = &yyTokenTypes[yyntoken-1]; tp > yyTokenTypes; tp--)
		if (tp->token == tok)
			return tp->type;
	return 0;
}
/*
 * Print a token legibly.
 */
char *
yyptok(tok)
int tok;
{
	yyNamedType * tp;
	for (tp = &yyTokenTypes[yyntoken-1]; tp > yyTokenTypes; tp--)
		if (tp->token == tok)
			return tp->name;
	return "";
}

/*
 * Read state 'num' from YYStatesFile
 */
#ifdef YYTRACE

static char *
yygetState(num)
int num;
{
	int	size;
	static FILE *yyStatesFile = (FILE *) 0;
	static char yyReadBuf[YYMAX_READ+1];

	if (yyStatesFile == (FILE *) 0
	 && (yyStatesFile = fopen(YYStatesFile, "r")) == (FILE *) 0)
		return "yyExpandName: cannot open states file";

	if (num < yynstate - 1)
		size = (int)(yyStates[num+1] - yyStates[num]);
	else {
		/* length of last item is length of file - ptr(last-1) */
		if (fseek(yyStatesFile, 0L, 2) < 0)
			goto cannot_seek;
		size = (int) (ftell(yyStatesFile) - yyStates[num]);
	}
	if (size < 0 || size > YYMAX_READ)
		return "yyExpandName: bad read size";
	if (fseek(yyStatesFile, yyStates[num], 0) < 0) {
	cannot_seek:
		return "yyExpandName: cannot seek in states file";
	}

	(void) fread(yyReadBuf, 1, size, yyStatesFile);
	yyReadBuf[size] = '\0';
	return yyReadBuf;
}
#endif /* YYTRACE */
/*
 * Expand encoded string into printable representation
 * Used to decode yyStates and yyRules strings.
 * If the expansion of 's' fits in 'buf', return 1; otherwise, 0.
 */
int
yyExpandName(num, isrule, buf, len)
int num, isrule;
char * buf;
int len;
{
	int	i, n, cnt, type;
	char	* endp, * cp;
	char	*s;

	if (isrule)
		s = yyRules[num].name;
	else
#ifdef YYTRACE
		s = yygetState(num);
#else
		s = "*no states*";
#endif

	for (endp = buf + len - 8; *s; s++) {
		if (buf >= endp) {		/* too large: return 0 */
		full:	(void) strcpy(buf, " ...\n");
			return 0;
		} else if (*s == '%') {		/* nonterminal */
			type = 0;
			cnt = yynvar;
			goto getN;
		} else if (*s == '&') {		/* terminal */
			type = 1;
			cnt = yyntoken;
		getN:
			if (cnt < 100)
				i = 2;
			else if (cnt < 1000)
				i = 3;
			else
				i = 4;
			for (n = 0; i-- > 0; )
				n = (n * 10) + *++s - '0';
			if (type == 0) {
				if (n >= yynvar)
					goto too_big;
				cp = yysvar[n];
			} else if (n >= yyntoken) {
			    too_big:
				cp = "<range err>";
			} else
				cp = yyTokenTypes[n].name;

			if ((i = strlen(cp)) + buf > endp)
				goto full;
			(void) strcpy(buf, cp);
			buf += i;
		} else
			*buf++ = *s;
	}
	*buf = '\0';
	return 1;
}
#ifndef YYTRACE
/*
 * Show current state of CMDparse
 */
void
yyShowState(tp)
yyTraceItems * tp;
{
	short * p;
	YYSTYPE * q;

	printf(
	    m_textmsg(2828, "state %d (%d), char %s (%d)\n", "I num1 num2 char num3"),
	      yysmap[tp->state], tp->state,
	      yyptok(tp->lookahead), tp->lookahead);
}
/*
 * show results of reduction
 */
void
yyShowReduce(tp)
yyTraceItems * tp;
{
	printf("reduce %d (%d), pops %d (%d)\n",
		yyrmap[tp->rule], tp->rule,
		tp->states[tp->nstates - tp->npop],
		yysmap[tp->states[tp->nstates - tp->npop]]);
}
void
yyShowRead(val)
int val;
{
	printf(m_textmsg(2829, "read %s (%d)\n", "I token num"), yyptok(val), val);
}
void
yyShowGoto(tp)
yyTraceItems * tp;
{
	printf(m_textmsg(2830, "goto %d (%d)\n", "I num1 num2"), yysmap[tp->state], tp->state);
}
void
yyShowShift(tp)
yyTraceItems * tp;
{
	printf(m_textmsg(2831, "shift %d (%d)\n", "I num1 num2"), yysmap[tp->state], tp->state);
}
void
yyShowErrRecovery(tp)
yyTraceItems * tp;
{
	short	* top = tp->states + tp->nstates - 1;

	printf(
	m_textmsg(2832, "Error recovery pops state %d (%d), uncovers %d (%d)\n", "I num1 num2 num3 num4"),
		yysmap[*top], *top, yysmap[*(top-1)], *(top-1));
}
void
yyShowErrDiscard(tp)
yyTraceItems * tp;
{
	printf(m_textmsg(2833, "Error recovery discards %s (%d), ", "I token num"),
		yyptok(tp->lookahead), tp->lookahead);
}
#endif	/* ! YYTRACE */
#endif	/* YYDEBUG */
