#include "console.h"
#include "ast.h"
#include "consoleInternal.h"
#include "gram.h"
#include "stdlib.h"
#include "stdio.h"
#include "tagDictionary.h"

enum { ExecNormal, ExecContinue, ExecBreak, ExecReturn };
extern void createFunction(const char *fnName, StmtNode *statements);

int serrors;

typedef int (*cplFunc)(DWORD *, int &, ExprEvalState *);
cplFunc cplTable[512];

int cplExecuteBlock(DWORD *code, int &eip, ExprEvalState *state)
{
   DWORD tok;
   while((tok = code[eip]) != 0)
   {
      eip++;
      int ret = cplTable[tok](code, eip, state);
      if(ret != ExecNormal)
         return ret;
   }
   return ExecNormal;
}

int cplExecuteBreak(DWORD *code, int &eip, ExprEvalState *state)
{
   code;
   eip;
   state;

   return ExecBreak;
}

int cplExecuteContinue(DWORD *code, int &eip, ExprEvalState *state)
{
   code;
   eip;
   state;

   return ExecContinue;
}

int cplExecuteReturn(DWORD *code, int &eip, ExprEvalState *state)
{
   DWORD tok = code[eip];
   eip++;
   if(tok)
      cplTable[tok](code, eip, state);
   else
      state->baseAddr[state->curIndex] = 0;
   return ExecReturn;
}

int cplExecuteIf(DWORD *code, int &eip, ExprEvalState *state)
{
   DWORD ifBlockEIP = code[eip];
   DWORD elseBlockEIP = code[eip+1];
   DWORD nextStmtEIP = code[eip+2];
   DWORD tok = code[eip+3];
   int ret = ExecNormal;

   eip += 4;
   cplTable[tok](code, eip, state);
   if(state->getBoolValue())
   {
      eip = ifBlockEIP;
      ret = cplExecuteBlock(code, eip, state);
   }
   else if(elseBlockEIP)
   {
      eip = elseBlockEIP;
      ret = cplExecuteBlock(code, eip, state);
   }
   eip = nextStmtEIP;
   return ret;
}

int cplExecuteLoop(DWORD *code, int &eip, ExprEvalState *state)
{
   DWORD nextStmtEIP = code[eip++];
   DWORD endLoopEIP = code[eip++];
   DWORD tok = code[eip++];

   if(tok)
   {
      cplTable[tok](code, eip, state);
   }
   DWORD startEIP = eip;
   for(;;)
   {
      // test expression:
      tok = code[eip++];
      cplTable[tok](code, eip, state);
      if(!state->getBoolValue())
      {
         eip = nextStmtEIP;
         return ExecNormal;
      }
      int ret = cplExecuteBlock(code, eip, state);
      if(ret == ExecNormal || ret == ExecContinue)
      {
         tok = code[endLoopEIP];
         eip = endLoopEIP + 1;
         cplTable[tok](code, eip, state);
         eip = startEIP;
      }
      else if(ret == ExecBreak)
      {
         eip = nextStmtEIP;
         return ExecNormal;
      }
      else
      {
         eip = nextStmtEIP;
         return ExecReturn;
      }
   }
}

void ASTNode::append(ASTNode *next)
{
   ASTNode *walk = this;
   while(walk->next)
      walk = walk->next;
   walk->next = next;
}

int StmtNode::execute(ExprEvalState *state)
{
   state;
   return ExecNormal;
}

int executeBlock(StmtNode *block, ExprEvalState *state)
{
   while(block)
   {
      int ret = block->execute(state);
      if(ret == ExecNormal)
         block = (StmtNode *) block->getNext();
      else
         return ret;
   }
   return ExecNormal;
}

int BreakStmtNode::execute(ExprEvalState *state)
{
   state;
   return ExecBreak;
}

int ContinueStmtNode::execute(ExprEvalState *state)
{
   state;
   return ExecContinue;
}

int ReturnStmtNode::execute(ExprEvalState *state)
{
   if(expr)
      expr->eval(state);
   else
      state->baseAddr[state->curIndex] = 0;

   return ExecReturn;
}

int ExprNode::execute(ExprEvalState *state)
{
   eval(state);
   return ExecNormal;
}

int IfStmtNode::execute(ExprEvalState *state)
{
   if(testExpr->evalFloat(state))
      return executeBlock(ifBlock, state);
   else if(elseBlock)
      return executeBlock(elseBlock, state);
   else
      return ExecNormal;
}

int LoopStmtNode::execute(ExprEvalState *state)
{
   if(initExpr)
      initExpr->eval(state);

   for(;;)
   {
      if(!testExpr->evalFloat(state))
         break;
      int ret = executeBlock(loopBlock, state);
      if(ret == ExecBreak)
         return ExecNormal;
      else if(ret == ExecNormal || ret == ExecContinue)
      {
         if(endLoopExpr)
            endLoopExpr->eval(state);
      }
      else // it was ExecReturn
         return ExecReturn;
   }
   return ExecNormal;
}

int TTagSetStmtNode::execute(ExprEvalState *state)
{
   valueExpr->eval(state);
   int value = atoi(state->baseAddr + state->curIndex);
   
   if(stringExpr)
      stringExpr->eval(state);
   else
      state->baseAddr[state->curIndex] = 0;

   StringTableEntry stringPtr = stringTable.insert(state->baseAddr + state->curIndex, true);

   tagDictionary.addEntry(value, tag, stringPtr);
   return ExecNormal;
}

int TTagExprNode::evalInt(ExprEvalState *)
{
   if(!value)
      value = tagDictionary.defineToId(tag);
   return value;
}

double TTagExprNode::evalFloat(ExprEvalState *state)
{
   return evalInt(state);
}

void TTagExprNode::eval(ExprEvalState *state)
{
   state->alloc(32);
   state->setIntValue(evalInt(state));
}

void TTagDerefNode::eval(ExprEvalState *state)
{
   StringTableEntry str = tagDictionary.idToString(expr->evalInt(state));
   if(!str)
      str = stringTable.insert("<INVALID TAG>");
   state->alloc(strlen(str)+1);
   strcpy(state->baseAddr + state->curIndex, str);
}

int ExprNode::evalInt(ExprEvalState *state)
{
   eval(state);
   if(!stricmp(state->baseAddr + state->curIndex, "True"))
      return 1;
   return atoi(state->baseAddr + state->curIndex);
}

double ExprNode::evalFloat(ExprEvalState *state)
{
   eval(state);
   if(!stricmp(state->baseAddr + state->curIndex, "True"))
      return 1;
   return atof(state->baseAddr + state->curIndex);
}

void ExprNode::eval(ExprEvalState *state)
{
   // do nothing...
   state;
}

bool isFloat(const char *s)
{
   if(!*s)
      return false;

   while(*s)
   {
      char c = *s++;
      if(!( (c >= '0' && c <= '9') || (c == '.') || c == 'e' || c == 'E' || c == '-' || c == '+'))
         return false;
   }
   return true;
}

static double compare(ExprNode *left, ExprNode *right, ExprEvalState *state)
{
   int lFlags = left->getNodeFlags();
   int rFlags = right->getNodeFlags();
   int andFlags = lFlags & rFlags;
   int orFlags = lFlags | rFlags;

   if(andFlags & ExprNode::ExprIsFloat)
      return left->evalFloat(state) - right->evalFloat(state);
   if(andFlags & ExprNode::ExprIsInt)
      return left->evalInt(state) - right->evalInt(state);
   if(orFlags & ( ExprNode::ExprIsInt | ExprNode::ExprIsFloat ) )
      return left->evalFloat(state) - right->evalFloat(state);

   int lStart = state->curIndex;
   left->eval(state);
   state->advance();
   right->eval(state);
   const char *s1 = state->baseAddr + lStart;
   const char *s2 = state->baseAddr + state->curIndex;
   state->curIndex = lStart;

   float arg1 = 0, arg2 = 0;
   bool boolchk = false;

   if(!stricmp(s1, "True"))
   {
      arg1 = 1;
      boolchk = true;
   }
   else if(isFloat(s1))
      arg1 = atof(s1);
   else if(stricmp(s1, "False"))
      return strcmp(s1, s2);
   else
      boolchk = true;

   if(!stricmp(s2, "True"))
   {
      arg2 = 1;
      boolchk = true;
   }
   else if(isFloat(s2))
      arg2 = atof(s2);
   else if(stricmp(s2, "False"))
      return strcmp(s1, s2);
   else
      boolchk = true;

   if(boolchk)
      return (arg1 == 0 && arg2 != 0) || (arg1 != 0 && arg2 == 0);
   else
      return arg1 - arg2;
}

void StrcatExprNode::eval(ExprEvalState *state)
{
   int start = state->curIndex;
   left->eval(state);
   state->curIndex = state->curIndex + strlen(state->baseAddr + state->curIndex);
   right->eval(state);
   state->curIndex = start;
}

void CommaCatExprNode::eval(ExprEvalState *state)
{
   int start = state->curIndex;
   left->eval(state);
   state->curIndex = state->curIndex + strlen(state->baseAddr + state->curIndex) + 1;
   state->baseAddr[state->curIndex - 1] = '_';
   right->eval(state);
   state->curIndex = start;
}

void FloatBinaryExprNode::eval(ExprEvalState *state)
{
   state->alloc(32);
   sprintf(state->baseAddr + state->curIndex, "%g", evalFloat(state));
}

double FloatBinaryExprNode::evalFloat(ExprEvalState *state)
{
   switch(op)
   {
      case '+':
         return left->evalFloat(state) + right->evalFloat(state);
      case '-':
         return left->evalFloat(state) - right->evalFloat(state);
      case '*':
         return left->evalFloat(state) * right->evalFloat(state);
      case '/':
         return left->evalFloat(state) / right->evalFloat(state);
   }
   return 0;
}

int FloatBinaryExprNode::evalInt(ExprEvalState *state)
{
   return (int) evalFloat(state);
}

int IntBinaryExprNode::evalInt(ExprEvalState *state)
{
   switch(op)
   {
      case '%':
      {
         int rv = right->evalInt(state);
         return rv ? left->evalInt(state) % rv : 0;
      }
      case '|':
         return left->evalInt(state) | right->evalInt(state);
      case '&':
         return left->evalInt(state) & right->evalInt(state);
      case '^':
         return left->evalInt(state) ^ right->evalInt(state);
      case opSHL:
         return left->evalInt(state) << right->evalInt(state);
      case opSHR:
         return left->evalInt(state) >> right->evalInt(state);
      case '>':
         return compare(left, right, state) > 0;
      case '<':
         return compare(left, right, state) < 0;
      case opGE:
         return compare(left, right, state) >= 0;
      case opLE:
         return compare(left, right, state) <= 0;
      case opEQ:
         return compare(left, right, state) == 0;
      case opNE:
         return compare(left, right, state) != 0;
      case opOR:
         return left->evalFloat(state) || right->evalFloat(state);
      case opAND:
         return left->evalFloat(state) && right->evalFloat(state);
   }
   return 0;
}

double IntBinaryExprNode::evalFloat(ExprEvalState *state)
{
   return (double) evalInt(state);
}

void IntBinaryExprNode::eval(ExprEvalState *state)
{
   sprintf(state->baseAddr + state->curIndex, "%d", evalInt(state));
}

double FloatUnaryExprNode::evalFloat(ExprEvalState *state)
{
   if(op == '-')
      return -expr->evalFloat(state);
   return 0;
}

int FloatUnaryExprNode::evalInt(ExprEvalState *state)
{
   return (int) evalFloat(state);
}

void FloatUnaryExprNode::eval(ExprEvalState *state)
{
   state->alloc(32);
   sprintf(state->baseAddr + state->curIndex, "%g", evalFloat(state));
}

int IntUnaryExprNode::evalInt(ExprEvalState *state)
{
   switch(op)
   {
      case '!':
         return !expr->evalFloat(state);
      case '~':
         return ~expr->evalInt(state);
   }
   return 0;
}

double IntUnaryExprNode::evalFloat(ExprEvalState *state)
{
   return evalInt(state);
}

void IntUnaryExprNode::eval(ExprEvalState *state)
{
   state->alloc(32);
   sprintf(state->baseAddr + state->curIndex, "%d", evalInt(state));
}

void VarNode::eval(ExprEvalState *state)
{
   if(!arrayIndex)
      state->lookupVariable(varName);
   else
   {
      int curIndex = state->curIndex;
      int vl = strlen(varName);

      state->alloc(vl + 1);
      strcpy(state->baseAddr + state->curIndex, varName);
      state->curIndex += vl;
      arrayIndex->eval(state);
      state->curIndex = curIndex;
      StringTableEntry e = stringTable.insert(state->baseAddr + state->curIndex);
      state->lookupVariable(e);
   }
}

void StrConstNode::eval(ExprEvalState *state)
{
   state->alloc(strlen(str) + 1);
   strcpy(state->baseAddr + state->curIndex, str);
}

void FloatNode::eval(ExprEvalState *state)
{
   state->alloc(32);
   sprintf(state->baseAddr + state->curIndex, "%g", value);
}

int FloatNode::evalInt(ExprEvalState *)
{
   return value;
}

double FloatNode::evalFloat(ExprEvalState *)
{
   return value;
}

void IntNode::eval(ExprEvalState *state)
{
   state->alloc(32);
   sprintf(state->baseAddr + state->curIndex, "%d", value);
}

int IntNode::evalInt(ExprEvalState *)
{
   return value;
}

double IntNode::evalFloat(ExprEvalState *)
{
   return value;
}

void ConstantNode::eval(ExprEvalState *state)
{
   state->alloc(strlen(value) + 1);
   strcpy(state->baseAddr + state->curIndex, value);
}

void AssignExprNode::eval(ExprEvalState *state)
{
   StringTableEntry e = varName;
   if(arrayIndex)
   {
      int curIndex = state->curIndex;
      int vl = strlen(varName);

      state->alloc(vl + 1);
      strcpy(state->baseAddr + state->curIndex, varName);
      state->curIndex += vl;
      arrayIndex->eval(state);
      state->curIndex = curIndex;
      e = stringTable.insert(state->baseAddr + state->curIndex);
   }
   expr->eval(state);
   state->assignVariable(e);
}

void AssignOpExprNode::eval(ExprEvalState *state)
{
   StringTableEntry e = varName;
   if(arrayIndex)
   {
      int curIndex = state->curIndex;
      int vl = strlen(varName);

      state->alloc(vl + 1);
      strcpy(state->baseAddr + state->curIndex, varName);
      state->curIndex += vl;
      arrayIndex->eval(state);
      state->curIndex = curIndex;
      e = stringTable.insert(state->baseAddr + state->curIndex);
   }
   VarNode v(e);
   if(integer)
   {
      IntBinaryExprNode b(op, &v, expr);
      b.eval(state);
      b.left = NULL; // so the destructor won't mess us up.
      b.right = NULL;
   }
   else
   {
      FloatBinaryExprNode b(op, &v, expr);
      b.eval(state);
      b.left = NULL;
      b.right = NULL;
   }
   state->assignVariable(e);
}

FuncCallExprNode::FuncCallExprNode(const char *in_funcName, ExprNode *in_args, ExprNode *in_dot)
{
   funcName = in_funcName;
   args = in_args;

   if(in_dot)
   {
      char buf[256];
      sprintf(buf, "dot_op_%s", funcName);
      funcName = stringTable.insert(buf);
      in_dot->append(in_args);
      args = in_dot;
   }
}

void FuncCallExprNode::eval(ExprEvalState *state)
{
   int argo[512];
   int argtos = 0;
   const char *argv[512];

   VarNode *argList;
   
   // evaluate all the arguments first:
   int argc = 1;
   int stStart = state->curIndex;
   int stargtos = argtos;

   ExprNode *walk = args;
   while(walk)
   {
      walk->eval(state);
      argo[stargtos + argc] = state->curIndex;
      argc++;
      argtos = stargtos + argc;
      state->advance();
      walk = (ExprNode *) walk->getNext();
   }
   argtos = stargtos;

   Dictionary::Entry *ent;
   ent = state->frames[0]->lookup(funcName);
   if(!ent)
   {
      state->curIndex = stStart;
      state->alloc(6);
      strcpy(state->baseAddr + state->curIndex, "False");
      state->console->printf("%s: Unknown command.", funcName);
   }
   else if(ent->func)
   {
      FunctionDecl *f = ent->func;

      int argNum = 1;
      state->pushFrame(funcName);
      argList = f->args;

      f->refCount++;
      while(argList)
      {
         if(argNum >= argc)
            break;
         state->curIndex = argo[argtos + argNum];
         argNum++;
         state->assignVariable(argList->getVarName());
         argList = (VarNode *) argList->getNext();
      }
      state->curIndex = stStart;

      executeBlock(f->stmts, state);
      state->popFrame();
      f->refCount--;
      if(!f->refCount)
         delete f;
   }
   else if(ent->callbackFunc || ent->callbackObj)
   {
      int i;
      argv[argtos] = ent->name;
      for(i = 1; i < argc; i++)
         argv[argtos + i] = state->baseAddr + argo[argtos + i];
      const char *ret;
      if(ent->callbackFunc)
         ret = ent->callbackFunc(state->console, ent->id, argc, (const char **) argv + argtos);
      else
         ret = ent->callbackObj->consoleCallback(state->console, ent->id, argc, (const char **) argv + argtos);
      state->curIndex = stStart;
      if(!ret)
         ret = "False";
      if(ret)
      {
         state->alloc(strlen(ret) + 1);
         strcpy(state->baseAddr + state->curIndex, ret);
      }
   }
}

int StructDeclNode::execute(ExprEvalState *state)
{
   state;
   Console->getDataManager()->declareDataBlock(className, objectName);
   SlotAssignListNode *walk = assigns;
   while(walk)
   {
      walk->eval(state, objectName);
      walk = (SlotAssignListNode *) walk->getNext();
   }
   return ExecNormal;
}

void SlotAccessNode::eval(ExprEvalState *state)
{
   objectExpr->eval(state);
   StringTableEntry obj = stringTable.insert(state->baseAddr + state->curIndex);
   const char *a1 = NULL;
   int stStart = state->curIndex;
   if(array1Expr)
   {
      array1Expr->eval(state);
      a1 = state->baseAddr + state->curIndex;
      state->advance();
   }
   const char *ret = Console->getDataManager()->getDataField(obj, slotName, a1);
   if(!ret)
      ret = "";
   state->curIndex = stStart;
   state->alloc(strlen(ret) + 1);
   strcpy(state->baseAddr + state->curIndex, ret);
}

void SlotAssignNode::eval(ExprEvalState *state)
{
   const char *a1 = NULL;
   int stStart = state->curIndex;
   objectExpr->eval(state);
   StringTableEntry obj = stringTable.insert(state->baseAddr + state->curIndex);

   if(array1Expr)
   {
      array1Expr->eval(state);
      a1 = state->baseAddr + state->curIndex;
      state->advance();
   }
   ExprNode *walk = valueExpr;
   const char *argv[128];
   
   // evaluate all the arguments first:
   int argc = 0;

   while(walk)
   {
      walk->eval(state);
      argv[argc++] = state->baseAddr + state->curIndex;
      state->advance();
      walk = (ExprNode *) walk->getNext();
   }
   Console->getDataManager()->setDataField(obj, slotName, a1, argc, argv);
   state->curIndex = stStart;
}

void SlotAssignOpNode::eval(ExprEvalState *state)
{
   objectExpr->eval(state);
   StringTableEntry obj = stringTable.insert(state->baseAddr + state->curIndex);
   const char *a1 = NULL;
   int stStart = state->curIndex;

   if(array1Expr)
   {
      array1Expr->eval(state);
      a1 = state->baseAddr + state->curIndex;
      state->advance();                     
   }
   const char *val = Console->getDataManager()->getDataField(obj, slotName, a1);
   if(!val)
      val = "0";
   if(integer)
   {
      IntNode i(atoi(val));
      IntBinaryExprNode b(op, &i, valueExpr);
      b.eval(state);
      b.left = NULL;
      b.right = NULL;
   }
   else
   {
      FloatNode f(atof(val));
      FloatBinaryExprNode b(op, &f, valueExpr);
      b.eval(state);
      b.left = NULL;
      b.right = NULL;
   }
   const char *argv0 = state->baseAddr + state->curIndex;
   Console->getDataManager()->setDataField(obj, slotName, a1, 1, &argv0);
   state->curIndex = stStart;
}

void SlotAssignListNode::eval(ExprEvalState *state, StringTableEntry objectName)
{
   const char *a1 = NULL;
   int stStart = state->curIndex;
      
   if(array1Expr)
   {
      array1Expr->eval(state);
      a1 = state->baseAddr + state->curIndex;
      state->advance();
   }
   ExprNode *walk = valueExpr;
   const char *argv[128];
   
   // evaluate all the arguments first:
   int argc = 0;

   while(walk)
   {
      walk->eval(state);
      argv[argc++] = state->baseAddr + state->curIndex;
      state->advance();
      walk = (ExprNode *) walk->getNext();
   }
   Console->getDataManager()->setDataField(objectName, slotName, a1, argc, argv);
   state->curIndex = stStart;
}

int ObjectDeclNode::execute(ExprEvalState *state)
{
   int stOffset = state->curIndex;
   char *objName = NULL;
   if(objectNameExpr)
   {
      objectNameExpr->eval(state);
      objName = state->baseAddr + state->curIndex;
      state->advance();
   }
   Console->getDataManager()->beginObject(className, objName);
   
   for(SlotAssignListNode *walk = slotDecls; walk; walk = (SlotAssignListNode *) walk->getNext())
      walk->eval(state, NULL);

   for(ObjectDeclNode *ow = subObjects; ow; ow = (ObjectDeclNode *) ow->getNext())
      ow->execute(state);

   state->curIndex = stOffset;
   Console->getDataManager()->endObject();
   return ExecNormal;
}


int FunctionDeclStmtNode::execute(ExprEvalState *state)
{
   state->frames[0]->addFunction(f);
   return ExecNormal;
}