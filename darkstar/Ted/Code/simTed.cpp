#include <limits.h>
#include "simAction.h"
#include "simTed.h"
#include "grdCollision.h"
#include "simSetIterator.h"
#include "simguitsctrl.h"
#include "simmoveobj.h"
#include "console.h"
#include "simGame.h"
#include "simconsoleevent.h"

static SimTerrain::Visibility DefTerrainVisibility =
{
   1500.0f,       // visible distance
   600.0f,       // haze distance
};

static SimTerrain::Detail DefTerrainDetail =
{
   200.0f,        // perspective distance
	0.02f,			// Pixel Size
};

static const char * stripPath( const char* fname )
{
	// Return the file name portion without the path
	const char* ptr = fname + strlen(fname) - 1;
	for (; ptr != fname; ptr--)
		if (*ptr == '\\' || *ptr == '/' || *ptr == ':')
			return ptr + 1;
	return fname;
}

SimTed::SimTed() :
   scaleShift( 0 ),
   blockShift( 0 ),
   mouseHandler( NULL ),
   mouseStartPt( 0.0, 0.0, 0.0 ),
   mouseDirection( 0.0, 0.0, 0.0 ),
   terrain( NULL ),
   landscaper( NULL ),
   focused( false ),
   surface( NULL ),
   pointArray( NULL ),
   currentUndo( NULL ),
   size( -1, -1 ),
   regionSelect( false ),
   selectStartPnt( -1, -1 ),
   selectCanceled( false ),
   selectLastPnt( -1, -1 ),
   mouseLButtonAction( -1 ),
   mouseRButtonAction( -1 ),
   mouseProcessAction(-1),
   feather( true ),
   snap( true ),
   brushPos( 0, 0 ),
   brushDetail( 0 ),
   hilightFillColor( 8 ),
   hilightFrameColor( 2 ),
   selectFillColor( 4 ),
   selectFrameColor( 3 ),
   shadowFillColor( 12 ),
   shadowFrameColor( 6 ),
   heightVal( 200.0 ),
   adjustVal( 5.0 ),
   materialIndex( 0 ),
   flagVal( set ),
   pasteVal( Material | Height ),
   smoothVal( 0.5 ),
   terrainType( 0 ),
   drawNotch( true ),
   scaleVal( 1.0 ),
   grabMode( false ),
   grabPoint( 0, 0 ),
   grabDist( 0.0 ),
   grabHeight( 0.0 ),
   drawNeighbors(false),
   blockOutline( false ),
   blockFrameColor( 14 ),
   pinDetail( 0 ),
   selFloating( false ),
   avgHeight( 0.0 ),
   mouseMoved( false ),
   cursorPos( 0, 0 ),
   lastCursorPos( 0, 0 ),
   cursorDif( 0, 0 ),
   hilightShow( Frame | Fill ),
   selectShow( Frame | Fill ),
   shadowShow( Frame | Fill )
{
}

SimTed::~SimTed()
{
   // delete the objects that this has control of
   if( currentUndo )
      delete currentUndo;
      
   // delete the version string
}

void SimTed::flushTextures()
{
   if( surface )
      surface->flushTextureCache();
}

void SimTed::setVars()
{
	scaleShift = getGridFile()->getScale();
	blockShift = getGridFile()->getDetailCount()-1;
	size = getGridFile()->getSize();
}

int SimTed::getActionIndex( String name )
{
   for( int i = 0; i < actions.size(); i++ )
      if( name == actions[i]->name )
         return( i );
   
   return( -1 );
}

void SimTed::setLButtonAction( String name ) 
{ 
   mouseLButtonAction = getActionIndex( name ); 
}

void SimTed::setRButtonAction( String name ) 
{ 
   mouseRButtonAction = getActionIndex( name ); 
}

void SimTed::init()
{
   // add the actions    
   // there is the potential that an end event will never occur... design
   // the mouse actions to be independant of end events
   addAction( "adjustControlPoint", &SimTed::adjustControlPointAction, true, "Adjust the control point for this brush" );
   addAction( "adjustCorner", &SimTed::adjustCornerAction, false, "Adjust the corner of a brush" );
   addAction( "adjustHeight", &SimTed::adjustHeightAction, true, "Adjust the height of terrain" );
   addAction( "clearFlags", &SimTed::clearFlagsAction, true, "Clear masked flags" );
   addAction( "clearPin", &SimTed::clearPinAction, true, "Clear terrain pinning" );
   addAction( "copy", &SimTed::copyAction, false, "Copy terrain info" );
   addAction( "deselect", &SimTed::deselectAction, true, "Deselect terrain" );
   addAction( "depress", &SimTed::depressAction, true, "Depress height" );
   addAction( "elevate", &SimTed::elevateAction, true, "Elevate height" );
   addAction( "getAvgHeight", &SimTed::getAvgHeightAction, false, "" );
   addAction( "lowerHeight", &SimTed::lowerHeightAction, true, "Lower height" );
   addAction( "noise", &SimTed::noiseAction, false, "Add noise to terrain" );
   addAction( "paste", &SimTed::pasteAction, false, "Paste using paste mask" );
   addAction( "pin", &SimTed::pinAction, true, "Pin down the terrain" );
   addAction( "raiseHeight", &SimTed::raiseHeightAction, true, "Raise height" );
   addAction( "redo", &SimTed::redoAction, true, "Redo last undo operation" );
   addAction( "relight", &SimTed::relightAction, true, "Relight the terrain" );
   addAction( "rotateLeft", &SimTed::rotateLeftAction, true, "Rotate terrain textures" );
   addAction( "rotateRight", &SimTed::rotateRightAction, true, "Rotate terrain textures" );
   addAction( "scale", &SimTed::scaleAction, true, "Scale height values" );
   addAction( "select", &SimTed::selectAction, true, "Select terrain" );
   addAction( "setControlHeight", &SimTed::setControlHeightAction, true, "Set control point height" );
   addAction( "setCornerHeight", &SimTed::setCornerHeightAction, true, "Set the height of a corner" );
   addAction( "setFlags", &SimTed::setFlagsAction, true, "Set masked flags" );
   addAction( "setHeight", &SimTed::setHeightAction, true, "Set terrain height" );
   addAction( "setMaterial", &SimTed::setMaterialAction, true, "Set terrain material index" );
   addAction( "setTerrainType", &SimTed::setTerrainTypeAction, true, "Set terrain type using terrainValue" );
   addAction( "smooth", &SimTed::smoothAction, true, "Smooth terrain" );
   addAction( "undo", &SimTed::undoAction, true, "Undo last operation" );
      
   // set the l/r mouse button actions
   setLButtonAction( "select" );
   setRButtonAction( "deselect" );

   // need to get rid of the simted object on server deletion
   SimObject * notifyObj = static_cast<SimObject*>(SimGame::get()->getManager(SimGame::SERVER)->findObject("ConsoleScheduler"));
   if(notifyObj)
      deleteNotify(notifyObj);
}

void SimTed::onDeleteNotify(SimObject * obj)
{
   // just check if it's a SimConsoleScheduler object (already removed from the manager)
   if(dynamic_cast<SimConsoleScheduler*>(obj))
      deleteObject();
}


bool SimTed::getActionInfo( int index, TedAction & info )
{
   if( index >= getNumActions() )
      return( false );
      
   // copy it
   info = *actions[index];
   
   return( true );
}

// set the corner height to the current height val
void SimTed::setCornerHeightAction( SimTed::BrushAction action, Selection & sel, Selection & undo )
{
   if( !sel.points.getSize() )
      return;
   
   if( !getSnap() )
      return;
      
   switch( action )
   {
      case begin: 
      
         // adjust this corner - darn.. the naming is all backwards!!!
//         adjustControlPoint( pos, height, undo, top, left, true );
         
         setBrushCornerHeight( brushPos, undo, heightVal, true );
         
         break;
   }
}

void SimTed::adjustCornerAction( SimTed::BrushAction action, Selection & sel, Selection & undo )
{
   if( !sel.points.getSize() )
      return;
   
   if( !getSnap() )
      return;
      
   switch( action )
   {
      case begin: 
      {
         // enter into grab mode
         grabMode = true;         
         drawNeighbors = true;
         grabPoint = brushPos;
         
         grabHeight = getHeight( grabPoint );
         
         // save off the info into the undo buffer
         setBrushCornerHeight( grabPoint, undo, grabHeight, true );

         break;
      }
         
      case update:
      {
         // save off the info into the undo buffer
         float mouseAdj = CMDConsole::getLocked()->getFloatVariable("$TED::mouseAdjustValue", 0.25);
         grabHeight -= ( ( float )cursorDif.y ) * mouseAdj;
         
         setBrushCornerHeight( grabPoint, undo, grabHeight, false );

         break;
      }
         
      case end:
         grabMode = false;
         getGridFile()->updateHeightRange();
         break;
   }
}

void SimTed::setControlHeightAction( SimTed::BrushAction action, Selection & sel, Selection & undo )
{
   if( !sel.points.getSize() )
      return;
   
   if( !getSnap() )
      return;
      
   switch( action )
   {
      case begin: 
      
         // set it
         setBrushCornerHeight( brushPos, undo, heightVal, true );
         
         break;

      case end:         
         getGridFile()->updateHeightRange();
         break;
         
   }
}

void SimTed::adjustHeightAction( SimTed::BrushAction action, Selection & sel, Selection & undo )
{
   static Selection initialSel;

   if(!sel.points.getSize())
      return;
   unsigned int i;
   switch(action)
   {
      // add to undo on begin
      case begin:
      
         initialSel.points.clear();
         
         // enter into grab mode
         grabMode = true;
         drawNeighbors = false;
         grabPoint = brushPos;
         grabHeight = getHeight(grabPoint);
         
         // copy the current selection into undo and initial
         for(i = 0; i < sel.points.getSize(); i++)
         {
            Selection::Info info;
            if(getInfo(sel.points[i].pos, info))
            {
               initialSel.addPoint(info);
               undo.addPoint(info);
            }
         }
         
         break;
         
      case update:
      {
         if(cursorDif.y == 0)
            break;
            
         // get the adjust value
         float mouseAdj = CMDConsole::getLocked()->getFloatVariable("$TED::mouseAdjustValue", 0.25);
         float adjVal = ((float)-cursorDif.y) * mouseAdj;
         
         for(i = 0; i < initialSel.points.getSize(); i++)
         {
            float height = getHeight(initialSel.points[i].pos) + adjVal;
            setHeight(initialSel.points[i].pos, height);
         }
         
         // do the feathering
         if(feather)
            featherAction(action, initialSel, undo, true, adjVal);
            
         break;
      }
         
      case end:
         grabMode = false;
         getGridFile()->updateHeightRange();
         break;
   }
}

// control adjustment mode
void SimTed::adjustControlPointAction( SimTed::BrushAction action, Selection & sel, Selection & undo )
{
   if( !sel.points.getSize() )
      return;
   
   if( !getSnap() )
      return;
      
   int brushDim = ( 1 << brushDetail );
   
   switch( action )
   {
      case begin: 
      {
         grabPoint = brushPos;
         grabHeight = getHeight( grabPoint );
         grabMode = true;         
         drawNeighbors = true;
         
         // save all the heights...
         for( int y = 0; y < brushDim; y++ )
            for( int x = 0; x < brushDim; x++ )
               for( int i = -1; i <=0; i++ )
                  for( int j = -1; j <=0; j++ )
                  {
                     Selection::Info info;
                     Point2I pos( brushPos );
                     pos.x += brushDim * i;
                     pos.y += brushDim * j;
                     
                     if( getInfo( pos, info ) )
                        undo.addPoint( info );
                  }
         
         break;
      }
      
      case update:
      {
         if( cursorDif.y == 0 )
            break;
            
         float mouseAdj = CMDConsole::getLocked()->getFloatVariable("$TED::mouseAdjustValue", 0.25);
         float diff = ((float)-cursorDif.y) * mouseAdj;
         
         // do the zero'th lvl
         Selection::Info info;
         if( getInfo( grabPoint, info ) )
         {
            info.height.height += diff;
            setInfo( info );
         }
         
         for( int i = 1; i < brushDim; i++ )
         {
            // get the amount to adjust at each level
            float adjAmt = float( float(brushDim) - float(i) ) / float(brushDim) * diff;

            // just grab the outside points
            for( int x = -i; x <= i; x++ )
               for( int a = 0; a < 2; a++ )
                  if( getInfo( Point2I( grabPoint.x + x, grabPoint.y + ( a ? -i : i ) ), info ) )
                  {
                     info.height.height += adjAmt;
                     setInfo( info );
                  }
            
            // do the sides - skip the corners...
            for( int y = (-i + 1); y <= (i - 1); y++ )
               for( int a = 0; a < 2; a++ )
                  if( getInfo( Point2I( grabPoint.x + ( a ? -i : i ), grabPoint.y + y ), info ) )
                  {
                     info.height.height += adjAmt;
                     setInfo( info );
                  }
         }
         break;
      }
      
      case end:
         grabMode = false;
         getGridFile()->updateHeightRange();
         break;
   }
}


// scale the terrain by a value
void SimTed::scaleAction( SimTed::BrushAction action, Selection & sel,
   Selection & undo )
{
   switch( action )
   {
      case begin:
      case update:
      {
         // go through the heights and scale them
         for( unsigned int i = 0; i < sel.points.getSize(); i++ )
         {
            // get the info
            Selection::Info info;
            
            if( getInfo( sel.points[i].pos, info ) )
            {
               // add to undo
               undo.addPoint( info );
            
               // scale the value
               info.height.height *= scaleVal;
            
               // set the value
               setInfo( info );
            }
         }
         
         break;
      }
      case end:
         getGridFile()->updateHeightRange();
         break;
   }
}

// pin down the terrain to the current detail
void SimTed::pinAction( SimTed::BrushAction action, Selection & sel,
   Selection & undo )
{
   switch( action )
   {
      case end:
      
         // go though all of the selection
         undo.pinArray.clear();
         for( unsigned int i = 0; i < sel.points.getSize(); i++ )
         {
            // get the info ( the current selection only has pos as valid )
            Selection::Info info;
      
            Point2I pos = sel.points[i].pos;
            
            // get the info      
            if( getInfo( pos, info ) )
            {            
               // add to the undo 
               undo.addPoint( info );
               undo.undoFlags = Selection::pin;

               Point2I bpos;
               bpos.x = pos.x >> blockShift;
               bpos.y = pos.y >> blockShift;

               if( bpos.x >= 0 && bpos.x < size.x && bpos.y >= 0 && bpos.y < size.y )
               {
                  if( GridBlock * block = getGridFile()->getBlock( bpos ) )
                  {
                     //get remainders
                     Point2I spos;
            			spos.x = ( pos.x - ( bpos.x << blockShift ) );
            			spos.y = ( pos.y - ( bpos.y << blockShift ) );

                     // pin it - position is in current detail coords
                     spos.x >>= pinDetail;
                     spos.y >>= pinDetail;
                     
                     // fill in the callback info
                     GridBlock::pinCallbackInfo callback;
                     callback.obj = this;
                     callback.func = SimTed::pinSquareCallback;
                     callback.gridBlock = block;
                     
                     block->pinSquare( pinDetail, spos, pinDetailMax, true, &callback );
                  }
               }
            }
         }
      
         break;
   }
}

// used to collect info for an undo/redo operation
void SimTed::pinSquareCallback( GridBlock::pinCallbackInfo * info )
{
   SimTed * ted = ( SimTed * )info->obj;
   Selection * undo = ted->getCurrentUndo();

   Selection::pinInfo pin;
   
   // need to check if this one already exists
   for( int i = 0; i < undo->pinArray.size(); i++ )
   {
      // check if the same
      pin = undo->pinArray[i];
      if( ( pin.detail == info->detail ) && ( pin.pos == info->pos ) && ( pin.gridBlock == info->gridBlock ) )
         return;
   }
   
   // add a pin info
   pin.detail = info->detail;
   pin.pos = info->pos;
   pin.val = info->val;
   pin.gridBlock = info->gridBlock;
   
   undo->pinArray.push_back( pin );   
}


// clear the entire map in the current block
void SimTed::clearPinMaps()
{
   GridBlockList * blockList = getGridFile()->getBlockList();

   for( int i = 0; i < blockList->size(); i++ )
   {
      GridBlock * block = ( * blockList )[i]->block;
      
      if( block )
         block->clearPinMaps();
   }
}

// clear the pin from the current brush - goes from pinDetail on up to pinDetailMax
void SimTed::clearPinAction( SimTed::BrushAction action, Selection & sel, 
   Selection & undo )
{
   switch( action )
   {
      case end:

         // go though all of the selection
         undo.pinArray.clear();
         for( unsigned int i = 0; i < sel.points.getSize(); i++ )
         {
            // get the info ( the current selection only has pos as valid )
            Selection::Info info;
      
            Point2I pos = sel.points[i].pos;
            
            // get the info      
            if( getInfo( pos, info ) )
            {            
               // add to the undo 
               undo.addPoint( info );
               undo.undoFlags = Selection::unpin;

               Point2I bpos;
               bpos.x = pos.x >> blockShift;
               bpos.y = pos.y >> blockShift;

               if( bpos.x >= 0 && bpos.x < size.x && bpos.y >= 0 && bpos.y < size.y )
               {
                  if( GridBlock * block = getGridFile()->getBlock( bpos ) )
                  {
                     //get remainders
                     Point2I spos;
            			spos.x = ( pos.x - ( bpos.x << blockShift ) );
            			spos.y = ( pos.y - ( bpos.y << blockShift ) );

                     // pin it - position is in current detail coords
                     spos.x >>= pinDetail;
                     spos.y >>= pinDetail;
                     
                     // fill in the callback info
                     GridBlock::pinCallbackInfo callback;
                     callback.obj = this;
                     callback.func = SimTed::pinSquareCallback;
                     callback.gridBlock = block;
                     
                     block->pinSquare( pinDetail, spos, pinDetailMax, false, &callback );
                  }
               }
            }
         }
      
         break;
   }
}

// just calls relight ( so can be mapped to a mouse key )
void SimTed::relightAction( SimTed::BrushAction action, Selection & sel,
   Selection & undo )
{
   sel; undo;
   switch( action )
   {
      case begin:
         relight(false);
         break;
   }
}

// just calls undo ( so we can use the mouse with this )
void SimTed::undoAction( SimTed::BrushAction action, Selection & sel, 
   Selection & undo )
{
   sel; undo;
   switch( action )
   {
      case begin:
         SimTed::undo();
         break;
      case end:
         getGridFile()->updateHeightRange();
         break;
   }
}

// just calls redo ( so we can use the mouse with this )
void SimTed::redoAction( SimTed::BrushAction action, Selection & sel, 
   Selection & undo )
{
   sel; undo;
   switch( action )
   {
      case begin:
         SimTed::redo();
         break;
      case end:
         getGridFile()->updateHeightRange();
         break;
   }
}

// set the terrain type for the selected terrain
void SimTed::setTerrainTypeAction( SimTed::BrushAction action, Selection & sel,
   Selection & undo )
{
   unsigned int i;
   
   switch( action )
   {
      case begin:
      {
      
         // clear the hilight selection
         hilightSel.clear();
         for( i = 0; i < sel.points.getSize(); i++ )
            hilightSel.addPoint( sel.points[i] );
         break;
      }
         
      case update:
      {
         // add to the hilight selection
         for( i = 0; i < sel.points.getSize(); i++ )
            hilightSel.addPoint( sel.points[i] );
         break;
      }
         
      case end:
      {
         Vector<Point2I> pntList;
         
         // add to the hilight selection
         for( i = 0; i < sel.points.getSize(); i++ )
            hilightSel.addPoint( sel.points[i] );
            
         // create the pnt list
         for( i = 0; i < hilightSel.points.getSize(); i++ )
         {
            // add to list
            pntList.push_back( hilightSel.points[i].pos );
            
            Selection::Info info;
            
            // add to undo
            if( getInfo( hilightSel.points[i].pos, info ) )
               undo.addPoint( info );
         }
         
         // save the flags
         UInt8 flagSave = flagVal;
         
         // clear the edit flag
         flagVal = GridBlock::Material::Edit | GridBlock::Material::Corner;
         setFlagsAction( action, hilightSel, undo );
         
         // apply the textures
         landscaper->applyTextures( &pntList, terrainType );

         // flush them textures            
         flushTextures();
               
         // set the relight flag for this undo
         undo.undoFlags = Selection::landscape | Selection::flushTextures;
         
         // set the edit flag
         flagVal = GridBlock::Material::Edit | GridBlock::Material::Corner;
         setFlagsAction( action, hilightSel, undo );
         
         // clear the hilight selection
         hilightSel.clear();
         
         // reset the flag mask
         flagVal = flagSave;
         
         break;
      }
   }
   
   if( hilightShow.test( Outline ) )
      clusterizeSelection( hilightClusterSel, hilightSel );
}

// copy info
void SimTed::copyAction( SimTed::BrushAction action, Selection & sel,
   Selection & undo )
{
   undo;
   
   switch( action )
   {
      case begin:
      {         
         // clear the copy selection
         copySel.clear();
       
         if( !sel.points.getSize() )
            return;
              
         // use the first point as the one for reference
         Point2I startPnt = sel.points[0].pos;
         
         // go though all of the selection
         for( unsigned int i = 0; i < sel.points.getSize(); i++ )
         {
            // get the info ( the current selection only has pos as valid )
            Selection::Info info;
            
            if( getInfo( sel.points[i].pos, info ) )
            {
               // adjust the point
               info.pos.x -= startPnt.x;
               info.pos.y -= startPnt.y;
            
               // add to the copy selection
               copySel.addPoint( info );
            }
         }
         
         // clusterize this selection in case we want it later
         clusterizeSelection( copyClusterSel, copySel );
         
         break;
      }
   }
}

// paste info
void SimTed::pasteAction( SimTed::BrushAction action, Selection & sel,
   Selection & undo )
{
   undo;sel;
   
   switch( action )
   {
      case begin:
      {
         // get the start point and check if there is something to paste
         if( !copySel.points.getSize() )
            return;
            
         Point2I startPnt = brushPos;
         
         // go though all of the copy buffer
         for( unsigned int i = 0; i < copySel.points.getSize(); i++ )
         {
            // get the point we are going to be working with
            Point2I pnt( startPnt.x + copySel.points[i].pos.x,
               startPnt.y + copySel.points[i].pos.y );
             
            if( !pointValid( pnt ) )
               continue;
                 
            // get the info for this point
            Selection::Info info;
            
            undo.undoFlags = Selection::flushTextures;
            
            UInt8 paste = selFloating ? floatPasteVal : pasteVal;

            if( getInfo( pnt, info ) )
            {
               // add to the undo
               undo.addPoint( info );
            
               // check for material change
               if( paste & Material )
                  info.material = copySel.points[i].material;   
            
               // check for height change
               if( paste & Height )
                  info.height = copySel.points[i].height;
                  
               // add the info
               setInfo( info );
            }
            
            // fjush the textures
            flushTextures();
         }

         break;
      }
      
      case end:
         selFloating = false;
         getGridFile()->updateHeightRange();
         break;
   }
}

// set the material of the selections
void SimTed::setMaterialAction( SimTed::BrushAction action, Selection & sel,
   Selection & undo )
{
   switch( action )
   {
      case begin:
      case update:
      {
         // go though all of the selection
         for( unsigned int i = 0; i < sel.points.getSize(); i++ )
         {
            // get the info ( the current selection only has pos as valid )
            Selection::Info info;
            
            if( getInfo( sel.points[i].pos, info ) )
            {            
               // add to the undo 
               undo.addPoint( info );
            
               // set the material index
               info.material.index = materialIndex;
            
               // set the edit flag
               info.material.flags |= GridBlock::Material::Edit;
            
               // store back
               setInfo( info );
            }
         }
         
         // flush them textures            
         flushTextures();

         undo.undoFlags = Selection::flushTextures;
            
         
         break;
      }
   }
}

// raise the terrain as a smooth operation about the center of the
// selection
void SimTed::elevateAction( SimTed::BrushAction action, Selection & sel,
   Selection & undo )
{
   if( !getBrushDetail() )
      raiseHeightAction( action, sel, undo );
      
   switch( action )
   {
      case begin:
      case update:
      {
         RectI bound;
         Point2F center;
         float radius;
         
         // get the selection info
         setSelectionVars( sel, bound, center, radius );
         
         // walk through the selection
         for( unsigned int i = 0; i < sel.points.getSize(); i++ )
         {
            Point2I p = sel.points[i].pos;
            float dx = p.x - center.x;
            float dy = p.y - center.y;
            float distance = sqrt( ( dx * dx ) + ( dy *dy ) );
            float scaledDelta = adjustVal * ( radius - distance );

            // get the info at this point
            Selection::Info info;
            
            if( getInfo( p, info ) )
            {
               // add to the undo 
               undo.addPoint( info );
            
               // adjust the height
    			   setHeight( p, info.height.height + scaledDelta );
            }
         }
         
         break;
      }
      
      case end:
         getGridFile()->updateHeightRange();
         break;
   }
}

// lower the terrain as a smooth operation about the center of the
// selection
void SimTed::depressAction( SimTed::BrushAction action, Selection & sel,
   Selection & undo )
{
   if( !getBrushDetail() )
      lowerHeightAction( action, sel, undo );
      
   switch( action )
   {
      case begin:
      case update:
      {
         RectI bound;
         Point2F center;
         float radius;
         
         // get the selection info
         setSelectionVars( sel, bound, center, radius );
         
         // walk through the selection
         for( unsigned int i = 0; i < sel.points.getSize(); i++ )
         {
            Point2I p = sel.points[i].pos;
            float dx = p.x - center.x;
            float dy = p.y - center.y;
            float distance = sqrt( ( dx * dx ) + ( dy *dy ) );
            float scaledDelta = -adjustVal * ( radius - distance );

            // get the info at this point
            Selection::Info info;
            
            if( getInfo( p, info ) )
            {
               // add to the undo 
               undo.addPoint( info );
            
               // adjust the height
    			   setHeight( p, info.height.height + scaledDelta );
            }
         }
         
         break;
      }
      
      case end:
         getGridFile()->updateHeightRange();
         break;
         
   }
}

// just call into the rotate action
void SimTed::rotateRightAction( SimTed::BrushAction action, Selection & sel,
   Selection & undo )
{
   rotateAction( action, sel, undo, true );
}

// just call into the rotate action
void SimTed::rotateLeftAction( SimTed::BrushAction action, Selection & sel,
   Selection & undo )
{
   rotateAction( action, sel, undo, false );
}


// rotate the selected terrain by 90 deg
void SimTed::rotateAction( SimTed::BrushAction action, Selection & sel,
   Selection & undo, bool rotRight )
{
   unsigned int i;
   
   switch( action )
   {
      case begin:
      case update:

         // go through the selection   
         for( i = 0; i < sel.points.getSize(); i++ )
         {
            // get the info at this position
            Selection::Info info;
            
            if( !getInfo( sel.points[i].pos, info ) )
               continue;
            
            // add to the undo
            undo.addPoint( info );

            const int rot = GridBlock::Material::Rotate;
            const int flipx = GridBlock::Material::FlipX;
            const int flipy = GridBlock::Material::FlipY;
            
            int f = info.material.flags;
            
            // set all the flags
            info.material.flags |= ( rot | flipx | flipy );
            
            // this kinda sucks, but the way this was done before
            // was corrupting the flags!            
            if( rotRight )
            {
               if( !( f & rot ) )
               {
                  if( !( f & flipx ) )
                     info.material.flags &= ~( flipx | flipy );
               }
               else
               {
                  if( !( f & flipx ) )
                     info.material.flags &= ~( rot );
                  else
                     info.material.flags &= ~( rot | flipx | flipy );
               }
            }
            else
            {
               if( !( f & rot ) )
               {
                  if( f & flipx )
                     info.material.flags &= ~( flipx | flipy );
               }
               else
               {
                  if( !( f & flipx ) )
                     info.material.flags &= ~( rot | flipx | flipy );
                  else
                     info.material.flags &= ~( rot );
               }
            }
            
            // set the info
            setInfo( info );
         }
         
         // flush them textures            
         flushTextures();
         undo.undoFlags = Selection::flushTextures;
         
         break;
   }
}

// set the flags on the selected terrain through the flagVal mask
void SimTed::setFlagsAction( SimTed::BrushAction action, Selection & sel,
   Selection & undo )
{
   flagAction( action, sel, undo, set );
}

// reset the flags on the selected terrain through the flagVal mask
void SimTed::clearFlagsAction( SimTed::BrushAction action, Selection & sel,
   Selection & undo )
{
   flagAction( action, sel, undo, clear );
}

// carry out the particular flag operation
void SimTed::flagAction( SimTed::BrushAction action, Selection & sel, 
   Selection & undo, SimTed::FlagOps flagOp )
{
   unsigned int i;
   
   switch( action )
   {
      case begin:
      case update:
      case end:

         // go through the selection   
         for( i = 0; i < sel.points.getSize(); i++ )
         {
            // get the info at this position
            Selection::Info info;
            
            if( !getInfo( sel.points[i].pos, info ) )
               continue;
               
            // add to the undo
            undo.addPoint( info );

            // adjust the flags
            switch( flagOp )
            {
               case clear:
                  info.material.flags &= ~( flagVal );
                  break;
                  
               case set:
                  info.material.flags |= flagVal;
                  break;
            }
            
            // set the info
            setInfo( info );            
            
         }

         // flush them textures            
         flushTextures();
         undo.undoFlags = Selection::flushTextures;
         
         break;
   }
}

// raise the height of the selection by adjustVal amount
void SimTed::raiseHeightAction( SimTed::BrushAction action, Selection & sel,
   Selection & undo )
{
   unsigned int i;
   
   switch( action )
   {
      case begin:
      case update:
      
         for( i = 0; i < sel.points.getSize(); i++ )
         {
            float height = getHeight( sel.points[i].pos ) + adjustVal;
            
            // add to the undo before changing
            Selection::Info info;
            
            if( getInfo( sel.points[i].pos, info ) )
            {
               undo.addPoint( info );
            
               setHeight( sel.points[i].pos, height );
            }
         }

         // check for feathering
         if( feather )
            featherAction( action, sel, undo, true, adjustVal );
            
         break;
         
      case end:
         getGridFile()->updateHeightRange();
         break;
   }
}

// lower the height of the selection by adjustVal amount
void SimTed::lowerHeightAction( SimTed::BrushAction action, Selection & sel,
   Selection & undo )
{
   unsigned int i;
   
   switch( action )
   {
      case begin:
      case update:
      
         for( i = 0; i < sel.points.getSize(); i++ )
         {
            float height = getHeight( sel.points[i].pos ) - adjustVal;
            
            // add to the undo before changing
            Selection::Info info;
            
            if( getInfo( sel.points[i].pos, info ) )
            {
               undo.addPoint( info );
            
               setHeight( sel.points[i].pos, height );
            }
         }

         // check for feathering
         if( feather )
            featherAction( action, sel, undo, true, -adjustVal );
            
         break;
         
      case end:
         getGridFile()->updateHeightRange();
         break;
   }
}

// set the height for the selected terrain
void SimTed::setHeightAction( SimTed::BrushAction action, Selection & sel, Selection & undo )
{
   unsigned int i;
   
   switch( action )
   {
      case begin:
      case update:
      
         // go through and set the heights
         for( i = 0; i < sel.points.getSize(); i++ )
         {
            Selection::Info info;
            
            if( getInfo( sel.points[i].pos, info ) )
            {
               // add to the undo
               undo.addPoint( info );
            
               setHeight( sel.points[i].pos, heightVal );
            }
         }  
         
         // check if we should smooth this
         if( feather )
            featherAction( action, sel, undo, false, heightVal );
                   
         break;
         
      case end:
         getGridFile()->updateHeightRange();
         break;
   }
}

// just force to fit
void SimTed::setSmoothVal( float smooth )
{
   if( smooth < 0.0f )
      smooth = 0.0f;
   if( smooth > 1.0f )
      smooth = 1.0f;
   smoothVal = smooth;
}

// smooth the selected terrain
void SimTed::smoothAction( SimTed::BrushAction action, Selection & sel, 
   Selection & undo )
{
   switch( action )
   {
      case begin:
      case update:
      {
         // how do you smooth a single point?
         if( !getBrushDetail() )
            return;
            
         //get some information about the current selection
         RectI bound;
         Point2F center;
         float radius;
         setSelectionVars( sel, bound, center, radius );

         for( unsigned int i = 0; i < sel.points.getSize(); i++ )
         {
            Point2I pnt = sel.points[i].pos;
            
            float dx = pnt.x - center.x;
            float dy = pnt.y - center.y;
            float distance = sqrt( float( ( dx * dx ) + ( dy * dy ) ) );

            Selection::Info info;
            
            if( !getInfo( pnt, info ) ) 
               continue;

            // add to info
            undo.addPoint( info );
                        
            float oldHeight = info.height.height;
            	
            float sum = 0.0;
            int count = 0;
            if( pointValid( Point2I( pnt.x - 1, pnt.y ) ) )
            {
               sum += getHeight( Point2I( pnt.x - 1, pnt.y ) );
               count++;
            }
            if( pointValid( Point2I( pnt.x + 1, pnt.y ) ) )
            {
               sum += getHeight( Point2I( pnt.x + 1, pnt.y ) );
               count++;
            }
            if( pointValid( Point2I( pnt.x, pnt.y - 1 ) ) )
            {
               sum += getHeight( Point2I( pnt.x, pnt.y - 1 ) );
               count++;
            }
            if( pointValid( Point2I( pnt.x, pnt.y + 1) ) )
            {
               sum += getHeight( Point2I( pnt.x, pnt.y + 1 ) );
               count++;
            }
            sum /= count;

            float increment = ( sum - oldHeight ) / 
               ( ( 10 - ( smoothVal * 9.999f ) ) );

            float adjust = increment - ( ( increment / 2 ) *
               distance / radius );
               
            if( fabs( adjust ) > fabs( sum - oldHeight ) )
               adjust = ( sum - oldHeight );
                              
            oldHeight += adjust;
               
            setHeight( pnt, oldHeight);
         }
         break;
      }
      
      case end:
         getGridFile()->updateHeightRange();
         break;
   }
}


// add noise to terrain
void SimTed::noiseAction( SimTed::BrushAction action, Selection & sel,
   Selection & undo )
{
   action;sel;undo;
}

// adjust the terrain around the selected terrain to create smooth
// transition for the heights
void SimTed::featherAction( SimTed::BrushAction action, Selection & sel,
   Selection & undo, bool raise, float height )
{
   switch( action )
   {
      case begin:
      case update:
      {
         Selection touched;
         unsigned int i;
           
         // check at which detail we should feather things
         if( !brushDetail || !snap )
         {
            for( i = 0; i < sel.points.getSize(); i++ )
            {
               int j, k;
               
               // walk around this point and add the ones touching
               for( j = -1; j <= 1; j++ )
               {
                  for( k = -1; k <= 1; k++ )
                  {
                     if( j || k )
                     {
                        Point2I pnt;
                        
                        pnt.x = sel.points[i].pos.x + k;
                        pnt.y = sel.points[i].pos.y + j;

                        if( pointValid( pnt ) && !touched.pointInSel( pnt ) &&
                           !sel.pointInSel( pnt ) )
                           touched.addPoint( pnt );
                     }
                  }
               }
            }
            
            Vector < float > heights;
            
            // go through all the points in the touched list and get an average height
            for( i = 0; i < touched.points.getSize(); i++ )
            {
               int j, k;
               int count = 0;
               float sum = 0.0;
               
               // walk around this point and sum the surrounding ones
               for( j = -1; j <= 1; j++ )
               {
                  for( k = -1; k <= 1; k++ )
                  {
                     Point2I pnt;
                     
                     pnt.x = touched.points[i].pos.x + k;
                     pnt.y = touched.points[i].pos.y + j;
                     
                     // add to the sum
                     if( pointValid( pnt ) )
                     {
                        count++;
                        sum += getHeight( pnt );
                     }
                  }
               }
               
               float height = sum / count;
               
               heights.push_back( height );
            }

            // now set these heights            
            for( i = 0; i < touched.points.getSize(); i++ )
            {
               Selection::Info info;
               
               if( !getInfo( touched.points[i].pos, info ) )
                  continue;
                  
               // add to the undo
               undo.addPoint( info );
               
               setHeight( touched.points[ i ].pos, heights[ i ] );
            }
            
            return;
         }

         for( int n = -1; n <= 1; n++ )
         {
            for( int m = -1; m <= 1; m++ )
            {
               // skip this brush
               if( !m && !n )
                  continue;
                  
               // get the position
               Point2I pos;
               pos.x = brushPos.x + m * ( 1 << brushDetail );
               pos.y = brushPos.y + n * ( 1 << brushDetail );

               // check that the brush exists
               if( !pointValid( pos ) )
                  continue;
                                 
               // check for raising an edge or a corner
               if( !m || !n )
               {
                  SimTed::Side side;
                  
                  if( !m )
                     side = ( n == -1 ) ? bottom : top;
                  else
                     side = ( m == -1 ) ? right : left;

                  // adjust the edge                     
                  raiseEdgeHeight( pos, undo, side, height, raise );
               }
               else
               {
                  bool top = true;
                  bool left = true;
                  
                  // figure out which corners to adjust to/from
                  if( m == -1 )
                     left = false;
                  if( n == -1 )
                     top = false;
                     
                  // raise a corner
                  Point2I src;
                  src.x = left ? brushPos.x + ( 1 << brushDetail ) - 1 :
                     brushPos.x;
                  src.y = left ? brushPos.y + ( 1 << brushDetail ) - 1 :
                     brushPos.y;

                  if( !pointValid( src ) )
                     continue;
                                    
                  float height = getHeight( src );
                  adjustControlPoint( pos, height, undo, top, left, true );
               }
            }
         }
         
         break;
      }
      
      case end:
         getGridFile()->updateHeightRange();
         break;
   }
}

// set the height for an edge of the terrain
void SimTed::raiseEdgeHeight( const Point2I brush, Selection & undo, SimTed::Side side, float height, bool raise )
{
   int brushDim = ( 1 << brushDetail );
   
   // go along the edge
   for( int i = 0; i < brushDim; i++ )
   {
      Point2I destPnt;
      
      // look at side
      switch( side )
      {
         case right:
            destPnt.x = brush.x + brushDim - 1;
            destPnt.y = brush.y + i;
            break;
         case bottom:
            destPnt.x = brush.x + i;
            destPnt.y = brush.y + brushDim - 1;
            break;
         case left:
            destPnt.x = brush.x;
            destPnt.y = brush.y + i;
            break;
         case top:
            destPnt.x = brush.x + i;
            destPnt.y = brush.y;
            break;
      }

      // get the amount we need to adjust to
      float heightStep;
      
      if( raise )
         heightStep = height / ( float )( brushDim - 1 );
      else
         heightStep = ( height - getHeight( destPnt ) ) / ( float )brushDim;
            
      for( int j = 0; j < brushDim; j++ )
      {
         Point2I currentPnt;

         // look at side
         switch( side )
         {
            case right:
               currentPnt.x = brush.x + j;
               currentPnt.y = destPnt.y;
               break;
            case bottom:
               currentPnt.x = destPnt.x;
               currentPnt.y = brush.y + j;
               break;
            case left:
               currentPnt.x = brush.x + brushDim - j - 1;
               currentPnt.y = destPnt.y;
               break;
            case top:
               currentPnt.x = destPnt.x;
               currentPnt.y = brush.y + brushDim - j - 1;
               break;
         }

         Selection::Info info;
      
         if( !getInfo( currentPnt, info ) )
            continue;
            
         undo.addPoint( info );
         
         setHeight( currentPnt, info.height.height + heightStep * j );
      }
   }
}


// adjust the corner of a brush
void SimTed::setBrushCornerHeight( const Point2I pnt, Selection & undo, 
   float height, bool setUndo )
{
   int brushDim = ( 1 << getBrushDetail() );

   // go through them
   for( int i = -1; i <= 0; i++ )
   {
      for( int j = -1; j <= 0; j++ )
      {
         bool top = true;
         bool left = true;
         
         Point2I pos( pnt.x + j * brushDim, pnt.y + i * brushDim );
         
         // check that there is a brush here
         if( !pointValid( pos ) )
            continue;
            
         // get the corner pos and directions
         if( j == -1 )
            left = false;
         if( i == -1 )
            top = false;

         // adjust this corner        
         adjustControlPoint( pos, height, undo, top, left, setUndo );
      }
   }
}


// adjust the height value for a corner of a brush selection
// assumes that a brush really exists at this coordinate
void SimTed::adjustControlPoint( const Point2I brush, float height, Selection & undo,
   bool top, bool left, bool setUndo )
{
   int brushHeight, brushWidth;
   
   Point2I control;
   Point2F endPnt;
   
   // get the brush dimensions
   brushHeight = brushWidth = ( 1 << brushDetail );
   
   // get the control point
   control.y = top ? 0 : ( brushHeight - 1 );
   control.x = left ? 0 : ( brushWidth - 1 );

   // get the height for this control point
   Point2I pnt( control.x + brush.x, control.y + brush.y );
   float controlHeight = getHeight( pnt );
      
   for( int y = 0; y < brushHeight; y++ )
   {
      for( int x = 0; x < brushWidth; x++ )
      {
         Point2I pos( x, y );  
         
         if( pos == control )
         {
            pos.x += brush.x;
            pos.y += brush.y;

            // add to the undo
            if( setUndo )
            {
               Selection::Info info;
               
               if( getInfo( pos, info ) )
                  undo.addPoint( info );
            }
            
            setHeight( pos, height );
            continue;
         }
         else if( pos.x == control.x ) 
         {
            endPnt.x = ( float )control.x;
            endPnt.y = ( float )( top ? ( brushHeight - 1 ) : 0 );
         }
         else if( pos.y == control.y )
         {
            endPnt.x = ( float )( left ? ( brushWidth - 1 ) : 0 );
            endPnt.y = ( float )control.y;
         }
         else
         { 
            // get the slope
            double slope;
            
            slope = fabs( float( pos.y - control.y ) ) / fabs( float( pos.x - control.x ) );
            
            if( slope <= 1.0 )
            {
               endPnt.x = ( float )( left ? ( brushWidth - 1 ) : 0 );
               endPnt.y = ( float )( top ? ( slope * float( brushHeight - 1 ) ) :
                  float( ( brushHeight - 1 ) ) - ( slope * float( brushHeight - 1 ) ) );
            }
            else
            {
               endPnt.x = ( float )( left ? ( float( brushWidth - 1 ) / slope ) :
                  float( brushWidth - 1 ) - ( float( brushWidth - 1 ) / slope ) );
               endPnt.y = ( float )( top ? ( brushHeight - 1 ) : 0 );
            }
         }
         
         double targetD;
         double endD;
         
         // get the distances
         targetD = sqrt( ( control.x - x ) * ( control.x - x ) +
            ( control.y - y ) * ( control.y - y ) );
         endD = sqrt( ( endPnt.x - control.x ) * ( endPnt.x - control.x ) +
            ( endPnt.y - control.y ) * ( endPnt.y - control.y ) );
            
         // adjust the coords
         pos.x += brush.x;
         pos.y += brush.y;

         float stepInc = ( height - controlHeight ) / endD;

         float posHeight = getHeight( pos );
         
         posHeight += ( height - controlHeight ) - ( stepInc * targetD );
         
         // add to the undo
         if( setUndo )
         {
            Selection::Info info;
            
            if( getInfo( pos, info ) )
               undo.addPoint( info );
         }
            
         setHeight( pos, posHeight );
      }
   }
}

// adds the selection to the current selection ( allows for 
// mass selection with the shift key )
void SimTed::selectAction( SimTed::BrushAction action, Selection & sel, Selection & undo )
{
   undo;
   
   unsigned int i;
   
   switch( action )
   {
      case begin:
      
         // check for ctrl button
   	   if( GetAsyncKeyState( VK_SHIFT ) & 0x8000 )
         {
            // clear out the highlight selection
            hilightSel.clear();
            regionSelect = true;
            selectCanceled = false;
            selectStartPnt = brushPos;
         }  
         else
         {
            regionSelect = false;
            for( i = 0; i < sel.points.getSize(); i++ )
               currentSel.addPoint( sel.points[i].pos );
         }
            
         break;
         
      case update:

         if( regionSelect && !selectCanceled )
         {
            hilightSel.clear();
            
            if( GetAsyncKeyState( VK_ESCAPE ) & 0x8000 )
            {
               selectCanceled = true;
               return;               
            }
            
            Point2I tlPnt;
            Point2I dim;
            
            int length = 1 << brushDetail;
            
            // get the coords for the box
            tlPnt.x = min( selectStartPnt.x, brushPos.x );
            tlPnt.y = min( selectStartPnt.y, brushPos.y );
            dim.x = max( selectStartPnt.x + length, brushPos.x + length ) - tlPnt.x;
            dim.y = max( selectStartPnt.y + length, brushPos.y + length ) - tlPnt.y;
            
            int i, j;
            
            // fill the region select with the info
            for( i = 0; i < dim.y; i++ )
               for( j = 0; j < dim.x; j++ )
               {
                  Point2I pnt( tlPnt.x + j, tlPnt.y + i );
                  if( pointValid( pnt ) )
                     hilightSel.addPoint( pnt );
               }
         }
         else
         {
            for( i = 0; i < sel.points.getSize(); i++ )
               currentSel.addPoint( sel.points[i].pos );
         }
         
         break;
         
      case end:
      
         if( regionSelect )
         {
            for( i = 0; i < hilightSel.points.getSize(); i++ )
               currentSel.addPoint( hilightSel.points[i].pos );
               
            hilightSel.clear();
         }
      
         break;
   }

   // clusterize the stuff
   if( selectShow.test( Outline ) )
      clusterizeSelection( currentClusterSel, currentSel );
   if( hilightShow.test( Outline ) && regionSelect )
      clusterizeSelection( hilightClusterSel, hilightSel );
}

// gets the average height for the selection
void SimTed::getAvgHeightAction( SimTed::BrushAction action, Selection & sel,
   Selection & undo )
{
   undo;
   
   switch( action )
   {
      case begin:
      case update:
      
         unsigned int i;
         int numPoints = sel.points.getSize();
         float sum = 0.0;
         
         for( i = 0; i < sel.points.getSize(); i++ )
            sum += getHeight( sel.points[i].pos );
            
         avgHeight = sum / numPoints;
         
         break;
   }
}


// remove the selected blocks from the current selection
void SimTed::deselectAction( SimTed::BrushAction action, Selection & sel,
   Selection & undo )
{
   undo;
   
   unsigned int i;
   
   switch( action )
   {
      case begin:
      
         // check for ctrl button
   	   if( GetAsyncKeyState( VK_SHIFT ) & 0x8000 )
         {
            // clear out the highlight selection
            hilightSel.clear();
            regionSelect = true;
            selectCanceled = false;
            selectStartPnt = brushPos;
         }  
         else
         {
            regionSelect = false;
            for( i = 0; i < sel.points.getSize(); i++ )
               currentSel.removePoint( sel.points[i].pos );
         }
            
         break;
         
      case update:

         if( regionSelect && !selectCanceled )
         {
            hilightSel.clear();
            
            if( GetAsyncKeyState( VK_ESCAPE ) & 0x8000 )
            {
               selectCanceled = true;
               return;               
            }
            
            Point2I tlPnt;
            Point2I dim;
            
            int length = 1 << brushDetail;
            
            // get the coords for the box
            tlPnt.x = min( selectStartPnt.x, brushPos.x );
            tlPnt.y = min( selectStartPnt.y, brushPos.y );
            dim.x = max( selectStartPnt.x + length, brushPos.x + length ) - tlPnt.x;
            dim.y = max( selectStartPnt.y + length, brushPos.y + length ) - tlPnt.y;
            
            int i, j;
            
            // fill the region select with the info
            for( i = 0; i < dim.y; i++ )
               for( j = 0; j < dim.x; j++ )
               {
                  Point2I pnt( tlPnt.x + j, tlPnt.y + i );
                  if( pointValid( pnt ) )
                     hilightSel.addPoint( pnt );
               }
         }
         else
         {
            for( i = 0; i < sel.points.getSize(); i++ )
               currentSel.removePoint( sel.points[i].pos );
         }
         
         break;
         
      case end:
      
         if( regionSelect )
         {
            for( i = 0; i < hilightSel.points.getSize(); i++ )
               currentSel.removePoint( hilightSel.points[i].pos );
               
            hilightSel.clear();
         }
      
         break;
   }
   
   // clusterize the stuff
   if( selectShow.test( Outline ) )
      clusterizeSelection( currentClusterSel, currentSel );
   if( hilightShow.test( Outline ) && regionSelect )
      clusterizeSelection( hilightClusterSel, hilightSel );
}

// add an action to the internal table
void SimTed::addAction( const char * name, void ( SimTed::*callback )
   ( SimTed::BrushAction, Selection & sel, Selection & undo ), bool mouseAction,
   const char * description )
{
   TedAction * newAction = new TedAction;
   
   newAction->name = name;
   newAction->callback = callback;
   newAction->mouseAction = mouseAction;
   newAction->description = description;
   
   // add to the vector
   actions.push_back( newAction );
}

// call an action routine
bool SimTed::processAction( const char * name )
{
   int actionIndex = getActionIndex( name );
   
   // check that it got a good action
   if( actionIndex == -1 )
      return( false );
      
   // check if there is something selected
   if( !currentSel.points.getSize() )
      return( true );
   
   // create an undo selection
   Selection * undo = new Selection;
   
   // save the brush detail size
   int detail = getBrushDetail();
   
   // go into detail mode 0 for this
   setBrushDetail( 0 );
   
   // call the callback twice ( begin then end )
   ( this->*actions[actionIndex]->callback )( begin, currentSel, *undo );
   ( this->*actions[actionIndex]->callback )( end, currentSel, *undo );
   
   // reset the detail mode
   setBrushDetail( detail );
   
   // do the undo stuff
   if( undo->points.getSize() )
   {
      // add to the undo stack and remove the redo stack
      undoStack.push( undo );
      redoStack.clear();
   }
   else
      delete undo;

   dataChanged = true;
   
   return( true );
}

void SimTed::floatCurrent()
{
   // fill with info ( just has positions )
   fillInfo( currentSel );
   
   // float it
   floatSelection( currentSel );
   floatPasteVal = pasteVal;
   currentSel.clear();
}

void SimTed::floatSelection( Selection & sel )
{
   if( selFloating || !sel.points.getSize() )
      return;
   
   // grab the first position, rest are offset from this
   Point2I origin = sel.points[0].pos;
   
   // clear the copy selection
   copySel.clear();
   
   // add all the points as an offset to the first point from the sel
   for( unsigned int i = 0; i < sel.points.getSize(); i++ )
   {
      Point2I pos( sel.points[i].pos.x - origin.x,
         sel.points[i].pos.y - origin.y );

      Selection::Info info;
      
      info = sel.points[i];
      info.pos = pos;
      
      copySel.addPoint( info );
   }
   
   // clusterize if wanted to   
   clusterizeSelection( copyClusterSel, copySel );
   
   selFloating = true;
}

void SimTed::setBrushPos( const Point2I & pos )
{
   brushPos = pos;
   
   // check for snap
   if( snap )
   {
      brushPos.x >>= brushDetail;
      brushPos.x <<= brushDetail;
      brushPos.y >>= brushDetail;
      brushPos.y <<= brushDetail;
   }
}

void SimTed::mouseAction( int button, SimTed::BrushAction action )
{
   Point2I hitPnt;

   cursorPos = getCanvas()->getCursorPos();
   cursorDif = cursorPos - lastCursorPos;
   lastCursorPos = cursorPos;
   
   if( action == begin )
      cursorDif.set( 0, 0 );
      
   setVars();
      
   if( !getInterceptCoord( hitPnt ) && ( action != end ) )
   {
      selectLastPnt.x = -1;
      selectLastPnt.y = -1;
      return;
   }
   
   // set the brush position ( will snap here if requested )
   setBrushPos( hitPnt );

   // check which button was pressed   
   if(!button)
   {
      // make sure the other button is not pressed
      if( ::GetAsyncKeyState( ( ::GetSystemMetrics( SM_SWAPBUTTON ) ) ?
         VK_LBUTTON : VK_RBUTTON ) & 0x8000 )
         return;

      if(action == begin)         
      {
         // protect against multiple begins (can happen when
         // lost the mouse and the mouse up event)..
         if(mouseProcessAction != -1)
            return;
         mouseProcessAction = mouseLButtonAction;
      }
   }
   else
   {
      // make sure the other button is not pressed
      if( ::GetAsyncKeyState( ( ::GetSystemMetrics( SM_SWAPBUTTON ) ) ?
         VK_RBUTTON : VK_LBUTTON ) & 0x8000 )
         return;

      if(action == begin)
      {
         if(mouseProcessAction != -1)
            return;
         mouseProcessAction = mouseRButtonAction;
      }
   }
   
   // check if floating
   if( selFloating )
   {
      // check for cancel
      if( button )
      {
         selFloating = false;
         return;
      }
      else if(action == begin)
         mouseProcessAction = getActionIndex( "paste" );
   }     
   
   // only do the comman on new position
   if( ( brushPos == selectLastPnt ) && ( action == update ) && !grabMode )
      return;
   
   // update the last mouse position so that we do not apply the same action
   selectLastPnt = brushPos;
   
   // check if cool
   if( ( mouseProcessAction != -1 ) && ( actions.size() > mouseProcessAction ) &&
      ( actions[mouseProcessAction]->callback != 0 ) )
   {
      Selection sel;
      
      fillSelection( sel );
      
      // create a new undo selection that spans the entire command
      if( ( action == begin ) && currentUndo )
      {
         delete currentUndo;
         currentUndo = NULL;
      }
         
      // create a new selection
      if( !currentUndo )
         currentUndo = new Selection;
         
      // call the callback
      ( this->*actions[mouseProcessAction]->callback )( action, sel, *currentUndo );
      
      // do the undo stuff
      if( action == end )
      {      
         // add to the undo if points were actually changed
         if( currentUndo->points.getSize() )
         {
            // add to the undo but flush the redo
            undoStack.push( currentUndo );
            redoStack.clear();
         }
         else 
            delete currentUndo;

         // reset the ptr ( the stack will deal with it )
         currentUndo = NULL;
         
         dataChanged = true;
   
         // update the toolbar
         Console->evaluate( "Ted::updateToolBar();", false );
         mouseProcessAction = -1;
      }
   }
}

void SimTed::fillSelection( Selection & sel )
{
   int i,j;
   
   int length = 1 << brushDetail;
   
   // add 2^detailLevelx2^detailLevel points
   for( i = 0; i < length; i++ )
      for( j = 0; j < length; j++ )
      {
         Point2I pnt( brushPos.x + j, brushPos.y + i );

         if( pointValid( pnt ) )
            sel.addPoint( pnt );
      }
}

// apply an action to the entire terrain block.. only
// works with oneBlockmapstoall.... 
bool SimTed::terrainAction(const char * action)
{
   if(getGridFile()->getBlockPattern() != GridFile::OneBlockMapsToAll)
      return(false);

   // get the width of a blcok
   int n = 1 << (getMaxBrushDetail() - 1);

   int actionIndex = getActionIndex(action);
   if(actionIndex == -1)
      return(false);
      
   // create a selection of the entire terrain
   Selection sel;
   sel.createMap(n,n);
   for(int i = 0; i < n; i++)
      for(int j = 0; j < n; j++)
         sel.addPoint(Point2I(i,j));
         
   // create an undo selection
   Selection * undo = new Selection;
   undo->createMap(n,n);

   // call the function
   int detail = getBrushDetail();
   setBrushDetail(0);
   ( this->*actions[actionIndex]->callback )( begin, sel, *undo );
   ( this->*actions[actionIndex]->callback )( end, sel, *undo );
   setBrushDetail(detail);
      
   undo->deleteMap();
   
   // add to the undo stack and remove the redo stack
   undoStack.push( undo );
   redoStack.clear();
   dataChanged = true;
   return(true);
}

// this will mirror the terrain according the flags... only
// mirrors the center block of a oneblockmapstoall pattern
bool SimTed::mirrorTerrain(UInt32 sides)
{
   if(getGridFile()->getBlockPattern() != GridFile::OneBlockMapsToAll)
      return(false);

   // check the sides
   if((sides & top) && (sides & bottom))
      return(false);
   if((sides & left) && (sides & right))
      return(false);

   // get the width of a blcok
   int n = 1 << (getMaxBrushDetail() - 1);

   Point2I src((sides & right) ? (n - 1) : 0, (sides & bottom) ? (n - 1) : 0);
   Point2I dest((sides & left) ? (n - 1) : 0, (sides & top) ? (n - 1) : 0);
   Point2I origSrc(src);
   Point2I origDest(dest);

   // check if diag...
   bool diag = ((sides & right)||(sides & left)) && ((sides & top)||(sides & bottom));
   
   // determine the run length
   int minStride = ((sides & top) || (sides & bottom)) ? n : n / 2;
   int majStride = ((sides & left) || (sides & right)) ? n : n / 2;
   
   Point2I srcStep((sides & right) ? -1 : 1, (sides & bottom) ? -1 : 1);
   Point2I destStep((sides & left) ? -1 : 1, (sides & top) ? -1 : 1);
   
   // create an undo stack for this operation
   Selection * undo = new Selection;
   
   // walk through all the positions
   for(int i = 0; i < majStride; i++)
   {
      for(int j = 0; j < minStride; j++)
      {
         // skip the same position
         if(src != dest)
         {   
            // set only the height...
            SimTed::Selection::Info info;
            if(getInfo(dest, info))
            {
               undo->addPoint(info);
               float height = getHeight(src);
               setHeight(dest, height);
            }
         }
         
         // get to the new position
         src.x += srcStep.x;
         diag ? (dest.y += destStep.y) : (dest.x += destStep.x);
      }
      
      // get the next position for a run
      src.y += srcStep.y;
      diag ? (dest.x += destStep.x) : (dest.y += destStep.y);

      // reset the minor run
      src.x = origSrc.x;
      diag ? (dest.y = origDest.y) : (dest.x = origDest.x);
               
      // shorten the run length for diag runs   
      if(diag)
         minStride--;
   }
   
   // add to the undo stack
   undoStack.push(undo);
   redoStack.clear();
   dataChanged = true;
      
   return(true);
}

bool SimTed::pointValid( const Point2I & pnt )
{
   if( ( pnt.x >= 0 ) && ( pnt.x < ( size.x << blockShift ) ) &&
      ( pnt.y >= 0 ) && ( pnt.y < ( size.y << blockShift ) ) )
      return( true );
   return( false );
}

bool SimTed::onSim3DMouseEvent( const Sim3DMouseEvent * event )
{
   if( !focused )
      return( false );
      
   switch( event->meType )
   {
      case Sim3DMouseEvent::MouseUp:
      case Sim3DMouseEvent::RightMouseUp:

         AssertFatal( mouseHandler, "SimTed::processEvent:: mouse handler not set" );

         // send off to the tedplugin
         mouseHandler->mouseCallback( true, ( event->meType == Sim3DMouseEvent::RightMouseDown ) );
         mouseAction( ( event->meType == Sim3DMouseEvent::RightMouseUp ), end );
         
         break;
         
      case Sim3DMouseEvent::RightMouseDown:
      case Sim3DMouseEvent::MouseDown:

         AssertFatal( mouseHandler, "SimTed::processEvent:: mouse handler not set" );

         // send to ted
         mouseHandler->mouseCallback( false, ( event->meType == Sim3DMouseEvent::RightMouseDown ) );
         mouseAction( ( event->meType == Sim3DMouseEvent::RightMouseDown ), begin );

         break;
         
      case Sim3DMouseEvent::MouseDragged: 
      case Sim3DMouseEvent::RightMouseDragged:
      case Sim3DMouseEvent::MouseMove:
      
         // update the current position and start point for the mouse
         mouseStartPt = event->startPt;
         mouseDirection = event->direction;

         // call the drag routine
         if( event->meType != Sim3DMouseEvent::MouseMove )
            mouseAction( ( event->meType == Sim3DMouseEvent::RightMouseDragged ), update );
         else
         {
            Point2I hitPnt;
            setVars();
            if( getInterceptCoord( hitPnt ) )   
            {
               setBrushPos( hitPnt );
               mouseMoved = true;
            }
         }

         break;
   }
   
   return( false );
}

int SimTed::getMaxBrushDetail()
{
   return( getGridFile()->getDetailCount() );
}

void SimTed::setBrushDetail( int detail )
{
   // check that we are not trying to change detail too high
   if( detail > ( getGridFile()->getDetailCount() ) )
      return;
      
   brushDetail = detail;
}

bool SimTed::getInterceptCoord( Point2I & pnt )
{
   TMat3F tmat = getInvTransform();
   LineSeg3F line;
   line.start = mouseStartPt;
   line.end = mouseDirection;
   line.end *= 100000;
   line.end += line.start;

   LineSeg3F tline;
   m_mul( line.start, tmat, &tline.start );
   m_mul( line.end, tmat, &tline.end );
   GridCollision coll( getGridFile(), 0 );
   if( coll.collide( tline.start, tline.end, false, true ) )
   {
		Point3F potenIcept = coll.surface.position;
      setVars();
      Point2I potenIceptI( potenIcept.x,potenIcept.y );
      potenIceptI >>= scaleShift;
      if ( ( potenIceptI.x >= 0 ) && ( potenIceptI.x < ( size.x << blockShift ) ) &&
         ( potenIceptI.y >= 0 ) && ( potenIceptI.y < ( size.y << blockShift ) )  )
      {
         pnt = potenIceptI;
         return( true );
      }
   } 
   
   return( false );
}
                 
bool SimTed::processEvent( const SimEvent * event )
{
   switch( event->type )
   {
      onEvent( Sim3DMouseEvent );
      
      case SimGainFocusEventType:
         focused = TRUE;
         break;
         
      case SimLoseFocusEventType:
         focused = FALSE;
         break;
         
      // let the parent class handle this event
      default:
         return( SimObject::processEvent( event ) );
   }
   return( true );
}

bool SimTed::processQuery( SimQuery* query )
{
   switch( query->type )
   {
      case SimRenderQueryImageType:
      {      
         if( !focused )
            return( false );
            
         // setup for an overlay render
         SimRenderQueryImage * queryImage = 
            static_cast< SimRenderQueryImage* >( query );
         
         queryImage->count = 1;
         queryImage->image[0] = this;
         itype = SimRenderImage::Overlay;
         
         break;
      }
        
      case SimInputPriorityQueryType:
      
         // set slightly higher than normal priority
         ( ( SimInputPriorityQuery * )query )->priority = SI_PRIORITY_NORMAL + 2;
         
         break;         
         
      default:
         return( SimObject::processQuery( query ) );
   }
   
   return( true );
}

bool SimTed::onAdd()
{
   if( !SimObject::onAdd() )
      return( false );
   
   addToSet( SimInputConsumerSetId );
	addToSet( SimRenderSetId );
   
   return( true );
}

void SimTed::onRemove()
{
	removeFromSet( SimInputConsumerSetId );
	removeFromSet( SimRenderSetId );
	focused = false;

   SimObject::onRemove();
}

void SimTed::render( TSRenderContext &rc )
{     
   setVars();
   
   rc.getCamera()->pushTransform( getTransform() ); 
   
   surface = rc.getSurface();
   surface->setHazeSource(GFX_HAZE_NONE);
   surface->setShadeSource(GFX_SHADE_NONE);
   surface->setAlphaSource(GFX_ALPHA_NONE);
   surface->setFillMode(GFX_FILL_CONSTANT);
   surface->setTexturePerspective(FALSE);
   surface->setTransparency(FALSE);
   
   pointArray = rc.getPointArray();  
   pointArray->reset();                              
   pointArray->useIntensities(false);                
   pointArray->useTextures(false);                   
   pointArray->useHazes(false);                      
   pointArray->useBackFaceTest(false);               
   pointArray->setVisibility( TS::ClipMask );        

   unsigned int i;
   int fillCol, frameCol;

   Selection * sel;

   // turn off flipping of colors
   flipColors = false;

   // draw our stuff
   if( selectShow.test( Outline ) )
   {
      sel = &currentClusterSel;

      // check if should clear
      if( currentSel.points.getSize() < currentClusterSel.points.getSize() )
         currentClusterSel.clear();
               
      // check if should fill
      if( currentSel.points.getSize() && !currentClusterSel.points.getSize() )
         clusterizeSelection( currentClusterSel, currentSel );
   }
   else
   {
      sel = &currentSel;
      currentClusterSel.clear();
   }
   
   // set the colors 
   fillCol = flipColors ? selectFrameColor : selectFillColor;
   frameCol = flipColors ? selectFillColor : selectFrameColor;
   
   for( i = 0; i < sel->points.getSize(); i++ )
   {
      if( selectShow.test( Fill ) )
         drawSquareFilled( sel->points[i].pos, rc, fillCol );
      if( selectShow.test( Frame ) )
         drawSquare( sel->points[i].pos, rc, frameCol );
   }
   
   // do the hilight now
   if( hilightShow.test( Outline ) )
      sel = &hilightClusterSel;
   else
      sel = &hilightSel;

   // set the colors 
   fillCol = flipColors ? hilightFrameColor : hilightFillColor;
   frameCol = flipColors ? hilightFillColor : hilightFrameColor;
   
   for( i = 0; i < sel->points.getSize(); i++ )
   {
      if( hilightShow.test( Fill ) )
         drawSquareFilled( sel->points[i].pos, rc, fillCol );
      if( hilightShow.test( Frame ))
         drawSquare( sel->points[i].pos, rc, frameCol );
   }

   // check for grab mode
   drawMouseShadow( rc );
   
   // check if drawing the frame around the current block
   if( blockOutline )
   {
      // grab the first block - just need to get dimensions
      Point2I size( 0, 0 );
      GridBlock * block = getGridFile()->getBlock( size );

      // get the size
      size = block->getSize();      

      Point2I begin, end;
      
      // set the start point
      begin.x = brushPos.x & ~( size.x - 1 );
      begin.y = brushPos.y & ~( size.y - 1 );
      
      // set the end point
      end.x = begin.x + size.x;
      end.y = begin.y + size.y;
      
      drawSquareOutline( begin, end, rc, blockFrameColor );
   }
   
   rc.getCamera()->popTransform();
}

void SimTed::drawSquareOutline( Point2I & begin, Point2I & end, TSRenderContext & rc, UInt8 col )
{
   int width, height;
   
   width = end.x - begin.x;
   height = end.y - begin.y;
   
   surface->setFillColor( col );

	Point2F p1, p2;
   float scale = ( float )(1 << scaleShift);
	
   // draw the top and bottom
   for( int x = 0; x < ( width - 1 ); x++ )
   {
      // fill in the point info
      p1.x = begin.x + x;
      p2.x = p1.x + 1;
      p1.y = p2.y = begin.y;
      
      p1 *= scale;
      p2 *= scale;
      drawLine( p1, p2, rc, col );

      // do again for fill in the point info
      p1.x = begin.x + x;
      p2.x = p1.x + 1;
      p1.y = p2.y = end.y;
      
      p1 *= scale;
      p2 *= scale;
      drawLine( p1, p2, rc, col );
   }
   
   // draw the sides
   for( int y = 0; y < height; y++ )
   {
      // fill in the point info for the left side
      p1.x = p2.x = begin.x;
      p1.y = begin.y + y;
      p2.y = p1.y + 1;
      
      p1 *= scale;
      p2 *= scale;
      drawLine( p1, p2, rc, col );

      // fill in the point info for the right side
      p1.x = p2.x = end.x;
      p1.y = begin.y + y;
      p2.y = p1.y + 1;
      
      p1 *= scale;
      p2 *= scale;
      drawLine( p1, p2, rc, col );
   }
}

// draw the mouse shadow, can change shapes depending on mode
void SimTed::drawMouseShadow( TSRenderContext & rc )
{
   // check if floating the selection
   if( selFloating )
   {
      int fillCol = flipColors ? shadowFrameColor : shadowFillColor;
      int frameCol = flipColors ? shadowFillColor : shadowFrameColor;
      
      Selection * sel;
      
      if( shadowShow.test(Outline) )
         sel = &copyClusterSel;
      else
         sel = &copySel;
      
      // go through and draw the squares
      for( unsigned i = 0; i < sel->points.getSize(); i++ )
      {
         Point2I pos( sel->points[i].pos.x + brushPos.x,
            sel->points[i].pos.y + brushPos.y );
            
         // make sure a valid point
         if( !pointValid( pos ) )
            continue;
            
         if( shadowShow.test( Fill ) )
            drawSquareFilled( pos, rc, fillCol );
         if( shadowShow.test( Frame ) )
            drawSquare( pos, rc, frameCol );
      }
      
      return;
   }
   
   // create a shadow selection
   Selection brushSel;
   
   if( grabMode )
   {
      int brushDim = ( 1 << brushDetail );
      
      // set the current brush position
      brushPos = grabPoint;
      
      // add all the brush edges only
      addBrushEdgePoints( brushPos, brushSel );

      if(drawNeighbors)
      {
         // walk around and add the surrounding brushes      
         Point2I pos( brushPos.x - brushDim, brushPos.y );

         // add it
         if( pointValid( pos ) )
            addBrushEdgePoints( pos, brushSel );
         
         pos.y = brushPos.y - brushDim;
      
         // add it
         if( pointValid( pos ) )
            addBrushEdgePoints( pos, brushSel );
      
         pos.x = brushPos.x;
      
         // add it
         if( pointValid( pos ) )
            addBrushEdgePoints( pos, brushSel );
      }
   }
   else
   {
      fillSelection( brushSel );
      
      // check if the mouse has moved
      if( mouseMoved )
      {   
         // get the avg height for the brush ( undo param is not used )
         getAvgHeightAction( begin, brushSel, brushSel );

         Point2I center;
         
         // get the center location
         getBrushCenter( center, brushPos );
         
         // write the info in the status bar
         char buffer[ 128 ];
         sprintf( buffer, "Ted::setStatusText(2,\"Center - ( %d, %d )  AvgHeight: %.3f\");",
            center.x, center.y, avgHeight );
         Console->evaluate( buffer, false );
      }
         
      // check for outline mode
      if( shadowShow.test( Outline ) )
      {
         brushSel.clear();
         addBrushEdgePoints( brushPos, brushSel );
      }
   }
   
   int fillCol = flipColors ? shadowFrameColor : shadowFillColor;
   int frameCol = flipColors ? shadowFillColor : shadowFrameColor;
   
   // go through and draw the squares
   for( unsigned i = 0; i < brushSel.points.getSize(); i++ )
   {
      if( shadowShow.test(Fill) )
         drawSquareFilled( brushSel.points[i].pos, rc, fillCol );
      if( shadowShow.test(Frame) )
         drawSquare( brushSel.points[i].pos, rc, frameCol );
   }
   
   // draw the corner piece inverted
   if( getSnap() )
   {
      if( shadowShow.test( Fill ) )
         drawSquareFilled( brushPos, rc, shadowFrameColor );
      if( shadowShow.test(Frame) )
         drawSquare( brushPos, rc, shadowFillColor );
   }

   // check if the mouse has moved
   if( mouseMoved )
   {   
      // get the info for the brush position
      Selection::Info info;
   
      if( getInfo( brushPos, info ) )
      {
         char buffer[ 128 ];
         sprintf( buffer, "Ted::setStatusText(1,\"Corner - ( %d, %d )  Height: %.3f\");",
            brushPos.x, brushPos.y, info.height.height );
         Console->evaluate( buffer, false );
      }
   }
   
   // reset the mouse moved flag
   mouseMoved = false;
}

void SimTed::getBrushCenter( Point2I & center, Point2I & origin )
{
   int brushDim = ( 1 << brushDetail );
   center.x = ( origin.x * 2 + brushDim ) / 2;
   center.y = ( origin.y * 2 + brushDim ) / 2;
}

void SimTed::clusterizeSelection( Selection & dest, Selection & src )
{
   dest.clear();
   
   for( unsigned int i = 0; i < src.points.getSize(); i++ )
   {
      Point2I pos = src.points[i].pos;
      
      // check the corners
      if( !src.pointInSel( Point2I( pos.x - 1, pos.y ) ) ||
         !src.pointInSel( Point2I( pos.x, pos.y - 1 ) ) ||
         !src.pointInSel( Point2I( pos.x + 1, pos.y ) ) ||
         !src.pointInSel( Point2I( pos.x, pos.y + 1) ) )
         dest.addPoint( src.points[i].pos );
   }
}

void SimTed::addBrushEdgePoints( const Point2I & pos, Selection & sel )
{
   int brushDim = ( 1 << brushDetail );
   
   for( int i = 0; i < brushDim; i++ )
   {
      Point2I newPnt;
      
      // add all 4 points around the edge
      // top row
      newPnt.x = pos.x + i;
      newPnt.y = pos.y;
      sel.addPoint( newPnt );
      
      // bottom row
      newPnt.y += ( brushDim - 1 );
      sel.addPoint( newPnt );
      
      // left column
      newPnt.x = pos.x;
      newPnt.y = pos.y + i;
      sel.addPoint( newPnt );
      
      // right column
      newPnt.x += ( brushDim - 1 );
      sel.addPoint( newPnt );
   }
}

void SimTed::drawSquareFilled( Point2I pos, TSRenderContext& rc, UInt8 col )
{
   rc;
   surface->setFillColor( col );
   
   // create the points CCW
	Point3F p1( pos.x, 	  pos.y,     0.0 );
   Point3F p2( pos.x,     pos.y + 1, 0.0 );
   Point3F p3( pos.x + 1, pos.y + 1, 0.0 );
   Point3F p4( pos.x + 1, pos.y,     0.0 );

   float scale = (float)( 1 << scaleShift );
	p1 *= scale;
	p2 *= scale;
	p3 *= scale;
	p4 *= scale;
   
   // get the heights
   p1.z = getHeight( pos ); pos.y++;
   p2.z = getHeight( pos ); pos.x++;
   p3.z = getHeight( pos ); pos.y--;
   p4.z = getHeight( pos );
   
   TS::VertexIndexPair vertexPairs[5];
   
   vertexPairs[0].fVertexIndex = pointArray->addPoint( p1 )  ;
   vertexPairs[0].fTextureIndex = 0;
   vertexPairs[1].fVertexIndex = pointArray->addPoint( p2 );
   vertexPairs[1].fTextureIndex = 0;
   vertexPairs[2].fVertexIndex = pointArray->addPoint( p3 );
   vertexPairs[2].fTextureIndex = 0;
   vertexPairs[3].fVertexIndex = pointArray->addPoint( p4 );
   vertexPairs[3].fTextureIndex = 0;
   vertexPairs[4] = vertexPairs[0];
   
   pointArray->drawPoly( 5, vertexPairs, 0 );
}

void SimTed::drawLine( Point2F start, Point2F end, TSRenderContext& rc, 
   UInt8 col )
{
   rc;
	Point2I st( start.x, start.y );
	Point2I en( end.x, end.y );
	st >>= scaleShift;
	en >>= scaleShift;
	
   float sheight = getHeight( st );
   float eheight = getHeight( en );

   Point3F ulft, urt, llft, lrt;
   ulft = Point3F( start.x, start.y, sheight );
   urt = Point3F( end.x, end.y, eheight );

   pointArray->drawLine( pointArray->addPoint( ulft ), 
      pointArray->addPoint( urt ), col);
}    

void SimTed::drawSquare( Point2I pos, TSRenderContext& rc, UInt8 col )
{
   surface->setFillColor( col );

	Point2F p1( pos.x, 	  pos.y );
	Point2F p2( pos.x + 1, pos.y );
	Point2F p3( pos.x, 	  pos.y + 1 );
	Point2F p4( pos.x + 1, pos.y + 1 );

   float scale = ( float )(1 << scaleShift);
	p1 *= scale;
	p2 *= scale;
	p3 *= scale;
	p4 *= scale;
   
   drawLine( p3, p1, rc, col );
   drawLine( p1, p2, rc, col );
   drawLine( p4, p2, rc, col );
   drawLine( p3, p4, rc, col );

   if( drawNotch )
   {   
	   Point2F p5 = p4;
	   p5 -= p1;
	   p5 *= 0.25f;
	   p5 += p1;
      drawLine( p1, p5, rc, col );
	}
}

bool SimTed::setHeight( const Point2I& pos, const float& ht )
{
   //this method assumes coords are already shifted down by scale
   // and that setVars was called prior
   GridBlock::Height h;
   h.height = ht;
	bool success = false;

	Point2I bpos;
	bpos.x = pos.x >> blockShift;
	bpos.y = pos.y >> blockShift;

	if( bpos.x >= 0 && bpos.x < size.x && bpos.y >= 0 && bpos.y < size.y )
	{
		Point2I block_coords[ 4 ];
		int cnt = 0;
		int blockMask = (1 << blockShift) - 1;
		
		block_coords[ cnt++ ] = bpos;
		
		// Also set heights of adjacent blocks, if on the border...
		if ( bpos.x >= 1  &&  !(pos.x & blockMask) )
			block_coords[ cnt++ ] = Point2I( bpos.x - 1, bpos.y );
	
		if ( bpos.y >= 1  &&  !(pos.y & blockMask) )
		{
			block_coords[ cnt++ ] = Point2I( bpos.x, bpos.y - 1 );
	
			if ( bpos.x >= 1  &&  !(pos.x & blockMask) )
				block_coords[ cnt++ ] = Point2I( bpos.x - 1, bpos.y - 1 );
		}
		
		Point2I *blpos = block_coords;

		for ( int i = 0; i < cnt; i++, blpos++ )	
			if ( GridBlock* block = getGridFile()->getBlock( *blpos ) )
			{
				// get remainders on coords so they are in block space
				Point2I spos;
				spos.x = (pos.x - (blpos->x << blockShift));
				spos.y = (pos.y - (blpos->y << blockShift));

	         //set the height
	      	int index = (spos.y << blockShift) + spos.y + spos.x;

	         int mapwid = block->getHeightMapWidth();
	         if (index < mapwid*mapwid )
	         {
					block->setDirtyFlag();

	   		   GridBlock::Height* heightMap = block->getHeightMap();
	            if (heightMap )
		          	heightMap[index] = h;

					success = TRUE;	
	         }
		  	}
	}

   return ( success );
}

bool SimTed::setMaterial( const Point2I& pos, const GridBlock::Material & material )
{
   Point2I bpos;
   bpos.x = pos.x >> blockShift;
   bpos.y = pos.y >> blockShift;

   if( bpos.x >= 0 && bpos.x < size.x && bpos.y >= 0 && bpos.y < size.y )
      if( GridBlock* block = getGridFile()->getBlock( bpos ) )
      {
         //get coords into block
         Point2I spos;
         spos.x = ( pos.x - ( bpos.x << blockShift ) );
         spos.y = ( pos.y - ( bpos.y << blockShift ) );

         //set the Material
         int index = ( spos.y << blockShift ) + spos.x;

         int mapwid = block->getMaterialMapWidth();
         if( index < mapwid*mapwid )
         {
            GridBlock::Material *currMat = block->getMaterialMap();
               if ( !currMat )
                  return ( false );
            currMat[index] = material;
				block->setDirtyFlag();
         }
         
         return ( true );
      }

   return ( false );
}


float SimTed::getHeight(const Point2I& pos)
{                        
   Point2I bpos;
   bpos.x = pos.x >> blockShift;
   bpos.y = pos.y >> blockShift;

   if( bpos.x >= 0 && bpos.x < size.x && bpos.y >= 0 && bpos.y < size.y )
   {
      if( GridBlock* block = getGridFile()->getBlock( bpos ))
      {
         //get remainders
         Point2I spos;
			spos.x = ( pos.x - ( bpos.x << blockShift ) );
			spos.y = ( pos.y - ( bpos.y << blockShift ) );

         //get the height
      	int index = ( spos.y  << blockShift )+ spos.y + spos.x;

         int mapwid = block->getHeightMapWidth();
         if( index < mapwid*mapwid )
         {
   		   GridBlock::Height* heightMap = block->getHeightMap();
            if( !heightMap )
               return( 0 );

            return heightMap[index].height;
         }
      }
   }
   
   return( 0 );
}


GridBlock::Material * SimTed::getMaterial( const Point2I& pos )
{
   // coords should already be shifted down by ground scale
   Point2I bpos;
   bpos.x = pos.x >> blockShift;
   bpos.y = (pos.y) >> blockShift;

   if( bpos.x >=0 && bpos.x < size.x && bpos.y >=0 && bpos.y < size.y )
      if( GridBlock* block = getGridFile()->getBlock( bpos ) )
      {
         //get remainders on coords
         Point2I spos;
         spos.x = ( pos.x - (bpos.x << blockShift ) );
         spos.y = ( ( pos.y ) - ( bpos.y << blockShift ) );

         //get a ptr to the material
         int index = (spos.y << blockShift) + spos.x;

         int mapwid = block->getMaterialMapWidth();
         if( index < mapwid*mapwid )
         {
            GridBlock::Material* matMap = block->getMaterialMap();
            if( !matMap )
               return ( NULL );

            return ( &matMap[index] );
         }
      }

   return ( NULL );
}

TSMaterialList * SimTed::getMaterialList()
{
   if( getGridFile() )
      return( getGridFile()->getMaterialList() );
   
   return( NULL );
}
      
bool SimTed::open( const char * terrainName )
{
   // close down this one
   if( terrain )
      deleteTerrain();
     
   // create a new terrain
   if( ( terrain = new SimTerrain ) != 0 )
   {
      SimManager * serverManager = SimGame::get()->getManager(SimGame::SERVER);
		serverManager->addObject( terrain );
		serverManager->assignName( terrain, terrainName );
      
		deleteNotify( terrain );
		terrain->load( terrainName );
		
      if( getGridFile() )
      {
			newLandscaper();
		   loadMatList( getGridFile()->getMaterialListName() );
			terrain->setContext(EulerF(.0f,.0f,.0f),Point3F(.0f,.0f,.0f));
			terrain->setVisibility( &DefTerrainVisibility );
			terrain->setDetail( &DefTerrainDetail );
         SimGainFocusEvent::post( this );
			return( true );
		}
   }
   
   dataChanged = false;
   
   return( false );
}

// create a new file
bool SimTed::newFile( int scale, int blocksWide, int blocksHigh, int blockDim, 
      const char * terrName, GridFile::GBPatternMap blockPattern )
{
	blocksHigh;
	
   // create a new terrain
	terrain = new SimTerrain;
   SimManager * serverManager = SimGame::get()->getManager(SimGame::SERVER);
	serverManager->addObject( terrain );
	serverManager->assignName( terrain, terrName );
	deleteNotify( terrain );
	terrain->create( terrName, blocksWide, scale, blockDim, 0, blockPattern );
	newLandscaper();
   dataChanged = true;
   return true;
}

void SimTed::checkSaveChanges()
{
   if( terrain && dataChanged )
   {
	   int res = MessageBox( NULL, "save changes?", "close", MB_YESNO );
	   if( res == IDYES )
		{
         Console->evaluate( "Ted::save();", false );
      }
         
      dataChanged = false;
   }
}

bool SimTed::save( const char * fileName )
{
   bool success = false;
   if( terrain )
   {
      getGridFile()->updateHeightRange();
      success = terrain->save( fileName );
      
      // save off the named selections
      char selName[1024];
      sprintf(selName, "temp\\%s.sel", getGridFile()->getFileName());
      namedSelections.save( selName );
      
      if( !success )
	      MessageBox( NULL, "Failed to save changes.  File write protected?", "Warning!", MB_OK );
   }
   dataChanged = false;
   return( success );
}

bool SimTed::attachToTerrain( const char * terrainName )
{
   SimObject * obj;
   SimTerrain * terr = 0;

   SimManager * serverManager = SimGame::get()->getManager(SimGame::SERVER);
   
   if( terrainName )
   {
      // locate the terrain
      obj = serverManager->findObject( terrainName );
      if( obj )
         terr = dynamic_cast< SimTerrain * >( obj );
   }
   else
   {
      // find the first simterrain object
      for( SimSetIterator i( serverManager ); *i; ++i )
         if( ( terr = dynamic_cast< SimTerrain * >( * i ) ) != 0 )
            break;
   }
   
   if( terr )
   {
      // check if attaching a new terain			
		if( terr != terrain )
      {
               
			deleteTerrain();
			terrain = terr;
			deleteNotify( terrain );
         
         if( !getGridFile() )
            return( false );
            
			newLandscaper();
         
         // load in the named selections associated with this grid file
         // create the selection name ( just tack on a .sel )
         char * selName = new char[ strlen( getGridFile()->getFileName() ) +
            strlen( ".sel" ) + 1 ];
         sprintf( selName, "%s.sel", getGridFile()->getFileName() );
         
         // save off the named selections
         namedSelections.load( selName );
         
         delete [] selName;
         
			loadMatList( getGridFile()->getMaterialListName() );
		}
      
      setVars();
      currentSel.createMap( size.x << blockShift, 
         size.y << blockShift );
      
      // set the max detail
      pinDetailMax = getMaxBrushDetail() - 1;

      // set the current filename
      ResourceManager * resManager = SimResource::get( manager );
      ResourceObject * obj = resManager ? resManager->find( getGridFile()->getFileName() ) : NULL;
      
      if( !obj )
         return( false );
         
      CMDConsole::getLocked()->setVariable( "$TED::currFile", avar("%s\\%s", obj->filePath, obj->fileName ) );
      
      return( true );
   }
   
   return( false );
}

// lock the material list
void SimTed::lockMatList()
{
   TSMaterialList *mList = getGridFile()->getMaterialList();
   if( mList && (! mList->isLocked() ) )
      mList->lock( *SimResource::get( manager ), true );
}  

void SimTed::deleteTerrain()
{
   if( terrain )
   {
      clearNotify( terrain );
      terrain->deleteObject();
      terrain = NULL;
   }
}

void SimTed::newLandscaper()
{
   AssertFatal( terrain != 0, "Terrain cannot be null!" );
   
   // remove current landscaper
   if( landscaper )
   {
      clearNotify( landscaper );
      landscaper->deleteObject( );
   }
   
   // create a new landscaper object
   landscaper = new LSMapper;
   manager->addObject( landscaper );
   manager->assignName( landscaper, "LandScapeMapper" );
   deleteNotify( landscaper );
   landscaper->create( terrain );
}

bool SimTed::loadMatList( const char * matListName )
{
   if ( !matListName )
      return ( false );

   const char *mname = extendName( matListName, ".dml" );
	const char *gname = fixName( matListName, ".grid.dat" );
	const char *rname = fixName( matListName, ".plr" );

   // this function changes the material list name, which will
   // free 'matListName' out from under us - must re-get material
   // list name
	landscaper->setTextures( mname, gname );
   matListName = const_cast<char*>(getGridFile()->getMaterialListName());

   // attemp to load the plr file, then attempt to load a dat file
	if( !landscaper->setRules( rname ) )
   {
      // delete the string and recreate
      delete [] const_cast<char *>( rname );
      rname = fixName( matListName, ".rules.dat" );
      
      // attempt load as a dat file
      landscaper->setRules( rname );
   }

   getGridFile()->setMaterialListName( matListName );
	
	delete [] const_cast<char*>( rname );
	delete [] const_cast<char*>( gname );
	delete [] const_cast<char*>( mname );
   
   return( true );
}

// set the extension - requires that first char of extn is a '.'
// to work correctly
char * SimTed::extendName( const char * fname, const char * extn )
{
   if( fname )
   {
      char * current = const_cast<char*>(fname) + strlen(fname);
      while(current-- != fname)
         if(*current == '.')
            break;
            
      int len = current - fname;
      int extnLen = extn ? strlen(extn) : 0;
      
      char * retStr = new char[ len + extnLen + 1 ];
      
      memcpy(retStr,fname,len);
      if(extn)
         memcpy(retStr+len,extn,extnLen);
      retStr[len+extnLen] = '\0';
      
      return(retStr);
	}
	return (NULL);
}

// copys the extension onto the first part of a base name
//  -- fixName( "mars.terrain.dml", "grid.dat" ) will return "mars.grid.dat"
char * SimTed::fixName( const char * fname, const char * extn )
{
   if( fname )
   {
      // get a large enough buffer
      int len = strlen( fname ) + 1;
      if( extn )
         len += strlen( extn );
      
      char * retStr = new char[ len ];
      strcpy( retStr, fname );
      
      char * curr = retStr;
      
      // get to a place to cut to
      while( *curr && ( *curr != '.' ) )
         curr++;
         
      // add the extension or terminate the string
      if( extn )
         sprintf( curr, extn );
      else
         *curr = 0;
      
      return( retStr );         
   }
   
   return( NULL );
}

void SimTed::relight( bool hires )
{
   // make sure terrain exists
	if( terrain )
	{
		SimActionEvent *ev = new SimActionEvent;
		ev->action = hires ? MoveDown : MoveUp;
		manager->postCurrentEvent( terrain, ev );
	}
}	

// ------------------
//  undo/redo stuff
// ------------------

bool SimTed::SelectionStack::load( const char * file )
{
   FileRStream stream;

   // clear the stack
   clear();
      
   // open the file
   if( !stream.open( file ) )
      return( false );
      
   // read the size
   int numSelections;
   if( !stream.read( sizeof( int ), &numSelections ) )
      return( false );

   // go through the selections
   for( int i = 0; i < numSelections; i++ )
   {
      // read the name len
      int nameLen;
      if( !stream.read( sizeof( int ), &nameLen ) )
         return( false );
         
      // allocate some space
      char * name = new char[ nameLen + 1 ];
      if( !name )
         return( false );
      memset( name, 0, nameLen + 1 );   
         
      // read the name
      if( !stream.read( nameLen, name ) )
      {
         delete [] name;
         return( false );
      }
         
      // create a selection
      SimTed::Selection * sel = new SimTed::Selection;
      if( !sel )
      {
         delete [] name;
         return( false );
      }

      // assign the name and delete our allocated one
      sel->name = name;
      delete [] name;
      
      // read in the number of poins
      int numPoints;
      if( !stream.read( sizeof( int ), &numPoints ) )
      {
         delete sel;
         return( false );
      }
      
      // read in all the points
      for( int j = 0; j < numPoints; j++ )
      {
         Point2I pos;
         if( !stream.read( sizeof( Point2I ), &pos ) )
         {
            delete sel;
            return( false );
         }
         
         // add to the selection
         sel->addPoint( pos );
      }

      // add the selection to the stack
      push( sel );      
   }
   
   return( true );
}

void SimTed::SelectionStack::clear()
{
   SimTed::Selection * sel;
   
   while( 1 )
   {
      // grab the first selection and delete it
      sel = pop();
      if( !sel )
         return;
      delete sel;
   }
}

bool SimTed::SelectionStack::save( const char * file )
{
   FileWStream stream;
   
   // open the file
   if( !stream.open( file ) )
      return( false );
      
   // write out the size
   int numSelections = getSize();
   if( !stream.write( sizeof( int ), &numSelections ) )
      return( false );
   
   // go through the selections
   for( int i = 0; i < numSelections; i++ )
   {
      // get the length of the name of this selection
      int nameLen = strlen( selections[i]->name.c_str() );
      if( !stream.write( sizeof( int ), &nameLen ) )
         return( false );
      
      // write out the name
      if( !stream.write( nameLen, selections[i]->name.c_str() ) )
         return( false );
      
      // get the number of points and write out
      int numPoints = selections[i]->points.getSize();
      if( !stream.write( sizeof( int ), &numPoints ) )
         return( false );
      
      // go through the points
      for( int j = 0; j < numPoints; j++ )
      {
         if( !stream.write( sizeof( Point2I ), &selections[i]->points[j].pos ) )
            return( false );
      }
   }
   
   return( true );
}

void SimTed::SelectionStack::setLimit( int num )
{
   if( num > 0 )
   {
      // remove some if needed
      while( selections.size() > num )
      {
         delete selections[ 0 ];
         selections.erase( 0 );
      }
                  
      limit = num;
   }
}

SimTed::Selection * SimTed::SelectionStack::pop()
{
   // make sure has some stuff in it
   if( selections.size() )
   {
      SimTed::Selection * sel = selections[ selections.size() - 1 ];
      selections.pop_back();
      return( sel );
   }
   
   return( NULL );
}

void SimTed::SelectionStack::push( SimTed::Selection * sel )
{
   // add to the stack
   selections.push_back( sel );   
   
   if( selections.size() >= limit )
   {
      delete selections[ 0 ];
      selections.erase( 0 );
   }
}

SimTed::SelectionStack::SelectionStack() :
   limit( 100 )
{
}

SimTed::SelectionStack::~SelectionStack()
{
   for( int i = 0; i < selections.size(); i++ )
      delete selections[i];
}

void SimTed::undo()
{
   SimTed::Selection * selection;
   
   // get the selection
   selection = undoStack.pop();
   if( !selection )
      return;

   // add to the redo stack
   SimTed::Selection * redoSel = new SimTed::Selection;
  
   // go throught the selection and set each of the info's
   for( unsigned int i = 0; i < selection->points.getSize(); i++ )
   {
      // add to the redo
      SimTed::Selection::Info info;
      
      if( getInfo( selection->points[i].pos, info ) )
         redoSel->addPoint( info );
         
      setInfo( selection->points[i] );
   }
   
   // copy the pin info
   redoSel->pinArray = selection->pinArray;
   
   // set the flags
   redoSel->undoFlags = selection->undoFlags;
   
   processUndo( *selection, true );
   
   delete selection;
   
   // put into the redo stack
   redoStack.push( redoSel );
}

void SimTed::redo()
{
   SimTed::Selection * selection;
   
   // get the selection
   selection = redoStack.pop();
   if( !selection )
      return;
      
   // add a undo selection
   SimTed::Selection * undoSel = new SimTed::Selection;
   
   // go throught the selection and set each of the info's
   for( unsigned int i = 0; i < selection->points.getSize(); i++ )
   {
      // add to the undo
      SimTed::Selection::Info info;
      
      if( getInfo( selection->points[i].pos, info ) )
         undoSel->addPoint( info );
      
      setInfo( selection->points[i] );
   }

   // copy the pin info
   undoSel->pinArray = selection->pinArray;
   
   // set the flags
   undoSel->undoFlags = selection->undoFlags;

   processUndo( *selection, false );
      
   delete selection;
   
   // add to the undo stack
   undoStack.push( undoSel );
}

// usually does not need to know if this is an undo or a redo operation
void SimTed::processUndo( Selection & sel, bool undo )
{
   // look at the flags
   if( sel.undoFlags & Selection::relight )
      relight(false);
      
   if( ( sel.undoFlags & Selection::landscape ) && landscaper )
      landscaper->applyTextures();

   if( sel.undoFlags & Selection::flushTextures )
      flushTextures();
      
   // go through and un/pin all the pieces
   if( ( sel.undoFlags & Selection::pin ) || ( sel.undoFlags & Selection::unpin ) )
   {
      for( int i = 0; i < sel.pinArray.size(); i++ )
      {
         Selection::pinInfo info = sel.pinArray[i];
         
         // undo resets back to last value, redo just sets the flag according to the action
         if( undo )
            info.gridBlock->pinSquare( info.detail, info.pos, info.detail, info.val, NULL );
         else
            info.gridBlock->pinSquare( info.detail, info.pos, info.detail, 
               ( sel.undoFlags & Selection::pin ) ? true : false, NULL );
      }
   }
}

// ---------------------------------------------------------
//  selection stuff
// ---------------------------------------------------------

bool SimTed::loadSel( const char * name )
{
   Selection sel;
   
   // load the selection
   if( !sel.load( name ) )
      return( false );
      
   // float it
   floatSelection( sel );
   floatPasteVal = pasteVal;
   
   return( true );
}

bool SimTed::saveCurrentSel( const char * name )
{
   Selection sel;
   
   // check if there are points
   if( !currentSel.points.getSize() )
      return( false );

   Point2I offset = currentSel.points[0].pos;
   
   // go though all of the selection
   for( unsigned int i = 0; i < currentSel.points.getSize(); i++ )
   {
      // get the info ( the current selection only has pos as valid )
      Selection::Info info;
      
      if( getInfo( currentSel.points[i].pos, info ) )
      {
         // adjust the point
         info.pos.x -= offset.x;
         info.pos.y -= offset.y;
      
         // add to the copy selection
         sel.addPoint( info );
      }
   }
   
   // save it off
   return( sel.save( name ) );
}

SimTed::Selection::Selection() :
   name( "" ),
   undoFlags( 0 ),
   mapped( false ),
   map( NULL ),
   mapWidth( 0 ),
   mapHeight( 0 ),
   mapSize( 0 )
{
   points.setSize( 0, 256 );
}

SimTed::Selection::~Selection()
{
   // delete the map if it exists
   if( mapped )
      deleteMap();
}

void SimTed::Selection::createMap( int width, int height )
{
   if( mapped )
      deleteMap();

   // get the width/height
   mapWidth = width;
   mapHeight = height;
   mapSize = ( width * height );
   
   // allocate the space      
   map = new int[ mapSize ];
   AssertFatal( map, "Failed to allocate memory for selection map." );
      
   // set the flag
   mapped = true;
   
   // reset the map
   clearMap();
   fillMap();
}

void SimTed::Selection::deleteMap()
{
   if( map )
   {
      delete [] map;
      map = NULL;
   }
   mapped = false;
}

void SimTed::Selection::clearMap()
{
   if( !mapped || !map )
      return;

   // just go through
   for( int i = 0; i < mapSize; i++ )
      map[i] = -1;
}

void SimTed::Selection::fillMap()
{
   if( !mapped || !map )
      return;
      
   // set the map location
   for( unsigned int i = 0; i < points.getSize(); i++ )
   {
      // set the index
      map[ points[i].pos.x + 
         ( points[i].pos.y * mapWidth ) ] = i;
   }
}

void SimTed::Selection::addPoint( const Info & info )
{
   // check if mapped or not
   if( mapped )
   {
      int index = info.pos.x + ( info.pos.y * mapWidth );
      if( map[index] >= 0 )
         return;
      
      // set the index
      map[index] = ( int )points.getSize();
            
      // add the point
      points.add( info );
   }
   else
   {
      if( !pointInSel( info.pos ) )
         points.add( info );
   }
}

void SimTed::Selection::addPoint( const Point2I & pnt )
{
   // check if mapped or not
   if( mapped )
   {
      int index = pnt.x + ( pnt.y * mapWidth );
      if( map[index] >= 0 )
         return;
         
      map[index] = ( int )points.getSize();
  
      Info info;
      info.pos = pnt;
      points.add( info );
   }
   else
   {
      if( !pointInSel( pnt ) )
      {
         Info info;
         info.pos = pnt;
         points.add( info );
      }
   }
}

void SimTed::Selection::removePoint( const Point2I & pnt )
{
   // a tad tricky if mapped - is EXTREMELY tied to 
   // the infoarray class
   if( mapped )
   {
      unsigned int index = pnt.x + ( pnt.y * mapWidth );
      
      // check if exists
      if( map[index] < 0 )
         return;
         
      // check if not the last element, and then move the 
      // index's
      if( map[index] < ( int )( points.getSize() - 1 ) )
      {
         Point2I pos = points[ points.getSize() - 1 ].pos;
         int lastIndex = pos.x + ( pos.y * mapWidth );
         map[lastIndex] = ( int )map[index];
         
         // remove it
         points.remove( map[index] );

         // clear this position
         map[index] = -1;
      }
      else
      {
         // remove the last pnt
         points.remove( map[index] );
         map[index] = -1;
      }
   }
   else
   {
      for( unsigned int i = 0; i < points.getSize(); i++ )
      {
         if( points[i].pos == pnt )
         {
            points.remove( i );
            return;
         }
      }
   }
}

void SimTed::fillInfo( Selection & sel )
{
   for( unsigned int i = 0; i < sel.points.getSize(); i++ )
   {
      Selection::Info info;
      getInfo( sel.points[i].pos, info );
      sel.points[i] = info;
   }
}

void SimTed::Selection::clear()
{
   if( mapped )
      clearMap();
   points.clear();
}

bool SimTed::Selection::save( const char * file )
{
   FileWStream stream;
   
   // open the file
   if( !stream.open( file ) )
      return( false );
   
   int numPoints = ( int )points.getSize();
      
   // write the number of points
   if( !stream.write( sizeof( int ), &numPoints ) )
      return( false );
   
   // go through and write the info out
   for( unsigned int i = 0; i < points.getSize(); i++ )
   {
      if( !stream.write( sizeof( Selection::Info ), &points[i] ) )
         return( false );
   }
      
   return( true );
}

bool SimTed::Selection::load( const char * file )
{
   FileRStream stream;
   
   // clear out the file
   clear();
   
   // open the file
   if( !stream.open( file ) )
      return( false );
   
   int numPoints;
   
   // read the number of points
   if( !stream.read( sizeof( int ), &numPoints ) )
      return( false );
   
   // go through the points
   for( int i = 0; i < numPoints; i++ )
   {
      Info info;
      if( !stream.read( sizeof( Selection::Info ), &info ) )
         return( false );
      addPoint( info );
   }
   
   return( true );
}

bool SimTed::Selection::pointInSel( const Point2I & pnt )
{
//   if( ( pnt.x == -1 ) || ( pnt.y == -1 ) )
//      return( false );
      
   // check if mapped or not
   if( mapped )
   {
      int index = pnt.x + ( pnt.y * mapWidth );
      if( map[index] >= 0 )
         return( true );
   }
   else
   {
      for( unsigned int i = 0; i < points.getSize(); i++ )
         if( points[i].pos == pnt )
            return( true );
   }
            
   return( false );
}

bool SimTed::setInfo( const SimTed::Selection::Info & info )
{
   // set the stuff
   if( !setHeight( info.pos, info.height.height ) )
      return( false );
   if( !setMaterial( info.pos, info.material ) )
      return( false );

   return( true );
}

bool SimTed::getInfo( const Point2I& pos, SimTed::Selection::Info & info )
{               
   GridBlock::Material * mat = getMaterial( pos );
   GridBlock::Height height;
   
   height.height = getHeight( pos );
   
   // check if got the material information
   if( mat )
   {
      // set the info
      info.pos = pos;
      info.height = height;
      info.material = *mat;
      
      return( true );
   }
   
   return( false );
}

void SimTed::setSelectionVars( SimTed::Selection & sel, RectI & bound, 
   Point2F & center, float & radius )
{
   // reset the info
   bound = NULLRECT;
   center.x = center.y = -1;
   radius = 0.0;

   // make sure there exist some points
   if( !sel.points.getSize() )
      return;
      
   int left, right, top, bottom;
   
   // set the initial val
   left = right = sel.points[0].pos.x;
   top = bottom = sel.points[0].pos.y;
   
   // go through all the points
   for( unsigned int i = 0; i < sel.points.getSize(); i++ )
   {
      Point2I p = sel.points[i].pos;
      
      // set the bounds
      if( p.x < left )
         left = p.x;
      else if( p.x > right )
         right = p.x;
         
      if( p.y < top )
         top = p.y;
      else if( p.y > bottom )
         bottom = p.y;
   }
   
   // set the bound dimensions
   bound.upperL = Point2I( left, top );
   bound.lowerR = Point2I( right, bottom );
   
   // setup the center pnt
   center.x = ( float )( left + right ) / 2.0;
   center.y = ( float )( top + bottom ) / 2.0;
   
   // get the radius
   float rx = float( right - left ) / 2.0;
   float ry = float( bottom - top ) / 2.0;
   
   radius = sqrt( ( rx * rx ) + ( ry * ry ) );
}

const char * SimTed::getNamedSelection( int index )
{
   // check for invalid
   if( index >= getNumNamedSelections() )
      return( NULL );
      
   // get it
   return( namedSelections.selections[index]->name.c_str() );
}

bool SimTed::addNamedSelection( const char * name )
{
   // check the name
   if( !name )
      return( false );
      
   // grow in needed
   if( namedSelections.getSize() >= namedSelections.getLimit() )
      namedSelections.setLimit( namedSelections.getLimit() + 1 );
      
   SimTed::Selection * selection;

   selection = new SimTed::Selection;
   
   // set the name
   selection->name = name;
   
   // go through and add all of the currently selected points to this
   for( unsigned int i = 0; i < currentSel.points.getSize(); i++ )
      selection->points.add( currentSel.points[i] );   
   
   // add to the stack
   namedSelections.push( selection );
   
   return( true );
}

bool SimTed::removeNamedSelection( const char * name )
{
   if( !name )
      return( false );
                    
   Selection * sel;
   
   // go through all the selections and find this one.. 
   // if cannot just return with no error
   for( int i = 0; i < namedSelections.getSize(); i++ )
   {
      sel = namedSelections.selections[i];
      
      // is this the $$$?
      if( sel->name == name )
      {
         // remove this one
         delete namedSelections.selections[ i ];
         namedSelections.selections.erase( i );
         
         return( true );
      }
   }
   
   return( true );
}

bool SimTed::selectNamedSelection( const char * name )
{
   if( !name )
      return( false );
      
   Selection * sel;
   
   // go through all the selections and find this one.. 
   // if cannot just return with no error
   for( int i = 0; i < namedSelections.getSize(); i++ )
   {
      sel = namedSelections.selections[i];
      
      // is this the $$$?
      if( sel->name == name )
      {
         // clear the current selection
         currentSel.clear();
         
         // select all the ones in this named selection to the current
         for( unsigned int j = 0; j < sel->points.getSize(); j++ )
            currentSel.addPoint( sel->points[j] );
            
         return( true );
      }
   }
   
   return( true );
}

void SimTed::Selection::InfoArray::setSize( unsigned int size, unsigned int grow )
{
   growSize = grow;
   
   if( !arraySize || ( size >= arraySize ) )
   {
      arraySize = size + growSize;
      
      Info * newElements = new Info[ arraySize ];
      
      // copy the elements
      for( unsigned int i = 0; i < numElements; i++ )
         newElements[i] = elements[i];
         
      delete [] elements;
      elements = newElements;
   }
   
   numElements = size;
}

void SimTed::Selection::InfoArray::remove( unsigned int index )
{
   if( index < numElements )
   {
      numElements--;
      
      // check for removal of last item
      if( index < numElements )
         elements[ index ] = elements[ numElements ];
   }
}


void SimTed::Selection::InfoArray::add( Info item )
{
   // adjust the size
   setSize( numElements + 1, growSize );
   
   // add to the last item
   elements[ numElements - 1 ] = item;
}

