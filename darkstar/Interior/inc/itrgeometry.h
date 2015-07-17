//----------------------------------------------------------------------------


//----------------------------------------------------------------------------

#ifndef _ITRGEOMETRY_H_
#define _ITRGEOMETRY_H_

#include <base.h>
#include <ml.h>
#include <tplane.h>
#include <tvector.h>
#include <talgorithm.h>
#include <persist.h>


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

template <class T> 
class ITRVector: public Vector<T>
{
public:
	int add(const T&);
};

template <class T> 
int ITRVector<T>::add(const T& val)
{
	iterator i = ::find(begin(),end(),val);
	if (i != end())
		return i - begin();
	push_back(val);
	return (end() - begin()) - 1;
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

class ITRGeometry: public Persistent::VersionedBase
{
   UInt32 flags;           // general purpose flags...

public:
   enum {
      LowDetailInterior = 1 << 0,
   };

	struct Vertex
	{
		UInt16 pointIndex;		// Index into point3List
		UInt16 textureIndex;		// Index into point2List
	};

	struct Surface
	{
		enum Constants {
			NullMaterial = 255,
         textureScaleBits = 4
		};
		enum Type {
			Material,
			Link,
		};
		struct Point2 {
			UInt8 x, y;
		};
		//
		UInt8  type: 1;         // type
      UInt8  textureScaleShift: 4;   // amount to shift 
      UInt8  applyAmbient: 1;        // allow amibient lighting of surface?
      UInt8  visibleToOutside: 1;    // 1 : can be seen from the outside, 0: cannot
		UInt8  planeFront: 1;	// Which side of plane, true = front
		UInt8  material;			// Index into materialList
		Point2 textureSize;		// 1-256 (stored value + 1)
		Point2 textureOffset;	// Testure offset
		UInt16 planeIndex;		// Index into planeList
		UInt32 vertexIndex;		// Index into vertexList
		UInt32 pointIndex;		// Index into bitList
		UInt8  vertexCount;
		UInt8  pointCount;
      
	};

	struct BSPNode
	{
		UInt16 planeIndex;		// Index into planeList
		Int16 front;				// Node/Leaf in +plane, -(front+1) for leaf
		Int16 back;					// Node/Leaf in -plane, -(back+1) for leaf
		Int16 fill;
	};

  public:
	struct BSPLeafEmpty
	{
		enum Flags {
			External = 1,			// Node is outside bounding box
		};
		//
      
      UInt16 flags    : 1;
      UInt16 pvsCount : 15;   // 2
		UInt16 surfaceCount;    // 4

		UInt32 pvsIndex;			// 8  (Index into pvs bitset)
		UInt32 surfaceIndex;		// 12 (Index into bitList)
		UInt32 planeIndex;      // 16 (Index into bitlist)
		Box3F  box;             // 40 (Bounding box)
      UInt16 planeCount;      // 42
//      UInt16 volumeMask;      // 44
	};

   struct BSPLeafSolid
   {
      UInt32 surfaceIndex;
      UInt32 planeIndex;
      UInt16 surfaceCount;
      UInt16 planeCount;
   };

  public:
   class BSPLeafWrap
   {
      BSPLeafEmpty* m_pLeafEmpty;
      BSPLeafSolid* m_pLeafSolid;

     public:
      BSPLeafWrap(ITRGeometry* io_pGeometry,
                  int          in_leafIndex);
      ~BSPLeafWrap() { m_pLeafSolid = NULL; m_pLeafEmpty = NULL; }

      // These should ONLY be used by creation tools
     public:
      BSPLeafSolid* getSolidLeaf() { return m_pLeafSolid; }
      BSPLeafEmpty* getEmptyLeaf() { return m_pLeafEmpty; }

      // app level accessors
     public:
      UInt32 getPVSIndex()     const;
      UInt16 getPVSCount()     const;
      UInt32 getSurfaceIndex() const;
      UInt16 getSurfaceCount() const;
      UInt32 getPlaneIndex()   const;
      UInt16 getPlaneCount()   const;
      
      bool isSolid()    const;
      bool isExternal() const;
      void getBoundingBox(const Box3F*& in_rpBox) const;
   };

	//
	enum Constants {
		MaxIndex = 65530,
		//
		RootNode = 0,			// Root node in nodeList
		// The first 6 leaf index values are used to track
		// outside visibility from the interior.
		OutsideMinX = (0x80 >> 0),
		OutsideMinY = (0x80 >> 1),
		OutsideMinZ = (0x80 >> 2),
		OutsideMaxX = (0x80 >> 3),
		OutsideMaxY = (0x80 >> 4),
		OutsideMaxZ = (0x80 >> 5),
		OutsideMask =  0xFC,
		ReservedOutsideLeafs = 6,
		// The first leaf entries are reserved for outside leafs
		// (Interior visibility from outside the bounding box).
		// The index is obtained using externalLeaf()
		ReservedLeafEntries = 43,
	};
	//
	Vector<Surface> surfaceList;
	Vector<BSPNode> nodeList;
	Vector<BSPLeafSolid> solidLeafList;
	Vector<BSPLeafEmpty> emptyLeafList;
	Vector<UInt8>   bitList;
	//
	Vector<Vertex>     vertexList;
	ITRVector<Point3F> point3List;
	ITRVector<Point2F> point2List;
	ITRVector<TPlaneF> planeList;
	//
	Box3F box;					// Bounding box of geometryer
	float textureScale;		// World to texture coor.
	Int32 buildId;				// Unique ID assigned by build
   
   Int32 highestMipLevel;  // Highest detail level that textures will be drawn
                           //  at on this geometry...

	//
	ITRGeometry();
	~ITRGeometry();
	int findLeaf(const Point3F& p,int nodeIndex = 0);
	int externalLeaf(const Point3F& p);
	int getOutsideBits(BSPLeafWrap& rLeaf);

   Point2F getTextureCoord(int            surfaceIndex,
                                   const Point3F& worldPoint);

   bool testFlag(const UInt32 in_flag) const;
   void setFlag(const UInt32 in_flag);
   void clearFlag(const UInt32 in_flag);

	// Persistent IO
	DECLARE_PERSISTENT(ITRGeometry);
	Error read( StreamIO &, int version, int );
	Error write( StreamIO &, int version, int );
	int version();
};

inline bool 
ITRGeometry::testFlag(const UInt32 in_flag) const
{
   return (flags & in_flag) != 0;
}

inline void 
ITRGeometry::setFlag(const UInt32 in_flag)
{
   flags |= in_flag;
}

inline void 
ITRGeometry::clearFlag(const UInt32 in_flag)
{
   flags &= ~in_flag;
}

inline
ITRGeometry::BSPLeafWrap::BSPLeafWrap(ITRGeometry* io_pGeometry,
                                      int          in_leafIndex)
{
   if (in_leafIndex < ITRGeometry::ReservedLeafEntries) {
      // Leaf is a reserved external node...
      //
      m_pLeafSolid = NULL;
      m_pLeafEmpty = &io_pGeometry->emptyLeafList[in_leafIndex];
   } else if (in_leafIndex < io_pGeometry->solidLeafList.size() + ITRGeometry::ReservedLeafEntries) {
      // Leaf is solid
      //
      m_pLeafEmpty = NULL;
      m_pLeafSolid = &io_pGeometry->solidLeafList[in_leafIndex -
                                                  ITRGeometry::ReservedLeafEntries];
   } else {
      // Leaf is empty
      //
      m_pLeafSolid = NULL;
      m_pLeafEmpty = &io_pGeometry->emptyLeafList[in_leafIndex -
                                                  io_pGeometry->solidLeafList.size()];
   }
}

inline UInt32
ITRGeometry::BSPLeafWrap::getPVSIndex() const
{
   if (m_pLeafSolid != NULL)
      return 0;
   return m_pLeafEmpty->pvsIndex;
}

inline UInt16
ITRGeometry::BSPLeafWrap::getPVSCount() const
{
   if (m_pLeafSolid != NULL)
      return 0;
   return m_pLeafEmpty->pvsCount;
}

inline UInt32
ITRGeometry::BSPLeafWrap::getSurfaceIndex() const
{
   if (m_pLeafSolid != NULL)
      return m_pLeafSolid->surfaceIndex;
   return m_pLeafEmpty->surfaceIndex;
}

inline UInt16
ITRGeometry::BSPLeafWrap::getSurfaceCount() const
{
   if (m_pLeafSolid != NULL)
      return m_pLeafSolid->surfaceCount;
   return m_pLeafEmpty->surfaceCount;
}

inline UInt32
ITRGeometry::BSPLeafWrap::getPlaneIndex() const
{
   if (m_pLeafSolid != NULL)
      return m_pLeafSolid->planeIndex;
   return m_pLeafEmpty->planeIndex;
}

inline UInt16
ITRGeometry::BSPLeafWrap::getPlaneCount() const
{
   if (m_pLeafSolid != NULL)
      return m_pLeafSolid->planeCount;
   return m_pLeafEmpty->planeCount;
}

inline bool
ITRGeometry::BSPLeafWrap::isSolid() const
{
   return (m_pLeafSolid != NULL);
}

inline bool
ITRGeometry::BSPLeafWrap::isExternal() const
{
   if (m_pLeafSolid != NULL)
      return false;
   return (m_pLeafEmpty->flags & BSPLeafEmpty::External) != 0;
}

inline void
ITRGeometry::BSPLeafWrap::getBoundingBox(const Box3F*& in_rpBox) const
{
   AssertFatal(m_pLeafEmpty != NULL, "Invalid call on a solid leaf...");
   in_rpBox = &m_pLeafEmpty->box;
}


#endif
