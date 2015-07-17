#include <ts.h>
#include <base.h>
#include <simbase.h>
#include <g_bitmap.h>
#include <simrendergrp.h>
#include <decalmanager.h>

#include <interiorshape.h>
#include <simterrain.h>
#include <simresource.h>
#include <console.h>
#include <g_surfac.h>
#include <d_caps.h>

float       g_decalManagerConstAlpha = 0.66f;
SimTerrain *terrain = NULL;

DecalManager::DecalInfo rgDecalInfo[] =
{
   { "lightfootl.bmp", 0.15f, },
   { "lightfootr.bmp", 0.15f, },

   { "medfootl.bmp", 0.20f, },
   { "medfootr.bmp", 0.20f, },

   { "heavyfootl.bmp", 0.32f, },
   { "heavyfootl.bmp", 0.32f, },

   { "bullethole.bmp", 0.15f, },
};

int DecalManager::iPrefNumDecals = 50;

static const Point2F TextureCoords[] =
{
   Point2F(1.0f, 1.0f), Point2F(0.0f, 1.0f), 
   Point2F(0.0f, 0.0f), Point2F(1.0f, 0.0f),
};

DecalManager::DecalManager() 
: fLoaded(false)
{
   image.iCurDecal  = 0;
   image.iNumDecals = 0;
   memset(&(image.rgDecals[0]), NULL, sizeof(Decal *) * MAX_DECALS);
}

DecalManager::~DecalManager()
{
   for (int i = 0; i < image.iNumDecals; i ++)
   {
      delete image.rgDecals[i];
   }
}

void DecalManager::reset()
{
   // Clear out the old decal array
   for (int i = 0; i < image.iNumDecals; i ++)
   {
      if (image.rgDecals[i])
      {
         delete image.rgDecals[i];
         image.rgDecals[i] = NULL;
      }
   }

   image.iCurDecal  = 0;
   image.iNumDecals = DecalManager::iPrefNumDecals;

   // Limit the amount
   if (image.iNumDecals > MAX_DECALS)
   {
      image.iNumDecals = MAX_DECALS;
   }
   else if (image.iNumDecals < 0)
   {
      image.iNumDecals = 0;
   }

   // Force a re-location of the terrain, in case
   // we're changing missions
   terrain = NULL;
}

bool DecalManager::onAdd()
{
   if (Parent::onAdd())
   {
      addToSet(SimRenderSetId);
      return (true);
   }
   return (false);
}

void DecalManager::addFootprint(TMat3F    tmat, 
                                DECALTYPE type, 
                                float     rFootspan)
{
   if (image.iNumDecals == 0)
   {
      return;
   }

   CollisionSurface   sfc;
   SimCollisionInfo   info;
   SimContainerQuery  query;
   SimContainer      *root;
   SimTerrain        *terrain;
   InteriorShape  *interior;
   EulerF             angles;
   Vector3F           temp, offset;
   Vector3F           perp1, temp1, temp2;
   Vector3F           perp2(0.0f, 1.0f, 0.0f);

   // Offset the footprint by the footspan, oriented to the
   // player's rotation
   tmat.angles(&angles);
   temp.set   (rFootspan / 2.0f, 0.0f, 0.0f);

   m_mul(temp, RMat3F(angles), &offset);

   tmat.p += offset;

   // Build a collision query to see what's below us
   query.id       = -1;
   query.type     = -1;
   query.mask     = SimTerrainObjectType;
   query.detail   = SimContainerQuery::DefaultDetail;
   query.box.fMin = tmat.p + (Point3F(0.0f, 0.0f, 0.25f));
   query.box.fMax = tmat.p - (Point3F(0.0f, 0.0f, 0.75f));

   root = findObject(manager, SimRootContainerId, root);
   
   AssertFatal(root, "No root container?");

   // See if we're obstructed ...
   if (root->findLOS(query, &info, SimCollisionImageQuery::High))
   {
//      // If we hit an interior
//      if (info.object->getType() & SimInteriorObjectType)
//      {
//         interior = dynamic_cast<InteriorShape *>(info.object);
//
//         if (interior == NULL)
//         {
//            return;
//         }
//
//         interior->getInstance()->getSurfaceInfo(sfc, NULL);
//      }
//      // If we hit the terrain
//      else
//      {
         terrain = dynamic_cast<SimTerrain *>(info.object);

         if (terrain == NULL)
         {
            return;
         }

         terrain->getSurfaceInfo(Point2F(tmat.p.x, tmat.p.y), &sfc);
//      }

      // Get two orthonormal vectors
      m_cross(perp2, sfc.normal, &perp1);
      m_cross(perp1, sfc.normal, &perp2);
      perp1.normalize();
      perp2.normalize();

      temp1 = perp1 * sin(angles.z)        + perp2 * cos(angles.z);
      temp2 = perp2 * sin(M_PI - angles.z) + perp1 * cos(M_PI - angles.z);

      perp1       = temp1 * rgDecalInfo[type].rSize;
      perp2       = temp2 * rgDecalInfo[type].rSize;
      sfc.normal *= 0.05f;

      tmat.p.z = sfc.position.z;

      addDecal(tmat.p, sfc.normal, perp1, perp2, type);
   }
}

void DecalManager::addBulletHole(Vector3F position, Vector3F normal, DECALTYPE type)
{
   if (image.iNumDecals == 0)
   {
      return;
   }

   float           rRand;
   Point3F         perp1, perp2;
   Point3F         temp1, temp2;

   if (normal.z == 1.0f || normal.z == -1.0f)
   {
      perp2.set(1.0f, 0.0f, 0.0f);
   }
   else
   {
      perp2.set(0.0f, 0.0f, 1.0f);
   }

   m_cross(perp2, normal, &perp1);
   m_cross(perp1, normal, &perp2);

   rRand = random.getFloat() * M_PI;

   temp1 = perp1 * sin(rRand)        + perp2 * cos(rRand);
   temp2 = perp2 * sin(M_PI - rRand) + perp1 * cos(M_PI - rRand);

   perp1   = temp1 * rgDecalInfo[type].rSize;
   perp2   = temp2 * rgDecalInfo[type].rSize;
   normal *= 0.05f;

   addDecal(position, normal, perp1, perp2, type);
}

void DecalManager::addDecal(const Vector3F &pos, 
                            const Vector3F &normal, 
                            const Vector3F &perp1,
                            const Vector3F &perp2,
                            DECALTYPE type)
{
   Decal *decal;

   // If there is a print in this slot, delete it
   if (image.rgDecals[image.iCurDecal])
   {
      delete image.rgDecals[image.iCurDecal];
   }

   decal = new Decal;

   // Build a square centered around our position, a little above surface
   decal->iType       = type;
   decal->center      = pos;
   decal->worldPts[0] = pos + normal + perp1 - perp2;
   decal->worldPts[1] = pos + normal + perp1 + perp2;
   decal->worldPts[2] = pos + normal - perp1 + perp2;
   decal->worldPts[3] = pos + normal - perp1 - perp2;

   image.rgDecals[image.iCurDecal] = decal;
   image.iCurDecal = (image.iCurDecal + 1) % image.iNumDecals;
}

bool DecalManager::onSimRenderQueryImage(SimRenderQueryImage *query)
{
   if (g_decalManagerConstAlpha > 1.0f)
   {
      g_decalManagerConstAlpha = 1.0f;
   }
   else if (g_decalManagerConstAlpha < 0.0f)
   {
      g_decalManagerConstAlpha = 0.0f;
   }

   if (iPrefNumDecals != image.iNumDecals)
   {
      reset();
   }

   if (fLoaded == false)
   {
      fLoaded = true;
      for (int i = 0; i < NUM_TYPES; i ++)
      {
         image.rgrsDecals[i] = 
            SimResource::get(manager)->load(rgDecalInfo[i].lpszDecal);
         
         AssertFatal(image.rgrsDecals[i], avar("could not load print! (%s)", rgDecalInfo[i].lpszDecal));
      }
   }

   if (terrain == NULL)
   {
      terrain = static_cast<SimTerrain *>(manager->findObject(SimTerrainId));
   }

   query->count    = 1;
   query->image[0] = &image;
   return (true);
}

bool DecalManager::processQuery(SimQuery *query)
{
   switch (query->type)
   {
      onQuery(SimRenderQueryImage);

      default :
         return (Parent::processQuery(query));
   }
}

void DecalManager::DecalRenderImage::render(TSRenderContext &rc)
{
   int                  iOffset;
   GFXSurface          *sfc = rc.getSurface();
   TS::PointArray      *pa  = rc.getPointArray();
   TS::VertexIndexPair  rgVIP[4];

   // Set hardware / software options for drawing
   sfc->setFillMode          (GFX_FILL_TEXTURE);
   sfc->setHazeSource        (GFX_HAZE_NONE);

//   if ((sfc->getCaps() & GFX_DEVCAP_SUPPORTS_CONST_ALPHA) != 0) {
//      sfc->setAlphaSource(GFX_ALPHA_CONSTANT);
//      sfc->setConstantAlpha(g_decalManagerConstAlpha);
//   } else {
//      sfc->setAlphaSource(GFX_ALPHA_NONE);
//   }
   sfc->setShadeSource(GFX_SHADE_CONSTANT);
   sfc->setConstantShade(1.0f);
   sfc->setTexturePerspective(false);
//   sfc->setTransparency(true);
   pa->useHazes(false);
   pa->useIntensities(false);
   pa->useTextures(true);
   pa->useTextures(TextureCoords);
   sfc->setZTest(GFX_ZTEST);

   // Cycle through array and blast all N bitmaps to screen
   for (int i = 0; i < iNumDecals; i ++)
   {
      if (rgDecals[i] && terrain)
      {
         // If the distance to this decal is less than the perspective
         // distance, draw it - should remove the "floating decal"
         // problem caused by terrain growing
         Vector3F distance = rgDecals[i]->center - rc.getCamera()->getTCW().p;

         if (distance.lenf() < terrain->getPerspectiveDistance())
         {
            pa->reset();

            if (rgrsDecals[rgDecals[i]->iType]->attribute & BMA_SUBTRACTIVE) {
               sfc->setShadeSource(GFX_SHADE_NONE);
               sfc->setAlphaSource(GFX_ALPHA_SUB);
               sfc->setTransparency(false);
            } else {
               if ((sfc->getCaps() & GFX_DEVCAP_SUPPORTS_CONST_ALPHA) != 0) {
                  sfc->setAlphaSource(GFX_ALPHA_CONSTANT);
                  sfc->setConstantAlpha(g_decalManagerConstAlpha);
               } else {
                  sfc->setAlphaSource(GFX_ALPHA_NONE);
               }
               sfc->setTransparency(true);
            }

            sfc->setTextureMap(rgrsDecals[rgDecals[i]->iType]);

            iOffset = pa->addPoints(4, rgDecals[i]->worldPts);

            rgVIP[0].fVertexIndex  = iOffset;
            rgVIP[0].fTextureIndex = 0;
            rgVIP[1].fVertexIndex  = iOffset + 1;
            rgVIP[1].fTextureIndex = 1;
            rgVIP[2].fVertexIndex  = iOffset + 2;
            rgVIP[2].fTextureIndex = 2;
            rgVIP[3].fVertexIndex  = iOffset + 3;
            rgVIP[3].fTextureIndex = 3;

            pa->drawPoly(4, rgVIP, 0);
         }
      }
   }
   sfc->setTransparency      (false);
   sfc->setZTest(GFX_ZTEST_AND_WRITE);
}

void DecalManager::DecalRenderImage::setSortValue(TSCamera *)
{
   sortValue = 1.0f;;
}
