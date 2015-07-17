//---------------------------------------------------------------------------

//	$Workfile:   ts_mesh.h  $
//	$Revision:   2.8  $
// $Version$
//	$Date:   15 Sep 1995 10:27:12  $
//	$Log:   R:\darkstar\develop\ts\vcs\ts_mesh.h_v  $
//	

//---------------------------------------------------------------------------

#ifndef _TS_BOUNDINGBOX_H_
#define _TS_BOUNDINGBOX_H_

#include <ml.h>

#include <ts_light.h>
#include <ts_camera.h>
#include <ts_RenderContext.h>

namespace TS
{
   //---------------------------------------------------------------------------

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

   class BoundingBox
   {
      //----------------------------------------------------
      // instance data:
   protected:
   	SphereF			         fSphere;
      Box3F                   fBox;
      //
      //----------------------------------------------------

      //----------------------------------------------------
      // instance methods:
   public:
      SphereF const & getSphere() const;
      Box3F const &   getBox() const;

      void            setSphere( SphereF const & sphere );
		void            setSphere( const Point3F & center, float radius );
		void            setSphere();
      void            setBox( Box3F const &box );
		void            setBox( const Point3F & min, const Point3F & max);
		void            setBox();

   	void 	render( RenderContext &rc, int color ) const;

   	// collision methods:
   	virtual Bool    collide( TMat3F const & tmat, 
   	   SphereF const & sphere ) const;
   	virtual Bool    collide( TMat3F const & tmat, 
   	   Box3F const & box ) const;
   	virtual Bool    collide( TMat3F const & tmat, 
   	   LineSeg3F const & lseg ) const;
   	virtual Bool    collide( TMat3F const & tmat, 
   	   BoundingBox const * bb ) const;

      int             pushVisibility( RenderContext & rc ) const;
      void            popVisibility( RenderContext & rc ) const;

      void            prepareLights( RenderContext &rc ) const;

      inline       BoundingBox();
      //
      //----------------------------------------------------
   };

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

   //---------------------------------------------------------------------------
   // BoundingBox inline methods
   //---------------------------------------------------------------------------

   inline BoundingBox::BoundingBox() : 
      fSphere( Point3F( 0.0f, 0.0f, 0.0f), 0.0f )
      {
      }

   //---------------------------------------------------------------------------

   inline void BoundingBox::setSphere( SphereF const & sphere )
      {
      fSphere = sphere;
      }

	inline void BoundingBox::setSphere( const Point3F & center, float radius )
	{
		fSphere.center = center;
		fSphere.radius = radius;
	}

	inline void BoundingBox::setSphere()
	{
		fSphere.center.x = 0.5f * (fBox.fMin.x+fBox.fMax.x);
		fSphere.center.y = 0.5f * (fBox.fMin.y+fBox.fMax.y);
		fSphere.center.z = 0.5f * (fBox.fMin.z+fBox.fMax.z);
		fSphere.radius = 0.5f * m_distf(fBox.fMin,fBox.fMax);
	}

   inline void BoundingBox::setBox( Box3F const & box )
      {
      fBox = box;
      }

	inline void BoundingBox::setBox( const Point3F & min, const Point3F & max)
	{
		fBox.fMin = min;
		fBox.fMax = max;
	}

	inline void BoundingBox::setBox()
	{
		fBox.fMin = fBox.fMax = fSphere.center;
		fBox.fMin.x -= fSphere.radius;
		fBox.fMin.y -= fSphere.radius;
		fBox.fMin.z -= fSphere.radius;
		fBox.fMax.x += fSphere.radius;
		fBox.fMax.y += fSphere.radius;
		fBox.fMax.z += fSphere.radius;
	}

   //---------------------------------------------------------------------------

   inline SphereF const & BoundingBox::getSphere() const
      {
      return fSphere;
      }

   inline Box3F const & BoundingBox::getBox() const
      {
      return fBox;
      }

   //---------------------------------------------------------------------------

   inline int BoundingBox::pushVisibility( RenderContext & rc ) const
      {
      int result = rc.getCamera()->testVisibility( fBox );
      rc.getCamera()->pushVisibility( result );
      return result;
      }
   
   //---------------------------------------------------------------------------

   inline void BoundingBox::popVisibility( RenderContext &rc ) const
      {
      rc.getCamera()->popVisibility();
      }

   //---------------------------------------------------------------------------

   inline void BoundingBox::prepareLights( RenderContext &rc ) const
      {
      rc.getLights()->prepare( fSphere, rc.getCamera()->getTOW() );
      }

   //---------------------------------------------------------------------------
}; // namespace TS

#endif
