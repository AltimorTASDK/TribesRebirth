//----------------------------------------------------------------------------


//----------------------------------------------------------------------------

#include <stdio.h>
#include <tpoly.h>
#include <itrgeometry.h>
#include <itrportal.h>
#include <itrbit.h>


//----------------------------------------------------------------------------

namespace {

int
mungeOutsideBits(int in_bits)
{
   // Takes the outsideBits int, and renders it into a decimal number that
   //  prints as binary, with the exception of the placeholder 8
   //
   in_bits >>= 2;
   
   int retVal = 8000000;

   if ((in_bits & (1 << 5)) != 0)
      retVal += 100000;
   if ((in_bits & (1 << 4)) != 0)
      retVal += 10000;
   if ((in_bits & (1 << 3)) != 0)
      retVal += 1000;
   if ((in_bits & (1 << 2)) != 0)
      retVal += 100;
   if ((in_bits & (1 << 1)) != 0)
      retVal += 10;
   if ((in_bits & (1 << 0)) != 0)
      retVal += 1;

   return retVal;
}

}; // namespace {}


namespace ITRPortal
{
   struct Node;
   class Stack;
   class Portal;
   class VisStack;

   class Poly: public TPoly<TPolyVertex>
   {
   public:
      bool isOutside(TPlaneF& plane);
      bool isAnyInside(TPlaneF& plane);
      bool isAnyOutside(TPlaneF& plane);
   };
   typedef Vector<Portal*> PortalList;

   struct Portal: Poly
   {
      struct Face {
         Node* node;
         ITRBitVector portals;   // Visible portals (front & back faces)
         ITRBitVector vis;       // Possibly visible nodes
         ITRBitVector pvs;       // Visible nodes
         bool pvsDone;
      };
      Portal();
      ~Portal();
      Portal(TPlaneF* plane);
      TPlaneF* plane;            // Pointer into geometry planeList
      Face front;                // Front/back nodes
      Face back;
      int index;
      static PortalList portalList;
      //
      Node* getAlternate(Node*);
   };

   struct Node
   {
      typedef Vector<ITRBitVector*> PortalVis;
      typedef Vector<Node*> NodeList;

      TPlaneF* plane;            // Pointer into geometry planeList
      Node* front;
      Node* back;
      int nodeIndex;             // Original node index
      bool solid;                // Void or solid?
      bool external;             // Outside bounding box node
      PortalList portalList;     //
      UInt8 outsideBits;
      static int maxNodeIndex;
      //
      Node();
      ~Node();
      void portalize();
      void remove(Portal*);
      void pushFront(Portal*);
      void pushBack(Portal*);
      void buildPortalVis(NodeList& nodeList);
      void buildLeafVis(VisStack* stack);
      void traversePortals(Portal* rp,bool rback);
      void traversePortals(Stack* stack);
      void getPortalPVS(ITRBitVector* bits);
      void import(ITRGeometry* geometry,int nodeIndex);
      void exportToGeometry(ITRGeometry* geometry);

      void findIncidentLeaves(ITRPortal::Poly&, ITRBitVector&, int&);
      void markOutsideBits(ITRBitVector&, const int outsideFace);

      static void buildBoundingPlanes(const Box3F&);
      static TPlaneF sm_oplane[6];
   };

   class Stack
   {
      typedef ITRVector<TPlaneF> PlaneList;

      struct Element {
         ~Element();
         Portal* portal;
         Portal::Face* face;
         Poly* poly;             // Clipped node portal
         Poly* root;             // Clipped root portal
         ITRBitVector vis;       // Merge previous portal vis's.
         PlaneList planeList;    // Planes formed with previous portal
      };
      //
      VectorPtr<Poly*> polyBuffer;
      VectorPtr<Element*> elementBuffer;
      Vector<Element*> stack;    // Actual stack
      //
      bool findPlanes(Poly*,Poly*,PlaneList*);
      Element* newElement();
      void free(Element*);
      Poly* newPoly();
      void free(Poly*);
   public:
      Stack();
      ~Stack();
      bool isVisible(int portalIndex);
      bool newNodes(ITRBitVector* bits);
      bool push(Portal* portal,Node* node);
      int size();
      void pop();
   };

   //
   extern float planePolySize;
   Point3F planePolyCenter;
   PortalList Portal::portalList;
   int Node::maxNodeIndex;

   // Metrics
   int maxDepth;
   int totalTest;
   int totalPlaneClip;

   int degenerateBSPNodes;
   int coplanerPortals;

   //
   void markOutsideVisibleSurfaces(ITRGeometry* geometry);
   void buildPortals(Node*, const Box3F&);
   void buildPortalPVS(PortalList& pl);
   void buildBoundingBoxPoly(const TPlaneF& in_rPlane,
                             Poly&          out_rPoly);
};


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

bool ITRPortal::portalClipping = false;
float ITRPortal::distancePrecision = 0.01f;
float ITRPortal::normalPrecision = 0.0001f;
float ITRPortal::boxExpansion = 1.0f;
float ITRPortal::planePolySize = 1.0E8;


//----------------------------------------------------------------------------

void ITRPortal::buildPVS(ITRGeometry* geometry)
{
   TPlaneF::DistancePrecision = distancePrecision;
   TPlaneF::NormalPrecision = normalPrecision;

   printf("Building PVS\n");
   //
   Node* root = new Node;
   root->import(geometry,0);

   // Setup bounding box to avoid degenerate polytopes
   Box3F eBox = geometry->box;
   planePolySize    = m_hyp(eBox.len_x(), eBox.len_y(), eBox.len_z());
   planePolyCenter  = eBox.fMax;
   planePolyCenter += eBox.fMin;
   planePolyCenter *= 0.5f;

   // Build portals & PVS
   printf("   Building Portals\n");
   buildPortals(root, eBox);
   printf("   Building PVS\n");
   buildPortalPVS(Portal::portalList);

   // Build new external PVS.  For each of the external nodes, we find all leaf
   //  nodes that contact the appropriate face of the bounding box, and set the
   //  PVS of the external node to the union of the PVS sets of all such nodes.
   // 
   ITRBitVector extBits[6];
   for (int i = 0; i < 6; i++) {
      Poly        bboxPoly;
      buildBoundingBoxPoly(Node::sm_oplane[i], bboxPoly);
      for (int j = 0; j < 6; j++) {
         if (j == i)
            continue;
         
         bboxPoly.clip(Node::sm_oplane[j]);
      }
      
      // We should never lose this poly...
      AssertFatal(bboxPoly.vertexList.size() != 0, "??");
      
      // Find the leaves this poly is incident on...
      //
      int incidentLeafCount = 0;
      root->findIncidentLeaves(bboxPoly, extBits[i], incidentLeafCount);
      root->markOutsideBits(extBits[i], i);
   }

   // We have to repeat this process, to get the outside bits from the outside
   //  boxes point of view...
   //
   for (int i = 0; i < 6; i++) {
      Poly        bboxPoly;
      buildBoundingBoxPoly(Node::sm_oplane[i], bboxPoly);
      for (int j = 0; j < 6; j++) {
         if (j == i)
            continue;
         
         bboxPoly.clip(Node::sm_oplane[j]);
      }
      
      extBits[i].clear();
      int incidentLeafCount = 0;
      root->findIncidentLeaves(bboxPoly, extBits[i], incidentLeafCount);
   }

   // Build external PVS.  This assumes that the first 43
   // entries in the leaf node list have been reserved as
   // external leaf nodes.
   for (int p = 0; p < 43; p++) {
      ITRBitVector pvs;
      if (p & 1)  pvs.set(extBits[0]); // -x
      if (p & 2)  pvs.set(extBits[3]); // +x
      if (p & 4)  pvs.set(extBits[1]); // -y
      if (p & 8)  pvs.set(extBits[4]); // +y
      if (p & 16) pvs.set(extBits[2]); // -z
      if (p & 32) pvs.set(extBits[5]); // +z

      //
      ITRGeometry::BSPLeafWrap leafWrap(geometry, p);
      AssertFatal(leafWrap.isExternal(), 
                  "ITRPortal::buildPVS: Missing reserved external leaf node");
      ITRGeometry::BSPLeafEmpty* pELeaf = leafWrap.getEmptyLeaf();

      pELeaf->pvsIndex = geometry->bitList.size();
      pELeaf->pvsCount = pvs.compress(&geometry->bitList);
   }

   //
   root->exportToGeometry(geometry);
   delete root;

   markOutsideVisibleSurfaces(geometry);
}

void
ITRPortal::markOutsideVisibleSurfaces(ITRGeometry* geometry)
{
   ITRBitVector surfaceFinalSet;

   printf("   Building Surface outsideBits: ");
   
   surfaceFinalSet.setSize((geometry->surfaceList.size() >> 3) + 1);
   surfaceFinalSet.clear(geometry->surfaceList.size());
   
   for (int i = 1; i < 43; i++) {
      ITRGeometry::BSPLeafWrap leafWrap(geometry, i);
      AssertFatal(leafWrap.isExternal(),
                  "ITRPortal::markOutsideVisibleSurfaces: "
                  "Missing reserved external leaf node");
      
      surfaceFinalSet.uncompress(&geometry->bitList[leafWrap.getSurfaceIndex()],
                                                    leafWrap.getSurfaceCount());

      UInt8* pbegin = &geometry->bitList[leafWrap.getPVSIndex()];
      UInt8* pend   = pbegin + leafWrap.getPVSCount();
      for (ITRCompressedBitVector::iterator itr(pbegin,pend); ++itr; ) {
         // Don't bother processing any of the outside entries,
         // they are alway empty.
         if (*itr >= ITRGeometry::ReservedOutsideLeafs) {
            ITRGeometry::BSPLeafWrap vleafWrap(geometry, *itr);
            surfaceFinalSet.uncompress(&geometry->bitList[vleafWrap.getSurfaceIndex()],
                                                          vleafWrap.getSurfaceCount());
         }
      }

      printf(".");
   }
   printf("\n");

   int numVis = 0;
   for (int i = 0; i < geometry->surfaceList.size(); i++) {
      ITRGeometry::Surface& rSurface = geometry->surfaceList[i];

      rSurface.visibleToOutside = surfaceFinalSet.test(i) == true ? 1 : 0;
      numVis++;
   }
   printf("      %d Surface visible to outside out of %d\n", numVis, geometry->surfaceList.size());
}

void
ITRPortal::Node::findIncidentLeaves(ITRPortal::Poly& io_rPoly,
                                    ITRBitVector&    out_rPVS,
                                    int&             out_count)
{
   AssertFatal(io_rPoly.vertexList.size() != 0, "No poly passed...");
   if (!plane) {
      if (solid)
         return;

      out_count++;

      // Leaf, OR in PVS and terminate...
      //
      ITRBitVector newBits;
      getPortalPVS(&newBits);
      newBits.set(-(nodeIndex+1));
      
      for (int i = 0; i < 6; i++) {
         if ((outsideBits & (1 << i)) != 0) {
            newBits.set(i);
         }
      }
      
      if (out_rPVS.size() == 0) {
         out_rPVS.setSize(newBits.size());
         memset(out_rPVS.begin(), 0, newBits.size());
      }
      
      ITRBitVector::iterator src(newBits);
      for (; ++src; ) {
         out_rPVS.set(*src);
      }
   } else {
      // Node, split the poly, and recurse...
      //
      Poly newPolyFront;
      Poly newPolyBack;
      io_rPoly.split(*plane, &newPolyFront, &newPolyBack);
      
      if (newPolyFront.vertexList.size() != 0)
         front->findIncidentLeaves(newPolyFront, out_rPVS, out_count);
      if (newPolyBack.vertexList.size() != 0)
         back->findIncidentLeaves(newPolyBack, out_rPVS, out_count);
   }
}


void
ITRPortal::Node::markOutsideBits(ITRBitVector& in_rPVS,
                                 const int     in_outsideFace)
{
   if (plane) {
      front->markOutsideBits(in_rPVS, in_outsideFace);
      back->markOutsideBits(in_rPVS, in_outsideFace);
   } else {
      if (solid)
         return;
         
      int index = -(nodeIndex+1);
      if (in_rPVS.test(index) == true) {
         AssertFatal(outsideBits < (1 << 7), "Oy!");
         outsideBits |= (1 << in_outsideFace);
         AssertFatal(outsideBits < (1 << 7), "Oy!");
      }
   }
}

void
ITRPortal::buildBoundingBoxPoly(const TPlaneF& in_rPlane,
                                Poly&          out_rPoly)
{
   // Starting vector to project onto plane
   Vector3F vec(0.0f,0.0f,0.0f);
   if (fabs(in_rPlane.x) > fabs(in_rPlane.y) && fabs(in_rPlane.x) > fabs(in_rPlane.z))
      vec.z = 1.0f;
   else
      vec.x = 1.0f;

   // Closest point on plane to center point.
   Vector3F origin = in_rPlane;
   origin *= -in_rPlane.distance(planePolyCenter);
   origin += planePolyCenter;

   // Build 2 right-angle vectors on the plane
   Vector3F vec1;
   vec1 = in_rPlane;
   vec1 *= m_dot(in_rPlane,vec);
   vec1.neg();
   vec1 += vec;
   vec1.normalize();

   Vector3F vec2;
   m_cross(in_rPlane,vec1,&vec2);

   vec1 *= planePolySize;
   vec2 *= planePolySize;

   // Build vertices that represents the plane surface.
   TPolyVertex ver;
   ver.point = origin;
   ver.point -= vec1; ver.point -= vec2;
   out_rPoly.vertexList.push_back(ver);
   ver.point += vec2; ver.point += vec2;
   out_rPoly.vertexList.push_back(ver);
   ver.point += vec1; ver.point += vec1;
   out_rPoly.vertexList.push_back(ver);
   ver.point -= vec2; ver.point -= vec2;
   out_rPoly.vertexList.push_back(ver);
}



//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

bool ITRPortal::Poly::isOutside(TPlaneF& plane)
{
   // Is poly totally on the outside of plane
   bool outside = false;
   VertexList::iterator end = vertexList.end();
   for (VertexList::iterator itr = vertexList.begin(); itr != end; itr++) {
      TPlaneF::Side side = plane.whichSide((*itr).point);
      if (side == TPlaneF::Inside)
         return false;
      if (side == TPlaneF::Outside)
         outside = true;
   }
   return outside;
}

bool ITRPortal::Poly::isAnyInside(TPlaneF& plane)
{
   // Is any point of the poly on the inside of the plane
   VertexList::iterator end = vertexList.end();
   for (VertexList::iterator itr = vertexList.begin(); itr != end; itr++)
      if (plane.whichSide((*itr).point) == TPlaneF::Inside)
         return true;
   return false;
}

bool ITRPortal::Poly::isAnyOutside(TPlaneF& plane)
{
   // Is any point of the poly on the outside of the plane
   VertexList::iterator end = vertexList.end();
   for (VertexList::iterator itr = vertexList.begin(); itr != end; itr++)
      if (plane.whichSide((*itr).point) == TPlaneF::Outside)
         return true;
   return false;
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

ITRPortal::Node::Node()
{
   plane = 0;
   front = back = 0;
   outsideBits = 0;
}

ITRPortal::Node::~Node()
{
   while (!portalList.empty()) {
      Portal* portal = portalList.last();
      portal->front.node->remove(portal);
      portal->back.node->remove(portal);
      delete portal;
   }
   delete front;
   delete back;
   // Don't own the plane.
   // delete plane;
}  


//----------------------------------------------------------------------------

void ITRPortal::Node::remove(ITRPortal::Portal* p)
{
   PortalList::iterator itr = ::find(portalList.begin(),portalList.end(),p);
   AssertFatal(itr != portalList.end(),"??");
   *itr = portalList.last();
   portalList.decrement();
}

void ITRPortal::Node::pushFront(ITRPortal::Portal* p)
{
   if (p->front.node == this)
      p->front.node = front;
   else
      p->back.node = front;
   p->front.node->portalList.push_back(p);
   p->back.node->portalList.push_back(p);
}  

void ITRPortal::Node::pushBack(ITRPortal::Portal* p)
{
   if (p->front.node == this)
      p->front.node = back;
   else
      p->back.node = back;
   p->front.node->portalList.push_back(p);
   p->back.node->portalList.push_back(p);
}


//----------------------------------------------------------------------------

void ITRPortal::Node::import(ITRGeometry* geometry,int index)
{
   // Inport into temp. bsp tree
   external = false;
   nodeIndex = index;
   if (index >= 0) {
      ITRGeometry::BSPNode& bnode = geometry->nodeList[index];
      plane = &geometry->planeList[bnode.planeIndex];
      solid = false;
      front = new Node;
      back = new Node;
      front->import(geometry,bnode.front);
      back->import(geometry,bnode.back);
   }
   else {
      ITRGeometry::BSPLeafWrap leafWrap(geometry, -(index+1));
      solid = leafWrap.isSolid();
      plane = 0;
      maxNodeIndex = max(maxNodeIndex,-(index+1));
   }
}


//----------------------------------------------------------------------------

void ITRPortal::Node::exportToGeometry(ITRGeometry* geometry)
{
   // Only leaf nodes contain pvs data
   if (plane) {
      front->exportToGeometry(geometry);
      back->exportToGeometry(geometry);
   }
   else {
      AssertFatal(nodeIndex < 0,"ITRPortal::Node::exportToGeometry:"
         "Leaf node with plane.");
      // Build bitvec, compress, and store into geometry
      ITRGeometry::BSPLeafWrap leafWrap(geometry, -(nodeIndex+1));

      if (!solid) {
         // Union of all portal visibility, ourself, and outside bits...
         ITRBitVector bits;
         getPortalPVS(&bits);
         bits.set(-(nodeIndex+1));
         
         for (int i = 0; i < 6; i++) {
            if ((outsideBits & (1 << i)) != 0) {
               bits.set(i);
            } else {
               bits.clr(i);
            }
         }

         ITRGeometry::BSPLeafEmpty* pLeaf = leafWrap.getEmptyLeaf();
         pLeaf->pvsIndex = geometry->bitList.size();
         pLeaf->pvsCount = bits.compress(&geometry->bitList);
      }
   }
}


//----------------------------------------------------------------------------

void ITRPortal::Node::getPortalPVS(ITRBitVector* bits)
{
   for (int p = 0; p < portalList.size(); p++) {
      Portal* portal = portalList[p];
      bits->set((portal->front.node == this)?
         portal->back.pvs: portal->front.pvs);
   }
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

ITRPortal::Portal::Portal()
{
   portalList.push_back(this);
   front.node = back.node = 0;
   front.pvsDone = back.pvsDone = false;
   plane = 0;
}

ITRPortal::Portal::~Portal()
{
   if (this == portalList.last())
      portalList.decrement();
   else {
      PortalList::iterator itr = 
         ::find(portalList.begin(),portalList.end(),this);
      if (itr != portalList.end()) {
         *itr = portalList.last();
         portalList.decrement();
      }
   }
}


//----------------------------------------------------------------------------

ITRPortal::Portal::Portal(TPlaneF* p)
{
   portalList.push_back(this);
   front.node = back.node = 0;
   front.pvsDone = back.pvsDone = false;
   plane = p;

   // Starting vector to project onto plane
   Vector3F vec(0.0f,0.0f,0.0f);
   if (fabs(plane->x) > fabs(plane->y) && fabs(plane->x) > fabs(plane->z))
      vec.z = 1.0f;
   else
      vec.x = 1.0f;

   // Closest point on plane to center point.
   Vector3F origin = *plane;
   origin *= -plane->distance(planePolyCenter);
   origin += planePolyCenter;

   // Build 2 right-angle vectors on the plane
   Vector3F vec1;
   vec1 = *plane;
   vec1 *= m_dot(*plane,vec);
   vec1.neg();
   vec1 += vec;
   vec1.normalize();

   Vector3F vec2;
   m_cross(*plane,vec1,&vec2);

   vec1 *= planePolySize;
   vec2 *= planePolySize;

   // Build vertices that represents the plane surface.
   Vertex ver;
   ver.point = origin;
   ver.point -= vec1; ver.point -= vec2;
   vertexList.push_back(ver);
   ver.point += vec2; ver.point += vec2;
   vertexList.push_back(ver);
   ver.point += vec1; ver.point += vec1;
   vertexList.push_back(ver);
   ver.point -= vec2; ver.point -= vec2;
   vertexList.push_back(ver);
}  


//----------------------------------------------------------------------------

ITRPortal::Node* ITRPortal::Portal::getAlternate(ITRPortal::Node* node)
{
   return (node == front.node)? back.node: front.node;
}


//----------------------------------------------------------------------------

void ITRPortal::buildPortals(ITRPortal::Node* root, const Box3F& eBox)
{
   Node::buildBoundingPlanes(eBox);
   root->portalize();

   // Strip out portals connected to solid leaf nodes.
   for (int i = 0; i < Portal::portalList.size(); i++) {
      Portal* portal = Portal::portalList[i];
      if (portal->front.node->solid || portal->back.node->solid) {
         portal->front.node->remove(portal);
         portal->back.node->remove(portal);
         // Hack: Portal destructor checks the end of the list first.
         Portal::portalList[i] = Portal::portalList.last();
         *(Portal::portalList.end() - 1) = portal;
         delete portal;
         --i;
      }
   }
   // Assign the portals index numbers so we can 
   // identify them later.
   for (int p = 0; p < Portal::portalList.size(); p++)
      Portal::portalList[p]->index = p;

   printf("      Portals: %d\n",Portal::portalList.size());
   if (degenerateBSPNodes)
      printf("      *******: %d Degenerate BSP nodes\n",degenerateBSPNodes);
   if (coplanerPortals)
      printf("      *******: %d Coplaner portals\n",coplanerPortals);
}


//----------------------------------------------------------------------------

TPlaneF ITRPortal::Node::sm_oplane[6];

void
ITRPortal::Node::buildBoundingPlanes(const Box3F& in_rBox)
{
   // Build outside portals around the bounding box
   // Don't change the order of these planes without updating
   // ITRGeometry::Constants::OutsideMin/Max.
   sm_oplane[0].setYZ(in_rBox.fMin.x);
   sm_oplane[1].setXZ(in_rBox.fMin.y);
   sm_oplane[2].setXY(in_rBox.fMin.z);
   sm_oplane[3].setYZ(in_rBox.fMax.x); sm_oplane[3].neg();
   sm_oplane[4].setXZ(in_rBox.fMax.y); sm_oplane[4].neg();
   sm_oplane[5].setXY(in_rBox.fMax.z); sm_oplane[5].neg();
}


void ITRPortal::Node::portalize()
{
   int i;
   if (!plane)
      // Nothing to do in leaf nodes.
      return;

   // Build portal for this node.
   // Clip the new portal against all existing portal planes
   Portal* portal = new Portal(plane);

   // But first, clip the portal against all the bounding planes...
   //
   
   for (i = 0; i < 6; i++) {
      TPlaneF& plane = sm_oplane[i];
      portal->clip(plane);
   }

   for (i = 0; i < portalList.size(); i++) {
      TPlaneF plane = *portalList[i]->plane;
      if (portalList[i]->front.node != this)
         // Always orient so inside of plane is what we
         // want to keep.
         plane.neg();
      portal->clip(plane);
   }

   if (portal->vertexList.size() != 0) {
      portal->front.node = front;
      portal->back.node = back;
      front->portalList.push_back(portal);
      back->portalList.push_back(portal);
   }
   else {
      ++degenerateBSPNodes;
      delete portal;
   }

   // Move all the portals into the front or back
   // leaf nodes.
   while (!portalList.empty()) {
      Portal& portal = *portalList.last();
      portal.front.node->remove(&portal);
      portal.back.node->remove(&portal);
      TPlaneF::Side side = portal.whichSide(*plane);
      switch (side) {
         case TPlaneF::OnPlane:
            ++coplanerPortals;
            // Drop down to inside case
         case TPlaneF::Inside:
            pushFront(&portal);
            break;
         case TPlaneF::Outside:
            pushBack(&portal);
            break;
         case TPlaneF::Intersect: {
            Portal* fport = new Portal;
            Portal* bport = new Portal;
            *fport = *bport = portal;
            if (portal.split(*plane,fport,bport)) {
               pushFront(fport);
               pushBack(bport);
            }
            else {
               AssertFatal(0,"Node::portalize: Portal does not intersect");
            }
            delete &portal;
            break;
         }
      }
   }

   //
   front->portalize();
   back->portalize();
}  


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

int _USERENTRY portalCompare(const void *a,const void *b)
{
   const ITRPortal::Portal*const* pa = 
      reinterpret_cast<const ITRPortal::Portal*const*>(a);
   const ITRPortal::Portal*const* pb = 
      reinterpret_cast<const ITRPortal::Portal*const*>(b);
   return ((*pa)->front.node->portalList.size() + 
         (*pa)->back.node->portalList.size()) -
      ((*pb)->front.node->portalList.size() + 
         (*pb)->back.node->portalList.size());
}


//----------------------------------------------------------------------------

void ITRPortal::buildPortalPVS(ITRPortal::PortalList& portalList)
{
   // Build portal culling visibility
   printf("      Portal Culling\n"); fflush(stdout);
   for (int p = 0; p < portalList.size(); p++) {
      Portal* portal = portalList[p];
      //
      portal->front.portals.reserve(portalList.size() * 2);
      portal->front.vis.reserve(Node::maxNodeIndex);
      portal->front.node->traversePortals(portal,false);
      //
      portal->back.portals.reserve(portalList.size() * 2);
      portal->back.vis.reserve(Node::maxNodeIndex);
      portal->back.node->traversePortals(portal,true);
   }
   if (portalClipping) {
      maxDepth = totalTest = totalPlaneClip = 0;
      // Build portal PVS
      printf("      Portal PVS: "); fflush(stdout);
      int dot = (portalList.size() / 10) + 1;

      // Order portals
      PortalList pList;
      pList = portalList;
      qsort(pList.address(),pList.size(),sizeof(Portal*),portalCompare);

      //
      Stack stack;
      for (int p = 0; p < pList.size(); p++) {
         Portal* portal = pList[p];
         //
         portal->front.pvs.reserve(Node::maxNodeIndex);
         stack.push(portal,portal->front.node);
         portal->front.node->traversePortals(&stack);
         portal->front.pvsDone = true;
         stack.pop();
         //
         portal->back.pvs.reserve(Node::maxNodeIndex);
         stack.push(portal,portal->back.node);
         portal->back.node->traversePortals(&stack);
         portal->back.pvsDone = true;
         stack.pop();
         //
         if (p && !(p % dot))
            printf(".");
      }
      printf("\n");
      //
      int averageTest = 0;
      int averageClip = 0;
      if (portalList.size()) {
         totalTest / (portalList.size() * 2);
         totalPlaneClip / (portalList.size() * 2);
      }
      printf("         MaxDepth: %d\n", maxDepth);
      printf("         AvrgTest: %d\n", averageTest);
      printf("         AvrgClip: %d\n", averageClip);
   }
   else {
      printf("      Using Culled Vis for PVS\n"); fflush(stdout);
      for (int p = 0; p < portalList.size(); p++) {
         Portal* portal = portalList[p];
         portal->front.pvs.reserve(Node::maxNodeIndex);
         portal->front.pvs.set(portal->front.vis);
         portal->back.pvs.reserve(Node::maxNodeIndex);
         portal->back.pvs.set(portal->back.vis);
      }
   }
}


//----------------------------------------------------------------------------

void ITRPortal::Node::traversePortals(ITRPortal::Portal* rp,bool rback)
{
   Portal::Face *rface = rback? &rp->back: &rp->front;
   rface->vis.set(-(nodeIndex+1));

   // Traverse all portals visible to the root portal.
   for (int p = 0; p < portalList.size(); p++) {
      Portal* portal = portalList[p];
      bool pback = portal->front.node != this;

      // Mutually visible with root?
      if (!rface->portals.test((portal->index * 2) + pback)) {
         if ((!rback && !portal->isAnyInside(*rp->plane)) ||
            (rback && !portal->isAnyOutside(*rp->plane)))
            continue;
         if ((!pback && !rp->isAnyInside(*portal->plane)) ||
            (pback && !rp->isAnyOutside(*portal->plane)))
            continue;
      }

      // Mark visible portals.
      Portal::Face *pface = pback? &portal->back: &portal->front;
      pface->portals.set((rp->index * 2) + rback);
      rface->portals.set((portal->index * 2) + pback);

      // Recurse into nodes we haven't already hit
      Node* alt = portal->getAlternate(this);
      if (!rface->vis.test(-(alt->nodeIndex+1)))
         alt->traversePortals(rp,rback);
   }
}

void ITRPortal::Node::traversePortals(ITRPortal::Stack* stack)
{
   if (stack->size() > maxDepth)
      maxDepth = stack->size();

   // Traverse all visible portals.
   for (int p = 0; p < portalList.size(); p++) {
      Portal* portal = portalList[p];
      Node* alt = portal->getAlternate(this);
      bool pback = portal->front.node != this;
      if (stack->isVisible((portal->index * 2) + pback) &&
            stack->push(portal,alt)) {
         alt->traversePortals(stack);
         stack->pop();
      }
   }
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

ITRPortal::Stack::Element::~Element()
{
   delete poly;
   delete root;
}  

ITRPortal::Stack::Stack()
{
}

ITRPortal::Stack::~Stack()
{
   for (int i = 0; i < stack.size(); i++)
      delete stack[i];
   for (int i = 0; i < elementBuffer.size(); i++)
      delete elementBuffer[i];
   for (int i = 0; i < polyBuffer.size(); i++)
      delete polyBuffer[i];
}

bool ITRPortal::Stack::isVisible(int portalIndex)
{
   for (int i = 0; i < stack.size(); i++)
      if (!stack[i]->face->portals.test(portalIndex))
         return false;
   return true;
}

int ITRPortal::Stack::size()
{
   return stack.size();
}


//----------------------------------------------------------------------------

bool ITRPortal::Stack::newNodes(ITRBitVector* vis)
{
   // Returns true if vis contains some bits not
   // in the current PVS.
   if (vis->size() > stack.first()->face->pvs.size())
      return true;
   UInt8* p = stack.first()->face->pvs.begin();
   UInt8* b = vis->begin();
   for (int i = 0; i < vis->size(); i++)
      // Identify bits in *b not set in *p
      if ((p[i] & b[i]) ^ b[i])
         return true;
   return false;
}


//----------------------------------------------------------------------------

ITRPortal::Stack::Element* ITRPortal::Stack::newElement()
{
   Element* ep;
   if (elementBuffer.size()) {
      ep = elementBuffer.last();
      elementBuffer.decrement();
      ep->planeList.clear();
      ep->vis.clear();
   }
   else {
      ep = new Element;
      ep->planeList.reserve(10);
      ep->vis.reserve(Node::maxNodeIndex);
      ep->poly = newPoly();
      ep->root = newPoly();
   }
   return ep;
}

void ITRPortal::Stack::free(ITRPortal::Stack::Element* ep)
{
   elementBuffer.push_back(ep);
}


//----------------------------------------------------------------------------

ITRPortal::Poly* ITRPortal::Stack::newPoly()
{
   Poly* p;
   if (polyBuffer.size()) {
      p = polyBuffer.last();
      polyBuffer.decrement();
      p->vertexList.clear();
   }
   else {
      p = new Poly;
      p->vertexList.reserve(10);
   }
   return p;
}

void ITRPortal::Stack::free(ITRPortal::Poly* p)
{
   polyBuffer.push_back(p);
}


//----------------------------------------------------------------------------

void ITRPortal::Stack::pop()
{
   AssertFatal(stack.size() != 0,"??");
   free(stack.last());
   stack.decrement();
}


//----------------------------------------------------------------------------

bool ITRPortal::Stack::push(ITRPortal::Portal* portal,ITRPortal::Node* node)
{
   Portal::Face *face = (portal->front.node == node)?
       &portal->front: &portal->back;
   ITRBitVector* vis = face->pvsDone? &face->pvs: &face->vis;

   // Easy if the stack is empty
   if (!stack.size()) {
      Element* ep = newElement();
      ep->portal = portal;
      *ep->poly = *portal;
      *ep->root = *portal;
      ep->face = face;

      // Copy visibility set
      ep->vis.setSize(vis->size());
      memcpy(ep->vis.begin(),vis->begin(),ep->vis.size());
      ep->vis.clr(-(node->nodeIndex+1));

      // Add the portal plane itself.
      ep->planeList.add(*portal->plane);
      if (portal->front.node != node)
         ep->planeList.last().neg();

      // Append to list of visible nodes.
      face->pvs.set(-(node->nodeIndex+1));
      stack.push_back(ep);
      return true;
   }

   // Do some quick elimination.  Either its not possibly
   // visible, or the portal can't see anything not already
   // in the current PVS
   if (!stack.last()->vis.test(-(node->nodeIndex+1)))
      return false;
   if (stack.first()->face->pvs.test(-(node->nodeIndex+1)) &&
         !newNodes(vis))
      return false;
   totalTest++;

   // Clip the portal against the current stack and build
   // seperating planes with what's left over.
   Element* ep = newElement();
   ep->portal = portal;
   *ep->poly = *portal;
   ep->face = face;
   Poly* poly[2];
   poly[0] = ep->poly;
   poly[1] = newPoly();
   int pi = 0;

   // Clip portal against the last set of planes
   Element* prev = stack.last();
   for (int p = 0; p < prev->planeList.size(); p++, pi^=1) {
      totalPlaneClip++;
      if (!poly[pi]->clip(prev->planeList[p],poly[pi^1],false)) {
         // Portal not visible
         ep->poly = poly[0];
         free(poly[1]);
         free(ep);
         return false;
      }
   }
   ep->poly = poly[pi];
   free(poly[pi^1]);

   // Build seperating planes with last portal
   findPlanes(ep->poly,prev->poly,&ep->planeList);

   // Is the root visible throught the new set of planes?
   poly[0] = ep->root;
   poly[1] = newPoly();
   *poly[0] = *prev->root;
   pi = 0;

   for (int p = 0; p < ep->planeList.size(); p++, pi^=1) {
      totalPlaneClip++;
      TPlaneF plane = ep->planeList[p];
      plane.neg();
      if (!poly[pi]->clip(plane,poly[pi^1],false)) {
         // Root not visible
         ep->root = poly[0];
         free(poly[1]);
         free(ep);
         return false;
      }
   }
   ep->root = poly[pi];
   free(poly[pi^1]);

   // Build seperating planes with new root
   ep->planeList.clear();
   findPlanes(ep->poly,ep->root,&ep->planeList);
   stack.push_back(ep);

   // Build new visibility set
   int size = min(prev->vis.size(),vis->size());
   ep->vis.setSize(size);
   UInt8* dst = ep->vis.begin();
   UInt8* s1 = prev->vis.begin();
   UInt8* s2 = vis->begin();
   for (int i = 0; i < size; i++)
      dst[i] = s1[i] & s2[i];
   ep->vis.clr(-(node->nodeIndex+1));

   // Node is visible to all portals on the stack.
   // All nodes on the stack are visible to the back face of
   // the current portal
   face = (face == &portal->front)? &portal->back: &portal->front;
   for (int t = stack.size() - 1; t >= 0; t--) {
      Element& e = *stack[t];
      e.face->pvs.set(-(node->nodeIndex+1));
      Portal::Face* ef = (e.face == &e.portal->front)?
         &e.portal->back: &e.portal->front;
      face->pvs.set(-(ef->node->nodeIndex+1));
   }

   return true;
}


//----------------------------------------------------------------------------

bool ITRPortal::Stack::findPlanes(ITRPortal::Poly* p1,ITRPortal::Poly* p2,
   ITRPortal::Stack::PlaneList* planeList)
{
   // Find the seperating planes between p1 & p2 and append
   // them to the plane list
   int i1 = p1->vertexList.size() - 1;
   for (int i2 = 0; i2 < p1->vertexList.size(); i1 = i2++) {
      Point3F& v1 = p1->vertexList[i1].point;
      Point3F& v2 = p1->vertexList[i2].point;
      Point3F vec1 = v2;
      vec1 -= v1;
      for (int v = 0; v < p2->vertexList.size(); v++) {
         // Try and build plane with edge.
         Point3F normal,vec2 = p2->vertexList[v].point;
         vec2 -= v1;
         m_cross(vec1,vec2,&normal);
         if (!isEqual(normal.len(),.0f)) {
            TPlaneF plane;
            plane.set(v1,normal);

            // See if the plane seperates the two portals.
            TPlaneF::Side side = p1->whichSide(plane);
            if (side != TPlaneF::OnPlane && side != TPlaneF::Intersect) {
               if (side == TPlaneF::Outside)
                  plane.neg();
               if (p2->isOutside(plane)) {
                  planeList->add(plane);
                  break;
               }
            }
         }
      }
   }
   return planeList->size() != 0;
}

