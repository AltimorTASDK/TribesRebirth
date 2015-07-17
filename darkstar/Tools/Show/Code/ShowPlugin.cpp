//----------------------------------------------------------------------------


//----------------------------------------------------------------------------

#include <sim.h>
#include <m_collision.h>

#include "simConsolePlugin.h"
#include "showPlugin.h"
#include "shapeDialog.h"
#include "camDialog.h"
#include "transitionDialog.h"
#include "simGuiCanvas.h"

#define defaultAnimationIncrement 0.5f
#pragma warn -lvc

int showLightNum=0;
bool cameraAttached;

extern void hackRender();
extern int shucks;

// globals for shadow rendering
extern Point3F worldLightDirection;
extern float shadowLift;
extern bool castingShadows;
extern HINSTANCE theHInstance;
extern HMENU hMenu;
//----------------------------------------------------------------------------

void showPlugin::init()
{
	console->printf("showPlugin");
	console->addCommand(ShowToolLoad,"view",this);
	console->addCommand(ShowToolGoto,"goto",this);
	console->addCommand(ShowToolList,"list",this);
	console->addCommand(ShowToolLight,"light",this);
	console->addCommand(ShowToolDetach,"detach",this);
	console->addCommand(ShowToolNext,"next",this);
	console->addCommand(ShowToolSetDetail,"setdetail",this);
	console->addCommand(ShowToolGetDetail,"getdetail",this);
	console->addCommand(ShowToolNewThread,"newThread",this);
	console->addCommand(ShowToolSelectThread,"selectThread",this);
	console->addCommand(ShowToolListThreads,"listThreads",this);
	console->addCommand(ShowToolNumSequences,"numSequences",this);
	console->addCommand(ShowToolSetSequence,"setSequence",this);
	console->addCommand(ShowToolListTransitions,"listTransitions",this);
	console->addCommand(ShowToolNewTransition,"newTransition",this);
	console->addCommand(ShowToolEditTransition,"editTransition",this);
	console->addCommand(ShowToolTransitionToSeq,"transitionTo",this);
	console->addCommand(ShowToolSetTransition,"setTransition",this);
	console->addCommand(ShowToolAnimationPause,"pause",this);
	console->addCommand(ShowToolAnimationPlay,"play",this);
	console->addCommand(ShowToolAnimationStep,"step",this);
	console->addCommand(ShowToolSetAnimSpeed,"setAnimSpeed",this);
	console->addCommand(ShowToolGetAnimSpeed,"getAnimSpeed",this);
	console->addCommand(ShowToolSetAnimPos,"setAnimPos",this);
	console->addCommand(ShowToolGetAnimPos,"getAnimPos",this);
	console->addCommand(ShowToolGetAnimDur,"getAnimDur",this);
	console->addCommand(ShowToolSetRelative,"setRelative",this);
	console->addCommand(ShowToolSetStickToGround,"stickToGround",this);
	console->addCommand(ShowToolSaveShape,"saveShape",this);
	console->addCommand(ShowToolImportTransitions,"importTransitions",this);
	console->addCommand(ShowToolImportSequence,"importSequence",this);
	console->addCommand(ShowToolShapeDialog,"shapeDialog",this);
	console->addCommand(ShowToolCamDialog,"camDialog",this);
	console->addCommand(ShowToolCollisionCheck,"collision",this);
	console->addCommand(ShowToolPerformanceCheck,"performanceCheck",this);
	console->addCommand(ShowToolShadowLift,"shadowLift",this);
	console->addCommand(ShowToolShadowLight,"shadowLight",this);
	console->addCommand(ShowToolShadows,"shadows",this);
   
   // 
	console->addCommand(GameEndFrame,"Game::EndFrame",this);
	console->addCommand(LoadMainMenu,"loadMainMenu",this);
   
	// Debugging
   console->addVariable(0, "Shape::renderBoundingBox", CMDConsole::Bool, &myGuyRenderImage::renderBounds);

	// make a tripod object
	showCam = new tripod();
	manager->addObject(showCam);
	manager->assignName(showCam,"showCam");
	showCam->setDistance(0.0f);
	showCam->setPos(Point3F(-250.0f,600.0f,50.0f));
	showCam->setRelative(true);

	// init a few variables
	numShapes=0;
	curShape=0;
	checkingCollision=false;

	// init the shape dialog box
	shapeDBox = new ShapeDialog(this);
	manager->addObject(shapeDBox); // shapeDBox is a simobject (for time updates...)
	shapeDBox->setObj(0);
	shapeDBox->updateThreads();

	// make the camera control dialog box
	camDBox = new CamDialog(showCam);
	manager->addObject(camDBox);

   const EulerF & orbit = showCam->getOrbitRot();
   console->addVariable(0, "Showcam::XROT", CMDConsole::Float, (float *)(&orbit.x) );
   console->addVariable(0, "Showcam::ZROT", CMDConsole::Float, (float *)(&orbit.z) );
   
   xrotIncrement = 0;
   zrotIncrement = 0;
   console->addVariable(0, "Showcam::xInc", CMDConsole::Float, &xrotIncrement );
   console->addVariable(0, "Showcam::zInc", CMDConsole::Float, &zrotIncrement );
   console->addVariable(0, "Showcam::dInc", CMDConsole::Float, &distanceIncrement );
}


//----------------------------------------------------------------------------

void showPlugin::startFrame()
{
}

void showPlugin::endFrame()
{
	// Update console variables.
   CMDConsole* console = CMDConsole::getLocked();

	// collide with camera focus
   myGuy *obj;
	obj=dynamic_cast< myGuy *>(showCam->getActor());

	// do collision checking here
	if (checkingCollision && obj)
	{
		CollisionSurfaceList cList;
		cList.clear();

		// if radius 0 then we'll test against line
		float radius = console->getFloatVariable("Coll::radius",0.0f);
		// if len 0 then we'll test against sphere
		float collisionLen = console->getFloatVariable("Coll::length",100.0f);
		// if both radius and len 0 then we won't check collision

		// define start and end of line in world space space
		TMat3F camToWorld(showCam->getRot(),showCam->getPos());
		Point3F sl;
		m_mul(Point3F(0.0f,0.0f,0.0f),camToWorld,&sl);
		Point3F el;
		m_mul(Point3F(0.0f,collisionLen,0.0f),camToWorld,&el); 

		// now put it into the collision target's object space
		TMat3F toObj(*(obj->getRot()),*(obj->getPos()));
		toObj.inverse();
		Point3F tStart,tEnd;
		m_mul(sl,toObj,&tStart);
		m_mul(el,toObj,&tEnd);

		bool collision;
		if (radius>0.0f && collisionLen>0.0f)
			collision = obj->getShapeInst().collideTube(0,tStart,tEnd,radius,0,&cList);
		else if (radius==0.0f && collisionLen>0.0f)
			collision = obj->getShapeInst().collideLine(0,tStart,tEnd,0,&cList);
		else if (radius>0.0f && collisionLen==0.0f)
			collision = obj->getShapeInst().collideSphere(0,tStart,radius,0,&cList);
		else if (numShapes>1) // collide this shape to shape 0
		{
			TMat3F tmpMat(*shapes[1]->getRot(),*shapes[1]->getPos());
			tmpMat.inverse();
			TMat3F fromThisShape;
			m_mul(TMat3F(*obj->getRot(),*obj->getPos()),tmpMat,&fromThisShape);
			collision = obj->getShapeInst().collideShape(0,0,
					shapes[1]->getShapeInst(),&fromThisShape,&cList,false);
			if (collision)
			{
				console->setVariable("Coll::name",
					shapes[1]->getShape().fNames[cList[0].part]);
			}
		}

		int timeCheck = console->getIntVariable("Coll::timeCheck",0);
		if (timeCheck)
		{
			DWORD start1=GetTickCount();
			if (radius>0.0f && collisionLen>0.0f)
				for (int k=0;k<1000;k++)
					obj->getShapeInst().collideTube(0,tStart,tEnd,radius,0,&cList);
			else if (radius==0.0f && collisionLen>0.0f)
				for (int k=0;k<1000;k++)
					obj->getShapeInst().collideLine(0,tStart,tEnd,0,&cList);
			else if (radius>0.0f && collisionLen==0.0f)
				for (int k=0;k<1000;k++)
					obj->getShapeInst().collideSphere(0,tStart,radius,0,&cList);
			else if (numShapes>1)
			{
				TMat3F tmpMat(*shapes[1]->getRot(),*shapes[1]->getPos());
				tmpMat.inverse();
				TMat3F fromThisShape;
				m_mul(TMat3F(*obj->getRot(),*obj->getPos()),tmpMat,&fromThisShape);
				for (int k=0;k<1000;k++)
					collision = obj->getShapeInst().collideShape(0,0,
						shapes[1]->getShapeInst(),&fromThisShape,&cList,false);
			}
			DWORD elapsed1=GetTickCount()-start1;
			console->setIntVariable("Coll::time1",elapsed1);
		}
		console->setIntVariable("Coll::timeCheck",0);

		Point3F hitPoint;
		if (collision && collisionLen==0.0f)
		{
			console->setIntVariable("Coll::collisionList",
					cList.size());
		}
		if (collision && collisionLen>0.0f)
		{
			// first get the collision point
			float hitTime=cList[0].time;
			hitPoint = el;
			hitPoint -= sl;
			hitPoint *= hitTime;
			hitPoint += sl;

			boxShape->setPos(hitPoint);
			boxShape->setVisible(true);

			// shrink box down
			float boxRad=boxShape->getSphere();
			float objRad=obj->getSphere();
			float boxScale;
			if (radius>0.0f)
				boxScale = radius / boxRad; 
			else
				boxScale = .015f * objRad/boxRad;
			boxShape->setScale(Point3F(boxScale,boxScale,boxScale));
		}
		else
		{
			boxShape->setVisible(false);
			hitPoint.set(0.0f,0.0f,0.0f);
		}
		console->setIntVariable("Coll::collide",collision);
		console->setFloatVariable("Coll::hit.x",hitPoint.x);
		console->setFloatVariable("Coll::hit.y",hitPoint.y);
		console->setFloatVariable("Coll::hit.z",hitPoint.z);
	}
	const Point3F & pos = showCam->getPos();
   console->setFloatVariable("Showcam::x",pos.x);
   console->setFloatVariable("Showcam::y",pos.y);
   console->setFloatVariable("Showcam::z",pos.z);

	console->setFloatVariable("shucks",shucks);

   //    9/98.
   // rotate the camera using these console variables.  this is a quick fix to get 
   // the camera rotating, not sure how to do it with the postAction() commands 
   // which are different now.  
   if( xrotIncrement!=0.0f || zrotIncrement!=0.0f || distanceIncrement!=0.0f )
   {   
      EulerF orbit = showCam->getOrbitRot();
      orbit.x += xrotIncrement;
      orbit.z += zrotIncrement;
      showCam->setOrbitRot(orbit);
      
      float dist = showCam->getDistance();
      dist += distanceIncrement;
      showCam->setDistance(dist);
   }
}

//----------------------------------------------------------------------------

void showPlugin::setShape(int newCurrent)
{
	if ((newCurrent>=1)&&(newCurrent<=numShapes)) {
	   curShape=newCurrent;
	   showCam->setActor(shapes[curShape]);
	   showCam->setSphere(shapes[curShape]->getSphere());
	   showCam->setOffset(shapes[curShape]->getCenter());
      console->setVariable( "ActionTarget", shObjNames[curShape] );
	   shapeDBox->setObj(shapes[curShape]);
	   cameraAttached=true;
	}
	else {
	  curShape=0;
	  showCam->setActor(0);
	  shapeDBox->setObj(0);
	  cameraAttached=false;
	}
}

// test if a is a substring of b
bool ainb(char *a,char *b)
{
   int i;
   int j=0;
   for (i=0;i < (int)strlen(b);i++)
   {
	   if (a[j]==b[i]) j++;
	   if (a[j]=='\0') break;
   }
   return (a[j]=='\0');
}

// ainb() was used for this purpose below but didn't work in certain cases:
//       ainb() thinks "art" is part of the path:  d:\show\exe;..\art
bool path_in_path ( char * full_path, char * other )
{
   char  *cp;
   while ( (cp = strchr ( full_path, ';' )) != NULL )
      if ( strnicmp ( full_path, other, cp - full_path ) == 0 )
         return true;
      else
         full_path = cp + 1;

   return stricmp ( full_path, other ) == 0;
}

bool showPlugin::parseInt(const char *str,const char *cmdName,int &i)
{
  char *endPtr;
  i = strtol(str,&endPtr,10);
  if (*endPtr!='\0')
  {
    console->printf("%s: cannot convert %s to integer",cmdName,str);
	 return false;
  }
  return true;
}

bool showPlugin::parseFloat(const char *str,const char *cmdName,float &f)
{
  char *endPtr;
  f = (float) strtod(str,&endPtr);
  if (*endPtr!='\0')
  {
    console->printf("%s: cannot convert %s to number",cmdName,str);
	 return false;
  }
  return true;
}

bool showPlugin::warnOnDetached(myGuy *obj,char *cmdstr)
{
	if (obj!=NULL)
		return false;
	console->printf("%s: not allowed when detached",cmdstr);
	return true;
}

bool showPlugin::seqInRange(myGuy *obj,int seqNum,char *cmdstr)
{
	if (seqNum<obj->numAnimSeq())
		return true;

	console->printf("%s: there is no sequence #%i",cmdstr,seqNum);
	return false;
}

const char * showPlugin::consoleCallback(CMDConsole *, int id, int argc, const char *argv[])
{
   myGuy      *obj;

	obj = dynamic_cast<myGuy *>(showCam->getActor());

    // The passed-in ID will tell us what type of command 
    // we wish to execute
	switch(id)
	{
		case ShowToolLoad:
		{
         char        *lpszTmp, *lpszEnd;
         char        szFullShapePath[256];
         FileRStream testFile;
         char        *lpszShapeName, szShapePath[256];
         char        szSearchPath[256], *lpszNewSearchPath;

         // Check for basic proper syntax
         if (argc != 2)
         {
            console->printf("view ( shapeName );");
            break;
         }

         // Make sure we haven't loaded too many shapes
         if (numShapes >= MAX_SHAPES)
         {
            console->printf("shape buffer is full");
            break;
         }

         // If the given shape name does not end in .DTS, tag the
         // file extension onto the end
         if (!ainb(".dts", (char *)argv[1]) && !ainb(".DTS", (char *)argv[1]))
         {
            lpszShapeName = new char[strlen(argv[1]) + 5];
            wsprintf(lpszShapeName, "%s.DTS", argv[1]);
         }
         else
         {
            lpszShapeName = new char[strlen(argv[1]) + 1];
            strcpy(lpszShapeName, argv[1]);
         }

         // Keep track of this shape's number
         curShape = ++ numShapes;

         // Get the default search path from the console environment
         strcpy(szSearchPath, (char *)console->
                getVariable("ConsoleWorld::DefaultSearchPath"));

         // Check to see if they gave us an absolute path.  If so, we 
         // don't have to worry about parsing the default paths
         if (testFile.open(lpszShapeName))
         {
            testFile.close();

            // If they gave us an absolute path, add it to the default
            // path string (if it's not already there), then we need to use 
            //    name without the path.  
            strcpy(szShapePath, lpszShapeName);
            
            char *  lastSlash = strrchr(lpszShapeName, '\\');
            
            if( lastSlash != NULL )
            {
               lastSlash = & szShapePath [ lastSlash - lpszShapeName ];
               *lastSlash = '\0';

               // copy the new name without absolute path.                 
               strcpy( lpszShapeName, lastSlash + 1 );

               // If this path is not already in the default path list ...
               // if (!ainb(szShapePath, szSearchPath))
               if ( ! path_in_path ( szSearchPath, szShapePath ) )
               {
                    // Create a buffer big enough to hold the concatenated
                    // new path (old + ';' + new + '\0')
                    lpszNewSearchPath = new char[strlen(szSearchPath) + 
                                                 strlen(szShapePath)  + 2];

                    // Format the path
                    wsprintf(lpszNewSearchPath, 
                             "%s;%s", szSearchPath, szShapePath);

                    // And set the default path
                    console->setVariable("ConsoleWorld::DefaultSearchPath",
                                         lpszNewSearchPath);

                    // Free up the memory
                    delete [] lpszNewSearchPath;
                }
             }
         }

         // Otherwise, try to auto-complete the absolute file name from
         // the default directory listing
         else
         {
            // If there is no semicolon on the end of the line, we need
            // to add one
            szSearchPath[strlen(szSearchPath) - 1] = ';';

            // Run through the list of default paths, to see if we can
            // locate the directory in which the requested file resides
            for (lpszTmp = szSearchPath; 
                 (lpszEnd = strchr(lpszTmp,  ';')) != NULL;
                  lpszTmp = lpszEnd + 1)
            {
               *lpszEnd = '\0';
               wsprintf(szFullShapePath, "%s\\%s", lpszTmp, lpszShapeName);

               // If we can open the file, we have the correct path
               if (testFile.open(szFullShapePath))
               {
                  testFile.close();

                  // Copy the completed filename into the shape name
                  // variable, for later use
                  delete [] lpszShapeName;
                  lpszShapeName = new char[strlen(szFullShapePath) + 1];
                  strcpy(lpszShapeName, szFullShapePath);
                  break;
               }
            }
         }

         bool AllowMultipleShapes = console->getBoolVariable("AllowMultipleShapes");

         if(!AllowMultipleShapes)
         {
             myGuy *oldObj = dynamic_cast<myGuy *>(manager->findObject("showObj"));
             
             if(oldObj)
             {
                 manager->deleteObject(oldObj);
             }
         }

         myGuy* obj = new myGuy(lpszShapeName);
         manager->addObject(obj);

         if (obj->shape()) 
         {
            // make name for shape object and give it to manager
            shObjNames[curShape] = new char[15];

            bool AllowMultipleShapes = console->getBoolVariable( "AllowMultipleShapes" );
            if( AllowMultipleShapes )
            {
               strcpy(shObjNames[curShape],"showObj");
               itoa(curShape,shObjNames[curShape]+strlen(shObjNames[curShape]),10);
            }
            else
            {
               strcpy(shObjNames[curShape],"showObj");
            }

            manager->assignName(obj,shObjNames[curShape]);

            // save the shape name (sic. file name)
            shapeNames[curShape] = lpszShapeName;

            // save a pointer to the object
            shapes[curShape] = obj;

            // use tripod object (showCam) to determine
            // rotation and position of object
            const Point3F & pos = showCam->getPos();
            const EulerF & rot = showCam->getRot();
            TMat3F transMat;
            transMat.set(rot,pos);
            Point3F newShapePos;
            if (showCam->getDistance()==0.0f) {
            	showCam->setDistance(2.0f*shapes[curShape]->getSphere());
            }

            m_mul(Point3F(0.0f,showCam->getDistance(),0.0f),transMat,&newShapePos);

            if (showCam->hasActor()){
            	newShapePos.x-=2.0f*shapes[curShape]->getSphere();
            }

            obj->setPos(newShapePos);
            obj->setRot(EulerF(0.0f,0.0f,rot.z));

            if (showCam->getRelative()){
            	showCam->setOrbitRot(EulerF(rot.x,rot.y,0.0f));
            }

            // stick to ground by default
            obj->setStickToGround(true);

            // set the current shape
            setShape(curShape);

            // if shape has at least 1 anim sequence, automatically create a thread
            if (shapes[curShape]->numAnimSeq()!=0)
            {
            	char *cmdArg[1];
            	cmdArg[0]="newThread";
            	console->execute(1,(const char **) cmdArg);
            }
         }
         else
         {
            console->printf("view: unable to load shape \"%s\"", lpszShapeName);
            numShapes--;
            setShape(0);
            manager->removeObject(obj);
         }

         delete [] lpszShapeName;
         break;
      }

      case ShowToolSaveShape:
      {
         if (warnOnDetached(obj,"saveShape"))
            break;

			if ((argc!=1)&&(argc!=2))
			{
				console->printf("saveShape: [fileName]");
				break;
			}

			const char *fileName;
			if (argc==1)
				fileName=shapeNames[curShape];
			else
				fileName=argv[1];

			// save the file
			FileRWStream shapeFile;
			shapeFile.open(fileName);
			obj->getShape().store(shapeFile);
			shapeFile.close();
			break;
		}
		case ShowToolImportTransitions:
		{
			if (warnOnDetached(obj,"importTransitions"))
				break;

			if (argc!=2)
			{
				console->printf("importTransitions:  fileName");
				break;
			}

			const char *fileName = argv[1];

			Persistent::Base::Error err;
			TSShape *tempShape; // load in shape and then read off transitions
			FileRWStream shapeFile;
			shapeFile.open(fileName);
			tempShape = static_cast<TSShape *>(TSShape::load(shapeFile,&err));
			shapeFile.close();

			if (err)
			{
				console->printf("importTransitions:  error loading shape file");
				break;
			}

			// get the current transition list
			Vector<TSShape::Transition> &transList = 
				const_cast< Vector<TSShape::Transition> & >(obj->getTransitionList());
			// get the transition list from disk
			Vector<TSShape::Transition> const &transListDisk = tempShape->fTransitions;

			// update transitions
			for (int i=0;i<transListDisk.size();i++)
				transList.push_back(transListDisk[i]);

			// give user feedback
			console->printf("%i transitions imported",transListDisk.size());
			break;
		}
		case ShowToolImportSequence:
		{
			if (warnOnDetached(obj,"importSequence"))
				break;

			if (argc!=3)
			{
				console->printf("importSequence:  fileName  sequence-number");
				break;
			}

			const char *fileName = argv[1];

			Persistent::Base::Error err;
			TSShape *impShape; // load in shape
			FileRWStream shapeFile;
			shapeFile.open(fileName);
			impShape = static_cast<TSShape *>(TSShape::load(shapeFile,&err));
			shapeFile.close();

			if (err)
			{
				console->printf("importSequence:  error loading shape file");
				break;
			}

			int impSeqNum;
			if (!parseInt(argv[2],"importSequence",impSeqNum))
				break;

			if ( (impSeqNum<0) || (impSeqNum>=impShape->fSequences.size()) )
			{
				console->printf("importSequence:  illegal sequence number");
				break;
			}

			if (!const_cast<TSShape &>(obj->getShape()).importSequence(impShape,impSeqNum))
			{
				console->printf("importSequence:  can't import from this shape");
				break;
			}

			// update some sequence information on the shape instance
			obj->getShapeInst().UpdateSequenceSubscriberLists();

			// set all threads to seq 0 because sequences may have changed address...
			for (int i=0;i<obj->getNumThreads();i++)
			{
				obj->selectThread(i);
				obj->setAnimSeq(0);
			}
			if (obj->getNumThreads()>0)
				obj->selectThread(0);

			break;
		}
 		case ShowToolGoto:
 		{
			if (argc==2) {
				int newShapeNum=atoi(argv[1]);
				if ((newShapeNum>=1) && (newShapeNum<=numShapes)) {
					curShape = newShapeNum;
					setShape(curShape);
				}
				else {
					if (numShapes==0)
					  console->printf("goto: no shapes loaded");
					else
					  console->printf("goto: shape# must be between 1 and %i",numShapes);
				}				
			}
			else
				console->printf("goto: loaded-shape#");
			break;
		}
		case ShowToolList:
		{
			if (argc==1) {
				console->printf("        shape #      shape file");
				int i;
				for (i=1;i<=numShapes;i++)
				  if (i!=curShape)
					console->printf("          %i          %s",i,shapeNames[i]);
				  else
					console->printf("          %i          %s*",i,shapeNames[i]);
			}
			else
				console->printf("list: takes no paramters");
			break;
		}
		case ShowToolLight:
		{
			if (argc==1) {
				// get current tripod object rotation, build rotString
				const EulerF & rot = showCam->getRot();
				Vector3F lightDir;
				m_mul(Vector3F(0.0f,1.0f,0.0f),RMat3F(rot),&lightDir);
				char *cmdargv[10];
            
//				cmdargv[0]="newObject \"lite1\" SimLight Directional 1 1 1 1 0 0"; 
				cmdargv[0]="newObject"; 
				cmdargv[1]= new char[32];
				strcpy(cmdargv[1],"light");
				itoa(++showLightNum,cmdargv[1]+strlen(cmdargv[1]),10);
				cmdargv[2]= new char[10];
				strcpy(cmdargv[2], "SimLight");
				cmdargv[3]= new char[12];
				strcpy(cmdargv[3], "Directional");
				cmdargv[4] = "1.0";
				cmdargv[5] = "1.0";
				cmdargv[6] = "1.0";
				cmdargv[7] = new char[5];
				itoa(floor(lightDir.x*1000),cmdargv[7],10);
				cmdargv[8] = new char[5];
				itoa(floor(lightDir.y*1000),cmdargv[8],10);
				cmdargv[9] = new char[5];
				itoa(floor(lightDir.z*1000),cmdargv[9],10);
				console->execute(10,(const char **)cmdargv);
				delete [] cmdargv[1];
				delete [] cmdargv[2];
				delete [] cmdargv[3];
				delete [] cmdargv[7];
				delete [] cmdargv[8];
				delete [] cmdargv[9];
			}
			else
				console->printf("light: takes no paramters");
			break;
		}
		case ShowToolDetach:
		{
			if (argc==1)
				setShape(0);
			else
				console->printf("detach: takes no paramters");
			break;
		}
		case ShowToolNext:
		{
			if (argc==1) {
				if (curShape++==numShapes) curShape=1;
				setShape(curShape); // setShape will set to 0 if no shapes
			}
			else
				console->printf("next: takes no paramters");
			break;
		}
		case ShowToolSetDetail:
		{
			if (warnOnDetached(obj,"setDetail"))
				break;

			if (argc==1)
				obj->setDetailLevel(0,1000); // will be set automatically before render
			else if (argc==2)
			{
				int ndl;
				if (!parseInt(argv[1],"setDetail",ndl))
					break;

				if (obj->numDetailLevels()<=ndl)
					console->printf("setDetail: only %i detail levels for this shape",obj->numDetailLevels());
				else
					obj->setDetailLevel(ndl);
			}
			else if (argc==3)
			{
				int mind,maxd;
				if (!parseInt(argv[1],"setDetail",mind))
					break;
				if (!parseInt(argv[2],"setDetail",maxd))
					break;
				obj->setDetailLevel(mind,maxd);
			}
			else
			{
				console->printf("setDetail: no parameter (to choose based on distance), or");
				console->printf("setDetail: detailLevel, or");
				console->printf("setDetail: minDetail maxDetail");
			}
		   break;
		}
		case ShowToolGetDetail:
		{
			if (warnOnDetached(obj,"getDetail"))
				break;

			if (argc==1)
			{
				int dlevel = obj->getDetailLevel();
				int numLevels = obj->numDetailLevels();
				if (numLevels==1)
					console->printf("detail level: 0 (only one)");
				else
					console->printf("detail level: %i (0-%i)",dlevel,numLevels-1);
			}
			else
				console->printf("getDetail: takes no parameters");
		   break;
		}
		case ShowToolNewThread :
		{
			if (warnOnDetached(obj,"newThread"))
				break;

			if (argc==1)
			{
				int threadNum = obj->newThread();
				console->printf("new thread #%i",threadNum);
			}
			else
				console->printf("newThread:  takes no parameters");
			break;
		}
		case ShowToolSelectThread :
		{
			if (warnOnDetached(obj,"selectThread"))
				break;
	
			if (argc==2)
			{
				int threadNum;
				if (!parseInt(argv[1],"selectThread",threadNum))
					break;
				obj->selectThread(threadNum);
			}
			else
				console->printf("selectThread: thread-number");
			break;
		}
		case ShowToolListThreads :
		{
			if (warnOnDetached(obj,"listThreads"))
				break;

			if (argc!=1)
			{
				console->printf("listThreads:  takes no parameters");
				break;
			}

			console->printf("Animation threads for current shape:");
			console->printf("  Thread #    Sequence #     Position");
			int recallThreadIndex = obj->getCurThreadIndex();
			for (int i=0;i<obj->getNumThreads();i++)
			{
				obj->selectThread(i);
				char marker=' ';
				if (i==recallThreadIndex)
					marker='*';
				int tranState = obj->getState();
				char sCh;
				if (tranState==TSShapeInstance::Thread::InTransition)
					sCh = 'T';
				else if (tranState==TSShapeInstance::Thread::TransitionPending)
					sCh = 'P';
				else
					sCh = ' ';
				console->printf("%c    %2i        %2i         %.1f out of %.1f    %c",
				  sCh,i,obj->getAnimSeq(),obj->getAnimPos(),obj->getAnimDur(),marker
				);
			}
			obj->selectThread(recallThreadIndex);
			break;
		}
		case ShowToolNumSequences :
		{
			if (warnOnDetached(obj,"numSequences"))
				break;

			if (argc==1)
				console->printf("%i sequences",obj->numAnimSeq());
			else
				console->printf("numSequences: takes no parameters");
			break;
		}
		case ShowToolSetAnimSpeed :
		{
			if (warnOnDetached(obj,"setAnimSpeed"))
				break;

			if (argc==2)
			{
				float newSpeed;
				if (!parseFloat(argv[1],"setAnimSpeed",newSpeed))
					break;
				obj->setAnimSpeed(newSpeed);
			}
			else
				console->printf("setAnimSpeed: speed");
			break;
		}
		case ShowToolGetAnimSpeed :
		{
			if (warnOnDetached(obj,"getAnimSpeed"))
				break;

			if (argc==1)
				console->printf("animation speed: %f",obj->getAnimSpeed());
			else
				console->printf("getAnimSpeed: takes no parameters");
			break;
		}
		case ShowToolSetSequence :
		{
			if (warnOnDetached(obj,"setSequence"))
				break;

			if (obj->getNumThreads()==0)
				console->printf("setSequence: shape has no threads");
			else if (argc==2)
			{
				int seqNum;
				if (!parseInt(argv[1],"setSequence",seqNum))
					break;

				if (seqInRange(obj,seqNum,"setSequence"))
					obj->setAnimSeq(seqNum); // on currently selected thread
			}
			else
				console->printf("setSequence: sequence-number");
			break;
		}
		case ShowToolListTransitions :
		{
			if (warnOnDetached(obj,"listTransitions"))
				break;

			if (argc!=1)
			{
				console->printf("listTransitions:  takes no parameters");
				break;
			}

			console->printf("Transitions for current shape:");	
			console->printf("  #  From   To  Duration        Transform");
			for (int i=0;i<obj->getTransitionList().size();i++)
			{
				TSShape::Transition const *trans = &(obj->getTransitionList()[i]);
				QuatF q;
				const Point3F &pt = trans->fTransform.getTranslate();
				trans->fTransform.getRotate( &q );
				console->printf(" %2i  (%i,%.1f)  (%i,%.1f)   %.1f    T=(%.1f,%.1f,%.1f) R=(%.1f,%.1f,%.1f,%.1f)",
					i,trans->fStartSequence,trans->fStartPosition,trans->fEndSequence,trans->fEndPosition,trans->fDuration,
					pt.x, pt.y, pt.z,  q.x, q.y, q.z, q.w );
			}
			break;
		}
		case ShowToolNewTransition :
		{
			if (warnOnDetached(obj,"newTransition"))
				break;

			if (argc!=1)
			{
				console->printf("newTransition:  takes no parameters");
				break;
			}
			
			TSShape::Transition newTran;
			newTran.fStartSequence=newTran.fEndSequence=obj->getAnimSeq();
			newTran.fStartPosition=0.0f;
			newTran.fEndPosition=0.0f;
			newTran.fDuration=0.1f;
			newTran.fTransform.identity();

//#pragma warn -ncf
//			obj->getTransitionList().push_back(newTran);
//#pragma warn .ncf
			break;
		}
		case ShowToolEditTransition :
		{
			if (warnOnDetached(obj,"editTransition"))
				break;

			if ((argc!=7) && (argc!=10) && (argc!=14))
			{
				console->printf("editTransition: # startSeq endSeq startPos endPos dur");
				break;
			}

			int transNum;
			if (!parseInt(argv[1],"editTransition",transNum))
				break;
			if (transNum>=obj->getTransitionList().size())
			{
				console->printf("editTransition: transition #%i does not exist",transNum);
				break;
			}

			int startSeq;
			if (!parseInt(argv[2],"editTransition",startSeq))
				break;
			if (!seqInRange(obj,startSeq,"editTransition (startSequence)"))
				break;

			int endSeq;
			if (!parseInt(argv[3],"editTransition",endSeq))
				break;
			if (!seqInRange(obj,endSeq,"editTransition (endSequence)"))
				break;

			float startPos;
			if (!parseFloat(argv[4],"editTransition",startPos))
				break;
			if ( (startPos<0.0f) || (startPos>1.0f) )
			{
				console->printf("editTransition:  startPos out of range");
				break;
			}

			float endPos;
			if (!parseFloat(argv[5],"editTransition",endPos))
				break;
			if ( (endPos<0.0f) || (endPos>1.0f) )
			{
				console->printf("editTransition:  endPos out of range");
				break;
			}

			float duration;
			if (!parseFloat(argv[6],"editTransition",duration))
				break;
			if (duration<0.0f)
			{
				console->printf("editTransition:  duration must be positive");
				break;
			}

			float x=0.0f;
			float y=0.0f;
			float z=0.0f;
			if (argc>7)
			{
				// get translation part of transform from command line
				if (!parseFloat(argv[7],"editTransition",x))
					break;
				if (!parseFloat(argv[8],"editTransition",y))
					break;
				if (!parseFloat(argv[9],"editTransition",z))
					break;
			}

			float qx=0.0f;
			float qy=0.0f;
			float qz=0.0f;
			float qw=1.0f;
			if (argc>10)
			{
				// get quaternion part of transform from command line
				if (!parseFloat(argv[10],"editTransition",qx))
					break;
				if (!parseFloat(argv[11],"editTransition",qy))
					break;
				if (!parseFloat(argv[12],"editTransition",qz))
					break;
				if (!parseFloat(argv[13],"editTransition",qw))
					break;
			}

			TSShape::Transition * trans = const_cast< TSShape::Transition *>	
				(&(obj->getTransitionList()[transNum]));
			trans->fStartSequence=startSeq;
			trans->fEndSequence=endSeq;
			trans->fStartPosition=startPos;
			trans->fEndPosition=endPos;
			trans->fDuration=duration;
			trans->fTransform.setTranslate( Point3F( x, y, z ) );
			trans->fTransform.setRotate( QuatF( qx, qy, qz, qw ) );
			shapeDBox->updateTransitions();  // this is the one place we need to talk to the
														// dialoge box.  It notices when the # of transitions
														// change and when the set transitions change,
														// but not when the contents of a transition change
			break;
		}
		case ShowToolTransitionToSeq :
		{
			if (warnOnDetached(obj,"transitionTo"))
				break;

			if (argc!=2)
			{
				console->printf("transitionTo:  newSeq");
				break;
			}

			int newSeq;
			if (!parseInt(argv[1],"transitionTo",newSeq))
				break;

			if (!seqInRange(obj,newSeq,"transitionTo"))
				break;

			obj->transitionTo(newSeq);
			break;
		}

		case ShowToolSetTransition :
		{
			if (warnOnDetached(obj,"setTransition"))
				break;

			if (obj->getNumThreads()==0)
				console->printf("setTransition: shape has no threads");

			if (argc!=2)
			{
				console->printf("setTransition:  transitionNum");
				break;
			}

			int transNum;
			if (!parseInt(argv[1],"setTransition",transNum))
				break;
			if (transNum>=obj->getTransitionList().size())
			{
				console->printf("setTransition:  transition #%i does not exist",transNum);
				break;
			}
			if ( (obj->getTransitionList()[transNum].fStartSequence != obj->getAnimSeq()) &&
				  (obj->getTransitionList()[transNum].fEndSequence != obj->getAnimSeq()) )
			{
				console->printf("setTransition:  transition must start or end in current sequence");
				break;
			}

			obj->setTransition(transNum);

			break;
		}
		case ShowToolAnimationPause :
		{
			if (!obj)
				// pause everyone
				for (int i=0;i<numShapes;i++)
					shapes[i]->pause();
			else if (argc==1)
				// just pause this guy
				shapes[curShape]->pause();
			else
				console->printf("pause: takes no parameters");
			break;
		}
		case ShowToolAnimationPlay :
		{
			if (!obj)
				// play everyone
				for (int i=0;i<numShapes;i++)
					shapes[i]->play();
			else if (argc==1)
				// just play this guy
				obj->play();
			else
				console->printf("play: takes no parameters");
			break;
		}
		case ShowToolAnimationStep :
		{
			float increment;
			if (argc==1)
				increment = defaultAnimationIncrement;
			else if (argc==2)
			{
				if (!parseFloat(argv[1],"step",increment))
					break;
			}
			else
			{
				console->printf("step: increment (optional)");
				break;
			}

			if (!obj)
				// step everyone
				for (int i=0;i<numShapes;i++)
					shapes[i]->stepThread(increment);
			else
				obj->stepThread(increment);
			break;
		}
		case ShowToolGetAnimDur :
		{
			if (warnOnDetached(obj,"getDuration"))
				break;

			if (obj->getNumThreads()==0)
			{
				console->printf("getDuration: no animation threads");
				break;
			}

			if (argc!=1)
			{
				console->printf("getDuration: takes no parameters");
				break;
			}
			
			console->printf("animation sequence %i duration:  %f",obj->getAnimSeq(),obj->getAnimDur());
			break;
		}
		case ShowToolSetAnimPos :
		{
			if (warnOnDetached(obj,"setAnimPos"))
				break;

			if (obj->getNumThreads()==0)
			{
				console->printf("setAnimPos: no animation threads");
				break;
			}

			if (argc>2)
			{
				console->printf("setAnimPos:  new-pos (optional)");
				break;
			}

			if (argc==1)
				obj->setAnimPos(0.0f);
			else
			{
				float newPos;
				if (!parseFloat(argv[1],"setAnimPos",newPos))
					break;
				if ((newPos<0.0f) || (newPos>1.0f))
				{
					console->printf("setAnimPos:  position out of range");
					break;
				}
				obj->setAnimPos(newPos);
			}
			break;
		}
		case ShowToolGetAnimPos :
		{
			if (warnOnDetached(obj,"getAnimPos"))
				break;

			if (obj->getNumThreads()==0)
			{
				console->printf("getAnimPos: no animation threads");
				break;
			}
			if (argc!=1)
			{
				console->printf("getAnimPos: takes no parameters");
				break;
			}

			float curPos;
			curPos = obj->getAnimPos();
			console->printf("current animation position: %f",curPos);
			break;
		}
		case ShowToolSetRelative :
		{
			if (argc!=2)
			{
				console->printf("setRelative:  0 | 1");
				break;
			}

			int val;
			if (!parseInt(argv[1],"setRelative",val) || val!=0)
				showCam->setRelative(true);
			else
				showCam->setRelative(false);
			break;
		}
		case ShowToolSetStickToGround :
		{
			if (argc!=2)
			{
				console->printf("stickToGround:  0 | 1");
				break;
			}

			int val;
			if (!parseInt(argv[1],"stickToGround",val) || val!=0)
				obj->setStickToGround(true);
			else
				obj->setStickToGround(false);
			break;
		}
		case ShowToolShadows :
		{
			if (argc!=2)
			{
				console->printf("shadows:  0 | 1");
				break;
			}

			int val;
			if (!parseInt(argv[1],"shadows",val) || val!=0)
				castingShadows=true;
			else
				castingShadows=false;
			break;
		}
		case ShowToolShadowLift :
		{
			if (argc!=2)
			{
				console->printf("shadowLift:  height");
				break;
			}

			float fval;
			if (!parseFloat(argv[1],"shadowLift",fval))
				break;

			shadowLift = fval;
			break;
		}
		case ShowToolShadowLight :
		{
			if (argc!=3)
			{
				console->printf("shadowLight:  x y");
				break;
			}
			float fval1,fval2;
			if (!parseFloat(argv[1],"shadowLight",fval1))
				break;

			if (!parseFloat(argv[2],"shadowLight",fval2))
				break;

			worldLightDirection.x=fval1;
			worldLightDirection.y=fval2;
			worldLightDirection.z=-1.0f;

			break;
		}
		case ShowToolShapeDialog :
		{
			if ( (argc!=1) && (argc!=2) )
			{
				console->printf("shapeDialog:  0 | 1");
				break;
			}

			int val;
			if (argc==1 || !parseInt(argv[1],"shapeDialog",val) || val!=0)
				shapeDBox->setHide(false);
			else
				shapeDBox->setHide(true);
			break;
		}
		case ShowToolCamDialog :
		{
			if ( (argc!=1) && (argc!=2) )
			{
				console->printf("camDialog:  0 | 1");
				break;
			}

			int val;
			if (argc==1 || !parseInt(argv[1],"camDialog",val) || val!=0)
				camDBox->setHide(false);
			else
				camDBox->setHide(true);
			break;
		}
		case ShowToolCollisionCheck :
		{
			if (warnOnDetached(obj,"collision"))
				break;

			if (argc!=1)
			{
				console->printf("collision: takes no parameters");
				break;
			}

			if (boxShape)
				break;

			// put box out there as line...
			int oldShapeNum=curShape;
			int oldNumShapes=numShapes;
			char *cmdStr[2];
			cmdStr[0]="view";
			cmdStr[1]="box";
			console->execute(2,(const char **) cmdStr);
			// set shape back
			setShape(oldShapeNum);
			// was box shape actually made?
			if (oldNumShapes==numShapes)
				break;
			checkingCollision=true;
			boxShape=shapes[numShapes--];

			// put the cursor box in motion and set it invisible by default
			boxShape->setRotVel(EulerF(1.5f,1.0f,2.0f));
			boxShape->setVisible(false);
			boxShape->setStickToGround(false);

			break;
		}
		case ShowToolPerformanceCheck :
		{
			if (argc%2==0 || argc==1)
			{
				console->printf("performanceCheck: [distance count]");
				break;
			}

			Vector<float> distToCheck;
			distToCheck.setSize((argc-1)/2);
			Vector<int> timesToCheck;
			timesToCheck.setSize((argc-1)/2);
			int i;
			for (i=0; i<(argc-1)/2; i++)
			{
				if (!parseFloat(argv[2*i+1],"performanceCheck",distToCheck[i]))
					return 0; // can't just break
				if (distToCheck[i]<0.0f)
				{
					console->printf("performanceCheck:  distance must be positive");
					return 0; // can't just break
				}
				if (!parseInt(argv[2*i+2],"performanceCheck",timesToCheck[i]))
					return 0; // can't just break
				if (timesToCheck[i]<0)
				{
					console->printf("performanceCheck:  count must be positive or zero");
					return 0; // can't just break
				}
			}
			float saveDist = showCam->getDistance();
			int totalRenders = 0;
			DWORD startTime = GetTickCount();
			for (i=0; i<(argc-1)/2; i++)
			{
				showCam->setDistance(distToCheck[i]);
				for (int j=0; j<timesToCheck[i]; totalRenders++, j++)
					hackRender();
			}
			DWORD elapsedTime = GetTickCount() - startTime;
			float avg;
			if (totalRenders)
				avg = (float) elapsedTime / (float) totalRenders;
			else
				avg = 0.0f;
			showCam->setDistance(saveDist);

			console->printf("performance check:  %f ms/frame",avg);
			console->setFloatVariable("Perf::msPerFrame",avg);
			break;
		}
	   case GameEndFrame :        // just a stub which sim game expects...
         break;
	   case LoadMainMenu :
         SimGui::Canvas *canvas;
         canvas = dynamic_cast<SimGui::Canvas *>( manager->findObject( "MainWindow" ) );
        
         hMenu = LoadMenu( canvas->getHInstance(), "IDM_MENU1");
         SetMenu( canvas->getHandle(), hMenu );
         SetFocus( canvas->getHandle() );
         break;
	}
	return 0;
}
