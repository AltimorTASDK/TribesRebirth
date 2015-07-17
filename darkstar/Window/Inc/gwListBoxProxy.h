//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _GWLISTBOXPROXY_H_
#define _GWLISTBOXPROXY_H_

//Includes
#include <gwControlProxy.h>

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

class GWListBoxProxy : public GWControlProxy {
  public:
   Int32  getCount() const;
   
   UInt32 getItemData(const int in_index) const;
   UInt32 setItemData(const int in_index, const UInt32 in_userData);
   
   Int32 getSelStatus(const int in_index) const;
   
   Int32 getItemText(const int in_index, char* in_pBuffer) const;

   Int32 clearListBox() const;
   
   Int32 addString(const char* in_pString);
   Int32 deleteString(const int in_index);
   Int32 insertString(const int in_index, const char* in_pString);

   // Single Selection Listboxes...
   //
   Int32 getCurSel() const;
   Int32 setCurSel(const int in_index);
   
   // Multi-selection listboxes
   //
   Int32 setSel(const in_index, const bool in_select);
   Int32 getSelCount() const;
   Int32 getSelItems(const UInt32 in_maxItems,
                             const int*   in_pItemBuffer) const;
};

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_GWLISTBOXPROXY_H_
