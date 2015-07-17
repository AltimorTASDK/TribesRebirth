//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _GWMSGMAP_H_
#define _GWMSGMAP_H_

//Includes
#include <gwwinsx.h>
#include <types.h>

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

class GWWindow;
#define GW_MSG_CALL __cdecl
typedef bool (GW_MSG_CALL GWWindow::*GWMsgMapFn)(void);

// Message map searching and dispatching functions...
//
enum GWParamSig {
   GWParamSig_End = 0,  // End of messagemap
   
   GWParamSig_v_v,        // ON_CONTROL_*
   GWParamSig_v_w,
   GWParamSig_v_wl,       // ON_WM_SYSCOMMAND
   
   GWParamSig_v_pNMHDRpL,
   GWParamSig_v_wpNMHDRpL
};

struct gwMsgMapEntry {
   UInt32      messageId;
   UInt32      code;
   UInt32      id;
   UInt32      lastId;
   GWParamSig  paramSig;
   GWMsgMapFn  pMsgFn;
};

const gwMsgMapEntry* 
GWFindMsgMapEntry(const gwMsgMapEntry* in_pEntries,
                  const UInt32         in_messageId,
                  const UInt32         in_code,
                  const UInt32         in_id);

struct gwMsgMap {
   const gwMsgMap*      pBaseClassMap;
   const gwMsgMapEntry* pMapEntries;
};

union GWMsgFn {
   GWMsgMapFn pFn;
   void (GW_MSG_CALL GWWindow::*GWMsgFn_v_v)();
   void (GW_MSG_CALL GWWindow::*GWMsgFn_v_w)(WORD);
   void (GW_MSG_CALL GWWindow::*GWMsgFn_v_wl)(WPARAM, LPARAM);
   
   void (GW_MSG_CALL GWWindow::*GWMsgFn_v_pNMHDRpL)(NMHDR*, LRESULT*);
   void (GW_MSG_CALL GWWindow::*GWMsgFn_v_wpNMHDRpL)(WORD, NMHDR*, LRESULT*);
};

#define DECLARE_MESSAGE_MAP()                   \
  private:                                      \
   static const gwMsgMapEntry _msgMapEntries[]; \
  protected:                                    \
   static const gwMsgMap _messageMap;           \
   virtual const gwMsgMap* getMsgMap() const;
   
#define BEGIN_MESSAGE_MAP(class, baseClass)             \
const gwMsgMap*                                 \
class::getMsgMap() const                                \
{                                                       \
   return &class::_messageMap;                          \
}                                                       \
                                                        \
const gwMsgMap class::_messageMap =                     \
{ &baseClass::_messageMap, &class::_msgMapEntries[0] }; \
                                                        \
const gwMsgMapEntry class::_msgMapEntries[] =           \
{

#define END_MESSAGE_MAP()                       \
   {0, 0, 0, 0, GWParamSig_End, (GWMsgMapFn)0 } \
};

#include <gwMsgs.h>

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_GWMSGMAP_H_


