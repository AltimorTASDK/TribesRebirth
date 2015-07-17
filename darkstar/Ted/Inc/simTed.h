#ifndef _SIMTED_H_
#define _SIMTED_H_

#include <g_surfac.h>
#include <ts_camera.h>
#include <LSMapper.h>
#include <LSEditor.h>
#include <simTerrain.h>
#include <FLOAT.H>
#include "simMoveobj.h"
#include "sim3dev.h"
#include "simGuiCanvas.h"
//#include "array.h"

const RectI NULLRECT = RectI(-1, -1, -1, -1);

// mouse callback
class SimTedMouseCallback
{
   public:
      virtual bool mouseCallback( bool up, int button ) = 0;
};


class SimTed : public SimObject, public SimRenderImage
{
   public:
   
      class Selection
      {
         private:
            int * map;
            int mapWidth;
            int mapHeight;
            int mapSize;
            bool mapped;
            
         public:

            enum UndoFlags { relight = 1, landscape = 2,
               flushTextures = 4, pin = 8, unpin = 16 };
            struct Info 
            {
               Point2I              pos;
               GridBlock::Height    height;
               GridBlock::Material  material;
            };
            
            class InfoArray
            {
               private:
                  Info * elements;
                  unsigned int numElements;
                  unsigned int arraySize;
                  unsigned int growSize;

               public:
                  InfoArray( unsigned int size = 0, unsigned int grow = 0 ) : 
                     numElements( size ), growSize( growSize ), 
                     arraySize( size ), elements( new Info[ size ] ) {}
                  ~InfoArray() { delete [] elements; }
                  unsigned int getSize() { return( numElements ); }
                  Info & operator[]( unsigned int index ) { return elements[ index ]; }      
                  
                  void setSize( unsigned int size = 0, unsigned int grow = 0 );
                  void remove( unsigned int index );
                  void add( Info item );
                  void clear() { numElements = 0; }
            };
            
            Selection();
            ~Selection();
            
            InfoArray points;
            String name;
            UInt8 undoFlags;
            
            // used for undo'n pin operations
            struct pinInfo
            {
               int         detail;
               Point2I     pos;
               bool        val;
               GridBlock * gridBlock;
            };
            
            Vector < pinInfo > pinArray;
            
            // the info one is used for undo/redo stuff
            void addPoint( const Info & info );
            void addPoint( const Point2I & pnt );
            void removePoint( const Point2I & pnt );
            bool pointInSel( const Point2I & pnt );
            void clear();

            bool load( const char * file );
            bool save( const char * file );
            
            void createMap( int width, int height );
            void clearMap();
            void deleteMap();
            void fillMap();
      };
      
      class SelectionStack
      {
         private:
            int limit;
            
         public:
            
            Vector < Selection * > selections;
            SelectionStack();
            ~SelectionStack();
            void setLimit( int num );
            int getLimit() { return( limit ); }
            int getSize() { return( selections.size() ); };
            Selection * pop();
            void clear();
            void push( Selection * sel );
            bool load( const char * file );
            bool save( const char * file );
      };
      
   bool getInfo( const Point2I & pnt, Selection::Info & info );
   bool setInfo( const Selection::Info & info );
   void setSelectionVars( Selection & sel, RectI & bound, 
      Point2F & center, float & radius );
   void fillInfo( Selection & sel );

   bool loadSel( const char * name );
   bool saveCurrentSel( const char * name );

   private:
   
      int scaleShift;
      int blockShift;
      
      SimTedMouseCallback * mouseHandler;
      Point3F mouseStartPt;
      Point3F mouseDirection;
      SimTerrain * terrain;
      LSMapper * landscaper;
      bool focused;
      
      GFXSurface * surface;
      TS::PointArray * pointArray;

      Selection currentSel;
      Selection currentClusterSel;
      Selection hilightSel;
      Selection hilightClusterSel;

      Selection copySel;
      Selection copyClusterSel;

      Selection * currentUndo;
            
      SelectionStack undoStack;
      SelectionStack redoStack;
      SelectionStack namedSelections;
      
      Point2I size;
      
      void setVars();
      bool getInterceptCoord( Point2I & pnt );
      void drawSquareFilled( Point2I pos, TSRenderContext& rc, UInt8 col );
      void drawSquareOutline( Point2I & begin, Point2I & end, TSRenderContext & rc, UInt8 col );
      void newLandscaper();
      TMat3F & getTransform() { return( ( TMat3F & )terrain->getTransform() ); }
      void drawLine( Point2F start, Point2F end, TSRenderContext& rc, UInt8 col );
      void drawSquare( Point2I pos, TSRenderContext& rc, UInt8 col );
      bool pointValid( const Point2I& pnt );
      bool setHeight( const Point2I& pos, const float& ht );
      bool setMaterial( const Point2I& pos, const GridBlock::Material & material );
      float getHeight( const Point2I& pos );
      GridBlock::Material * getMaterial( const Point2I& pos );

      // variables used in the selection of terrain      
      bool regionSelect;
      Point2I selectStartPnt;
      bool selectCanceled;
      Point2I selectLastPnt;
      
      // index's to the entry in the table
      int mouseLButtonAction;
      int mouseRButtonAction;
      int mouseProcessAction;
      
      bool feather;
      bool snap;
      bool grabMode;
      bool drawNeighbors;
      Point2I grabPoint;
      float grabDist;
      float grabHeight;
      float avgHeight;
      Point2I brushPos;
      int brushDetail;
      int pinDetail;
      int pinDetailMax;

      Point2I cursorPos;
      Point2I lastCursorPos;
      Point2I cursorDif;

   public:
   
      enum {
         Outline  = BIT(0),
         Frame    = BIT(1),
         Fill     = BIT(2),
      };     
   
   private:
             
      // drawing stuff...
      UInt8 hilightFillColor;
      UInt8 hilightFrameColor;
      BitSet32 hilightShow;
      UInt8 selectFillColor;
      UInt8 selectFrameColor;
      BitSet32 selectShow;
      UInt8 shadowFillColor;
      UInt8 shadowFrameColor;
      BitSet32 shadowShow;
      
      bool blockOutline;
      UInt8 blockFrameColor;

      UInt8 materialIndex;
      bool flipColors;
      SimGui::Canvas * canvas;
      
      void clusterizeSelection( Selection & dest, Selection & src );
      
   public:

      void setCanvas( SimGui::Canvas * sc ){ canvas = sc; };
      SimGui::Canvas * getCanvas(){ return( canvas ); };
      
      TMat3F & getInvTransform() { return( ( TMat3F & )terrain->getInvTransform() ); }
      
      enum FlagOps { set, clear };
      enum BrushAction { begin, update, end };
      enum PasteOps { Material = BIT(0), Height = BIT(1) };
      enum Side { top = 1, right = 2, bottom = 4, left = 8};

      struct TedAction
      {
            String name;
            void ( SimTed::*callback )( BrushAction action, 
               Selection & sel, Selection & undo );
            bool mouseAction;
            String description;
      };

      static void SimTed::pinSquareCallback( GridBlock::pinCallbackInfo * info );

      void mouseAction( int button, SimTed::BrushAction action );

      // add the Action suffix just so we dont use up all the 
      // cool function names
      
      // the actions that can be done on the terrain
      void selectAction( SimTed::BrushAction action, Selection & sel, Selection & undo );
      void deselectAction( SimTed::BrushAction action, Selection & sel, Selection & undo );
      void setHeightAction( SimTed::BrushAction action, Selection & sel, Selection & undo );
      void raiseHeightAction( SimTed::BrushAction action, Selection & sel, Selection & undo );
      void lowerHeightAction( SimTed::BrushAction action, Selection & sel, Selection & undo );
      void smoothAction( SimTed::BrushAction action, Selection & sel, Selection & undo );
      void getAvgHeightAction( SimTed::BrushAction action, Selection & sel, Selection & undo );
      void setFlagsAction( SimTed::BrushAction action, Selection & sel, Selection & undo );
      void clearFlagsAction( SimTed::BrushAction action, Selection & sel, Selection & undo );
      void elevateAction( SimTed::BrushAction action, Selection & sel, Selection & undo );
      void noiseAction( SimTed::BrushAction action, Selection & sel, Selection & undo );
      void setMaterialAction( SimTed::BrushAction action, Selection & sel, Selection & undo );
      void copyAction( SimTed::BrushAction action, Selection & sel, Selection & undo );
      void pasteAction( SimTed::BrushAction action, Selection & sel, Selection & undo );
      void setTerrainTypeAction( SimTed::BrushAction action, Selection & sel, Selection & undo );
      void undoAction( SimTed::BrushAction action, Selection & sel, Selection & undo );
      void redoAction( SimTed::BrushAction action, Selection & sel, Selection & undo );
      void relightAction( SimTed::BrushAction action, Selection & sel, Selection & undo );
      void rotateRightAction( SimTed::BrushAction action, Selection & sel, Selection & undo );
      void rotateLeftAction( SimTed::BrushAction action, Selection & sel, Selection & undo );
      void pinAction( SimTed::BrushAction action, Selection & sel, Selection & undo );
      void scaleAction( SimTed::BrushAction action, Selection & sel, Selection & undo );
      void adjustCornerAction( SimTed::BrushAction action, Selection & sel, Selection & undo );
      void depressAction( SimTed::BrushAction action, Selection & sel, Selection & undo );
      void setCornerHeightAction( SimTed::BrushAction action, Selection & sel, Selection & undo );
      void adjustControlPointAction( SimTed::BrushAction action, Selection & sel, Selection & undo );
      void setControlHeightAction( SimTed::BrushAction action, Selection & sel, Selection & undo );
      void clearPinAction( SimTed::BrushAction action, Selection & sel, Selection & undo );
      void adjustHeightAction( SimTed::BrushAction action, Selection & sel, Selection & undo );
      
      // actions that are internal only
      void flagAction( SimTed::BrushAction action, Selection & sel, Selection & undo, SimTed::FlagOps flagOp );
      void rotateAction( SimTed::BrushAction action, Selection & sel, Selection & undo, bool rotRight = true );
      void featherAction( SimTed::BrushAction action, Selection & sel, Selection & undo, bool raise, float height );

      bool addNamedSelection( const char * name );
      bool removeNamedSelection( const char * name );
      bool selectNamedSelection( const char * name );
      
   private:
         
      float heightVal;
      float adjustVal;
      float smoothVal;
      UInt8 flagVal;
      UInt8 pasteVal;
      UInt8 floatPasteVal;
      int   terrainType;
      bool dataChanged;
      bool drawNotch;
      bool selFloating;
      bool mouseMoved;
      float scaleVal;
      
      void SimTed::addAction( const char * name, void ( SimTed::*callback )
         ( SimTed::BrushAction, Selection & sel, Selection & undo ), bool mouseAction,
         const char * description );
      Vector< TedAction * > actions;
      void fillSelection( Selection & sel );
      void adjustControlPoint( const Point2I control, float height, Selection & undo, bool top, bool left, bool setUndo );
      void setBrushCornerHeight( const Point2I pnt, Selection & undo, float height, bool setUndo );
      void getBrushCenter( Point2I & center, Point2I & origin );
      void raiseEdgeHeight( const Point2I brush, Selection & undo, SimTed::Side side, float height, bool raise );
      void processUndo( Selection & sel, bool undo );
      void addBrushEdgePoints( const Point2I & pos, Selection & sel );
      void drawMouseShadow( TSRenderContext & rc );
         
   public:

      Selection * getCurrentUndo(){ return( currentUndo ); }
      void floatCurrent();
      void floatSelection( Selection & sel );
      bool getSelFloating(){ return( selFloating ); }
      void setSelFloating( bool b ){ selFloating = b; }
      void setFloatPasteVal( UInt8 val ){ floatPasteVal = val; }
      int getNumActions(){ return( actions.size() ); }
      bool getActionInfo( int index, TedAction & info );
      int getNumNamedSelections(){ return( namedSelections.getSize() ); }
      const char * getNamedSelection( int index );
      void setBrushPos( const Point2I & pos );
      void setBrushDetail( int detail );
      int getMaxBrushDetail();
      GridFile * getGridFile() { return( terrain ? terrain->getGridFile() : NULL); }
            
      bool processAction( const char * name );
      int getActionIndex( String name );   
      void setLButtonAction( String name );
      void setRButtonAction( String name ); 
      int getLButtonActionIndex(){return(mouseLButtonAction);}
      int getRButtonActionIndex(){return(mouseRButtonAction);}
      void clearPinMaps();
   
      SimTed();
      ~SimTed();
      void init();
      char * fixName( const char * fname, const char * extn );
      char * extendName( const char * fname, const char * extn );
      bool save( const char * fileName );
      void deleteTerrain();
      void lockMatList();
      bool loadMatList( const char * matListName );
      void setMouseHandler( SimTedMouseCallback * mHandler ) 
         { mouseHandler = mHandler; };
      bool open( const char * terrainName );
      bool newFile( int scale, int blocksWide, int blocksHigh, int blockDim, 
         const char * terrName, GridFile::GBPatternMap blockPattern );
		void checkSaveChanges();
      bool attachToTerrain( const char * terrain );      
      bool onSim3DMouseEvent( const Sim3DMouseEvent * event );
      bool onAdd();
      void onDeleteNotify(SimObject * obj);
      void onRemove();
      bool processEvent( const SimEvent * event );
      bool processQuery( SimQuery * query );
      void render( TSRenderContext &rc );      
      void relight(bool hires);
      
      SimTerrain * getTerrain() { return( terrain ); };
      const char * getFileName(){ return( getGridFile()->getFileName() ); }
      
      LSMapper * getLSMapper() { return landscaper; }
      void flushTextures();
      TSMaterialList * getMaterialList();
      int getUndoCount(){ return( undoStack.getSize() ); }
      int getRedoCount(){ return( redoStack.getSize() ); }
      bool pasteBufferValid(){ return( copySel.points.getSize() ? true : false ); }
      bool currentSelValid(){ return( currentSel.points.getSize() ? true : false ); }
      
      void setPinDetail( int detail ){ pinDetail = detail; }
      int getPinDetail(){ return( pinDetail ); }
      void setPinDetailMax( int detail ){ pinDetailMax = detail; }
      int getPinDetailMax(){ return( pinDetailMax ); }
      
      int getBrushDetail() { return( brushDetail ); }
      int getLButtonAction() { return( mouseLButtonAction ); }
      int getRButtonAction() { return( mouseRButtonAction ); }
      void setSnap( bool b ){ snap = b; }
      void setBlockOutline( bool b ){ blockOutline = b; }
      bool getBlockOutline(){ return( blockOutline ); }
      bool getSnap(){ return( snap ); }
      bool getFeather(){ return( feather ); }
      void setFeather( bool b ){ feather = b; }
      void setBlockFrameColor( UInt8 col ){ blockFrameColor = col; }
      void setSelectFrameColor( UInt8 col ){ selectFrameColor = col; }
      void setSelectFillColor( UInt8 col ){ selectFillColor = col; }
      void setHilightFrameColor( UInt8 col ){ hilightFrameColor = col; }
      void setHilightFillColor( UInt8 col ){ hilightFillColor = col; }
      void setShadowFrameColor( UInt8 col ){ shadowFrameColor = col; }
      void setShadowFillColor( UInt8 col ){ shadowFillColor = col; }

      void setSelectShow( BitSet32 val ){ selectShow = val; }
      void setShadowShow( BitSet32 val ){ shadowShow = val; }
      void setHilightShow( BitSet32 val ){ hilightShow = val; }
      BitSet32 getSelectShow(){ return( selectShow ); }
      BitSet32 getShadowShow(){ return( shadowShow ); }
      BitSet32 getHilightShow(){ return( hilightShow ); }

      UInt8 getBlockFrameColor(){ return( blockFrameColor ); }
      UInt8 getSelectFrameColor(){ return( selectFrameColor ); }
      UInt8 getSelectFillColor(){ return( selectFillColor ); }
      UInt8 getHilightFrameColor(){ return( hilightFrameColor ); }
      UInt8 getHilightFillColor(){ return( hilightFillColor ); }
      UInt8 getShadowFrameColor(){ return( shadowFrameColor ); }
      UInt8 getShadowFillColor(){ return( shadowFillColor ); }
      void setTerrainType( int val ){ terrainType = val; };
      int getTerrainType(){ return( terrainType ); };
      void clearSelect() { currentSel.clear(); }
      void undo();
      void redo();
      void setScaleVal( float val ){ scaleVal = val; }
      void setHeightVal( float height ){ heightVal = height; }
      float getSmoothVal(){ return( smoothVal ); }
      void setSmoothVal( float smooth );
      void setAdjustVal( float height ){ adjustVal = height; }
      void setFlagVal( UInt8 val ){ flagVal = val; }
      void setMaterialIndex( UInt8 val ){ materialIndex = val; }
      UInt8 getMaterialIndex(){return(materialIndex);}
      void setUndoLimit( int limit ){ undoStack.setLimit( limit ); redoStack.setLimit( limit ); }
      void setPasteValue( UInt8 val ){ pasteVal = val; }
      UInt8 getPasteValue(){ return( pasteVal ); }
      bool mirrorTerrain(UInt32 sides);
      bool terrainAction(const char * action);
};

#endif