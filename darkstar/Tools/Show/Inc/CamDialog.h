#ifndef _CamDialog_H_
#define _CamDialog_H_

#include "wintools.h"

#include <gw.h>
#include <gwCanvas.h>

#include <COMMCTRL.h>
#include <COMMDLG.h>

#include <sim.h>

#define COREAPI

class showPlugin;
class tripod;

class CamDialog : public GWDialog, public SimObject
{
   typedef SimObject ObjParent;
	tripod *showCam;
	bool hide;
	bool trackingAngle;
	EulerF startTrack; 
	int timeCount;
	bool prevHasActor;
	bool prevStickToGround;
	bool prevRelativeCam;
	float prevOrbitDist;
public:
	typedef GWDialog Parent;
   CamDialog(tripod * const sc);
	LRESULT COREAPI windowProc(UINT message, WPARAM wParam, LPARAM lParam);
	bool handleTB(WPARAM wparam,int ctrlID,float & pos, float min, float max);
	void handleAngleTB(WPARAM wparam,int ctrlID);
	void setHide(bool nv);
 
	// sim object procedures
	bool COREAPI processEvent(const SimEvent*);
	bool COREAPI processQuery(SimQuery*);
    bool onAdd();
};

#endif 
