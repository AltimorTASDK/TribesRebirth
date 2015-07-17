//------------------------------------------------------------------------------
// Description 
//   
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//                    
//------------------------------------------------------------------------------

#ifndef _INSPECT_H_
#define _INSPECT_H_

#include <simBase.h>
#include <memstrm.h>   
#include <simTagDictionary.h>
#include <stdarg.h>

class Inspect;

class DLLAPI Inspect
{
public:
   enum Type {
      Type_Invalid = -1,
      Type_Description,
      Type_StringDescription,
      Type_StringIndexDescription,
      Type_Int32,
      Type_RealF,
      Type_Bool,
      Type_IString,   
      Type_Tag,      // SimTagDictionary Tag ID
      Type_TagRange, // SimTagDiciontary Tag ID
      Type_Point2I,
      Type_Point3I,        
      Type_Point2F,
      Type_Point3F,
      Type_SimTagValPair,
      Type_IStringValPair,
      Type_List,
      Type_ActionBtn,
      Type_Divider,
   };

   typedef char *IString; // IStrings must be of length <= Inspect::MAX_STRING_LEN
                          // ie, buffers holding a IString must be of minimum
                          // size (Inspect::MAX_STRING_LEN + 1)

   enum Display {
      Decimal,
      Hex,
   };

   enum Constants {
      MAX_STRING_LEN = 80,        // max string length, for any Inspect::IString param
      INSPECT_STREAM_SIZE = 4048, // size of inspectStrm, 4K
   };

   struct SimTagValPair {
      SimTag tag; // the simTag and its
      Int32  val; // associated value
   };

   struct IStringValPair {
      const char* string; // string to be displayed
      Int32       val;    // value
   };

private:
   Vector<char> inspectBuffer;
   int curPos;

   void writeString(const IString str);
   void readString(IString str);

   void _write(Type t, const SimTag des, int size, const void *val); 
   void _read(Type t, const SimTag des, int size, void *val); 

   void validateSize(int sizeAdd);
public:
   Inspect();
   
   void clear();
   void top();

   //--------------------------------------
   // all parameters added are copied and not modified
   void write(const SimTag des);
   void write(const SimTag des, const Int32 val, const Display display=Decimal); 
   void write(const SimTag des, const RealF val); 
   void write(const SimTag des, const Bool val);  
   void write(const SimTag des, const IString val); 
   void write(const SimTag des, const Point2I &val);
   void write(const SimTag des, const Point3I &val);
   void write(const SimTag des, const Point2F &val);
   void write(const SimTag des, const Point3F &val);
   void write(const SimTag des, const Bool listDefines, const IString filter, const Int32 tagVal);
   void write(const SimTag des, const Bool listDefines, const Int32 minTag, const Int32 maxTag, const Int32 tagVal);
   void write(const SimTag des, const Int32 val, const Int32 numPairs, const SimTagValPair *stvList); 
   void __cdecl writeSimTagPairList(const SimTag des, const Int32 val, const Int32 numPairs, ...);  
   void writeActionButton(const SimTag des, const SimObject *recv, Int32 simMesg);
   void writeDivider();
   void writeStringDes(const char* strDes);
   void writeStringIndexDes(const char* strDes, int in_index);

   // Note: The inspector does _not_ own the passed IStrings, and will not delete them
   void writeIStringPairList(const SimTag des,     const Int32 val,
                                   const Int32 numPairs, const IStringValPair *svList); 

   //--------------------------------------
   Type   peekNextType(); 
   SimTag peekNextDes();
   Int32  peekNextListSize();  // only valid if next item is a list
   Type   peekNextListType();   // only valid if next item is a list
   void read(const SimTag des);
   void read(const SimTag des, Int32 &val, Display *display=NULL);
   void read(const SimTag des, RealF &val);
   void read(const SimTag des, Bool &val);
   void read(const SimTag des, IString val);
   void read(const SimTag des, Point2I &val);
   void read(const SimTag des, Point3I &val);
   void read(const SimTag des, Point2F &val);
   void read(const SimTag des, Point3F &val);
   void read(const SimTag des, Bool *listDefines, IString filter, Int32 &tagVal);
   void read(const SimTag des, Bool *listDefines, Int32 *minTag, Int32 *maxTag, Int32 &tagVal);
   void read(const SimTag des, Int32 &val, const Int32 numPairs, SimTagValPair *stvList);
   void readSimTagPairList(const SimTag des, Int32 &val);
   void readActionButton(const SimTag des, SimObject **recv, Int32 *simMesg); 
   void readDivider();
   void readStringDes(const char*& out_rpString);
   void readStringIndexDes(const char*& out_rpString, int& out_rIndex);

   void read(const SimTag des, Int32 &val, const Int32 svLen, IStringValPair *svList);
   void readIStringPairList(const SimTag des, Int32 &val); 
};

//------------------------------------------------------------------------------
//  Helper Structs
//------------------------------------------------------------------------------

typedef char Inspect_Str[Inspect::MAX_STRING_LEN + 1];

struct Inspect_Int32 {
   Inspect::Display display;
   Int32 val;
};

struct Inspect_Pt2I {
   Int32 x, y;
};

struct Inspect_Pt3I {
   Int32 x, y, z;
};

struct Inspect_Pt2F {
   RealF x, y;
};

struct Inspect_Pt3F {
   RealF x, y, z;
};

struct Inspect_Tag {
   Bool listDefines;   // list #Define strings or the actual values in the list box
   Inspect_Str filter; // list only the SimTag entries whose #Defines match the filter.
                       // The filter may have one or more search strings separated by
                       // comma.  A search string may use * and ? wildcards.
   Int32 tagVal;
};

struct Inspect_TagRange {
   Bool listDefines;
   SimTag minTag;
   SimTag maxTag;
   SimTag initTag;
};

struct Inspect_List {
   Int32 size;
   Inspect::Type type;
   Int32 val;
};

struct Inspect_ActionBtn {
   SimObject *recipient;
   Int32 simMessage;   
};

struct Inspect_StringIndexDes {
   const char* string;
   int         index;   
};

//----------------------------------------------------------------------------
//  write members
//----------------------------------------------------------------------------
inline void Inspect::write(const SimTag des)
{
   _write(Type_Description, des, 0, 0);
}

inline void Inspect::write(const SimTag des, const Int32 val, const Inspect::Display display)
{
   Inspect_Int32 i;

   i.val = val;
   i.display = display;
   _write(Type_Int32, des, sizeof(Inspect_Int32), (const void *)&i);
}

inline void Inspect::write(const SimTag des, const RealF val)
{
   _write(Type_RealF, des, sizeof(RealF), (const void *)&val);
}

inline void Inspect::write(const SimTag des, const Bool val)
{
   _write(Type_Bool, des, sizeof(Bool), (const void *)&val);
}

inline void Inspect::write(const SimTag des, const IString val)
{
   Inspect_Str str;

   str[0] = '\0';
   if ( val )
   {
      AssertFatal(strlen(val) <= Inspect::MAX_STRING_LEN, "string is too long");
      strcpy(str, val);   
   }

   _write(Type_IString, des, sizeof(Inspect_Str), (const void *)str);
}

inline void Inspect::write(const SimTag des, const Point2I &val)
{
   Inspect_Pt2I pt;
   pt.x = val.x; 
   pt.y = val.y;
   _write(Type_Point2I, des, sizeof(Inspect_Pt2I), (const void *)&pt);
}

inline void Inspect::write(const SimTag des, const Point3I &val)
{
   Inspect_Pt3I pt;
   pt.x = val.x;
   pt.y = val.y;
   pt.z = val.z;
   _write(Type_Point3I, des, sizeof(Inspect_Pt3I), (const void *)&pt);
}

inline void Inspect::write(const SimTag des, const Point2F &val)
{
   Inspect_Pt2F pt;
   pt.x = val.x;
   pt.y = val.y;
   _write(Type_Point2F, des, sizeof(Inspect_Pt2F), (const void *)&pt);
}

inline void Inspect::write(const SimTag des, const Point3F &val)
{
   Inspect_Pt3F pt;
   pt.x = val.x;
   pt.y = val.y;
   pt.z = val.z;
   _write(Type_Point3F, des, sizeof(Inspect_Pt3F), (const void *)&pt);
}

inline void Inspect::write(const SimTag des, const Bool listDefines, const IString filter, const Int32 tagVal)
{
   Inspect_Tag tag;
   tag.listDefines = listDefines;
   if (filter)
      strcpy(tag.filter, filter);
   else
      tag.filter[0] = '\0';
   tag.tagVal = tagVal;

   _write(Type_Tag, des, sizeof(Inspect_Tag), (const void *)&tag);
}

inline void Inspect::write(const SimTag des, const Bool listDefines, const Int32 minTag, const Int32 maxTag, const Int32 tagVal)
{
   Inspect_TagRange tagR;
   tagR.listDefines = listDefines;
   tagR.minTag = minTag;
   tagR.maxTag = maxTag;
   tagR.initTag = tagVal;
   AssertFatal( minTag < maxTag, "minTag must be less then maxTag" );
   _write(Type_TagRange, des, sizeof(Inspect_TagRange),(const void *)&tagR);
}

inline void Inspect::write(const SimTag des,  const Int32 val, const Int32 stvLen, const SimTagValPair *stvList) 
{
   AssertFatal(stvLen > 0, "Inavlid list size");

   Inspect_List list;
   list.size = stvLen;
   list.type = Type_SimTagValPair;
   list.val = val;

   _write(Type_List, des, sizeof(Inspect_List), (const void *)&list);
   _write(Type_SimTagValPair, des, sizeof(SimTagValPair) * stvLen, (const void *)stvList);
}

inline void Inspect::writeIStringPairList(const SimTag des,  const Int32 val,
                                                const Int32 svLen, const IStringValPair *svList)
{
   AssertFatal(svLen > 0, "Inavlid list size");

   Inspect_List list;
   list.size = svLen;
   list.type = Type_IStringValPair;
   list.val  = val;

   _write(Type_List, des, sizeof(Inspect_List), (const void *)&list);
   _write(Type_IStringValPair, des, sizeof(IStringValPair) * svLen, (const void *)svList);
}

inline void Inspect::writeActionButton(const SimTag des, const SimObject *recv, Int32 simMesg)
{
   Inspect_ActionBtn actBtn;
   
   actBtn.recipient = (SimObject *)recv;
   actBtn.simMessage = simMesg;
   _write(Type_ActionBtn, des, sizeof(Inspect_ActionBtn), (const void *)&actBtn);   
}

inline void Inspect::writeDivider()
{
   _write(Type_Divider, 0, 0, 0);
}

inline void Inspect::writeStringDes(const char* strDes)
{
   _write(Type_StringDescription, 0, sizeof(const char*), &strDes);
}

inline void Inspect::writeStringIndexDes(const char* strDes, int index)
{
   Inspect_StringIndexDes strIdx;
   
   strIdx.string = strDes;
   strIdx.index  = index;

   _write(Type_StringIndexDescription, 0, sizeof(Inspect_StringIndexDes), &strIdx);
}

//----------------------------------------------------------------------------
//  read members
//----------------------------------------------------------------------------
inline void Inspect::read(const SimTag des)
{
   _read(Type_Description, des, 0, NULL);
}

inline void Inspect::read(const SimTag des, Int32 &val, Inspect::Display *display)
{
   Inspect_Int32 i;

   _read(Type_Int32, des, sizeof(Inspect_Int32), (void *)&i);
   val = i.val;
   if (display != NULL)
      *display = i.display; 
}

inline void Inspect::read(const SimTag des, RealF &val)
{
   _read(Type_RealF, des, sizeof(RealF), (void *)&val);
}

inline void Inspect::read(const SimTag des, Bool &val)
{
   _read(Type_Bool, des, sizeof(Bool), (void *)&val);
}

inline void Inspect::read(const SimTag des, IString val)
{
   // val must be at least of size MAX_STRING_LEN + 1
   _read(Type_IString, des, sizeof(Inspect_Str), (void *)val);
}

inline void Inspect::read(const SimTag des, Point2I &val)
{
   Inspect_Pt2I pt;

   _read(Type_Point2I, des, sizeof(Inspect_Pt2I), (void *)&pt);
   val.x = pt.x;
   val.y = pt.y;
}

inline void Inspect::read(const SimTag des, Point3I &val)
{
   Inspect_Pt3I pt;

   _read(Type_Point3I, des, sizeof(Inspect_Pt3I), (void *)&pt);
   val.x = pt.x;
   val.y = pt.y;
   val.z = pt.z;
}

inline void Inspect::read(const SimTag des, Point2F &val)
{
   Inspect_Pt2F pt;

   _read(Type_Point2F, des, sizeof(Inspect_Pt2F), (void *)&pt);
   val.x = pt.x;
   val.y = pt.y;
}

inline void Inspect::read(const SimTag des, Point3F &val)
{
   Inspect_Pt3F pt;

   _read(Type_Point3F, des, sizeof(Inspect_Pt3F), (void *)&pt);
   val.x = pt.x;
   val.y = pt.y;
   val.z = pt.z;
}

inline void Inspect::read(const SimTag des, Bool *listDefines, IString filter, Int32 &tagVal)
{
   Inspect_Tag tag;

   _read(Type_Tag, des, sizeof(Inspect_Tag), (void *)&tag);
   if (listDefines)
      *listDefines = tag.listDefines;
   if (filter)
      strcpy(filter, tag.filter);
   tagVal = tag.tagVal;
}

inline void Inspect::read(const SimTag des, Bool *listDefines, Int32 *minTag, Int32 *maxTag, Int32 &tagVal)
{
   Inspect_TagRange tagR;
   _read(Type_TagRange, des, sizeof(Inspect_TagRange),(void *)&tagR);

   if (listDefines)
      *listDefines = tagR.listDefines;
   if (minTag)
      *minTag = tagR.minTag;
   if (maxTag)
      *maxTag = tagR.maxTag;

   tagVal = tagR.initTag;   
}

inline void Inspect::read(const SimTag des, Int32 &val, const Int32 stvLen, SimTagValPair *stvList)
{
   if (stvList)
      AssertFatal(stvLen == peekNextListSize(), "list size does not match");

   Inspect_List list;
   _read(Type_List, des, sizeof(Inspect_List), (void *)&list);
   AssertFatal(list.type == Type_SimTagValPair, "list types do not match");
   val = list.val;

   if (stvList)
   {
      _read(Type_SimTagValPair, des, sizeof(SimTagValPair) * stvLen, (void *)stvList);
   }
   else
   {
      // eat it up
      AssertFatal(int(curPos + sizeof(Type) + sizeof(SimTag) + sizeof(SimTagValPair) * stvLen) <= 
               inspectBuffer.size(), "Inspect::read: field does not exist");
      curPos += sizeof(Type) + sizeof(SimTag) + sizeof(SimTagValPair) * stvLen;
   }
}

inline void Inspect::readSimTagPairList(const SimTag des, Int32 &val)
{
   SimTagValPair *stvList = new SimTagValPair[peekNextListSize()];
   read(des, val, peekNextListSize(), stvList);
   delete[] stvList;
}

inline void Inspect::read(const SimTag des, Int32 &val, const Int32 svLen, IStringValPair *svList)
{
   if (svList)
      AssertFatal(svLen == peekNextListSize(), "list size does not match");

   Inspect_List list;
   _read(Type_List, des, sizeof(Inspect_List), (void *)&list);
   AssertFatal(list.type == Type_IStringValPair, "list types do not match");
   val = list.val;

   if (svList)
   {
      _read(Type_IStringValPair, des, sizeof(IStringValPair) * svLen, (void *)svList);
   }
   else
   {
      // eat it up
      AssertFatal(int(curPos + sizeof(Type) + sizeof(SimTag) + sizeof(IStringValPair) * svLen) <= 
               inspectBuffer.size(), "Inspect::read: field does not exist");
      curPos += sizeof(Type) + sizeof(SimTag) + sizeof(IStringValPair) * svLen;
   }
}

inline void Inspect::readIStringPairList(const SimTag des, Int32 &val)
{
   IStringValPair *svList = new IStringValPair[peekNextListSize()];
   read(des, val, peekNextListSize(), svList);
   delete[] svList;
}

inline void Inspect::readActionButton(const SimTag des, SimObject **recv, Int32 *simMesg)
{
   Inspect_ActionBtn actBtn;
   _read(Type_ActionBtn, des, sizeof(Inspect_ActionBtn), (void *)&actBtn); 
   if (recv)
      *recv = actBtn.recipient;
   if (simMesg)
      *simMesg = actBtn.simMessage;
} 

inline void Inspect::readDivider()
{
   _read(Type_Divider, 0, 0, NULL);
}

inline void Inspect::readStringDes(const char*& out_rpString)
{
   const char* pDummy;
   _read(Type_StringDescription, 0, sizeof(const char*), (void*)&pDummy);
   out_rpString = pDummy;
}

inline void Inspect::readStringIndexDes(const char*& out_rpString, int& out_rIndex)
{
   Inspect_StringIndexDes dummy;

   _read(Type_StringIndexDescription, 0, sizeof(Inspect_StringIndexDes), (void*)&dummy);
   out_rpString = dummy.string;
   out_rIndex   = dummy.index;
}

//----------------------------------------------------------------------------
//                         - misc -
//----------------------------------------------------------------------------
inline void Inspect::top()
{
   curPos = 0;
}

inline void Inspect::clear()
{
   inspectBuffer.setSize(0);
   curPos = 0;
}
 

#endif //_INSPECT_H_
