//------------------------------------------------------------------------------
// Description: Radiosity Lighting routines
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _ITRRADIOSITYLIGHTING_H_
#define _ITRRADIOSITYLIGHTING_H_

#include "ts_material.h"
#include "ITRBasicLighting.h"
#include <limits.h>

//----------------------------------------------------------------------------

namespace ITRRadiosityLighting
{
	struct LightMap;

	struct MapPoint {
		int average;	// Point to be averaged.
		Point3F pos;
		int mapIndex;
		bool visible;
		int  nearestVisible;
		MapPoint::MapPoint() : nearestVisible(INT_MIN) {} 
	};
	typedef Vector<MapPoint> MapPointList;

	struct Color {
		float red,green,blue,mono;
		Color& operator=(const Color&);
		Color& operator+=(const Color&);
		Color& operator*(const float);
		void   assignToMono()
			{ mono = red * 0.3f + green * 0.6f + blue * 0.1f; }
		void   clearRGB()
		   { red = green = blue = 0.0f; }
	};

	struct Light {
		struct State {
			struct Surface {
				int surface;
				int lightIndex;
				int mapIndex;
				Surface::Surface() : mapIndex(-1) { }
			};
			enum {
				PointLight,
				SpotLight
			} lightType;
			enum Falloff {
				Distance,
				Linear,
			} falloff;
			Point3F pos;
			Point3F spotVector;
			float spotTheta;     // inner angle for spotlights
			float spotPhi;       // outer angle for spotlights
			float radius;
			Color color;
			Point3F coneVec;
			float coneAngle;
			float d1,d2,d3;		// Distance coefficients
			bool useNormal;		// Use surface normal in calculations.
			bool  overrideGlobRadParms;
			float lightAbsorbLevel;
			float lightEmissionLevel;
			Vector<Surface> surfaceData;

			//
			bool light(ITRGeometry& geometry,MapPointList& pointList,
				Point3F& normal,LightMap* map);
			bool lightRadiosity(ITRGeometry& geometry,MapPointList& pointList,
				Point3F& normal,LightMap* map);
			bool isLit(ITRGeometry& geometry,ITRGeometry::Surface& surface);
			void appendSurface(int surface,int lightIndex, int mapIndex);
			int  findOrAppendSurface(int surf, ITRLighting *lighting, LightMap &lmap);
			int  getSurfMapIndex(int in_surfNum);
		};
		int id;
		Vector<State*> state;
	};
	typedef Vector<Light*> LightList;

	struct MaterialProp {
		bool	modified;
		enum {
			loadFromTexture,
			flatEmission,
			flatColor
		} emissionType;

		Color emitColor;
		Color setColor;
		float intensity;

		bool  modifiedPhong;
		float phongExp;
		
		GFXBitmap *pBitmap;
      GFXBitmap *pEmissionMask;
      
		bool overrideRadParams;
		float	reflectance;
		float	absorbance;
	};
	typedef Vector<MaterialProp> MaterialPropList;

	//
	using ITRBasicLighting::filterScale;
	using ITRBasicLighting::lightScale;
	using ITRBasicLighting::testPattern;
	using ITRBasicLighting::distancePrecision;
	using ITRBasicLighting::normalPrecision;
	extern   bool  statelessRadiosity;
	extern   bool  monoRadiosity;
	extern   bool  alphaTransmission;
   extern   bool  emittingMaterials;
	extern   int   quantumNumber;
	extern	int	maxRadiosityPasses;
	extern	int	emissionThreshold;
	extern	float	absorbanceLevel;
	extern	float	emissionLevel;

	//
	void light(ITRGeometry& geometry,LightList& lights,ITRLighting* lighting,
					TS::MaterialList *materialList, MaterialPropList &matPropList,
					GFXPalette *pPalette);
};

inline
ITRRadiosityLighting::Color&
ITRRadiosityLighting::Color::operator=(const ITRRadiosityLighting::Color& in_color)
{
	red   = in_color.red;
	green = in_color.green;
	blue  = in_color.blue;
	mono  = in_color.mono;

        return *this;
}

#endif // _ITRRADIOSITYLIGHTING_H_
