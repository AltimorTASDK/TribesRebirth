//----------------------------------------------------------------------------


//----------------------------------------------------------------------------

#ifndef _GRIDMETRICS_H_
#define _GRIDMETRICS_H_

//---------------------------------------------------------------------------

struct GridMetricsInfo
{
	struct Render {
		int blocks;
		int vertices;
		int squares;
		int polys;
		int textureBuild;
		int textureRegister;
		int lightMapBuild;
		//
		void incBlocks() { blocks++; }
		void incVertices() { vertices++; }
		void incSquares() { squares++; }
		void incPolys() { polys++; }
		void incTextureRegister() { textureRegister++; }
		void incTextureBuild() { textureBuild++; }
		void incLightMapBuild() { lightMapBuild++; }
		void reset();
	} render;
};

extern GridMetricsInfo GridMetrics;


#endif
