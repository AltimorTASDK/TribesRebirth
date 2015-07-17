//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include "fearInteriorShape.h"
#include "fearDcl.h"
//#include "fearExplosion.h"

IMPLEMENT_PERSISTENT_TAG(FearInteriorShape, FearInteriorShapePersTag);

//------------------------------------------------------------------------------
//--------------------------------------
// Explosion work...
//--------------------------------------
//
void
FearInteriorShape::spawnExplosion()
{
   if (m_explosionTag != 0) {
//      FearExplosionScoper* explosion = new FearExplosionScoper(m_explosionTag);
//      explosion->setExpPosition(getLinearPosition());
//      explosion->setExpAxis(Point3F(0, 0, 1));
//      manager->addObject(explosion);
   }
}

char*
FearInteriorShape::getExplosionFilter() const
{
   return "IDEXP_ITRSHAPE_*";
}

void FearInteriorShape::initPersistFields()
{
   addField("position", TypePoint3F, Offset(getShapePosition(), FearInteriorShape));
   addField("rotation", TypePoint3F, Offset(getShapeRotation(), FearInteriorShape));
   addField("filename", TypeString, Offset(m_pFileName, FearInteriorShape));
}
