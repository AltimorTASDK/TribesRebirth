#include "FearMissionEditor.h"
#include "fileName.h"
#include "fearglobals.h"
#include "simconsoleevent.h"
#include "simmovement.h"

//----------------------------------------------------------------

int FearMissionEditor::getWorldId( SimObject * obj )
{
   if( obj->getManager() == SimGame::get()->getManager( SimGame::SERVER ) )
      return( SimGame::SERVER );
   else
      return( SimGame::CLIENT );
}

//----------------------------------------------------------------
// delete all the items in the set

void FearMissionEditor::SelectionSet::deleteSet()
{
   // delete the selection
   while( size() )
   {
      if( !front()->isDeleted() )
         front()->deleteObject();
   }
   
   // clear the set ( update display )
   clearSet();
}

//----------------------------------------------------------------
// remove all the items from the set

void FearMissionEditor::SelectionSet::clearSet()
{
   // remove all the items
   while( size() )
      removeObject( front() );
}

//----------------------------------------------------------------
// add an object to the set, return true if it already was there

SimObject * FearMissionEditor::SelectionSet::addObject( SimObject * obj )
{
   // add to the list - or return null if already in list
   if( find( begin(), end(), obj ) == end() )
   {
      objectList.pushBack( obj );
		deleteNotify( obj );
      return( obj );
   }
   
   return( NULL );
}

//----------------------------------------------------------------
// remove an object from the set, return false if it is not there      

void FearMissionEditor::SelectionSet::removeObject( SimObject * obj )
{
   // let SimSet do this
	Parent::removeObject( obj );
}

//----------------------------------------------------------------
// will return the center point for all the objects
// or (0,0,0) when no objects are selected

void FearMissionEditor::SelectionSet::calcCentroid()
{
   mCentroid.set( 0.f, 0.f, 0.f );
   int objCount = 0;

   // go through and average the thing   
   for( SimSet::iterator itr = begin(); itr != end(); itr++ )
   {
      SimObjectTransformQuery objQuery;
      
      if( (*itr)->processQuery( &objQuery ) )
      {
         objCount++;
         mCentroid += objQuery.tmat.p;
      }
   }
   if( objCount )
      mCentroid /= objCount;
}

//----------------------------------------------------------------
// find out if there are any objects in the set that are locked         

bool FearMissionEditor::SelectionSet::anyItemLocked()
{
   for( SimSet::iterator itr = begin(); itr != end(); itr++ )
      if( (*itr)->isLocked() )
         return( true );
   return( false );
}

//----------------------------------------------------------------
// tell if object is in set

bool FearMissionEditor::SelectionSet::objectInSet( const SimObject * obj )
{
   for( SimSet::iterator itr = begin(); itr != end(); itr++ )
      if( (*itr) == obj )
         return( true );
   return( false );
}

//----------------------------------------------------------------

FearMissionEditor::TypeInfo::TypeInfo() :
   mClassName( NULL ),
   mPlaceMask( 0 )
{
}

//----------------------------------------------------------------

FearMissionEditor::TypeInfo::~TypeInfo()
{
   delete [] mClassName;
}

//----------------------------------------------------------------

DWORD FearMissionEditor::getPlaceMask( const SimObject * obj )
{
   TypeInfo * info = getTypeInfo( obj );
   return( info ? info->mPlaceMask : mDefPlaceMask );
}

//----------------------------------------------------------------

FearMissionEditor::TypeInfo * FearMissionEditor::getTypeInfo( const SimObject * obj )
{
   // go through and find first type that matches the object
   for( int i = 0; i < mTypeInfo.size(); i++ )
      if( !stricmp( obj->getClassName(), mTypeInfo[i]->mClassName ) )
         return( mTypeInfo[i] );
         
   return( NULL );
}

//----------------------------------------------------------------

FearMissionEditor::TypeInfo * FearMissionEditor::getTypeInfo( const char * name )
{
   if( !name )
      return( NULL );
      
   // find the entry with the same name
   for( int i = 0; i < mTypeInfo.size(); i++ )
      if( !stricmp( name, mTypeInfo[i]->mClassName ) )
         return( mTypeInfo[i] );
         
   return( NULL );
}

//----------------------------------------------------------------

bool FearMissionEditor::removeTypeInfo( const char * name )
{
   if( !name )
      return( false );
   
   // go through and find this one
   for( int i = 0; i < mTypeInfo.size(); i++ )
      if( !stricmp( name, mTypeInfo[i]->mClassName ) )
      {
         delete mTypeInfo[i]->mClassName;
         mTypeInfo.erase( i );
         return( true );
      }
   return( false );
}

//----------------------------------------------------------------

bool FearMissionEditor::addTypeInfo( const char * name, int mask, const char * selBmpName, const char * unselBmpName )
{
   if( !name )
      return( false );
      
   // see if this one is here already...
   for( int i = 0; i < mTypeInfo.size(); i++ )
      if( !stricmp( name, mTypeInfo[i]->mClassName ) )
         return( false );
   
   TypeInfo * typeInfo = new TypeInfo;
   typeInfo->mClassName = strnew( name );
   typeInfo->mPlaceMask = mask;
   
   // selected bitmap?
   if( selBmpName )
   {
      typeInfo->mSelectedBmp = SimResource::get( getManager() )->load( selBmpName );
      if( bool( typeInfo->mSelectedBmp ) )
         typeInfo->mSelectedBmp->attribute |= BMA_TRANSPARENT;
   }
   else
      typeInfo->mSelectedBmp = mDefSelectedBmp;
      
   // unselected bitmap?
   if( unselBmpName )
   {
      typeInfo->mUnselectedBmp = SimResource::get( getManager() )->load( unselBmpName );
      if( bool( typeInfo->mUnselectedBmp ) )
         typeInfo->mUnselectedBmp->attribute |= BMA_TRANSPARENT;
   }
   else
      typeInfo->mUnselectedBmp = mDefUnselectedBmp;
      
   mTypeInfo.push_back( typeInfo );
   
   return( true );
}

//----------------------------------------------------------------
//----------------------------------------------------------------
//----------------------------------------------------------------

FearMissionEditor::FearMissionEditor() :
   mFlags( 0 ),
   mTargetManager( NULL ),
   mCanvas( NULL )
{
   copyPosition.set(0,0,0);
   // reset the bookmarks
   for( int i = 0; i < 10; i++ )
      mBookmarkPos[i] = TMat3F( true );
}

//----------------------------------------------------------------

FearMissionEditor::~FearMissionEditor()
{
   // remove the typeinfo's
   while( mTypeInfo.size() )
      removeTypeInfo( (mTypeInfo.front())->mClassName );
}

//----------------------------------------------------------------

bool FearMissionEditor::init()
{
   if( !getManager() || !getCanvas() )
      return( false );

   // load in the cursor's      
   loadCursor( HandCursor, "CUR_hand.bmp", Point2I(5,1) );
   loadCursor( LockCursor, "CUR_waypoint.bmp", Point2I(10,13) );
   loadCursor( RotateCursor, "CUR_rotate.bmp", Point2I(11,18) );
   loadCursor( MoveCursor, "CUR_Centering.bmp", Point2I(10,13) );
   loadCursor( ArrowCursor, "CUR_Arrow.bmp", Point2I(0,0) );
   loadCursor( GrabHand, "CUR_grab.bmp", Point2I(9,13) );
   
   // load in the default select/unselect bitmaps
   loadBitmap( mDefSelectedBmp, "Node.bmp" );
   loadBitmap( mDefUnselectedBmp, "NodeOpen.bmp" );
   loadBitmap( mLockSelectedBmp, "DropPtOpen.bmp" );
   loadBitmap( mLockUnselectedBmp, "DropPt.bmp" );

   return( true );
}

//----------------------------------------------------------------

void FearMissionEditor::lockManager()
{
	if( getManager() != manager )
      getManager()->lock();
}

//----------------------------------------------------------------

void FearMissionEditor::unlockManager()
{
   if( getManager() != manager )
      getManager()->unlock();
}      

//----------------------------------------------------------------

void FearMissionEditor::onDeleteNotify(SimObject * obj)
{
   // just check if it's a SimConsoleScheduler object (already removed from the manager)
   if(dynamic_cast<SimConsoleScheduler*>(obj))
      deleteObject();
}

//----------------------------------------------------------------

bool FearMissionEditor::onAdd()
{
   if( !Parent::onAdd() )
      return( false );
      
   addToSet( SimRenderSetId );
   addToSet( SimInputConsumerSetId );

   return( true );
}

//----------------------------------------------------------------

SimGroup * FearMissionEditor::getAddGroup()
{
   CMDConsole * con = CMDConsole::getLocked();
   
   int parentId = con->getIntVariable( "$ME::InspectObject", -1 );

   lockManager();
   
   SimObject * parentObj;
   
   // see if there is an inspect object - if not then use missiongroup
   if( parentId == -1 )
      parentObj = getManager()->findObject( "MissionGroup" );
   else
      parentObj = getObject( parentId );
      
   if( !parentObj )
   {
      unlockManager();
      return( NULL );
   }
      
   SimGroup * sg = dynamic_cast< SimGroup * >( parentObj );
   
   if( !sg )
      sg = parentObj->getGroup();

   unlockManager();
   
   return( sg );
}

//----------------------------------------------------------------

SimObject * FearMissionEditor::addObject( SimObject * obj )
{
   SimGroup * sg = getAddGroup();
   if( !sg )
      return( NULL );
      
   lockManager();
         
   // add to this group
   if( !sg->addObject( obj ) )
   {
      unlockManager();
      return( NULL );
   }
   
   // add to the edit set
   obj->addToSet( SimEditSetId );
   unlockManager();
   
   SimEditEvent::post( obj, SimMissionEditorId, mFlags.test( InputFocus ) );
      
   return( obj );
}

//----------------------------------------------------------------

void FearMissionEditor::onRemove()
{
   Parent::onRemove();
}

//----------------------------------------------------------------

bool FearMissionEditor::processQuery( SimQuery * query )
{
   switch( query->type )
   {
      onQuery( SimInputPriorityQuery );
      onQuery( SimRenderQueryImage );
      default:
         return( Parent::processQuery( query ) );
   }
}

//----------------------------------------------------------------

bool FearMissionEditor::onSimInputPriorityQuery( SimInputPriorityQuery * query )
{
   query->priority = SI_PRIORITY_NORMAL + 1;
   return true;                          
}

//----------------------------------------------------------------

bool FearMissionEditor::onSimRenderQueryImage( SimRenderQueryImage * query )
{
   // make sure has input focus and drawing handles
   if( !mFlags.test( InputFocus ) || !mFlags.test( RenderHandles ) )
      return( false );
      
   // fill in the query
	query->count = 1;
	query->image[0] = &mRenderImage;
   
   // fill in the renderimage
   mRenderImage.itype = SimRenderImage::Overlay;
   mRenderImage.mManager = getManager();
   mRenderImage.mME = this;
   
	return( true );
}

//----------------------------------------------------------------

void FearMissionEditor::RenderImage::render( TSRenderContext & rc )
{
   for( SimSetIterator i(mManager); *i; ++i )
   {
      SimObjectTransformQuery objQuery;
      GFXSurface * sfc = rc.getSurface();
      TSPointArray * pa = rc.getPointArray();

      // check if object has a transform
      if( (*i)->processQuery( &objQuery ) )
      {
         TS::TransformedVertex & v = 
            pa->getTransformedVertex( pa->addPoint( objQuery.tmat.p ) );

         // check if procjected on-screen   
         if( v.fStatus & TS::TransformedVertex::Projected )
         {
            Point2I ul( v.fPoint.x, v.fPoint.y );
            Point2I lr;

            GFXBitmap * bmp;
            
            if( (*i)->isLocked() )
               bmp = (*i)->isSelected() ? mME->mLockSelectedBmp : mME->mLockUnselectedBmp;
            else
            {
               TypeInfo * typeInfo = mME->getTypeInfo( *i );
               if( (*i)->isSelected() )
                  bmp = typeInfo ? typeInfo->mSelectedBmp : mME->mDefSelectedBmp;
               else
                  bmp = typeInfo ? typeInfo->mUnselectedBmp : mME->mDefUnselectedBmp;
            }
                           
            lr =  Point2I( bmp->getWidth() >> 1, 
               bmp->getHeight() >> 1);
            ul -= Point2I( bmp->getWidth() >> 2, 
               bmp->getHeight() >> 2);
            lr += ul;
            sfc->drawBitmap2d( bmp, &ul );
         }
      }
   }   
}

//----------------------------------------------------------------

bool FearMissionEditor::processEvent( const SimEvent * event )
{
   switch( event->type )
   {
      onEvent( Sim3DMouseEvent );
      onEvent( SimEditEvent );
      onEvent( SimGainFocusEvent );
      onEvent( SimLoseFocusEvent );
      default:
         return( Parent::processEvent( event ) );
   }
}

//------------------------------------------------------------------------------

SimObject * FearMissionEditor::getCamera()
{
   // get the camera object
   CMDConsole * con     = CMDConsole::getLocked();
   const char * name    = con->getVariable( "ME::camera" );
   SimManager * man     = SimGame::get()->getManager( SimGame::CLIENT );
   return( man->findObject( name ) );
}

//------------------------------------------------------------------------------

bool FearMissionEditor::getTransform( SimObject * obj, TMat3F & transform )
{
   SimObjectTransformQuery query;
   
   if( obj && obj->processQuery( &query ) )
   {
      transform = query.tmat;
      return( true );
   }   
   return( false );
}

//------------------------------------------------------------------------------

bool FearMissionEditor::setTransform( SimObject * obj, TMat3F & transform )
{
   if( obj )
   {
      SimObjectTransformEvent event( transform );
      obj->processEvent( &event );
      return( true );
   }   
   
   return( false );
}

//----------------------------------------------------------------

void FearMissionEditor::moveObjectRelative( SimObject * obj, Point3F & delta, bool allowRecurse )
{
   AssertFatal( obj, "Invalid Object" );

   if( allowRecurse )
   {
      SimSet * ss;
      if( ( ss = dynamic_cast< SimSet * >( obj ) ) != NULL )
      {
         for( SimSet::iterator itr = ss->begin(); itr != ss->end(); itr++ )
            moveObjectRelative( (*itr), delta, false );
         return;
      }
   }   

   TMat3F tmat;
   if( getTransform( obj, tmat ) )
   {
      if( mFlags.test( ConstrainXAxis ) ) delta.x = 0.f;
      if( mFlags.test( ConstrainYAxis ) ) delta.y = 0.f;
      if( mFlags.test( ConstrainZAxis ) ) delta.z = 0.f;
      
      tmat.p += delta;
      tmat.flags |= TMat3F::Matrix_HasTranslation;
      
      setTransform( obj, tmat );
   }
}

//----------------------------------------------------------------

void FearMissionEditor::moveObjectAbsolute( SimObject * obj, Point3F & pos, bool allowRecurse )
{
   AssertFatal( obj, "Invalid Object" );

   if( allowRecurse )
   {
      SimSet * ss;
      if( ( ss = dynamic_cast< SimSet * >( obj ) ) != NULL )
      {
         for( SimSet::iterator itr = ss->begin(); itr != ss->end(); itr++ )
            moveObjectAbsolute( (*itr), pos, false );
         return;
      }
   }   

   TMat3F tmat;
   if( getTransform( obj, tmat ) )
   {
      if( mFlags.test( ConstrainXAxis ) ) pos.x = tmat.p.x;
      if( mFlags.test( ConstrainYAxis ) ) pos.y = tmat.p.y;
      if( mFlags.test( ConstrainZAxis ) ) pos.z = tmat.p.z;
      
      tmat.p = pos;
      tmat.flags |= TMat3F::Matrix_HasTranslation;
      
      setTransform( obj, tmat );
   }
}

//----------------------------------------------------------------


void FearMissionEditor::dropObjectDown( SimObject * obj, bool allowRecurse )
{
   AssertFatal( obj, "Invalid Object" );
   
   if( allowRecurse )
   {
      SimSet * ss;
      if( ( ss = dynamic_cast< SimSet * >( obj ) ) != NULL )
      {
         for( SimSet::iterator itr = ss->begin(); itr != ss->end(); itr++ )
            dropObjectDown( (*itr), false );
         return;
      }
   }
   
   TMat3F tmat;
   if( getTransform( obj, tmat ) )
   {
      Point3F endPt = tmat.p;
      endPt.z -= 1000;
      
      SimCollisionInfo info;
      SimContainerQuery query;
      
      // fill in the query...
      query.id = obj->getId();
      query.type = -1;
      query.box.fMin = tmat.p;
      query.box.fMax = endPt;
      query.mask = getPlaceMask( obj );

      lockManager();
      SimContainer * root = findObject( getManager(), SimRootContainerId, root );
      for( int i = 0; i < 2; i++ )
      {
         if( root->findLOS( query, &info ) && ( info.object->getType() & getPlaceMask( info.object ) ) )
         {
            // Translate collision to world coordinates and move object there
            Point3F  pt;
            m_mul( info.surfaces[0].position, info.surfaces.tWorld, &pt );
            pt -= tmat.p;
            moveObjectRelative( obj, pt );
            break;
         }   
         
         // attempt drop from far above...
         query.box.fMin.z += 5000;
      }
      
      unlockManager();
   }
}

// grabs the position that the drop will occur - no rotations and
// does not care what item type is being dropped
bool FearMissionEditor::getDropTarget(int dropType, Point3F & target)
{
   SimObject * cam = getCamera();
   if(!cam)
      return(false);
      
   TMat3F camMat;
   if(!getTransform(cam,camMat))
      return(false);
      
   switch(dropType)
   {
      case DropAtCamera:
      case DropWithRotAtCamera:
         target = camMat.p;
         break;
         
      case DropBelowCamera:
         target = camMat.p;
         target.z -= 15.f;
         break;
         
      case DropToScreenCenter:
      {
         Point3F endPt;
         m_mul( Point3F( 0, 200, 0 ), camMat, &endPt );

         SimCollisionInfo info;
         SimContainerQuery query;
         query.id = cam->getId();
         query.type = -1;
         query.box.fMin = camMat.p;
         query.box.fMax = endPt;
         query.mask = 0xffffffff;

      	lockManager();
         SimContainer * root = findObject(getManager(), SimRootContainerId, root);
         if(root->findLOS(query, &info))
         {
            // Translate collision to world coordinates and move object there
            Point3F  pt;
            m_mul(info.surfaces[0].position, info.surfaces.tWorld, &pt);
            target = pt;
         }   
         else
            target = camMat.p;
      	unlockManager();
         break;
      }

      case DropToSelectedObject:
      {
         target = mSelection.getCentroid();
         break;
      }
   }
   return(true);
}

//----------------------------------------------------------------

void FearMissionEditor::onDrop( SimObject * obj, int dropType, bool allowRecurse )
{
   if( allowRecurse )
   {
      SimSet * ss;
      if( ( ss = dynamic_cast< SimSet * >( obj ) ) != NULL )
      {
         for( SimSet::iterator itr = ss->begin(); itr != ss->end(); itr++ )
            onDrop( ( *itr ), dropType, false );
         return;
      }
   }
   
   SimObject * cam = getCamera();
   if( !cam )
      return;
   
   TMat3F objMat, camMat;
   if( !getTransform( cam, camMat ) || !getTransform( obj, objMat ) )
      return;

   // if this is a simmovement object (flags/people/...) reset velocity
   SimMovement * move = dynamic_cast<SimMovement *>(obj);
   if(move)
   {
      Vector3F none(0.f, 0.f, 0.f);
      move->setAngulerVelocity(none);
      move->setLinearVelocity(none);
   }
         
   switch( dropType )
   {
      case DropToCopyPosition:
         objMat.p += copyPosition;
         objMat.flags |= TMat3F::Matrix_HasTranslation;
         setTransform(obj, objMat);
         break;
         
      case DropAtCamera:

         objMat.p += camMat.p;
         objMat.flags |= TMat3F::Matrix_HasTranslation;
         setTransform( obj, objMat );
         break;
         
      case DropWithRotAtCamera:
      {
         Point3F pos = camMat.p + objMat.p;
         objMat = camMat;
         objMat.p = pos;
         objMat.flags |= TMat3F::Matrix_HasTranslation | 
            TMat3F::Matrix_HasRotation;
         setTransform( obj, objMat );
         break;
      }
         
      case DropBelowCamera:
      {
         objMat.p += camMat.p;
         objMat.p.z -= 15.f;
         objMat.flags |= TMat3F::Matrix_HasTranslation;
         setTransform( obj, objMat );
         break;
      }
         
      case DropToScreenCenter:
      {
         Point3F endPt;
         m_mul( Point3F( 0, 200, 0 ), camMat, &endPt );

         SimCollisionInfo info;
         SimContainerQuery query;
         query.id = cam->getId();
         query.type = -1;
         query.box.fMin = camMat.p;
         query.box.fMax = endPt;
         query.mask = getPlaceMask( obj );

      	lockManager();
         SimContainer * root = findObject( getManager(), SimRootContainerId, root );
         if( root->findLOS( query, &info ) && ( info.object->getType() & getPlaceMask( info.object ) ) )
         {
            // Translate collision to world coordinates and move object there
            Point3F  pt;
            m_mul( info.surfaces[0].position, info.surfaces.tWorld, &pt );
            
            objMat.p += pt;
            setTransform( obj, objMat );   
         }   
         else
         {
            objMat.p += endPt;
            setTransform( obj, objMat );   
         }
      	unlockManager();
         
         break;
      }

      case DropToSelectedObject:
      {
         objMat.p += mSelection.getCentroid();
         setTransform( obj, objMat );
         break;
      }
   }
}

//----------------------------------------------------------------

void FearMissionEditor::rotateObject( SimObject * obj, Point3F & anchor, EulerF & rot, bool allowRecurse )
{
   if( allowRecurse )
   {
      SimSet * ss;
      if( ( ss = dynamic_cast<SimSet*>( obj ) ) != NULL )
      {
         for( SimSet::iterator itr=ss->begin(); itr!=ss->end(); itr++ )
            rotateObject( (*itr), anchor, rot, false );
         return;
      }
   }

   TMat3F tmat;
   if( !obj || !getTransform( obj, tmat ) )
      return;
      
   SimObjectTransformEvent event(tmat);

   TMat3F rotMat( true );
   rotMat.preRotateX( rot.x );
   rotMat.preRotateY( rot.y );
   rotMat.preRotateZ( rot.z );

   tmat.p -= anchor;
   m_mul( tmat, rotMat, &event.tmat );
   event.tmat.p += anchor;

	event.tmat.flags |= TMat3F::Matrix_HasTranslation | TMat3F::Matrix_HasRotation;

   obj->processEvent( &event );
}

//----------------------------------------------------------------

void FearMissionEditor::getSystemKeys()
{
   CMDConsole * con = CMDConsole::getLocked();
   mFlags.set( Mod1, con->getBoolVariable( "ME::Mod1", false ) );  // control
   mFlags.set( Mod2, con->getBoolVariable( "ME::Mod2", false ) );  // shift
   mFlags.set( Mod3, con->getBoolVariable( "ME::Mod3", false ) );  // alt
}

//----------------------------------------------------------------
// calculates the centroid as the planeOrigin and gets the intercept points
// for the different planes...
bool FearMissionEditor::getPlaneIntercepts( const Sim3DMouseEvent * event )
{     
   // get plane origin
   mSelection.calcCentroid();
   Point3F planeOrigin = mSelection.getCentroid();

   // get cast vector
   Point3F direction = event->direction;

   // get camera position
   TMat3F tmat;   
   SimObject * cam = getCamera();
   if( !cam || !getTransform( cam, tmat ) )
      return( false );
   Point3F & camera = tmat.p;
   
   // -- calculate the xy intercept point to the plane
   double planeDistance = camera.z - planeOrigin.z;
   double projectedDistance = m_dot( direction, Point3F( 0, 0, -1 ) );
   if( projectedDistance == 0.f )
      return( false );
   
   double scale = planeDistance / projectedDistance;
   
   mPlaneXYIntercept = camera + ( direction * scale );
   
   // -- calculate the z intercept coordinate to the plane perp. to camera
   // plane distance is distance between xy coords of planeOrigin and camera
   planeDistance = sqrt( ( camera.x - planeOrigin.x ) * ( camera.x - planeOrigin.x ) + 
      ( camera.y - planeOrigin.y ) * ( camera.y - planeOrigin.y ) );
      
   // get the vector we want to project through
   Point3F vec( planeOrigin.x - camera.x, planeOrigin.y - camera.y, 0.f );
   vec.normalize();
   
   projectedDistance = m_dot( direction, vec );
   if( projectedDistance == 0.f )
      return( false );
      
   scale = planeDistance / projectedDistance;
   vec = camera + ( direction * scale );

   mPlaneZIntercept = vec.z;
   
   return(true);
}

//----------------------------------------------------------------

#define DBLCLICK_DELAY  750

bool FearMissionEditor::onSim3DMouseEvent( const Sim3DMouseEvent * event )
{
   CMDConsole * con = CMDConsole::getLocked();
   
   // get the current mouse position
   Point2I mousePos( getCanvas()->getCursorPos() );
   Point2F mouseDif( mousePos.x - mLastMousePos.x, mousePos.y - mLastMousePos.y );
   
   getSystemKeys();
   
   float moveMod = con->getFloatVariable( "$ME::MoveSensitivity", 0.2f );
   float rotateMod = con->getFloatVariable( "$ME::RotateSensitivity", 0.02f );

   SimGui::TSControl * editGui = dynamic_cast<SimGui::TSControl*>(event->sender);
   if( editGui && editGui->findHitControl(getCanvas()->getCursorPos()) != editGui )
   {
      setCursor(ArrowCursor);
      return(false);
   }

   // look at the mouse event type
   switch( event->meType )
   {
      case Sim3DMouseEvent::MouseMove:
      {
         if( mFlags.test( InputFocus ) && getLOSIntercept( event, &mGrabInfo, mGrabMask ) )
         {
            if( mGrabInfo.object->isLocked() )
               setCursor( LockCursor );
            else
               setCursor( HandCursor );
         }
         else
            setCursor( ArrowCursor );
         break;
      }
      
      case Sim3DMouseEvent::MouseDown:
      {
         if( !mFlags.test( InputFocus ) )
            break;
            
         mFlags.clear( LeftMouseDrag );
         
         // add a hit object
         if( getLOSIntercept( event, &mGrabInfo, mGrabMask ) )
         {
            if( !mSelection.objectInSet( mGrabInfo.object ) && !mFlags.test( Mod2 ) )
            {
               // clear the selection
               con->evaluatef( "MissionObjectList::ClearSelection();" );
               con->evaluatef( "MissionObjectList::SelectObject( %d, %d );",
                  getWorldId( mGrabInfo.object ), mGrabInfo.object->getId() );
            }

            // also calculates the centroid...
            getPlaneIntercepts( event );

            // grab the offsets...            
            mPlaneXYOffset = mSelection.getCentroid() - mPlaneXYIntercept;
            mPlaneZOffset = mSelection.getCentroid().z - mPlaneZIntercept;
         }
         else
            con->evaluatef( "MissionObjectList::ClearSelection();" );

         mRotSum = EulerF( 0.f, 0.f, 0.f );
            
         break;
      }
      
      case Sim3DMouseEvent::MouseDragged:
      {
         if( !mFlags.test( InputFocus ) )
            break;
            
         // check to save info for undo on first time through
         if( !mFlags.test( LeftMouseDrag ) && !mSelection.anyItemLocked() && mSelection.size() )
            saveInfo( &mSelection );
            
         mFlags.set( LeftMouseDrag );

         // cannot alter locked or empty sets...
         if( mSelection.anyItemLocked() || !mSelection.size() )
            break;
            
         // make sure the mouse has moved since last call
         if( mouseDif.x == 0.f && mouseDif.y == 0.f )
            break;
            
         bool hasRotation = false;
         bool hasDelta = false;
         bool allowDrop = false;
         EulerF objRotate( 0.f, 0.f, 0.f );
         Point3F objDelta( 0.f, 0.f, 0.f );
         
         if( mFlags.test( Mod1 ) ) // control - up/down
         {
            // check the movement type
            if( mFlags.test( UsePlaneMovement ) )
            {
               if( !getPlaneIntercepts( event ) )
                  break;
               objDelta.z = ( mPlaneZIntercept + mPlaneZOffset ) - mSelection.getCentroid().z;
            }
            else // move relative to mouse movement
               objDelta.z += -( mouseDif.y * moveMod );

            // check to snap to the z-coord
            if( mFlags.test( SnapToGrid ) )
            {
               float target = mSelection.getCentroid().z + objDelta.z;
               
               target += mGranularity.z/2;
               target -= fmod( target, mGranularity.z );

               objDelta.z = target - mSelection.getCentroid().z;
            }
               
            hasDelta = true;   
         }
         else if( mFlags.test( Mod2 ) ) // shift - rotate-z
         {
            setCursor( RotateCursor );
            objRotate.set( 0.f, 0.f, mouseDif.x * rotateMod );
            hasRotation = true;
         }
         else if( mFlags.test( Mod3 ) ) // alt - rotate-y
         {
            setCursor( RotateCursor );
            objRotate.set( 0.f, mouseDif.x * rotateMod, 0.f );
            hasRotation = true;
         }
         else
         {
            // check the movement type
            if( mFlags.test( UsePlaneMovement ) )
            {
               if( !getPlaneIntercepts( event ) )
                  break;
               objDelta = ( mPlaneXYIntercept + mPlaneXYOffset ) - mSelection.getCentroid();
            }
            else
            {
               DWORD mask = mDefPlaceMask;
               if( mSelection.size() == 1 )
                  mask = getPlaceMask( mSelection.front() );
                  
               // needs to intersept something
               if( !getLOSIntercept( event, &mPlaceInfo, mask, mSelection.front()->getId() ) )
                  break;

               // get the position of the collision                  
               mSelection.calcCentroid();
               m_mul( mPlaceInfo.surfaces[0].position, mPlaceInfo.surfaces.tWorld, &objDelta );
               objDelta = ( objDelta + mPlaneXYOffset ) - mSelection.getCentroid();
            }

            // check for snap to grid...
            if( mFlags.test( SnapToGrid ) )
            {
               Point3F target = mSelection.getCentroid() + objDelta;

               target += ( mGranularity / 2 );
               target.x -= fmod( target.x, mGranularity.x );
               target.y -= fmod( target.y, mGranularity.y );
               target.z -= fmod( target.z, mGranularity.z );

               objDelta = target - mSelection.getCentroid();
            }
            
            allowDrop = true;
            hasDelta = true;   
         }
         
         // check if it has some movement/rotation
         if( hasDelta )
         {
            // set the cursor
            if( mFlags.test( UsePlaneMovement ) )
               setCursor( GrabHand );
            else
               setCursor( MoveCursor );

            moveObjectRelative( &mSelection, objDelta );
            if( allowDrop && !mFlags.test( UsePlaneMovement ) && mFlags.test( DropToGround ) )
               dropObjectDown( &mSelection );
         }
         else if( hasRotation )
         {
            setCursor( RotateCursor );
            
            // check for rotation snapping
            if( mFlags.test( SnapRotations ) && ( mRotationSnap != 0.f ) )
            {
               // get the radian version of the snapping
               float rotSnap = ( M_PI / ( 180 / mRotationSnap ) );
               
               EulerF rotA;
               
               rotA.x = mRotSum.x - fmod( mRotSum.x, rotSnap );
               rotA.y = mRotSum.y - fmod( mRotSum.y, rotSnap );
               rotA.z = mRotSum.z - fmod( mRotSum.z, rotSnap );

               // store up the rotations
               mRotSum += objRotate;

               EulerF rotB;
               
               rotB.x = mRotSum.x - fmod( mRotSum.x, rotSnap );
               rotB.y = mRotSum.y - fmod( mRotSum.y, rotSnap );
               rotB.z = mRotSum.z - fmod( mRotSum.z, rotSnap );

               objRotate = ( rotB - rotA );
               
               // set the rotation amount
               if( mRotateAxis != RotateXAxis ) objRotate.x = 0.f;
               if( mRotateAxis != RotateYAxis ) objRotate.y = 0.f;
               if( mRotateAxis != RotateZAxis ) objRotate.z = 0.f;
            }
            
            rotateObject( &mSelection, mSelection.getCentroid(), objRotate );
         }
         
         break;
      }
      
      case Sim3DMouseEvent::MouseUp:
      {
         if( !mFlags.test( InputFocus ) )
            break;
            
         // check if hit anything            
         if( getLOSIntercept( event, &mGrabInfo, mGrabMask ) )
         {
            setCursor( HandCursor );
            
            if( mFlags.test( LeftMouseDrag ) )
               break;

            int worldId = getWorldId( mGrabInfo.object );
            int objId = mGrabInfo.object->getId();
            
            // check for dbl-click
            if( ( GetTickCount() - mLastTime ) < DBLCLICK_DELAY )
            {
               if( mSelection.size() )
               {
                  // add the item/expand/inspect
                  con->evaluatef( "MissionObjectList::SelectObject(%d,%d);", worldId, objId );
                  con->evaluatef( "MissionObjectList::ExpandToObject(%d,%d);", worldId, objId );
                  con->evaluatef( "MissionObjectList::Inspect(%d,%d);", worldId, objId );
               }
            }
            else
            {
               // multiple selections?
               if( mFlags.test( Mod2 ) ) // shift
               {
                  // toggle object selection state
                  if( mSelection.objectInSet( mGrabInfo.object ) )
                     con->evaluatef( "MissionObjectList::UnselectObject(%d,%d);", worldId, objId );
                  else
                     con->evaluatef( "MissionObjectList::SelectObject(%d,%d);", worldId, objId );
               }
            }
         }
         else
            setCursor( ArrowCursor );
         
         // update the time
         mLastTime = GetTickCount();
         
         break;
      }
      
      case Sim3DMouseEvent::RightMouseDown:
      {
         getCanvas()->showCursor( false );
         getCanvas()->forwardMouse( true );
         break;
      }
      
      case Sim3DMouseEvent::RightMouseUp:
      {
         getCanvas()->showCursor( true );
         getCanvas()->forwardMouse( false );
         break;
      }
   }
   
   // get the last mouse position
   mLastMousePos = mousePos;
   
   return( false );
}

//----------------------------------------------------------------

bool FearMissionEditor::onSimEditEvent( const SimEditEvent * event )
{
   static bool inRecurse = false;
   if( inRecurse )
      return( Parent::processEvent(event) );

   if( event->editorId == id )
   {
      mFlags.set( InputFocus, event->state );

      lockManager();
      
      // broadcast edit event to every object
      SimEditEvent ev;
      ev.editorId = SimMissionEditorId;
      ev.state    = event->state;
      
      inRecurse = true;
      getManager()->processEvent( &ev );
      inRecurse = false;
      
      unlockManager();
   }
   return( Parent::processEvent( event ) );
}

//----------------------------------------------------------------

bool FearMissionEditor::onSimGainFocusEvent( const SimGainFocusEvent * event )
{
   mFlags.set( InputFocus, true );
   mFlags.set( Mod1 | Mod2 | Mod3, false );
   
   // broadcast an edit event
   SimEditEvent ev;
   ev.editorId = SimMissionEditorId;
   ev.state = true;
   getManager()->processEvent( &ev );

   sg.editMode = true;
   
   return( Parent::processEvent( event ) );
}

//----------------------------------------------------------------

bool FearMissionEditor::onSimLoseFocusEvent( const SimLoseFocusEvent * event )
{
   mFlags.set( InputFocus, false );
   mFlags.set( Mod1 | Mod2 | Mod3, false );

   // broadcast an edit event
   SimEditEvent ev;
   ev.editorId = SimMissionEditorId;
   ev.state = false;
   getManager()->processEvent( &ev );

   sg.editMode = false;
   
   return( Parent::processEvent( event ) );
}

//----------------------------------------------------------------

bool FearMissionEditor::getLOSIntercept( const Sim3DMouseEvent * event, SimCollisionInfo * info, DWORD allowedTypes, int objId )
{
   // calculate an endpt to cast
   Point3F endPt = event->direction;
   endPt *= 10000;
   endPt += event->startPt;
   
   SimContainerQuery query;
   if( objId == -1 )
      query.id = ( dynamic_cast< SimGui::TSControl *>( event->sender ))->getObject()->getId();
   else
      query.id = objId;
      
   query.type = -1;
   query.box.fMax = endPt;
   query.box.fMin = event->startPt;
   query.mask = allowedTypes;
	query.detail = SimContainerQuery::DefaultDetail;

	// Run the LOS on the server now...
	lockManager();
   info->object = NULL;
   SimContainer * root = findObject( getManager(), SimRootContainerId, root );
   root->findLOS( query, info, SimCollisionImageQuery::Editor ) &&
      ( info->object->getType() & allowedTypes );
	unlockManager();
      
   return( info->object != NULL );
}

//----------------------------------------------------------------

SimObject * FearMissionEditor::getObject( UINT objID )
{
   return( getManager()->findObject( objID ) );
}

//----------------------------------------------------------------

void FearMissionEditor::getConsoleOptions()
{
   CMDConsole * con = CMDConsole::getLocked();
   
   mFlags.set( ShowEditObjects, con->getBoolVariable( "ME::ShowEditObjects", true ) );
   mFlags.set( RenderHandles, con->getBoolVariable( "ME::ShowGrabHandles", true ) );
   mFlags.set( SnapToGrid, con->getBoolVariable( "ME::SnapToGrid", false ) );
   mGranularity.x = con->getFloatVariable( "ME::XGridSnap", 1.f );
   mGranularity.y = con->getFloatVariable( "ME::YGridSnap", 1.f );
   mGranularity.z = con->getFloatVariable( "ME::ZGridSnap", 0.001f );
   mFlags.set( ConstrainXAxis, con->getBoolVariable( "ME::ConstrainX", false ) );
   mFlags.set( ConstrainYAxis, con->getBoolVariable( "ME::ConstrainY", false ) );
   mFlags.set( ConstrainZAxis, con->getBoolVariable( "ME::ConstrainZ", false ) );
   if( con->getBoolVariable( "ME::RotateXAxis", false ) )
      mRotateAxis = RotateXAxis;
   if( con->getBoolVariable( "ME::RotateYAxis", false ) )
      mRotateAxis = RotateYAxis;
   if( con->getBoolVariable( "ME::RotateZAxis", true ) )
      mRotateAxis = RotateZAxis;
   mRotationSnap = con->getFloatVariable( "ME::RotationSnap", 90.f );
   mFlags.set( SnapRotations, con->getBoolVariable( "ME::SnapRotations", false ) );
   if( con->getBoolVariable( "ME::DropAtCamera", false ) )
      mDropType = DropAtCamera;
   if( con->getBoolVariable( "ME::DropWithRotAtCamera", false ) )
      mDropType = DropWithRotAtCamera;
   if( con->getBoolVariable( "ME::DropBelowCamera", false ) )
      mDropType = DropBelowCamera;
   if( con->getBoolVariable( "ME::DropToScreenCenter", true ) )
      mDropType = DropToScreenCenter;
   if( con->getBoolVariable( "ME::DropToSelectedObject", false ) )
      mDropType = DropToSelectedObject;
   mFlags.set( UsePlaneMovement, con->getBoolVariable( "ME::UsePlaneMovement", false ) );
   mFlags.set( DropToGround, con->getBoolVariable( "ME::ObjectsSnapToTerrain", false ) );
}

//----------------------------------------------------------------

#define SAVE_BUFFER_NAME   "temp\\clipbuffer.cs"
#define BUFFER_NAME        "clipbuffer.cs"

void FearMissionEditor::copySelection()
{
   // delete the file buffer file
   FileWStream file( SAVE_BUFFER_NAME );
   file.close();
   
   mSelection.calcCentroid();
   copyPosition = mSelection.getCentroid();
   SimGame::get()->focus( SimGame::SERVER );
   SimSet::iterator itr;
   
   // walk through and adjust for centroid
   for(itr = mSelection.begin(); itr != mSelection.end(); itr++)
   {
      if(!(*itr)->isSelected())
         continue;
         
      TMat3F objMat;
      if(getTransform((*itr), objMat))
      {
         objMat.p -= mSelection.getCentroid();
         setTransform((*itr), objMat);
      }
   }

   // walk trough and export only the groups (they export their children)
   for(itr = mSelection.begin(); itr != mSelection.end(); itr++)
   {
      if(!(*itr)->isSelected())
         continue;

      // let the parent export
      if(!(*itr)->getGroup() || !(*itr)->getGroup()->isSelected())
      {
         // only export selected objects..            
         CMDConsole::getLocked()->executef( 6,
            "exportObjectToScript", avar("%d", (*itr)->getId()),
            SAVE_BUFFER_NAME, "false", "true", "true");
      }
   }

   // adjust back from the centroid
   for(itr = mSelection.begin(); itr != mSelection.end(); itr++)
   {
      if(!(*itr)->isSelected())
         continue;

      TMat3F objMat;
      if(getTransform((*itr), objMat))
      {
         objMat.p += mSelection.getCentroid();
         setTransform((*itr), objMat);
      }
   }
      
   SimGame::get()->focus( SimGame::CLIENT );
}

//----------------------------------------------------------------

void FearMissionEditor::dropSelection(void)
{
   if(mSelection.anyItemLocked())
   {
      CMDConsole::getLocked()->printf("Failed to move objects -one or more are locked.");
      return;
   }

   saveInfo(&mSelection);
   mSelection.calcCentroid();
   SimGame::get()->focus( SimGame::SERVER );
   
   SimSet::iterator itr;

   // walk through and adjust for centroid
   for(itr = mSelection.begin(); itr != mSelection.end(); itr++)
   {
      if(!(*itr)->isSelected())
         continue;
         
      TMat3F objMat;
      if(getTransform((*itr), objMat))
      {
         objMat.p -= mSelection.getCentroid();
         setTransform((*itr), objMat);
      }
   }
   
   for(itr = mSelection.begin(); itr != mSelection.end(); itr++)
   {
      if(!(*itr)->isSelected())
         continue;
      onDrop(*itr, mDropType, false);
   }
   SimGame::get()->focus( SimGame::CLIENT );
}

//----------------------------------------------------------------

void FearMissionEditor::pasteFile(const char * pasteFile, bool toCopyPosition)
{
   // get the set for the parent...
   SimGroup * sg = getAddGroup();
   if( !sg )
      return;
   
   // add to a temporary group...
   SimGroup tmpParent;
   lockManager();
   getManager()->registerObject( &tmpParent );
   ( ( SimObject * )&tmpParent )->assignName( "editGroupObj" );
   unlockManager();
   
   CMDConsole * con = CMDConsole::getLocked();
   
   // load into this set
   con->evaluatef( "missionLoadObject( %d, \"%s\" );", tmpParent.getId(), pasteFile );
   
   // check that the item was added   
   if( !tmpParent.size() )
   {
      con->printf( "Failed to load object from file (%s)", pasteFile );
      getManager()->unregisterObject( &tmpParent );
   
      return;
   }
   
   // for 'DropToSelectedObject'
   mSelection.calcCentroid();
         
   // clear the selection
   con->evaluatef( "MissionObjectList::ClearSelection();" );

//   Point3F target;
//   if(!getDropTarget(mDropType, target))
//      return;
   
   lockManager();

   // get a list of all the objects, add to the manager, then move them
   Vector<SimObject*> objList;
   for(SimSetIterator itr(&tmpParent); *itr; ++itr)
      objList.push_back((*itr));

   // add to the manager
   while(tmpParent.size())
   {
      SimObject * obj = tmpParent.front();
      sg->getManager()->addObject(obj);
      sg->addObject(obj);
      
      // send it an edit event();
      SimEditEvent::post( obj, SimMissionEditorId, mFlags.test(InputFocus));

      // select it
      con->evaluatef("MissionObjectList::SelectObject( %d, %d );",
         getWorldId(obj), obj->getId());
   }
   
   // move all the objects
   for(Vector<SimObject*>::iterator itr2 = objList.begin(); itr2 != objList.end(); itr2++)
   {
      if(toCopyPosition)
         onDrop((*itr2), DropToCopyPosition, false);
      else
      {
         onDrop((*itr2), mDropType, false);
         if(mFlags.test(DropToGround))
            dropObjectDown((*itr2), false );
      }
   }
      
   getManager()->unregisterObject( &tmpParent );
   unlockManager();
}

//----------------------------------------------------------------

void FearMissionEditor::pasteSelection()
{
   pasteFile(BUFFER_NAME, true);
}

//----------------------------------------------------------------

void FearMissionEditor::deleteSelection()
{
   // clear the selection
   mSelection.deleteSet();
   CMDConsole::getLocked()->evaluatef( "MissionObjectList::ClearSelection();" );
}

//----------------------------------------------------------------

void FearMissionEditor::placeBookmark( int index )
{
   if( index < 0 || index > 9 )
      return;
      
   SimObject * cam = getCamera();
   if( cam )
   {
      getTransform( cam, mBookmarkPos[index] );
      mBookmarkPos[index].flags |= TMat3F::Matrix_HasTranslation | 
         TMat3F::Matrix_HasRotation;
   }
}

//----------------------------------------------------------------

void FearMissionEditor::gotoBookmark( int index )
{
   if( index < 0 || index > 9 )
      return;
      
   SimObject * cam = getCamera();
   if( cam )
      setTransform( cam, mBookmarkPos[index] );
}

//----------------------------------------------------------------
// really simple undo
void FearMissionEditor::saveInfo( SimObject * obj, bool allowRecurse )
{
   if( allowRecurse )
   {
      mUndoInfo.clear();
      SimSet * ss;
      if( ( ss = dynamic_cast< SimSet * >( obj ) ) != NULL )
      {
         for( SimSet::iterator itr = ss->begin(); itr != ss->end(); itr++ )
            saveInfo( (*itr), false );
         return;
      }
   }
   
   TMat3F info;
   if( getTransform( obj, info ) );
   {
      InfoEntry entry;
      entry.mTmat = info;
      entry.mNum = obj->getNumber();
      entry.mId = obj->getId();
      entry.mManager = obj->getManager();
      mUndoInfo.push_back( entry );
   }
}

//----------------------------------------------------------------

void FearMissionEditor::restoreInfo()
{        
   for( int i = 0; i < mUndoInfo.size(); i++ )
   {
      SimObject * obj = mUndoInfo[i].mManager->findObject( avar( "%d", mUndoInfo[i].mId ) );
      if( obj && ( mUndoInfo[i].mNum == obj->getNumber() ) )
         setTransform( obj, mUndoInfo[i].mTmat );
   }
   mUndoInfo.clear();
}

//----------------------------------------------------------------

void FearMissionEditor::loadCursor( const int cursorId, const char * name, Point2I & hotspot )
{
   CursorInfo * info = new CursorInfo;
   info->mCursorId = cursorId;
   loadBitmap( info->mBmp, name );
   info->mHotspot = hotspot;
   mCursors.push_back( info );
}

//----------------------------------------------------------------

void FearMissionEditor::setCursor( const int cursorId )
{
   for( int i = 0; i < mCursors.size(); i++ )
      if( mCursors[i]->mCursorId == cursorId )
      {
         getCanvas()->setCursor( mCursors[i]->mBmp, mCursors[i]->mHotspot );
         return;
      }
}

//----------------------------------------------------------------
void FearMissionEditor::loadBitmap( Resource<GFXBitmap> & hBitmap, const char * name )
{
   hBitmap = SimResource::get( getManager() )->load( name );
   hBitmap->attribute |= BMA_TRANSPARENT;
}

//----------------------------------------------------------------

void FearMissionEditor::rebuildCommandMap()
{
   // clear the edit render stuff
   BitSet32 flags = mFlags;
   mFlags.set(ShowEditObjects, false);
   mFlags.set(RenderHandles, false);
   
   // make sure all objects get out of edit mode (dont want edit images)
   SimEditEvent ev;
   ev.editorId = SimMissionEditorId;
   ev.state = false;
   manager->processEvent(&ev);
   
   // rebuild the map
   CMDConsole::getLocked()->executef(1, "rebuildCommandMap");
   
   // reenable the editor
   ev.state = true;
   manager->processEvent(&ev);
   
   // restore the flags
   mFlags = flags;
}
