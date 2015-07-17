//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//               (c) Copyright 1996, Dynamix Inc.   All rights reserved.
//
//------------------------------------------------------------------------------

#ifndef _MISSION_EDITOR_H_
#define _MISSION_EDITOR_H_

#include "SimCollision.h"
#include "SimTreeView.h"
#include "GWEditBox.h"
#include "GWEditProxy.h"
#include "sim3Dev.h"
#include "simRenderGrp.h"
#include "simGuiCanvas.h"

//------------------------------------------------------------------------------

class MissionEditor : public SimTreeView
{
   class SelectionSet : public SimSet
   {
      MissionEditor     *parent;
      UINT              grabMask;   // allowed types for grabbing
      UINT              placeMask;  // allowed collision surface types for placement
      Point3F           granularity;
      SimCollisionInfo  grabInfo;
      SimCollisionInfo  placeInfo;
      EulerF            rotSum;
      Point3F           rotDegreeSnap;
      float             rotGranularity;
      typedef  SimSet   Parent;
      
      enum {
         leftMouseDrag     =  BIT(0),
         rightMouseDrag    =  BIT(1),
      };
      BitSet32          flags;
      DWORD             lastTime;
      Point2I           lastMousePos;

      float getMouseRotateSensitivity();
      float getMouseMoveSensitivity();
      
      bool getPlaneIntercepts( const Sim3DMouseEvent * event );

      Point3F           m_planeOrigin;         // the original centroid position
      Point3F           m_planeXYIntercept;  // intercepts on the plane from the origin
      Point3F           m_planeXZIntercept;
      Point3F           m_lastPlaneXYIntercept;
      Point3F           m_lastPlaneXZIntercept;
      
      Point3F           centroidOffset;   // offset from move operations to the centroid
      Point3F           lastPt;

      // OLD STUFF
      Point2I  lastPOINT;
      bool     mouseLeftDrag;
      bool     mouseRightDrag;
      bool     useOldMovement();
      void     oldHandleSim3DMouseEvent( const Sim3DMouseEvent *event );
      // END OLD STUFF
            
      void     removeObject(SimObject*);
      bool     processEvent(const SimEvent*);
      bool     processQuery(SimQuery *query);
		void     onRemove();
      void     onDeleteNotify(SimObject* obj);
      int      recurseCentroid( SimObject *obj );
      bool     anyObjectLocked();

   // undo stuff ---------------------------------------------
   public:
   
      class Undo
      {
         public:
         
            void setInfo();
            bool restoreInfo();
            void setParent( SelectionSet * _parent ){ parent = _parent; };
            
         private:
         
            SelectionSet * parent;
            
            struct Entry {
               int      id;
               TMat3F   tmat;
            };
            Vector< Entry > objList;   
         
      } undo;
      
   // --------------------------------------------------------
   
   public:
      Resource<GFXBitmap>  handCursor; 
      Resource<GFXBitmap>  lockCursor;
      Resource<GFXBitmap>  oldCursor;
      Point3F              centroidPt;

   public:
   
      friend   Undo;
      
      SelectionSet();
      SimObject* addObject(SimObject*);
      bool     toggleObject(SimObject*);
      void     setParent( MissionEditor *me ) { parent = me; }
      void     setGrabMask( UINT gm ) { grabMask = gm; }
      void     setPlaceMask( UINT pm ) { placeMask = pm; }
      UINT     getGrabMask() { return grabMask; }
      UINT     getPlaceMask() { return placeMask; }
      void     setGranularity( Point3F &grain ) { granularity = grain; }
      Point3F& getGranularity() { return granularity; }
      void     setRotGranularity( float val ) { rotGranularity = val; }
      float    getRotGranularity() { return( rotGranularity ); }
      void     setRotDegreeSnap( Point3F & snap ) { rotDegreeSnap = snap; }
      Point3F& getRotDegreeSnap() { return( rotDegreeSnap ); }
      void     handleSim3DMouseEvent( const Sim3DMouseEvent *event );
      void     clearSet();
      void     deleteSet();
      void     calculateCentroid();
      bool     isObjectSelected( UINT id, SimObject *obj=NULL );
      float    findLowest( SimObject *obj );
   }selectionSet;

   friend   SelectionSet;

   typedef  SimTreeView Parent;
   Point2I        tvSize;
   GWEditBox      editBox;
   SimGui::Canvas *parentCanvas;
   UINT           objMenu_IDM;
   UINT           terrainMenu_IDM;
   HMENU          hOptionsMenu;
   HMENU          hObjMenu;
   HMENU          hTerrainMenu;
   char           *caption;

   FileInfo       missionFile;
   FileInfo       macroFile;

   enum  MenuID
   {
      IDM_NEW = SimTreeView::IDM_STV_FILE_USER+1,
      IDM_LOAD,
      IDM_SAVE,
      IDM_SAVE_AS,
      IDM_SAVE_TED,
      IDM_LOAD_MACRO,
      IDM_SAVE_MACRO,
      IDM_CAM_FOLLOW,
      IDM_CAM_DROP_A,
      IDM_CAM_DROP_ROT,
      IDM_CAM_DROP_B,
      IDM_CAM_DROP_C,
      IDM_CAM_DROP_T,
      IDM_CAM_DROP_R,
      IDM_SNAP_GRID,
      IDM_SNAP_ROT,
      IDM_EDITOR,
      IDM_GRAB_MASK,
      IDM_PLACE_MASK,
      IDM_SHOW_HANDLES,
      IDM_CONST_X,
      IDM_CONST_Y,
      IDM_CONST_Z,
      IDM_ROT_X,
      IDM_ROT_Y,
      IDM_ROT_Z,
      IDM_GRANULARITY,
      IDM_ROTSETTINGS,
   };

   enum  EditMenuID
   {
      IDM_ADDTOGRP = SimTreeView::IDM_STV_EDIT_USER + 1,
      IDM_DROP_CAM,
      IDM_DROP_CAMROT,
      IDM_DROP_TERRAIN,
      IDM_DROP_INFRONT,
      IDM_DROP_DOWN,
      IDM_CAM_MOVE,
   };

   enum
   {
      IDM_MISSION_OBJ_BASE     = IDM_STV_USER,
      IDM_MISSION_TERRAIN_BASE = IDM_STV_USER+1000,
      IDM_MISSION_BUTTON_BASE  = IDM_STV_USER+2000
   };

   enum
   {
      ME_CAM_FOLLOW     = (STV_LASTSTATE<<1),
      ME_SNAP_GRID      = (STV_LASTSTATE<<2),
      ME_SNAP_ROT       = (STV_LASTSTATE<<3),
      ME_SIMINPUTFOCUS  = (STV_LASTSTATE<<4),
      ME_DROP_AC        = (STV_LASTSTATE<<5),   // new objects at camera ( actually a bit below )
      ME_DROP_CROT      = (STV_LASTSTATE<<6),   // new objects at camera with cameras rotation
      ME_DROP_BC        = (STV_LASTSTATE<<7),   // new objects below camera
      ME_DROP_CS        = (STV_LASTSTATE<<8),   // new objects at center screen
      ME_DROP_TG        = (STV_LASTSTATE<<9),   // objects seek ground below them
      ME_DROP_GR        = (STV_LASTSTATE<<10),   // groups seek ground relative to lowest in group
      ME_CONST_X        = (STV_LASTSTATE<<11),
      ME_CONST_Y        = (STV_LASTSTATE<<12),
      ME_CONST_Z        = (STV_LASTSTATE<<13),
      ME_SHOW_HANDLES   = (STV_LASTSTATE<<14),
      ME_ROT_X          = (STV_LASTSTATE<<15),
      ME_ROT_Y          = (STV_LASTSTATE<<16),
      ME_ROT_Z          = (STV_LASTSTATE<<17),
      ME_LASTSTATE      = (STV_LASTSTATE<<17), // shift count must match previous
   };

protected:
   class RenderImage : public SimRenderImage
   {
   public:
      SimManager *targetManager;
      MissionEditor *me;
      RenderImage() { itype = SimRenderImage::Overlay; }
      void render ( TSRenderContext &rc);
   };

   RenderImage renderImage;

public:
   static bool enabled;             // true if mission editor is enabled

   struct MaskTypes
   {
      DWORD       mask;
      char        *description;
   };
   static VectorPtr  <MaskTypes*>   maskTypes;   

   struct MissionMenu
   {
      HMENU       hMenu;            // handle to individual obj category menu
      char        *menuCategory;    // text description of obj category
   };

   struct MissionMenuItem
   {
      char        *consoleString;   // console string to evaluate on construction
      char        *description;     // class name of this object
   };

   VectorPtr   <MissionMenu*>       objectMenus;
   VectorPtr   <MissionMenuItem*>   objMenuItem;
   VectorPtr   <MissionMenu*>       terrainMenus;
   VectorPtr   <MissionMenuItem*>   terrainMenuItem;

private:
   void     onCommand(int id, HWND hwndCtl, UINT codeNotify);
   void     onSysCommand(UINT cmd, int x, int y);
   int      onNotify(int id, LPNMHDR pnmhdr);
   void     onSize( UINT state, int cx, int cy );
   void     positionWindows();
   void     onSetFocus(HWND hwndOldFocus);
   void     initMenu( HMENU hMainMenu );
   void     checkMenu( HMENU hMenu );
   static float   atofDif( float val, const char * buff );

public:
   //------------------------------------------------------------------------------
   // MissionEditor methods
   ~MissionEditor();
   bool     create( GWWindow *parent, const char *caption, RectI &r );
   char*    getCaption() { return caption; }
   void     getMissionGroup();

   SimObject* addObject( SimObject *obj );  // adds object to mission editor
   void     registerObject( const char *menuCategory, const char *menuText, const char *consoleString );
   void     registerTerrain( const char *menuCategory, const char *menuText, const char *consoleString );

   SimObject * getCamera();
   bool     getCameraTransform( TMat3F & transform );
   bool     setCameraTransform( TMat3F & transform );

   bool     onObjectLoad( FileInfo *fi, SimSet * parent );
   bool     onObjectSave( FileInfo *fi, SimObject *obj );

   bool     objectLoadPersist( const char * parentName, const char * fileName );
   bool     objectSavePersist( const char * objName, const char * fileName );

   const char * getObjectFullName( SimObject * obj );
   const char * stripPath( const char * file );

   bool     undoMoveRotate(){ return( selectionSet.undo.restoreInfo() ); };
   void     onDropThroughCamera();
   void     onDropAtCamera();
   void     onDropBelowCamera();
   void     onDropAtCenterScreen();
   void     onDropDown( SimObject *obj );
   void     onDropDownRel();
   bool     onIDMCamMove();
   bool     getLOSIntercept( const Sim3DMouseEvent *event, SimCollisionInfo *info, DWORD allowedTypes );
   void     moveObjectToPoint( SimObject *obj, Point3F& pt );
   void     moveObjectRelative( SimObject *obj, Point3F& deltaPt );
   void     rotateObject( SimObject *obj, Point3F &anchor, AngleF angle );
   void     rotateObject( SimObject *obj, Point3F& anchor, EulerF& rot );
   void     rotateObject( SimObject *obj, EulerF& rot, EulerF& snap );
   void     rotateObject( SimObject *obj, AngleF angle );
   void     rotateObject( SimObject *obj, EulerF& angle );
   void     setObject( SimObject *obj, Point3F& pos, EulerF& rot );
   void     snapToGrid( Point3F &pt );
   bool     saveMission( const char * name );

   //------------------------------------------------------------------------------
   // toolbar button stuff
   struct ToolbarButton
   {
      static int              maxButtons;       // maximum number of buttons on bar
      static int              currentID;        // current id to assign a button
      static int              width;            // width of button
      static int              height;           // height for the button

      enum ButtonType {       // button types
         Normal   = 1 << 0,
         Toggle   = 1 << 1,
         Gap      = 1 << 2
      };
                           
      char *                  name;             // name of the button
      char *                  help;             // help text
      char *                  commandA;         // primary command
      char *                  commandB;         // command for toggle buttons
      int                     type;             // the type of the button
      unsigned int            id;               // id passed in through onCommand
   };
   
   //------------------------------------------------------------------------------

private:
   enum {
      AutoSaveTimerID = 0x1234
   };
   
   char *   autoSaveName;
   int      autoSaveInterval;
   UINT     autoSaveTimer;
   
   void     onTimer( UINT id );
   void     postAutoSaveEvent();
   
public:


   bool     setAutoSaveInterval( int min );
   bool     setAutoSaveName( const char * name );

   //------------------------------------------------------------------------------
   
private:

   ToolbarButton * MissionEditor::getButton( const char * name );
   Vector   <ToolbarButton>   toolbarButtons;
   HWND     hToolbar;

public:


   bool     addButton( const char * name, const char * bmp, const char * help, const char * commandA, 
      const char * commandB );
   bool     removeButtons();
   bool     removeButton( const char * name );
   HWND     getToolbarHandle(){ return( hToolbar ); }
   
   bool     setButtonChecked( const char * name, bool check );
   bool     isButtonChecked( const char * name );
   bool     setButtonEnabled( const char * name, bool enabled );
   bool     isButtonEnabled( const char * name );
   
   //------------------------------------------------------------------------------
   
   static   BOOL CALLBACK setMaskDlgProc( HWND, UINT, WPARAM, LPARAM );
   static   BOOL CALLBACK setGrainDlgProc( HWND, UINT, WPARAM, LPARAM );
   static   BOOL CALLBACK setRotationDlgProc( HWND, UINT, WPARAM, LPARAM );

   //------------------------------------------------------------------------------
   // simTreeView methods
   void     handleItemSelection( HTREEITEM hItem, bool dblClick );
   void     handleContextMenuItemInsert( HMENU hMenu );
   bool     handleContextMenuItemSelection( int id );
   char*    handleItemNameChange( HTREEITEM hItem, char *name );
   int      handleGetBitmapIndex( SimObject *obj, bool selected );
   int      handleGetBitmapIndex( HTREEITEM hItem, bool selected ) { return Parent::handleGetBitmapIndex( hItem,selected ); }

   //------------------------------------------------------------------------------
   // simObject methods
	bool     onAdd();
	void     onRemove();
   bool     processEvent(const SimEvent* event);
   bool     processQuery(SimQuery *query);
   bool     onSim3DMouseEvent(const Sim3DMouseEvent *event);
   bool     onSimEditEvent(const SimEditEvent *event);
   bool     onSimGainFocusEvent(const SimGainFocusEvent *event);
   bool     onSimLoseFocusEvent(const SimLoseFocusEvent *event);
   bool     onSimInputPriorityQuery( SimInputPriorityQuery *query );
   bool     onSimRenderQueryImage(SimRenderQueryImage *query);
};

#endif   _MISSION_EDITOR_H_