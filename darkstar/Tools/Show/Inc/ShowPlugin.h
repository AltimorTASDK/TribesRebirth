//----------------------------------------------------------------------------

#ifndef _showPlugin_H_
#define _showPlugin_H_

//----------------------------------------------------------------------------

#include "myGuy.h"
#include "tripod.h"

#define COREAPI

//----------------------------------------------------------------------------

#define MAX_SHAPES 50

class ShapeDialog;
class CamDialog;

class showPlugin: public SimConsolePlugin
{
	friend ShapeDialog;

	enum CallbackID {
	  ShowToolLoad,
	  ShowToolGoto,
	  ShowToolList,
	  ShowToolLight,
	  ShowToolDetach,
	  ShowToolNext,
	  ShowToolSetDetail,
	  ShowToolGetDetail,
	  ShowToolNewThread,
	  ShowToolSelectThread,
	  ShowToolListThreads,
	  ShowToolNumSequences,
	  ShowToolSetSequence,
	  ShowToolListTransitions,
	  ShowToolNewTransition,
	  ShowToolEditTransition,
	  ShowToolTransitionToSeq,
	  ShowToolSetTransition,
	  ShowToolAnimationPause,
	  ShowToolAnimationPlay,
	  ShowToolAnimationStep,
	  ShowToolSetAnimSpeed,
	  ShowToolGetAnimSpeed,
	  ShowToolSetAnimPos,
	  ShowToolGetAnimPos,
	  ShowToolGetAnimDur,
	  ShowToolSetRelative,
	  ShowToolSetStickToGround,
	  ShowToolSaveShape,
	  ShowToolImportTransitions,
	  ShowToolImportSequence,
	  ShowToolShapeDialog,
	  ShowToolCamDialog,
	  ShowToolCollisionCheck,
	  ShowToolPerformanceCheck,
	  ShowToolShadowLift,
	  ShowToolShadowLight,
	  ShowToolShadows,
	  GameEndFrame,
     LoadMainMenu
	};

	int numShapes;
	int curShape;
	char *shapeNames[MAX_SHAPES+1]; // shapes indexed from 1 .. numShapes
	char *shObjNames[MAX_SHAPES+1]; // this is because 0 is for detached
	myGuy *shapes[MAX_SHAPES+1];
	tripod *showCam;
	ShapeDialog *shapeDBox;
	CamDialog *camDBox;

   float    xrotIncrement;   
   float    zrotIncrement;   
   float    distanceIncrement;

	// for collision checking
	bool checkingCollision; // if true, check for collisions
	myGuy * boxShape;

	void setShape(int newCurShape);
	bool parseInt(const char *str,const char *cmdName,int &i);
	bool parseFloat(const char *str,const char *cmdName,float &f);
	bool warnOnDetached(myGuy *obj,char *str);
	bool seqInRange(myGuy *obj,int seqNum,char *cmdstr);

public:
	void init();
	void startFrame();
	void endFrame();
	const char * consoleCallback(CMDConsole*,int id,int argc,const char *argv[]);
};


#endif
