//----------------------------------------------------------------------------



//----------------------------------------------------------------------------

#ifndef _SIMCOLLISION_H_
#define _SIMCOLLISION_H_

#include <sim.h>
#include <m_collision.h>


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

class SimImageTransformQuery : public SimQuery
{
public:
   TMat3F transform;
   Int32  userData;

	SimImageTransformQuery() { type = SimImageTransformQueryType; }
};   



//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

class DLLAPI SimCollisionImage;

//----------------------------------------------------------------------------

class SimCollisionImageQuery: public SimQuery
{
public:
	enum ImageSize { MaxImage = 20 };
	SimCollisionImageQuery() {
		type = SimCollisionImageQueryType;
		detail = Normal;
	}
	enum Detail {
		Bounding,	// Bounding box
		Low,			// Used by what?
		Normal,		// Normally used in a game
		High,			// High-precision weapons
		Editor,		// Use by mission editor
	};
	// Arguments
	Detail detail;
	// Returns
   int count;
	SimCollisionImage* image[MaxImage];
};


//----------------------------------------------------------------------------

class DLLAPI SimCollisionInfo
{
public:
	float time;
	SimObject* object;
   SimCollisionImage* image;
   int imageIndex;
	CollisionSurfaceList surfaces;

	// Apply transform to point & normal
	SimCollisionInfo();
	void transform(TMat3F &m);
   // void destruct(void )        {  surfaces.~CollisionSurfaceList();   }
};

class SimCollisionInfoList: private Vector<SimCollisionInfo>
{
	typedef Vector<SimCollisionInfo> Base;

public:
   
	void transform(TMat3F &m);
   ~SimCollisionInfoList ();

   
   // 
   // Special list that has an increment() with construct.  
   // 
	typedef SimCollisionInfo * iterator;
	iterator begin()        {  return Base::begin();      }
	iterator end()          {  return Base::end();        }
   int  size() const       {  return Base::size();       }
	bool empty() const      {  return Base::empty();      }
   void clear ( void );
	SimCollisionInfo& operator[](int index) { return Base::operator[](index);  }
   
	SimCollisionInfo& increment (void )  {
		Base::increment();
		new(end()-1) SimCollisionInfo();
      return last ();
	}
   void decrement (void)   {
		(end()-1)->~SimCollisionInfo ();
      Base::decrement();
   }
   void erase (int index)   {
		operator[] (index).~SimCollisionInfo ();
      Base::erase(index);
   }
	SimCollisionInfo& last()   { return Base::last();     }
};


//----------------------------------------------------------------------------

class DLLAPI SimCollisionImage
{
public:
	enum Types {
		// Base types, type id also used for base
		// type priorities.
		Sphere,
		Box,
		Tube,
		Line,
		Terrain,
		Interior,
		TSShape,
		ApplicationType,
	};

protected:
	int type;
	int priority;

public:
	bool moveable;
	bool trigger;
	TMat3F transform;		// Object to world space

	//
	SimCollisionImage();
	int getType()  { return type; }
	int getPriority()  { return priority; }
	void buildImageTransform(SimCollisionImage*,TMat3F*);
	static bool test(SimCollisionImage*,SimCollisionImage*,
		CollisionSurfaceList* list);

	virtual ~SimCollisionImage();
	virtual bool collide(SimCollisionImage*,CollisionSurfaceList*) = 0;
};

typedef VectorPtr<SimCollisionImage*> SimCollisionImageList;



//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

class DLLAPI SimCollisionSphereImage: public SimCollisionImage
{
public:
	Point3F center;
	float radius;
	SimCollisionSphereImage();
	bool collide(SimCollisionImage*,CollisionSurfaceList*);
};

class DLLAPI SimCollisionBoxImage: public SimCollisionImage
{
public:
	Box3F box;
	SimCollisionBoxImage();
	bool collide(SimCollisionImage*,CollisionSurfaceList*);
};

class DLLAPI SimCollisionLineImage: public SimCollisionImage
{
public:
	Point3F start;
	Point3F end;
	SimCollisionLineImage();
	bool collide(SimCollisionImage*,CollisionSurfaceList*);
};

class DLLAPI SimCollisionTubeImage: public SimCollisionImage
{
public:
	Point3F start;
	Point3F end;
	float radius;
	SimCollisionTubeImage();
	bool collide(SimCollisionImage*,CollisionSurfaceList*);
};


#endif
