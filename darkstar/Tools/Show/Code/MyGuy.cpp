//--------------------------------------------------------------------------- 


//--------------------------------------------------------------------------- 

#include <ts.h>
#include <base.h>
#include <sim.h>

#include "simGame.h"
#include "simAction.h"
// #include "simEv.h"

#include "myGuy.h"
#include "SimTerrain.h"
#include "SimInterior.h"
#include "simResource.h"
#include "simCollision.h"
// #include "simTime.h"

//#include "simClientRep.h"
#include "simPersman.h"

IMPLEMENT_PERSISTENT(myGuy);

//--------------------------------------------------------------------------- 
Point3F worldLightDirection=Point3F(1.0f,1.0f,-1.0f);
float shadowLift = 0.1f;
bool castingShadows = true;
int shucks,shucks2;

// ick
void myGuyRenderImage::setTerrain( const TMat3F & shapeToWorld, const TMat3F & worldToShape )
{
   // clear out any polys from previous round
   shadow.projectionList.clearPolys();

   // we need center of shape in world coords
   Point3F shapeInWorld;
   m_mul(shape->getShape().fCenter,shapeToWorld,&shapeInWorld);

   // shape center in terrain space
   Point3F inTerrSpace;
   GridFile *terrFile = terr->getGridFile();
   m_mul(shapeInWorld,terr->getInvTransform(),&inTerrSpace);

   // get terrain height under shape
   CollisionSurface info;
   if (!terrFile->getSurfaceInfo(Point2F(inTerrSpace.x,inTerrSpace.y),&info))
      return; // not over terrain
   float terrH = info.position.z;

   // get the light direction in terrain space
   Point3F lightInTerrSpace;
   m_mul(shadow.lightInWorld,RMat3F(terr->getInvTransform()),&lightInTerrSpace);
   if (IsEqual(lightInTerrSpace.z,0.0f))
      return; // light parallel to ground

   // project light vector onto ground to get 8 corners of cube
   // use these 8 projected points to find box on terrain
   Point3F corner[8];
   float minX,maxX,minY,maxY;
   float rad = shape->getShape().fRadius * 0.5f;
   int i;
   for (i=0;i<8;i++)
      corner[i] = inTerrSpace;
   corner[0].x += rad; corner[0].y += rad; corner[0].z += rad;
   corner[1].x += rad; corner[1].y += rad; corner[1].z -= rad;
   corner[2].x += rad; corner[2].y -= rad; corner[2].z += rad;
   corner[3].x += rad; corner[3].y -= rad; corner[3].z -= rad;
   corner[4].x -= rad; corner[4].y += rad; corner[4].z += rad;
   corner[5].x -= rad; corner[5].y += rad; corner[5].z -= rad;
   corner[6].x -= rad; corner[6].y -= rad; corner[6].z += rad;
   corner[7].x -= rad; corner[7].y -= rad; corner[7].z -= rad;
   for (i=0;i<8;i++)
   {
      float t = (terrH - corner[i].z)/lightInTerrSpace.z; // light can't be parallel to the ground
      Point3F projV = lightInTerrSpace;
      projV *= t;
      projV += corner[i];
      if (i==0)
      {
         minX=projV.x;
         maxX=projV.x;
         minY=projV.y;
         maxY=projV.y;
      }
      else
      {
         if (projV.x<minX)
            minX=projV.x;
         else if (projV.x>maxX)
            maxX=projV.x;
         if (projV.y<minY)
            minY=projV.y;
         else if (projV.y>maxY)
            maxY=projV.y;
      }
   }

   // we have our box in terrain space...
   Box2F shadowBox;
   shadowBox.fMin.x = minX;
   shadowBox.fMin.y = minY;
   shadowBox.fMax.x = maxX;
   shadowBox.fMax.y = maxY;

   // now get polys from terrain to project shadow onto
   TMat3F   terrToShape;
   m_mul(terr->getTransform(),worldToShape,&terrToShape);
   terrFile->getPolys(shadowBox,terrToShape,shadow.projectionList,300);
   shucks = shadow.projectionList.size();
}

#if 0
void myGuyRenderImage::setItr( const TMat3F & shapeToWorld, const TMat3F & worldToShape )
{
   // clear out any polys from previous round
   shadow.projectionList.clearPolys();

   // get center of shape
   Point3F shapeCenter = shape->getShape().fCenter;

   // get light in shape space, and project top of bb to ground
   Point3F lightInShape;
   m_mul(shadow.lightInWorld,(RMat3F &) worldToShape,&lightInShape);
   if (lightInShape.z==0.0f)
      return; // don't allow parallel light
   Point3F corner[4];
   float rad = shape->getShape().fRadius *0.5f;
   corner[0] = shapeCenter;
   corner[0].z += rad;
   int i;
   for (i=1;i<4;i++)
      corner[i]=corner[0];
   corner[0].x += rad; corner[0].y += rad;
   corner[1].x += rad; corner[1].y -= rad;
   corner[2].x -= rad; corner[2].y += rad;
   corner[3].x -= rad; corner[3].y -= rad;
   float minX,minY,maxX,maxY;
   minX=maxX=corner[0].x;
   minY=maxY=corner[0].y;
   for (i=1;i<4;i++)
   {
      if (corner[i].x<minX)
         minX=corner[i].x;
      else if (corner[i].x>maxX)
         maxX=corner[i].x;
      if (corner[i].y<minY)
         minY=corner[i].y;
      else if (corner[i].y>maxY)
         maxY=corner[i].y;
   }   
   // project onto plane w/ normal (0,0,1) and plane constant 0 (assumes feet at 0 z-coord)
   for (i=0;i<4;i++)
   {
      float t = - corner[i].z/lightInShape.z; // light can't be parallel to the ground
      Point3F projV = lightInShape;
      projV *= t;
      projV += corner[i];
      if (projV.x<minX)
         minX=projV.x;
      else if (projV.x>maxX)
         maxX=projV.x;
      if (projV.y<minY)
         minY=projV.y;
      else if (projV.y>maxY)
         maxY=projV.y;
   }

   // now make our box
   Box3F box;
   box.fMin.x=minX;
   box.fMax.x=maxX;
   box.fMin.y=minY;
   box.fMax.y=maxY;
   box.fMin.z=shapeCenter.z - 2.0f * rad; // make sure we find the ground
   box.fMax.z=shapeCenter.z + rad;

   TMat3F transBack;
   m_mul(itr->getTransform(),worldToShape,&transBack);

   TMat3F toItr;
   m_mul(shapeToWorld,itr->getInvTransform(),&toItr);

//   Point3F directionInItrSpace;
//   m_mul(lightDirection,(RMat3F &)itr->getInvTransform(),&directionInItrSpace);

   itr->getInstance()->getPolys(box,toItr,transBack,shadow.projectionList);

shucks2 = shadow.projectionList.size();
}
#endif


void myGuyRenderImage::render(TSRenderContext &rc)
{
   rc.getCamera()->pushTransform(transform);
   currentDetail=shape->selectDetail(rc,minDetail,maxDetail);
   shape->animate();
   shape->render(rc);

#if CAST_SHADOWS
   if (castShadowTerr)     // || castShadowItr)
   {
      shadow.shadowLift=shadowLift;
      shadow.getShape( currentDetail, shape );
      shadow.setLight( lightDirection, (RMat3F &) invTransform );
      if (castShadowTerr)
      {
         setTerrain( transform, invTransform );
         shadow.render( rc, 0 );
      }
      // else
      // {
      //    setItr( transform, invTransform );
      //    shadow.render( rc, 0 );
      // }
   }
#endif
   
   rc.getCamera()->popTransform();
   
   if( renderBounds )
      renderBoundingBox( rc );
}



bool myGuyRenderImage::renderBounds = false;

void myGuyRenderImage::renderBoundingBox(TSRenderContext& rc)
{
   if( shape != NULL )
   {
      TS::PointArray* pArray = rc.getPointArray();
      Point3F bboxPts[8];
      
      Box3F bBox = shape->getShape().fBounds, boundsBox;
      // boundsBox.fMin += transform.p;
      // boundsBox.fMax += transform.p;
      m_mul ( bBox.fMin, transform, &boundsBox.fMin );
      m_mul ( bBox.fMax, transform, &boundsBox.fMax );
      // boundsBox = bBox;
      
      // Draw container bbox
      bboxPts[0].set( boundsBox.fMin.x, boundsBox.fMin.y, boundsBox.fMin.z );
      bboxPts[1].set( boundsBox.fMin.x, boundsBox.fMax.y, boundsBox.fMin.z );
      bboxPts[2].set( boundsBox.fMin.x, boundsBox.fMax.y, boundsBox.fMax.z );
      bboxPts[3].set( boundsBox.fMin.x, boundsBox.fMin.y, boundsBox.fMax.z );
      bboxPts[4].set( boundsBox.fMax.x, boundsBox.fMin.y, boundsBox.fMin.z );
      bboxPts[5].set( boundsBox.fMax.x, boundsBox.fMax.y, boundsBox.fMin.z );
      bboxPts[6].set( boundsBox.fMax.x, boundsBox.fMax.y, boundsBox.fMax.z );
      bboxPts[7].set( boundsBox.fMax.x, boundsBox.fMin.y, boundsBox.fMax.z );
      
      pArray->reset();
      int start = pArray->addPoints(8, bboxPts);
      
      static int color = 253;
      color = (color + 1) & 0xff;
      pArray->drawLine(start + 0, start + 1, color);
      pArray->drawLine(start + 1, start + 2, color);
      pArray->drawLine(start + 2, start + 3, color);
      pArray->drawLine(start + 3, start + 0, color);
      pArray->drawLine(start + 4, start + 5, color);
      pArray->drawLine(start + 5, start + 6, color);
      pArray->drawLine(start + 6, start + 7, color);
      pArray->drawLine(start + 7, start + 4, color);
      pArray->drawLine(start + 0, start + 4, color);
      pArray->drawLine(start + 1, start + 5, color);
      pArray->drawLine(start + 2, start + 6, color);
      pArray->drawLine(start + 3, start + 7, color);
   }
}




void myGuy::init()
{
   shape_name = NULL;
   ghost_name = NULL;
   image.shape = NULL;
   killRootTransform=false;
   isScaled=false;
   visible=true;
   numThreads=0;
   curThread=0;

   groupId = 0;
   managerId = -1;

   // mover automatically initialized
}

myGuy::myGuy(const char* shapeFile, const char *ghostName,
                       int manager_id, int ghost_group_id, int group_id )
{
   init();
   shape_name = new char[strlen (shapeFile) + 1];
   strcpy (shape_name, shapeFile);
   managerId = manager_id;
   groupId = group_id;
   ghostGroupId = ghost_group_id;

   if ( ghostName )
   {
      ghost_name = new char[ strlen( ghostName ) + 1 ];
      strcpy( ghost_name, ghostName );
   }
}

myGuy::myGuy()
{
   init();
}


myGuy::~myGuy()
{
   if (shape_name)
      delete [] shape_name;

   if ( image.shape )
   {
      for (int i=0;i<getNumThreads();i++)
         image.shape->DestroyThread(animThreads[i]);
      delete image.shape;
   }
}

void myGuy::loadResources ()
{
   ResourceManager *rm = SimResource::get(manager);

   Resource<TSShape> shape;
   shape = rm->load(shape_name, true);

   if ((bool)shape)
   {
      image.shape = new TSShapeInstance( shape, *rm );
       pause();
   }
   else
     image.shape=NULL;
}

//--------------------------------------------------------------------------- 
// myGuy detail level routines
//--------------------------------------------------------------------------- 

int myGuy::numDetailLevels()
{
  return image.numDetailLevels();
}

int myGuy::getDetailLevel()
{
  return image.getDetailLevel();
}

void myGuy::setDetailLevel(int nl)
{
  image.setDetailLevel(nl);
}

void myGuy::setDetailLevel(int min,int max)
{
  image.setDetailLevel(min,max);
}

//--------------------------------------------------------------------------- 
// myGuy animation control routines
//--------------------------------------------------------------------------- 

int myGuy::newThread(int seqNum)
{
   if (getNumThreads()==maxThreads)
      return -1;
   image.shape->UpdateSequenceSubscriberLists();
   animThreads[numThreads]=image.shape->CreateThread();
   animThreads[numThreads]->SetSequence(seqNum,0.0f);
//   animThreads[numThreads]->UpdateSubscriberList();
   animating[numThreads]=false;
   animSpeed[numThreads]=1.0f;
   curThread=numThreads;
   return numThreads++;
}

void myGuy::selectThread(int threadNum)
{
   if (threadNum<numThreads)
      curThread=threadNum;
}

void myGuy::setAnimSeq(int seqNum,float startTime)
{ 
   if (animThreads[curThread])
   {
      animThreads[curThread]->SetSequence(seqNum,startTime);
//      animThreads[curThread]->UpdateSubscriberList();
   }
}

int myGuy::getAnimSeq()
{
   if (animThreads[curThread])
      return animThreads[curThread]->getSequence().getIndex(animThreads[curThread]->getShape());
   else
      return -1;
}

// until euler.h is corrected...
extern void Eset( RMat3F const &mat, EulerF & e);

void myGuy::stepThread(float increment, int tNum)
{
   int threadNum=tNum;
   if (threadNum==-1)
      threadNum=curThread;
   // in case called with -1 when no thread
   if (tNum==-1 && !numThreads)
      return;

   animThreads[threadNum]->setTimeScale(animSpeed[threadNum]);
   animThreads[threadNum]->AdvanceTime(increment);
//   animThreads[threadNum]->UpdateSubscriberList();

   // if we are steping just this thread, must animateRoot else shape won't move
   if (tNum==-1)
      image.shape->animateRoot();
}

void myGuy::step(float increment)
{
   for (int i=0;i<numThreads;i++)
      if (animating[i])
         stepThread(increment,i);
      else
         stepThread(0.0f,i);

   image.shape->animateRoot();

   if (!killRootTransform && numThreads)
   {
      // transfer animation movement to myGuy pos,rot
      TMat3F absTrans;
      m_mul(image.shape->fRootDeltaTransform,TMat3F(mover.getRot(),mover.getPos()),&absTrans);
      EulerF tmpE;
      Eset(absTrans,tmpE);
      mover.setRot(tmpE);
      mover.setPos(absTrans.p);
   }

   // set shape transform to identity so it doesn't move 2x (or at all if killRootTransform)
   image.shape->fRootDeltaTransform.identity();
}

//--------------------------------------------------------------------------- 

/*
void myGuy::setEventMap()
{
   AnalogEvent joy;
   SimActionEvent action;

   eventMap.clear();

   int flags = SimActionFlagValue|SimActionFlagCenter|
         SimActionFlagDeadZone|SimActionFlagSquare;
   joy.attributes = EVENT_LOCAL;
   joy.deviceNo= 0;
   action.fValue = 0.10f;

   joy.message   = AnalogEvent::MoveX;
   action.action = MoveRoll;
   eventMap.mapEvent( &joy, &action, flags, false );

   joy.message   = AnalogEvent::MoveY;
   action.action = MovePitch;
   eventMap.mapEvent( &joy, &action, flags, false );

   joy.message   = AnalogEvent::MoveR;
   action.action = MoveYaw;
   eventMap.mapEvent( &joy, &action, flags, false );

   SimActionHandler::find( manager )->push( this, &eventMap );
}
*/

void myGuy::update(SimTime t)
{
   // This function is only called when
   // we get events from the timer group which is
   // at a fixed interval.
   step(t);

   // now implement action commands (from the keyboard)
   mover.update(t);

   // shadow stuff, if needed...
   image.shadowLift=shadowLift;
   image.lightDirection=worldLightDirection;

   // are we standing on an interior?
   image.castShadowItr = false;
// since collision manager is no more...
#if 0
   SimCollisionManager *scm = dynamic_cast<SimCollisionManager *>(manager->findObject(SimCollisionManagerId));
   if (scm)
   {
      SimLineIntersectQuery *q = new SimLineIntersectQuery;
      q->start = mover.getPos();
      q->start.z -= 5.0f;
      q->end = q->start;
      q->end.z+=10.0f;
      scm->processQuery(q);
      if (q->collided && q->itrPtr)
      {
         SimInterior *itr = dynamic_cast<SimInterior *>(q->itrPtr);
         if (itr)
         {
            if (stickToGround)
               mover.setPos(q->point);
            if (castingShadows)
            {
               image.castShadowItr=true;
               image.itr = itr;
            }
         }
      }
   }
#endif

   // get terrain height and stick him there if "stickToGround" is set
   image.castShadowTerr=false;
   SimTerrain *terr = dynamic_cast<SimTerrain *>(manager->findObject(SimTerrainId));
   if (terr)
   {
      GridFile *file=terr->getGridFile();
      CollisionSurface info;
      bool overTerrain;

      Point3F inTerrSpace;
      m_mul(mover.getPos(),(TMat3F &) terr->getInvTransform(),&inTerrSpace);

      overTerrain=file->getSurfaceInfo(Point2F(inTerrSpace.x,inTerrSpace.y),&info);
      inTerrSpace.z=info.position.z;
   
      Point3F backToMySpace;
      m_mul(inTerrSpace,(TMat3F &) terr->getTransform(),&backToMySpace);         

      if (overTerrain && stickToGround)
         mover.setPos(backToMySpace);

      // if we're over terrain, set up shadow info.
      if (overTerrain && castingShadows)
      {
         image.castShadowTerr=true;
         image.terr=terr;
      }

   }
}

bool myGuy::onAdd()
{
   if (!ObjParent::onAdd())
      return false;
      
   loadResources ();

   if (shape()) // if we failed to load a shape, then don't add to groupds
   {
      // Post by sending an event
      addToSet(SimTimerSetId);
      SimSet *grp = (SimSet*)manager->findObject( SimRenderSetId );

      if ( grp )
          grp->addObject( this );

      grp = (SimSet*)manager->findObject( SimCameraSetId );
         if ( grp )
           grp->addObject( this );

      if( groupId )
         addToSet(groupId);
        
      eventMap = SimResource::loadByName(manager, "showMap.sae", true);
      if(bool(eventMap))
         SimActionHandler::find (manager)->push (this, eventMap);
   }
   return true;
}

bool myGuy::processEvent(const SimEvent* event)
{
   switch (event->type) 
   {
      case SimTimerEventType:
         // Update every 1/30 sec. from timer group
         update(static_cast<const SimTimerEvent *>(event)->timerInterval);
         break;
      case SimActionEventType:
      {
         const SimActionEvent* ep = 
            static_cast<const SimActionEvent*>(event);

         switch(ep->action)
         {
            case ViewRoll:
            case ViewPitch:
            case ViewYaw:
            case MoveUp:
            case MoveDown:
            case MoveForward:
            case MoveBackward:
            case MoveRight:
            case MoveLeft:
               mover.processAction(ep->action,ep->fValue);
               break;
         }
         break;
      }
      default:
         return false;
   }
   return true;
}

bool myGuy::processQuery( SimQuery *q )
{
   switch (q->type){
      case SimRenderQueryImageType: {
         if (!visible)
            return false;

         // Update shape & image data
         image.transform.set(mover.getRot(),mover.getPos());
         // scale the image if set
         if (isScaled)
            image.transform.preScale(scaleVector);

         // set inverse tranform
         image.invTransform=image.transform;
         image.invTransform.inverse();
         image.itype = SimRenderImage::Translucent;

         // Return the image
         SimRenderQueryImage *qp = static_cast<SimRenderQueryImage*>(q);
         qp->count = 1;
         qp->image[0] = &image;
         break;
      }
      default:
         return false;
   }
   return true;
}

//--------------------------------------------------------------------------- 

Persistent::Base::Error COREAPI myGuy::read(StreamIO &s,int,int)
{
   int size;

   s.read (sizeof (moverHelper), &mover);
   s.read( &managerId );
   s.read( &groupId );
   s.read( &id );
   s.read( &ghostGroupId );

   s.read ( &size);
   if (size)
   {
      shape_name = new char[size];
      s.read (size, shape_name);
   }

   s.read ( &size);
   if (size)
   {
      ghost_name = new char[size];
      s.read (size, ghost_name);
   }

   return (s.getStatus() == STRM_OK)? Ok: ReadError;
}

Persistent::Base::Error COREAPI myGuy::write(StreamIO &s,int,int)
{
   int size;

   s.write (sizeof (moverHelper), &mover);
   s.write( managerId );
   s.write( groupId );
   s.write( id );
   s.write( ghostGroupId );

   size = shape_name?  (strlen (shape_name) + 1) : 0;
   s.write (size);
   if (size)
      s.write (size, shape_name);

   size = ghost_name?  (strlen (ghost_name) + 1) : 0;
   s.write (size);
   if (size)
      s.write (size, ghost_name);

   return (s.getStatus() == STRM_OK)? Ok: WriteError;
}

//***********************************************************
