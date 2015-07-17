#ifndef _H_AST
#define _H_AST


class ExprEvalState;

class ASTNode
{
   ASTNode *next;
public:
   ASTNode()
      { next = NULL; }
   virtual ~ASTNode()
      { delete next; }

   void append(ASTNode *next);
   ASTNode *getNext() { return next; }
};

class StmtNode : public ASTNode
{
public:
   virtual int execute(ExprEvalState *);
};

class BreakStmtNode : public StmtNode
{
public:
   virtual int execute(ExprEvalState *);
};

class ContinueStmtNode : public StmtNode
{
public:
   virtual int execute(ExprEvalState *);
};

class ExprNode : public StmtNode
{
public:
   enum flags { ExprIsFloat = 1, ExprIsInt = 2 };

   virtual int execute(ExprEvalState *);
   virtual void eval(ExprEvalState *state);
   virtual double evalFloat(ExprEvalState *state);
   virtual int evalInt(ExprEvalState *state);
   virtual int getNodeFlags() { return 0; }
};

class ReturnStmtNode : public StmtNode
{
   ExprNode *expr;
public:
   virtual int execute(ExprEvalState *);
   ReturnStmtNode(ExprNode *in_expr = NULL)
      { expr = in_expr; }
   ~ReturnStmtNode()
      { delete expr; }
};

class IfStmtNode : public StmtNode
{
   ExprNode *testExpr;
   StmtNode *ifBlock, *elseBlock;
public:
   virtual int execute(ExprEvalState *);
   IfStmtNode(ExprNode *in_testExpr, StmtNode *in_ifBlock, StmtNode *in_elseBlock=NULL)
      { testExpr = in_testExpr; ifBlock = in_ifBlock; elseBlock = in_elseBlock; }
   ~IfStmtNode()
      { delete testExpr; delete ifBlock; delete elseBlock; }
};

class LoopStmtNode : public StmtNode
{
   ExprNode *testExpr;
   ExprNode *initExpr;
   ExprNode *endLoopExpr;
   StmtNode *loopBlock;
public:
   virtual int execute(ExprEvalState *);
   LoopStmtNode(ExprNode *in_initExpr, ExprNode *in_testExpr, ExprNode *in_endLoopExpr, StmtNode *in_loopBlock)
      { testExpr = in_testExpr; initExpr = in_initExpr; endLoopExpr = in_endLoopExpr; loopBlock = in_loopBlock; }
   ~LoopStmtNode()
      { delete testExpr; delete initExpr; delete endLoopExpr; delete loopBlock; }
};

class BinaryExprNode : public ExprNode
{
public:
   int op;
   ExprNode *left;
   ExprNode *right;
public:
   BinaryExprNode(int in_op, ExprNode *in_left, ExprNode *in_right)
      { op = in_op; left = in_left; right = in_right; }
   ~BinaryExprNode()
      { delete left; delete right; }
};

class FloatBinaryExprNode : public BinaryExprNode
{
public:
   FloatBinaryExprNode(int in_op, ExprNode *in_left, ExprNode *in_right) : BinaryExprNode(in_op, in_left, in_right){}
   
   double evalFloat(ExprEvalState *state);
   int evalInt(ExprEvalState *state);
   void eval(ExprEvalState *state);
   int getNodeFlags() { return ExprIsFloat; }
};

class IntBinaryExprNode : public BinaryExprNode
{
public:
   IntBinaryExprNode(int in_op, ExprNode *in_left, ExprNode *in_right) : BinaryExprNode(in_op, in_left, in_right){}

   double evalFloat(ExprEvalState *state);
   int evalInt(ExprEvalState *state);
   void eval(ExprEvalState *state);
   int getNodeFlags() { return ExprIsInt; }
};

class StrcatExprNode : public BinaryExprNode
{
public:
   StrcatExprNode(int in_op, ExprNode *in_left, ExprNode *in_right) : BinaryExprNode(in_op, in_left, in_right){}

   void eval(ExprEvalState *state);
};

class CommaCatExprNode : public BinaryExprNode
{
public:
   CommaCatExprNode(ExprNode *in_left, ExprNode *in_right) : BinaryExprNode(0, in_left, in_right){}

   void eval(ExprEvalState *state);
};

class UnaryExprNode : public ExprNode
{
protected:
   int op;
   ExprNode *expr;
public:
   UnaryExprNode(int in_op, ExprNode *in_expr)
      { op = in_op; expr = in_expr; }
   ~UnaryExprNode()
      { delete expr; }
};

class IntUnaryExprNode : public UnaryExprNode
{
public:
   IntUnaryExprNode(int in_op, ExprNode *in_expr) : UnaryExprNode(in_op, in_expr) {}

   double evalFloat(ExprEvalState *state);
   int evalInt(ExprEvalState *state);
   void eval(ExprEvalState *state);
   int getNodeFlags() { return ExprIsInt; }
};

class FloatUnaryExprNode : public UnaryExprNode
{
public:
   FloatUnaryExprNode(int in_op, ExprNode *in_expr) : UnaryExprNode(in_op, in_expr) {}

   double evalFloat(ExprEvalState *state);
   int evalInt(ExprEvalState *state);
   void eval(ExprEvalState *state);
   int getNodeFlags() { return ExprIsFloat; }
};

class VarNode : public ExprNode
{
   StringTableEntry varName;
   ExprNode *arrayIndex;
public:
   const char *getVarName() { return varName; }
   void eval(ExprEvalState *state);
   VarNode(const char *in_varName, ExprNode *in_arrayIndex = NULL)
      { varName = in_varName; arrayIndex = in_arrayIndex; }
   ~VarNode()
      { delete arrayIndex; }
};

class IntNode : public ExprNode
{
   int value;
public:
   double evalFloat(ExprEvalState *state);
   int evalInt(ExprEvalState *state);
   void eval(ExprEvalState *state);
   int getNodeFlags() { return ExprIsInt; }
   IntNode(int in_value) { value = in_value; }
};

class FloatNode : public ExprNode
{
   double value;
public:
   double evalFloat(ExprEvalState *state);
   int evalInt(ExprEvalState *state);
   void eval(ExprEvalState *state);
   int getNodeFlags() { return ExprIsFloat; }
   FloatNode(double in_value) { value = in_value; }
};

class StrConstNode : public ExprNode
{
   char *str;
public:
   void eval(ExprEvalState *state);
   StrConstNode(char *in_str) { str = in_str; }
   ~StrConstNode() { delete[] str; }
};

class ConstantNode : public ExprNode
{
   StringTableEntry value;
public:
   void eval(ExprEvalState *state);
   ConstantNode(const char *in_value)
      { value = in_value; }
};

class AssignExprNode : public ExprNode
{
   StringTableEntry varName;
   ExprNode *expr;
   ExprNode *arrayIndex;
public:
   void eval(ExprEvalState *state);
   AssignExprNode(const char *in_varName, ExprNode *in_expr)
      { varName = in_varName; expr = in_expr; arrayIndex = NULL;}
   AssignExprNode(const char *in_varName, ExprNode *in_array, ExprNode *in_expr)
      { varName = in_varName; expr = in_expr; arrayIndex = in_array;}
   ~AssignExprNode()
      { delete expr; delete arrayIndex;}
};

struct AssignDecl
{
   int token;
   ExprNode *expr;
   bool integer;
};

class AssignOpExprNode : public ExprNode
{
   StringTableEntry varName;
   ExprNode *expr;
   ExprNode *arrayIndex;
   int op;
   bool integer;
public:
   void eval(ExprEvalState *state);
   AssignOpExprNode(const char *in_varName, ExprNode *in_array, int in_op, ExprNode *in_expr, bool isInteger)
      { op = in_op; varName = in_varName; expr = in_expr; arrayIndex = in_array; integer = isInteger;}
   ~AssignOpExprNode()
      { delete expr; delete arrayIndex;}
};

class TTagSetStmtNode : public StmtNode
{
   StringTableEntry tag;
   ExprNode *valueExpr;
   ExprNode *stringExpr;
public:
   int execute(ExprEvalState *state);
   TTagSetStmtNode(const char *in_tag, ExprNode *value, ExprNode *string)
      { tag = in_tag; valueExpr = value; stringExpr = string; }
   ~TTagSetStmtNode()
      { delete valueExpr; delete stringExpr; }

};

class TTagDerefNode : public ExprNode
{
   ExprNode *expr;
public:
   void eval(ExprEvalState *state);
   TTagDerefNode(ExprNode *in_expr)
      { expr = in_expr; }
   ~TTagDerefNode()
      { delete expr; }
};

class TTagExprNode : public ExprNode
{
   StringTableEntry tag;
   int value;
public:
   void eval(ExprEvalState *state);
   int evalInt(ExprEvalState *state);
   double evalFloat(ExprEvalState *state);
   
   TTagExprNode(const char *in_tag)
      { tag = in_tag; value = 0; }
};

class FuncCallExprNode : public ExprNode
{
   StringTableEntry funcName;
   ExprNode *args;
public:
   void eval(ExprEvalState *state);
   FuncCallExprNode(const char *in_funcName, ExprNode *in_args = NULL, ExprNode *in_dot = NULL);
   ~FuncCallExprNode()
      { delete args; }
   void setExpr(ExprNode *dotexpr);
};
struct SlotDecl
{
   ExprNode *object;
   StringTableEntry slotName;
   ExprNode *array1;
   ExprNode *array2;
};

class SlotAccessNode : public ExprNode
{
   ExprNode *objectExpr, *array1Expr, *array2Expr;
   StringTableEntry slotName;

public:
   SlotAccessNode(ExprNode *obj, StringTableEntry sname, ExprNode *a1, ExprNode *a2)
      { objectExpr = obj; slotName = sname; array1Expr = a1; array2Expr = a2; }
   ~SlotAccessNode()
      { delete objectExpr; delete array1Expr; delete array2Expr; }
   void eval(ExprEvalState *state);
};

class SlotAssignNode : public ExprNode
{
   ExprNode *objectExpr, *array1Expr, *array2Expr;
   StringTableEntry slotName;
   ExprNode *valueExpr;

public:
   SlotAssignNode(ExprNode *obj, StringTableEntry sname, ExprNode *a1, ExprNode *a2, ExprNode *ve)
      { objectExpr = obj; slotName = sname; array1Expr = a1; array2Expr = a2; valueExpr = ve; }
   ~SlotAssignNode()
      { delete objectExpr; delete array1Expr; delete array2Expr; delete valueExpr; }
   void eval(ExprEvalState *state);
};

class SlotAssignOpNode : public ExprNode
{
   ExprNode *objectExpr, *array1Expr, *array2Expr;
   StringTableEntry slotName;
   int op;
   ExprNode *valueExpr;
   bool integer;

public:
   SlotAssignOpNode(ExprNode *obj, StringTableEntry sname, ExprNode *a1, ExprNode *a2, int in_op, ExprNode *ve, bool in_integer)
      { op = in_op; objectExpr = obj; slotName = sname; array1Expr = a1; array2Expr = a2; valueExpr = ve; integer = in_integer;}
   ~SlotAssignOpNode()
      { delete objectExpr; delete array1Expr; delete array2Expr; delete valueExpr; }
   void eval(ExprEvalState *state);
};

class SlotAssignListNode : public ASTNode
{
   StringTableEntry slotName;
   ExprNode *array1Expr, *array2Expr;
   ExprNode *valueExpr;
public:
   SlotAssignListNode(StringTableEntry sname, ExprNode *a1, ExprNode *a2, ExprNode *ve)
      { slotName = sname; array1Expr = a1; array2Expr = a2; valueExpr = ve; }
   ~SlotAssignListNode()
      { delete array1Expr; delete array2Expr; delete valueExpr; }

   void eval(ExprEvalState *state, StringTableEntry objectName);
};

class StructDeclNode : public StmtNode
{
   StringTableEntry className;
   StringTableEntry objectName;
   SlotAssignListNode *assigns;
public:
   int execute(ExprEvalState *state);
   StructDeclNode(StringTableEntry cn, StringTableEntry on, SlotAssignListNode *as)
      { className = cn; objectName = on; assigns = as; }
   ~StructDeclNode()
      { delete assigns; }
};

class ObjectDeclNode : public StmtNode
{
   StringTableEntry className;
   ExprNode *objectNameExpr;
   SlotAssignListNode *slotDecls;
   ObjectDeclNode *subObjects;
public:
   int execute(ExprEvalState *state);
   ObjectDeclNode(StringTableEntry cn, ExprNode *on, SlotAssignListNode *sl, StmtNode *dc)
      { className = cn; objectNameExpr = on; slotDecls = sl; subObjects = (ObjectDeclNode *) dc; }
   ~ObjectDeclNode()
      { delete objectNameExpr; delete slotDecls; delete subObjects; }
};

struct ObjectBlockDecl
{
   SlotAssignListNode *slots;
   StmtNode *decls;
};

struct FunctionDecl
{
   int refCount;
   StringTableEntry fnName;
   VarNode *args;
   StmtNode *stmts;

   FunctionDecl(StringTableEntry in_fnName, VarNode *in_args, StmtNode *in_stmts)
      { fnName = in_fnName; args = in_args; stmts = in_stmts; refCount = 0; }
   ~FunctionDecl()
      { delete args; delete stmts; }
};

class FunctionDeclStmtNode : public StmtNode
{
   FunctionDecl *f;
public:
   int execute(ExprEvalState *state);
   FunctionDeclStmtNode(StringTableEntry name, VarNode *args, StmtNode *stmts)
      { f = new FunctionDecl(name, args, stmts); f->refCount = 1; }
   ~FunctionDeclStmtNode()
      { f->refCount--; if(!f->refCount) delete f; }
};

extern StmtNode *statementList;
extern void createFunction(const char *fnName, VarNode *args, StmtNode *statements);
extern ExprEvalState gEvalState;
extern bool lookupFunction(const char *fnName, VarNode **args, StmtNode **statements);
typedef const char *(*cfunc)(int argc, char **argv);
extern bool lookupCFunction(const char *fnName, cfunc *f);


#endif