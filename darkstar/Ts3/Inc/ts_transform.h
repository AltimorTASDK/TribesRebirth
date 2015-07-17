//---------------------------------------------------------------------------

//	$Workfile:   ts_mesh.h  $
//	$Revision:   2.8  $
// $Version$
//	$Date:   15 Sep 1995 10:27:12  $
//	$Log:   R:\darkstar\develop\ts\vcs\ts_mesh.h_v  $
//	

//---------------------------------------------------------------------------

#ifndef _TS_TRANSFORM_H_
#define _TS_TRANSFORM_H_

#include <tvector.h>
#include <ml.h>

// Once all shapes are exported with version #7, this can be set to FALSE,
//  and version #8 can be started.  
#define TRANS_USE_SCALE		0

namespace TS
{
#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

	// Transform for ts_shape version #6
	struct V6Transform
	{
      QuatF             fRotate;
      Point3F           fTranslate;
      Point3F           fScale;
	};
   
	// class V7Transform;
   

   //---------------------------------------------------------------------------
   // class Transform
   //    used to interpolate transforms for animation
   class Transform
   {
      friend class V7Transform;
      
		class Quat16
		{
			enum { MAX_VAL = 0x7fff };
		
			Int16 x, y, z, w;
		public:

			// would like to get rid of this, since it unpacks Quat16, only to
			//  repack.  Probably gets unpacked later to setMatrix also!
			//  Still used by in_transition code though.
			void interpolate( const Quat16 &src1, const Quat16 &src2, float interp )
			{
				QuatF q, q1, q2;
				q.interpolate( src1.getQuatF( &q1 ), src2.getQuatF( &q2 ), interp );
				set( q );
			} 
			void identity()
			{
				x = y = z = 0;
				w = MAX_VAL;
			}
			QuatF &getQuatF( QuatF *q ) const
			{
				q->x = float( x ) / MAX_VAL;
				q->y = float( y ) / MAX_VAL;
				q->z = float( z ) / MAX_VAL;
				q->w = float( w ) / MAX_VAL;
				return *q;
			}
			void set( const QuatF &q )
			{
				x = q.x * MAX_VAL;
				y = q.y * MAX_VAL;
				z = q.z * MAX_VAL;
				w = q.w * MAX_VAL;
			}
			int operator==( const Quat16 &q ) const
			{
				return( x == q.x && y == q.y && z == q.z && w == q.w );
			}
		};
		
      Quat16            fRotate;
      Point3F           fTranslate;
#if TRANS_USE_SCALE		
      Point3F           fScale;
#endif		

		static QuatF &interpolate( const Quat16 &src1, const Quat16 &src2, 
										   float interp, QuatF *q )
		{
			QuatF q1, q2;
			q->interpolate( src1.getQuatF( &q1 ), src2.getQuatF( &q2 ), interp );
			return *q;
		}
		
   public:
		void set( const V6Transform &old )
		{
			fRotate.set( old.fRotate );
			fTranslate = old.fTranslate;
#if TRANS_USE_SCALE			
			fScale = old.fScale;
#endif			
		}
		int operator==( const Transform &other ) const
		{	
			return ( (m_dist( fTranslate, other.fTranslate ) < 0.001) &&
#if TRANS_USE_SCALE
						(fScale == other.fScale) &&
#endif									
						(fRotate == other.fRotate) );
		}
		const Point3F &getTranslate() const { return fTranslate; }
		const QuatF &getRotate( QuatF *q ) const
		{
			return fRotate.getQuatF( q );
		}
		void setTranslate( Point3F &pt ) { fTranslate = pt; }
		void setRotate( QuatF &q ) { fRotate.set( q ); }
		void identity()
		{
			fRotate.identity();
			fTranslate.set( 0 );
#if TRANS_USE_SCALE			
			fScale.set( 1.0f, 1.0f, 1.0f );
#endif			
		}
		static TMat3F & interpolate( Transform const &src1, 
														Transform const &src2,
								   			  		RealF interpParam, TMat3F *mat );
		// would like to get rid of this, since it unpacks Quat16, only to
		//  repack.  Probably gets unpacked later to setMatrix also!
		//  Still used by in_transition code though.
      Transform& interpolate( Transform const & t1, Transform const & t2, 
												RealF t )
      {
         fRotate.interpolate( t1.fRotate, t2.fRotate, t );
         fTranslate.interpolate( t1.fTranslate, t2.fTranslate, t );
#if TRANS_USE_SCALE			
         fScale.interpolate( t1.fScale, t2.fScale, t );
#endif			
			return *this;
      }
      void setMatrix( TMat3F *pDest ) const
      {
			QuatF q;
	      pDest->set( fRotate.getQuatF( &q ), fTranslate );
#if TRANS_USE_SCALE			
         pDest->preScale( fScale );
#endif			
      }
      
#if TRANS_USE_SCALE		
      Point3F const & fetchScale(void) { return fScale; }
#endif		
      
   };
   
   
   // For the version 8 upgrade.  
	class V7Transform
	{
         Transform::Quat16    fRotate;
         Point3F              fTranslate;
      public: 
         Point3F              fScale;
         void AssignToNew( Transform & giveTo ) const 
         {
            giveTo.fRotate = fRotate;
            giveTo.fTranslate = fTranslate;
         }
	};


   //---------------------------------------------------------------------------
   // class TransformStack
   //    used by cameras to accumulate hierarchical transforms

   class TransformStack : public Vector<TMat3F>
      {
   public:
             TransformStack();
             ~TransformStack();

      virtual void push( TMat3F const & );
      void         pop();

      operator TMat3F const & () const;
      };

   //---------------------------------------------------------------------------
   // class PointStack
   //    used by cameras to save transformed camera centers

   class PointStack : public Vector<Point3F>
      {
   public:
             PointStack();
             ~PointStack();

      virtual void push( Point3F const & );
      void         pop();

      operator Point3F const & () const;
      };

   //---------------------------------------------------------------------------

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

   //---------------------------------------------------------------------------

   inline TransformStack::TransformStack()
      {
      increment();
      last().identity();
      }

   inline TransformStack::~TransformStack()
      {
      AssertWarn( size() == 1, 
         "TransformStack::~TransformStack: pushes and pops not balanced" );
      }

   //---------------------------------------------------------------------------

   inline void TransformStack::push( TMat3F const & mat )
      {
      if( empty() )
         push_back( mat );
      else
         {
         increment();
         m_mul( mat, *(&last() - 1), &last() );
         }
      }

   inline void TransformStack::pop()
      {
      pop_back();
      }

   //---------------------------------------------------------------------------

   inline TransformStack::operator TMat3F const & () const
      {
      return last();
      }

   //---------------------------------------------------------------------------

   inline PointStack::PointStack()
      {
      increment();
      last().set();
      }

   inline PointStack::~PointStack()
      {
      AssertWarn( size() == 1, 
         "PointStack::~PointStack: pushes and pops not balanced" );
      }

   inline void PointStack::push( Point3F const & pt )
      {
      push_back( pt );
      }

   inline void PointStack::pop()
      {
      pop_back();
      }

   inline PointStack::operator Point3F const & () const
      {
      return last();
      }

   //---------------------------------------------------------------------------
};  // namespace TS

#endif
