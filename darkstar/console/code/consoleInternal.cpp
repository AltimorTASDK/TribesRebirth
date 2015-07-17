#include "console.h"
#include "ast.h"
#include "talgorithm.h"
#include "stdio.h"
#include "stdlib.h"
#include "resManager.h"
#include "m_qsort.h"
#include "consoleInternal.h"
#include "streamio.h"
#include "volstrm.h"

#define ST_INIT_SIZE 15

static char scratchBuffer[1024];

void expandEscape(char *dest, const char *src)
{
   unsigned char c;
   while((c = (unsigned char) *src++) != 0)
   {
      if(c == '\"')
      {
         *dest++ = '\\';
         *dest++ = '\"';
      }
      else if(c == '\\')
      {
         *dest++ = '\\';
         *dest++ = '\\';
      }
      else if(c == '\r')
      {
         *dest++ = '\\';
         *dest++ = 'r';
      }
      else if(c == '\n')
      {
         *dest++ = '\\';
         *dest++ = 'n';
      }
      else if(c == '\t')
      {
         *dest++ = '\\';
         *dest++ = 't';
      }
      else if((c < 32) || (c > 127))
      {
         *dest++ = '\\';
         *dest++ = 'x';
         int dig1 = c >> 4;
         int dig2 = c & 0xf;
         if(dig1 < 10)
            dig1 += '0';
         else
            dig1 += 'A';
         if(dig2 < 10)
            dig2 += '0';
         else
            dig2 += 'A';
         *dest++ = dig1;
         *dest++ = dig2;
      }
      else
         *dest++ = c;
   }
   *dest = '\0';
}   

bool CMDConsole::getBool(const char *string)
{
   if(string[0] == 'T' || string[0] == 't')
      return true;
   if(string[0] == 'F' || string[0] == 'f')
      return false;
   else return (atof(string) != 0);
}


bool canTabComplete(const char *prevText, const char *bestMatch, 
               const char *newText, int baseLen, bool fForward)
{
   // test if it matches the first baseLen chars:
   if(strnicmp(newText, prevText, baseLen))
      return false;

   if (fForward)
   {
      if(!bestMatch)
         return stricmp(newText, prevText) > 0;
      else
         return (stricmp(newText, prevText) > 0) && 
                (stricmp(newText, bestMatch) < 0);
   }
   else
   {
      if (strlen(prevText) == (DWORD) baseLen)
      {
         // look for the 'worst match'
         if(!bestMatch)
            return stricmp(newText, prevText) > 0;
         else
            return stricmp(newText, bestMatch) > 0;
      }
      else
      {
         if (!bestMatch)
            return (stricmp(newText, prevText)  < 0);
         else
            return (stricmp(newText, prevText)  < 0) &&
                   (stricmp(newText, bestMatch) > 0);
      }
   }
}
void VarString::setValue(const char *in_string)
{
   DWORD len = strlen(in_string) + 1;
   if(len > maxlen)
   {
      delete[] string;
      maxlen = len;
      string = new char[len];

   }
   strcpy(string, in_string);
}

VarString::VarString()
{
   maxlen = 0;
   string = NULL;
}

VarString::~VarString()
{
   delete[] string;
}

//---------------------------------------------------------------
//
// Dictionary functions
//
//---------------------------------------------------------------

const char *AliasDictionary::tabComplete(const char *prevText, const char *bestMatch, int baseLen, bool fForward)
{
   Vector<Entry>::iterator i;

   for(i = table.begin(); i != table.end(); i++)
   {
      if(canTabComplete(prevText, bestMatch, i->name, baseLen, fForward))
         bestMatch = i->name;
   }
   return bestMatch;
}

const char *AliasDictionary::lookup(StringTableEntry name)
{
   Vector<Entry>::iterator i;
   for(i = table.begin(); i != table.end(); i++)
   {
      if(i->name == name)
         return i->value;
   }
   return NULL;
}

void AliasDictionary::add(StringTableEntry name, const char *value)
{
   Entry e;
   Vector<Entry>::iterator i;
   for(i = table.begin(); i != table.end(); i++)
   {
      if(i->name == name)
      {
         delete[] i->value;
         i->value = new char[strlen(value)+1];
         strcpy(i->value, value);
         return;
      }
   }
   e.name = name;
   e.value = new char[strlen(value)+1];
   strcpy(e.value, value);
   table.push_back(e);
}

AliasDictionary::~AliasDictionary()
{
   for(Vector<Entry>::iterator i = table.begin(); i != table.end(); i++)
      delete[] i->value;   
}

static int _USERENTRY varCompare(const void* a,const void* b)
{
	return stricmp( *((StringTableEntry *) a),
		*((StringTableEntry *) b));
}

const char *Dictionary::getSearchString(SEARCH_MODE mode, const char *entryString)
{
   AssertFatal(entryString, "invalid param");
   AssertFatal(strlen(entryString) < 255, "invalid str len");

   static char searchStrBuffer[256];
   
   if (mode == VARIABLES)
   {
      // prepend dollar
      if (entryString[0] != '$')
      {
         searchStrBuffer[0] = '$';
         strcpy(&searchStrBuffer[1], entryString);    
      }
      else 
         strcpy(searchStrBuffer, entryString);
   }
   else
   {
      AssertFatal(entryString[0] != '$', "attempting to search functions using search string for variables");
      strcpy(searchStrBuffer, entryString);
   }
   return searchStrBuffer;
}   

void Dictionary::exportEntries(SEARCH_MODE mode, const char *entryString, Vector<const char *> &varName, Vector<const char *> &value)
{
   const char *searchStr = getSearchString(mode, entryString);
   Vector<StringTableEntry> sortList;

   for(int i = 0; i < hashTableSize;i ++)
   {
      Entry *walk = hashTable[i];
      while(walk)
      {
         if(FindMatch::isMatch((char *) searchStr, (char *) walk->name))
         {
            // if mode and entry agree, add to list,
            // This check is necessary because the searchString 
            // might start with ? or *
            bool entryIsVariable = (walk->name[0] == '$');
            bool modeIsVariable = (mode == VARIABLES);
            if (entryIsVariable == modeIsVariable) 
               sortList.push_back(walk->name);
         }
         walk = walk->nextEntry;
      }
   }

   if(!sortList.size())
      return;

   m_qsort((void *) &sortList[0], sortList.size(), sizeof(StringTableEntry),
      varCompare);
   
   Vector<StringTableEntry>::iterator s;
   for(s = sortList.begin(); s != sortList.end(); s++)
   {
      varName.push_back(*s);
      value.push_back((mode == VARIABLES) ? getVariable(*s) : "");
   }
}   

void Dictionary::exportEntries(SEARCH_MODE mode, const char *entryString, const char *fileName, bool append)
{
   Vector<const char *> varNameList;
   Vector<const char *> varValueList;

   exportEntries(mode, entryString, varNameList, varValueList);
   if (!varNameList.size())
      return;
   
   char expandBuffer[1024];
   int i;
   if(fileName)
   {
      StreamIO *iostrm;
      if(append)
      {
         iostrm = new FileRWStream(fileName);
         iostrm->setPosition(((FileRWStream*)iostrm)->getSize());
      }
      else
         iostrm = new FileWStream(fileName);

      char buffer[1024];

      AssertFatal(varNameList.size() == varValueList.size(), "invalid varName, varValue pair");
      for(i = 0; i < varNameList.size(); i++)
      {
         if (mode == VARIABLES)
         {
            expandEscape(expandBuffer, varValueList[i]);
            sprintf(buffer, "%s = \"%s\";\r\n", varNameList[i], expandBuffer);
         }
         else
         {
            sprintf(buffer, "%s();\r\n", varNameList[i]);
         }
         iostrm->write(strlen(buffer), buffer);
      }
      iostrm->close();
      delete iostrm;
   }
   else
   {
      for(i = 0; i < varNameList.size(); i++)
      {
         if (mode == VARIABLES)
         {
            expandEscape(expandBuffer, varValueList[i]);
            Console->printf("%s = \"%s\";", varNameList[i], expandBuffer);
         }
         else
         {
            Console->printf("%s();", varNameList[i]);
         }
      }
   }
}

void Dictionary::deleteEntries(SEARCH_MODE mode, const char *entryString)
{
   const char *searchStr = getSearchString(mode, entryString);

   for(int i = 0; i < hashTableSize; i++)
   {
      Entry *walk = hashTable[i];
      while(walk)
      {
         Entry *matchedEntry = (FindMatch::isMatch((char *) searchStr, (char *) walk->name)) ? walk : NULL;
         walk = walk->nextEntry;
         if (matchedEntry)
            remove(matchedEntry); // assumes remove() is a stable remove (will not reorder entries on remove)
      }
   }
}

void Dictionary::exportVariables(const char *varString, const char *fileName, bool append)
{
   exportEntries(VARIABLES, varString, fileName, append);
}

void Dictionary::exportFunctions(const char *funcString, const char *fileName, bool append)
{
   exportEntries(FUNCTIONS, funcString, fileName, append);
}

void Dictionary::exportVariables(const char *varString, Vector<const char *> &varName, Vector<const char *> &value)
{
   exportEntries(VARIABLES, varString, varName, value);
}  
 
void Dictionary::deleteVariables(const char *varString)
{
   deleteEntries(VARIABLES, varString);
}

void Dictionary::deleteFunctions(const char *funcString)
{
   deleteEntries(FUNCTIONS, funcString);
}      

int HashPointer(StringTableEntry ptr)
{
   return int(DWORD(ptr) >> 2);
}

Dictionary::Entry *Dictionary::lookup(StringTableEntry name)
{
   Entry *walk = hashTable[HashPointer(name) % hashTableSize];
   while(walk)
   {
      if(walk->name == name)
         return walk;
      else
         walk = walk->nextEntry;
   }
   
   return NULL;
}

Dictionary::Entry *Dictionary::add(StringTableEntry name)
{
   Entry *walk = hashTable[HashPointer(name) % hashTableSize];
   while(walk)
   {
      if(walk->name == name)
         return walk;
      else
         walk = walk->nextEntry;
   }
   Entry *ret;
   entryCount++;

   if(entryCount > hashTableSize * 2)
   {
   	Entry head(NULL), *walk;
   	int i;
   	walk = &head;
   	walk->nextEntry = 0;
   	for(i = 0; i < hashTableSize; i++) {
   		while(walk->nextEntry) {
   			walk = walk->nextEntry;
   		}
   		walk->nextEntry = hashTable[i];
   	}
      delete[] hashTable;
      hashTableSize = hashTableSize * 4 - 1;
      hashTable = new Entry *[hashTableSize];
      for(i = 0; i < hashTableSize; i++)
         hashTable[i] = NULL;
      walk = head.nextEntry;
      while(walk)
      {
         Entry *temp = walk->nextEntry;
         int idx = HashPointer(walk->name) % hashTableSize;
         walk->nextEntry = hashTable[idx];
         hashTable[idx] = walk;
         walk = temp;
      }
   }

   ret = new(exprState->allocEntry()) Entry(name);
   int idx = HashPointer(name) % hashTableSize;
   ret->nextEntry = hashTable[idx];
   hashTable[idx] = ret;
   return ret;
}

// deleteVariables() assumes remove() is a stable remove (will not reorder entries on remove)
void Dictionary::remove(Dictionary::Entry *ent)
{
   Entry **walk = &hashTable[HashPointer(ent->name) % hashTableSize];
   while(*walk != ent)
      walk = &((*walk)->nextEntry);

   *walk = (ent->nextEntry);
   ent->~Entry();
   exprState->freeEntry(ent);
   entryCount--;
}

Dictionary::Dictionary(ExprEvalState *state)
{
   int i;
   entryCount = 0;
   exprState = state;
   hashTableSize = ST_INIT_SIZE;
   hashTable = new Entry *[hashTableSize];

   for(i = 0; i < hashTableSize; i++)
      hashTable[i] = NULL;
}

Dictionary::~Dictionary()
{
   reset();
   delete [] hashTable;
}

void Dictionary::reset()
{
   int i;
   Entry *walk, *temp;

   for(i = 0; i < hashTableSize; i++)
   {
      walk = hashTable[i];
      while(walk)
      {
         temp = walk->nextEntry;
         walk->~Entry();
         exprState->freeEntry(walk);
         walk = temp;
      }
      hashTable[i] = NULL;
   }
   hashTableSize = ST_INIT_SIZE;
   entryCount = 0;
}


const char *Dictionary::tabComplete(const char *prevText, const char *bestMatch, int baseLen, bool fForward)
{
   int i;
   for(i = 0; i < hashTableSize; i++)
   {
      Entry *walk = hashTable[i];
      while(walk)
      {
         if(canTabComplete(prevText, bestMatch, walk->name, baseLen, fForward))
            bestMatch = walk->name;
         walk = walk->nextEntry;
      }
   }
   return bestMatch;
}


Dictionary::Entry::Entry(StringTableEntry in_name)
{
   dataPtr = NULL;
   func = NULL;
   callbackFunc = NULL;
   callbackObj = NULL;
   name = in_name;
   type = CMDConsole::Default;
}

Dictionary::Entry::~Entry()
{
   if(func)
   {
      func->refCount--;
      if(!func->refCount)
         delete func;
      func = NULL;
   }
}

struct InternalPoint3F { float x,y,z; InternalPoint3F(float F){x=y=z=F;}  };

bool Dictionary::Entry::setData( const char * value )
{
   switch(type)
   {
      case CMDConsole::Bool:
         *reinterpret_cast<bool*>(dataPtr) = CMDConsole::getBool(value);
         break;
      case CMDConsole::Int:
         *reinterpret_cast<int*>(dataPtr) = atoi(value);
         break;
      case CMDConsole::Float:
         *reinterpret_cast<float*>(dataPtr) = atof(value);
         break;
      case CMDConsole::Double:
         *reinterpret_cast<double*>(dataPtr) = atof(value);
         break;
      case CMDConsole::Unit:{
         float val = atof(value);
         val = (val < 0) ? 0 : ( (val > 1) ? 1 : val);
         *reinterpret_cast<float*>(dataPtr) = val;
         break;
      }
      case CMDConsole::Point3F:{
         InternalPoint3F  vec(0);
         sscanf( value, "%f %f %f", &vec.x, &vec.y, &vec.z );
         *reinterpret_cast<InternalPoint3F*>(dataPtr) = vec;
         break;
      }
      default:
         return false;
   }
   return true;
 }

const char *Dictionary::getVariable(StringTableEntry name, bool *entValid)
{
   Entry *ent = lookup(name);
   if(ent)
   {
      if(entValid)
         *entValid = true;
      if(ent->type != CMDConsole::Default)
      {
         switch(ent->type)
         {
            case CMDConsole::Bool:
               ent->value.setValue(*reinterpret_cast<bool*>(ent->dataPtr) ? "True" : "False");
               break;
            case CMDConsole::Int:
               sprintf(scratchBuffer, "%d", *reinterpret_cast<int*>(ent->dataPtr));
               ent->value.setValue(scratchBuffer);
               break;
            case CMDConsole::Float:
               sprintf(scratchBuffer, "%g", *reinterpret_cast<float*>(ent->dataPtr));
               ent->value.setValue(scratchBuffer);
               break;
            case CMDConsole::Double:
               sprintf(scratchBuffer, "%f", *reinterpret_cast<double*>(ent->dataPtr));
               ent->value.setValue(scratchBuffer);
               break;
            case CMDConsole::Unit:
               sprintf(scratchBuffer, "%f", *reinterpret_cast<float*>(ent->dataPtr));
               ent->value.setValue(scratchBuffer);
               break;
            case CMDConsole::Point3F:
               InternalPoint3F  vec = *reinterpret_cast<InternalPoint3F*>(ent->dataPtr);
               sprintf(scratchBuffer, "%f %f %f", vec.x, vec.y, vec.z );
               ent->value.setValue(scratchBuffer);
               break;
         }
      }
      if(ent->value.getValue())
         return ent->value.getValue();
   }
   if(entValid)
      *entValid = false;
   return "";
}

void Dictionary::setVariable(StringTableEntry name, const char *value)
{
   Entry *ent = add(name);
   const char *defValue = "";
   if(!value)
      value = defValue;

   if(!*value && ent->type == CMDConsole::Default && 
      !ent->callbackFunc && !ent->callbackObj)
   {
      remove(ent);
   }
   else
   {
      ent->value.setValue(value);

      // update the goods:
      ent->setData( value );
      
      const char *argv[2];
      argv[0] = ent->name;
      argv[1] = value;

      if(ent->callbackFunc)
         ent->callbackFunc(Console, ent->id, 2, argv);
      if(ent->callbackObj)
         ent->callbackObj->consoleCallback(Console, ent->id, 2, argv);
   }
}

void Dictionary::addVariable(int id, const char *name, CMDCallback::Callback cf, CMDCallback *co, int type, void *dataPtr, const char *value)
{
   if(name[0] != '$')
   {
      scratchBuffer[0] = '$';
      strcpy(scratchBuffer + 1, name);
      name = scratchBuffer;
   }
   Entry *ent = add(stringTable.insert(name));
   ent->id = id;
   if(dataPtr)
   {
      ent->dataPtr = dataPtr;
      ent->type = type;
      const char *evalue = ent->value.getValue();
      if(evalue)
      {
         ent->setData( evalue );
      }
   }
   if(cf)
      ent->callbackFunc = cf;
   if(co)
      ent->callbackObj = co;
   if(value)
      ent->value.setValue(value);
}

bool Dictionary::removeVariable(StringTableEntry name)
{
   if( Entry *ent = lookup(name) ){
      remove( ent );
      return true;
   }
   return false;
}

void Dictionary::addFunction(FunctionDecl *fn)
{
   Entry *ent = add(fn->fnName);
   ent->~Entry();
   new(ent) Entry(fn->fnName);
   ent->func = fn;
   fn->refCount++;
}

void Dictionary::addCommand(int id,const char *name,CMDCallback::Callback cb,int privilegeLevel)
{
   Entry *ent = add(name);
   ent->~Entry();
   new(ent) Entry(name);
   ent->id = id;
   ent->callbackFunc = cb;
   ent->privilegeLevel = privilegeLevel;
}

void Dictionary::addCommand(int id,const char *name,CMDCallback *cb,int privilegeLevel)
{
   Entry *ent = add(name);
   ent->~Entry();
   new(ent) Entry(name);
   ent->id = id;
   ent->callbackObj = cb;
   ent->privilegeLevel = privilegeLevel;

}

void ExprEvalState::lookupVariable(const char *varName)
{
   const char *ret;
   bool valid;
   if(*varName == '$')
      ret = frames[0]->getVariable(varName, &valid);
   else
      ret = frames[frameCount-1]->getVariable(varName, &valid);
   if(!valid)
      Console->dbprintf(2, "Varaible referenced before assignment: %s", varName);
   if(!ret)
      ret = "";
   alloc(strlen(ret) + 1);
   strcpy(baseAddr + curIndex, ret);
}

void ExprEvalState::assignVariable(const char *varName)
{
   if(*varName == '$')
      frames[0]->setVariable(varName, baseAddr + curIndex);
   else
      frames[frameCount-1]->setVariable(varName, baseAddr + curIndex);
}

void ExprEvalState::pushFrame(StringTableEntry frameName)
{
   frameCount++;
   if(frames.size() < frameCount)
      frames.push_back(new Dictionary(this));
   frames[frameCount-1]->scopeName = frameName;
}

void ExprEvalState::popFrame()
{
   frameCount--;
   frames[frameCount]->reset();
}

ExprEvalState::ExprEvalState()
{
   // global dictionary
   frames.push_back(new Dictionary(this));

   size = 4096;
   baseAddr = (char *) malloc(4096);
   frameCount = 1;
   curIndex = 0;
   freeList = NULL;
}

Dictionary::Entry *ExprEvalState::allocEntry()
{
   Dictionary::Entry *ret = freeList;
   if(ret)
      freeList = ret->nextEntry;
   else
      ret = (Dictionary::Entry *) entryPool.alloc(sizeof(Dictionary::Entry));
   return ret;
}

void ExprEvalState::freeEntry(Dictionary::Entry *ent)
{
   ent->nextEntry = freeList;
   freeList = ent;
}

ExprEvalState::~ExprEvalState()
{
   Vector<Dictionary *>::iterator i;
   for(i = frames.begin(); i != frames.end(); i++)
      delete *i;
   free(baseAddr);
}

void ExprEvalState::alloc(int numBytes)
{
   if(numBytes + curIndex > size)
   {
      size = numBytes + curIndex + 1024;
      baseAddr = (char *) realloc(baseAddr, size);
   }
}

bool ExprEvalState::getBoolValue()
{
   return CMDConsole::getBool(baseAddr + curIndex);
}

double ExprEvalState::getFloatValue()
{
   return atof(baseAddr + curIndex);
}

int ExprEvalState::getIntValue()
{
   return atoi(baseAddr + curIndex);
}

void ExprEvalState::setFloatValue(double f)
{
   sprintf(baseAddr + curIndex, "%g", f);
}

void ExprEvalState::setBoolValue(bool b)
{
   strcpy(baseAddr + curIndex, b ? "True" : "False");
}

void ExprEvalState::setIntValue(int i)
{
   sprintf(baseAddr + curIndex, "%d", i);
}

void ExprEvalState::advance()
{
   curIndex += strlen(baseAddr + curIndex) + 1;
}

static int depth = 0;

void CMDDataManager::beginObject(StringTableEntry className, const char *objectName)
{
   Console->printf("%d: Declaring new object %s of class %s", depth, objectName ? objectName : "(NULL)", className);
   depth++;
}

void CMDDataManager::endObject()
{
   depth--;
   Console->printf("%d: Ending object declaration.", depth);
}

void CMDDataManager::declareDataBlock(StringTableEntry className, StringTableEntry objectName)
{
   Console->printf("Declaring new data block %s of class %s", objectName, className);
}

const char *CMDDataManager::getDataField(StringTableEntry objectName, StringTableEntry slotName, const char *array)
{
   static char ret[] = "True";

   Console->printf("getting data field %s[%s] from object %s", slotName, array, objectName);
   return ret;
}

void CMDDataManager::setDataField(StringTableEntry objectName, StringTableEntry slotName, const char *array, int argc, const char **argv)
{
   if(argc == 1)
      Console->printf("setting data field %s[%s] of object %s to %s", slotName, array, objectName, argv[0]);
   else
   {
      Console->printf("setting data field %s[%s] of object %s to:", slotName, array, objectName);
      int i;
      for(i = 0; i < argc; i++)
         Console->printf("  %d: %s", i, argv[i]);
   }
}
