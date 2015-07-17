//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _INSPECTDLG_H_
#define _INSPECTDLG_H_

#include <core.h>
#include <gw.h>
#include "LSTerrainStamp.h"
#include "inspectDlg_def.h"


typedef LSTerrainStamp::TextureType TextureInfo;


#include "GridWindow.h"
class GridWindow;      


class InspectDlg : public GWDialog
{
private:
   GridWindow  *parent;

public:
   InspectDlg(GridWindow *parent);

   void onCommand(int id, HWND hwndCtl, UINT codeNotify);
   void updateView();
   void onApplyTile();
   void onApplyStamp();
};

#endif //_INSPECTDLG_H_
