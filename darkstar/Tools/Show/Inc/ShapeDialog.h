#ifndef _ShapeDialog_H_
#define _ShapeDialog_H_

#include "wintools.h"

#include <gw.h>
#include <gwCanvas.h>

#include <COMMCTRL.h>
#include <COMMDLG.h>

#include <sim.h>

#define COREAPI

class showPlugin;
class myGuy;
class TransitionDialog;

class ShapeDialog : public GWDialog, public SimObject
{
   typedef SimObject ObjParent;
	showPlugin *showTool;
	myGuy *obj;
	TransitionDialog *transDBox;
	bool wasPlaying; // work var. used by play track bar
	int timeCount; // work var. used by update process event to skip update cycles
	int prevThread; // these next five variabls allow the update procedure
	int prevSequence; // to update listboxes whenever current thread,
	int prevTransition; // sequence, or transition changes
	int prevNumThread;
	int prevNumSeq;
	int prevInTNum;
	int prevForwardTNum;
	int prevBackwardTNum;
	int prevTransLBSize;
	float prevSpeed;
	bool hide;
public:
	typedef GWDialog Parent;
   ShapeDialog(showPlugin * const sp);
	LRESULT COREAPI windowProc(UINT message, WPARAM wParam, LPARAM lParam);
	void updateObj();
	void updateThreads();
	void updateSequences();
	void updateTransitions();
	void updatePlayTB();
	void updateDetailTB();
	void updateSpeed();
	bool handleThreadLB(int wparam);
	bool handleSequenceLB(int wparam);
	bool handleTransitionLB(int wparam);
	bool handleSpeedEB(int wparam);
	void handlePlayTB(WPARAM wparam);
	void handleDetailTB(WPARAM wparam);
	void getSetTransitions(int &inTransNum,int &forwardTransNum,int &backwardTransNum);
	void selectSetTransitions(int inTransNum,int forwardTransNum,int backwardTransNum);
	void setHide(bool newVal);
	void setObj(myGuy *newObj);
 
	// sim object procedures
	bool COREAPI processEvent(const SimEvent*);
	bool COREAPI processQuery(SimQuery*);
    bool onAdd();
};

inline void ShapeDialog::setHide(bool newVal)
{
	hide=newVal;
	if ( (hide) || !obj)
		ShowWindow(getHandle(),SW_HIDE);
	else
		ShowWindow(getHandle(),SW_SHOW);
}

inline void ShapeDialog::setObj(myGuy *newObj)
{ 
	obj=newObj;
	setHide(hide);
	updateObj();
}

#endif 
