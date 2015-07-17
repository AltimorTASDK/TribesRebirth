/*----------------------------------------------------------------------

   simVolumetric.h
 
   Class for things like liquids, lava, etc.  
   
   simVolumetric has a sub-class - BoxRenderDmlImage (a class originally 
      modified from simObjects\BoxRender.h).  

----------------------------------------------------------------------*/

#ifndef _SIMVOLUMETRIC_H_
#define _SIMVOLUMETRIC_H_

// in Sim\Inc: 
#include <simContainer.h>
// #include <simMovement.h>

// in SimOjbects\Inc:
// #include <boxRenderBma.h>


//--------------------------------------------------------------------------- 
class SimMovement;

class SimVolumetric : public SimDefaultOpenContainer 
{
   typedef SimDefaultOpenContainer Parent;
       
   private:
   
      class BoxRenderDmlImage : public SimRenderImage
      {
         private:
            struct TopView
            {
               Point3F a, b, c, d;
            };

            enum 
            { 
               ALPHAPAL_INDEX = 1976,
               COLOR_WATER_TD = 244, 
               COLOR_WATER_LR, 
               COLOR_WATER_FB, 
            };
            
            typedef enum 
            {
               XY,
               YZ,
               ZX,
            } WhichAxis;
    
            void clipHaze    (TS::TransformedVertex &, Point3F);
            void buildTopView(Point3F, Point3F);
            void drawSquare  (Point3F, Point3F, Point3F, Point3F, TS::PointArray *, WhichAxis);
            void drawCube    (const Point3F &, const Point3F &, 
                              TSRenderContext &, bool, bool);

            void set_text_coords(const Point3F &, const Point3F &, 
                                 const Point3F &, const Point3F &, WhichAxis);
    
            Vector3F         cameraPos;
            float            rVisibleDistance, rHazeDistance;
            float            rDelta, rHazeScale;
            TopView        **tvView;
            TSMaterialList  *Dml;
            Int16            curBmpIndex;
            BitSet16         flags;
            Point3F          oldP0, oldP1;
      
         public:

            enum                    // Note the order of the bits must match
            {                       // the order of the character array that
               TOP = BIT(0),        // defines available flags.  (The user
               DOWN = BIT(1),       // enters upper case flags..  A lower
               LEFT = BIT(2),       // case prompt can thus be given...)
               RIGHT = BIT(3),
               FRONT = BIT(4),
               BACK = BIT(5),
         
               ALL = (TOP | DOWN | LEFT | RIGHT | FRONT | BACK),
         
               NOTEXTURE = BIT(10),       // not present, or not wanted. 
               TEXTURING = BIT(11),       // 
               COLORING = BIT(12),
               CANWRAP = BIT(13),         // Texture wrapping supported. 
            };
            Int32 iSubDivisions, iOldDivs;
            void setBmpIndex ( Int16 index )  
                        { curBmpIndex = index;          }
            void flagSet ( BitMask16 whichBit )        
                        { flags.set ( whichBit );       }
            BitSet16 flagsGet ( void )  
                        {  return flags;                }
            void flagClr ( BitMask16 whichBit )        
                        { flags.clear ( whichBit );     }
            void faceOn ( BitMask16 whichFace )
                        { flagSet ( whichFace );           }
            void faceOff ( BitMask16 whichFace )
                        { flagClr ( whichFace );           }
            bool drawFace ( BitMask16 whichFace )
                        { return(flags.test(whichFace|COLORING));  }
            bool coloring ( void )                  
                        { return(flags.test(COLORING));  }
            bool texturing ( void )                 
                        { return(flags.test(TEXTURING));  }
            void texturesOn ( void )      { 
                                              flagSet ( TEXTURING );
                                              flagClr ( COLORING );
                                          }
            void texturesOff ( void )     {
                                              flagSet ( COLORING );
                                              flagClr ( TEXTURING );
                                          }

            bool faceTest ( BitMask16 whichFace )   
                        { return(flags.test(whichFace)); }

            TMat3F            transform;
            Point3F           dimensions;
            float             textureScale;
            float             transLevel;
            float             boxExtra;
            SimManager       *manager;


            BoxRenderDmlImage();
            ~BoxRenderDmlImage();
         
      
            // really should set BMP and have owner animate...
            void setDml ( TSMaterialList * dml )     { Dml = dml; }

            void render ( TSRenderContext &rc );
             
      }; // end class BoxRenderDmlImage 

      typedef enum {
         EDITING,
         PLAYING
      } DisplayMode;

      enum NetMaskBits
      {
         Moved = BIT(0),
         // Resized = BIT(1),
         Edited = BIT(1),
         
         NumberOfNetFlags = 2, 
      };
      
      
      enum SimVolumetricFlags
      {
          Awake = BIT(0), 
      };

      BitSet16                      flags;
      DisplayMode                   mode;
      Resource<TSMaterialList>      matList;
      // SimCollisionBoxImage       collisionImage;


      void        set_bounding_box();        // inform parent
      Box3F       get_display_box ( void );
      UInt16      string_to_bits ( const char * str );
      char  *     bits_to_string ( char modify [], UInt16 flags );
      bool        setFileName ( const char* fileName );
      bool        enable_editing ( void );
      bool        enable_playing ( void );
      void        loadDml ( void );
      void        syncBoxes ();


   protected:          // data.
      BoxRenderDmlImage       image;
      // axe this, it's in base.  
      // Box3F                   boundingBox;               // Ml\inc\M_BOX.H
      char                    *dmlFileName;

      float                   animTimeScale;
      float                   damagePerSec;
      Point3F                 currentVelocity;     // as in Water current
      float                   currentDrag;  
      float                   containerDrag;
      float                   density;
      Int32                   volType;


   protected:          // members.
    
      bool onSimCollisionImageQuery(SimCollisionImageQuery* query);
      bool onSimObjectTransformQuery(SimObjectTransformQuery *query);
      bool onSimRenderQueryImage(SimRenderQueryImage* query);
      bool onSimImageTransformQuery(SimImageTransformQuery* query);
      void setWakeUp ( float );
        // ADD COLLISION QUERY
      bool onSimObjectTransformEvent(const SimObjectTransformEvent *event);
      bool onSimEditEvent(const SimEditEvent *event);    
      // OLD_TIMER_CODE
      // bool addObject ( SimContainer * );
      // bool onSimTimerEvent ( const SimTimerEvent * event );
      const char * fetchTagString(Int32 Id);    // fetch from tag dictionary
      void applyInfluence ( SimMovement * mobj );
      bool updateObject ( SimContainer* obj );

      // Container database virtuals
      void enterContainer(SimContainer*);
      void leaveContainer(SimContainer*);

   public:
   
      SimVolumetric ();
      ~SimVolumetric ();

      const TMat3F& fetchTransform()  { return ( image.transform ); }
      void initTransform ( const Point3F * pos = NULL );
      void initTransform ( Point3F & pos)  { initTransform ( &pos ); }

      bool    processEvent(const SimEvent*);
      bool    processQuery(SimQuery*);
      bool    onAdd();
      void    getAlphaBlend(TMat3F &objectPos, ColorF *alphaColor, float *alphaBlend);
        
      void    inspectRead(Inspect* inspect);
      void    inspectWrite(Inspect* inspect);

      DWORD   packUpdate(Net::GhostManager *gm, DWORD mask, BitStream *stream);
      void    unpackUpdate(Net::GhostManager *gm, BitStream *stream);

      DECLARE_PERSISTENT(SimVolumetric);
      Persistent::Base::Error read(StreamIO &, int a, int b);
      Persistent::Base::Error write(StreamIO &, int a, int b);
};



/*
    Objects that inherit could configure or supply:
        Player type for client side containerMask.
        Additional currents with ID values.  Supply a vector?  
        Same for damages and drags.  
*/




#endif
