//------------------------------------------------------------------------------
// Description 
//    Mission Editor class for handling multiple selections
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//               (c) Copyright 1996, Dynamix Inc.   All rights reserved.
//
//------------------------------------------------------------------------------

#include "MissionEditor.h"
#include "simGuiTSCtrl.h"
#include "simTerrain.h"
#include "simGuiCanvas.h"
#include "g_bitmap.h"
#include <console.h>

//------------------------------------------------------------------------------

MissionEditor::SelectionSet::SelectionSet()
{
   handCursor      = NULL;
   oldCursor       = NULL;
   grabMask        = -1 & ~(SimTerrainObjectType | SimContainerObjectType);
   placeMask       = SimTerrainObjectType;
   granularity.set(1.0f,1.0f,0.1f);
   rotDegreeSnap.set( 90.0, 90.0, 90.0 );
   rotGranularity  = 0.01f;
   undo.setParent( this );
}   

bool MissionEditor::SelectionSet::anyObjectLocked()
{
   for( SimSet::iterator itr = begin(); itr != end(); itr++ )
      if( (*itr)->isLocked() )
         return( true );
   return( false );
}

//------------------------------------------------------------------------------

SimObject* MissionEditor::SelectionSet::addObject( SimObject *obj )
{
   if( find( begin(), end(),obj ) == end() )
   {
   	objectList.pushBack( obj );
      parent->hilightObjectItem( obj );
      parent->refresh();
      calculateCentroid();
      return( obj );
   }
   
   return( NULL );
}   

//------------------------------------------------------------------------------

bool MissionEditor::SelectionSet::toggleObject( SimObject *obj )
{
   // check if in the set or not
   if( find( begin(), end(), obj ) == end() )
   {
      // add to the set
   	objectList.pushBack(obj);
      parent->hilightObjectItem( obj );
      parent->refresh();
      calculateCentroid();
      return( true );
   }
   else
   {
      // if object already in list, then remove it
      removeObject( obj ); 
      parent->refresh();
      calculateCentroid();
      return( false );
   }
}

//------------------------------------------------------------------------------

void MissionEditor::SelectionSet::removeObject( SimObject *obj )
{
	Parent::removeObject(obj);
//      parent->refresh();
}   

//------------------------------------------------------------------------------

bool MissionEditor::SelectionSet::processEvent( const SimEvent * )
{
   return false;
}

//------------------------------------------------------------------------------

bool MissionEditor::SelectionSet::processQuery(SimQuery *query)
{
   return ( Parent::processQuery( query ) );
}

//------------------------------------------------------------------------------

void MissionEditor::SelectionSet::onRemove()
{
   Parent::onRemove();
}

//------------------------------------------------------------------------------

void MissionEditor::SelectionSet::onDeleteNotify(SimObject* obj)
{
	objectList.removeStable(obj);
	Parent::onDeleteNotify(obj);
}

//------------------------------------------------------------------------------
// number of pixels for full turn
float MissionEditor::SelectionSet::getMouseRotateSensitivity()
{
   const char * var = CMDConsole::getLocked()->getVariable( "MED::mouseRotateSens" );
   if( !var || !var[0] )
      return( 50.f );
   
   return( atof( var ) );
}

//------------------------------------------------------------------------------
// number of world units per pixels
float MissionEditor::SelectionSet::getMouseMoveSensitivity()
{
   const char * var = CMDConsole::getLocked()->getVariable( "MED::mouseMoveSens" );
   if( !var || !var[0] )
      return( 1.f );
   
   return( atof( var ) );
}

//------------------------------------------------------------------------------

bool MissionEditor::SelectionSet::getPlaneIntercepts( const Sim3DMouseEvent * event )
{
   m_lastPlaneXYIntercept = m_planeXYIntercept;
   m_lastPlaneXZIntercept = m_planeXZIntercept;
   
   // get the camera transform
   TMat3F tmat;
   SimObject * cam = parent->getCamera();
   if( !cam || !parent->getCameraTransform( tmat ) )
      return( false );
   Point3F & camPos = tmat.p;
      
   Point3F mouseVec = event->direction;

   Point3F posA = camPos;
   Point3F posB = camPos + mouseVec * 10000.f;
   Point3F castVec = posA - posB;

   // up/down movement
   double planeDistance = camPos.y - m_planeOrigin.y;
   double projectedDistance = m_dot( castVec, Point3F( 0, -1, 0 ) );
   if( projectedDistance == 0.f )
      return( false );
   double scale = planeDistance / projectedDistance;

   m_planeXZIntercept = posA + ( castVec * scale );
   
   planeDistance = camPos.z - m_planeOrigin.z;
   projectedDistance = m_dot( castVec, Point3F( 0, 0, -1 ) );
   if( projectedDistance == 0.f )
      return( false );
   scale = planeDistance / projectedDistance;
   
   m_planeXYIntercept = posA + ( castVec * scale );

   return( true );
}

//------------------------------------------------------------------------------
// TEMP function
bool MissionEditor::SelectionSet::useOldMovement()
{
   const char * var = CMDConsole::getLocked()->getVariable( "MED::useOldMovementMethod" );
   if( !var || !var[0] )
      return( true );
   return( stricmp( var, "true" ) ? false : true );
}

//------------------------------------------------------------------------------
#define DBLCLICK_DELAY  400
void MissionEditor::SelectionSet::handleSim3DMouseEvent( const Sim3DMouseEvent *event )
{
   // check for old movement... TEMP
   if( useOldMovement() )
   { 
      oldHandleSim3DMouseEvent( event );
      return;
   }
   
   // get the current mouse position
   Point2I mousePos( parent->parentCanvas->getCursorPos() );
   Point2F mouseDif( mousePos.x - lastMousePos.x, mousePos.y - lastMousePos.y );
   
   switch( event->meType )
   {
      // handles what cursor to display and some info
      case Sim3DMouseEvent::MouseMove:
      {
         // show coordinates that can be placed at
         if( parent->getLOSIntercept( event, &placeInfo, placeMask ) )
         {
            Point3F pt;
            m_mul( placeInfo.surfaces[0].position, placeInfo.surfaces.tWorld, &pt );
            SetWindowText( parent->getHandle(), 
               avar("%s    %6.2f, %6.2f, %6.2f", parent->getCaption(), pt.x, pt.y, pt.z) );
         }

         // check if we can grab this object         
         if( parent->getLOSIntercept( event, &grabInfo, grabMask ) )
         {
            // if locked then set locked cursor - otherwise shot manipulation cursor
            if( grabInfo.object->isLocked() )
               parent->parentCanvas->setCursor( lockCursor );
            else
               parent->parentCanvas->setCursor( handCursor );
         }
         else
         {
            // set to the arrow cursor
            parent->parentCanvas->setCursor( oldCursor );
         }
            
         break;
      }
         
      // handle object selection - set left mouse down flag
      case Sim3DMouseEvent::MouseDown:
      {
         flags.clear( leftMouseDrag );
         
         // check for multiple selecting - selects on mouseUp
         if( event->modifier & SI_SHIFT )
            break;

         // clear the set
         clearSet();
                  
         // add a hit object
         if( parent->getLOSIntercept( event, &grabInfo, grabMask ) )
         {
            addObject( grabInfo.object );
            undo.setInfo();

            calculateCentroid();
            
            // get the plane origin for the move operations
            m_planeOrigin = centroidPt;
         
            // get the offset to the centroid from the hit position
            m_mul( grabInfo.surfaces[0].position, grabInfo.surfaces.tWorld, &centroidOffset );
            centroidOffset -= centroidPt;
         }
         
         // check the placement
         if( parent->getLOSIntercept( event, &placeInfo, placeMask ) )
         {
            m_mul( placeInfo.surfaces[0].position, placeInfo.surfaces.tWorld, &lastPt );
            
            if( parent->state.test(MissionEditor::ME_SNAP_GRID ) )
            {
               lastPt.x += granularity.x / 2;
               lastPt.y += granularity.y / 2;
               lastPt.z += granularity.z / 2;
               lastPt.x -= fmod( lastPt.x, granularity.x );
               lastPt.y -= fmod( lastPt.y, granularity.y );
               lastPt.z -= fmod( lastPt.z, granularity.z );
            }
         }
         
         lastMousePos = mousePos;
         
         getPlaneIntercepts( event );
            
         break;
      }
         
      // set right mouse down flag
      case Sim3DMouseEvent::RightMouseDown:
      {   
         flags.clear( rightMouseDrag );
         calculateCentroid();
         lastMousePos = mousePos;
         
         
         parent->parentCanvas->setUseWindowsMouseEvents(false);
//         parent->parentCanvas->setCursorON( false );
//         // hide the cursor
//         parent->parentCanvas->showCursor( false );
//         ::SetCursor( NULL );
            
         break;
      }
      
      case Sim3DMouseEvent::MouseUp:
      {
         if( flags.test( leftMouseDrag ) )
            break;
            
         // check for dbl-click
         if( ( GetTickCount() - lastTime ) < DBLCLICK_DELAY )
         {
            if( size() && parent->getLOSIntercept( event, &grabInfo, grabMask ) )
            {
               parent->inspectObject( grabInfo.object );
//               parent->hilightObjectItem( grabInfo.object, true );
            }
            break;
         }
         
         // check for selecting of multiple items...
         if( !flags.test( leftMouseDrag ) &&
            ( event->modifier & SI_SHIFT ) &&
            ( parent->getLOSIntercept( event, &grabInfo, grabMask ) ) )
            {
               toggleObject( grabInfo.object );
               undo.setInfo();
            }

         // get the current tick count for dbl-click purposes               
         lastTime = GetTickCount();
         
         break;
      }
      
      case Sim3DMouseEvent::RightMouseUp:
      {
         // show the cursor
         parent->parentCanvas->showCursor( true );
         ::SetCursor( ::LoadCursor( NULL, IDC_ARROW ) );
            
         break;
      }
         
      // need to move the objects
      case Sim3DMouseEvent::MouseDragged:
      {
         flags.set( leftMouseDrag );
         if( mouseDif.x == 0.f && mouseDif.y == 0.f )
            break;
         
         // check to proceed
         if( anyObjectLocked() || !size() )
            break;

         if( !getPlaneIntercepts( event ) )
            break;

         EulerF objRotate( 0.f, 0.f, 0.f );
         Point3F objDelta( 0.f, 0.f, 0.f );
                     
         bool hasRotation = false;
         bool hasDelta = false;
         bool allowDrop = false;
         
         // check the method of manipulation
         if( event->modifier & SI_CTRL )
         {
            objDelta = m_planeXZIntercept - m_lastPlaneXZIntercept;
            objDelta.x = 0.f;
            hasDelta = true;
         }
         else if( event->modifier & SI_ALT )
         {
            // rotate around the y plane
            Point3F a = m_planeXZIntercept - m_planeOrigin;
            Point3F b = m_lastPlaneXZIntercept - m_planeOrigin;
            a.normalize();
            b.normalize();
            
            // rotate the object...
            AngleF ang = acos( m_dot( a, b ) );
            
            // find out if it was cw or ccw
            Point3F cross;
            m_cross( a, b, &cross );

            objRotate.set( 0.f, ( cross.y > 0.f ) ? -ang : ang, 0.f );
            hasRotation = true;
         }
         else if( event->modifier & SI_SHIFT )
         {
            // rotate around the z-plane
            Point3F a = m_planeXYIntercept - m_planeOrigin;
            Point3F b = m_lastPlaneXYIntercept - m_planeOrigin;
            a.normalize();
            b.normalize();
            
            // rotate the object...
            AngleF ang = acos( m_dot( a, b ) );
            
            // find out if it was cw or ccw
            Point3F cross;
            m_cross( a, b, &cross );
            
            objRotate.set( 0.f, 0.f, ( cross.z > 0 ) ? -ang : ang );
            hasRotation = true;
         }
         else
         {
            // normal movement
            if( !parent->getLOSIntercept( event, &placeInfo, placeMask ) )
               break;
               
            Point3F pt;
            m_mul( placeInfo.surfaces[0].position, placeInfo.surfaces.tWorld, &pt );

            if( parent->state.test( MissionEditor::ME_SNAP_GRID ) )
            {
               pt.x += granularity.x/2;
               pt.y += granularity.y/2;
               pt.z += granularity.z/2;
               pt.x -= fmod( pt.x,granularity.x );
               pt.y -= fmod( pt.y,granularity.y );
               pt.z -= fmod( pt.z,granularity.z );

               // allow concrete snapping to coordinates for single object only
               if( ( size() == 1 ) && ( ( Point2F )lastPt != ( Point2F )pt ) )
                  lastPt = centroidPt;
            }
            
            objDelta = pt - lastPt;
            hasDelta = true;
            allowDrop = true;
         }
         
         // check what was worked on...
         if( hasRotation )
         {
            parent->rotateObject( this, centroidPt, objRotate );
         }
         else if( hasDelta )
         {
            parent->moveObjectRelative( this, objDelta );
            if( allowDrop && parent->state.test( MissionEditor::ME_DROP_TG ) )
               parent->onDropDown( this );
         }

         // get the lastPt of intersection with a placement obj.   
         if( parent->getLOSIntercept( event, &placeInfo, placeMask ) )
         {
            m_mul( placeInfo.surfaces[0].position, placeInfo.surfaces.tWorld, &lastPt );
            SetWindowText( parent->getHandle(), 
               avar("%s    %6.2f, %6.2f, %6.2f", parent->getCaption(), lastPt.x, lastPt.y, lastPt.z ) );
         }
         
         calculateCentroid();
         
         break;
      }
         
      // the camera movement...
      case Sim3DMouseEvent::RightMouseDragged:
      {
         return;
         flags.set( rightMouseDrag );
         if( mouseDif.x == 0.f && mouseDif.y == 0.f )
            return;
         
         TMat3F tmat;
         SimObject * cam = parent->getCamera();
         if( !cam || !parent->getCameraTransform( tmat ) )
            return;
         
         // check camera manipulation method
         if( event->modifier & SI_SHIFT )
         {
            // move along the xy plane
            Point3F move( 0.f, 0.f, 0.f );

            float mouseSens = getMouseMoveSensitivity();
            
            // get the move amount
            move.x = mouseDif.x * mouseSens;
            move.y = -mouseDif.y * mouseSens;
            
            Point3F offset;
            m_mul( move, RMat3F( tmat ), &offset );
            
            parent->moveObjectRelative( cam, offset );
         }
         else if( event->modifier & SI_CTRL )
         {
            // move along the xz plane
            Point3F move( 0.f, 0.f, 0.f );

            float mouseSens = getMouseMoveSensitivity();
            
            // get the move amount
            move.x = mouseDif.x * mouseSens;
            move.z = -mouseDif.y * mouseSens;
            
            Point3F offset;
            m_mul( move, RMat3F( tmat ), &offset );
                        
            parent->moveObjectRelative( cam, offset );
         }
         else if( event->modifier & SI_ALT )
         {
            // this rotates the camera around an object(s) 
            // centroid, need to get transform to object from 
            // camera, do rotation, then reget transform to set
            // the camera's rotation
            
            // make sure something is selected
            if( !size() )
               return;
               
            float mouseSens = getMouseRotateSensitivity();

            // get the vector to the object(s)
            Point3F objVector = centroidPt - tmat.p;
            objVector.normalize();
            
            Point3F normA, normB;
            
            // get the normal to this and 'up' vector's
            m_cross( objVector, Point3F( 0, 0, -1 ), &normA );
            normA.normalize();
            
            // get normal between the normal and the objvector
            m_cross( objVector, normA, &normB );
            normB.normalize();
            
            // set the transform with these vectors as axis'...
            tmat.setRow( 0, normA );
            tmat.setRow( 1, objVector );
            tmat.setRow( 2, normB );
            tmat.flags |= TMat3F::Matrix_HasRotation;

            // get the amount to rotate
            EulerF camRot( 0.f, 0.f, 0.f );
            camRot.z = -mouseDif.x / ( M_PI * mouseSens );
            camRot.x = mouseDif.y / ( M_PI * mouseSens );
         
            RMat3F rotMat( camRot );
         
            objVector = centroidPt - tmat.p;
            objVector.set( 0.f, -objVector.len(), 0.f );
            Point3F offset;
            m_mul( objVector, RMat3F( rotMat ), &offset );
         
            m_mul( offset, RMat3F( tmat ), &objVector );
            tmat.p = centroidPt + objVector;

            // now re get the transform to the obj...
            objVector = centroidPt - tmat.p;
            objVector.normalize();
            m_cross( objVector, Point3F( 0, 0, -1 ), &normA );
            normA.normalize();
            m_cross( objVector, normA, &normB );
            normB.normalize();
         
            // set the transform
            tmat.setRow( 0, normA );
            tmat.setRow( 1, objVector );
            tmat.setRow( 2, normB );
               
            parent->setCameraTransform( tmat );
         }
         else
         {
            float mouseSens = getMouseRotateSensitivity();
  
            EulerF camRot;
            Point3F & camPos = tmat.p;
            tmat.angles( &camRot );
            
            Point3F rot( 0.f, 0.f, 0.f );
            camRot.z += -mouseDif.x / ( M_PI * mouseSens );
            camRot.x += -mouseDif.y / ( M_PI * mouseSens );

            // make sure cannot flip the camera
            if( camRot.x < -( (float)M_PI / 2.0f ) )
               camRot.x = -( (float)M_PI / 2.0f );
            if( camRot.x > ( (float)M_PI / 2.0f ) )
               camRot.x = ( (float)M_PI / 2.0f );
               
            tmat.set( camRot, camPos );
            
            parent->setCameraTransform( tmat );
         }
         
         mousePos = lastMousePos;
         parent->parentCanvas->setCursorPos( mousePos );
         POINT pos;
         pos.x = mousePos.x;
         pos.y = mousePos.y;
         if( ::ClientToScreen( parent->parentCanvas->getHandle(), &pos ) )
            ::SetCursorPos( pos.x, pos.y );

         break;
      }
   }
   
   // get the current mouse window's position
   lastMousePos = mousePos;
}   

void MissionEditor::SelectionSet::oldHandleSim3DMouseEvent( const Sim3DMouseEvent *event )
{
   switch(event->meType) 
   {
      case Sim3DMouseEvent::MouseMove: 
      
         // show terrain coordinates of cursor
         if( parent->getLOSIntercept( event, &placeInfo, placeMask ) )
         {
            Point3F pt;
            m_mul( placeInfo.surfaces[0].position, placeInfo.surfaces.tWorld, &pt );
            SetWindowText( parent->getHandle(), 
               avar("%s    %6.2f, %6.2f, %6.2f", parent->getCaption(), pt.x, pt.y, pt.z) );
         }
         
         if( parent->getLOSIntercept( event, &grabInfo, grabMask ) )
         {
            if( grabInfo.object->isLocked() )
               parent->parentCanvas->setCursor( lockCursor );
            else
               parent->parentCanvas->setCursor( handCursor );
         }
         else
            parent->parentCanvas->setCursor( oldCursor );
            
         break;

      case Sim3DMouseEvent::RightMouseDown:
      case Sim3DMouseEvent::MouseDown:
         {
            // reset the drag flag
            if( event->meType == Sim3DMouseEvent::MouseDown )
               mouseLeftDrag = false;
            else
            {
               mouseRightDrag = false;
               rotSum = EulerF( 0,0,0 );
            }
               
            lastPOINT( parent->parentCanvas->getCursorPos() );

            // clear set if not in multiple select mode, otherwise
            if( ( GetKeyState( VK_SHIFT ) & 0x80000000 ) ||
               ( GetKeyState( VK_CONTROL ) & 0x80000000 ) )
               break;
               
            SimGui::TSControl *send = dynamic_cast<SimGui::TSControl*>( event->sender );
            if( send ) 
               send->mouseLock();

            if( parent->getLOSIntercept( event, &placeInfo, placeMask ) )
               m_mul( placeInfo.surfaces[0].position, placeInfo.surfaces.tWorld, &lastPt );

            if ( parent->state.test(MissionEditor::ME_SNAP_GRID) )
            {
               lastPt.x += granularity.x / 2;
               lastPt.y += granularity.y / 2;
               lastPt.z += granularity.z / 2;
               lastPt.x -= fmod( lastPt.x, granularity.x );
               lastPt.y -= fmod( lastPt.y, granularity.y );
               lastPt.z -= fmod( lastPt.z, granularity.z );
            }
            // add the object - clear the set if hit nothing
            if( parent->getLOSIntercept( event, &grabInfo, grabMask ) )
            {
               // clear the selection if this is a new object in our set
               if( addObject( grabInfo.object ) )
               {
                  clearSet();
                  addObject( grabInfo.object );
               }
            }
            else
               clearSet();
  
            // always rotate around the centroid             
            calculateCentroid();
            
            // set the info for the objects in this set
            undo.setInfo();
         }
         break;

      case Sim3DMouseEvent::MouseUp:
      case Sim3DMouseEvent::RightMouseUp:
         {
            SimGui::TSControl *send = dynamic_cast<SimGui::TSControl*>(event->sender);

            if( send )
               send->mouseUnlock();

            // check the button
            if( event->meType == Sim3DMouseEvent::MouseUp )
            {
               // check for a selection and for no drag'n
               if( !mouseLeftDrag )
               {
                  // check for dbl-click
                  if( ( GetTickCount() - lastTime ) < DBLCLICK_DELAY )
                  {
                     // if we hit something then inspect it
                     if( size() && parent->getLOSIntercept( event, &grabInfo, grabMask ) )
                        parent->inspectObject( grabInfo.object );
                  }               
                  else
                  {
                     // if control or shift then toggle the object
                     if( ( ( GetKeyState( VK_SHIFT ) & 0x80000000 ) ||
                        ( GetKeyState( VK_CONTROL ) & 0x80000000 ) ) &&
                        parent->getLOSIntercept(event, &grabInfo, grabMask ) )
                        {
                           toggleObject( grabInfo.object );
                           undo.setInfo();
                        }
                  }
               }
            }
            else
            {
               if( !mouseRightDrag && ( GetKeyState( VK_CONTROL ) & 0x80000000 ) )
                  parent->contextMenu();
            }

            // get the current tick            
            lastTime = GetTickCount();
         }
         break;

      // move objects
      case Sim3DMouseEvent::MouseDragged:
      {
         // set the drag flag
         mouseLeftDrag = true;

         // check to proceed
         if( anyObjectLocked() || !size() || !( GetTickCount() - lastTime > DBLCLICK_DELAY ) )
            break;

         Point3F pt, delta;
         Point2I PT;
         PT( parent->parentCanvas->getCursorPos() );

         float modifier = 1.0f;
         
         // check the modifier keys
         if( GetKeyState( 'Z' ) & 0x80000000 )
            modifier *= 10;
         if( GetKeyState( 'X' ) & 0x80000000 )
            modifier /= 10;
           
         // check for relative movement           
         if( GetKeyState( VK_SHIFT ) & 0x80000000 ) // move in x/y by mouse offset
         {
            // get camera z rot
            TMat3F tran;
            if( !parent->getCameraTransform( tran ) )
               break;
               
            // rotate around z then scale
            EulerF rot;
            tran.angles( &rot );
            
            float dx = ( PT.x - lastPOINT.x );
            float dy = ( lastPOINT.y - PT.y ); 
            
            float xOffset = ( dx * cos( rot.z ) - dy * sin( rot.z ) ) * ( granularity.x * modifier );
            float yOffset = ( dx * sin( rot.z ) + dy * cos( rot.z ) ) * ( granularity.y * modifier );

            // check if camera is upsidedown
            if( rot.y == 0.f )
               delta.set( xOffset, yOffset, 0 );
            else
               delta.set( -xOffset, -yOffset, 0 );
               
            pt = delta + lastPt;
         }
         else if( GetKeyState( VK_CONTROL ) & 0x80000000 )  // move in z only by mouse offset
         {
            delta.set( 0, 0, ( lastPOINT.y - PT.y ) * ( granularity.z * modifier ) );
            pt = delta + lastPt;
         }
         else // move to point intercepted on terrain
         {
            if( !parent->getLOSIntercept( event, &placeInfo, placeMask ) )
               break;
               
            m_mul( placeInfo.surfaces[0].position, placeInfo.surfaces.tWorld, &pt );
         }
         
         SetWindowText( parent->getHandle(), 
            avar("%s    %6.2f, %6.2f, %6.2f", parent->getCaption(), pt.x, pt.y, pt.z ) );
         
         if( parent->state.test( MissionEditor::ME_SNAP_GRID ) )
         {
            pt.x += granularity.x/2;
            pt.y += granularity.y/2;
            pt.z += granularity.z/2;
            pt.x -= fmod( pt.x,granularity.x );
            pt.y -= fmod( pt.y,granularity.y );
            pt.z -= fmod( pt.z,granularity.z );

            // allow concrete snapping to coordinates for single object only
            if( ( size() == 1 ) && ( ( Point2F )lastPt != ( Point2F )pt ) )
               lastPt = centroidPt;
         }

         // get the delta
         delta = pt;
         delta -= lastPt;
      
         parent->moveObjectRelative( this, delta );
         
         if( parent->state.test( MissionEditor::ME_DROP_TG ) )
            parent->onDropDown( this );

         // save the last 3d and screen mouse positions
         lastPt = pt; // 3d
         lastPOINT = PT; // screen
         
         calculateCentroid();
         
         break;
      }
      
      // rotate object(s)
      case Sim3DMouseEvent::RightMouseDragged:  
      {      
         // set the drag flag
         mouseRightDrag = true;
      
         if( anyObjectLocked() || !size() || !( GetTickCount()-lastTime > DBLCLICK_DELAY ) )
            break;
            
         Point3F  pt;
         EulerF rot( 0, 0, 0 );
         Point2I PT;
         PT( parent->parentCanvas->getCursorPos() );
         
         // convert to radians         
         EulerF rotSnap( ( M_PI / ( 180 / rotDegreeSnap.x ) ), 
            ( M_PI / ( 180 / rotDegreeSnap.y ) ), 
            ( M_PI / ( 180 / rotDegreeSnap.z ) ) );
         
         float modifier = 1.0f;
         
         // check the modifier keys
         if( GetKeyState( 'Z' ) & 0x80000000 )
            modifier *= 10;
         if( GetKeyState( 'X' ) & 0x80000000 )
            modifier /= 10;
            
         // check for type of rotation
         if( GetKeyState( VK_SHIFT ) & 0x80000000 )
         {
            float dx = ( PT.x - lastPOINT.x );

            // set the rotation amount
            if( parent->state.test( MissionEditor::ME_ROT_X ) )
               rot.x = dx * ( rotGranularity * modifier );
            if( parent->state.test( MissionEditor::ME_ROT_Y ) )
               rot.y = dx * ( rotGranularity * modifier );
            if( parent->state.test( MissionEditor::ME_ROT_Z ) )
               rot.z = dx * ( rotGranularity * modifier );
         }
         else if( parent->state.test( MissionEditor::ME_ROT_Z ) ) // just rotating the z
         {
            // check for intercept
            if( !parent->getLOSIntercept( event, &placeInfo, placeMask ) )
               break;
               
            m_mul( placeInfo.surfaces[0].position, placeInfo.surfaces.tWorld, &pt );
            
            if( pt.y-centroidPt.y && pt.x-centroidPt.x && lastPt.y-centroidPt.y && lastPt.x-centroidPt.x )
            {
               rot.z = m_atan( pt.x-centroidPt.x, pt.y-centroidPt.y  );
               rot.z -= m_atan( lastPt.x-centroidPt.x, lastPt.y-centroidPt.y  );
            }
         }
            
         SetWindowText( parent->getHandle(), 
            avar("%s    %6.2f, %6.2f, %6.2f", parent->getCaption(), pt.x, pt.y, pt.z) );

         if( parent->state.test( MissionEditor::ME_SNAP_ROT ) )
         {
            EulerF rotA;
            
            rotA.x = rotSum.x - fmod( rotSum.x, rotSnap.x );
            rotA.y = rotSum.y - fmod( rotSum.y, rotSnap.y );
            rotA.z = rotSum.z - fmod( rotSum.z, rotSnap.z );

            // store up the rotations
            rotSum += rot;

            EulerF rotB;
            
            rotB.x = rotSum.x - fmod( rotSum.x, rotSnap.x );
            rotB.y = rotSum.y - fmod( rotSum.y, rotSnap.y );
            rotB.z = rotSum.z - fmod( rotSum.z, rotSnap.z );

            rot = ( rotB - rotA );
            
            // set the rotation amount
            if( !parent->state.test( MissionEditor::ME_ROT_X ) ) rot.x = 0.0;
            if( !parent->state.test( MissionEditor::ME_ROT_Y ) ) rot.y = 0.0;
            if( !parent->state.test( MissionEditor::ME_ROT_Y ) ) rot.y = 0.0;
            
            // check for absolute rotations for a single obj
            if( size() == 1 )
               parent->rotateObject( this, rot, rotSnap );
            else
               parent->rotateObject( this, centroidPt, rot );         
         }
         else
            // rotate the object(s)
            parent->rotateObject( this, centroidPt, rot );
            
         // save the last 3d and screen mouse positions
         lastPt = pt; // 3d
         lastPOINT = PT; // screen
         
         break;
      }
   }
}   


//------------------------------------------------------------------------------

void MissionEditor::SelectionSet::calculateCentroid()
{
   centroidPt.set(0,0,0);
   int count = recurseCentroid( this );
   if ( count )
      centroidPt /= count;
}   

int MissionEditor::SelectionSet::recurseCentroid( SimObject *obj )
{
   int objCount = 0;

   SimSet   *ss;
   if ( (ss = dynamic_cast<SimSet*>(obj)) != NULL )
      for ( SimSet::iterator itr=ss->begin(); itr!=ss->end(); itr++ )
         objCount += recurseCentroid( (*itr) ); // recurse into children

   SimObjectTransformQuery objQuery;

   if ( obj->processQuery(&objQuery) )
   {
      objCount++;
      centroidPt += objQuery.tmat.p;
   }
   return objCount;
}   

//------------------------------------------------------------------------------

void MissionEditor::SelectionSet::clearSet()
{
   while( size() )
      removeObject( front() );
      parent->refresh();
}   

//------------------------------------------------------------------------------

void MissionEditor::SelectionSet::deleteSet()
{
   while( size() )
   {
      if ( !front()->isDeleted() )
         front()->deleteObject();
      objectList.pop_front();
   }
   clearSet();
}   

//------------------------------------------------------------------------------

bool MissionEditor::SelectionSet::isObjectSelected( UINT id, SimObject *obj )
{
   if ( !obj )
      obj = this;
   if ( obj->getId() == id )
      return true;
   
   bool found = false;
   SimSet *ss = dynamic_cast<SimSet*>(obj);
   if ( ss )
   {
      for ( SimSet::iterator itr=ss->begin(); !found && itr!=ss->end(); itr++ )
            found = isObjectSelected( id, (*itr) );
   }
   return found;
}   

//------------------------------------------------------------------------------

float MissionEditor::SelectionSet::findLowest( SimObject *obj )
{
   float    lowest = -9999999.9f;
   SimSet   *ss;

   if ( (ss = dynamic_cast<SimSet*>(obj)) != NULL )
   {
      for ( SimSet::iterator itr=ss->begin(); itr!=ss->end(); itr++ )
         lowest = max( lowest, findLowest(*itr) ); // recurse into children to find lowest
   }
   else
   {
      SimObjectTransformQuery objQuery;
      if ( obj->processQuery(&objQuery) )
      {
         Point3F endPt = objQuery.tmat.p;
         endPt.z -= 10000;

         SimCollisionInfo info;
         SimContainerQuery query;
         query.id = obj->getId();   // don't collide with object
         query.type = -1;
         query.box.fMin = objQuery.tmat.p;
         query.box.fMax = endPt;
         query.mask = getPlaceMask();

         if ( !parent->lockManager() )
            return lowest;

         SimContainer *root = NULL;
         
         root = findObject(parent->targetManager, SimRootContainerId, root);
         if ( root->findLOS(query, &info) && (info.object->getType() & getPlaceMask()) )
         {
            Point3F  pt;
            m_mul(info.surfaces[0].position, info.surfaces.tWorld, &pt );
            pt -= objQuery.tmat.p;
            lowest = max( lowest, pt.z );
         }   
         parent->unlockManager();
      }
   }   
   return lowest;
}   

// -- UNDO STUFF ----------------------------------------------

void MissionEditor::SelectionSet::Undo::setInfo()
{
   AssertFatal( parent, "SelectionSet::Undo - parent object not set!" );

   // clear the list
   objList.clear();
   
   // go through the objects and add their info to the list
   for( SimSet::iterator itr = parent->begin(); itr != parent->end(); itr++ )
   {
      SimObject & obj = *(*itr);
      
      SimObjectTransformQuery objQuery;

      // grab the transform
      if( obj.processQuery( &objQuery ) )
      {
         Entry entry;
         entry.tmat = objQuery.tmat;
         entry.id = obj.getId();
         
         objList.push_back( entry );
      }
   }
}

// ------------------------------------------------------------

bool MissionEditor::SelectionSet::Undo::restoreInfo()
{
   SimManager * manager = parent->parent->targetManager;
   if( !manager )
      return( false );
         
   // go through and restore the info
   for( int i = 0; i < objList.size(); i++ )
   {
      char idBuf[20];
      sprintf( idBuf, "%d", objList[i].id );
      
      SimObject * obj = manager->findObject( idBuf );

      // check that we located this object ( could have been deleted! )
      if( obj )
      {
         // apply the old tmat
         SimObjectTransformEvent event( objList[i].tmat );
         obj->processEvent( &event );
      }
   }
   
   return( true );
}