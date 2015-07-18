//----------------------------------------------------------------------------


//----------------------------------------------------------------------------

#include <stdio.h>
#include <itrbit.h>
#include <itrgeometry.h>
#include <itrbsp.h>
#include <tpoly.h>


//----------------------------------------------------------------------------

namespace ITRBSPBuild
{
   struct Poly: TPoly<TPolyVertex>
   {
      int planeIndex;         // Buffer planeList
      int planeFront;         // Front or backside of plane
      int surfaceIndex;       // Buffer surfaceList
      UInt32 volumeMask;      //
      bool splitable;         // Ok to split this poly?
      bool used;              // Used as a BSP plane.
      bool backface;          // Backface of a surface.
      bool nullMaterial;      // Surface has a null Material
   };

   struct Node
   {
      bool solid;             // only used in leaves, is solid?
      static int nodeCount;   // Total nodes constructed
      int planeIndex;         // Buffer planeList
      Node* front;
      Node* back;
      Box3F box;              // Bounding box
      Vector<Poly*> polyList;
      //
      Node();
      ~Node();
      bool pickPlane(ITRGeometry*);
      void splitNode(ITRGeometry*);
      void setBoundingBox();
      void import(ITRGeometry* geometry,Vector<UInt32>* volumeMasks);
      int  exportToGeometry(ITRGeometry* geometry, int&, int&);
   };

   inline float fmax(float a, float b) { return (a > b)? a: b; }

   enum Constants {
      NullMaterial = -1,
   };

   //
   int polyPlaneMismatch;
   bool g_isLinked;
};


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

float ITRBSPBuild::distancePrecision = 0.01f;
float ITRBSPBuild::normalPrecision = 0.0001f;


//----------------------------------------------------------------------------

int maxSurfacesInNode = 0;

bool ITRBSPBuild::buildTree(ITRGeometry* geometry,Vector<UInt32>* volumeMasks, bool isLinked)
{
   g_isLinked = isLinked;

   TPlaneF::DistancePrecision = distancePrecision;
   TPlaneF::NormalPrecision = normalPrecision;
   Node::nodeCount = 0;

   // Assign build time.
   geometry->buildId = GetTickCount();

   printf("Building BSP\n");
   // Convert surfaces into BSP polys.
   Node* root = new Node;
   root->import(geometry,volumeMasks);

   // Build the tree.
   int solidLeafs    = 0;
   int emptyLeafs    = 0;
   int leafPolyCount = 0;
   VectorPtr<Node*> nodeStack;
   nodeStack.push_back(root);
   while (!nodeStack.empty()) {
      Node* node = nodeStack.last();
      nodeStack.decrement();
      if (node->pickPlane(geometry)) {
         node->front = new Node;
         node->back = new Node;
         node->splitNode(geometry);
         nodeStack.push_back(node->front);
         nodeStack.push_back(node->back);
      }
      else {
//         bool solid = false;
//         for (int i = 0; i < node->polyList.size(); i++) {
//            if (node->polyList[i]->backface     == true ||
//                (isLinked == true && node->polyList[i]->nullMaterial == true)) {
//               solid = true;
//               break;
//            }
//         }
         bool solid = true;
         for (int i = 0; i < node->polyList.size(); i++) {
            if (node->polyList[i]->backface == false &&
                (node->polyList[i]->nullMaterial == false || isLinked == false)) {
               solid = false;
               break;
            }
         }

         node->solid = solid;
         if (solid) {
            solidLeafs++;
         } else {
            emptyLeafs++;
            // Getting a count of visible surfaces after
            // being split by BSP.
            leafPolyCount += node->polyList.size();
         }
      }
   }

   root->setBoundingBox();
   geometry->box = root->box;

   // Reserve bounding outside leaf nodes
   geometry->emptyLeafList.setSize(ITRGeometry::ReservedLeafEntries + emptyLeafs);
   geometry->solidLeafList.setSize(solidLeafs);

   ITRGeometry::BSPLeafEmpty bleaf;
   bleaf.flags        = ITRGeometry::BSPLeafEmpty::External;
   bleaf.pvsCount     = bleaf.pvsIndex = 0;
   bleaf.surfaceCount = bleaf.surfaceIndex = 0;
   bleaf.planeCount   = bleaf.planeIndex = 0;
   bleaf.box          = root->box;
   for (int i = 0; i < ITRGeometry::ReservedLeafEntries; i++)
      geometry->emptyLeafList[i] = bleaf;

   // Stuff the BSP tree into the geometry.
   // The root node will end up as the first entry in the
   // nodeList.
   geometry->nodeList.setSize(0);
   geometry->nodeList.reserve(Node::nodeCount/2);
   int currEmptyLeaf = solidLeafs + ITRGeometry::ReservedLeafEntries;
   int currSolidLeaf = ITRGeometry::ReservedLeafEntries;

   root->exportToGeometry(geometry, currEmptyLeaf, currSolidLeaf);

   delete root;

   //
   printf("   Nodes: %d\n", geometry->nodeList.size());
   printf("   Leafs: %d\n", geometry->solidLeafList.size() +
                            geometry->emptyLeafList.size());
   printf("   Solid: %d\n", geometry->solidLeafList.size());
   printf("   Empty: %d\n", geometry->emptyLeafList.size());
   printf("   Polys: %d\n", leafPolyCount);

   printf("Max Surf: %d\n", maxSurfacesInNode);

   if (polyPlaneMismatch)
      printf("   *****: %d Poly plane mismatchs\n",polyPlaneMismatch);
   return true;
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

int ITRBSPBuild::Node::nodeCount;


//----------------------------------------------------------------------------

ITRBSPBuild::Node::Node()
{
   planeIndex = -1;
   front = back = 0;
   nodeCount++;
}

ITRBSPBuild::Node::~Node()
{
   for (int i = 0; i < polyList.size(); i++)
      delete polyList[i];
   delete front;
   delete back;
}


//----------------------------------------------------------------------------

void ITRBSPBuild::Node::import(ITRGeometry*    geometry,
                               Vector<UInt32>* volumeMasks)
{
   // Convert all the surfaces into polys the BSP node.
   for (int s = 0; s < geometry->surfaceList.size(); s++) {
      ITRGeometry::Surface& surface = geometry->surfaceList[s];
      Poly* npoly = new Poly;
      npoly->surfaceIndex = s;
      npoly->planeIndex = surface.planeIndex;
      npoly->planeFront = surface.planeFront;
      npoly->splitable = surface.type != ITRGeometry::Surface::Link;
      npoly->nullMaterial = surface.material == ITRGeometry::Surface::NullMaterial;
      npoly->volumeMask = (*volumeMasks)[s];
      npoly->used = false;
      npoly->backface = false;
      npoly->vertexList.reserve(surface.vertexCount);

      for (int v = 0; v < surface.vertexCount; v++) {
         npoly->vertexList.increment();
         npoly->vertexList.last().point = 
            geometry->point3List[geometry->vertexList
               [surface.vertexIndex + v].pointIndex];
      }
      polyList.push_back(npoly);

      // Creat backface poly.
      Poly* fpoly = new Poly;
      *fpoly = *npoly;
      fpoly->used = true;
      fpoly->backface = true;
      fpoly->planeFront = !fpoly->planeFront;
      polyList.push_back(fpoly);
   }
}


//----------------------------------------------------------------------------

int ITRBSPBuild::Node::exportToGeometry(ITRGeometry* geometry,
                                        int&         currEmptyLeaf,
                                        int&         currSolidLeaf)
{
   // Recursively copy the BSP tree into the geometry.
   // The root node will end up as the first entry in the
   // nodeList.
   int nodeIndex;
   if (planeIndex >= 0) {
      nodeIndex = geometry->nodeList.size();
      geometry->nodeList.push_back(ITRGeometry::BSPNode());
      ITRGeometry::BSPNode& bnode = geometry->nodeList.last();
      bnode.planeIndex = planeIndex;
      bnode.front = front->exportToGeometry(geometry, currEmptyLeaf, currSolidLeaf);
      bnode.back = back->exportToGeometry(geometry, currEmptyLeaf, currSolidLeaf);
      bnode.fill = 0;
   }
   else {
      int listIndex;
      if (solid) {
         listIndex = currSolidLeaf;
         nodeIndex = -(currSolidLeaf+1);
         currSolidLeaf++;
         AssertFatal(listIndex < geometry->solidLeafList.size() + ITRGeometry::ReservedLeafEntries,
                     avar("Out of range listIndex (%d, %d)", listIndex, geometry->solidLeafList.size()));
      } else {
         listIndex = currEmptyLeaf;
         nodeIndex = -(currEmptyLeaf+1);
         currEmptyLeaf++;
      }

      ITRGeometry::BSPLeafWrap leafWrap(geometry, listIndex);
      ITRVector<int> surfaces;

      if (solid == false) {
         // This is an empty leaf
         //
         ITRGeometry::BSPLeafEmpty* pELeaf = leafWrap.getEmptyLeaf();
         AssertFatal(pELeaf != NULL, "Not empty in geometry?");

         pELeaf->flags    = 0;
         pELeaf->pvsCount = pELeaf->pvsIndex = 0;
         pELeaf->box      = box;

         for (int p = 0; p < polyList.size(); p++) {
            // Don't include surfaces with null materials
            if (!polyList[p]->backface && !polyList[p]->nullMaterial)
               surfaces.add(polyList[p]->surfaceIndex);
         }
      } else {
         // This is a solid leaf
         //
         ITRGeometry::BSPLeafSolid* pSLeaf = leafWrap.getSolidLeaf();
         AssertFatal(pSLeaf != NULL, "Not solid in geometry?");

         for (int p = 0; p < polyList.size(); p++) {
            // Null materials are included here because surfaces
            // on solid nodes are used for collision.
            if (polyList[p]->backface)
               surfaces.add(polyList[p]->surfaceIndex);
         }
      }

      // Create bitvector of surfaces.
      ITRBitVector sbv;
      for (int s = 0; s < surfaces.size(); s++)
         sbv.set(surfaces[s]);

      // Create bitvector of planes used by surfaces.
      ITRBitVector pbv;
      for (int s = 0; s < surfaces.size(); s++)
         pbv.set(geometry->surfaceList[surfaces[s]].planeIndex);

      // Compress surface and planes appropriately...
      //
      if (leafWrap.isSolid()) {
         ITRGeometry::BSPLeafSolid* pSLeaf = leafWrap.getSolidLeaf();
         AssertFatal(pSLeaf != NULL, "Not a solid leaf?");

         pSLeaf->surfaceIndex = geometry->bitList.size();
         pSLeaf->surfaceCount = sbv.compress(&geometry->bitList);

         pSLeaf->planeIndex = geometry->bitList.size();
         pSLeaf->planeCount = pbv.compress(&geometry->bitList);
      } else {
         ITRGeometry::BSPLeafEmpty* pELeaf = leafWrap.getEmptyLeaf();
         AssertFatal(pELeaf != NULL, "Not an empty leaf?");

         pELeaf->surfaceIndex = geometry->bitList.size();
         pELeaf->surfaceCount = sbv.compress(&geometry->bitList);

         pELeaf->planeIndex = geometry->bitList.size();
         pELeaf->planeCount = pbv.compress(&geometry->bitList);
      }
   }

   return nodeIndex;
}


//----------------------------------------------------------------------------

void ITRBSPBuild::Node::setBoundingBox()
{
   // Recursively set bounding box for node.
   // Solid nodes will end up with the bounding box
   // init values, basically wrong.
   box.fMin.set(+1.0E20f,+1.0E20f,+1.0E20f);
   box.fMax.set(-1.0E20f,-1.0E20f,-1.0E20f);
   if (planeIndex >= 0) {
      front->setBoundingBox();
      back->setBoundingBox();

      box.fMin.setMin(front->box.fMin);
      box.fMax.setMax(front->box.fMax);
      box.fMin.setMin(back->box.fMin);
      box.fMax.setMax(back->box.fMax);
   }
   else
      for (int p = 0; p < polyList.size(); p++)
         if (!polyList[p]->backface) {
            Poly& poly = *polyList[p];
            for (int v = 0; v < poly.vertexList.size(); v++) {
               box.fMin.setMin(poly.vertexList[v].point);
               box.fMax.setMax(poly.vertexList[v].point);
            }
         }
}


//----------------------------------------------------------------------------

bool ITRBSPBuild::Node::pickPlane(ITRGeometry* geometry)
{
   Point3F boxCenter;
   box.fMin.set(+1.0E20f,+1.0E20f,+1.0E20f);
   box.fMax.set(-1.0E20f,-1.0E20f,-1.0E20f);

   // Get list of planes not already used and bounding
   // box for later.
   ITRVector<int> planeList;
   for (int p = 0; p < polyList.size(); p++)
      if (!polyList[p]->backface) {
         if (!polyList[p]->used)
            // ITRVector::add will weed out duplicates
            planeList.add(polyList[p]->planeIndex);
         //
         Poly& poly = *polyList[p];
         for (int v = 0; v < poly.vertexList.size(); v++) {
            box.fMin.setMin(poly.vertexList[v].point);
            box.fMax.setMax(poly.vertexList[v].point);
         }
      }
   if (planeList.size() == 0)
      // Must have all been used already
      return false;

   boxCenter = box.fMin;
   boxCenter += box.fMax;
   boxCenter *= 0.5;

   // Pick a plane.
   int minSplit = M_MAX_INT;
   float bestOrtho = 0.0f;
   float bestDist  = 0.0f;
   int bestCoplane = 0;
   int bestPlane   = -1;
   for (int i = 0; i < planeList.size(); i++) {
      TPlaneF& plane = geometry->planeList[planeList[i]];
      int splitCount = 0;
      int coplane = 0;
      for (int b = 0; b < polyList.size() && splitCount <= minSplit; b++) {
         if (!polyList[b]->backface) {
            if (polyList[b]->planeIndex == planeList[i])
               coplane++;
            if (polyList[b]->whichSide(plane) == TPlaneF::Intersect) {
               if (!polyList[b]->splitable)
                  // Like to avoid splitting this poly.
                  splitCount++;
               splitCount++;
            }
         }
      }

      if (splitCount > minSplit)
         continue;

      float ortho = fmax(fmax(fabs(plane.x),fabs(plane.y)),fabs(plane.z));
      float dist = fabs(m_dot(plane,boxCenter));

      if (splitCount == minSplit) {
         if (ortho < bestOrtho)
            continue;
         if (ortho == bestOrtho) {
            if (coplane < bestCoplane)
               continue;
            if (coplane == bestCoplane)
               if (dist <= bestDist)
                  continue;
         }
      }

      minSplit    = splitCount;
      bestOrtho   = ortho;
      bestDist    = dist;
      bestPlane   = planeList[i];
      bestCoplane = coplane;
   }
   planeIndex = bestPlane;

   // Mark the polys on the same plane as being used by the bsp.
   for (int p = 0; p < polyList.size(); p++)
      if (polyList[p]->planeIndex == planeIndex)
         polyList[p]->used = true;

   return true;
}


//----------------------------------------------------------------------------

void ITRBSPBuild::Node::splitNode(ITRGeometry* geometry)
{
   // Split all the polys along the node's plane and insert
   // into front or back child nodes.
   TPlaneF& plane = geometry->planeList[planeIndex];
   for (int p = 0; p < polyList.size(); p++) {
      Poly& poly = *polyList[p];
      TPlaneF::Side side = (poly.planeIndex == planeIndex)?
         TPlaneF::OnPlane: poly.whichSide(plane);
      switch (side) {
         case TPlaneF::OnPlane: {
            if (poly.planeIndex != planeIndex)
               ++polyPlaneMismatch;
            TPlaneF& pplane = geometry->planeList[poly.planeIndex];
            if (pplane.isFacing(plane) ^ !poly.planeFront)
               back->polyList.push_back(&poly);
            else
               front->polyList.push_back(&poly);
            break;
         }
         case TPlaneF::Inside:
            front->polyList.push_back(&poly);
            break;
         case TPlaneF::Outside:
            back->polyList.push_back(&poly);
            break;
         case TPlaneF::Intersect: {
            Poly* fpoly = new Poly;
            Poly* bpoly = new Poly;
            *fpoly = *bpoly = poly;
            if (poly.split(plane,fpoly,bpoly)) {
               front->polyList.push_back(fpoly);
               back->polyList.push_back(bpoly);
            }
            else {
               AssertFatal(0,"Node::splitNode: Poly does not intersect");
            }
            delete polyList[p];
            break;
         }
         default:
            // Should only get this if the poly has no vertices
            // The poly will get thrown away.
            printf("   Warning: Degenerate clipped poly");
            break;
      }
   }
   // Polys have either been moved into leaf nodes
   // or split and deleted.
   polyList.clear();
}

