#ifndef _transitionDialog_H_
#define _transitionDialog_H_

#include <ts.h>
#include <base.h>
// #include <simtime.h>

#include "simConsolePlugin.h"
#include "transitiondialog.h"
#include "myGuy.h"
#include "shapeDialog.h"
#include "showPlugin.h"

#define IEB_fromSeq 104
#define ITB_fromPos 101
#define IRB_showFrom 109

#define ICLB_toSeq 103
#define ITB_toPos 102
#define IRB_showTo 110

#define IEB_transX 106
#define IEB_transY 107
#define IEB_transZ 108

#define IEB_rotX 112
#define IEB_rotY 113
#define IEB_rotZ 114

#define IEB_duration 105

#define IPB_OK 1
#define IPB_cancel 2
#define ICB_altMode 111

char *ftos(char *str,float val)
{
	sprintf(str,"%.5f",val);
	// get rid of trailing 0's
	while (str[strlen(str)-1]=='0')
		str[strlen(str)-1]='\0';
	// get rid of decimal point if not needed
	if (str[strlen(str)-1]=='.')
		str[strlen(str)-1]='\0';

	return str;
}

TransitionDialog::TransitionDialog(ShapeDialog *shDB)
{
	AssertFatal( create("TransitionDialog", NULL),"Error opening transition dialog box (missing .RC?)");

	// set from and to trackbar range 0 to 1000
	SendDlgItemMessage(getHandle(),ITB_fromPos,TBM_SETRANGE,(WPARAM) true,MAKELONG(0,1000));
	SendDlgItemMessage(getHandle(),ITB_toPos,TBM_SETRANGE,(WPARAM) true,MAKELONG(0,1000));

	// set alternate mode to false
	alternateMode=false;
	SendDlgItemMessage( getHandle(),ICB_altMode,BM_SETCHECK,(WPARAM) 0,0);

	shapeDBox = shDB;
	pTrans=0;
	obj=0;
	timeCount=0;
	altTimeCount=0;
}

bool TransitionDialog::editTransition(myGuy *newobj,int transIndex)
{
	obj=newobj;
	iTran=transIndex;

	if ( (!obj) || (transIndex>=obj->getTransitionList().size()) )
		return false;

	// set to pause
	obj->pause();

	// set speed to unity and remember old speed
	oldSpeed = obj->getAnimSpeed();
	obj->setAnimSpeed(1.0f);

	// set transition, remember old value
	pTrans=const_cast< TSShape::Transition *>(&(obj->getTransitionList()[transIndex]));
	oldTrans=*pTrans;

	// set transEuler
	QuatF quat;
	transEuler.set( pTrans->fTransform.getRotate( &quat ) );

	// enter transition
	obj->setAnimSeq(pTrans->fStartSequence);
	obj->setAnimPos(pTrans->fStartPosition);
	obj->setTransition(transIndex);
	obj->stepThread(pTrans->fDuration/2);

	// go to the beginning of the transition
	setShowFrom();

	// set transition trackbars
	SendDlgItemMessage(getHandle(),ITB_fromPos,TBM_SETPOS,(WPARAM) true,
		(int) 1000 * pTrans->fStartPosition);
	SendDlgItemMessage(getHandle(),ITB_toPos,TBM_SETPOS,(WPARAM) true,
		(int) 1000 * pTrans->fEndPosition);

	// set transitionFrom text
	char tmpStr[10];
	SendDlgItemMessage(getHandle(),IEB_fromSeq,WM_SETTEXT,0,(LPARAM) itoa(pTrans->fStartSequence,tmpStr,10));

	// set transitionTo combo box
	SendDlgItemMessage(getHandle(),ICLB_toSeq,CB_RESETCONTENT,0,0);
	for (int i=0;i<obj->numAnimSeq();i++)
		SendDlgItemMessage(getHandle(),ICLB_toSeq,CB_ADDSTRING,0,(LPARAM) itoa(i,tmpStr,10));
	SendDlgItemMessage(getHandle(),ICLB_toSeq,CB_SETCURSEL,pTrans->fEndSequence,0);

	// set translation edit boxes
	const Point3F &pt = pTrans->fTransform.getTranslate();
	
	SendDlgItemMessage( getHandle(),IEB_transX,WM_SETTEXT,0,
		(LPARAM) ftos(tmpStr,pt.x) );
	SendDlgItemMessage( getHandle(),IEB_transY,WM_SETTEXT,0,
		(LPARAM) ftos(tmpStr,pt.y) );
	SendDlgItemMessage( getHandle(),IEB_transZ,WM_SETTEXT,0,
		(LPARAM) ftos(tmpStr,pt.z) );
	SendDlgItemMessage( getHandle(),IEB_rotX,WM_SETTEXT,0,
		(LPARAM) ftos(tmpStr,transEuler.x) );
	SendDlgItemMessage( getHandle(),IEB_rotY,WM_SETTEXT,0,
		(LPARAM) ftos(tmpStr,transEuler.y) );
	SendDlgItemMessage( getHandle(),IEB_rotZ,WM_SETTEXT,0,
		(LPARAM) ftos(tmpStr,transEuler.z) );

	// set duration edit box
	SendDlgItemMessage( getHandle(),IEB_duration,WM_SETTEXT,0,
		(LPARAM) ftos(tmpStr,pTrans->fDuration) );

	// show the window
	ShowWindow(getHandle(),SW_SHOW);

	return true;
}

void TransitionDialog::acceptTransition()
{
	obj->setAnimSpeed(oldSpeed);
	ShowWindow(getHandle(),SW_HIDE);
	obj=0;
	pTrans=0;
}

void TransitionDialog::rejectTransition()
{
	*pTrans=oldTrans;
	shapeDBox->updateTransitions();
	acceptTransition();
}

void TransitionDialog::reactivate()
{
	if (obj)
		editTransition(obj,iTran);
}

void TransitionDialog::deactivate()
{
	if (!obj)
		return;
	obj->setAnimSpeed(oldSpeed);
	alternateMode=false;
	SendDlgItemMessage( getHandle(),ICB_altMode,BM_SETCHECK,(WPARAM) 0,0);
}

void TransitionDialog::setShowFrom()
{
	obj->setAnimPos(0.0f,true);
	SendDlgItemMessage(getHandle(),IRB_showFrom,BM_SETCHECK,(WPARAM) 1,0);	
	SendDlgItemMessage(getHandle(),IRB_showTo,BM_SETCHECK,(WPARAM) 0,0);	
}

void TransitionDialog::setShowTo()
{
	obj->setAnimPos(1.0f,true);
	SendDlgItemMessage(getHandle(),IRB_showFrom,BM_SETCHECK,(WPARAM) 0,0);	
	SendDlgItemMessage(getHandle(),IRB_showTo,BM_SETCHECK,(WPARAM) 1,0);	
}

// -- returns true iff legal number entered
// -- val should be a copy of the last legal #
//    (it can be the original if there are no
//    restrictions on it beyond being a legal #)
// -- if user enters a legal float and hits return,
//    val will hold the updated editbox value
// -- to change back to old value, you must call changeEB
bool TransitionDialog::handleEB(int wparam,int id,float &val)
{
	if (wparam==EN_CHANGE)
	{
		// check to see if return was pressed...if so, lose focus
		// get text from box
		char text[32];
		char *tmpPtr;
		SendDlgItemMessage(getHandle(),id,WM_GETTEXT,32,(LPARAM) text);
		if ( (tmpPtr=strchr(text,'\r'))!=NULL ) // I don't know why '\r', but...
		{
			do
				*tmpPtr=*(tmpPtr+2);
			while (*(tmpPtr++)!='\0'); // cut out the "\r\n"
			SendDlgItemMessage(getHandle(),id,WM_SETTEXT,0,(LPARAM) text);
			SetFocus(getHandle()); // return hit, set focus to outer dialog box
		}
		return false;
	}

	if (wparam!=EN_KILLFOCUS)
		return false;

	// get text from box	
	char text[32];
	SendDlgItemMessage(getHandle(),id,WM_GETTEXT,32,(LPARAM) text);

	char *endPtr;
	float oldVal = val;
	val = (float) strtod(text,&endPtr);
	if (*endPtr=='\0')
		return true;

	changeEBText(id,oldVal);
	return false;	
}

void TransitionDialog::changeEBText(int id,float val)
{
	char tmpStr[10];
	SendDlgItemMessage( getHandle(),id,WM_SETTEXT,0,
		(LPARAM) ftos(tmpStr,val) );
}

void TransitionDialog::handlePosTB(WPARAM wparam, float &pos, int ctrlID)
{
  float scrollPos=
	SendDlgItemMessage(getHandle(),ctrlID,TBM_GETPOS,0,0)/1000.0f;

  switch (LOWORD(wparam))
  {
	 case TB_THUMBTRACK :
	 case TB_THUMBPOSITION :
		scrollPos=HIWORD(wparam)/1000.0f;
		// no break
	 case TB_BOTTOM	:
	 case TB_LINEDOWN :
	 case TB_LINEUP	:
	 case TB_PAGEDOWN :
	 case TB_PAGEUP :
	 case TB_TOP :
	 {
		pos = scrollPos;
		shapeDBox->updateTransitions();
		break;
	 }
  }
}

bool TransitionDialog::handleToSeqCLB(int wparam)
{
	switch (wparam)
	{
		case CBN_SELCHANGE :
			pTrans->fEndSequence=
				SendDlgItemMessage(getHandle(),ICLB_toSeq,CB_GETCURSEL,0,0);
			obj->invalidateSList();
			break;
	}
	return false;
}

LRESULT TransitionDialog::windowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
		case WM_CLOSE:
			rejectTransition();
			return 0;
		case WM_COMMAND:
		{
			Point3F trans = pTrans->fTransform.getTranslate();
		
			switch (LOWORD(wParam))
			{
				case IPB_OK :
					if (HIWORD(wParam)!=BN_CLICKED) break;
					acceptTransition();
					break;
				case IPB_cancel :
					if (HIWORD(wParam)!=BN_CLICKED) break;
					rejectTransition();
					break;
				case ICB_altMode :
					if (HIWORD(wParam)!=BN_CLICKED) break;
					alternateMode=
						SendDlgItemMessage(getHandle(),ICB_altMode,BM_GETCHECK,0,0);
					break;
				case ICLB_toSeq : 
					return handleToSeqCLB(HIWORD(wParam));
				case IRB_showFrom :
					if (HIWORD(wParam)!=BN_CLICKED) break;
					setShowFrom();
					return true;
				case IRB_showTo :
					if (HIWORD(wParam)!=BN_CLICKED) break;
					setShowTo();
					return true;
				case IEB_transX :
				{
					float val = trans.x;
					if (handleEB(HIWORD(wParam),IEB_transX,val))
					{
						float curPos=obj->getAnimPos();
						obj->setAnimPos(0.0f,true);
						trans.x = val;
						pTrans->fTransform.setTranslate( trans );
						obj->setAnimPos(curPos,true);
					}
					break;
				}
				case IEB_transY :
				{
					float val = trans.y;
					if (handleEB(HIWORD(wParam),IEB_transY,val))
					{
						float curPos=obj->getAnimPos();
						obj->setAnimPos(0.0f,true);
						trans.y = val;
						pTrans->fTransform.setTranslate( trans );
						obj->setAnimPos(curPos,true);
					}
					break;
				}
				case IEB_transZ :
				{
					float val = trans.z;
					if (handleEB(HIWORD(wParam),IEB_transZ,val))
					{
						float curPos=obj->getAnimPos();
						obj->setAnimPos(0.0f,true);
						trans.z = val;
						pTrans->fTransform.setTranslate( trans );
						obj->setAnimPos(curPos,true);
					}
					break;
				}
				case IEB_rotX :
				{
					if (handleEB(HIWORD(wParam),IEB_rotX,transEuler.x))
					{
						float curPos=obj->getAnimPos();
						obj->setAnimPos(0.0f,true);
						QuatF newRot(transEuler);
						pTrans->fTransform.setRotate( newRot );
						obj->setAnimPos(curPos,true);
					}
					break;
				}
				case IEB_rotY :
				{
					if (handleEB(HIWORD(wParam),IEB_rotY,transEuler.y))
					{
						float curPos=obj->getAnimPos();
						obj->setAnimPos(0.0f,true);
						QuatF newRot(transEuler);
						pTrans->fTransform.setRotate( newRot );
						obj->setAnimPos(curPos,true);
					}
					break;
				}
				case IEB_rotZ :
				{
					if (handleEB(HIWORD(wParam),IEB_rotZ,transEuler.z))
					{
						float curPos=obj->getAnimPos();
						obj->setAnimPos(0.0f,true);
						QuatF newRot(transEuler);
						pTrans->fTransform.setRotate( newRot );
						obj->setAnimPos(curPos,true);
					}
					break;
				}
				case IEB_duration :
				{
					float val=pTrans->fDuration;
					if (handleEB(HIWORD(wParam),IEB_duration,val))
					{
						if (val>.005)
							pTrans->fDuration=val;
						else
							changeEBText(IEB_duration,pTrans->fDuration);
					}
					break;
				}
				default:
					return Parent::windowProc(message, wParam, lParam);
			}
			break;
		}
	 	case WM_HSCROLL :
		{
			int ctrlID = GetDlgCtrlID((HWND) lParam);
			switch (ctrlID)
			{
				case ITB_toPos :
					handlePosTB(wParam,pTrans->fEndPosition,ctrlID);
					if (!alternateMode)
						setShowTo();
					break;
				case ITB_fromPos :
					handlePosTB(wParam,pTrans->fStartPosition,ctrlID);
					if (!alternateMode)
						setShowFrom();
					break;
			}
			return Parent::windowProc(message, wParam, lParam);
		}
		case WM_ACTIVATE :
		{
			if (LOWORD(wParam)==WA_INACTIVE)
				deactivate();
			else
				reactivate();
			return Parent::windowProc(message, wParam, lParam);
		}
		default:
			return Parent::windowProc(message, wParam, lParam);
	}
	return 0;
}

bool TransitionDialog::onAdd()
{
	if (!ObjParent::onAdd())
		return false;
    addToSet(SimTimerSetId);
    return true;
}

bool TransitionDialog::processEvent(const SimEvent* event)
{
	switch (event->type) 
   {
		case SimTimerEventType:
		{
			// Occurs every 1/30 sec.
			// only respond every 4th time...
			altTimeCount++;
			if (altTimeCount==5)
			{
				altTimeCount=0;
				if (obj && alternateMode)
				{
					if (SendDlgItemMessage(getHandle(),IRB_showFrom,BM_GETCHECK,0,0))
						setShowTo();
					else
						setShowFrom();
				}
			}

			timeCount++;
			if (timeCount==8)
			{
				timeCount=0;
				// if anything changed w/ the transition, update
				if ( obj && pTrans &&
					 ( (prevStartSeq!=pTrans->fStartSequence) ||
					   (prevEndSeq!=pTrans->fEndSequence) ||
					   (prevStartPos!=pTrans->fStartPosition) ||
					   (prevEndPos!=pTrans->fEndPosition) ||
					   (prevDur!=pTrans->fDuration) ||
					   (prevXtran!=pTrans->fTransform.getTranslate().x) ||
					   (prevYtran!=pTrans->fTransform.getTranslate().y) ||
					   (prevZtran!=pTrans->fTransform.getTranslate().z) ) )
				{
					shapeDBox->updateTransitions();
					prevStartSeq=pTrans->fStartSequence;
					prevEndSeq=pTrans->fEndSequence;
					prevStartPos=pTrans->fStartPosition;
					prevEndPos=pTrans->fEndPosition;
					prevDur=pTrans->fDuration;
					prevXtran=pTrans->fTransform.getTranslate().x;
					prevYtran=pTrans->fTransform.getTranslate().y;
					prevZtran=pTrans->fTransform.getTranslate().z;
				}
			}
			break;
		}
		default :
			return false;
   }
   return true;
}

bool TransitionDialog::processQuery( SimQuery *q )
{
   q;
	return false;
}

#endif