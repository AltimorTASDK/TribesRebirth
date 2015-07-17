// ===================================
// light.cpp : light file
// ===================================

#include "stdafx.h"
#include "Light.h"
#include "lightview.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// static member data - should only be one of the prop stuff's
CLightBlankProp * CItemBase::blankProp = NULL;
CLightPropertySheet * CItemBase::propSheet = NULL;
CItemBase * CItemBase::currentInspectObj = NULL;
CItemBase * CItemBase::copyObj = NULL;
CLightViewTree * CItemBase::tree = NULL;
CTHREDDoc * CItemBase::pDoc = NULL;
CLightProp * CLight::lightProp = NULL;
CLightArray * CLight::currentArray = NULL;
CLightStateProp * CLightState::lightStateProp = NULL;
CLightEmitterProp * CLightEmitter::lightEmitterProp = NULL;

bool CLight::showFalloffs = true;

//---------------------------------------------------------------

CItemBase::CItemBase() :
   type( -1 ),
   baseFlags( 0 ),
   hTreeItem( 0 ),
   _parent( NULL )
{
}

//---------------------------------------------------------------

void CItemBase::inspect( bool read, bool create )
{
   // check for no property sheet
   if( !propSheet )
      propSheet = new CLightPropertySheet( "Properties...", tree );
      
   // remove all the current pages...
   while( propSheet->GetPageCount() )
      propSheet->RemovePage( 0 );
   
   // check for a current inspect item
//TODO
//   if( currentInspectObj )
//      onInspect( false );
   
   // get the property page ( dont need result, but will load if necc.. )
   getPropPage();      
   currentInspectObj = this;
   onInspect( read );

   // show the page
   propSheet->AddPage( getPropPage() );
      
   // chck if should create the window again
   if( !propSheet->m_hWnd && create )
      propSheet->Create( tree );
         
   propSheet->SetActivePage( getPropPage() );
}

//---------------------------------------------------------------

void CItemBase::remove()
{
}

//---------------------------------------------------------------

void CItemBase::onSelect()
{
}

//---------------------------------------------------------------

bool CItemBase::onDragOver( CItemBase * source )
{
   return( false );
}

//---------------------------------------------------------------

void CItemBase::onDrop( CItemBase * source )
{
}

//---------------------------------------------------------------

bool CItemBase::onInspect( bool read )
{
   return( true );
}

//---------------------------------------------------------------

void CItemBase::onContextMenu( CMenu & menu )
{
}

//---------------------------------------------------------------

void CItemBase::onContextMenuSelect( int id )
{
}

//---------------------------------------------------------------

bool CItemBase::onNameChanged( const char * name )
{
   return( false );
}

//---------------------------------------------------------------

void CItemBase::getName( CString & str )
{
   str = "";
}

//---------------------------------------------------------------

bool CItemBase::add( CItemBase * parent, int image, int selectedImage )
{
   // set the parent obj
   _parent = parent;
   
   CString str;
   getName( str );
   
   // insert the item
   hTreeItem = tree->InsertItem( str, image, selectedImage, 
      parent ? parent->hTreeItem : NULL );
   
   // set the data
   tree->SetItemData( hTreeItem, ( DWORD )this );

   return( true );
}

//---------------------------------------------------------------

CPropertyPage * CItemBase::getPropPage()
{
   if( !blankProp )
      blankProp = new CLightBlankProp;
   return( blankProp );
}

//---------------------------------------------------------------

bool CItemBase::canPaste()
{
   return( false );
}

//---------------------------------------------------------------

int CItemBase::getIndex()
{
   return(-1);
}

//---------------------------------------------------------------
// CLight

CLight::CLight( bool createFirst ) :
   id( -1 ),
   animationDuration( 0.f ),
   flags( 0 ),
   group( -1 ),
   m_active( true )
{
   baseFlags = CItemBase::CAN_NAME;

   if( createFirst )
   {
      // set a default name 
      setName( "light" );
      
      // create a default state and emitter..
      CLightState state;
      CLightEmitter emitter;
      state.addEmitter( emitter );
      addState( state );
   }
}

//------------------------------------------------------------------

CLight::~CLight()
{
}

//------------------------------------------------------------------

bool CLight::onDragOver( CItemBase * source )
{
   // get the type.. doing stupid way be ok..
   CLight * light = dynamic_cast< CLight * >( source );
   CLightState * state = dynamic_cast< CLightState * >( source );
   
   // check the source
   if( light && light != this && light->getParent() == getParent() )
      return( true );
  
   // check the source
   if( state && ( state->getParent() != this ) )
      return( true );
   
   return( false );
}

//------------------------------------------------------------------

int CLight::getIndex()
{
   AssertFatal( currentArray, "No light array." );
   
   for( int i = 0; i < currentArray->GetSize(); i++ )
   {
      if( this == &(*currentArray)[i] )
         return( i );
   }
   
   return( -1 );
}

//------------------------------------------------------------------

void CLight::onDrop( CItemBase * source )
{
   // get the type.. doing stupid way be ok..
   CLight * light = dynamic_cast< CLight * >( source );
   CLightState * state = dynamic_cast< CLightState * >( source );
   
   if( light )
   {
      AssertFatal( currentArray, "No light array." );
      
      CLight tmp;
      tmp = (*currentArray)[source->getIndex()];
      
      // position the lights - depends on where they are
      if( source->getIndex() > getIndex() )
      {
         currentArray->RemoveAt( source->getIndex() );
         currentArray->InsertAt( getIndex() + 1, tmp );
      }
      else
      {
         currentArray->InsertAt( getIndex() + 1, tmp );
         currentArray->RemoveAt( source->getIndex() );
      }
      
      // need to rebuild...
      tree->Rebuild( NULL );
      
      return;
   }
   
   if( state )
   {
      CLight * light = ( CLight * )state->getParent();
      
      // copy the state and remove from the list
      CLightState tmp;
      tmp = light->states[ state->getIndex() ];
      light->states.RemoveAt( state->getIndex() );
      
      // add it back in
      addState( tmp );
      
      // do some tree rebuilding...
      tree->Rebuild( this );
      tree->Rebuild( state->getParent() );
      
      return;
   }
}

//------------------------------------------------------------------

CPropertyPage * CLight::getPropPage()
{
   if( !lightProp )
      lightProp = new CLightProp;
   return( lightProp );
}

//------------------------------------------------------------------

bool CLight::onInspect( bool read )
{
   AssertFatal( lightProp, "Failed to inspect object." );
   
   if( read )
   {
      CString str;
      getName( str );
      
      // fill in the data
      lightProp->m_Name = str;
      lightProp->m_Duration = animationDuration;
      
      // set the flags
      lightProp->m_AutoCheck = ( flags & AUTOSTART ) ? true : false;
      lightProp->m_LoopCheck = ( flags & LOOPTOEND ) ? true : false;
      lightProp->m_ManageCheck = ( flags & MANAGEBYMISSIONEDITOR ) ? true : false;
      lightProp->m_Random = ( flags & RANDOMFLICKER ) ? true : false;
   }
   else
   {
      // grab the data
      setName( lightProp->m_Name );
      animationDuration = lightProp->m_Duration;
      
      // get the flags - dont want to destroy bits that not used by dialog jic
      lightProp->m_AutoCheck ? ( flags |= AUTOSTART ) : ( flags &= ~AUTOSTART );
      lightProp->m_LoopCheck ? ( flags |= LOOPTOEND ) : ( flags &= ~LOOPTOEND );
      lightProp->m_ManageCheck ? ( flags |= MANAGEBYMISSIONEDITOR ) : ( flags &= ~MANAGEBYMISSIONEDITOR );
      lightProp->m_Random ? ( flags |= RANDOMFLICKER ) : ( flags &= ~RANDOMFLICKER );
   }
   return( true );
}

//------------------------------------------------------------------

void CLight::onContextMenu( CMenu & menu )
{
   // add the commands
   menu.AppendMenu( MF_SEPARATOR );
   
   UINT menuFlags = MF_STRING;
   
   // check for a lightstate to past
   if( !canPaste() )
      menuFlags |= MF_GRAYED;
   menu.AppendMenu( menuFlags, PASTE_STATE, "Paste State" );
   menu.AppendMenu( MF_STRING, ADD_STATE, "Add State" );
}

//------------------------------------------------------------------

void CLight::onContextMenuSelect( int id )
{
   switch( id )
   {
      case PASTE_STATE:
         
         // paste it
         paste();
         
         break;
      
      case ADD_STATE:
      {
         CLightState lightState;
         addState( lightState );
         tree->Rebuild( this );         
         break;
      }
         
      default:
         break;
   }
}

//------------------------------------------------------------------

void CLight::onSelect()
{
   // reset the entire thing
   pDoc->ResetSelectedLights();   
   select( true );
   pDoc->UpdateAllViews(NULL);
}

//-------------------------------------------------------------------
// ItemBase functions
bool CLight::onNameChanged( const char * newName )
{
   setName( newName );
   return( true );
}

//-------------------------------------------------------------------

bool CLight::selected()
{
   for( int i = 0; i < states.GetSize(); i++ )
      for( int j = 0; j < states[i].numEmitters(); j++ )
      {
         CLightEmitter & emitter = states[i].getEmitter( j );
         if( emitter.getSelect() )
            return( true );
      }
   return( false );
}

//-------------------------------------------------------------------
void CLight::resetSelected()
{
   for( int i = 0; i < states.GetSize(); i++ )
      for( int j = 0; j < states[i].numEmitters(); j++ )
      {
         CLightEmitter & emitter = states[i].getEmitter( j );
         if( emitter.getSelect() )
            emitter.selectable = true;
         else
            emitter.selectable = false;

         emitter.select( false );
      }
}

//-------------------------------------------------------------------
void CLight::moveLight( double x, double y, double z )
{
   for( int i = 0; i < states.GetSize(); i++ )
      for( int j = 0; j < states[i].numEmitters(); j++ )
      {
         CLightEmitter & emitter = states[i].getEmitter( j );
         if( emitter.getSelect() )
         {
            emitter.pos.x -= float( x );
            emitter.pos.y -= float( y );
            emitter.pos.z -= float( z );
         }
      }
}

void CLight::rotateLight( EulerF & rot, Point3F & center )
{
   for( int i = 0; i < states.GetSize(); i++ )
      for( int j = 0; j < states[i].numEmitters(); j++ )
      {
         CLightEmitter & emitter = states[i].getEmitter( j );
         if( emitter.getSelect() )
         {
            // add to the rotation - warning... hack!!!!
            if( emitter.rotValid )
               emitter.rotSum += rot;
            else
            {
               emitter.rotSum = rot;
               emitter.rotPos = center;
            }
            emitter.rotValid = true;
            
            TMat3F rotMat( true );
            rotMat.preRotateX( -rot.y );   // swap x/y
            rotMat.preRotateY( -rot.x );
            rotMat.preRotateZ( -rot.z );
            
            emitter.pos -= center;
            Point3F tmp;
            m_mul( emitter.pos, rotMat, &tmp );
            emitter.pos = tmp + center;
         }
      }
}

void CLight::doneRotateLight( float snapRot ) // in degrees!
{
   for( int i = 0; i < states.GetSize(); i++ )
      for( int j = 0; j < states[i].numEmitters(); j++ )
      {
         CLightEmitter & emitter = states[i].getEmitter( j );
         if( emitter.getSelect() )
         {
            emitter.rotValid = false;

            if( snapRot != 0.f )
            {
               EulerF rot;
               rot.x = fmod( emitter.rotSum.x, snapRot );
               rot.y = fmod( emitter.rotSum.y, snapRot );
               rot.z = fmod( emitter.rotSum.z, snapRot );
               
               // snap it to the nearist snap point.... could have problems if rot.? is exactly half-snap..
               if( fabs( rot.x ) > ( snapRot / 2 ) )
                  rot.x = ( snapRot - fabs( rot.x ) ) * ( ( rot.x > 0.f ) ? -1.f : 1.f );
               if( fabs( rot.y ) > ( snapRot / 2 ) )
                  rot.y = ( snapRot - fabs( rot.y ) ) * ( ( rot.y > 0.f ) ? -1.f : 1.f );
               if( fabs( rot.z ) > ( snapRot / 2 ) )
                  rot.z = ( snapRot - fabs( rot.z ) ) * ( ( rot.z > 0.f ) ? -1.f : 1.f );
                  
               TMat3F rotMat( true );
               rotMat.preRotateX( rot.y );
               rotMat.preRotateY( rot.x );
               rotMat.preRotateZ( rot.z );
               emitter.pos -= emitter.rotPos;
               Point3F tmp;
               m_mul( emitter.pos, rotMat, &tmp );
               emitter.pos = tmp + emitter.rotPos;
            }
         }
      }
}

//-------------------------------------------------------------------
double CLight::getDistance( ThredPoint pnt )
{
   double minDistance = 1000000000.f;
   
   for( int i = 0; i < states.GetSize(); i++ )
      for( int j = 0; j < states[i].numEmitters(); j++ )
      {
         CLightEmitter & emitter = states[i].getEmitter(j);
         
      	double x1, y1, x2, y2;

      	// check which vars to use
      	if( pnt.X == 0 ) {
      		x1 = emitter.pos.y;
      		y1 = emitter.pos.z;
      		x2 = pnt.Y;
      		y2 = pnt.Z;
      	}
      	else if( pnt.Y == 0 ) {
      		x1 = emitter.pos.x;
      		y1 = emitter.pos.z;
      		x2 = pnt.X;
      		y2 = pnt.Z;
      	}
      	else {
      		x1 = emitter.pos.y;
      		y1 = emitter.pos.x;
      		x2 = pnt.Y;
      		y2 = pnt.X;
      	}

      	// calculate our deltas
      	double xdistance = x2 - x1;
      	double ydistance = y2 - y1;
         
         double distance = sqrt( ( xdistance * xdistance ) +
            ( ydistance * ydistance ) );
            
         if( distance < minDistance )
            minDistance = distance;
      }
      
   return( minDistance );
}

//-------------------------------------------------------------------
void CLight::renderLight( CRenderCamera * camera, TSRenderContext * rc, 
   int viewType, bool selected )
{
	Point2I	start, end;
	Point3F	in_point;
   
   // go through all the states
   for( int i = 0; i < states.GetSize(); i++ )
   {
      for( int j = 0; j < states[i].numEmitters(); j++ )
      {
         CLightEmitter & e = states[i].getEmitter(j);
         
         if( e.getSelect() != selected )
            continue;
            
         ThredPoint pos = ThredPoint( e.pos.x, e.pos.y, e.pos.z );
               
         COLORREF col = ( e.getSelect() ? 252 : 251 );
      
         switch( viewType )
         {
            case ID_VIEW_SOLIDRENDER:
            case ID_VIEW_TEXTUREVIEW:
            {
               // check which version of the light to draw
               const float offset = 15.0;
               camera->DrawTri( pos, offset, col, rc );
               
               break;
            }
               
            default:
            {
               CPoint point, scalePoint;

               // translate the point then draw
               camera->TranslateToScreen( point, pos, rc );
               
               ThredPoint fakePos = ThredPoint( pos.X + 8, pos.Y + 8, pos.Z + 8 );
               
               // scale it
               camera->TranslateToScreen( scalePoint, fakePos, rc );

         	   // get a size		
         	   int xsize = scalePoint.x - point.x;

         	   // DPW - Needed for TS compatibility
         	   start.x = point.x - xsize;
         	   start.y = point.y - xsize;
         	   end.x = point.x + xsize;
         	   end.y = point.y + xsize;
         	   camera->DrawLine( &start, &end, 3, col, rc );

         	   // draw another one
         	   start.x = point.x - xsize;
         	   start.y = point.y + xsize;
         	   end.x = point.x + xsize;
         	   end.y = point.y - xsize;
         	   camera->DrawLine( &start, &end, 3, col, rc );
               
               // check for the circle draw...
               if( CLight::showFalloffs )
               {
                  if( e.getSelect() && ( e.lightType == CLightEmitter::PointLight ) &&
                     ( e.falloff == CLightEmitter::Linear ) )
                  {
                     COLORREF innerCol = 251;
                     COLORREF outerCol = 253;
                     
                     CPoint scrCenter;
                     camera->TranslateToScreen( scrCenter, pos, rc );
                     
                     for( int i = 0; i < 2; i++ )
                     {
                        float falloff = i ? e.d2 : e.d1;
                        if( falloff == 0.f )
                           continue;
                        
                        ThredPoint radPnt;
                        if( viewType == ID_VIEW_SIDEVIEW )
                           radPnt = ThredPoint( e.pos.x, e.pos.y, e.pos.z + falloff );
                        else
                           radPnt = ThredPoint( e.pos.x + falloff, e.pos.y, e.pos.z );
                           
                        CPoint scrRad;
                        
                        camera->TranslateToScreen( scrRad, radPnt, rc );
                        
                        int radius = scrRad.x - scrCenter.x;
                        
                        start.set( (int)scrCenter.x, (int)scrCenter.y );
                        camera->DrawCircle( start, radius, i ? outerCol : innerCol, 32, rc );
                     }
                  }
               }
               
               break;
            }
         }
      }
   }
}

//-------------------------------------------------------------------
void CLight::doneMoveLight( double gridSize, bool useEditBox, int viewType )
{
   for( int i = 0; i < states.GetSize(); i++ )
      for( int j = 0; j < states[i].numEmitters(); j++ )
      {
         CLightEmitter & e = states[i].getEmitter( j );
         if( e.getSelect() )
         {
//            // grab the coord. from the doc
//            if( useEditBox )
//            {
//               Point3F disp;
//               disp.set( 0.f, 0.f, 0.f );
//            
//               switch( viewType )
//               {
//                  case ID_VIEW_TOPVIEW:
//                     disp.x = pDoc->EditBox.mLargest.X - (RoundFloat((pDoc->EditBox.mLargest.X / gridSize)) * gridSize);
//                     disp.z = pDoc->EditBox.mLargest.Z - (RoundFloat((pDoc->EditBox.mLargest.Z / gridSize)) * gridSize);
//                     break;
//                  case ID_VIEW_FRONTVIEW:
//                     disp.x = pDoc->EditBox.mLargest.X - (RoundFloat((pDoc->EditBox.mLargest.X / gridSize)) * gridSize);
//                     disp.y = pDoc->EditBox.mLargest.Y - (RoundFloat((pDoc->EditBox.mLargest.Y / gridSize)) * gridSize);
//                     break;
//                  case ID_VIEW_SIDEVIEW:
//                     disp.y = pDoc->EditBox.mLargest.Y - (RoundFloat((pDoc->EditBox.mLargest.Y / gridSize)) * gridSize);
//                     disp.z = pDoc->EditBox.mLargest.Z - (RoundFloat((pDoc->EditBox.mLargest.Z / gridSize)) * gridSize);
//                     break;
//               }
//               
//               e.pos += disp;
//            }
//            else
//            {
               // now clamp
               if( Globals.mAxis & AXIS_X )
                  e.pos.x = RoundFloat( e.pos.x / gridSize ) * gridSize;
               if( Globals.mAxis & AXIS_Y )
                  e.pos.y = RoundFloat( e.pos.y / gridSize ) * gridSize;
               if( Globals.mAxis & AXIS_Z )
                  e.pos.z = RoundFloat( e.pos.z / gridSize ) * gridSize;
//            }
               
         }
      }
}

//-------------------------------------------------------------------

bool CLight::canPaste()
{
   return( dynamic_cast< CLightState * >( copyObj ) ? true : false );
}

//-------------------------------------------------------------------

void CLight::remove()
{
   CItemBase * parent = getParent();
   
   // go through and remove from the array
   currentArray->RemoveAt( getIndex() );
   
   tree->Rebuild( parent );
}

//-------------------------------------------------------------------

void CLight::copy()
{
   // remove the current one
   delete copyObj;
   copyObj = NULL;
   
   // create and copy
   CLight * pLight = new CLight;
   *pLight = *this;
   
   // set
   copyObj = pLight;
}

//-------------------------------------------------------------------

void CLight::paste()
{
   CLightState * pState = dynamic_cast< CLightState * >( copyObj );
   if( pState )
   {
      CLightState state;
      state = *pState;
      addState( state );

      if( tree )
         tree->Rebuild( getParent() );
   }
}

//-------------------------------------------------------------------

#define DISTANCE_THRESHHOLD (double)8.0
//-------------------------------------------------------------------
CLightEmitter * CLight::markSelected( ThredPoint pnt )
{
   // find selected ones first
   for( int sel = 0; sel < 2; sel++ )
   {
      for( int i = 0; i < states.GetSize(); i++ )
      {
         for( int j = 0; j < states[i].numEmitters(); j++ )
         {
            CLightEmitter & e = states[i].getEmitter( j );
            
            // check for selected on first pase ( sel == 0 pass )   
            if( !sel && !e.getSelect() )
               continue;
                  
         	double x1, y1, x2, y2;

         	// check which vars to use
         	if( pnt.X == 0 ) {
         		x1 = e.pos.y;
         		y1 = e.pos.z;
         		x2 = pnt.Y;
         		y2 = pnt.Z;
         	}
         	else if( pnt.Y == 0 ) {
         		x1 = e.pos.x;
         		y1 = e.pos.z;
         		x2 = pnt.X;
         		y2 = pnt.Z;
         	}
         	else {
         		x1 = e.pos.y;
         		y1 = e.pos.x;
         		x2 = pnt.Y;
         		y2 = pnt.X;
         	}

         	// calculate our deltas
         	double xdistance = x2 - x1;
         	double ydistance = y2 - y1;
            
            double distance = sqrt( ( xdistance * xdistance ) +
               ( ydistance * ydistance ) );
               
            if( distance < DISTANCE_THRESHHOLD )
            {
               e.selectable = true;
               return( &e );
            }
         }
      }
   }
   return( 0 );
}

//-------------------------------------------------------------------
void CLight::select( bool all )
{
   // go through and mark all emitters that are in as selectable
   for( int i = 0; i < states.GetSize(); i++ )
      for( int j = 0; j < states[i].numEmitters(); j++ )
      {
         CLightEmitter & e = states[i].getEmitter(j);
         e.movePos = e.pos;
         if( all )
            e.select( true );
         else
            if( e.selectable )
               e.select( true );
      }   
}

//-------------------------------------------------------------------
bool CLight::inBox( ThredBox & box )
{
   bool retVal = false;
   
   // go through and mark all emitters that are in as selectable
   for( int i = 0; i < states.GetSize(); i++ )
      for( int j = 0; j < states[i].numEmitters(); j++ )
      {
         CLightEmitter & e = states[i].getEmitter(j);
         
      	ThredPoint newOrig = ThredPoint( e.pos.x, e.pos.y, e.pos.z );

      	// make a new origin that is zero in the same
      	// axis as the box so when tested it works
      	if( box.mSmallest.X == 0 )
      		newOrig.X = 0;
      	if( box.mSmallest.Y == 0 )
      		newOrig.Y = 0;
      	if( box.mSmallest.Z == 0 )
      		newOrig.Z = 0;
            
         if( box.CheckPointIntersection( newOrig ) )
         {
            e.selectable = true;
            retVal = true;
         }
      }      

   return( retVal );
}

//-------------------------------------------------------------------

CLight & CLight::operator=( CLight & light )
{
   states.RemoveAll();
   
   // CArrays do not allow for much!
   id = light.id;
   name = light.name;
   animationDuration = light.animationDuration;
   flags = light.flags;
   m_active = light.m_active;
   group = light.group;
   
   // copy the state array
   for( int i = 0; i < light.states.GetSize(); i++ )
      states.Add( light.states[i] );
   
   return( *this );
}

//-------------------------------------------------------------------

// old define's for old serialize capability
#define CLIGHTTYPE_OLD              "CLight"
#define CLIGHTFLAGS_OLD             "lFlags"
#define CLIGHTNAME_OLD              "lName"
#define CLIGHTID_OLD                "lID"
#define CLIGHTGROUP_OLD             "lGroup"
#define CLIGHTSTATECOUNT_OLD        "lStateCount"
#define CLIGHTSTATEFLAGS_OLD        "lsFlags"
#define CLIGHTSTATEFALLOFFS_OLD     "lsFalloffs"
#define CLIGHTSTATEDURATION_OLD     "lsDuration"
#define CLIGHTSTATEPOSITION_OLD     "lsPosition"
#define CLIGHTSTATECOLOR_OLD        "lsColor"
#define CLIGHTSTATESPOTFACTOR_OLD   "lsSpotFactor"
#define CENDLIGHTTYPE_OLD           "End CLight"

// new define's
#define CLIGHTTYPE                  "CLight"
#define CLIGHTFLAGS                 "lFlags"
#define CLIGHTID                    "lID"
#define CLIGHTNAME                  "lName"
#define CLIGHTDURATION              "lDuration"
#define CLIGHTGROUP                 "lGroup"
#define CLIGHTSTATECOUNT            "lStateCount"
#define CLIGHTSTATEDURATION         "lsDuration"
#define CLIGHTSTATECOLOR            "lsColor"
#define CLIGHTEMITTERCOUNT          "lsEmitterCount"
#define CLIGHTEMITTERTYPE           "leType"
#define CLIGHTEMITTERPOS            "lePos"
#define CLIGHTEMITTERSPOTVECTOR     "leSpotVector"
#define CLIGHTEMITTERSPOTTHETA      "leSpotTheta"
#define CLIGHTEMITTERSPOTPHI        "leSpotPhi"
#define CLIGHTEMITTERFALLOFFTYPE    "leFalloffType"
#define CLIGHTEMITTERFALLOFFS       "leFalloffs"
#define CENDLIGHTTYPE               "End CLight"

void CLight::Serialize( CThredParser & Parser )
{
   CString str;
   
   // check for saving or not...
   if( Parser.IsStoring )
   {
      Parser.SetEnd( CENDLIGHTTYPE );
      
      // write the transaction type
      Parser.WriteString( CLIGHTTYPE );
      
      // write the light data
      Parser.WriteInt( CLIGHTFLAGS, flags );
      Parser.WriteInt( CLIGHTID, id );
      Parser.WriteString( CLIGHTNAME, name );
      Parser.WriteFloat( CLIGHTDURATION, animationDuration );
      Parser.WriteInt( CLIGHTGROUP, group );
      Parser.WriteInt( CLIGHTSTATECOUNT, states.GetSize() );
      
      // write the state info
      for( int i = 0; i < states.GetSize(); i++ )
      {
         CLightState & state = states[i];
         Parser.WriteFloat( CLIGHTSTATEDURATION, state.stateDuration );
         str.Format( "%f %f %f", state.color.red, state.color.green, state.color.blue );
         Parser.WriteString( CLIGHTSTATECOLOR, str );
         Parser.WriteInt( CLIGHTEMITTERCOUNT, state.numEmitters() );
         
         // write out the emitters
         for( int k = 0; k < state.numEmitters(); k++ )
         {
            CLightEmitter & emitter = state.getEmitter(k);
            Parser.WriteInt( CLIGHTEMITTERTYPE, emitter.lightType );
   
            // convert the light to integer coords...
            Point3I pos;
            pos.x = emitter.pos.x;
            pos.y = emitter.pos.y;
            pos.z = emitter.pos.z;
            str.Format( "%d %d %d", pos.x, pos.y, pos.z );
            Parser.WriteString( CLIGHTEMITTERPOS, str );
            str.Format( "%f %f %f", emitter.spotVector.x, emitter.spotVector.y, emitter.spotVector.z );
            Parser.WriteString( CLIGHTEMITTERSPOTVECTOR, str );
            Parser.WriteFloat( CLIGHTEMITTERSPOTTHETA, emitter.spotTheta );
            Parser.WriteFloat( CLIGHTEMITTERSPOTPHI, emitter.spotPhi );
            Parser.WriteInt( CLIGHTEMITTERFALLOFFTYPE, emitter.falloff );
            str.Format( "%f %f %f", emitter.d1, emitter.d2, emitter.d3 );
            Parser.WriteString( CLIGHTEMITTERFALLOFFS, str );
         }         
      }
      
		// we want to write our our transaction type
		Parser.WriteString( CENDLIGHTTYPE );
   }
   else
   {
      // check for the version
      if( theApp.m_fileVersion <= makeVersion( 1, 80 ) )
      {
         Parser.SetEnd( CENDLIGHTTYPE_OLD );
         
         // convert this to the new light format...
         int         IntVal;
         int         lightFlags;
         int         stateFlags;
         CString     String;
      
         // get the key
         if( !Parser.GetKey( CLIGHTTYPE_OLD ) )
            return;
         
         // get the flags
         Parser.GetKeyInt( CLIGHTFLAGS_OLD, IntVal );
            lightFlags = IntVal;
      
         // get the group
         Parser.GetKeyInt( CLIGHTGROUP_OLD, IntVal );
            group = IntVal;
            
         // get name
         if( lightFlags & LIGHT_HASNAME )
            Parser.GetKeyString( CLIGHTNAME_OLD, name );

         // convert the flags to the new ones.. 
         flags = 0;
         if( lightFlags & LIGHT_AUTOSTART )
            flags |= AUTOSTART;
         if( lightFlags & LIGHT_LOOPTOEND )
            flags |= LOOPTOEND;
         if( lightFlags & LIGHT_RANDOMFLICKER )
            flags |= RANDOMFLICKER;
         if( lightFlags & LIGHT_MANAGEBYMISSIONEDITOR )
            flags |= MANAGEBYMISSIONEDITOR;
         
         // read the id
         Parser.GetKeyInt( CLIGHTID_OLD, IntVal );
         id = IntVal;
        
         // read in the state flags
         Parser.GetKeyInt( CLIGHTSTATEFLAGS_OLD, IntVal );
         stateFlags = IntVal;

         // the default info's
         CLightEmitter                 defEmitter;
         CLightState                   defState;   

         // grab the default info...
         if( stateFlags & CLightState::STATE_LINEAR_FALLOFF )
         {
            Parser.GetKeyString( CLIGHTSTATEFALLOFFS_OLD, String );
            defEmitter.falloff = CLightEmitter::Linear;
            sscanf( LPCTSTR( String ), "%f %f", &defEmitter.d1,
               &defEmitter.d2 );            
         }
      
         if( stateFlags & CLightState::STATE_DISTANCE_FALLOFF )
         {
            Parser.GetKeyString( CLIGHTSTATEFALLOFFS_OLD, String );
            defEmitter.falloff = CLightEmitter::Distance;
            sscanf( LPCTSTR( String ), "%f %f %f", &defEmitter.d1,
               &defEmitter.d2, &defEmitter.d3 );
         }

         if( stateFlags & CLightState::STATE_HASDURATION )
         {
            Parser.GetKeyString( CLIGHTSTATEDURATION_OLD, String );
            sscanf( LPCTSTR( String ), "%f", &defState.stateDuration );
         }
      
         if( stateFlags & CLightState::STATE_HASPOSITION )
         {
            Point3I pnt;
            Parser.GetKeyString( CLIGHTSTATEPOSITION_OLD, String );
            sscanf( LPCTSTR( String ), "%d %d %d", &pnt.x, &pnt.y, &pnt.z );
            defEmitter.pos.x = pnt.x;
            defEmitter.pos.y = pnt.y;
            defEmitter.pos.z = pnt.z;
         }
      
         if( stateFlags & CLightState::STATE_HASCOLOR )
         {
            Parser.GetKeyString( CLIGHTSTATECOLOR_OLD, String );
            sscanf( LPCTSTR( String ), "%f %f %f", &defState.color.red,
               &defState.color.green, &defState.color.blue );
         }
   
         if( stateFlags & CLightState::STATE_HASSPOTFACTOR )
         {
            Parser.GetKeyString( CLIGHTSTATESPOTFACTOR_OLD, String );
            sscanf( LPCTSTR( String ), "%f %f %f %f %f", 
               &defEmitter.spotVector.x, &defEmitter.spotVector.y,
               &defEmitter.spotVector.z, &defEmitter.spotTheta,
               &defEmitter.spotPhi );
         }
      
         int NumStates = 0;
      
         // get number of states
         Parser.GetKeyInt( CLIGHTSTATECOUNT_OLD, NumStates );

         // have to have at least 1 state
         if( !NumStates )
            return;
      
         // go through states
         for( int State = 0; State < NumStates; State++ )
         {
            CLightState    ls;
            CLightEmitter  emitter;

            // fill in the default data that may not be read
            ls = defState;
            emitter = defEmitter;
            
            // get the flags
            Parser.GetKeyInt( CLIGHTSTATEFLAGS_OLD, IntVal );
            stateFlags = IntVal;
                     
            // get falloffs
            if( stateFlags & CLightState::STATE_LINEAR_FALLOFF )
            {
               Parser.GetKeyString( CLIGHTSTATEFALLOFFS_OLD, String );
               sscanf( LPCTSTR( String ), "%f %f", &emitter.d1,
                  &emitter.d2 );
            }
            else if( stateFlags & CLightState::STATE_DISTANCE_FALLOFF )
            {
               Parser.GetKeyString( CLIGHTSTATEFALLOFFS_OLD, String );
               sscanf( LPCTSTR( String ), "%f %f %f", 
                  &emitter.d1, &emitter.d2, &emitter.d3 );
            }

            // set the emitters falloff value
            if( stateFlags & CLightState::STATE_DISTANCE_FALLOFF )
               emitter.falloff = CLightEmitter::Distance;
            else
               emitter.falloff = CLightEmitter::Linear;
            
            // get duration
            if( stateFlags & CLightState::STATE_HASDURATION )
            {
               float tmp1;
               Parser.GetKeyString( CLIGHTSTATEDURATION_OLD, String );
               sscanf( LPCTSTR( String ), "%f", &tmp1 );
               ls.stateDuration = tmp1;
            }
            
            // get position
            if( stateFlags & CLightState::STATE_HASPOSITION )
            {
               int tmp1, tmp2, tmp3;
               Parser.GetKeyString( CLIGHTSTATEPOSITION_OLD, String );
               sscanf( LPCTSTR( String ), "%d %d %d", &tmp1, &tmp2, &tmp3 );
               emitter.pos.x = float( tmp1 );
               emitter.pos.y = float( tmp2 );
               emitter.pos.z = float( tmp3 );
            }

            // get color
            if( stateFlags & CLightState::STATE_HASCOLOR )
            {
               float tmp1, tmp2, tmp3;
               Parser.GetKeyString( CLIGHTSTATECOLOR_OLD, String );
               sscanf( LPCTSTR( String ), "%f %f %f", &tmp1, &tmp2, &tmp3 );
               ls.color.red = float( tmp1 );
               ls.color.green = float( tmp2 );
               ls.color.blue = float( tmp3 );
            }
            
            // get spotfactors
            if( stateFlags & CLightState::STATE_HASSPOTFACTOR )
            {
               float tmp1, tmp2, tmp3, tmp4, tmp5;
               Parser.GetKeyString( CLIGHTSTATESPOTFACTOR_OLD, String );
               sscanf( LPCTSTR( String ), "%f %f %f %f %f", &tmp1, &tmp2,
                  &tmp3, &tmp4, &tmp5 );
               emitter.spotVector.x = float( tmp1 );
               emitter.spotVector.y = float( tmp2 );
               emitter.spotVector.z = float( tmp3 );
               emitter.spotTheta = float( tmp4 );
               emitter.spotPhi = float( tmp5 );
            }
                     
            // set the emitter's type
            if( stateFlags & CLightState::STATE_HASSPOTFACTOR )
               emitter.lightType = CLightEmitter::SpotLight;
            else
               emitter.lightType = CLightEmitter::PointLight;
               
            // add the state
            ls.addEmitter( emitter );
            addState( ls );
         }
      
   		Parser.GotoEnd();
      }
      else
      {
         int intVal;
         int numStates=0;
         
      	Parser.SetEnd( CENDLIGHTTYPE );
      
         // get the key
         if( !Parser.GetKey( CLIGHTTYPE ) )
            return;
            
         // get the light data
         Parser.GetKeyInt( CLIGHTFLAGS, intVal ); 
         flags = intVal;
         Parser.GetKeyInt( CLIGHTID, intVal );
         id = intVal;
         Parser.GetKeyString( CLIGHTNAME, name );
         Parser.GetKeyFloat( CLIGHTDURATION, animationDuration );
         Parser.GetKeyInt( CLIGHTGROUP, intVal );
         group = intVal;
         Parser.GetKeyInt( CLIGHTSTATECOUNT, numStates );
         
         // get the state data
         for(; numStates; numStates-- )
         {
            CLightState state;
            int numEmitters=0;
            
            Parser.GetKeyFloat( CLIGHTSTATEDURATION, state.stateDuration );
            Parser.GetKeyString( CLIGHTSTATECOLOR, str );
            sscanf( LPCTSTR( str ), "%f %f %f", &state.color.red,
               &state.color.green, &state.color.blue );
            Parser.GetKeyInt( CLIGHTEMITTERCOUNT, numEmitters );
            
            // get the emitter data
            for(; numEmitters; numEmitters-- )
            {
               Point3I pnt;
               CLightEmitter emitter;
               
               Parser.GetKeyInt( CLIGHTEMITTERTYPE, intVal );
               emitter.lightType = ( CLightEmitter::LightType )intVal;
               Parser.GetKeyString( CLIGHTEMITTERPOS, str );
               sscanf( LPCTSTR( str ), "%d %d %d", &pnt.x,
                  &pnt.y, &pnt.z );
               emitter.pos.x = pnt.x;
               emitter.pos.y = pnt.y;
               emitter.pos.z = pnt.z;
               Parser.GetKeyString( CLIGHTEMITTERSPOTVECTOR, str );
               sscanf( LPCTSTR( str ), "%f %f %f", &emitter.spotVector.x,
                  &emitter.spotVector.y, &emitter.spotVector.z );
               Parser.GetKeyFloat( CLIGHTEMITTERSPOTTHETA, emitter.spotTheta );               
               Parser.GetKeyFloat( CLIGHTEMITTERSPOTPHI, emitter.spotPhi );
               Parser.GetKeyInt( CLIGHTEMITTERFALLOFFTYPE, intVal );
               emitter.falloff = ( CLightEmitter::Falloff )intVal;
               Parser.GetKeyString( CLIGHTEMITTERFALLOFFS, str );
               sscanf( LPCTSTR( str ), "%f %f %f", &emitter.d1,
                  &emitter.d2, &emitter.d3 );
                  
               // add this emitter to the state
               state.addEmitter( emitter );
            }
            
            // add this state to the light
            addState( state );            
         }
         
   		Parser.GotoEnd();
      }
   }
}

//-------------------------------------------------------------------
// CLightState

CLightState::CLightState() :
   stateDuration( 0.f ),
   color( 0.f, 0.f, 0.f )
{
}

//------------------------------------------------------------------

bool CLightState::canPaste()
{
   return( dynamic_cast< CLightEmitter * >( copyObj ) ? true : false );
}

//------------------------------------------------------------------

void CLightState::remove()
{
   CLight * parent = ( CLight * )getParent();
   
   if( !parent || ( getIndex() == -1 ) )
      return;
      
   // remove
   parent->states.RemoveAt( getIndex() );
   
   // rebuild
   tree->Rebuild( parent );
}

//------------------------------------------------------------------

void CLightState::copy()
{
   // delete the current one
   delete copyObj;
   copyObj = NULL;
   
   // creaet a new one and set it
   CLightState * pState = new CLightState;
   
   *pState = *this;
   copyObj = pState;
}

//------------------------------------------------------------------

void CLightState::paste()
{
   // make sure cool
   CLightEmitter * pEmitter = dynamic_cast< CLightEmitter * >( copyObj );
   if( pEmitter )
   {
      // copy and add
      CLightEmitter emitter;
      emitter = *pEmitter;
      addEmitter( emitter );

      if( tree )
         tree->Rebuild( getParent() );
   }
}

//------------------------------------------------------------------

void CLightState::onContextMenu( CMenu & menu )
{
   // add the commands
   menu.AppendMenu( MF_SEPARATOR );

   UINT menuFlags = MF_STRING;
   
   // check for a lightemitter to past
   if( !canPaste() )
      menuFlags |= MF_GRAYED;
   menu.AppendMenu( menuFlags, PASTE_EMITTER, "Paste Emitter" );
   menu.AppendMenu( MF_STRING, ADD_EMITTER, "Add Emitter" );
}

//------------------------------------------------------------------

void CLightState::onContextMenuSelect( int id )
{
   switch( id )
   {
      case PASTE_EMITTER:
         paste();
         break;
      
      case ADD_EMITTER:
      {
         CLightEmitter lightEmitter;
         addEmitter( lightEmitter );
         tree->Rebuild( this );
         break;
      }
         
      default:
         break;
   }
}

//-------------------------------------------------------------------

void CLightState::onSelect()
{
   pDoc->ResetSelectedLights();   
   
   for( int i = 0; i < numEmitters(); i++ )
   {
      CLightEmitter & e = getEmitter(i);
      e.select( true );
   }   

   pDoc->UpdateAllViews(NULL);
}

//-------------------------------------------------------------------

bool CLightState::onDragOver( CItemBase * source )
{
   CLightState * state = dynamic_cast< CLightState * >( source );
   CLightEmitter * emitter = dynamic_cast< CLightEmitter * >( source );
   
   // check if a state is being drug over ( need to reorder on drop )
   if( state && state != this && state->getParent() == getParent() )
      return( true );
   
   // check if an emitter is being put here
   if( emitter && ( emitter->getParent() != this ) )
      return( true );
   
   return( false );
}

//-------------------------------------------------------------------

void CLightState::getName( CString & str )
{  
   // get the parent and figure out which one this is...
   CLight * pLight = dynamic_cast< CLight *>(_parent);
   AssertFatal( pLight, "Light!" );
   for( int i = 0; i < pLight->states.GetSize(); i++ )
   {
      if( this == &pLight->states[i] )
      {
         str.Format( "State %d", i );
         return;
      }
   }
   
   str = "";
}

//-------------------------------------------------------------------

int CLightState::getIndex()
{
   if( !getParent() )
      return( -1 );
      
   CLight * pLight = dynamic_cast<CLight *>(getParent());
   
   for( int i = 0; i < pLight->states.GetSize(); i++ )
      if( this == &pLight->states[i] )
         return( i );
         
   return( -1 );
}

//------------------------------------------------------------------

void CLightState::onDrop( CItemBase * source )
{
   // get the type.. doing stupid way be ok..
   CLightState * state = dynamic_cast< CLightState * >( source );
   CLightEmitter * emitter = dynamic_cast<CLightEmitter*>(source);   
   
   if( state )
   {                                
      CLightStateArray * pArray = &( ( CLight * )getParent() )->states;

      AssertFatal( pArray, "No state array." );
      
      CLightState tmp;
      tmp = (*pArray)[source->getIndex()];
      
      // position the lights - depends on where they are
      if( source->getIndex() > getIndex() )
      {
         pArray->RemoveAt( source->getIndex() );
         pArray->InsertAt( getIndex() + 1, tmp );
      }
      else
      {
         pArray->InsertAt( getIndex() + 1, tmp );
         pArray->RemoveAt( source->getIndex() );
      }
      
      // just need to rebuild this guy...
      tree->Rebuild( getParent() );
      
      return;
   }
   
   if( emitter )
   {
      CLightState * state = ( CLightState * )emitter->getParent();
      
      // get a copy and remove from the source
      CLightEmitter tmp = state->emitters[ emitter->getIndex() ];
      state->emitters.RemoveAt( emitter->getIndex() );
      
      // add in
      addEmitter( tmp );
      
      // rebuild...
      tree->Rebuild( getParent() );
      tree->Rebuild( emitter->getParent() );
           
      return;
   }
}

//-------------------------------------------------------------------

CLightState & CLightState::operator=( CLightState & state )
{
   emitters.RemoveAll();
   
   stateDuration = state.stateDuration;
   color = state.color;

   // copy the emitters array
   for( int i = 0; i < state.emitters.GetSize(); i++ )
      emitters.Add( state.emitters[i] );
   
   return( *this );
}

// ------------------------------------------------------------------------------

CPropertyPage * CLightState::getPropPage()
{
   if( !lightStateProp )
      lightStateProp = new CLightStateProp;
   return( lightStateProp );
}

//-------------------------------------------------------------------

bool CLightState::onInspect( bool read )
{
   CLightStateProp * p = lightStateProp;
   
   if( read )
   {
      p->m_color = color;
      p->m_duration = stateDuration;
   }
   else
   {
      color = p->m_color;
      stateDuration = p->m_duration;
   }
   return( true );
}

//-------------------------------------------------------------------
// CLightEmitter

CLightEmitter::CLightEmitter() :
   lightType( PointLight ),
   pos( 0.f, 0.f, 0.f ),
   spotVector( 0.f, 0.f, 0.f ),
   spotTheta( 0.f ),
   spotPhi( 0.f ),
   falloff( Linear ),
   d1( 0.f ),
   d2( 0.f ),
   d3( 0.f ),
   _selected( false ),
   rotValid(false),
   selectable( false )
{
}

//-------------------------------------------------------------------

void CLightEmitter::copy()
{
   delete copyObj;
   copyObj = NULL;
   
   CLightEmitter * pEmitter = new CLightEmitter;
   *pEmitter = *this;
   
   copyObj = pEmitter;
}

//-------------------------------------------------------------------

int CLightEmitter::getIndex()
{
   if( !getParent() )
      return( -1 );
      
   CLightState * pLightState = dynamic_cast<CLightState*>(getParent());

   for( int i = 0; i < pLightState->emitters.GetSize(); i++ )
      if( this == &pLightState->emitters[i] )
         return( i );
         
   return( -1 );
}

//------------------------------------------------------------------

void CLightEmitter::remove()
{
   CLightState * parent = ( CLightState * )getParent();
   
   if( !parent || ( getIndex() == -1 ) )
      return;
      
   // remove
   parent->emitters.RemoveAt( getIndex() );
   
   // rebuild
   tree->Rebuild( parent );
}

//------------------------------------------------------------------

void CLightEmitter::onSelect()
{
   pDoc->ResetSelectedLights();   
   _selected = true;
   pDoc->UpdateAllViews(NULL);
}

//------------------------------------------------------------------
   
bool CLightEmitter::onInspect( bool read )
{

   CLightEmitterProp * p = dynamic_cast<CLightEmitterProp*>(getPropPage());
   if( !p )
      return( false );
         
   if( read )
   {
      // set the data...
      p->m_posString.Format( "%d, %d, %d", ( int )pos.x, ( int )pos.y, ( int )pos.z );
      p->m_distance1 = d1;
      p->m_distance2 = d2;
      p->m_distance3 = d3;
      p->m_angleString.Format( "%f, %f", spotTheta, spotPhi );
      p->m_directionString.Format( "%f, %f, %f", spotVector.x, spotVector.y,
         spotVector.z );
      
      // set our check-box values   
      p->m_lightType = lightType;
      p->m_falloff = falloff;
   }
   else
   {
      Point3I tmp;
      
      // grab the position...
      if( sscanf( LPCTSTR( p->m_posString ), "%d, %d, %d", &tmp.x, &tmp.y, &tmp.z ) == 3 )
      {
         pos.x = tmp.x;
         pos.y = tmp.y;
         pos.z = tmp.z;
      }
      else
         return( false );

      // grab the distances...
      d1 = p->m_distance1;
      d2 = p->m_distance2;
      d3 = p->m_distance3;
      
      // grab the spot info(s)
      float f1, f2, f3;
      if( sscanf( LPCTSTR( p->m_angleString ), "%f, %f", &f1, &f2 ) == 2 )
      {
         spotTheta = f1;
         spotPhi = f2;
      }
      else
         return( false );
      
      if( sscanf( LPCTSTR( p->m_directionString ), "%f, %f, %f", &f1, &f2, &f3 ) == 3 )
      {
         spotVector.x = f1;
         spotVector.y = f2;
         spotVector.z = f3;
      }
      else
         return( false );
         
      // grab the checkbox ones...
      lightType = ( CLightEmitter::LightType )p->m_lightType;
      falloff = ( CLightEmitter::Falloff )p->m_falloff;
      
      // need to redraw some stuff...
      pDoc->UpdateAllViews(NULL);
   }
   return( true );
}

//-------------------------------------------------------------------
bool CLightEmitter::onDragOver( CItemBase * source )
{
   CLightEmitter * emitter = dynamic_cast< CLightEmitter * >( source );
   
   if( emitter && ( emitter != this ) && ( emitter->getParent() == getParent() ) )
      return( true );
      
   return( false );
}

//------------------------------------------------------------------

void CLightEmitter::onDrop( CItemBase * source )
{
   // get the type.. doing stupid way be ok..
   CLightEmitter * emitter = dynamic_cast<CLightEmitter*>(source);   
   if( !emitter )
      return;
   
   CLightEmitterArray * pArray = &( ( CLightState * )getParent() )->emitters;

   AssertFatal( pArray, "No emitter array." );
      
   CLightEmitter tmp;
   tmp = (*pArray)[source->getIndex()];
   
   // position the lights - depends on where they are
   if( source->getIndex() > getIndex() )
   {
      pArray->RemoveAt( source->getIndex() );
      pArray->InsertAt( getIndex() + 1, tmp );
   }
   else
   {
      pArray->InsertAt( getIndex() + 1, tmp );
      pArray->RemoveAt( source->getIndex() );
   }
   
   // need to rebuild...
   tree->Rebuild( getParent() );
}

//-------------------------------------------------------------------

void CLightEmitter::getName( CString & str )
{
   // get the parent and figure out which one this is...
   CLightState * pLightState = dynamic_cast< CLightState *>(_parent);
   
   for( int i = 0; i < pLightState->emitters.GetSize(); i++ )
   {
      if( this == &pLightState->emitters[i] )
      {
         str.Format( "Emitter %d", i );
         return;
      }
   }
   
   str = "";
}


// ------------------------------------------------------------------------------

CPropertyPage * CLightEmitter::getPropPage()
{
   if( !lightEmitterProp )
      lightEmitterProp = new CLightEmitterProp;
   return( lightEmitterProp );
}

//// ------------------------------------------------------------------------------
//// constructor
//CLight::CLight( bool fCreateFirstState ) :
//   group( -1 ),
//   flags( 0 ),
//   id( -1 ),
//   animationDuration( 0.f )
//{
//   // check if we have a first state on creation
//   if( fCreateFirstState )
//   {   
//      CLightState LightState;
//      AddState( LightState );
//   }
//      
//   // these ptrs are used in lightset manipulation
//   pNext = NULL;
//   pPrevious = NULL;
//}
//
//// ------------------------------------------------------------------------------
//// destructor
//CLight::~CLight()
//{
//   // remove all the state information ( destroy the list )
//   states.RemoveAll();
//}
//
//// ------------------------------------------------------------------------------
//// display the light shape
//void CLight::RenderLight( CRenderCamera * Camera, TSRenderContext * rc, int ViewType )
//{
//	Point2I	start, end;
//	Point3F	in_point;
//   
//   // go through all the states
//   for( int i = 0; i < states.GetCount(); i++ )
//   {
//      CLightState & state = states.GetAt( states.FindIndex( i ) );
//      ThredPoint pos = ThredPoint( state.position.x, state.position.y, state.position.z );
//            
//      COLORREF col = ( state.selected ? 251 : 252 );
//      
//      switch( ViewType )
//      {
//         case ID_VIEW_SOLIDRENDER:
//         case ID_VIEW_TEXTUREVIEW:
//         {
//            // check which version of the light to draw
//            const float offset = 10.0;
//            Camera->DrawDiamond( pos, offset, col, rc );
//            
//            break;
//         }
//            
//         default:
//         {
//            CPoint point, scalePoint;
//
//            // translate the point then draw
//            Camera->TranslateToScreen( point, pos, rc );
//            
//            ThredPoint fakePos = ThredPoint( pos.X + 8, pos.Y + 8, pos.Z + 8 );
//            
//            // scale it
//            Camera->TranslateToScreen( scalePoint, fakePos, rc );
//
//      	   // get a size		
//      	   int xsize = scalePoint.x - point.x;
//
//      	   // DPW - Needed for TS compatibility
//      	   start.x = point.x - xsize;
//      	   start.y = point.y - xsize;
//      	   end.x = point.x + xsize;
//      	   end.y = point.y + xsize;
//      	   Camera->DrawLine( &start, &end, 3, col, rc );
//
//      	   // draw another one
//      	   start.x = point.x - xsize;
//      	   start.y = point.y + xsize;
//      	   end.x = point.x + xsize;
//      	   end.y = point.y - xsize;
//      	   Camera->DrawLine( &start, &end, 3, col, rc );
//            
//            break;
//         }
//      }
//   }
//}
//
//// ------------------------------------------------------------------------------
//
//// export the light info to a file
//void CLight::Export( FILE * OutFile )
//{
////   FILE * fp = OutFile;
////   
////   // print out LIGHT
////   fprintf( fp, "Light\n" );
////   
////   // set the light stuff
////   if( mFlags & LIGHT_AUTOSTART )
////      fprintf( fp, "\tautoStart\n" );
////   if( mFlags & LIGHT_LOOPTOEND )
////      fprintf( fp, "\tloopToEnd\n" );
////   if( mFlags & LIGHT_RANDOMFLICKER )
////      fprintf( fp, "\trandomFlicker\n" );
////      
////   // remove any none-alpha type chars from the string prior to export
////   if( ( mFlags & LIGHT_HASNAME ) && ( strlen( mName ) ) )
////   {
////      char * convert = new char[ strlen( mName ) + 1 ];
////      strcpy( convert, mName );
////      char * pos = convert;
////      while( *pos )
////      {
////         // just convert to an underscore
////         if( !isalnum( *pos ) )
////            *pos = '_';
////         pos++;
////      } 
////      fprintf( fp, "\tname %s\n", convert );
////      delete [] convert;
////   }
////
////   // check for multi states, only write out if > 1
////   if( mStates.GetCount() > 1 )
////   {
////      if( mFlags & LIGHT_MANAGEBYMISSIONEDITOR )
////         fprintf( fp, "\tmanageByMissionEditor\n" );
////   }
////   
////   if( mID != -1 )
////         fprintf( fp, "\tid %d\n", mID );
////      
////   // do the default state
////   if( mDefaultState.mFlags & STATE_HASPOSITION )
////      fprintf( fp, "\tpos %d %d %d\n", int( mDefaultState.mPosition.X ),
////         int( mDefaultState.mPosition.Y ), int( mDefaultState.mPosition.Z ) );
////   if( mDefaultState.mFlags & STATE_HASCOLOR )
////      fprintf( fp, "\tcolor %f %f %f\n", float( mDefaultState.mColor.red ),
////         float( mDefaultState.mColor.green ), float( mDefaultState.mColor.blue ) );
////   if( mDefaultState.mFlags & STATE_LINEAR_FALLOFF )
////      fprintf( fp, "\tLinear %d %d\n", mDefaultState.mFalloff1, mDefaultState.mFalloff2 );
////   if( mDefaultState.mFlags & STATE_DISTANCE_FALLOFF )
////      fprintf( fp, "\tDistance %d %d %d\n", mDefaultState.mFalloff1, 
////         mDefaultState.mFalloff2, mDefaultState.mFalloff3 );
////            
////   // go through all the states now
////   for( int State = 0; State < mStates.GetCount(); State++ )
////   {
////      // get this state
////      CLightState & LightState = 
////         mStates.GetAt( mStates.FindIndex( State ) );
////      
////      // write state
////      fprintf( fp, "\tState\n" );
////      
////      // write duration
////      if( LightState.mFlags & STATE_HASDURATION )
////         fprintf( fp, "\t\tduration %f\n", float( LightState.mDuration ) );
////      else if( mStates.GetCount() > 1 )
////         fprintf( fp, "\t\tduration %f\n", 0.f );
////         
////      // write position
////      if( LightState.mFlags & STATE_HASPOSITION )
////         fprintf( fp, "\t\tpos %d %d %d\n", int( LightState.mPosition.X ),
////            int( LightState.mPosition.Y ), int( LightState.mPosition.Z ) );
////            
////      // write color
////      if( LightState.mFlags & STATE_HASCOLOR )
////         fprintf( fp, "\t\tcolor %f %f %f\n", float( LightState.mColor.red ),
////            float( LightState.mColor.green ), float( LightState.mColor.blue ) );
////      
////      // write linear
////      if( LightState.mFlags & STATE_LINEAR_FALLOFF )
////         fprintf( fp, "\t\tLinear %d %d\n", LightState.mFalloff1, LightState.mFalloff2 );
////         
////      // write distance
////      if( LightState.mFlags & STATE_DISTANCE_FALLOFF )
////         fprintf( fp, "\t\tDistance %d %d %d\n", LightState.mFalloff1, 
////            LightState.mFalloff2, LightState.mFalloff3 );
////         
////      // write spotfactor
////      if( LightState.mFlags & STATE_HASSPOTFACTOR )
////         fprintf( fp, "\t\tspotFactors %f %f %f %f %f\n",
////            float( LightState.mSpotVector.X ), float( LightState.mSpotVector.Y ),
////            float( LightState.mSpotVector.Z ), float( LightState.mSpotTheta ),
////            float( LightState.mSpotPhi ) );
////   }
////   
////   return;
//}
//
//// ------------------------------------------------------------------------------
//
//#define CLIGHTTYPE            "CLight"
//#define CLIGHTFLAGS           "lFlags"
//#define CLIGHTNAME            "lName"
//#define CLIGHTID              "lID"
//#define CLIGHTGROUP           "lGroup"
//#define CLIGHTSTATECOUNT      "lStateCount"
//#define CLIGHTSTATEFLAGS      "lsFlags"
//#define CLIGHTSTATEFALLOFFS   "lsFalloffs"
//#define CLIGHTSTATEDURATION   "lsDuration"
//#define CLIGHTSTATEPOSITION   "lsPosition"
//#define CLIGHTSTATECOLOR      "lsColor"
//#define CLIGHTSTATESPOTFACTOR "lsSpotFactor"
//#define CENDLIGHTTYPE         "End CLight"
//
//void CLight::Serialize( CThredParser& Parser )
//{
////	Parser.SetEnd( CENDLIGHTTYPE );
////
////   CString String;
////   
////	if(Parser.IsStoring ) 
////   {
////		// we want to write our our transaction type
////		Parser.WriteString( CLIGHTTYPE );
////
////      // write out the light info
////		Parser.WriteInt( CLIGHTFLAGS, mFlags );
////      Parser.WriteInt( CLIGHTGROUP, mGroup );
////      if( mFlags & LIGHT_HASNAME )
////         Parser.WriteString( CLIGHTNAME, mName );
////       
////      // write the id
////      Parser.WriteInt( CLIGHTID, mID );
////       
////      // do the default state ( just like a state that is always here )
////      Parser.WriteInt( CLIGHTSTATEFLAGS, mDefaultState.mFlags );
////      
////      // figure out the falloffs
////      if( mDefaultState.mFlags & STATE_LINEAR_FALLOFF )
////      {
////         String.Format( "%d %d", mDefaultState.mFalloff1, mDefaultState.mFalloff2 );
////         Parser.WriteString( CLIGHTSTATEFALLOFFS, String );
////      }
////      if( mDefaultState.mFlags & STATE_DISTANCE_FALLOFF )
////      {
////         String.Format( "%d %d %d", mDefaultState.mFalloff1, 
////            mDefaultState.mFalloff2, mDefaultState.mFalloff3 );
////         Parser.WriteString( CLIGHTSTATEFALLOFFS, String );
////      }
////      
////      // do duration, position, color, ...
////      if( mDefaultState.mFlags & STATE_HASDURATION )
////      {
////         String.Format( "%f", float( mDefaultState.mDuration ) );
////         Parser.WriteString( CLIGHTSTATEDURATION, String );
////      }
////      if( mDefaultState.mFlags & STATE_HASPOSITION )
////      {
////         String.Format( "%d %d %d", int( mDefaultState.mPosition.X ),
////            int( mDefaultState.mPosition.Y ), 
////            int( mDefaultState.mPosition.Z ) );
////         Parser.WriteString( CLIGHTSTATEPOSITION, String );
////      }
////      if( mDefaultState.mFlags & STATE_HASCOLOR )
////      {
////         String.Format( "%f %f %f", float( mDefaultState.mColor.red ),
////            float( mDefaultState.mColor.green ), 
////            float( mDefaultState.mColor.blue ) );
////         Parser.WriteString( CLIGHTSTATECOLOR, String );
////      }
////      if( mDefaultState.mFlags & STATE_HASSPOTFACTOR )
////      {
////         String.Format( "%f %f %f %f %f", float( mDefaultState.mSpotVector.X ),
////            float( mDefaultState.mSpotVector.Y ), 
////            float( mDefaultState.mSpotVector.Z ),
////            float( mDefaultState.mSpotTheta ), 
////            float( mDefaultState.mSpotPhi ) );
////         Parser.WriteString( CLIGHTSTATESPOTFACTOR, String );
////      }
////   
////      // write out the number of states to walk through
////      Parser.WriteInt( CLIGHTSTATECOUNT, mStates.GetCount() );
////      
////      // go through the states for this light  
////      for( int State = 0; State < mStates.GetCount(); State++ )
////      {
////         // get this state
////         CLightState & LS = mStates.GetAt( mStates.FindIndex( State ) );
////
////         // write out the state info
////         Parser.WriteInt( CLIGHTSTATEFLAGS, LS.mFlags );
////         
////         // figure out the falloffs
////         if( LS.mFlags & STATE_LINEAR_FALLOFF )
////         {
////            String.Format( "%d %d", LS.mFalloff1, LS.mFalloff2 );
////            Parser.WriteString( CLIGHTSTATEFALLOFFS, String );
////         }
////         else if( LS.mFlags & STATE_DISTANCE_FALLOFF )
////         {
////            String.Format( "%d %d %d", LS.mFalloff1, LS.mFalloff2, 
////               LS.mFalloff3 );
////            Parser.WriteString( CLIGHTSTATEFALLOFFS, String );
////         }
////         
////         // do duration, position, color, ...
////         if( LS.mFlags & STATE_HASDURATION )
////         {
////            String.Format( "%f", float( LS.mDuration ) );
////            Parser.WriteString( CLIGHTSTATEDURATION, String );
////         }
////         if( LS.mFlags & STATE_HASPOSITION )
////         {
////            String.Format( "%d %d %d", int( LS.mPosition.X ),
////               int( LS.mPosition.Y ), int( LS.mPosition.Z ) );
////            Parser.WriteString( CLIGHTSTATEPOSITION, String );
////         }
////         if( LS.mFlags & STATE_HASCOLOR )
////         {
////            String.Format( "%f %f %f", float( LS.mColor.red ),
////               float( LS.mColor.green ), float( LS.mColor.blue ) );
////            Parser.WriteString( CLIGHTSTATECOLOR, String );
////         }
////         if( LS.mFlags & STATE_HASSPOTFACTOR )
////         {
////            String.Format( "%f %f %f %f %f", float( LS.mSpotVector.X ),
////               float( LS.mSpotVector.Y ), float( LS.mSpotVector.Z ),
////               float( LS.mSpotTheta ), float( LS.mSpotPhi ) );
////            Parser.WriteString( CLIGHTSTATESPOTFACTOR, String );
////         }
////      }
////         
////		// we want to write our our transaction type
////		Parser.WriteString( CENDLIGHTTYPE );
////	}
////	else 
////   {
////      int   IntVal;
////      
////      // get the key
////      if( !Parser.GetKey( CLIGHTTYPE ) )
////         return;
////         
////      // get the flags
////      Parser.GetKeyInt( CLIGHTFLAGS, IntVal );
////      mFlags = IntVal;
////      
////      // get the group
////      Parser.GetKeyInt( CLIGHTGROUP, IntVal );
////      mGroup = IntVal;
////            
////      // get name
////      if( mFlags & LIGHT_HASNAME )
////         Parser.GetKeyString( CLIGHTNAME, mName );
//// 
////      // read the id
////      Parser.GetKeyInt( CLIGHTID, IntVal );
////      mID = IntVal;
////        
////      // read in the state flags
////      Parser.GetKeyInt( CLIGHTSTATEFLAGS, IntVal );
////      mDefaultState.mFlags = IntVal;
////
////      // have to load in for the default state like the ones below
////      // ( could merge the two but will be screwy )
////      if( mDefaultState.mFlags & STATE_LINEAR_FALLOFF )
////      {
////         Parser.GetKeyString( CLIGHTSTATEFALLOFFS, String );
////         sscanf( LPCTSTR( String ), "%d %d", &mDefaultState.mFalloff1,
////            &mDefaultState.mFalloff2 );
////      }
////      if( mDefaultState.mFlags & STATE_DISTANCE_FALLOFF )
////      {
////         Parser.GetKeyString( CLIGHTSTATEFALLOFFS, String );
////         sscanf( LPCTSTR( String ), "%d %d %d", &mDefaultState.mFalloff1,
////            &mDefaultState.mFalloff2, &mDefaultState.mFalloff3 );
////      }
////
////      // get duration
////      if( mDefaultState.mFlags & STATE_HASDURATION )
////      {
////         float tmp1;
////         Parser.GetKeyString( CLIGHTSTATEDURATION, String );
////         sscanf( LPCTSTR( String ), "%f", &tmp1 );
////         mDefaultState.mDuration = double( tmp1 );
////      }
////      
////      // get position
////      if( mDefaultState.mFlags & STATE_HASPOSITION )
////      {
////         int tmp1, tmp2, tmp3;
////         Parser.GetKeyString( CLIGHTSTATEPOSITION, String );
////         sscanf( LPCTSTR( String ), "%d %d %d", &tmp1, &tmp2, &tmp3 );
////         mDefaultState.mPosition.X = float( tmp1 );
////         mDefaultState.mPosition.Y = float( tmp2 );
////         mDefaultState.mPosition.Z = float( tmp3 );
////      }
////      
////      // get color
////      if( mDefaultState.mFlags & STATE_HASCOLOR )
////      {
////         float tmp1, tmp2, tmp3;
////         Parser.GetKeyString( CLIGHTSTATECOLOR, String );
////         sscanf( LPCTSTR( String ), "%f %f %f", &tmp1, &tmp2, &tmp3 );
////         mDefaultState.mColor.red = float( tmp1 );
////         mDefaultState.mColor.green = float( tmp2 );
////         mDefaultState.mColor.blue = float( tmp3 );
////      }
////      
////      // get spotfactors
////      if( mDefaultState.mFlags & STATE_HASSPOTFACTOR )
////      {
////         float tmp1, tmp2, tmp3, tmp4, tmp5;
////         Parser.GetKeyString( CLIGHTSTATESPOTFACTOR, String );
////         sscanf( LPCTSTR( String ), "%f %f %f %f %f", &tmp1, &tmp2,
////            &tmp3, &tmp4, &tmp5 );
////         mDefaultState.mSpotVector.X = float( tmp1 );
////         mDefaultState.mSpotVector.Y = float( tmp2 );
////         mDefaultState.mSpotVector.Z = float( tmp3 );
////         mDefaultState.mSpotTheta = float( tmp4 );
////         mDefaultState.mSpotPhi = float( tmp5 );
////      }
////      
////      int NumStates = 0;
////      
////      // get number of states
////      Parser.GetKeyInt( CLIGHTSTATECOUNT, NumStates );
////
////      // have to have at least 1 state
////      if( !NumStates )
////         return;
////      
////      // go through states
////      for( int State = 0; State < NumStates; State++ )
////      {
////         CLightState    LS;
////         
////         // get the flags
////         Parser.GetKeyInt( CLIGHTSTATEFLAGS, IntVal );
////         LS.mFlags = IntVal;
////                  
////         // get falloffs
////         if( LS.mFlags & STATE_LINEAR_FALLOFF )
////         {
////            Parser.GetKeyString( CLIGHTSTATEFALLOFFS, String );
////            sscanf( LPCTSTR( String ), "%d %d", &LS.mFalloff1,
////               &LS.mFalloff2 );
////         }
////         else if( LS.mFlags & STATE_DISTANCE_FALLOFF )
////         {
////            Parser.GetKeyString( CLIGHTSTATEFALLOFFS, String );
////            sscanf( LPCTSTR( String ), "%d %d %d", &LS.mFalloff1,
////               &LS.mFalloff2, &LS.mFalloff3 );
////         }
////
////         // get duration
////         if( LS.mFlags & STATE_HASDURATION )
////         {
////            float tmp1;
////            Parser.GetKeyString( CLIGHTSTATEDURATION, String );
////            sscanf( LPCTSTR( String ), "%f", &tmp1 );
////            LS.mDuration = double( tmp1 );
////         }
////         
////         // get position
////         if( LS.mFlags & STATE_HASPOSITION )
////         {
////            int tmp1, tmp2, tmp3;
////            Parser.GetKeyString( CLIGHTSTATEPOSITION, String );
////            sscanf( LPCTSTR( String ), "%d %d %d", &tmp1, &tmp2, &tmp3 );
////            LS.mPosition.X = float( tmp1 );
////            LS.mPosition.Y = float( tmp2 );
////            LS.mPosition.Z = float( tmp3 );
////            
////            // check if state 0
////            if( !State )
////               mCurrentPosition = mMovePosition = LS.mPosition;
////         }
////
////         // check for no state[0] position         
////         if( !State && !( LS.mFlags & STATE_HASPOSITION ) )
////            mCurrentPosition = mMovePosition = LS.mPosition = mDefaultState.mPosition;
////         
////         // get color
////         if( LS.mFlags & STATE_HASCOLOR )
////         {
////            float tmp1, tmp2, tmp3;
////            Parser.GetKeyString( CLIGHTSTATECOLOR, String );
////            sscanf( LPCTSTR( String ), "%f %f %f", &tmp1, &tmp2, &tmp3 );
////            LS.mColor.red = float( tmp1 );
////            LS.mColor.green = float( tmp2 );
////            LS.mColor.blue = float( tmp3 );
////         }
////         
////         // get spotfactors
////         if( LS.mFlags & STATE_HASSPOTFACTOR )
////         {
////            float tmp1, tmp2, tmp3, tmp4, tmp5;
////            Parser.GetKeyString( CLIGHTSTATESPOTFACTOR, String );
////            sscanf( LPCTSTR( String ), "%f %f %f %f %f", &tmp1, &tmp2,
////               &tmp3, &tmp4, &tmp5 );
////            LS.mSpotVector.X = float( tmp1 );
////            LS.mSpotVector.Y = float( tmp2 );
////            LS.mSpotVector.Z = float( tmp3 );
////            LS.mSpotTheta = float( tmp4 );
////            LS.mSpotPhi = float( tmp5 );
////         }
////                  
////         // add the state
////         AddState( LS );
////      }
////      
////		Parser.GotoEnd();
////	}
//}
//
//
//// ------------------------------------------------------------------------------
//
//// check if this light is active
//int CLight::IsActive( void )
//{
//   // return the flag value
//	return( flags & LIGHT_ACTIVE );
//}
//
//// overloaded = function
//CLight& CLight::operator=( CLight & Light )
//{
//   // copy all the values
//   id                = Light.id;
//   name              = Light.name;
//   group             = Light.group;
//   animationDuration = Light.animationDuration;
//
//   // remove the current list ( in case there is one )
//   states.RemoveAll();
//   
//   // copy the list of states
//   states.AddHead( &Light.states );
//   
//	return *this;
//}
//
//// ------------------------------------------------------------------------------
//
//// add a state to the light and return the number of states in the list
//int CLight::AddState( CLightState & LightState )
//{
//   // add this state ( to the top for now )
//   states.AddTail( LightState );
//   
//   // return the number of states
//   return( states.GetCount() );
//}
//
//// ------------------------------------------------------------------------------
//
//// return the number of states in this
//int CLight::GetNumStates( void )
//{
//   // return the num
//   return( states.GetCount() );
//}
//
//// ------------------------------------------------------------------------------
//
//// get teh unique id for this light
//bool CLight::GetID( void )
//{
////   mID = -1;
////   
////   if( states.GetCount() > 1 )
////   {
////      if( !( mFlags & LIGHT_HASNAME ) )
////      {
////         AfxMessageBox( "All lights with multiple states need to be named" );
////         return( FALSE );
////      }
////      
////      char szBuf[ 128 ];
////      mID = 0;
////      strcpy( szBuf, mName );
////      for( unsigned int i = 0; i < strlen( szBuf ); i++ )
////         mID += szBuf[ i ] + ( mID / ( i + 1 ) ) + ( 1 << i );
////
////      // make sure over 32k
////      mID += 0xffff;
////   }
//   
//   return( true );
// }
//
//// ------------------------------------------------------------------------------
//
//CLightState::CLightState() :
//   selected( false ),
//   duration( 0.f ),
//   position( 0.f, 0.f, 0.f ),
//   color( 1.f, 1.f, 1.f ),
//   spotVector( 0.f, 0.f, 0.f ),
//   spotTheta( 0.f ),
//   spotPhi( 0.f ),
//   falloff( Linear ),
//   lightType( PointLight ),
//   d1( 0.f ),
//   d2( 0.f ),
//   d3( 0.f )
//{
//   //
//}
//
//// ------------------------------------------------------------------------------
//
//int CLightState::IsStateInBox( ThredBox & Box )
//{
//	ThredPoint NewOrig = ThredPoint( position.x, position.y, position.z );
//
//	// make a new origin that is zero in the same
//	// axis as the box so when tested it works
//	if( Box.mSmallest.X == 0 )
//		NewOrig.X = 0;
//	if( Box.mSmallest.Y == 0 )
//		NewOrig.Y = 0;
//	if( Box.mSmallest.Z == 0 )
//		NewOrig.Z = 0;
//
//	// check the point
//	return( Box.CheckPointIntersection( NewOrig ) );
//}
//
//// get the distance between this light and a point
//double CLightState::GetDistance( ThredPoint Point )
//{
//	double x1, y1, x2, y2;
//
//	// check which vars to use
//	if( Point.X == 0 ) {
//		x1 = position.y;
//		y1 = position.z;
//		x2 = Point.Y;
//		y2 = Point.Z;
//	}
//	else if( Point.Y == 0 ) {
//		x1 = position.x;
//		y1 = position.z;
//		x2 = Point.X;
//		y2 = Point.Z;
//	}
//	else {
//		x1 = position.y;
//		y1 = position.x;
//		x2 = Point.Y;
//		y2 = Point.X;
//	}
//
//	// calculate our deltas
//	double xdistance = x2 - x1;
//	double ydistance = y2 - y1;
//	return( sqrt( ( xdistance * xdistance ) + ( ydistance * ydistance ) ) );
//}
//
//// ------------------------------------------------------------------------------
//
//// move a light to a point
//void CLightState::MoveState( double X, double Y, double Z )
//{
//   // change the values
//   if( Globals.mAxis & AXIS_X ) {
//   	position.x -= X;
//   }
//   if( Globals.mAxis & AXIS_Y ) {
//   	position.y -= Y;
//   }
//   if( Globals.mAxis & AXIS_Z ) {
//   	position.z -= Z;
//   }
//}
//
//// ------------------------------------------------------------------------------
//
//// set the done condition on a move for a light object
//void CLightState::DoneMoveState( double GridSize )
//{
//   ThredPoint  PosDiff;
//   
//	// Snap to the grid
//	if( Globals.mAxis & AXIS_X ) 
//		position.x = RoundFloat( ( position.x / GridSize ) ) * GridSize;
//	if( Globals.mAxis & AXIS_Y ) 
//		position.y = RoundFloat( ( position.y / GridSize ) ) * GridSize;
//	if( Globals.mAxis & AXIS_Z ) 
//		position.z = RoundFloat( ( position.z / GridSize ) ) * GridSize;
//
////	// check if the shift key is being pressed, move only on the axis with most 
////	if( GetAsyncKeyState( VK_SHIFT ) & 0x8000 )
////   {
////		double XDis, YDis, ZDis;
////		XDis = fabs( mMovePosition.X - mCurrentPosition.X );
////		YDis = fabs( mMovePosition.Y - mCurrentPosition.Y );
////		ZDis = fabs( mMovePosition.Z - mCurrentPosition.Z );
////		
////      // check which is greater
////      if(XDis > YDis ) 
////      {
////			if( XDis > ZDis ) 
////				mCurrentPosition.X = mMovePosition.X;
////			else
////				mCurrentPosition.Z = mMovePosition.Z;
////		}
////		else 
////      {
////			if( YDis > ZDis )
////				mCurrentPosition.Y = mMovePosition.Y;
////			else
////				mCurrentPosition.Z = mMovePosition.Z;
////		}
////		mMovePosition = mCurrentPosition;
////	}
////	else mCurrentPosition = mMovePosition;
////   
////   // check if the states positions are locked   
////   if( mLockLightPosition )
////   {
////      for( int i = 0; i < parent->mStates.GetCount(); i++ )
////      {
////         CLightState & LightState = 
////            mStates.GetAt( mStates.FindIndex( i ) );
////            
////         // make sure this state contains position information
////         if( LightState.mFlags & STATE_HASPOSITION )
////            LightState.mPosition = LightState.mPosition + PosDiff;
////      }
////   }
////   
////   // set state0 with same position
////   if( mStates.GetCount() )
////   {
////      CLightState & LightState = 
////         mStates.GetAt( mStates.FindIndex( 0 ) );
////      LightState.mPosition = mCurrentPosition;
////      
////      // check if need to set default
////      if( !( LightState.mFlags & STATE_HASPOSITION ) )
////         mDefaultState.mPosition = mCurrentPosition;
////   }
//}
