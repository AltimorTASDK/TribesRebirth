#ifndef _H_CONSOLE
#define _H_CONSOLE

#include "tVector.h"

class CMDConsole;
class ResourceManager;

//---------------------------------------------------------------
struct RawDataRes
{
   int size;
   BYTE *data;
   RawDataRes() { size = 0; data = NULL; }
   ~RawDataRes() { delete[] data; }
};

typedef RawDataRes ConsoleScript;


//--------------------------------------------------------------------------- 
typedef const char *StringTableEntry;

class CMDCallback {
public:
	typedef const char *(*Callback)(CMDConsole*,int id,int argc, const char *argv[]);
	virtual const char *consoleCallback(CMDConsole*,int id,int argc, const char *argv[]) = 0;
};

class CMDConsumer {
public:
   virtual void consoleCallback(CMDConsole *, const char *consoleLine) = 0;
};

class CMDDataManager {
public:
   virtual void declareDataBlock(StringTableEntry className, StringTableEntry objectName);
   virtual const char *getDataField(StringTableEntry objectName, StringTableEntry slotName, const char *array);
   virtual void setDataField(StringTableEntry objectName, StringTableEntry slotName, const char *array, int argc, const char **argv);
   virtual void beginObject(StringTableEntry className, const char *objectName);
   virtual void endObject();
};

//--------------------------------------------------------------------------- 

class StmtNode;
class VarNode;

class CMDConsole : public CMDCallback
{
public:
	enum Constants {
		MaxArguments = 128,
		MaxLineLength = 512,
		MaxPrintfLength = 512,
		MaxDisplayLines = 25,
		MaxHistory = 25,
		MaxPromptLength = 10,
		MaxAliasRecursion = 20,
		MinArgumentsPushed = 10,
		//
		ConsoleVariable = -1,
		VariablePrefix = '$',
		DefaultCaret = '|',
	};
	enum Type {
		Default,
		Bool,
		Int,
		Float,
		Double,
		Unit,
		Point3F,
	};

   static bool updateMetrics;
   static int printLevel;
   char *logFileName;
   bool newLogFile;

private:
   CMDDataManager defaultDataManager, *dataManager;
   ResourceManager *resManager;
   Vector<CMDConsumer *> consumerList;
	void log(const char *string);
   void dbprint(int level, const char* fmt, va_list argptr);

public:
	static CMDConsole* getLocked();
   CMDConsole();
   ~CMDConsole();
   void setVariable(const char *name, const char *value);
	const char *consoleCallback(CMDConsole*,int id,int argc, const char *argv[]);

   void addConsumer(CMDConsumer *consumer);
   void removeConsumer(CMDConsumer *consumer);
   void addDataManager(CMDDataManager *dm);
   void removeDataManager(CMDDataManager *dm);

   void addFunction(StringTableEntry name, VarNode *vars, StmtNode *stmts);

   bool addVariable(int id, const char *name, CMDCallback::Callback, const char *value = 0);
   bool addVariable(int id, const char *name, CMDCallback *, const char *value = 0);
   bool addVariable(int id, const char *name, Type, void *);
   bool removeVariable(const char *name); 

	const char* getVariable(const char* name);
   void setResourceManager(ResourceManager *rm) { resManager = rm; }
   ResourceManager *getResourceManager() { return resManager; }

	void addCommand(int id,const char *name,CMDCallback::Callback,int privilegeLevel = 0);
	void addCommand(int id,const char *name,CMDCallback*,int privilegeLevel = 0);
	bool removeCommand(const char *name);
	void __cdecl printf(const char *_format, ...);
	void __cdecl dbprintf(int level, const char *_format, ...);

	const char *execute(int argc, const char* argv[], int privelegeLevel=0);
   const char *executef(int argc, ...); // first param is funcName, remaining params are args
                              // only pass strings to this function!!
	const char *evaluate(const char* string, bool echo = true, const char *fileName = NULL, int privilegeLevel=0);
	const char *evaluatef(const char* string, ...);

   bool isFunction(const char *fn);

	static bool getBool(const char*);
	bool setBoolVariable(const char* name,bool var);
	bool getBoolVariable(const char* name,bool def = false);
	int setIntVariable(const char* name,int var);
	int getIntVariable(const char* name,int def = 0);
	float setFloatVariable(const char* name,float var);
	float getFloatVariable(const char* name,float def = .0f);

   const char *tabComplete(const char *prevText, int baseLen, bool);

   void exportVariables(const char *varString, Vector<const char *> &varName, Vector<const char *> &value);

   Vector<const char *> *getLog();
   CMDDataManager *getDataManager() { return dataManager; }
};

extern CMDConsole *Console;

#endif