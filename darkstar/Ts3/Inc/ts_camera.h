//---------------------------------------------------------------------------
//	$Workfile:   ts_mesh.h  $
//	$Revision:   2.8  $
//      $Version: $
//	$Date:   15 Sep 1995 10:27:12  $
//	$Log:   R:\darkstar\develop\ts\vcs\ts_mesh.h_v  $
//	

//---------------------------------------------------------------------------

#ifndef _TS_CAMERA_H_
#define _TS_CAMERA_H_

#include <ml.h>

#include <ts_types.h>
#include <ts_transform.h>


namespace TS
{
#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

enum CameraType
{
   PerspectiveCameraType,
   OrthographicCameraType
};

   class PackedVertex;
   class TransformedVertex;
   class BoundingBox;

extern "C" int __cdecl persp_transform_project(const Point3F *raw, TransformedVertex *pDest);
extern "C" int __cdecl ortho_transform_project(const Point3F *raw, TransformedVertex *pDest);
extern "C" void __cdecl transform_project_prepare(TMat3F const &toc, TMat3F const &tos, const Point2F *fVs, const Point2F *fVc, float);

   //---------------------------------------------------------------------------

   //
   //---------------------------------------------------------------------------
   // Tolerance in point and sphere visibility tests:
   //
   const int VisibilitySlack     = 3;


   class Camera
      {
      //----------------------------------------------------
      // instance data:
   protected:
      CameraType        type;
      TMat3F            fTCS;       // Camera to Screen - +viewport scaling
      TMat3F            fTWC;       // World to Camera  - Camera position, rot
      TMat3F            fTCW;       // Camera to World, inverse of fTWC

   private:
      PointStack        fCC;        // Camera center in object space

      TransformStack    fTOW;       // Object to World  - for lighting
   	TransformStack    fTOC;       // Object to Camera - for transform
	   TransformStack    fTOS;       // Object to Screen - for transform/project

      Vector<int>       fVisibility;

      Bool              fLocked;
		Bool              fWVPChanged; // Has the world viewport changed since last lock?

   protected:
      RectI             fScreenViewport;  // +y is down
      RectF             fWorldViewport;   // +y is up
      Point2F           fWs, fWc, fVs, fVc;
      RealF             fLeftPlane, fRightPlane, fTopPlane, fBottomPlane;
		// length of normals to planes
		RealF             fLeftLenSq, fRightLenSq, fTopLenSq, fBottomLenSq;
		RealF             fLeftLen, fRightLen, fTopLen, fBottomLen;

      RealF             fNearDist, fInvNearDist;
      RealF             fFarDist;
		RealF					fPixelScale;

      int (__cdecl *fptrTransformProject)(const Point3F *raw, TransformedVertex *pDest);
      void (__cdecl *fptrTransformProjectPrepare)( TMat3F const &toc, TMat3F const &tos, const Point2F *fVs, const Point2F *fVc, float invNearDist);
      //
      //----------------------------------------------------

      //----------------------------------------------------
      // instance methods:
   public:

      // Constructors/Destructors

   	Camera( const RectI & sViewport, const RectF & wViewport, 
   	   RealF nearDist, RealF farDist );
                   Camera();
                   ~Camera();

      // Lock/Unlock state

      void            lock();
      void            unlock();
      Bool            isLocked() const;

      // Transforms

      TMat3F const &  getTWC() const;
      TMat3F const &  getTCW() const;
      TMat3F const &  getTCS() const;
      TMat3F const &  getTOW() const;
      TMat3F const &  getTOC() const;
      TMat3F const &  getTOS() const;
      Point3F const & getCC() const;

      TMat3F const &  setTWC( TMat3F const & mat );
      void            pushTransform( TMat3F const & mat );
      void            popTransform();

      // Visibility
      void            pushVisibility( int );
      void            popVisibility();
      int             getVisibility() const;

      // View volume definition:
	   RealF           getNearDist() const;
	   RealF           getFarDist() const;
      RectF const &   getWorldViewport() const;

	   RealF           setNearDist( RealF d );
	   RealF           setFarDist( RealF d );

      RectF const &   setWorldViewport(const RectF& );

		RealF           getPixelScale();

      // Screen Viewport:

      RectI const &   getScreenViewport() const;
      RectI const &   setScreenViewport(const RectI& );

   protected:
      virtual void    buildProjectionMatrix() = 0;
      
   public:
		virtual void    getViewconeBox( Box3F & box, float farDist = -1.0f) const = 0;
      virtual RealF   projectRadius( RealF dist, RealF radius ) const = 0;
      virtual RealF   projectionDistance( RealF pixel,RealF radius ) const = 0;
      virtual RealF   transformProjectRadius( Point3F const &loc, 
                                 RealF radius ) const = 0;

      void transformProject2Inverse( Point4F const & src, Point4F *pDest ) const;
      void transformProject2Inverse( TransformedVertex *pVert ) const;
      inline void getVisibilityFlags( TransformedVertex *pVert) const;
      inline CameraType getCameraType() { return type; }

      // Transform Points
   public:
      // transform transforms from object to funky camera space.
      // also tests visibility.

      // Transform and Project Points
      int       transformProject( Int32 nRaw,
                                    PackedVertex const *pRaw,
                                    Point3F const & scale,
                                    Point3F const & origin,
                                    TransformedVertex *pDest ) const;

      int       transformProject( Int32 nRaw,
                                    Point3F const *pRaw,
                                    TransformedVertex *pDest ) const;

      int       transformProject( Point3F const &raw, 
                                    TransformedVertex *pDest ) const;

   	// Determine Visibility: (point, sphere, and bounding box)

      int testVisibility( Point3F const &point ) const;
      int testVisibility( SphereF const &bs ) const;
      int testVisibility( Box3F const &box, float fardist = -1 ) const;
      int testVisibility( BoundingBox const &bb ) const; // obsolete, use Box3F
      void project(TransformedVertex *pDest) const;
      };

      inline int Camera::transformProject( Point3F const &raw, 
                                    TransformedVertex *pDest ) const
      {
         fptrTransformProjectPrepare(fTOC, fTOS, &fVs, &fVc, fInvNearDist);
         return fptrTransformProject(&raw, pDest); 
      }

   //---------------------------------------------------------------------------

   class PerspectiveCamera : public Camera 
      {
      //----------------------------------------------------
      // instance methods:
   protected:
      void buildProjectionMatrix();
   public:
      void transform( Point3F const &raw, TransformedVertex *pDest ) const;
      PerspectiveCamera( const RectI & sViewport, 
         const RectF & wViewport, RealF nearDist, RealF farDist );

      void               castRay(Point2I &pixel, Point3F *ray);
		void    getViewconeBox( Box3F & box, float farDist = -1.0f) const;

      RealF projectRadius( RealF dist, RealF radius ) const;
      RealF projectionDistance( RealF pixel,RealF radius ) const;
      RealF transformProjectRadius( Point3F const &loc, 
                                 RealF radius ) const;
      //
      //----------------------------------------------------
      };

   //---------------------------------------------------------------------------

   class OrthographicCamera : public Camera 
   {
      //----------------------------------------------------
      // instance methods:
   protected:
      void buildProjectionMatrix();
   public:
      void transform( Point3F const &raw, TransformedVertex *pDest ) const;
      OrthographicCamera( const RectI & sViewport, 
         const RectF & wViewport, RealF nearDist, RealF farDist );
      RealF projectRadius( RealF dist, RealF radius ) const;
      RealF projectionDistance( RealF pixel,RealF radius ) const;
      RealF transformProjectRadius( Point3F const &loc, 
         RealF radius ) const;
		void    getViewconeBox( Box3F & box, float farDist = -1.0f) const;
      //
      //----------------------------------------------------
   };

   //---------------------------------------------------------------------------

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

   //---------------------------------------------------------------------------
   // Camera methods
   //---------------------------------------------------------------------------

   //---------------------------------------------------------------------------
   // constructors & destructors

   inline Camera::Camera( const RectI & sViewport, const RectF & wViewport, 
   	   RealF nearDist, RealF farDist ) 
      {
      fLocked = FALSE;
      setScreenViewport( sViewport );
      setWorldViewport( wViewport );
      setNearDist( nearDist );
      setFarDist( farDist );

      // put the camera at the origin looking up the y axis:
      fTWC.identity();
      fTCW.identity();

      // set visibility unknown:
      fVisibility.push_back( ClipUnknown );

      fLocked = TRUE;   // push wants to see that the camera is locked
      // set initial object transform to identity:
      fTOW.push( fTWC );
      fLocked = FALSE;

      }

   inline Camera::~Camera()
      {
      AssertFatal( !isLocked(),
         "TS::Camera::~Camera: camera is locked" );
      fTOW.pop();
      }

   //---------------------------------------------------------------------------
   // visibility & clipping

   inline void Camera::pushVisibility( int vis )
      {
      AssertFatal( isLocked(), "TS::Camera::pushVisibility: camera not locked" );
      fVisibility.push_back( vis );
      }

   inline void Camera::popVisibility()
      {
      AssertFatal( isLocked(), "TS::Camera::popVisibility: camera not locked" );
      fVisibility.pop_back();
      }

   inline int Camera::getVisibility() const
      {
      AssertFatal( isLocked(), "TS::Camera::getVisibility: camera not locked" );
      return fVisibility.last();
      }

   //---------------------------------------------------------------------------
   // lock state

   inline Bool Camera::isLocked() const
      {
      return fLocked;
      }

   //---------------------------------------------------------------------------
   // push & pop transforms

   inline void Camera::pushTransform( TMat3F const & mat )
      {
      AssertFatal( isLocked(), "TS::Camera::pushTransform: camera not locked" );

      // accumulate transforms:
      fTOW.push( mat );
      fTOC.push( mat );
      fTOS.push( mat );

      // we also need to keep track of the camera's location in object space:
      TMat3F temp = fTOC;
      temp.inverse();
      fCC.push( temp.p );
      }

   inline void Camera::popTransform()
      {
      AssertFatal( isLocked(), "TS::Camera::popTransform: camera not locked" );

      fTOW.pop();
      fTOC.pop();
      fTOS.pop();
      fCC.pop();
      }

   //---------------------------------------------------------------------------
   // setting world to camera transform

   inline TMat3F const & Camera::setTWC( TMat3F const & mat )
      { 
      AssertFatal( !isLocked(),
         "TS::Camera::setTWC: must call unlock() first" );
      fTCW = mat;
      fTCW.inverse();
      return fTWC = mat;
      }

   //---------------------------------------------------------------------------
   // access methods for all transforms &  object space camera center

   inline TMat3F const & Camera::getTWC() const
      { 
      return fTWC;
      }

   inline TMat3F const & Camera::getTCW() const
      { 
      return fTCW;
      }

   inline TMat3F const & Camera::getTCS() const
      {
      AssertFatal( isLocked(),
         "TS::Camera::getTCS: must call lock() first" );

      return fTCS;
      }

   inline TMat3F const & Camera::getTOW() const
      {
      AssertFatal( isLocked(),
         "TS::Camera::getTOW: must call lock() first" );

      return fTOW;
      }

   inline TMat3F const & Camera::getTOC() const
      {
      AssertFatal( isLocked(),
         "TS::Camera::getTOC: must call lock() first" );

      return fTOC;
      }

   inline TMat3F const & Camera::getTOS() const
      {
      AssertFatal( isLocked(),
         "TS::Camera::getTOS: must call lock() first" );

      return fTOS;
      }

   inline Point3F const & Camera::getCC() const
      {
      AssertFatal( isLocked(),
         "TS::Camera::getCC: must call lock() first" );

      return fCC; 
      }

   //---------------------------------------------------------------------------
   // near and far clip distances

   inline RealF Camera::getNearDist() const
      {
      return fNearDist; 
      }

   inline RealF Camera::getFarDist() const
      {
      return fFarDist; 
      }

    inline RealF Camera::setNearDist( RealF d )
      {
      AssertFatal( !isLocked(),
         "TS::Camera::setNearDist: must call unlock() first" );
      AssertFatal( d > 0.00001,
         "TS::Camera::setNearDist: distance must be > 0" );
      fInvNearDist = 1.0f / d;
		fWVPChanged = true;
      return fNearDist = d; 
      }

   inline RealF Camera::setFarDist( RealF d )
      {
      AssertFatal( !isLocked(),
         "TS::Camera::setFarDist: must call unlock() first" );
      fFarDist = d; 
		fWVPChanged = true;
		return d;
      }

   //---------------------------------------------------------------------------
   // viewports

   inline const RectI & Camera::getScreenViewport() const
      {
      return fScreenViewport;
      }

   inline const RectI & Camera::setScreenViewport(const RectI& rect)
      {
      AssertFatal( !isLocked(),
         "TS::Camera::setScreenViewport: must call unlock() first" );

      fScreenViewport = rect;
      // calculate size and center representation:
      fVs.x = fScreenViewport.len_x() * 0.5f;
      fVs.y = fScreenViewport.len_y() * (-0.5f);

      fPixelScale = 640.0f / fScreenViewport.len_x();

      // adjust center offset by small value so that it doesn't flicker along
      // the left and top edges.

      fVc.x = (fScreenViewport.upperL.x + fScreenViewport.lowerR.x) * 0.5f - .00001f;
      fVc.y = (fScreenViewport.upperL.y + fScreenViewport.lowerR.y) * 0.5f - .00001f;
      return fScreenViewport;
      }

   inline const RectF & Camera::getWorldViewport() const
      {
      return fWorldViewport;
      }

   inline const RectF & Camera::setWorldViewport(const RectF& rect)
      {
      AssertFatal( !isLocked(),
         "TS::Camera::setWorldViewport: must call unlock() first" );

      fWorldViewport = rect;
		fWVPChanged = true;
      // calculate size and center representation:
      fWs.x = fWorldViewport.len_x() * 0.5f;
      fWs.y = -fWorldViewport.len_y() * 0.5f;   // rect has +down, we want +up
      fWc.x = (fWorldViewport.upperL.x + fWorldViewport.lowerR.x) * 0.5f;
      fWc.y = (fWorldViewport.upperL.y + fWorldViewport.lowerR.y) * 0.5f;
      return fWorldViewport;
      }

	inline RealF Camera::getPixelScale()
	   {
		// Pixel size / standard window size (640)
		return fPixelScale;
	   }


   //---------------------------------------------------------------------------
   // PerspectiveCamera methods
   //---------------------------------------------------------------------------

   //---------------------------------------------------------------------------
   // constructors/destructors

   inline PerspectiveCamera::PerspectiveCamera( 
      const RectI & sViewport, const RectF & wViewport, 
      RealF nearDist, RealF farDist ) 
      : Camera( sViewport, wViewport, nearDist, farDist )
      {
         fptrTransformProject = persp_transform_project;
         fptrTransformProjectPrepare = transform_project_prepare;
      }

   //---------------------------------------------------------------------------
   // OrthographicCamera methods
   //---------------------------------------------------------------------------
   
   //---------------------------------------------------------------------------
   // constructors/destructors

   inline OrthographicCamera::OrthographicCamera( 
      const RectI & sViewport, const RectF & wViewport, 
      RealF nearDist, RealF farDist ) 
      : Camera( sViewport, wViewport, nearDist, farDist )
   {
         fptrTransformProject = ortho_transform_project;
         fptrTransformProjectPrepare = transform_project_prepare;
   }

   //---------------------------------------------------------------------------
}; // namespace TS

//
typedef TS::Camera TSCamera;
typedef TS::PerspectiveCamera TSPerspectiveCamera;
typedef TS::OrthographicCamera TSOrthographicCamera;

#endif
