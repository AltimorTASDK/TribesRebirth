#include "console.h"
#include "ast.h"
#include "talgorithm.h"
#include "stdio.h"
#include "stdlib.h"
#include "resManager.h"
#include "m_qsort.h"
#include "consoleInternal.h"
#include "streamio.h"
#include "filstrm.h"

AliasDictionary aliasDictionary;
ExprEvalState gEvalState;
StmtNode *statementList;
extern void SetScanBuffer(const char *sb, const char *fn);
extern int CMDparse();
extern void CMD_reset();

static char scratchBuffer[1024];
static DataChunker consoleLogChunker;
static Vector<const char *> consoleLog;
bool CMDConsole::updateMetrics = false;
static bool logBufferEnabled=true;
int CMDConsole::printLevel = 1;
static FileRWStream consoleLogFile(false);
static const char *defLogFileName = "console.log";
static int consoleLogMode = 0; // default to no logging

static const char *c_cls(CMDConsole *, int, int , const char **)
{
   consoleLogChunker.freeBlocks();
   consoleLog.setSize(0);
   return "True";
};

static const char * prependDollar ( const char * name )
{
   if(name[0] != '$'){
      int   len = strlen(name);
      AssertFatal(len < sizeof(scratchBuffer)-2, "CONSOLE: name too long");
      scratchBuffer[0] = '$';
      memcpy(scratchBuffer + 1, name, len + 1);
      name = scratchBuffer;
   }
   return name;
}

extern int executeBlock(StmtNode *block, ExprEvalState *state);

enum Commands
{
   SETLOGMODE  = 1,
};    // CANNOT exceed 99 Commands, SimGame starts at 100!  


class ResourceTypeConsoleScript: public ResourceType
{
  public:   
   ResourceTypeConsoleScript(const char *ext = ".cs") :
	ResourceType( ResourceType::typeof(ext) )  { }
   void* construct(StreamIO *stream,int);
   void destruct(void *p);
};

static ResourceTypeConsoleScript			_rescon(".cs");

void* ResourceTypeConsoleScript::construct(StreamIO *stream, int size)
{
   ConsoleScript *ret = new ConsoleScript;
   ret->data = new BYTE[size+1];
   stream->read(size, ret->data);
   ret->data[size] = 0; // null terminate the buffer
   ret->size = size;
   return (void*) ret;
}

void ResourceTypeConsoleScript::destruct(void *p)
{
   delete (ConsoleScript *) p;
}


CMDConsole* Console = NULL;

static const char *c_sqrt(CMDConsole *, int, int argc, const char **argv)
{
   if(argc < 2)
      return "0";
   sprintf(scratchBuffer, "%g", sqrt(atof(argv[1])));
   return scratchBuffer;
}

static const char *c_floor(CMDConsole *, int, int argc, const char **argv)
{
   if(argc < 2)
      return "0";
   sprintf(scratchBuffer, "%d", int(atof(argv[1])));
   return scratchBuffer;
}

static const char *c_echo(CMDConsole *console, int, int argc, const char **argv)
{
   scratchBuffer[0] = 0;
   for(int i = 1; i < argc; i++)
      strcat(scratchBuffer, argv[i]);

   console->printf("%s", scratchBuffer);
   return "True";
}

static const char *c_dbecho(CMDConsole *console, int, int argc, const char **argv)
{
   scratchBuffer[0] = 0;
   if (argc > 1)
   {
      int dblevel =  atoi(argv[1]);
      argc--; argv++;
      for(int i = 1; i < argc; i++)
         strcat(scratchBuffer, argv[i]);

      console->dbprintf(dblevel, "%s", scratchBuffer);
   }
   return "True";
}

static const char *c_strcat(CMDConsole *, int, int argc, const char **argv)
{
   scratchBuffer[0] = 0;
   for(int i = 1; i < argc; i++)
      strcat(scratchBuffer, argv[i]);

   return scratchBuffer;
}

static const char *c_quit(CMDConsole *, int, int, const char **)
{
   PostQuitMessage(0);
   return NULL;
}

static const char *c_exec(CMDConsole *, int, int argc, const char **argv)
{
   if(argc != 2)
   {
      Console->printf("exec(filename);");
      return "False";
   }
   char scriptFileName[512];
   strcpy(scriptFileName, argv[1]);
   if(!strstr(scriptFileName, ".cs") && 
      !strstr(scriptFileName, ".mis") &&
      !strstr(scriptFileName, ".dsc") &&
      !strstr(scriptFileName, ".mac"))
      strcat(scriptFileName, ".cs");

   Resource<ConsoleScript> cs;
   cs = Console->getResourceManager()->load(scriptFileName);
   if(!bool(cs) || cs->size == 0)
   {
      Console->printf("exec: invalid script file %s.", scriptFileName);
      return "False";
   }
   Console->printf("Executing %s.", scriptFileName);
   Console->evaluate((const char *)cs->data, false, scriptFileName);
   cs.purge();
   return "True";
}

static const char *c_eval(CMDConsole *, int, int argc, const char **argv)
{
   if(argc != 2)
   {
      Console->printf("eval(consoleString);");
      return "False";
   }
   return Console->evaluate(argv[1], false, NULL, gEvalState.privilegeLevel);
}

static const char *c_export(CMDConsole *, int, int argc, const char **argv)
{
   if(argc < 2)
   {
      Console->printf("export(searchString, [fileName], [append]);");
      return "False";
   }
   
   const char *fileName =(argc >= 3) ? argv[2] : NULL;
   bool append = (argc == 4) ? CMDConsole::getBool(argv[3]) : false;
   
   gEvalState.frames[0]->exportVariables(argv[1], fileName, append);
   return "True";
}

static const char *c_deleteVariables(CMDConsole *, int, int argc, const char **argv)
{
   if(argc < 2)
   {
      Console->printf("deleteVariables(<wild card function variable string>);");
      return "False";
   }

   gEvalState.frames[0]->deleteVariables(argv[1]);
   return "True";
}

static const char *c_exportFunctions(CMDConsole *, int, int argc, const char **argv)
{
   if(argc < 2)
   {
      Console->printf("exportFunctions(searchString, [fileName], [append]);");
      return "False";
   }
   const char *fileName =(argc >= 3) ? argv[2] : NULL;
   bool append = (argc == 4) ? CMDConsole::getBool(argv[3]) : false;
   
   gEvalState.frames[0]->exportFunctions(argv[1], fileName, append);
   return "True";
}

static const char *c_deleteFunctions(CMDConsole *, int, int argc, const char **argv)
{
   if(argc < 2)
   {
      Console->printf("deleteFunctions(<wild card function name string>);");
      return "False";
   }
   
   gEvalState.frames[0]->deleteFunctions(argv[1]);
   return "True";
}

static const char *c_trace(CMDConsole *, int, int, const char **)
{
   char buf[2048];
   strcpy(buf, "TRACE: ");

   for(int i = 1; i < gEvalState.frameCount; i++)
   {
      strcat(buf, gEvalState.frames[i]->scopeName);
      if(i != gEvalState.frameCount - 1)
         strcat(buf, "->");
   }
   Console->printf("%s", buf);
   return "True";
}

static const char *c_debug(CMDConsole *, int, int, const char **)
{
	DebugBreak();
   return "True";
}

CMDConsole::CMDConsole()
{
   Console = this;
   gEvalState.console = this;
   logFileName = NULL;
   newLogFile  = true;

   // Commands
   addCommand(0, "cls",  c_cls);
   addCommand(0, "floor",  c_floor);
   addCommand(0, "sqrt",   c_sqrt);
   addCommand(0, "echo",   c_echo);
   addCommand(0, "dbecho", c_dbecho);
   addCommand(0, "strcat", c_strcat);
   addCommand(0, "quit",   c_quit);
   addCommand(0, "export", c_export);
   addCommand(0, "deleteVariables", c_deleteVariables);
   addCommand(0, "exportFunctions", c_exportFunctions);
   addCommand(0, "deleteFunctions", c_deleteFunctions);
   addCommand(0, "exec",   c_exec);
   addCommand(0, "eval",   c_eval);
   addCommand(0, "debug",  c_debug);
   addCommand(0, "trace",  c_trace);

   // Variables
   addVariable(0, "Console::logBufferEnabled", CMDConsole::Bool, &logBufferEnabled);
   addVariable(0, "Console::printLevel", CMDConsole::Int, &CMDConsole::printLevel);
   addVariable(0, "Console::updateMetrics", CMDConsole::Bool, &CMDConsole::updateMetrics);
   addVariable(SETLOGMODE, "Console::logMode", this);

   dataManager = &defaultDataManager;
}

CMDConsole::~CMDConsole()
{
   consoleLogFile.close();
}

CMDConsole* CMDConsole::getLocked()
{
	return Console;
}	

void CMDConsole::addDataManager(CMDDataManager *dm)
{
   dataManager = dm;
}

void CMDConsole::removeDataManager(CMDDataManager *dm)
{
   if(dataManager == dm)
      dataManager = &defaultDataManager;
}

Vector<const char *> *CMDConsole::getLog()
{
   return &consoleLog;
}

const char *CMDConsole::tabComplete(const char *prevText, int baseLen, bool fForward)
{
   const char *bestMatch = NULL;
   bestMatch = aliasDictionary.tabComplete(prevText, bestMatch, baseLen, fForward);
   bestMatch = gEvalState.frames[0]->tabComplete(prevText, bestMatch, baseLen, fForward);

   return bestMatch;
}

void CMDConsole::exportVariables(const char *varString, Vector<const char *> &varName, Vector<const char *> &value)
{
   gEvalState.frames[0]->exportVariables(varString, varName, value);
}   

//------------------------------------------------------------------------------
void CMDConsole::log(const char *string)
{
   if(!consoleLogMode)
      return;

   if(consoleLogMode == 1)
      consoleLogFile.open(defLogFileName);

   if(consoleLogFile.getStatus() == STRM_OK)
   {
      consoleLogFile.setPosition(consoleLogFile.getSize());
      if (newLogFile)
      {
         SYSTEMTIME sysTime;
         GetLocalTime( &sysTime );

         char buffer[128];
         sprintf(buffer, "-------------------------- %d/%d/%d -- %02d:%02d:%02d -----\r\n",   
               sysTime.wMonth,
               sysTime.wDay,
               sysTime.wYear,
               sysTime.wHour,
               sysTime.wMinute,
               sysTime.wSecond);
         consoleLogFile.write(strlen(buffer), buffer);
         newLogFile = false;
      }
      consoleLogFile.write(strlen(string), string);
      consoleLogFile.write(2, "\r\n");
   }
   if(consoleLogMode == 1)
      consoleLogFile.close();
}

//------------------------------------------------------------------------------
void __cdecl CMDConsole::printf(const char* fmt,...)
{
   if(!printLevel)
      return;

   va_list argptr;
   va_start(argptr, fmt);
   dbprint(1, fmt, argptr);
   va_end(argptr);
}

void __cdecl CMDConsole::dbprintf(int level, const char* fmt,...)
{
   if (!printLevel || printLevel < level)
      return;

   va_list argptr;
   va_start(argptr, fmt);
   dbprint(level, fmt, argptr);
   va_end(argptr);
}

void CMDConsole::dbprint(int, const char* fmt, va_list argptr)
{
	char buffer[1024];
   vsprintf(buffer, fmt, argptr);

   Vector<CMDConsumer *>::iterator i;
   for(i = consumerList.begin(); i != consumerList.end(); i++)
      (*i)->consoleCallback(this, buffer);

   log(buffer);
   if(logBufferEnabled)
   {
      char *string = (char *) consoleLogChunker.alloc(strlen(buffer) + 1);
      strcpy(string, buffer);
      consoleLog.push_back(string);
   }
}   

//--------------------------------------------------------------------------- 

void CMDConsole::setVariable(const char *name, const char *value)
{
   name = prependDollar(name);
   gEvalState.frames[0]->setVariable(stringTable.insert(name), value);
}

bool CMDConsole::setBoolVariable(const char *varName, bool value)
{
   setVariable(varName, value ? "True" : "False");
   return value;
}

int CMDConsole::setIntVariable(const char *varName, int value)
{
   char scratchBuffer[32];
   sprintf(scratchBuffer, "%d", value);
   setVariable(varName, scratchBuffer);
   return value;
}

float CMDConsole::setFloatVariable(const char *varName, float value)
{
   char scratchBuffer[32];
   sprintf(scratchBuffer, "%f", value);
   setVariable(varName, scratchBuffer);
   return value;
}

//--------------------------------------------------------------------------- 
void CMDConsole::addConsumer(CMDConsumer *consumer)
{
   consumerList.push_back(consumer);
}

const char *CMDConsole::getVariable(const char *name)
{
   name = prependDollar(name);
   return gEvalState.frames[0]->getVariable(stringTable.insert(name));
}

bool CMDConsole::getBoolVariable(const char *varName, bool def)
{
   const char *value = getVariable(varName);
   return *value ? getBool(value) : def;
}

int CMDConsole::getIntVariable(const char *varName, int def)
{
   const char *value = getVariable(varName);
   return *value ? atoi(value) : def;
}

float CMDConsole::getFloatVariable(const char *varName, float def)
{
   const char *value = getVariable(varName);
   return *value ? atof(value) : def;
}

//--------------------------------------------------------------------------- 

bool CMDConsole::addVariable(int id, const char *name, CMDCallback::Callback cb, const char *value)
{
   gEvalState.frames[0]->addVariable(id, name, cb, NULL, Default, NULL, value);
   return true;
}

bool CMDConsole::addVariable(int id, const char *name, CMDCallback *cb, const char *value)
{
   gEvalState.frames[0]->addVariable(id, name, NULL, cb, Default, NULL, value);
   return true;
}

bool CMDConsole::addVariable(int id, const char *name, Type t, void *dp)
{
   gEvalState.frames[0]->addVariable(id, name, NULL, NULL, t, dp, NULL);
   return true;
}

bool CMDConsole::removeVariable(const char *name)
{
   name = stringTable.lookup(prependDollar(name));
   return name!=0 && gEvalState.frames[0]->removeVariable(name);
}

//--------------------------------------------------------------------------- 

void CMDConsole::addFunction(StringTableEntry name, VarNode *args, StmtNode *stmts)
{
   name;
   args;
   stmts;
}

void CMDConsole::addCommand(int id,const char *name,CMDCallback::Callback cb,int privilegeLevel)
{
   gEvalState.frames[0]->addCommand(id, stringTable.insert(name), cb, privilegeLevel);
}

void CMDConsole::addCommand(int id,const char *name,CMDCallback* cb,int privilegeLevel)
{
   gEvalState.frames[0]->addCommand(id, stringTable.insert(name), cb, privilegeLevel);
}

const char *CMDConsole::evaluate(const char* string, bool echo, const char *fileName, int privilegeLevel)
{
   if (echo)
   {
      printf("%s%s", getVariable( "$Console::Prompt" ), string);
   }

   statementList = NULL;
   if(!fileName)
      fileName = string;

   SetScanBuffer(string, fileName);
   gEvalState.privilegeLevel = privilegeLevel;
   CMD_reset();
   CMDparse();
   StmtNode *stmts = statementList;
   StmtNode *walk = statementList;
   statementList = NULL;

   while(walk)
   {
      walk->execute(&gEvalState);
      walk = (StmtNode *) walk->getNext();
   }
   delete stmts;
   return gEvalState.baseAddr + gEvalState.curIndex;
}

//------------------------------------------------------------------------------
const char *CMDConsole::evaluatef(const char* string, ...)
{
   char buffer[512];
   va_list args;
   va_start(args, string);
   vsprintf(buffer, string, args);
   return( evaluate(buffer, false, 0) );
}   

//------------------------------------------------------------------------------
const char *CMDConsole::executef(int argc, ...)
{
   const char *argv[128];

   va_list args;
   va_start(args, argc);
   for(int i = 0; i < argc; i++)
      argv[i] = va_arg(args, const char *);
   va_end(args);
   return execute(argc, argv);
}   

//------------------------------------------------------------------------------
const char *CMDConsole::execute(int argc, const char **argv, int privilegeLevel)
{
   gEvalState.privilegeLevel = privilegeLevel;
   Dictionary::Entry *ent;
   const char *funcName = stringTable.insert(argv[0]);
   ent = gEvalState.frames[0]->lookup(funcName);

   if(!ent)
   {
      gEvalState.alloc(6);
      strcpy(gEvalState.baseAddr + gEvalState.curIndex, "False");
      printf("%s: Unknown command.", argv[0]);
   }
   else if(ent->func)
   {
      FunctionDecl *f = ent->func;
      VarNode *argList;
      int argNum = 1;

      gEvalState.pushFrame(funcName);
      argList = f->args;

      f->refCount++;
      while(argList)
      {
         if(argNum >= argc)
            break;
         gEvalState.frames[gEvalState.frameCount-1]->setVariable(argList->getVarName(), argv[argNum]);
         argList = (VarNode *) argList->getNext();
         argNum++;
      }
      executeBlock(ent->func->stmts, &gEvalState);
      gEvalState.popFrame();
      f->refCount--;
      if(!f->refCount)
         delete f;
   }
   else if(ent->callbackFunc || ent->callbackObj)
   {
      const char *ret;
      if(ent->callbackFunc)
         ret = ent->callbackFunc(this, ent->id, argc, (const char **) argv);
      else
         ret = ent->callbackObj->consoleCallback(this, ent->id, argc, (const char **) argv);
   
      if(!ret)
         ret = "True";

      if(ret)
      {
         gEvalState.alloc(strlen(ret) + 1);
         strcpy(gEvalState.baseAddr + gEvalState.curIndex, ret);
      }
   }
   return gEvalState.baseAddr + gEvalState.curIndex;
}

//------------------------------------------------------------------------------
bool CMDConsole::isFunction(const char *fn)
{
   gEvalState.privilegeLevel = 0;
   const char *string = stringTable.lookup(fn);
   if(!string)
      return false;
   else
      return (gEvalState.frames[0]->lookup(string) != NULL);
}

//------------------------------------------------------------------------------
const char *CMDConsole::consoleCallback(CMDConsole*,int id,int argc, const char *argv[])
{
   switch (id)
   {
      case SETLOGMODE:
         if (argc == 2)
         {
            int newMode = atoi(argv[1]);
            if(newMode != consoleLogMode)
            {
               if(newMode && !consoleLogMode)
                  newLogFile = true;
               if(consoleLogMode == 2)
                  consoleLogFile.close();
               else if(newMode == 2)
                  consoleLogFile.open(defLogFileName);
               consoleLogMode = newMode;
            }
         }
         break;
   }
   return NULL;
}


