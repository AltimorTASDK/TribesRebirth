//--------------------------------------------------------------------------- 


//--------------------------------------------------------------------------- 

#ifndef _SIMRENDERGRP_H_
#define _SIMRENDERGRP_H_

#include "simBase.h"
#include "simEvDcl.h"
#include <ts_camera.h>
#include <ts_types.h>
#include <ts_RenderItem.h>
#include <ts_RenderContext.h>


//--------------------------------------------------------------------------- 

class SimRenderImage: public TSRenderItem
{
public:
   enum ImageType { Normal = 0, Translucent, Overlay, Background };
   ImageType itype;
   float sortValue; // normally the w position of the center
                    // of the object
   int visibility;  // starting visibility flags -- to be filled in by container
   float hazeValue;

	int operator<(const SimRenderImage &ri) { return sortValue < ri.sortValue; }

	void render(TS::RenderContext &rc) = 0;
   SimRenderImage() { visibility = TS::ClipUnknown; hazeValue = -1; }
};

class SimRenderDefaultImage: public SimRenderImage
{
public:

	TMat3F transform;
	TSRenderItem* shape;
	void render(TSRenderContext &rc) {
		rc.getCamera()->pushTransform(transform);
		shape->render(rc);
		rc.getCamera()->popTransform();
	}
};


//--------------------------------------------------------------------------- 

struct SimRenderQueryImage: public SimQuery
{
	enum ImageSize {
      MaxImage = 256
   };
   bool containerRenderQuery;
   int count;
	SimRenderImage* image[MaxImage];
   TSRenderContext *renderContext;
	SimObject* viewportObject;
	SimRenderQueryImage(const bool in_containerRenderQuery = false)
    : containerRenderQuery(in_containerRenderQuery)
   {
      type = SimRenderQueryImageType;
      count = 0;
   }
};

class SimRenderSet: public SimSet
{
public:
	SimRenderSet() { id = SimRenderSetId; }
};


#endif
