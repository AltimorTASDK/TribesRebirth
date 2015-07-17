//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include "inspect.h"
      

//------------------------------------------------------------------------------
Inspect::Inspect()
{
   curPos = 0;
}   

//------------------------------------------------------------------------------
void Inspect::validateSize(int sizeAdd)
{
   int newSize = curPos + sizeAdd;
   if(inspectBuffer.capacity() < newSize)
      inspectBuffer.reserve(newSize + 1024);
}

//------------------------------------------------------------------------------
void Inspect::writeString(const IString str)
{
   validateSize(sizeof(Inspect_Str));
   Inspect_Str string = {'\0'};

   if (str)
   {
      AssertFatal(strlen(str) <= MAX_STRING_LEN, "str size is out of bounds");
      strcpy(string, str);
   }
   memcpy(&inspectBuffer[curPos], string, sizeof(Inspect_Str));
   curPos += sizeof(Inspect_Str);
}  

//------------------------------------------------------------------------------

void Inspect::readString(IString str)
{
   AssertFatal(str, "no buffer to read string into");      
   // str better of at least of size Inspect_IString

   memcpy(str, &inspectBuffer[curPos], sizeof(Inspect_Str));
   curPos += sizeof(Inspect_Str);
} 

//------------------------------------------------------------------------------ 
void Inspect::_write(Type type, const SimTag des, int size, const void *val)
{
   validateSize(sizeof(Type) + size + sizeof(SimTag));
   
   // write member type
   memcpy(&inspectBuffer[curPos], (void *) &type, sizeof(Type));
   curPos += sizeof(Type);
   
   // write description of member
   memcpy(&inspectBuffer[curPos], (void *) &des, sizeof(SimTag));
   curPos += sizeof(SimTag);

   // write value
   if (val)
      memcpy(&inspectBuffer[curPos], val, size);
   curPos += size;

   inspectBuffer.setSize(curPos);
}   

void __cdecl Inspect::writeSimTagPairList(const SimTag des, const Int32 val, const Int32 numPairs ...)
{
   va_list args;
   
   va_start(args, numPairs);
   SimTagValPair *stvList = new SimTagValPair[numPairs];
   for (int i= 0; i < numPairs; i++)
   {
      stvList[i].tag = va_arg(args, SimTag);
      stvList[i].val = va_arg(args, Int32);
   }
   va_end(args);
   write(des, val, numPairs, stvList);
   delete[] stvList;
}

//------------------------------------------------------------------------------
Inspect::Type Inspect::peekNextType()
{
   Type type;
   if(int(curPos + sizeof(Type)) > inspectBuffer.size())
      return Type_Invalid;
   memcpy((void *) &type, &inspectBuffer[curPos], sizeof(Type));
   return type;
}   

SimTag Inspect::peekNextDes()
{
   SimTag tag;
   if (int(curPos + sizeof(Type) + sizeof(SimTag)) > inspectBuffer.size())
      return 0;
   memcpy((void *) &tag, &inspectBuffer[curPos + sizeof(Type)], sizeof(SimTag));
   return tag;
}   

Int32 Inspect::peekNextListSize()
{
   AssertFatal(peekNextType() == Type_List, "Can only peek size on List types");
   Inspect_List list;
   int saveCurPos = curPos;
   _read(Type_List, peekNextDes(), sizeof(Inspect_List), (void *)&list);
   curPos = saveCurPos;
   return list.size;
}  

Inspect::Type Inspect::peekNextListType()
{
   AssertFatal(peekNextType() == Type_List, "Can only peek size on List types");
   Inspect_List list;
   int saveCurPos = curPos;
   _read(Type_List, peekNextDes(), sizeof(Inspect_List), (void *)&list);
   curPos = saveCurPos;
   return list.type;
}   

//------------------------------------------------------------------------------
void Inspect::_read(Type type, const SimTag des, int size, void *val)
{
   AssertFatal(int(curPos + size + sizeof(Type) + sizeof(SimTag)) <= 
               inspectBuffer.size(), "Inspect::read: field does not exist");

   // make sure that the next Type is the one we are trying to read
   AssertFatal(peekNextType() == type, "Inspect::read: wrong field type on read.");
   AssertFatal(peekNextDes() == des, "Inspect::Get: wrong SimTag for inspect field description");
   curPos += sizeof(Type);
   curPos += sizeof(SimTag);
   memcpy(val, &inspectBuffer[curPos], size);
   curPos += size;
}  
 
