//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _GWCTRLPROXY_H_
#define _GWCTRLPROXY_H_

//Includes
#include <gwBase.h>
#include <gwWinSx.h>

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

class GWControlProxy
{
  private:
   bool   m_attached;

   HWND   m_parentHWnd;
   HWND   m_ctrlHWnd;
   UInt32 m_ctrlId;

  protected:
   HWND getParentHWnd() const;
   HWND getControlHWnd() const;

  public:
   GWControlProxy();
   virtual ~GWControlProxy();
   
   bool   attach(const HWND   in_parentHWnd,
                         const UInt16 in_ctrlId);
   void   setEnable(const bool in_enable) const;

   UInt16 getControlId() const;
   
   void getWindowText(char* out_pString, const Int32 in_bufSize);
   void setWindowText(const char* in_pString);
};


inline UInt16 
GWControlProxy::getControlId() const
{
   AssertFatal(m_attached == true, "Error, not attached!");
   return UInt16(m_ctrlId);
}

inline HWND 
GWControlProxy::getParentHWnd() const
{
   AssertFatal(m_attached == true, "Error, not attached!");
   return m_parentHWnd;
}

inline HWND 
GWControlProxy::getControlHWnd() const
{
   AssertFatal(m_attached == true, "Error, not attached!");
   return m_ctrlHWnd;
}


#define PROXY_ATTACH(proxy, ctrlId)         \
   proxy.attach(this->getHandle(), ctrlId);


#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_GWCTRLPROXY_H_

