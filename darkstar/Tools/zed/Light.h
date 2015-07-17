/////////////////////////////////////////////////////////////////////////////
//
// Light.h : header file
//
/////////////////////////////////////////////////////////////////////////////

#ifndef _LIGHT_H_
#define _LIGHT_H_

#include "thred.h"
#include "Thredparser.h"
#include "ts_PointArray.h"
#include "thredprimitives.h"
#include "rendercamera.h"
#include "ts_vertex.h"
#include "lightblankprop.h"
#include "lightprop.h"
#include "lightStateProp.h"
#include "lightemitterprop.h"
#include "lightpropertysheet.h"
#include "itrlighting.h"

class CTreeBase;
class CTHREDDoc;
class CLight;
class CLightState;
class CLightEmitter;
class CLightView;
class CLightViewTree;
class CLightPropertySheet;
class CTHREDDoc;


typedef CArray< CLight, CLight& > CLightArray;
typedef CArray< CLightState, CLightState& > CLightStateArray;
typedef CArray< CLightEmitter, CLightEmitter& > CLightEmitterArray;

class CItemBase
{
   friend CLightView;
   friend CLightViewTree;
   friend CLightPropertySheet;
   friend CTHREDDoc;
   
   public:

      enum menuIds
      {
         CUT = 1,
         COPY,
         REMOVE,
         INSPECT,
         ADD_LIGHT,
         PASTE_LIGHT,
         LAST_MENU_ID
      };   
      
      enum types {
         LIGHT,
         LIGHT_STATE,
         LIGHT_EMITTER
      };
      
      enum flags {
         CAN_NAME              = BIT_0
      };

   private:
         
      static   CLightBlankProp  * blankProp;
      
   protected:
   
      DWORD                      type;
      DWORD                      baseFlags;
      HTREEITEM                  hTreeItem;
      CItemBase *                _parent;

      static   CLightViewTree *       tree;
      static   CLightPropertySheet *  propSheet;
      static   CItemBase *   currentInspectObj;
      static   CItemBase *   copyObj;
      static   CTHREDDoc * pDoc;
   
   public:
   
      CItemBase();
      
      // member functions
      virtual void onSelect();
      virtual bool onDragOver( CItemBase * source );
      virtual void onDrop( CItemBase * source );
      virtual bool onInspect( bool read = true );
      virtual void onContextMenu( CMenu & menu );
      virtual void onContextMenuSelect( int id );
      virtual bool onNameChanged( const char * newName );
      virtual void getName( CString & str );
      virtual bool add( CItemBase * parent, int image, int selectedImage );
      virtual CPropertyPage * getPropPage();
      virtual int getIndex();

      // edit functions      
      virtual bool canPaste();
      virtual void copy(){};
      virtual void paste(){};
      virtual void remove();
      
      void inspect( bool read = true, bool create = true );
      CItemBase * getParent(){ return( _parent ); };
            
      // inlined
      DWORD getFlags(){ return( baseFlags ); };
      DWORD getType(){ return( type ); };
};

class CLight : public CItemBase
{
   friend CTHREDDoc;
   public:
   
      enum menuIds {
         ADD_STATE = CItemBase::LAST_MENU_ID,
         PASTE_STATE
      };
      
      enum oldFlags {
         LIGHT_AUTOSTART                  = BIT_0,
         LIGHT_LOOPTOEND					   = BIT_1,
         LIGHT_RANDOMFLICKER				   = BIT_2,
         LIGHT_HASDEFAULTDISTANCEFALLOFF	= BIT_3,
         LIGHT_HASDEFAULTPOSITION		   = BIT_4,
         LIGHT_HASDEFAULTCOLOR			   = BIT_5,
         LIGHT_HASNAME					      = BIT_6,
         LIGHT_HASDEFAULTLINEARFALLOFF	   = BIT_7,
         LIGHT_ACTIVE					      = BIT_8,
         LIGHT_SELECTED				      	= BIT_9,
         LIGHT_INLIGHTSET			      	= BIT_10,
         LIGHT_MANAGEBYMISSIONEDITOR      = BIT_11
      };
      
      enum flags {
         AUTOSTART                        = ITRLIGHTANIM_AUTOSTART,
         LOOPTOEND                        = ITRLIGHTANIM_LOOPTOENDFRAME,
         RANDOMFLICKER                    = ITRLIGHTANIM_RANDOMFRAME,
         MANAGEBYMISSIONEDITOR            = ITRLIGHTANIM_MANAGEDBYMEDITOR,
      };
   
   private:
   
      int      id;
      CString  name;
      float    animationDuration;
      DWORD    flags;
   
      bool     m_active;
      
      static   CLightProp  * lightProp;
      
   public:

      static   bool showFalloffs;
            
      // ItemBase member functions
      bool onNameChanged( const char * newName );
      void onSelect();
      CPropertyPage * getPropPage();
      bool onInspect( bool read = true );
      void onContextMenu( CMenu & menu );
      void onContextMenuSelect( int id );
      bool onDragOver( CItemBase * source );
      void onDrop( CItemBase * source );
      int getIndex();
      
      // edit functions      
      bool canPaste();
      void copy();
      void paste();
      void remove();
   
      int            group;
      
      static CLightArray * currentArray;
      CLightStateArray states;   
      
      bool selected();
      void resetSelected();
      void moveLight( double x, double y, double z );
      void rotateLight( EulerF & rot, Point3F & center );
      void doneRotateLight( float snapRot );
      double getDistance( ThredPoint pnt );
      
      void renderLight( CRenderCamera * camera, TSRenderContext * rc, int viewType, bool selected );
      void doneMoveLight( double gridSize, bool useEditBox, int viewType );
      bool active(){ return( m_active ); };
      void setActive( bool active ){ m_active = active; };
      CLight( bool createFirst = false );
      ~CLight();
      bool inBox( ThredBox & box );
      void select( bool all = false );
      CLightEmitter * markSelected( ThredPoint pnt );
      
      void Serialize( CThredParser & LightState );
      void Export( FILE * OutFile ){};
      void addState( CLightState & state ){ states.Add( state ); };
      void getName( CString & str ){ str = name; };
      void setName( const char * _name ){ name = _name; };
      
      CLight & operator=( CLight & light );
};

class CLightState : public CItemBase
{
   private:
   
      static   CLightStateProp  *  lightStateProp;
      
   public:
   
      enum menuIds {
         ADD_EMITTER = CItemBase::LAST_MENU_ID,
         PASTE_EMITTER
      };
      
      // ItemBase member functions
      CPropertyPage * getPropPage();
      void getName( CString & str );
      bool onInspect( bool read = true );
      void onContextMenu( CMenu & menu );
      void onContextMenuSelect( int id );
      bool onDragOver( CItemBase * source );
      void onDrop( CItemBase * source );
      int getIndex();
      void onSelect();
      
      // edit functions      
      bool canPaste();
      void copy();
      void paste();
      void remove();
      
      enum oldFlags {
         STATE_LINEAR_FALLOFF	            = BIT_0,
         STATE_DISTANCE_FALLOFF	         = BIT_1,
         STATE_HASSPOTFACTOR		         = BIT_2,
         STATE_HASPOSITION		            = BIT_3,
         STATE_HASDURATION		            = BIT_4,
         STATE_HASCOLOR			            = BIT_5,
      };
      
      float    stateDuration;
      ColorF   color;
      
      CLightEmitterArray emitters;
      
      CLightState();
      
      int numEmitters(){ return( emitters.GetSize() ); };
      CLightEmitter & getEmitter( int i ){ return( emitters[i] ); };
      void addEmitter( CLightEmitter & emitter ){ emitters.Add( emitter ); };

      CLightState & operator=( CLightState & state );
};

class CLightEmitter : public CItemBase
{
   private:
      
      static   CLightEmitterProp * lightEmitterProp;
      bool     _selected;
      
   public:
   
      // ItemBase member functions
      CPropertyPage * getPropPage();
      void getName( CString & str );
      bool onInspect( bool read = true );
      bool onDragOver( CItemBase * source );
      void onDrop( CItemBase * source );
      int getIndex();
      void onSelect();
      void copy();
      void remove();
      
      enum LightType {
         PointLight = 0,
         SpotLight = 1
      };
      enum Falloff {
         Distance = 0,
         Linear = 1
      };
   
      Point3F     movePos;
   
      // zed!!!!!!
      EulerF      rotSum;
      Point3F     rotPos;
      bool        rotValid;
      
      LightType   lightType;
      Point3F     pos;
      Point3F     spotVector;
      float       spotTheta;        // inner angle
      float       spotPhi;          // outer angle
      Falloff     falloff;
      float d1, d2, d3;
      
      bool        getSelect(){ return( _selected ); };
      void        select( bool sel ){ _selected = sel; movePos = pos; };
      
      bool        selectable;
      
      CLightEmitter();
};


#endif // _LIGHT_H_
