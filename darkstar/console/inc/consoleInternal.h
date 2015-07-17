#ifndef _H_CONSOLEINT
#define _H_CONSOLEINT

#include "stringTable.h"

class VarString
{
   unsigned int maxlen;
   char *string;
public:
   const char *getValue() { return string; }
   void setValue(const char *string);
   VarString();
   ~VarString();
};

class ExprEvalState;
struct FunctionDecl;

class Dictionary
{
public:
   struct Entry
   {
      Entry *nextEntry;

      StringTableEntry name;
      int id;
      int privilegeLevel;
      VarString value;
      void *dataPtr;
      int type;
      FunctionDecl *func;

		CMDCallback::Callback callbackFunc;
		CMDCallback* callbackObj;

      Entry(StringTableEntry name);
      ~Entry();
      
      bool setData(const char * value);
   };

private:
   int hashTableSize;
   int entryCount;

   Entry **hashTable;
   ExprEvalState *exprState;

   enum SEARCH_MODE {
      VARIABLES,
      FUNCTIONS,
   };

   const char *getSearchString(SEARCH_MODE mode, const char *entryString);
   void exportEntries(SEARCH_MODE mode, const char *entryString, Vector<const char *> &varName, Vector<const char *> &value);
   void exportEntries(SEARCH_MODE mode, const char *entryString, const char *fileName, bool append);
   void deleteEntries(SEARCH_MODE mode, const char *entryString);

public:
   StringTableEntry scopeName;

   Dictionary(ExprEvalState *state);
   ~Dictionary();
   Entry *lookup(StringTableEntry name);
   Entry *add(StringTableEntry name);
   void remove(Entry *);
   void reset();
   
   void exportVariables(const char *varString, const char *fileName, bool append);
   void exportVariables(const char *varString, Vector<const char *> &varName, Vector<const char *> &value);
   void deleteVariables(const char *varString);

   void exportFunctions(const char *funcString, const char *fileName, bool append);
   void deleteFunctions(const char *funcString);

   void setVariable(StringTableEntry name, const char *value);
   const char *getVariable(StringTableEntry name, bool *valid = NULL);

   void addFunction(FunctionDecl *func);
   void addVariable(int id, const char *name, CMDCallback::Callback, CMDCallback *, int type, void *dataPtr, const char *value);

   bool removeVariable(StringTableEntry name); 
	void addCommand(int id,StringTableEntry name,CMDCallback::Callback,int privilegeLevel = 0);
	void addCommand(int id,StringTableEntry name,CMDCallback*,int privilegeLevel = 0);
	bool removeCommand(StringTableEntry name);

   // return the best tab completion for prevText, with the length
   // of the pre-tab string in baseLen

   const char *tabComplete(const char *prevText, const char *bestMatch, int baseLen, bool);
};

class ExprEvalState
{
public:
   // stuff for doing expression evaluation

   char *baseAddr;
   int curIndex;
   int size;
   int privilegeLevel;
   int frameCount;
   CMDConsole *console;

   ExprEvalState();
   ~ExprEvalState();

   void alloc(int numBytes);
   bool getBoolValue();
   double getFloatValue();
   int getIntValue();
   void setFloatValue(double f);
   void setBoolValue(bool b);
   void setIntValue(int i);
   void advance();

   // stack management
   
   Vector<Dictionary *> frames;
   Dictionary::Entry *allocEntry();
   void freeEntry(Dictionary::Entry *);
   Dictionary::Entry *freeList;
   DataChunker entryPool;

   void lookupVariable(StringTableEntry varName); // puts on top of expr stack
   void assignVariable(StringTableEntry varName); // assigns from top of expr stack
   void pushFrame(const char *frameName);
   void popFrame();
};

class AliasDictionary
{
   struct Entry
   {
      StringTableEntry name;
      char *value;
   };
   Vector<Entry> table;
public:
   ~AliasDictionary();

   const char *lookup(StringTableEntry name);
   void add(StringTableEntry name, const char *value);

   const char *tabComplete(const char *prevText, const char *bestMatch, int baseLen, bool);
};

extern AliasDictionary aliasDictionary;
extern void expandEscape(char *dest, const char *src);

#endif
