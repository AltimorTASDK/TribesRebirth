%{
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
%}
%{
        /* Reserved Word Definitions */
%}
%token <i> rwDEFINE rwENDDEF rwDECLARE
%token <i> rwBREAK rwCLASS rwELSE rwCONTINUE rwGLOBAL
%token <i> rwIF rwNIL rwRETURN rwWHILE
%token <i> rwENDIF rwENDWHILE rwENDFOR
%token <i> rwFOR
%token ILLEGAL_TOKEN
%{
        /* Constants and Identifier Definitions */
%}
%token <c> CHRCONST
%token <i> INTCONST
%token <s> TTAG
%token <s> VAR
%token <s> IDENT
%token <str> STRATOM
%token <f> FLTCONST

%{
        /* Operator Definitions */
%}
%token <i> '+' '-' '*' '/' '<' '>' '=' '.' '|' '&' '%'
%token <i> '(' ')' ',' ':' ';' '{' '}' '^' '~' '!' '@'
%token <i> opMINUSMINUS opPLUSPLUS
%token <i> STMT_SEP
%token <i> opSHL opSHR opPLASN opMIASN opMLASN opDVASN opMODASN opANDASN
%token <i> opXORASN opORASN opSLASN opSRASN opCAT
%token <i> opEQ opNE opGE opLE opAND opOR
%token <i> opCOLONCOLON

%union {
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
}

%type <decl> program
%type <decl> decl
%type <stmt> fn_decl_stmt
%type <stmt> statement_list
%type <stmt> stmt
%type <expr> expr_list
%type <expr> aidx_expr
%type <expr> funcall_expr
%type <expr> stmt_expr
%type <stmt> if_stmt
%type <stmt> while_stmt
%type <stmt> for_stmt
%type <stmt> stmt_block
%type <stmt> object_decl
%type <stmt> object_decl_list
%type <odcl> declare_block
%type <expr> expr
%type <slist> slot_assign_list
%type <slist> slot_assign
%type <slot> slot_acc
%type <stmt> expression_stmt
%type <var> var_list
%type <asn> assign_op_struct

%left '['
%right opMODASN opANDASN opXORASN opPLASN opMIASN opMLASN opDVASN opMDASN opNDASN opNTASN opORASN opSLASN opSRASN '='
%left opOR
%left opAND 
%left '|'
%left '^'
%left '&'
%left opEQ opNE
%left '<' opLE '>' opGE
%left '@' opCAT
%left opSHL opSHR
%left '+' '-'
%left '*' '/' '%'
%right '!' '~' opPLUSPLUS opMINUSMINUS UNARY
%left '.'

%%

start
	: program
	;

program 
	:
      { $$ = nil; }
	| program decl
	;
	
decl
	: stmt
      { if(!statementList) { statementList = $1; } else { statementList->append($1); } $$ = nil; }
	;

statement_list
	:
		{ $$ = nil; }
	| statement_list stmt
		{ if(!$1) { $$ = $2; } else { ($1)->append($2); $$ = $1; } }
	;

stmt
	: if_stmt
	| while_stmt
	| for_stmt
   | fn_decl_stmt
   | object_decl
	| rwBREAK ';'
		{ $$ = new BreakStmtNode(); }
	| rwCONTINUE ';'
		{ $$ = new ContinueStmtNode(); }
	| rwRETURN ';'
		{ $$ = new ReturnStmtNode(); }
	| rwRETURN expr ';'
		{ $$ = new ReturnStmtNode($2); }
	| expression_stmt ';'
		{ $$ = $1; }
   | IDENT IDENT '{' slot_assign_list '}' ';'
      { $$ = new StructDeclNode($1, $2, $4); }
   | IDENT '{' slot_assign_list '}' ';'
      { $$ = new StructDeclNode($1, stringTable.insert(""), $3); }
   | IDENT IDENT ';'
      { $$ = new StructDeclNode($1, $2, NULL); }
   | TTAG '=' expr ';'
      { $$ = new TTagSetStmtNode($1, $3, NULL); }
   | TTAG '=' expr ',' expr ';'
      { $$ = new TTagSetStmtNode($1, $3, $5); }
	;

fn_decl_stmt
	: rwDEFINE IDENT '(' ')' '{' statement_list '}'
   { $$ = new FunctionDeclStmtNode($2, NULL, $6); }
	| rwDEFINE IDENT '(' var_list ')' '{' statement_list '}'
   { $$ = new FunctionDeclStmtNode($2, $4, $7); }
	;

var_list
   : VAR
      { $$ = new VarNode($1); }
   | var_list ',' VAR
      { $$ = $1; ($1)->append(new VarNode($3)); }
   ;

object_decl
   : rwDECLARE IDENT expr '{' declare_block '}' ';'
      { $$ = new ObjectDeclNode($2, $3, $5.slots, $5.decls); }
   | rwDECLARE IDENT '{' declare_block '}' ';'
      { $$ = new ObjectDeclNode($2, NULL, $4.slots, $4.decls); }
   | rwDECLARE IDENT expr ';'
      { $$ = new ObjectDeclNode($2, $3, NULL, NULL); }
   | rwDECLARE IDENT ';'
      { $$ = new ObjectDeclNode($2, NULL, NULL, NULL); }
   ;

declare_block
   : slot_assign_list
      { $$.slots = $1; $$.decls = NULL; }
   | object_decl_list
      { $$.slots = NULL; $$.decls = $1; }
   | slot_assign_list object_decl_list
      { $$.slots = $1; $$.decls = $2; }
   ;

object_decl_list
   : object_decl
      { $$ = $1; }
   | object_decl_list object_decl
      { $1->append($2); $$ = $1; }
   ;

stmt_block
   : '{' statement_list '}'
      { $$ = $2; }
   | stmt
      { $$ = $1; }
   ;

if_stmt
	: rwIF '(' expr ')' stmt_block
		{ $$ = new IfStmtNode($3, $5); }
	| rwIF '(' expr ')' stmt_block rwELSE stmt_block
		{ $$ = new IfStmtNode($3, $5, $7); }
	;

while_stmt
	: rwWHILE '(' expr ')' stmt_block
		{ $$ = new LoopStmtNode(nil, $3, nil, $5); }
	;

for_stmt
	: rwFOR '(' expr ';' expr ';' expr ')' stmt_block
		{ $$ = new LoopStmtNode($3, $5, $7, $9); }
	;	

expression_stmt
	: stmt_expr
		{ $$ = $1; }
	;

expr
	: stmt_expr
		{ $$ = $1; }
	| '(' expr ')'
		{ $$ = $2; }
	| expr '^' expr
		{ $$ = new IntBinaryExprNode($2, $1, $3); }
	| expr '%' expr
		{ $$ = new IntBinaryExprNode($2, $1, $3); }
	| expr '&' expr
		{ $$ = new IntBinaryExprNode($2, $1, $3); }
	| expr '|' expr
		{ $$ = new IntBinaryExprNode($2, $1, $3); }
	| expr '+' expr
		{ $$ = new FloatBinaryExprNode($2, $1, $3); }
	| expr '-' expr
		{ $$ = new FloatBinaryExprNode($2, $1, $3); }
	| expr '*' expr
		{ $$ = new FloatBinaryExprNode($2, $1, $3); }
	| expr '/' expr
		{ $$ = new FloatBinaryExprNode($2, $1, $3); }
	| '-' expr	%prec UNARY
		{ $$ = new FloatUnaryExprNode($1, $2); }
   | '*' expr %prec UNARY
      { $$ = new TTagDerefNode($2); }
   | TTAG
      { $$ = new TTagExprNode($1); }
	| expr '<' expr
		{ $$ = new IntBinaryExprNode($2, $1, $3); }
	| expr '>' expr
		{ $$ = new IntBinaryExprNode($2, $1, $3); }
	| expr opGE expr
		{ $$ = new IntBinaryExprNode($2, $1, $3); }
	| expr opLE expr
		{ $$ = new IntBinaryExprNode($2, $1, $3); }
	| expr opEQ expr
		{ $$ = new IntBinaryExprNode($2, $1, $3); }
	| expr opNE expr
		{ $$ = new IntBinaryExprNode($2, $1, $3); }
	| expr opOR expr
		{ $$ = new IntBinaryExprNode($2, $1, $3); }
	| expr opSHL expr
		{ $$ = new IntBinaryExprNode($2, $1, $3); }
	| expr opSHR expr
		{ $$ = new IntBinaryExprNode($2, $1, $3); }
	| expr opAND expr
		{ $$ = new IntBinaryExprNode($2, $1, $3); }
	| expr opCAT expr
		{ $$ = new StrcatExprNode($2, $1, $3); }
	| expr '@' expr
		{ $$ = new StrcatExprNode(opCAT, $1, $3); }
	| '!' expr
		{ $$ = new IntUnaryExprNode($1, $2); }
   | '~' expr
		{ $$ = new IntUnaryExprNode($1, $2); }
	| VAR
		{ $$ = new VarNode($1); }
   | VAR '[' aidx_expr ']'
      { $$ = new VarNode($1, $3); }
	| STRATOM
		{ $$ = new StrConstNode($1); }
   | FLTCONST
		{ $$ = new FloatNode($1); }
   | INTCONST
		{ $$ = new IntNode($1); }
   | IDENT
      { $$ = new ConstantNode($1); }
   | rwBREAK
      { $$ = new ConstantNode(stringTable.insert("break")); }
   | slot_acc
      { $$ = new SlotAccessNode($1.object, $1.slotName, $1.array1, $1.array2); }
	;

slot_acc
   : expr '.' IDENT
      { $$.object = $1; $$.slotName = $3; $$.array1 = NULL; $$.array2 = NULL; }
   | expr '.' IDENT '[' aidx_expr ']'
      { $$.object = $1; $$.slotName = $3; $$.array1 = $5; $$.array2 = NULL; }
   | expr '.' IDENT '[' aidx_expr ']' '[' expr ']'
      { $$.object = $1; $$.slotName = $3; $$.array1 = $8; $$.array2 = $5; }
   ;

assign_op_struct
   : opPLUSPLUS
      { $$.token = '+'; $$.expr = new FloatNode(1); $$.integer = false; }
   | opMINUSMINUS
      { $$.token = '-'; $$.expr = new FloatNode(1); $$.integer = false; }
   | opPLASN expr
      { $$.token = '+'; $$.expr = $2; $$.integer = false; }
   | opMIASN expr
      { $$.token = '-'; $$.expr = $2; $$.integer = false; }
   | opMLASN expr
      { $$.token = '*'; $$.expr = $2; $$.integer = false; }
   | opDVASN expr
      { $$.token = '/'; $$.expr = $2; $$.integer = false; }
   | opMODASN expr
      { $$.token = '%'; $$.expr = $2; $$.integer = true; }
   | opANDASN expr
      { $$.token = '&'; $$.expr = $2; $$.integer = true; }
   | opXORASN expr
      { $$.token = '^'; $$.expr = $2; $$.integer = true; }
   | opORASN expr
      { $$.token = '|'; $$.expr = $2; $$.integer = true; }
   | opSLASN expr
      { $$.token = opSHL; $$.expr = $2; $$.integer = true; }
   | opSRASN expr
      { $$.token = opSHR; $$.expr = $2; $$.integer = true; }
   ;

stmt_expr
   : funcall_expr
      { $$ = $1; }
   | VAR '=' expr
      { $$ = new AssignExprNode($1, $3); }
   | VAR '[' aidx_expr ']' '=' expr
      { $$ = new AssignExprNode($1, $3, $6); }
   | VAR assign_op_struct
      { $$ = new AssignOpExprNode($1, NULL, $2.token, $2.expr, $2.integer); }
   | VAR '[' aidx_expr ']' assign_op_struct
      { $$ = new AssignOpExprNode($1, $3, $5.token, $5.expr, $5.integer); }
   | slot_acc assign_op_struct
      { $$ = new SlotAssignOpNode($1.object, $1.slotName, $1.array1, $1.array2, $2.token, $2.expr, $2.integer); }
   | slot_acc '=' expr
      { $$ = new SlotAssignNode($1.object, $1.slotName, $1.array1, $1.array2, $3); }
   | slot_acc '=' '{' expr_list '}'
      { $$ = new SlotAssignNode($1.object, $1.slotName, $1.array1, $1.array2, $4); }
   ;

funcall_expr
   : IDENT '(' ')'
	  { $$ = new FuncCallExprNode($1); }
   | IDENT '(' expr_list ')'
	  { $$ = new FuncCallExprNode($1, $3); }
   | expr '.' IDENT '(' ')'
      { $$ = new FuncCallExprNode($3, NULL, $1); }
   | expr '.' IDENT '(' expr_list ')'
      { $$ = new FuncCallExprNode($3, $5, $1); }
   ;

expr_list
	: expr
		{ $$ = $1; }
	| expr_list ',' expr
		{ ($1)->append($3); $$ = $1; }
	;

slot_assign_list
   : slot_assign
      { $$ = $1; }
   | slot_assign_list slot_assign
      { $1->append($2); $$ = $1; }
   ;
slot_assign
   : IDENT '=' expr ';'
      { $$ = new SlotAssignListNode($1, NULL, NULL, $3); }
   | IDENT '[' aidx_expr ']' '=' expr ';'
      { $$ = new SlotAssignListNode($1, $3, NULL, $6); }
   | IDENT '[' aidx_expr ']' '[' expr ']' '=' expr ';'
      { $$ = new SlotAssignListNode($1, $3, $6, $9); }
   | IDENT '=' '{' expr_list '}' ';'
      { $$ = new SlotAssignListNode($1, NULL, NULL, $4); }
   | IDENT '[' aidx_expr ']' '=' '{' expr_list '}' ';'
      { $$ = new SlotAssignListNode($1, $3, NULL, $7); }
   ;

aidx_expr
   : expr
      { $$ = $1; }
   | aidx_expr ',' expr
      { $$ = new CommaCatExprNode($1, $3); }
   ;
%%

