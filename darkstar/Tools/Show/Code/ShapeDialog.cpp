#include "simConsolePlugin.h"
#include "ShapeDialog.h"
#include "ShowPlugin.h"
#include "transitionDialog.h"
// #include "simGame.h"

#include "tripod.h"
#include "myGuy.h"

#define ILB_Threads 101
#define ILB_Sequences 102
#define ILB_Transitions 105
#define IPB_Play 108
#define IPB_Step 109
#define IPB_Pause	110
#define IPB_NewThread 111
#define IPB_NewTransition 112
#define ITB_Play 106
#define IRB_TransitionTo 116
#define IRB_SetSequence 115
#define IEB_Speed 114
#define ICB_AutoDetail 113
#define ITB_Detail 117

ShapeDialog::ShapeDialog(showPlugin * const sp)
{
	showTool = sp;
	obj=0;
	hide = false;
	wasPlaying = false;
	timeCount = 0;
	prevThread = -1;
	prevSequence = -1;
	prevTransition = -1;
	prevInTNum = -1;
	prevForwardTNum = -1;
	prevBackwardTNum = -1;
	AssertFatal( create("ShapeDialog", NULL),"Error opening shape dialog box (missing .RC?)");
	// set play trackbar range 0 to 1000
	SendDlgItemMessage(getHandle(),ITB_Play,TBM_SETRANGE,(WPARAM) true,MAKELONG(0,1000));

	// don't transition by default
	SendDlgItemMessage(getHandle(),IRB_SetSequence,BM_SETCHECK,(WPARAM) 1,0);
	// autoDetail by default
	SendDlgItemMessage(getHandle(),ICB_AutoDetail,BM_SETCHECK,(WPARAM) 1,0);
	updateObj();
	transDBox = new TransitionDialog(this);
}

void ShapeDialog::updateObj()
{
	// give detail level trackbar the right number of tick marks
	int numDL = (obj) ? obj->numDetailLevels() : 0;
	SendDlgItemMessage(getHandle(),ITB_Detail,TBM_SETRANGE,(WPARAM) true,
		MAKELONG(0,numDL-1));

	updateThreads();
	updatePlayTB();
	updateDetailTB();
}

void ShapeDialog::updateThreads()
{
	// if no shape selected then just return (dialog box will be hidden when no shape)
	if (!obj)
		return;
	
	// how many items currently in the list box
	int oldSize = SendDlgItemMessage(getHandle(),ILB_Threads,LB_GETCOUNT,0,0);
	// how many do we want
	int newSize = obj->getNumThreads();
	if (oldSize>newSize)
		// delete excess entries
		for (int i=oldSize-1;i>=newSize;i--)
			SendDlgItemMessage(getHandle(),ILB_Threads,LB_DELETESTRING,(WPARAM) i,0);
	else if (oldSize<newSize)
		// add new entries
		for (int i=oldSize;i<newSize;i++)
		{
			char tmpStr[3];
			SendDlgItemMessage(getHandle(),ILB_Threads,LB_ADDSTRING,0,
				(LPARAM) itoa(i,tmpStr,10) );
		}

	// right number of items in list, now select the current one
	SendDlgItemMessage(getHandle(),ILB_Threads,LB_SETCURSEL,
		(WPARAM) obj->getCurThreadIndex(),0);

	if (newSize>0)
		updateSequences();
}

void ShapeDialog::updateSequences()
{
	// if no shape selected then just return (dialog box will be hidden when no shape)
	if (!obj)
		return;
	
	// how many sequences
	int newSize = obj->numAnimSeq();

	// get rid of current entries
	SendDlgItemMessage(getHandle(),ILB_Sequences,LB_RESETCONTENT,0,0);

	for (int i=0;i<newSize;i++)
	{
		// build sequence str
		char seqStr[100];
		sprintf(
			seqStr,"%s  %i  Dur=%.2f",
			obj->getSeqName(i),
			i,
			obj->getAnimDur(i)+.005
		);
		// add the new entry
		SendDlgItemMessage(getHandle(),ILB_Sequences,LB_ADDSTRING,0,
			(LPARAM) seqStr );
	}

	// if no sequences, get out while we still can
	if (!newSize)
		return;

	// select current sequence
	SendDlgItemMessage(getHandle(),ILB_Sequences,LB_SETCURSEL,
		(WPARAM) obj->getAnimSeq(),0);

	updateTransitions();
}

void ShapeDialog::updateSpeed()
{
	if (!obj)
		return;

	char tmpStr[10];
	sprintf(tmpStr,"%.4f",obj->getAnimSpeed()+.000005);
	// get rid of trailing 0's
	while (tmpStr[strlen(tmpStr)-1]=='0')
		tmpStr[strlen(tmpStr)-1]='\0';
	// get rid of decimal point if not needed
	if (tmpStr[strlen(tmpStr)-1]=='.')
		tmpStr[strlen(tmpStr)-1]='\0';
	// replaced existing edit box text
	SendDlgItemMessage(getHandle(),IEB_Speed,WM_SETTEXT,0,(LPARAM) (LPCTSTR) tmpStr);
}

void ShapeDialog::getSetTransitions(int &inTransNum,
												int &forwardTransNum,
												int &backwardTransNum)
{
	// select any transitions that are currently set
	TSShapeInstance::Thread & curThread = obj->getThread();
	Vector<TSShape::Transition> const &transList = obj->getTransitionList();
	forwardTransNum=-1;
	backwardTransNum=-1;
	inTransNum=-1;
	for (int i=0;i<transList.size();i++)
	{
		if (curThread.getState()==TSShapeInstance::Thread::TransitionPending)
		{
			if ( (&(curThread.getForwardTransition())!=0) &&
			  (&(curThread.getForwardTransition())==&transList[i]) )
					forwardTransNum=i;
			if ( (&(curThread.getBackwardTransition())!=0) &&
			  (&(curThread.getBackwardTransition())==&transList[i]) )
					backwardTransNum=i;
		}
		else if (curThread.getState()==TSShapeInstance::Thread::InTransition)
		{
			if ( &(curThread.getTransition()) == &transList[i] )
				inTransNum=i;
		}
	}
}

void ShapeDialog::selectSetTransitions(int inTransNum,
													int forwardTransNum,
													int backwardTransNum)
{
	// now find any set transitions in listbox
	int sizeLB = SendDlgItemMessage(getHandle(),ILB_Transitions,LB_GETCOUNT,0,0);
	for (int i=0;i<sizeLB;i++)
	{
		int tNum = SendDlgItemMessage(getHandle(),ILB_Transitions,LB_GETITEMDATA,i,0);
		if ( (tNum==forwardTransNum) ||
			  (tNum==backwardTransNum) ||
			  (tNum==inTransNum) )
			SendDlgItemMessage(getHandle(),ILB_Transitions,LB_SETSEL,1,i);
		else
			SendDlgItemMessage(getHandle(),ILB_Transitions,LB_SETSEL,0,i);
	}
}

void ShapeDialog::updateTransitions()
{
	// if no shape selected then just return (dialog box will be hidden when no shape)
	if (!obj)
		return;
	
	// get rid of current entries
	SendDlgItemMessage(getHandle(),ILB_Transitions,LB_RESETCONTENT,0,0);

	// find transitions that apply to curent sequence
	Vector<TSShape::Transition> const &transList = obj->getTransitionList();
	int curSeq = obj->getAnimSeq();
	for (int i=0;i<transList.size();i++)
	{
		char tranStr[30];
		tranStr[0]='\0';
		if (transList[i].fStartSequence==curSeq)
			// transition from current sequence
			sprintf(tranStr,"%.2f --> (%i,%.2f) Dur=%.2f",transList[i].fStartPosition+.005,
				transList[i].fEndSequence,transList[i].fEndPosition+.005,
				transList[i].fDuration+.005);
		else if (transList[i].fEndSequence==curSeq)
			// transition to current sequence
			sprintf(tranStr,"(%i,%.2f) --> %.2f Dur=%.2f",transList[i].fStartSequence,
				transList[i].fStartPosition+.005,transList[i].fEndPosition+.005,
				transList[i].fDuration+.005);

		if (tranStr[0]!='\0')
		{
			int newIndex = SendDlgItemMessage(getHandle(),ILB_Transitions,LB_ADDSTRING,0,
				(LPARAM) tranStr);
			// so we'll know which transition to take if user clicks
			SendDlgItemMessage(getHandle(),ILB_Transitions,LB_SETITEMDATA,newIndex,i);
		}
	}

	// get set transitions
	int inTransNum;
	int forwardTransNum;
	int backwardTransNum;
	getSetTransitions(inTransNum,forwardTransNum,backwardTransNum);

	// select these transitions
	selectSetTransitions(inTransNum,forwardTransNum,backwardTransNum);
}

void ShapeDialog::updatePlayTB()
{
  long pos = ((obj) && (obj->getNumThreads()!=0)) ? obj->getAnimPos() * 1000 : 0;
  SendDlgItemMessage(getHandle(),ITB_Play,TBM_SETPOS,(WPARAM) true,pos);
}

void ShapeDialog::updateDetailTB()
{
  long pos = (obj) ? obj->getDetailLevel() : 0;
  SendDlgItemMessage(getHandle(),ITB_Detail,TBM_SETPOS,(WPARAM) true,pos);
}

bool ShapeDialog::handleThreadLB(int wparam)
{
	switch (wparam)
	{
		case LBN_SELCHANGE: 
		{
			int threadNum = SendDlgItemMessage(getHandle(),ILB_Threads,LB_GETCURSEL,0,0);
			obj->selectThread(threadNum);
			updateSequences();
			// give up focus on selection
			SetFocus(getHandle());
			return true;
		}
	}
	return false;
}

bool ShapeDialog::handleSequenceLB(int wparam)
{
	switch (wparam)
	{
		case LBN_SELCHANGE: 
		{
			int seqNum = SendDlgItemMessage(getHandle(),ILB_Sequences,LB_GETCURSEL,0,0);
			if (SendDlgItemMessage(getHandle(),IRB_TransitionTo,BM_GETCHECK,0,0))
				obj->transitionTo(seqNum);
			else
				obj->setAnimSeq(seqNum);
			updateSequences(); // was updateTransitions...done this way because sequence
									 // may not actually change right away...let periodic update
									 // notice the change instead of doing it right away
			// give up focus on selection
			SetFocus(getHandle());
			return true;
		}
	}
	return false;
}

bool ShapeDialog::handleTransitionLB(int wparam)
{
	switch (wparam)
	{
		case LBN_SELCHANGE: 
		{
			int selIndex=SendDlgItemMessage(getHandle(),ILB_Transitions,LB_GETANCHORINDEX,0,0);
			int tranIndex = SendDlgItemMessage(getHandle(),ILB_Transitions,LB_GETITEMDATA,selIndex,0);
			// set the transition
			obj->setTransition(tranIndex);

			// update selected transitions (some may need to be turned off)
			int inTransNum;
			int forwardTransNum;
			int backwardTransNum;
			getSetTransitions(inTransNum,forwardTransNum,backwardTransNum);
			selectSetTransitions(inTransNum,forwardTransNum,backwardTransNum);

			// give up focus
			SetFocus(getHandle());
			return true;
		}
		case LBN_DBLCLK : 
		{
			int clickIndex=SendDlgItemMessage(getHandle(),ILB_Transitions,LB_GETANCHORINDEX,0,0);
			int tranIndex = SendDlgItemMessage(getHandle(),ILB_Transitions,LB_GETITEMDATA,clickIndex,0);
			transDBox->editTransition(obj,tranIndex);
	 	   ShowWindow(transDBox->getHandle(),SW_SHOW);
			// give up focus
			SetFocus(getHandle());
			return true;
		}
	}

	return false;
}

void ShapeDialog::handlePlayTB(WPARAM wparam)
{
  float scrollPos=
	SendDlgItemMessage(getHandle(),ITB_Play,TBM_GETPOS,0,0)/1000.0f;

  switch (LOWORD(wparam))
  {
    case TB_ENDTRACK : 
	 	if (wasPlaying)
			obj->play();
		wasPlaying=false;
		break;
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
		if (!wasPlaying)
			wasPlaying = obj->isPlaying();
		obj->pause();
		obj->setAnimPos(scrollPos,true);
		break;
	 }
  }
}

void ShapeDialog::handleDetailTB(WPARAM wparam)
{
  int scrollPos=
	SendDlgItemMessage(getHandle(),ITB_Detail,TBM_GETPOS,0,0);

  switch (LOWORD(wparam))
  {
	 case TB_THUMBTRACK :
	 case TB_THUMBPOSITION :
		scrollPos=HIWORD(wparam);
		// no break
	 case TB_BOTTOM	:
	 case TB_LINEDOWN :
	 case TB_LINEUP	:
	 case TB_PAGEDOWN :
	 case TB_PAGEUP :
	 case TB_TOP :
	 {
		bool autoDetail=SendDlgItemMessage(getHandle(),ICB_AutoDetail,BM_GETCHECK,0,0);
		if (!autoDetail)
			// set new detail level
			obj->setDetailLevel(scrollPos);
		else
			// set position back
			SendDlgItemMessage(getHandle(),ITB_Detail,TBM_SETPOS,(WPARAM) true,
				(LPARAM) obj->getDetailLevel());
		break;
	 }
  }
}

bool ShapeDialog::handleSpeedEB(int wparam)
{
	if (wparam==EN_CHANGE)
	{
		// check to see if return was pressed...if so, lose focus
		// get text from box	
		char text[32];
		char *tmpPtr;
		SendDlgItemMessage(getHandle(),IEB_Speed,WM_GETTEXT,32,(LPARAM) text);
		if ( (tmpPtr=strchr(text,'\r'))!=NULL ) // I don't know why '\r', but...
		{
			do
				*tmpPtr=*(tmpPtr+2);
			while (*(tmpPtr++)!='\0'); // cut out the "\r\n"
			SendDlgItemMessage(getHandle(),IEB_Speed,WM_SETTEXT,0,(LPARAM) text);
			SetFocus(getHandle()); // return hit, set focus to outer dialog box
		}
		return true;
	}

	if (wparam!=EN_KILLFOCUS)
		return false;

	// if no shape, don't set anything
	if (!obj)
		return true;

	// get text from box	
	char text[32];
	SendDlgItemMessage(getHandle(),IEB_Speed,WM_GETTEXT,32,(LPARAM) text);

	char *endPtr;
	float newSpeed = (float) strtod(text,&endPtr);
	if (*endPtr=='\0')
	{
		// then we have a legal float -- but only set speed if we have a shape
		if (obj)
			obj->setAnimSpeed(newSpeed);
	}
	else
		// illegal value, reset edit box with existing speed
		updateSpeed();

	return true;
}

LRESULT ShapeDialog::windowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    CMDConsole *console = CMDConsole::getLocked();
    switch (message)
    {
		case WM_CLOSE:
			setHide(true);
			return 0;
		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
				case ILB_Threads : 
					return handleThreadLB(HIWORD(wParam));
				case ILB_Sequences :
					return handleSequenceLB(HIWORD(wParam));
				case ILB_Transitions :
					return handleTransitionLB(HIWORD(wParam));
				case IPB_Play :
					if (HIWORD(wParam)!=BN_CLICKED) break;
					obj->play();
					return true;
				case IPB_Pause :
					if (HIWORD(wParam)!=BN_CLICKED) break;
					obj->pause();
					return true;
				case IPB_Step :
					if (HIWORD(wParam)!=BN_CLICKED) break;
					obj->stepThread(0.1f); // let speed control step size
					return true;
				case ICB_AutoDetail :
				{
					if (HIWORD(wParam)!=BN_CLICKED) break;
					bool autoDetail=SendDlgItemMessage(getHandle(),ICB_AutoDetail,BM_GETCHECK,0,0);
					if (autoDetail)
						obj->setDetailLevel(0,1000);
					else
					{
						SendDlgItemMessage(getHandle(),ITB_Detail,TBM_SETPOS,
							(WPARAM) true,(LPARAM) obj->getDetailLevel());
						obj->setDetailLevel(obj->getDetailLevel(),obj->getDetailLevel());
					}
					return true;
				}
				case IEB_Speed :
					return handleSpeedEB(HIWORD(wParam));
				case IPB_NewThread :
				{
					if (HIWORD(wParam)!=BN_CLICKED) break;
					char *cmdStr[1];
					cmdStr[0]="newThread";
					console->execute(1,(const char **) cmdStr);
					SetFocus(getHandle()); // give focus back to main dialog box
					return true;
				}
				case IPB_NewTransition :
				{
					if (HIWORD(wParam)!=BN_CLICKED) break;
					char *cmdStr[1];
					cmdStr[0]="newTransition";
					console->execute(1,(const char **) cmdStr);
					updateTransitions();
					SetFocus(getHandle()); // give focus back to main dialog box
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
			switch (ctrlID)
			{
				case ITB_Play :
					handlePlayTB(wParam);
					break;
				case ITB_Detail :
					handleDetailTB(wParam);
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

bool ShapeDialog::onAdd()
{
   if (!ObjParent::onAdd())
      return false;
    addToSet(SimTimerSetId);
    manager->addObject(transDBox);

    return true;
}

bool ShapeDialog::processEvent(const SimEvent* event)
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
			  if (obj)
			  {
  				// do rough tests for listboxes being out of date--update if needed
				if ( obj->getNumThreads()!=prevNumThread)
					updateThreads();
				if (obj->numAnimSeq()!=prevNumSeq)
					updateSequences();
				prevNumThread=obj->getNumThreads();
				prevNumSeq=obj->numAnimSeq();

				// right # of entries, hi-light the right ones
				if (prevThread!=obj->getCurThreadIndex())
				{
					SendDlgItemMessage(getHandle(),ILB_Threads,LB_SETCURSEL,
						(WPARAM) obj->getCurThreadIndex(),0);
					// now remember the current thread
					prevThread=obj->getCurThreadIndex();
				}
				if ( obj->numAnimSeq() && prevSequence!=obj->getAnimSeq())
				{
					SendDlgItemMessage(getHandle(),ILB_Sequences,LB_SETCURSEL,
					(WPARAM) obj->getAnimSeq(),0);
					updateTransitions();
					// now remember the current sequence
					prevSequence=obj->getAnimSeq();
				}

				// get set transitions
				int inTransNum;
				int forwardTransNum;
				int backwardTransNum;
				getSetTransitions(inTransNum,forwardTransNum,backwardTransNum);
				// do these differ from previous ones?
				if ( (inTransNum!=prevInTNum) ||
					  (forwardTransNum!=prevForwardTNum) ||
					  (backwardTransNum!=prevBackwardTNum) )
				{
					// update selections
					selectSetTransitions(inTransNum,forwardTransNum,backwardTransNum);
					// set prev transitions
					prevInTNum=inTransNum;
					prevForwardTNum=forwardTransNum;
					prevBackwardTNum=backwardTransNum;
				}
			  }

		  	  // update play trackbar
		  	  updatePlayTB();
		  	  // update detail trackbar
		  	  updateDetailTB();

			  // update speed if nec.
			  if (obj && (prevSpeed!=obj->getAnimSpeed()) )
			  {
			    updateSpeed();
				 prevSpeed=obj->getAnimSpeed();
			  }
		   }
			break;
		}
		default :
			return false;
	}
	return true;
}

bool ShapeDialog::processQuery( SimQuery *q )
{
   q;
	return false;
}