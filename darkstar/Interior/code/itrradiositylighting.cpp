//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include <core.h>
#include <g_bitmap.h>
#include <g_pal.h>
#include <itrbit.h>
#include <itrgeometry.h>
#include <itrcollision.h>
#include <itrlighting.h>
#include <itrradiositylighting.h>
#include <tplane.h>

#include "m_mul.h"

Point3F		collisionPoint;
bool findCollisionSurface(ITRGeometry *in_geometry, TMat3F *in_transform,
									const Point3F& start,const Point3F& end,
									int *out_surfaceIndex);

//--------------------------------------
//  Just quick functions for distance so we don't get the
// sqrt() hit from m_dist...
inline float
getDistSQ(Point3F &in_p1, Point3F &in_p2)
{
	return ((in_p1.x - in_p2.x) * (in_p1.x - in_p2.x) +
	        (in_p1.y - in_p2.y) * (in_p1.y - in_p2.y) +
	        (in_p1.z - in_p2.z) * (in_p1.z - in_p2.z));
}

//----------------------------------------------------------------------------

#define MIN_INTENSITY (0.01f)

bool itrInverse(TMat3F& tmat);


//----------------------------------------------------------------------------

namespace ITRRadiosityLighting
{
	enum BoxMapping {
		MapYZ,
		MapXZ,
		MapXY,
	};
	struct LightMap {
		enum Constants {
			MaxWidth = 256,
			MaxHeight = 256,
		};
		Point2I size;
		Color *color;
		Color *emissionBucket;
		Color *emissionAbsorb;

		//
		LightMap();
		~LightMap();
		void clear(const Point2I& size);
	};
	//
	struct MapData {
		Point2I offset;
		Point2I size;
		TMat3F transform;
		float scale;
		//
		MapData();
		bool set(ITRLighting& lighting,ITRGeometry& geometry,
			ITRGeometry::Surface& surface);
		void buildPointList(ITRGeometry& geometry,ITRGeometry::Surface& surface,
			MapPointList* pointList);
		void clampPointList(ITRGeometry& geometry,ITRGeometry::Surface& surface,
			MapPointList* pointList);
		void markPointList(ITRGeometry& geometry,ITRGeometry::Surface& surface,
			MapPointList* pointList);
	};
	//
	static int _USERENTRY compareLights(const void* a,const void* b);
	int exportMap(LightMap& map,ITRLighting* lighting, bool do_mono = true);
	void exportLights(LightList& map,ITRLighting* lighting);
	void basicLightSurface(ITRGeometry& geometry, LightList& lights,
									ITRLighting* lighting, int surface);
	void radLightSurfaceStatic(ITRGeometry& geometry, LightList& lights,
										ITRLighting* lighting, TS::MaterialList *materialList,
										MaterialPropList &matPropList,
										GFXPalette *pPalette);
	void radLightSurfacesDynamic(ITRGeometry& geometry, LightList& lights,
								ITRLighting* lighting);

	void emitRadiosityLight(ITRGeometry& geometry,
									MapPointList *globalPoints, MapPointList &mapPoints,
									LightMap *staticMaps, LightMap &lmap,
						 			TPlaneF &plane, int &maxEmission,
						 			MaterialPropList &matPropList,
						 			int surfaceIndex);


	void modifyMapRGB(ITRRadiosityLighting::LightMap &map,
							ITRLighting* lighting, int map_index);
	//
	bool  lightFilter = true;
	bool  statelessRadiosity = true;
	bool  monoRadiosity = false;
	bool  alphaTransmission  = false;
   bool  emittingMaterials  = false;
	int   quantumNumber      = 50;
	int	maxRadiosityPasses = 20;
	int	emissionThreshold  = 1;
	float	absorbanceLevel    = 0.8;
	float	emissionLevel      = 0.2;

	//  Distance beyond which a point is not considered to be on
	// a plane...
	const float distTolerance = 1e-4f;
};




void
getCosRandDeflectedNormal(const Point3F& in_normal, Point3F& deflectedNormal,
									float phongExp = 1.0)
{
	float cosRandTheta;
	float cosRandPhi;
	float rand1 = float((rand() << 16) | rand());
	float rand2 = float((rand() << 16) | rand());
	float epsilon1 = rand1 / float((RAND_MAX << 16) | RAND_MAX);
	float epsilon2 = rand2 / float((RAND_MAX << 16) | RAND_MAX);

	cosRandTheta = acos(pow((1.0f - epsilon1),1.0/(1+phongExp)));
	cosRandPhi   = 2 * M_PI * epsilon2;

	// Deflect the surface normal
	//
	EulerF angle;
	RMat3F rotMatrix;
	Point3F unitYVector, randCosVector;
	deflectedNormal.set(0.0f,0.0f,0.0f);

	angle.x = (M_PI/2.0) - cosRandTheta;
	angle.y = 0.0f;
	angle.z = cosRandPhi;
	unitYVector.x = 0.0f;
	unitYVector.y = 1.0f;
	unitYVector.z = 0.0f;
	angle.makeMatrix(&rotMatrix);
	m_mul(unitYVector, rotMatrix, &randCosVector);

	Point3F xPrime, yPrime, zPrime, temp;
	RMat3F tempRot;

	zPrime.x = in_normal.x;
	zPrime.y = in_normal.y;
	zPrime.z = in_normal.z;
	tempRot.set(EulerF(float(M_PI)/1.36f, float(M_PI)/.56f, float(M_PI)/6.5f));
	m_mul(zPrime, tempRot, &temp);
	m_cross(zPrime, temp, &yPrime);
	yPrime.normalize();
	m_cross(zPrime, yPrime, &xPrime);
	xPrime.normalize();

	xPrime *= randCosVector.x;
	yPrime *= randCosVector.y;
	zPrime *= randCosVector.z;
	deflectedNormal += xPrime;
	deflectedNormal += yPrime;
	deflectedNormal += zPrime;
}


void
ITRRadiosityLighting::emitRadiosityLight(ITRGeometry& geometry,
													MapPointList *globalPoints, MapPointList &mapPoints,
													LightMap *staticMaps, LightMap &lmap,
						 							TPlaneF &plane, int &maxEmission,
						 							MaterialPropList &matPropList,
						 							int surfaceIndex)
{
	int filterMul = (lightFilter == true) ? 4 : 1;

	for (int y = 0; y < lmap.size.y; y++) {
		for (int x = 0; x < lmap.size.x; x++) {
			Color &emitColor = lmap.emissionBucket[y * lmap.size.x + x];
			Color quantaColor;
			int   numQuanta;

			float intensity = 0.3f * emitColor.red + 0.6f * emitColor.green +
									0.1f * emitColor.blue;
			numQuanta = int(intensity * float(quantumNumber));

			if (numQuanta != 0)
				quantaColor = emitColor * (1.0f/float(numQuanta));

			if (numQuanta > maxEmission)
				maxEmission = numQuanta;

			//  OK, we know the number of rays, and their color, now choose
			// a random vector facing away from the surface, and shoot them.
			for (int r = 0; r < numQuanta; r++) {
				Point3F deflectedNormal;
				if (emittingMaterials &&
				    matPropList[geometry.surfaceList[surfaceIndex].material].modified == true &&
					 matPropList[geometry.surfaceList[surfaceIndex].material].modifiedPhong == true) {
					getCosRandDeflectedNormal(plane, deflectedNormal,
							matPropList[geometry.surfaceList[surfaceIndex].material].phongExp);
				} else {
					getCosRandDeflectedNormal(plane, deflectedNormal, 1.0);
				}
				// Make sure the vector is pointed away from the plane.
				//
				if (m_dot(plane, deflectedNormal) < 0.0f) {
					deflectedNormal *= -1.0f;
				}
				deflectedNormal.normalize();

				// Find the point in worldSpace.  The point to shoot the
				// vector from is (pMapPList[ss])[y * lmap.size.x * filterMul + x + randChoice]
				// filterMul takes care of the fact that if filtering is turned on, there are
				// four times as many mapPoints, randChoice selects one of the filtering
				// points on the surface to shoot from.
				int randChoice = (lightFilter == true) ? (rand() % 4) : 1;
				int pointIndex = ((y * lmap.size.x + x) * filterMul) + randChoice;
				if (mapPoints[pointIndex].visible != true) {
						// don't shoot if the point isn't visible
						continue;
				}
				Point3F shootPoint =	mapPoints[pointIndex].pos;
				Point3F ludicrousPoint = shootPoint;
				Point3F tmp = deflectedNormal;
				tmp *= 1e5;
				ludicrousPoint += tmp;

				// Shoot the ray
				int surfColIndex;
				if (findCollisionSurface(&geometry, NULL, shootPoint,
					                      ludicrousPoint, &surfColIndex)) {
					float minLenSQ = 1e50;
					int minIndex = -1;
					for (int i = 0; i < globalPoints[surfColIndex].size(); i++) {
						if (globalPoints[surfColIndex][i].visible == false)
							continue;

						float tempLen = getDistSQ(collisionPoint, globalPoints[surfColIndex][i].pos);
						if (tempLen < minLenSQ) {
							minLenSQ = tempLen;
							minIndex = i;
						}
					}

					if (minIndex == -1) {
						// We seem to have lost this one...
						continue;
					}

					//  Add quanta to the appropriate lightMap's color and emission
					// bucket...
					LightMap &lmap = staticMaps[surfColIndex];
					int mapPosIndex = (globalPoints[surfColIndex])[minIndex].mapIndex;

					if (emittingMaterials &&
					    matPropList[geometry.surfaceList[surfColIndex].material].modified == true &&
						 matPropList[geometry.surfaceList[surfColIndex].material].overrideRadParams == true) {
						lmap.emissionAbsorb[mapPosIndex] +=
								quantaColor * matPropList[geometry.surfaceList[surfColIndex].material].absorbance;
						lmap.emissionBucket[mapPosIndex] +=
								quantaColor * matPropList[geometry.surfaceList[surfColIndex].material].reflectance;
					} else {
						lmap.emissionAbsorb[mapPosIndex] +=
								quantaColor * absorbanceLevel;
						lmap.emissionBucket[mapPosIndex] +=
								quantaColor * (emissionLevel);
					}
				} else {
				}
			}
			// wipe the emissionBucket clean. (cheesy)
			//
			emitColor.clearRGB();
		}
	}
}




//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

ITRRadiosityLighting::MapData::MapData()
{
}	


//----------------------------------------------------------------------------

ITRRadiosityLighting::Color& ITRRadiosityLighting::Color::operator+=
	(const ITRRadiosityLighting::Color& cc)
{
	red += cc.red;
	green += cc.green;
	blue += cc.blue;
	return *this;
}


//----------------------------------------------------------------------------

ITRRadiosityLighting::Color&
ITRRadiosityLighting::Color::operator*(const float in_factor)
{
	static Color temp;
	temp.red   = red * in_factor;
	temp.green = green * in_factor;
	temp.blue  = blue * in_factor;
	return temp;
}


//----------------------------------------------------------------------------

ITRRadiosityLighting::LightMap::LightMap()
{
	color = NULL;
	emissionBucket = NULL;
	emissionAbsorb = NULL;
}

void ITRRadiosityLighting::LightMap::clear(const Point2I& psize)
{
	if (color != NULL) delete [] color;
	if (emissionBucket != NULL) delete [] emissionBucket;
	if (emissionAbsorb != NULL) delete [] emissionAbsorb;

	color          = new Color[psize.y * psize.x];
	emissionBucket = new Color[psize.y * psize.x];
	emissionAbsorb = new Color[psize.y * psize.x];
	
	size = psize;
	memset(&color[0],0,size.x*size.y*sizeof(Color));
	memset(&emissionBucket[0],0,size.x*size.y*sizeof(Color));
	memset(&emissionAbsorb[0],0,size.x*size.y*sizeof(Color));
}

ITRRadiosityLighting::LightMap::~LightMap()
{
	if (color != NULL) 			 delete [] color;
	if (emissionBucket != NULL) delete [] emissionBucket;

	color = emissionBucket = NULL;
}

//----------------------------------------------------------------------------
// Light all the points in the point list.
// Also does the point light averaging.
//
bool ITRRadiosityLighting::Light::State::light(
	ITRGeometry& geometry,
	ITRRadiosityLighting::MapPointList& pointList,Point3F& normal,
	ITRRadiosityLighting::LightMap* map)
{
	ITRCollision los(&geometry);

	for (int p = 0; p < pointList.size(); ) {
		MapPoint& first = pointList[p];
		Color lcolor;
		lcolor.red = lcolor.blue = lcolor.green = 0.0f;

		// Average consecutive points
		for (int end = p + first.average; p < end; p++) {
			MapPoint& point = pointList[p];
			if (!los.collide(point.pos,pos)) {
				float ndot;
				if (useNormal) {
					Point3F vec = pos;
					vec -= point.pos;
					vec.normalize();
					ndot = m_dot(normal,vec);
				}
				else
					ndot = 1.0f;

				float distance = m_dist(point.pos,pos);
				float distCoef = 1.0;
            if (falloff == Distance) {
               distCoef = 1.0f / (d1 +  (d2 + distance * d3) * distance);
            } else if (falloff == Linear) {
					if (distance < d1) {
                  distCoef = 1.0f;
               } else if (distance > d2) {
                  distCoef = 0.0f;
               } else {
                  distCoef = 1.0f - ((distance - d1) / (d2 - d1));
               }
            }
				if (lightType == SpotLight) {
					float projDist;
					Point3F temp = point.pos;
					temp -= pos;
					projDist = m_dot(spotVector, temp);
					float angle = acos(projDist/distance);
					if (angle <= spotTheta) {
						// point is in maximally lit 
					} else if (angle > spotTheta && angle <= spotPhi) {
						// point is between inner and outer angles of the spotlight
						//
						distCoef *= 1.0f - ((angle-spotTheta)/(spotPhi-spotTheta));
					} else {
						// point is totally outside spot
						distCoef = 0.0f;
					}
				}
				
            distCoef *= ndot;
				lcolor.red   += color.red   * distCoef;
				lcolor.green += color.green * distCoef;
				lcolor.blue  += color.blue  * distCoef;
			}
		}
		if (first.average > 1) {
			float scale = 1.0f / first.average;
			lcolor.red *= scale;
			lcolor.green *= scale;
			lcolor.blue *= scale;
		}

		// Just add the color in, it will get clamped later.
		map->color[first.mapIndex] += lcolor;
	}
	
	// Only mark map as lit if some minimum amount of
	// light falls on it. DMMNOTE Now always return true...
	return true;
}


//----------------------------------------------------------------------------
//  Light all the points in the point list, placing some in emission
//	buckets for later emission.
// Also does the point light averaging.
bool ITRRadiosityLighting::Light::State::lightRadiosity(
	ITRGeometry& geometry,
	ITRRadiosityLighting::MapPointList& pointList,Point3F& normal,
	ITRRadiosityLighting::LightMap* map)
{
	ITRCollision los(&geometry);

	for (int p = 0; p < pointList.size(); ) {
		MapPoint& first = pointList[p];
		Color lcolor;
		lcolor.red = lcolor.blue = lcolor.green = 0.0f;

		// Average consecutive points
		for (int end = p + first.average; p < end; p++) {
			MapPoint& point = pointList[p];

			if (!los.collide(point.pos,pos)) {
				float ndot;
				if (useNormal) {
					Point3F vec = pos;
					vec -= point.pos;
					vec.normalize();
					ndot = m_dot(normal,vec);
				}
				else
					ndot = 1.0f;

				float distance = m_dist(point.pos,pos);
				float distCoef = 1.0;
            if (falloff == Distance) {
               distCoef = 1.0f / (d1 +  (d2 + distance * d3) * distance);
            } else if (falloff == Linear) {
					if (distance < d1) {
                  distCoef = 1.0f;
               } else if (distance > d2) {
                  distCoef = 0.0f;
               } else {
                  distCoef = 1.0f - ((distance - d1) / (d2 - d1));
               }
            }
				if (lightType == SpotLight) {
					float projDist;
					Point3F temp = point.pos;
					temp -= pos;
					projDist = m_dot(spotVector, temp);
					float angle = acos(projDist/distance);
					if (angle <= spotTheta) {
						// point is in maximally lit 
					} else if (angle > spotTheta && angle <= spotPhi) {
						// point is between inner and outer angles of the spotlight
						distCoef *= 1.0f - ((angle-spotTheta)/(spotPhi-spotTheta));
					} else {
						// point is totally outside spot
						distCoef = 0.0f;
					}
				}
				
				lcolor.red   += (color.red * ndot) * distCoef;
				lcolor.green += (color.green * ndot) * distCoef;
				lcolor.blue  += (color.blue * ndot) * distCoef;
			}
		}
		if (first.average > 1) {
			float scale = 1.0f / float(first.average);
			lcolor.red   *= scale;
			lcolor.green *= scale;
			lcolor.blue  *= scale;
		}

		// Just add the color in, it will get clamped later.
		if (overrideGlobRadParms == false) {
			map->color[first.mapIndex]          += (lcolor * absorbanceLevel);
			map->emissionBucket[first.mapIndex] += (lcolor * emissionLevel);
		} else {
			map->color[first.mapIndex]          += (lcolor * lightAbsorbLevel);
			map->emissionBucket[first.mapIndex] += (lcolor * lightEmissionLevel);
		}
	}
	return true;
}


//----------------------------------------------------------------------------
// Return true if the surface might be lit by this state
//
bool ITRRadiosityLighting::Light::State::isLit(ITRGeometry& geometry,
	ITRGeometry::Surface& surface)
{
	TPlaneF& plane = geometry.planeList[surface.planeIndex];
	float distance = m_dot(plane,pos) + plane.d;
	if (!surface.planeFront)
		distance = -distance;
	if (distance < 0.0f || distance > radius) {
		return false;
	}
	return true;
}


//----------------------------------------------------------------------------

void ITRRadiosityLighting::Light::State::appendSurface(int surf,int lightIndex,
	int mapIndex)
{
	surfaceData.increment();
	Light::State::Surface& sp = surfaceData.last();
	sp.surface = surf;
	sp.lightIndex = lightIndex;
	sp.mapIndex = mapIndex;
}


//----------------------------------------------------------------------------

int
ITRRadiosityLighting::Light::State::getSurfMapIndex(int in_surfNum)
{
	for (int i = 0; i < surfaceData.size(); i++) {
		if (surfaceData[i].surface == in_surfNum)
			return surfaceData[i].mapIndex;
	}

	AssertFatal(0, "Should never be here...");
	return -1;
}



//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Get light map size and offset and setup transform needed to
// create the point list (texture to world space mat.)
//
bool ITRRadiosityLighting::MapData::set(ITRLighting& lighting,ITRGeometry& geometry,
	ITRGeometry::Surface& surface)
{
	ITRGeometry::Poly& poly = geometry.polyList[surface.polyIndex];
	ITRGeometry::Vertex* vp = &geometry.vertexList[poly.vertexIndex];
	scale = float(1 << lighting.lightScaleShift);

	// Light Maps are aligned.
	offset.x = surface.textureOffset.x &
		((1 << lighting.lightScaleShift) - 1);
	offset.y = surface.textureOffset.y &
		((1 << lighting.lightScaleShift) - 1);

	// Surface.textureSize is stored size-1
	int half = 1 << (lighting.lightScaleShift - 1);
	size.x = ((surface.textureSize.x+1 + offset.x + half) >>
				lighting.lightScaleShift) + 2;
	size.y = ((surface.textureSize.y+1 + offset.y + half) >>
				lighting.lightScaleShift) + 2;

	// Build texture to vector space
	Point2F tv[3];
	tv[0] = geometry.point2List[vp[1].textureIndex];
	tv[1] = geometry.point2List[vp[0].textureIndex];
	tv[2] = geometry.point2List[vp[2].textureIndex];
	tv[0].x *= surface.textureSize.x + 1;
	tv[0].y *= surface.textureSize.y + 1;
	tv[1].x *= surface.textureSize.x + 1;
	tv[1].y *= surface.textureSize.y + 1;
	tv[2].x *= surface.textureSize.x + 1;
	tv[2].y *= surface.textureSize.y + 1;
	Point2F tv1 = tv[1]; tv1 -= tv[0];
	Point2F tv2 = tv[2]; tv2 -= tv[0];

	TMat3F tvs;
	tvs.flags = TMat3F::Matrix_HasRotation | 
		TMat3F::Matrix_HasTranslation | TMat3F::Matrix_HasScale;
	tvs.m[0][0] = tv1.x; tvs.m[0][1] = tv1.y; tvs.m[0][2] = 0;
	tvs.m[1][0] = tv2.x; tvs.m[1][1] = tv2.y; tvs.m[1][2] = 0;
	tvs.m[2][0] = tv[0].x; tvs.m[2][1] = tv[0].y; tvs.m[2][2] = 1.0;
	tvs.p.set(.0f,.0f,.0f);
	if (!itrInverse(tvs)) {
		transform.identity();
		return false;
	}

	// Build vector space to world space
	Point3F vv[3];
	vv[0] = geometry.point3List[vp[1].pointIndex];
	vv[1] = geometry.point3List[vp[0].pointIndex];
	vv[2] = geometry.point3List[vp[2].pointIndex];
	Point3F vv1 = vv[1]; vv1 -= vv[0];
	Point3F vv2 = vv[2]; vv2 -= vv[0];

	TMat3F vws;
	vws.flags = TMat3F::Matrix_HasRotation;
	vws.m[0][0] = vv1.x; vws.m[0][1] = vv1.y; vws.m[0][2] = vv1.z;
	vws.m[1][0] = vv2.x; vws.m[1][1] = vv2.y; vws.m[1][2] = vv2.z;
	vws.m[2][0] = vv[0].x; vws.m[2][1] = vv[0].y; vws.m[2][2] = vv[0].z;
	vws.p.set(.0f,.0f,.0f);

	// Build texture to world space.
	m_mul(tvs,vws,&transform);
	return true;
}


//----------------------------------------------------------------------------
// Build the list of points that will be sample on the surface
//
void ITRRadiosityLighting::MapData::buildPointList(ITRGeometry& geometry,
	ITRGeometry::Surface& surface, ITRRadiosityLighting::MapPointList* pointList)
{
	// Generate points in world space to be sampled.
	TPlaneF plane = geometry.planeList[surface.planeIndex];
	for (int y = 0; y < size.y; y++)
		for (int x = 0; x < size.x; x++) {
			// Convert to texture coors.
			Point3F tp;
			tp.x = (x * scale) - offset.x;
			tp.y = (y * scale) - offset.y;
			tp.z = 1.0f;

			//  DMMNOTE: If the offset code changes, this needs to
			// change as well...
			tp.x += (scale / 2);
			tp.y += (scale / 2);

			int count = 1;
			if (lightFilter) {
				tp.x -= (scale * filterScale * 0.5f);
				tp.y -= (scale * filterScale * 0.5f);
				count = 4;
			}
			for (int i = 0; i < count; i++) {
				pointList->increment();
				MapPoint& point = pointList->last();
				point.mapIndex = y * size.x + x;
				point.average = (count > 1 && i == 0)? 4: 1;

				Point3F xp = tp;
				xp.x += (i & 1)? (scale * filterScale): 0.0f;
				xp.y += (i & 2)? (scale * filterScale): 0.0f;
				Point3F wpos;
				m_mul(xp,transform,&wpos);

				// Snap the point on to the plane.
				float distance = plane.distance(wpos);
				point.pos = plane;
				point.pos *= -distance;
				point.pos += wpos;
			}
		}
}


//----------------------------------------------------------------------------
// We don't want sample points that extend outside the poly if
// they pass through a solid leaf. This causes light and shadow bleeding.
//
void ITRRadiosityLighting::MapData::clampPointList(ITRGeometry& geometry,
	ITRGeometry::Surface& surface, ITRRadiosityLighting::MapPointList* pointList)
{
	TPlaneF plane = geometry.planeList[surface.planeIndex];
	if (!surface.planeFront)
		plane.neg();

	// Build list of bounding planes for all polys
	Vector<TPlaneF> planeList;
	Vector<Point3F> polyCenter;
	Vector<Vector<TPlaneF>*> polyList;
	for (int p = 0; p < surface.polyCount; p++) {
		ITRGeometry::Poly& poly = geometry.polyList[surface.polyIndex + p];
		ITRGeometry::Vertex* vp = &geometry.vertexList[poly.vertexIndex];
		polyList.push_back(new Vector<TPlaneF>);
		Vector<TPlaneF>& pList = *polyList.last();

		polyCenter.increment();
		Point3F& center = polyCenter[p];
		center.set(0.0f,0.0f,0.0f);

		Point3F *start = &geometry.point3List
			[vp[poly.vertexCount - 1].pointIndex];
		for (int i = 0; i < poly.vertexCount; i++) {
			Point3F* end = &geometry.point3List[vp[i].pointIndex];
			Point3F lnormal,lvec = *end; lvec -= *start;
			float len = lvec.len();
			if (len > distancePrecision) {
				lvec.normalize();
				m_cross(lvec,plane,&lnormal);
				if (!isEqual(lnormal.len(),.0f)) {
					pList.increment();
					pList.last().set(*start,lnormal);
				} 
			}
			center += *end;
			start = end;
		}
		center *= 1.0f / poly.vertexCount;
	}

	// Test all the points to see if they are inside a poly.
	ITRCollision los(&geometry);
	for (int i = 0; i < pointList->size(); i++) {
		MapPoint& point = (*pointList)[i];
		// See if the point is contained in any of the polys.
		int pp = 0;
		for (; pp < polyList.size(); pp++) {
			Vector<TPlaneF>& pList = *polyList[pp];
			int pl = 0;
			for (; pl < pList.size(); pl++)
				if (pList[pl].whichSide(point.pos) == TPlaneF::Outside) {
					break;
				}
			if (pl == pList.size())
				// Inside last poly
				break;
		}

		// If not inside, see if it's visibly to a poly center.
		// If it's not visible, it's brought in to the closest
		// point in a poly.
		if (pp == polyList.size()) {
			int c = 0;
			for (; c < polyCenter.size(); c++)
				if (!los.collide(polyCenter[c],point.pos))
					break;
			if (c == polyCenter.size()) {
				// Not visible, so pick a poly to be in.
				// Should fix this if we end up supporting multi-poly
				// surfaces.
				int poly = 0;

				// Find closest inside point.
				Point3F ip;
				LineSeg3F line(polyCenter[poly],point.pos);
				Vector<TPlaneF>& pList = *polyList[poly];
				for (int pl = 0; pl < pList.size(); pl++) {
					if (pList[pl].intersect(line,&ip))
						line.end = ip;
				}
				point.pos = line.end;
			}
		}
	}

	for (int i = 0; i < polyList.size(); i++)
		delete polyList[i];
}


//----------------------------------------------------------------------------
// We don't want sample points that extend outside the poly if
// they pass through a solid leaf. This causes light and shadow bleeding.
//
void ITRRadiosityLighting::MapData::markPointList(ITRGeometry& geometry,
	ITRGeometry::Surface& surface, ITRRadiosityLighting::MapPointList* pointList)
{
	TPlaneF plane = geometry.planeList[surface.planeIndex];
	if (!surface.planeFront)
		plane.neg();

	// Build list of bounding planes for all polys
	Vector<TPlaneF> planeList;
	Vector<Point3F> polyCenter;
	Vector<Vector<TPlaneF>*> polyList;
	for (int p = 0; p < surface.polyCount; p++) {
		ITRGeometry::Poly& poly = geometry.polyList[surface.polyIndex + p];
		ITRGeometry::Vertex* vp = &geometry.vertexList[poly.vertexIndex];
		polyList.push_back(new Vector<TPlaneF>);
		Vector<TPlaneF>& pList = *polyList.last();

		polyCenter.increment();
		Point3F& center = polyCenter[p];
		center.set(0.0f,0.0f,0.0f);

		Point3F *start = &geometry.point3List
			[vp[poly.vertexCount - 1].pointIndex];
		for (int i = 0; i < poly.vertexCount; i++) {
			Point3F* end = &geometry.point3List[vp[i].pointIndex];
			Point3F lnormal,lvec = *end; lvec -= *start;
			float len = lvec.len();
			if (len > distancePrecision) {
				lvec.normalize();
				m_cross(lvec,plane,&lnormal);
				if (!isEqual(lnormal.len(),.0f)) {
					pList.increment();
					pList.last().set(*start,lnormal);
				} 
			}
			center += *end;
			start = end;
		}
		center *= 1.0f / poly.vertexCount;
	}

	// Test all the points to see if they are inside a poly.
	ITRCollision los(&geometry);
	for (int i = 0; i < pointList->size(); i++) {
		MapPoint& point = (*pointList)[i];
		point.visible  = false;

		// See if the point is contained in any of the polys.
		int pp = 0;
		for (; pp < polyList.size(); pp++) {
			Vector<TPlaneF>& pList = *polyList[pp];
			int pl = 0;
			float temp;
			for (; pl < pList.size(); pl++)
				if (pList[pl].whichSide(point.pos) == TPlaneF::Outside)
					break;
			if (pl == pList.size()) {
				// Inside last poly
				point.visible = true;
				break;
			}
		}
	}

	// Find closest visible point, and store a reference for all
	// invisible points...
	for (int i = 0; i < pointList->size(); i++) {
		Point3F &testPoint = (*pointList)[i].pos;
		if ((*pointList)[i].visible == false) {
			float minDist = 1e50f;
			int minIdx = -1;
			
			for (int j = 0; j < pointList->size(); j++) {
				if ((*pointList)[j].visible == true) {
					float dist = getDistSQ(testPoint, (*pointList)[j].pos);
					if (dist < minDist) {
						minDist = dist;
						minIdx  = j;
					}
				}
			}
			//  If minIdx is -1, then all pts on this surface are not visible.
			// this should be fine...
			(*pointList)[i].nearestVisible     = minIdx;
		}
	}

	for (int i = 0; i < polyList.size(); i++)
		delete polyList[i];
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

void ITRRadiosityLighting::basicLightSurface(ITRGeometry& geometry,
	LightList& lights,ITRLighting* lighting,int ss)
{
	ITRLighting::Surface& lsurface = lighting->surfaceList[ss];
	ITRGeometry::Surface& bsurface = geometry.surfaceList[ss];

	MapData mapData;
	mapData.set(*lighting,geometry,bsurface);
	lsurface.mapSize.x = UInt8(mapData.size.x);
	lsurface.mapSize.y = UInt8(mapData.size.y);
	lsurface.mapOffset.x = UInt8(mapData.offset.x);
	lsurface.mapOffset.y = UInt8(mapData.offset.y);
	lsurface.lightIndex = lighting->lightMapListSize;
	lsurface.lightCount = 0;

	// Default to full intensity if no lights
	if (!lights.size()) {
		lsurface.mapColor = (-1 & ~0xffff) |
			(15 << ITRLighting::LightMap::AlphaShift);
		return;
	}

	// Build list of points to be sampled.
	MapPointList pointList;
	mapData.buildPointList(geometry,bsurface,&pointList);
	mapData.clampPointList(geometry,bsurface,&pointList);

	static LightMap staticMap;
	staticMap.clear(mapData.size);
	TPlaneF& plane = geometry.planeList[bsurface.planeIndex];

	// Light the point list with all the lights within range.
	for (int l = 0; l < lights.size(); l++) {

		Light& light = *lights[l];
		// If light only has a single state, its added
		// to the static lightmap
		if (light.state.size() == 1) {
			Light::State& state = *light.state[0];
			if (state.isLit(geometry,bsurface))
				state.light(geometry,pointList,plane,&staticMap);
		}
		else {
			// If any state lights the surface, they all do.
			int i = 0;
			for (; i < light.state.size(); i++)
				if (light.state[i]->isLit(geometry,bsurface))
					break;
			if (i != light.state.size()) {
				// Light map for each state
				for (int i = 0; i < light.state.size(); i++) {
					Light::State& state = *light.state[i];
					static LightMap map;
					map.clear(mapData.size);
					if (state.isLit(geometry,bsurface) &&
							state.light(geometry,pointList,plane,&map))
						state.appendSurface(ss,lighting->lightMapListSize,
							exportMap(map,lighting,true));
					else
						// No light map for this guy.
						state.appendSurface(ss,lighting->lightMapListSize,-1);
				}

				// All the states for this light got assigned the
				// same lightMapList index;
				lighting->lightMapListSize++;
				lsurface.lightCount++;
			}
		}
	}
	//
	lsurface.mapIndex = exportMap(staticMap,lighting,true);
}



//----------------------------------------------------------------------------
//
void ITRRadiosityLighting::radLightSurfaceStatic(ITRGeometry& geometry,
	LightList& lights,ITRLighting* lighting, TS::MaterialList */*materialList*/,
	MaterialPropList &matPropList,
	GFXPalette *pPalette)
{
	typedef Vector<MapData> MapDataList;

	MapDataList 		mapDataList;
	MapPointList*		pMapPList = new MapPointList[geometry.surfaceList.size()];
	MapPointList*		markPMapPList = new MapPointList[geometry.surfaceList.size()];
	LightMap*         staticMaps = new LightMap[geometry.surfaceList.size()];

	printf("\tBuilding Data Structures...");
	for (int i = 0; i < geometry.surfaceList.size(); i++) {
		mapDataList.increment();
		MapData &mData = mapDataList.last();

		mData.set(*lighting,geometry,geometry.surfaceList[i]);
	}
	for (int i = 0; i < geometry.surfaceList.size(); i++) {
		mapDataList[i].buildPointList(geometry,geometry.surfaceList[i],
												&pMapPList[i]);
		mapDataList[i].clampPointList(geometry,geometry.surfaceList[i],
												&pMapPList[i]);
	}
	for (int i = 0; i < geometry.surfaceList.size(); i++) {
		staticMaps[i].clear(mapDataList[i].size);
	}
	printf("done.\n");

	printf("\tBeginning Radiosity Pass 1...");
	for (int ss = 0; ss < geometry.surfaceList.size(); ss++) {
		ITRGeometry::Surface& bsurface = geometry.surfaceList[ss];
		TPlaneF&	plane = geometry.planeList[bsurface.planeIndex];
		int materialIndex = geometry.surfaceList[ss].material;

		// Light the point list with all the lights within range.
		for (int l = 0; l < lights.size(); l++) {
			Light& light = *lights[l];
			// If light only has a single state, its added
			// to the static lightmap (shoot to clamped points)
			if (light.state.size() == 1) {
				Light::State& state = *light.state[0];
				if (state.isLit(geometry,bsurface)) {
					state.lightRadiosity(geometry,pMapPList[ss],plane,&staticMaps[ss]);
				}
			}
			else {
				// Lights with states are not considered in this pass
			}
		}

		//  If the material on this surface emits light, add it to the 
		// appropriate color buckets
		if (ITRRadiosityLighting::emittingMaterials == true) {
			if (emittingMaterials &&
			    matPropList[materialIndex].modified == true) {
            GFXBitmap* pEmissionMask = matPropList[materialIndex].pEmissionMask;

				if (matPropList[materialIndex].emissionType == MaterialProp::flatEmission) {
					MapData &mapData = mapDataList[ss];
					LightMap &lmap   = staticMaps[ss];
               Color emitColor = matPropList[materialIndex].emitColor;

					for (int y = 0; y < lmap.size.y; y++) {
						for (int x = 0; x < lmap.size.x; x++) {
							// Create texCoord point
							Point2I texPos;
							texPos.x = (x * mapData.scale) - mapData.offset.x;
							texPos.y = (y * mapData.scale) - mapData.offset.y;
							
							// Average all emission values on the lightmap square...
                     double colorMul;
                     if (pEmissionMask == NULL) {
                        colorMul = 1.0;
                     } else {
                        colorMul = 0.0;
							   for (int i = -(mapData.scale/2); i < (mapData.scale/2); i++) {
							   	for (int j = -(mapData.scale/2); j < (mapData.scale/2); j++) {
							   		Point2I tmpPos = texPos;

							   		tmpPos.x += i;
							   		tmpPos.y += j;

							   		tmpPos.x %= pEmissionMask->width;
							   		tmpPos.y %= pEmissionMask->height;
							   		if (tmpPos.x < 0)
							   			tmpPos.x += pEmissionMask->width;
							   		if (tmpPos.y < 0)
							   			tmpPos.y += pEmissionMask->height;
							
							   		colorMul += pEmissionMask->pBits[texPos.y * pEmissionMask->stride + texPos.x];
							   	}
							   }
							   // colorMul now holds our multFactor * (scale^2) * 255
							   colorMul *= (1.0/double(mapData.scale * mapData.scale));
                        colorMul /= 255.0;
                     }

						   staticMaps[ss].color[y*lmap.size.x + x]          += emitColor * colorMul;
						   staticMaps[ss].emissionBucket[y*lmap.size.x + x] += emitColor * colorMul;
						}
					}
				}

				else if (matPropList[materialIndex].emissionType == MaterialProp::flatColor) {
					MapData &mapData = mapDataList[ss];
					LightMap &lmap   = staticMaps[ss];
               Color setColor = matPropList[materialIndex].setColor;
					for (int y = 0; y < lmap.size.y; y++) {
						for (int x = 0; x < lmap.size.x; x++) {
							// Create texCoord point
							Point2I texPos;
							texPos.x = (x * mapData.scale) - mapData.offset.x;
							texPos.y = (y * mapData.scale) - mapData.offset.y;
							
							// Average all emission values on the lightmap square...
                     double colorMul;
                     if (pEmissionMask == NULL) {
                        colorMul = 1.0;
                     } else {
                        colorMul = 0.0;
							   for (int i = -(mapData.scale/2); i < (mapData.scale/2); i++) {
							   	for (int j = -(mapData.scale/2); j < (mapData.scale/2); j++) {
							   		Point2I tmpPos = texPos;

							   		tmpPos.x += i;
							   		tmpPos.y += j;

							   		tmpPos.x %= pEmissionMask->width;
							   		tmpPos.y %= pEmissionMask->height;
							   		if (tmpPos.x < 0)
							   			tmpPos.x += pEmissionMask->width;
							   		if (tmpPos.y < 0)
							   			tmpPos.y += pEmissionMask->height;
							
							   		colorMul += pEmissionMask->pBits[texPos.y * pEmissionMask->stride + texPos.x];
							   	}
							   }
							   // colorMul now holds our multFactor * (scale^2) * 255
							   colorMul *= (1.0/double(mapData.scale * mapData.scale));
                        colorMul /= 255.0;
                     }
						   staticMaps[ss].color[y*lmap.size.x + x] += setColor * colorMul;
						}
					}
				}

				else if (emittingMaterials &&
				         matPropList[materialIndex].emissionType == MaterialProp::loadFromTexture) {
					AssertFatal(matPropList[materialIndex].pBitmap != NULL, "Error, NULL Bitmap!");
					MapData &mapData = mapDataList[ss];
					LightMap &lmap   = staticMaps[ss];
					const GFXBitmap *pBitmap = matPropList[materialIndex].pBitmap;

					for (int y = 0; y < lmap.size.y; y++) {
						for (int x = 0; x < lmap.size.x; x++) {
							// Create texCoord point
							Point2I texPos;
							texPos.x = (x * mapData.scale) - mapData.offset.x;
							texPos.y = (y * mapData.scale) - mapData.offset.y;
							
							Color emitColor;
							emitColor.clearRGB();
							// Average all colors on the lightmap square...
							for (int i = -(mapData.scale/2); i < (mapData.scale/2); i++) {
								for (int j = -(mapData.scale/2); j < (mapData.scale/2); j++) {
									Point2I tmpPos = texPos;

									tmpPos.x += i;
									tmpPos.y += j;

									tmpPos.x %= pBitmap->width;
									tmpPos.y %= pBitmap->height;
									if (tmpPos.x < 0)
										tmpPos.x += pBitmap->width;
									if (tmpPos.y < 0)
										tmpPos.y += pBitmap->height;
							
									UInt8 colorIndex = pBitmap->pBits[texPos.y * pBitmap->width + texPos.x];
									emitColor.red   +=
										(pPalette->color[colorIndex].peRed * matPropList[materialIndex].intensity) / 255,0;
									emitColor.green +=
										(pPalette->color[colorIndex].peGreen * matPropList[materialIndex].intensity) / 255,0;
									emitColor.blue  +=
										(pPalette->color[colorIndex].peBlue * matPropList[materialIndex].intensity) / 255,0;
								}
							}
							// Average all emission values on the lightmap square...
                     double colorMul;
                     if (pEmissionMask == NULL) {
                        colorMul = 1.0;
                     } else {
                        colorMul = 0.0;
							   for (int i = -(mapData.scale/2); i < (mapData.scale/2); i++) {
							   	for (int j = -(mapData.scale/2); j < (mapData.scale/2); j++) {
							   		Point2I tmpPos = texPos;

							   		tmpPos.x += i;
							   		tmpPos.y += j;

							   		tmpPos.x %= pEmissionMask->width;
							   		tmpPos.y %= pEmissionMask->height;
							   		if (tmpPos.x < 0)
							   			tmpPos.x += pEmissionMask->width;
							   		if (tmpPos.y < 0)
							   			tmpPos.y += pEmissionMask->height;
							
							   		colorMul += pEmissionMask->pBits[texPos.y * pEmissionMask->stride + texPos.x];
							   	}
							   }
							   // colorMul now holds our multFactor * (scale^2) * 255
							   colorMul *= (1.0/double(mapData.scale * mapData.scale));
                        colorMul /= 255.0;
                     }

							// emit Color now holds our color * (scale^2)
							emitColor = emitColor * (1.0/double(mapData.scale * mapData.scale));

							lmap.emissionBucket[y * lmap.size.x + x] += emitColor * colorMul;
							lmap.color[y * lmap.size.x + x]          += emitColor * colorMul;
						}
					}
				}
			}
		}
	}
	printf("done.\n");
	
	// We don't need these points anymore...
	delete [] pMapPList;
	pMapPList = NULL;

	printf("\tBuilding Marked List...");
	for (int i = 0; i < geometry.surfaceList.size(); i++) {
		// We need to rebuild these points non-clamped...
		mapDataList[i].buildPointList(geometry,geometry.surfaceList[i],
												&markPMapPList[i]);
		mapDataList[i].markPointList(geometry,geometry.surfaceList[i],
												&markPMapPList[i]);
	}
	printf("done.\n");

	//  Second pass.  Go until maxPasses is exceeded, or the maximum emission
	// in a pass drops below the emission threshold.
	//
	printf("\tBeginning Radiosity Emission Passes...");
	int numPasses   = 0;
	int maxEmission = emissionThreshold + 1;
	while (numPasses++ < maxRadiosityPasses && maxEmission >= emissionThreshold) {
		maxEmission = -1;
		for (int ss = 0; ss < geometry.surfaceList.size(); ss++) {
			ITRGeometry::Surface& bsurface = geometry.surfaceList[ss];
			TPlaneF&	 plane = geometry.planeList[bsurface.planeIndex];
			LightMap& lmap = staticMaps[ss];
			
			emitRadiosityLight(geometry,
									markPMapPList, markPMapPList[ss],
									staticMaps, lmap,
									plane, maxEmission,
									matPropList, ss);
		} // for (int ss = 0...
		printf(" %d", numPasses);
	} // while (numPasses++...
	printf(" done\n");

	printf("\tFiltering and Remapping Invisible Points...");
	for (int ss = 0; ss < geometry.surfaceList.size(); ss++) {
		MapPointList &pList = markPMapPList[ss];
		LightMap     &lmap  = staticMaps[ss];

		for (int i = 0; i < pList.size(); ) {
			int count = pList[i].average;
			
			int numVisible = 0;
			for (int j = 0; j < count; j++)
				if (pList[i+j].visible == true)
					numVisible++;

			if (numVisible != 0) {
				lmap.emissionAbsorb[pList[i].mapIndex] =
					(lmap.emissionAbsorb[pList[i].mapIndex] * (float(count)/float(numVisible)));
			}
			
			i += count;
		}
	}

	// move all light absorbed from radiosity into the actual color buckets.
	for (int ss = 0; ss < geometry.surfaceList.size(); ss++) {
		LightMap &lmap = staticMaps[ss];

		for (int y = 0; y < lmap.size.y; y++) {
			for (int x = 0; x < lmap.size.x; x++) {
				lmap.color[y*lmap.size.x + x] += lmap.emissionAbsorb[y*lmap.size.x + x];
				lmap.emissionAbsorb[y*lmap.size.x + x].clearRGB();
			}
		}
	}

	for (int ss = 0; ss < geometry.surfaceList.size(); ss++) {
		MapPointList &pList = markPMapPList[ss];
		LightMap		 &lmap  = staticMaps[ss];

		Vector<int> surfaceMatches;
		for (int i = 0; i < geometry.surfaceList.size(); i++) {
			if (i == ss)
				continue;
			// if surfaces are on the same plane, push back the index...
			if ((geometry.surfaceList[ss].planeIndex == geometry.surfaceList[i].planeIndex) &&
			    (geometry.surfaceList[ss].planeFront == geometry.surfaceList[i].planeFront)) {
				surfaceMatches.push_back(i);
			}
		}

		for (int i = 0; i < pList.size(); ) {
			int count = pList[i].average;
			Color temp;
			temp.clearRGB();
			for (int pp = i; pp < (i + count); pp++) {
				float minDist     = 0.25;
				int newSurfIndex  = -1;
				int newLMapIndex  = -1;
				if (pList[pp].visible == true) {
					temp += lmap.color[pList[pp].mapIndex];
					continue;
				}

				for (int ms = 0; ms < surfaceMatches.size(); ms++) {
					MapPointList &matchList = markPMapPList[surfaceMatches[ms]];
					for (int mp = 0; mp < matchList.size(); mp++) {
						MapPoint &matchPoint = matchList[mp];
						if (matchPoint.visible == false)
							continue;
						
						float distance = getDistSQ(pList[pp].pos, matchPoint.pos);
						if (distance < minDist) {
							minDist = distance;
							newSurfIndex  = surfaceMatches[ms];
							newLMapIndex  = matchPoint.mapIndex;
						}
					}
				}
				
				if (newSurfIndex != -1) {
					// we found a valid point on another surface...
					temp += staticMaps[newSurfIndex].color[newLMapIndex];
				} else {
					// else, remap to the closest visible point on this surface
					int newPointIndex = pList[pp].nearestVisible;
					if (newPointIndex != -1) {
						int newMapIndex = pList[newPointIndex].mapIndex;
						temp += lmap.color[newMapIndex];
					} else {
						// oops, goof...
					}
				}
			}

			// ok, temp now holds color*count, insert into our lightmap...
			//
			temp = temp * (1.0f/float(count));
			lmap.color[pList[i].mapIndex] = temp;

			i += count;
		}
      printf(".");
      fflush(stdout);
	}
	printf("done.\n");

	printf("\tWriting modified static Lightmaps");
	for (int ss = 0; ss < lighting->surfaceList.size(); ss++) {
		ITRLighting::Surface& lsurface = lighting->surfaceList[ss];
		modifyMapRGB(staticMaps[ss], lighting, lsurface.mapIndex);
	}
	printf("...done.\n");
	
	if (pMapPList != NULL)  delete [] pMapPList;
	if (markPMapPList != NULL)  delete [] markPMapPList;
	if (staticMaps != NULL) delete [] staticMaps;
}


//----------------------------------------------------------------------------
//
void ITRRadiosityLighting::radLightSurfacesDynamic(ITRGeometry& /*geometry*/,
	LightList& /*lights*/,ITRLighting* /*lighting*/)
{
//
//
//
}


//----------------------------------------------------------------------------
// Light the geometry file with the given lights and export
// to the given lighting struct.
//
void ITRRadiosityLighting::light(ITRGeometry& geometry,LightList& lights,
	ITRLighting* lighting,
	TS::MaterialList *materialList, MaterialPropList &matPropList,
	GFXPalette *pPalette)
{
	TPlaneF::DistancePrecision = distancePrecision;
	TPlaneF::NormalPrecision = normalPrecision;
	if (filterScale == 0.0f)
		lightFilter = false;
	lighting->geometryBuildId = geometry.buildId;
	lighting->lightScaleShift = lightScale;
	lighting->lightMapListSize = 0;
	lighting->surfaceList.setSize(geometry.surfaceList.size());

	if (testPattern) {
		// This should never happen...
		AssertFatal(0, "Error, test pattern not supported in Radiosity model.");
	}
	else {
		//   First Pass: perform normal lighting.  This pass is stored
		// in the mono component of the light maps, and ignored if the
		// light maps are treated as RGB values.  [Mark as MONO_USED?]
		printf("Beginning Ray Casting Stage, %d surfaces...",lighting->surfaceList.size());
		for (int s = 0; s < lighting->surfaceList.size(); s++) {
			basicLightSurface(geometry,lights,lighting,s);
		}
		printf("done.\n");

		//   Second Pass: perform radiosity calculations for lights with
		// only one state.  This information is stored in the RGB component
		// of the static surface lightmaps.  The mono component of these
		// lightmaps is not affected.  [Mark as RGB_USED?]
		//
		printf("Beginning Static Radiosity Stage:\n");
		radLightSurfaceStatic(geometry, lights, lighting, materialList, matPropList, pPalette);
		printf("Done.\n");

//		//   Third Pass: perform radiosity calculations for lights with
//		// more than one state, on a light-by-light basis.  Stored in RGB
//		// component of lightmaps, mono not affected.  [Mark as RGB_USED?]
//		// If the statelessRadiosity flag is set, then state lights are cast
//		// without radiosity.
//		//
//		if (statelessRadiosity == false) {
//			printf("Beginning Dynamic Radiosity Stage...");
//			radLightSurfacesDynamic(geometry, lights, lighting);
//			printf("done.\n");
//		} else {
//			// Stateless casting is already done
//		}

	}

	exportLights(lights,lighting);
}


//----------------------------------------------------------------------------
// Compare function used to sort ITRLighting::Lights in ::exportLights
//
static int _USERENTRY
ITRRadiosityLighting::compareLights(const void* a,const void* b)
{
	const ITRLighting::Light* aa =
		reinterpret_cast<const ITRLighting::Light*>(a);
	const ITRLighting::Light* bb =
		reinterpret_cast<const ITRLighting::Light*>(b);
	return aa->id - bb->id;
}


//----------------------------------------------------------------------------
// Export all the state animated lights to the lighting struct.
//
void ITRRadiosityLighting::exportLights(LightList& lights,ITRLighting* lighting)
{
	for (LightList::iterator itr = lights.begin(); itr != lights.end(); itr++)
		// Only exports lights with at least two state.
		if ((*itr)->state.size() > 1) {
			Light& light = **itr;
			lighting->lightList.increment();
			ITRLighting::Light& llight = lighting->lightList.last();

			llight.id = light.id;
			llight.stateCount = light.state.size();
			llight.stateIndex = lighting->stateList.size();
			lighting->stateList.setSize(lighting->stateList.size() + llight.stateCount);

			for (int c = 0; c < light.state.size(); c++) {
				ITRLighting::State& lstate =
					lighting->stateList[llight.stateIndex + c];
				Light::State& state = *light.state[c];

				lstate.dataCount = state.surfaceData.size();
				lstate.dataIndex = lighting->stateDataList.size();
				lighting->stateDataList.setSize(lighting->stateDataList.size() +
					lstate.dataCount);

				for (int s = 0; s < state.surfaceData.size(); s++) {
					ITRLighting::StateData& ldata =
						lighting->stateDataList[lstate.dataIndex + s];
					Light::State::Surface& csurf = state.surfaceData[s];

					ldata.surface = csurf.surface;
					ldata.lightIndex = csurf.lightIndex;
					ldata.mapIndex = csurf.mapIndex;
				}
			}
		}

	// Sort the lightList by ID for later searching.
	qsort(lighting->lightList.address(),lighting->lightList.size(),
		sizeof(ITRLighting::Light),compareLights);
}


//----------------------------------------------------------------------------
// Copy the light map into the Lighting data struct.
//
int ITRRadiosityLighting::exportMap(ITRRadiosityLighting::LightMap& map,
												ITRLighting* lighting, bool do_mono)
{
	// The light map is appended to the end of Lighting's
	// light map buffer.
	int mapIndex = lighting->mapData.size();
	int mapSize = map.size.x * map.size.y * sizeof(UInt16) +
			ITRLighting::LightMap::HeaderSize;
	lighting->mapData.setSize(lighting->mapData.size() + mapSize);
	ITRLighting::LightMap* lmap =
		(ITRLighting::LightMap*) &lighting->mapData[mapIndex];

	//  Convert and copy colors.
	UInt16* md = (UInt16*) &lmap->data;
	if (do_mono == true) {
		for (int y = 0; y < map.size.y; y++)
			for (int x = 0; x < map.size.x; x++) {
				UInt16 *dst = &md[y * map.size.x + x];
				Color& color = map.color[y * map.size.x + x];
				*dst = ITRLighting::buildColor(color.red,color.green,color.blue);
			}
	} else {
		for (int y = 0; y < map.size.y; y++)
			for (int x = 0; x < map.size.x; x++) {
				UInt16 *dst = &md[y * map.size.x + x];
				Color& color = map.color[y * map.size.x + x];
				*dst = ITRLighting::buildColorWOMono(color.red,color.green,color.blue);
			}
	}

	return mapIndex;
}


void
ITRRadiosityLighting::modifyMapRGB(ITRRadiosityLighting::LightMap &map,
											ITRLighting* lighting, int mapIndex)
{
	ITRLighting::LightMap* lmap =
		(ITRLighting::LightMap*) &lighting->mapData[mapIndex];

	UInt16* md = (UInt16*) &lmap->data;
	for (int y = 0; y < map.size.y; y++) {
		for (int x = 0; x < map.size.x; x++) {
			UInt16 *dst = &md[y * map.size.x + x];
			Color &color = map.color[y * map.size.x + x];
			if (monoRadiosity == true) {
				*dst = ITRLighting::buildColor(color.red, color.green, color.blue);
			} else {
				ITRLighting::changeRGB(dst, color.red, color.green, color.blue);
			}
		}
	}
}


// Quick (writing time, not execution) function to determine whether a point is
// inside the given surface
bool
pointContainedInSurf(ITRGeometry *in_geometry, const ITRGeometry::Surface &in_surf,
							const Point3F &in_point)
{
	TPlaneF plane = in_geometry->planeList[in_surf.planeIndex];
	if (!in_surf.planeFront)
		plane.neg();

	// Build list of bounding planes for all polys
	Vector<TPlaneF> planeList;
	Vector<Point3F> polyCenter;
	Vector<Vector<TPlaneF>*> polyList;
	for (int p = 0; p < in_surf.polyCount; p++) {
		ITRGeometry::Poly& poly = in_geometry->polyList[in_surf.polyIndex + p];
		ITRGeometry::Vertex* vp = &in_geometry->vertexList[poly.vertexIndex];
		polyList.push_back(new Vector<TPlaneF>);
		Vector<TPlaneF>& pList = *polyList.last();

		polyCenter.increment();
		Point3F& center = polyCenter[p];
		center.set(0.0f,0.0f,0.0f);

		Point3F *start = &in_geometry->point3List
			[vp[poly.vertexCount - 1].pointIndex];
		for (int i = 0; i < poly.vertexCount; i++) {
			Point3F* end = &in_geometry->point3List[vp[i].pointIndex];
			Point3F lnormal,lvec = *end; lvec -= *start;
			float len = lvec.len();
			if (len > ITRRadiosityLighting::distancePrecision) {
				lvec.normalize();
				m_cross(lvec,plane,&lnormal);
				if (!isEqual(lnormal.len(),.0f)) {
					pList.increment();
					pList.last().set(*start,lnormal);
				}
			}
			center += *end;
			start = end;
		}
		center *= 1.0f / poly.vertexCount;
	}

	int p = 0;
	for (; p < polyList.size(); p++) {
		Vector<TPlaneF>& pList = *polyList[p];
		int pl = 0;
		for (; pl < pList.size(); pl++)
			if (pList[pl].whichSide(in_point) == TPlaneF::Outside)
				break;
		if (pl != pList.size()) {
			// Outside last poly
			break;
		}
	}

	bool returnVal = false;
	if (p == polyList.size()) {
		// Point was in every poly on this surface.
		returnVal = true;
	}

	for (int i = 0; i < polyList.size(); i++)
		delete polyList[i];

	return returnVal;
}


//----------------------------------------------------------------------------
// Collide a line with the geometry.
// Sets ITRCollision::collisionPoint to the collision point
// closest to the start point.
//
// Copied and modified from ITRCollision - DMM
//
bool findCollisionSurfaceLine(int nodeIndex,const Point3F& start,	const Point3F& end,
										int *out_surfaceIndex);

// to replace ITRCollision internal variables...
ITRGeometry	*geometry = NULL;
TMat3F		*transform = NULL;
LineSeg3F   line;

bool findCollisionSurface(ITRGeometry *in_geometry, TMat3F *in_transform,
									const Point3F& start,const Point3F& end,
									int *out_surfaceIndex)
{
	// Set up internal vars
	//
	geometry  = in_geometry;
	transform = in_transform;

	bool returnVal = findCollisionSurfaceLine(0,start,end,out_surfaceIndex);

	geometry  = NULL;
	transform = NULL;

	return returnVal;
}

bool findCollisionSurfaceLine(int nodeIndex,const Point3F& start,
	const Point3F& end, int *out_surfaceIndex)
{
	using ITRRadiosityLighting::distTolerance;

	if (nodeIndex >= 0) {
		ITRGeometry::BSPNode& node = geometry->nodeList[nodeIndex];
		TPlaneF& plane = geometry->planeList[node.planeIndex];
		TPlaneF::Side startSide = plane.whichSide(start);
		TPlaneF::Side endSide = plane.whichSide(end);
		switch(startSide * 3 + endSide) {
			case -4:		// S-, E-
			case -3:		// S-, E.
			case -1:		// S., E-
				return findCollisionSurfaceLine(node.back,start,end,out_surfaceIndex);
			case 4:		// S+, E+
			case 3:		// S+, E.
			case 1:		// S., E+
				return findCollisionSurfaceLine(node.front,start,end,out_surfaceIndex);
			case 0: {	// S., E.
				// Line lies on the plane.
				if (node.back >= 0)
					if (findCollisionSurfaceLine(node.back,start,end,out_surfaceIndex))
						return true;
				if (node.front >= 0)
					if (findCollisionSurfaceLine(node.front,start,end,out_surfaceIndex))
						return true;
				return false;
			}
			case -2: {	// S-, E+
				Point3F ip;
				plane.intersect(LineSeg3F(start,end),&ip);

				if (findCollisionSurfaceLine(node.back,start,ip,out_surfaceIndex))
					return true;
				return findCollisionSurfaceLine(node.front,ip,end,out_surfaceIndex);
			}
			case 2: {	// S+, E-
				Point3F ip;
				plane.intersect(LineSeg3F(start,end),&ip);

				if (findCollisionSurfaceLine(node.front,start,ip,out_surfaceIndex))
					return true;
				return findCollisionSurfaceLine(node.back,ip,end,out_surfaceIndex);
			}
		}
	}

	ITRGeometry::BSPLeaf& leaf = geometry->leafList[-(nodeIndex+1)];
	if (leaf.flags & ITRGeometry::BSPLeaf::Solid) {
		collisionPoint = start;
		Point3F collisionVec = end;
		collisionVec -= start;

		UInt8 *pbegin = &geometry->bitList[leaf.surfaceIndex];
		UInt8 *pend   = pbegin + leaf.surfaceCount;

		for (ITRCompressedBitVector::iterator itr(pbegin, pend); ++itr; ) {
			ITRGeometry::Surface &surface = geometry->surfaceList[*itr];
			TPlaneF &planeRef = geometry->planeList[surface.planeIndex];
			Point3F  normal    = planeRef;
			
			if (!surface.planeFront) {
				normal.neg();
			}

			float dot = m_dot(collisionVec, normal);
			if (dot >= 0.0f) {
				//  surface is either facing away from shooting ray, or is
				// parallel to it.
				//
				continue;
			}
			
			float distance = planeRef.distance(collisionPoint);
			if (distance < -distTolerance || distance > distTolerance) {
				// point is not on the surface...
				continue;
			}

			if (pointContainedInSurf(geometry, surface, collisionPoint)) {
				*out_surfaceIndex = *itr;
				return true;
			}
		}

		return false;
	}
	return false;
}
