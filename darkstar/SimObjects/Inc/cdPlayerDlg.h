//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _CDPLAYERDLG_H_
#define _CDPLAYERDLG_H_

#include <types.h>
#include <gwDialog.h>
#include <simBase.h>

#include <redbook.h>

class CDPlayerDlg : public GWDialog, public SimObject
{
private:
   typedef     SimObject Parent;
   Redbook     *pRb; 
   Int32       curTrack;

public:
   CDPlayerDlg();

	bool onAdd();
   void onCommand(int id, HWND hwndCtl, UINT codeNotify);
   void render();
   void onDestroy();

   // simobject methods
   bool processEvent(const SimEvent *event);
};

#endif //_CDPLAYERDLG_H_
