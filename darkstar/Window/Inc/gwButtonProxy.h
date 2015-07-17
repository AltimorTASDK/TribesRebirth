//------------------------------------------------------------------------------
// Description Proxy object to mediate a connection to a button control.
//              Currently only functionality to support miinimal check/radio
//              button support is implemented.
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _GWBUTTONPROXY_H_
#define _GWBUTTONPROXY_H_

//Includes
#include "gwControlProxy.h"

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

class GWButtonProxy : public GWControlProxy {
  public:
   bool getCheck() const;
   void setCheck(const bool in_state) const;
};


class GWRadioGroup {
  private:
   bool           m_buttonsAttached;

   GWButtonProxy* m_pButtons;
   HWND           m_parentHWnd;
   UInt16         m_firstId;
   UInt16         m_lastId;
   
  public:
   GWRadioGroup();
   ~GWRadioGroup();
   
   bool attach(const HWND in_parentHWnd,
                       const UInt16 in_firstId,
                       const UInt16 in_lastId);
   
   UInt16 getCheckedId() const;
   bool   setChecked(const UInt16 in_id) const;
   void   setEnable(const bool in_enable) const;
};

#define RADIOGROUP_ATTACH(proxy, firstCtrlId, lastCtrlId) \
   proxy.attach(this->getHandle(), firstCtrlId, lastCtrlId);

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_GWBUTTONPROXY_H_
