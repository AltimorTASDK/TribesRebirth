#include "CamDialog.h"

#include "tripod.h"
#include "myGuy.h"

#define ITB_Hoffset 105
#define ITB_Voffset 106
#define ITB_Hangle 101
#define ITB_Vangle 102
#define ITB_zoom 107
#define ICB_Stick 109
#define ICB_RotateRelative 110

CamDialog::CamDialog(tripod * const sc)
{
	showCam = sc;
	timeCount=0;
	prevHasActor=0;
	setHide(false);
	trackingAngle=false;

	AssertFatal( create("CameraDialog", NULL),"Error opening shape dialog box (missing .RC?)");

	// set trackbar ranges to 0 to 1000
	SendDlgItemMessage(getHandle(),ITB_Hoffset,TBM_SETRANGE,(WPARAM) true,MAKELONG(0,1000));
	SendDlgItemMessage(getHandle(),ITB_Voffset,TBM_SETRANGE,(WPARAM) true,MAKELONG(0,1000));
	SendDlgItemMessage(getHandle(),ITB_Hangle,TBM_SETRANGE,(WPARAM) true,MAKELONG(0,1000));
	SendDlgItemMessage(getHandle(),ITB_Vangle,TBM_SETRANGE,(WPARAM) true,MAKELONG(0,1000));
	SendDlgItemMessage(getHandle(),ITB_zoom,TBM_SETRANGE,(WPARAM) true,MAKELONG(0,1000));

	// set initial position of trackbars
	SendDlgItemMessage(getHandle(),ITB_Hoffset,TBM_SETPOS,(WPARAM) true,500);
	SendDlgItemMessage(getHandle(),ITB_Voffset,TBM_SETPOS,(WPARAM) true,500);
	SendDlgItemMessage(getHandle(),ITB_Hangle,TBM_SETPOS,(WPARAM) true,500);
	SendDlgItemMessage(getHandle(),ITB_Vangle,TBM_SETPOS,(WPARAM) true,500);
	SendDlgItemMessage(getHandle(),ITB_zoom,TBM_SETPOS,(WPARAM) true,0);

	// set RotateRelative and stick to ground buttons
	SendDlgItemMessage(getHandle(),ICB_RotateRelative,BM_SETCHECK,
			showCam->getRelative(),0);
	if (showCam->hasActor())
		SendDlgItemMessage(getHandle(),ICB_Stick,BM_SETCHECK,
				showCam->getActor()->getStickToGround(),0);
}

/*
void CamDialog::setDriveMode()
{
	orbitMode=false;
	// change dialog text -- later
	prevRotX=-10.0f;
	prevRotY=-10.0f;
}

void CamDialog::setOrbitMode()
{
	orbitMode=true;
	// change dialog text -- later
}
*/

void CamDialog::setHide(bool nv)
{
	hide=nv;
	if (hide || !showCam->hasActor())
		ShowWindow(getHandle(),SW_HIDE);
	else
		ShowWindow(getHandle(),SW_SHOW);
}

// return true if pos changes
bool CamDialog::handleTB(WPARAM wparam, int ctrlID, float & pos, float min, float max)
{
  float scrollPos=
	min + (max-min)*SendDlgItemMessage(getHandle(),ctrlID,TBM_GETPOS,0,0)/1000.0f;

  switch (LOWORD(wparam))
  {
	 case TB_THUMBTRACK :
	 case TB_THUMBPOSITION :
		scrollPos=
			min + (max-min) * HIWORD(wparam)/1000.0f;
		// no break
	 case TB_BOTTOM	:
	 case TB_LINEDOWN :
	 case TB_LINEUP	:
	 case TB_PAGEDOWN :
	 case TB_PAGEUP :
	 case TB_TOP :
		pos = scrollPos;
		return true;
  }
  return false;
}

void CamDialog::handleAngleTB(WPARAM wparam, int ctrlID)
{
  const float min = (float)-M_2PI;
  const float max = (float) M_2PI;

  float scrollPos=
	min + (max-min)*SendDlgItemMessage(getHandle(),ctrlID,TBM_GETPOS,0,0)/1000.0f;

  switch (LOWORD(wparam))
  {
	 case TB_ENDTRACK :
		trackingAngle=false;
		SendDlgItemMessage(getHandle(),ctrlID,TBM_SETPOS,true,500);
	 case TB_THUMBTRACK :
	 case TB_THUMBPOSITION :
		scrollPos=
			min + (max-min) * HIWORD(wparam)/1000.0f;
		// no break
	 case TB_BOTTOM	:
	 case TB_LINEDOWN :
	 case TB_LINEUP	:
	 case TB_PAGEDOWN :
	 case TB_PAGEUP :
	 case TB_TOP :
	 {
		if (!trackingAngle)
		{
			trackingAngle=true;
			startTrack = showCam->getOrbitRot();
		}
		if (ctrlID==ITB_Hangle)
			showCam->setOrbitRot( EulerF(startTrack.x,startTrack.y,startTrack.z+scrollPos) );
		else if (ctrlID==ITB_Vangle)
			showCam->setOrbitRot( EulerF(startTrack.x+scrollPos,startTrack.y,startTrack.z) );
		else
			AssertFatal(0,"oops, how'd I get here");
	 }
  }
}

LRESULT CamDialog::windowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
		case WM_CLOSE:
			setHide(true);
			return 0;
		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
				case ICB_Stick :
				{
					if (HIWORD(wParam)!=BN_CLICKED) break;
					if (showCam->hasActor())
						showCam->getActor()->setStickToGround(SendDlgItemMessage(getHandle(),ICB_Stick,BM_GETCHECK,0,0));
					return true;
				}
				case ICB_RotateRelative :
				{
					if (HIWORD(wParam)!=BN_CLICKED) break;
					showCam->setRelative(SendDlgItemMessage(getHandle(),ICB_RotateRelative,BM_GETCHECK,0,0));
					return true;
				}
				default:
					return Parent::windowProc(message, wParam, lParam);
			}
			break;
		}
	 	case WM_HSCROLL :
		{
			int ctrlID = GetDlgCtrlID((HWND) lParam);
			float newPos;
			switch (ctrlID)
			{
				case ITB_Hoffset :
					if (handleTB(wParam,ctrlID,newPos,-1.0f,1.0f))
						showCam->setHoffset(newPos);
					break;
//				case ITB_Hangle :
//					if (handleTB(wParam,ctrlID,newPos,-M_2PI,M_2PI)) // let them go around twice
//					{
//						EulerF tmpR = showCam->getOrbitRot();
//						showCam->setOrbitRot(EulerF(tmpR.x,tmpR.y,newPos));
//					}
//					break;
				case ITB_Hangle :
					handleAngleTB(wParam,ctrlID);
					break;
				case ITB_zoom :
					if (showCam->hasActor() &&
						 handleTB(wParam,ctrlID,newPos,showCam->getActor()->getSphere(),
						 								   100*showCam->getActor()->getSphere()))
							showCam->setDistance(newPos);
					break;
			}
			return Parent::windowProc(message, wParam, lParam);
		}
		case WM_VSCROLL :
		{
			int ctrlID = GetDlgCtrlID((HWND) lParam);
			float newPos;
			switch (ctrlID)
			{
				case ITB_Voffset :
					if (handleTB(wParam,ctrlID,newPos,-1.0f,1.0f))
						showCam->setVoffset(-newPos);
					break;
//				case ITB_Vangle :
//					if (handleTB(wParam,ctrlID,newPos,-M_2PI,M_2PI)) // let them go around twice
//					{
//						EulerF tmpR = showCam->getOrbitRot();
//						showCam->setOrbitRot(EulerF(newPos,tmpR.y,tmpR.z));
//					}
//					break;
				case ITB_Vangle :
					handleAngleTB(wParam,ctrlID);
					break;
			}
			return Parent::windowProc(message, wParam, lParam);
		}
		default:
			return Parent::windowProc(message, wParam, lParam);
	}
	return 0;
}

//--------------------------------------------------------------------------------

bool CamDialog::onAdd()
{
   if (!ObjParent::onAdd())
      return false;
    addToSet(SimTimerSetId);
    return true;
}

bool CamDialog::processEvent(const SimEvent* event)
{

	switch (event->type) 
   {
		case SimTimerEventType:
		{
			// Occurs every 1/30 sec.
			// only respond every 4th time...
			timeCount++;
			if (timeCount==4)
			{
				timeCount=0;
			  	if (showCam->hasActor()!=prevHasActor)
			  	{
					if (showCam->hasActor() && !hide)
						ShowWindow(getHandle(),SW_SHOW);
					else
						ShowWindow(getHandle(),SW_HIDE);
					prevHasActor=showCam->hasActor();
				}

				if (showCam->hasActor())
			  	{
					// has actor, anything different?
					if (showCam->getActor()->getStickToGround()!=prevStickToGround)
					{
						prevStickToGround=showCam->getActor()->getStickToGround();
						SendDlgItemMessage(getHandle(),ICB_Stick,BM_SETCHECK,prevStickToGround,0);
					}
					if (showCam->getRelative()!=prevRelativeCam)
					{
						prevRelativeCam=showCam->getRelative();
						SendDlgItemMessage(getHandle(),ICB_RotateRelative,BM_SETCHECK,prevRelativeCam,0);
					}
					if (showCam->getDistance()!=prevOrbitDist)
					{
						prevOrbitDist=showCam->getDistance();
						float newPos = 10.0f * (prevOrbitDist-showCam->getSphere()) / showCam->getSphere();
						if (newPos>1000.0f)
							newPos=1000.0f;
						if (newPos<0.0f)
							newPos=0.0f;
						SendDlgItemMessage(getHandle(),ITB_zoom,TBM_SETPOS,(WPARAM) true,newPos);
					}
//					const EulerF & orbitRot = showCam->getOrbitRot();
//					if (orbitRot.x!=prevOrbitRotX)
//					{
//						prevOrbitRotX=orbitRot.x;
//						SendDlgItemMessage(getHandle(),ITB_Vangle,TBM_SETPOS,(WPARAM) true,
//							(prevOrbitRotX+M_2PI)*1000.0f/(2*M_2PI));
//					}
//					if (orbitRot.z!=prevOrbitRotZ)
//					{
//						prevOrbitRotZ=orbitRot.z;
//						SendDlgItemMessage(getHandle(),ITB_Hangle,TBM_SETPOS,(WPARAM) true,
//							(prevOrbitRotZ+M_2PI)*1000.0f/(2*M_2PI));
//					}
			  	}
			  	else
			  	{
					// no actor
			  	}

			}
			break;
		}
		default :
			return false;
	}
	return true;
}

bool CamDialog::processQuery( SimQuery *q )
{
   q;
	return false;
}
