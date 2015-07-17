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
#define CHRCONST	274
#define INTCONST	275
#define TTAG	276
#define VAR	277
#define IDENT	278
#define STRATOM	279
#define FLTCONST	280
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
extern YYSTYPE CMDlval;
