#ifndef _TransitionDialog_H_
#define _TransitionDialog_H_

#include "wintools.h"

#include <gw.h>
#include <gwCanvas.h>

#include <COMMCTRL.h>
#include <COMMDLG.h>

#include <sim.h>

#define COREAPI

class ShapeDialog;
class showPlugin;
class myGuy;

class TransitionDialog : public GWDialog, public SimObject
{
   typedef SimObject ObjParent;
	showPlugin *showTool;
	ShapeDialog *shapeDBox;
	myGuy *obj;
	int timeCount; // work var. to skip update cycles
	int altTimeCount; // work var. to skip update cycles while in altmode
	float oldSpeed;
	TSShape::Transition *pTrans; // transition being edited
	TSShape::Transition oldTrans; // copy of old value of transition in case of cancel
	EulerF transEuler;
	int iTran;
	int prevStartSeq;
	int prevEndSeq;
	float prevStartPos;
	float prevEndPos;
	float prevDur;
	float prevXtran;
	float prevYtran;
	float prevZtran;
	bool alternateMode;

public:
	typedef GWDialog Parent;
    TransitionDialog(ShapeDialog *);
	LRESULT COREAPI windowProc(UINT message, WPARAM wParam, LPARAM lParam);

	void acceptTransition();
	void rejectTransition();
	void reactivate();
	void deactivate();

	bool editTransition(myGuy *newobj,int transIndex);
	void setShowFrom();
	void setShowTo();

	bool handleEB(int wparam,int id,float &val);
	void changeEBText(int id,float val);

	void handlePosTB(WPARAM wparam,float &pos,int ctrlID);
	bool handleToSeqCLB(int wparam);

	// sim object procedures
	bool COREAPI processEvent(const SimEvent*);
	bool COREAPI processQuery(SimQuery*);
    bool onAdd();
};

#endif 
