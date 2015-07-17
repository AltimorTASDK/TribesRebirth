//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _INSPECTWINDOW_H_
#define _INSPECTWINDOW_H_

#include <simBase.h>
#include <simEv.h>
#include <gw.h>
#include "inspect.h"

//------------------------------------------------------------------------------
// Child window classes for displaying an inspected value
//------------------------------------------------------------------------------
class InspectMemberWin
{
public:
   HWND desHWnd;        // Window to display member description
   HWND valHWnd;        // Window to display member value
   Point2I desMinSize;  // min width and height of desHWnd
   Point2I valMinSize;  // min width and height of valHWnd
   Point2I curSize;     // bounding width and height of both windows 
   SimTag des;          // member description tag
   
   // convert the float number based on string value - fix apply button problem
   double atof( const char * buffer, const double orig_val );
   bool getFloatString( const char * buffer, const unsigned int index,
      char * floatBuf, const unsigned int floatBufSize );
      
   void displayErrDialog(const char *fmt);

public:    
   enum Constants {
      BORDER_SPACING     = 2,
      DBL_BORDER_SPACING = 4, 
      EDIT_BORDER        = 10,
      SCROLLBAR_WIDTH    = 30,    
   };

   static char *STR_NONE;   // static <NONE> string

   InspectMemberWin() { desHWnd = NULL; desMinSize.set(0,0); 
                                   valHWnd = NULL; valMinSize.set(0,0);
                                   curSize.set(0,0); 
                                   des = 0; }
                                   
   virtual ~InspectMemberWin()   { if (desHWnd) DestroyWindow(desHWnd);
                                              if (valHWnd) DestroyWindow(valHWnd); }

   Point2I& getDesMinSize()      { return desMinSize; }
   Point2I& getValMinSize()      { return valMinSize; }
   Point2I& getCurSize()         { return curSize; }
   SimTag   getDes()             { return des; }

   // subclass responsibilities
   virtual Inspect::Type getType() = 0;
   virtual void position(Box2I &bbox, Point2I &recommendedDesSize) = 0; 

public:
   // class member functions
   static SimTagDictionary *simTagDict; 
   static void setSimTagDict(SimTagDictionary *std) { simTagDict = std; }
};


//------------------------------------------------------------------------------
class Description_MemberWin: public InspectMemberWin
{
private:
   typedef InspectMemberWin Parent;

public:
   Inspect::Type getType() { return Inspect::Type_Description; }

   void create(const HWND parent, HINSTANCE hInst, SimTag des);
   void position(Box2I &bbox, Point2I &recommendedDesSize);
};  

//------------------------------------------------------------------------------
class StringDescription_MemberWin: public InspectMemberWin
{
private:
   typedef InspectMemberWin Parent;

   const char* pString;

public:
   Inspect::Type getType() { return Inspect::Type_StringDescription; }

   void create(const HWND parent, HINSTANCE hInst, const char* string);
   void position(Box2I &bbox, Point2I &recommendedDesSize);
   
   const char* getVal() const { return pString; }
};

//------------------------------------------------------------------------------
class StringIndexDescription_MemberWin: public InspectMemberWin
{
private:
   typedef InspectMemberWin Parent;

   Inspect_StringIndexDes strIdxDes;

public:
   Inspect::Type getType() { return Inspect::Type_StringIndexDescription; }

   void create(const HWND parent, HINSTANCE hInst, const char* string, int index);
   void position(Box2I &bbox, Point2I &recommendedDesSize);

   const char* getStringVal() const { return strIdxDes.string; }
   const int   getIndexVal() const  { return strIdxDes.index; }
};

//------------------------------------------------------------------------------
class Divider_MemberWin: public InspectMemberWin
{
private:
   typedef InspectMemberWin Parent;

public:
   Inspect::Type getType() { return Inspect::Type_Divider; }

   void create(const HWND parent, HINSTANCE hInst);
   void position(Box2I &bbox, Point2I &recommendedDesSize);
};  

//------------------------------------------------------------------------------
class IString_MemberWin: public InspectMemberWin
{
private:
   typedef InspectMemberWin Parent;

public:
   Inspect::Type getType() { return Inspect::Type_IString; }

   void create(const HWND parent, HINSTANCE hInst, SimTag des, const char *val);
   void position(Box2I &bbox, Point2I &recommendedDesSize);
   Bool getVal(char *buffer, int bufLen);
};  

//--------------------------------------
class Int32_MemberWin: public IString_MemberWin
{
private:
   typedef IString_MemberWin Parent;
   Inspect::Display display;
      
public:
   Inspect::Type getType() { return Inspect::Type_Int32; }

   void create(const HWND parent, HINSTANCE hInst, const SimTag des, const Int32 val, const Inspect::Display _display);
   void position(Box2I &bbox, Point2I &recommendedDesSize);
   Bool getVal(Int32 &val);
};

//--------------------------------------
class RealF_MemberWin: public IString_MemberWin
{
private:
   typedef IString_MemberWin Parent;
   RealF orig_val;
      
public:
   Inspect::Type getType() { return Inspect::Type_RealF; }

   void create(const HWND parent, HINSTANCE hInst, const SimTag des, const RealF val);
   void position(Box2I &bbox, Point2I &recommendedDesSize);
   Bool getVal(RealF &val);
};

//------------------------------------------------------------------------------
class Bool_MemberWin: public InspectMemberWin
{
private:
   typedef InspectMemberWin Parent;

public:
   Inspect::Type getType() { return Inspect::Type_Bool; }

   void create(const HWND parent, HINSTANCE hInst, const SimTag des, const Bool val);
   void position(Box2I &bbox, Point2I &recommendedDesSize);
   Bool getVal(Bool &val);
};  

//------------------------------------------------------------------------------
class Show_Tag_Define_MemberWin: public InspectMemberWin
{
private:
   typedef InspectMemberWin Parent;

public:
   Inspect::Type getType() { return Inspect::Type_Tag; }

   void create(const HWND parent, HINSTANCE hInst, const SimTag des, const Inspect::IString filter, const Int32 val);
   void position(Box2I &bbox, Point2I &recommendedDesSize);
   Bool getVal(Int32 &val);
};  

//------------------------------------------------------------------------------
class Show_Tag_IString_MemberWin: public InspectMemberWin
{
private:
   typedef InspectMemberWin Parent;

public:
   Inspect::Type getType() { return Inspect::Type_Tag;}

   void create(const HWND parent, HINSTANCE hInst, const SimTag des, const Inspect::IString filter, const Int32 val);
   void position(Box2I &bbox, Point2I &recommendedDesSize);
   Bool getVal(Int32 &val);
};  

//------------------------------------------------------------------------------
class Show_TagRange_Define_MemberWin: public Show_Tag_Define_MemberWin
{
private:
   typedef Show_Tag_Define_MemberWin Parent;

public:
   Inspect::Type getType() { return Inspect::Type_TagRange; }

   void create(const HWND parent, HINSTANCE hInst, const SimTag des, const Int32 minTag, const Int32 maxTag, const Int32 val);
}; 

//------------------------------------------------------------------------------
class Show_TagRange_IString_MemberWin: public Show_Tag_IString_MemberWin
{
private:
   typedef Show_Tag_IString_MemberWin Parent;

public:
   Inspect::Type getType() { return Inspect::Type_TagRange;}

   void create(const HWND parent, HINSTANCE hInst, const SimTag des, const Int32 minTag, const Int32 maxTag, const Int32 val);
};  

//------------------------------------------------------------------------------
class SimTagValPair_MemberWin: public InspectMemberWin
{
private:
   typedef InspectMemberWin Parent;
   Inspect::SimTagValPair *stvList;
   Int32 stvLen;

public:
   SimTagValPair_MemberWin()  { stvList = NULL; stvLen = 0; }
   ~SimTagValPair_MemberWin() { if (stvList) delete[] stvList; }
   Inspect::Type getType() { return Inspect::Type_SimTagValPair; }

   void create(const HWND parent, HINSTANCE hInst, const SimTag des, const Int32 val, const Int32 _stvLen, Inspect::SimTagValPair *_stvList);
   void position(Box2I &bbox, Point2I &recommendedDesSize);
   Bool getVal(Int32 &val, Int32 &_stvLen);
};  

//------------------------------------------------------------------------------
class IStringValPair_MemberWin: public InspectMemberWin
{
private:
   typedef InspectMemberWin Parent;
   Inspect::IStringValPair *svList;
   Int32 svLen;

public:
   IStringValPair_MemberWin()  { svList = NULL; svLen = 0; }
   ~IStringValPair_MemberWin() { delete [] svList; }
   Inspect::Type getType() { return Inspect::Type_IStringValPair; }

   void create(const HWND parent, HINSTANCE hInst, const SimTag des, const Int32 val, const Int32 _svLen, Inspect::IStringValPair *_stvList);
   void position(Box2I &bbox, Point2I &recommendedDesSize);
   Bool getVal(Int32 &val, Int32 &_svLen);
};  

//--------------------------------------
class Point2I_MemberWin: public IString_MemberWin
{
private:
   typedef IString_MemberWin Parent;
      
public:
   Inspect::Type getType() { return Inspect::Type_Point2I; }

   void create(const HWND parent, HINSTANCE hInst, const SimTag des, const Point2I val);
   void position(Box2I &bbox, Point2I &recommendedDesSize);
   Bool getVal(Point2I &val);
};

//--------------------------------------
class Point3I_MemberWin: public IString_MemberWin
{
private:
   typedef IString_MemberWin Parent;
      
public:
   Inspect::Type getType() { return Inspect::Type_Point3I; }

   void create(const HWND parent, HINSTANCE hInst, const SimTag des, const Point3I val);
   void position(Box2I &bbox, Point2I &recommendedDesSize);
   Bool getVal(Point3I &val);
};

//--------------------------------------
class Point2F_MemberWin: public IString_MemberWin
{
private:
   typedef IString_MemberWin Parent;
   Point2F orig_val;
      
public:
   Inspect::Type getType() { return Inspect::Type_Point2F; }

   void create(const HWND parent, HINSTANCE hInst, const SimTag des, const Point2F val);
   void position(Box2I &bbox, Point2I &recommendedDesSize);
   Bool getVal(Point2F &val);
};

//--------------------------------------
class Point3F_MemberWin: public IString_MemberWin
{
private:
   typedef IString_MemberWin Parent;
   Point3F orig_val;
         
public:
   Inspect::Type getType() { return Inspect::Type_Point3F; }

   void create(const HWND parent, HINSTANCE hInst, const SimTag des, const Point3F val);
   void position(Box2I &bbox, Point2I &recommendedDesSize);
   Bool getVal(Point3F &val);
};

//--------------------------------------
class ActionBtn_MemberWin: public InspectMemberWin
{
private:
   typedef InspectMemberWin Parent;
   SimObjectId sender;
   SimObjectId recipient;
   Int32 simMessage;
   SimManager *manager;
      
public:
   Inspect::Type getType() { return Inspect::Type_ActionBtn; }

   void create(const HWND parent, HINSTANCE hInst, const SimTag fieldDes, const Inspect_ActionBtn &actBtn, const SimManager *mngr);
   void position(Box2I &bbox, Point2I &recommendedDesSize);
   Bool getVal();
   void updateBtnState();
   void doAction();
};

//------------------------------------------------------------------------------
// Womdow to display the contents of an inspector
// and edit it's values.
//------------------------------------------------------------------------------
class InspectWindow : public SimObject, GWWindow
{
   typedef SimObject Parent;
private:
   enum WindConst {
      InspectWindowStyle = WS_OVERLAPPED | WS_CLIPCHILDREN | WS_SYSMENU | WS_SIZEBOX | WS_VSCROLL,
   };

   static GWWindowClass InspectWindowClass;
   static bool classRegistered;

   GWWindow *_parent;
   char *windowName;
   Point2I  initPos;
   Point2I  initSize;

   Int32 toolbarHeight;
   HWND toolbarHWnd;
   HWND resizeHWnd;
   HWND lockedHWnd;
   Int32 numWindows;
   bool resizeOnInit;

   HWND applyBtnHWnd;
   SimObject *inspectObj;

   SimTagDictionary *simTagDict; // the location of predefine data strings
   
   // constructing the InspectWindow
   void createWin();
   Point2I clientSize;       

   // data and functions to manage the member windows 
   // within the InspectWindow
   Vector<InspectMemberWin *> memberList; 

   Point2I minDesSize;
   Point2I minValSize;
   Int32 contentHeight;
   Int32 contentYOrig; // content position for scroll bar

   void createToolbar();
   
   void createChildWindows();
   void positionChildWindows(int vertOrig);
   void removeChildWindows();

   void init(GWWindow *parent, char *name, Point2I& ppos, Point2I& psize, SimTagDictionary *std);
   Inspect inspector;

public:
   InspectWindow(GWWindow *parent, char *name, Point2I& ppos, Point2I& psize, SimTagDictionary *std = NULL);
   InspectWindow(GWWindow *parent, char *name, RECT &r, SimTagDictionary *std = NULL);
   ~InspectWindow();

   // registry pref functions
   bool setPrefFlag( const char * name, const bool value );
   bool getPrefFlag( const char * name, bool & value );
   
   //--------------------------------------
   // obj from which to display and change
   // it's Inspect
   void setObjToInspect(SimObject *obj, const char *); 
   void initDisplay(); 

   //--------------------------------------
   // simObject messages

   bool onAdd();
   void onRemove();
   void onDeleteNotify(SimObject *object);

   //--------------------------------------
   // windows messages
   BOOL onWindowPosChanging(LPWINDOWPOS lpwpos);
   void onWindowPosChanged(const LPWINDOWPOS lpwpos);
   void onVScroll(HWND, UINT, int pos);
   void onClose();
   void onCommand(int id, HWND hwndCtl, UINT codeNotify);
};

#endif //_INSPECTWINDOW_H_
