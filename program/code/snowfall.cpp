// Snowfall class taken from skiing
// just functional right now, still needs to be cleaned up and tailored
// for Earthsiege/Fear

#include <g_surfac.h>
#include <g_pal.h>
#include <g_bitmap.h>
#include "SoundFX.h"
#include "FearSFX.h"
#include "sfx.strings.h"
#include "InteriorShape.h"
#include "d_caps.h"
#include "interiorShape.h"

#include "snowfall.h"
// tied to earthsiege for now:
//#include "esEvDcl.h"
#include "FearDcl.h"
#include "Fear.Strings.h"
#include "FearGlobals.h"
#include "GameBase.h"
#include "FearPlayerPSC.h"
#include "ts.h"
#include "console.h"

//#include <itag.strings.h>
//#include <persistTags.h>

//simple types
typedef signed long int s32;
typedef unsigned long int u32;
typedef unsigned char u8;
typedef float f32;


#define MAXFLAKES 4096
#define COLORRENDER 0 
	//choose 0 for white flakes, 1 for a mix
#define MINSIZE 0.3
#define MAXSIZE 0.75
#define LACEMASK 7

#define RAINSPEED -75
#define RAINAMOUNT 2048


//complex types

typedef struct
 {
  Point3F world;
  u8 active;
  u8 clipped;
  f32 distance;
  Point3F drift;
 }
snow;

typedef struct
 {
 	int MaxFlakes;
  u32 NumFlakes;
  u32 DesiredFlakes;
  snow Flake[MAXFLAKES];
  f32 SphereRadius;
  f32 SphereRadiusSquared;
  f32 snowNearDist;
  f32 maxRadius; // of an individual flake
  Point3F CameraForward,CameraUp,CameraRight;
  Point3F Camera,OldCamera;
  Point3F Wind;
  bool rain;
 }
snowGlobals;


//variables
snowGlobals Snow;
u32 fastRandSeed=0;
static u32 interlace;
int FillColor=0;
u8 DesiredR,DesiredG,DesiredB;


//functions
static u32 fastRand(void)
 {
  fastRandSeed*=5;
  fastRandSeed+=(fastRandSeed>>13)+1;
  return fastRandSeed;
 }


void SetSnowColor(u8 r,u8 g,u8 b)
 {
  DesiredR=r;
  DesiredG=g;
  DesiredB=b;
 }
 
 
bool Snowfall::precipitation = true;


//------------------------------------------------------------------------------
static void initSnow(snowGlobals *snow)
 {
  u32 i;

  FillColor=-1;
  SetSnowColor(255,255,255);

  snow->NumFlakes=0;
  snow->CameraForward.x=0;
  snow->CameraForward.y=1;
  snow->CameraForward.z=0;
  snow->Camera.x=0;
  snow->Camera.y=0;
  snow->Camera.z=0;
  snow->SphereRadius=200;
  snow->SphereRadiusSquared=snow->SphereRadius*snow->SphereRadius;
  snow->maxRadius = MAXSIZE;
  snow->snowNearDist = 2;
  for (i=0;i<MAXFLAKES;i++)
   {
    snow->Flake[i].active=0;
    snow->Flake[i].drift.x=0;
    snow->Flake[i].drift.y=0;
    snow->Flake[i].drift.z=0;
   }
 }

//------------------------------------------------------------------------------
static void createAFlake(snowGlobals *snow)
{
  u32 i;
  f32 r;
  s32 r2;
  f32 sx,sy,sz;

  if (snow->NumFlakes<snow->DesiredFlakes)
   {
    r=snow->SphereRadius;
    r2=(s32)(r+r);
    sx=snow->Camera.x-r;
    sy=snow->Camera.y-r;
    sz=snow->Camera.z-r;
    for (i=0;i<MAXFLAKES;i++)
     {
      if (snow->Flake[i].active==0)
       {
        snow->Flake[i].active=1;
        snow->NumFlakes++;
        snow->Flake[i].world.x=sx+(f32)(fastRand()%r2);
        snow->Flake[i].world.y=sy+(f32)(fastRand()%r2);
        snow->Flake[i].world.z=sz+(f32)(fastRand()%r2);
        return;
       }
     }
   }
}

//---------------------------------------------------------------------------
static void updateFlakes(snowGlobals *snow,f32 timediff,TSCamera * camera)
{
  u32 i;
  Point3F diff;
  f32 distSquared;
  f32 dot;
  Point3F fpoint;
  f32 dx,dy,dz,d,dotX,dotY,x,y,z;
//  u8 tooFar = false;

  interlace++;
  interlace&=LACEMASK;

  //has the camera moved too far?
  dx=snow->OldCamera.x-snow->Camera.x;
  dy=snow->OldCamera.y-snow->Camera.y;
  dz=snow->OldCamera.z-snow->Camera.z;
  if (dx*dx+dy*dy+dz*dz>snow->SphereRadiusSquared/4)
  	{
//	 tooFar = true;
    for (i=0;i<MAXFLAKES;i++)
      if (snow->Flake[i].active!=0)
       {
        snow->Flake[i].world.x-=dx;
        snow->Flake[i].world.y-=dy;
        snow->Flake[i].world.z-=dz;
       }
	 }

/*
  //Let this check run if you want to see just what conditions
  // are resetting the snow camera. Only camera cuts should reset it.

  if (tooFar==1)
    MessageBox(0,"Camera Moved Far: Resetting.",0,0);
*/

  // get viewport info to determine left/right and up/down clipping
  const RectF & viewport = camera->getWorldViewport();
  float camNearInv = 1.0f/camera->getNearDist();
  float yRatio = 0.5f * (viewport.upperL.y-viewport.lowerR.y) * camNearInv;
  float xRatio = 0.5f * (viewport.lowerR.x-viewport.upperL.x) * camNearInv;

  for (i=0;i<MAXFLAKES;i++)
   {
    if (snow->Flake[i].active==0)
      continue;
    snow->Flake[i].clipped=0;

    //update position
	 if (!snow->rain)
	 	{
		    snow->Flake[i].world.x+=(snow->Wind.x+snow->Flake[i].drift.x)*timediff;
		    snow->Flake[i].world.y+=(snow->Wind.y+snow->Flake[i].drift.y)*timediff;
		    snow->Flake[i].world.z+=(snow->Wind.z+snow->Flake[i].drift.z)*timediff;
		    if ((fastRand()&15)==0)
		     {
		      snow->Flake[i].drift.x=((f32)(fastRand()%11)-5);
		      snow->Flake[i].drift.y=((f32)(fastRand()%11)-5);
		      snow->Flake[i].drift.z=((f32)(fastRand()%11)-5);
		     }
	  }
	 else
	 	{
			snow->Flake[i].world.x+=snow->Wind.x*timediff*2;
			snow->Flake[i].world.y+=snow->Wind.y*timediff*2;
			snow->Flake[i].world.z+=snow->Wind.z*timediff*2;
		}

    //kill flakes that fall out of sphere
    if (interlace==(i&LACEMASK)) //only check 1/8 of them at a time
     {
      diff.x=snow->Camera.x-snow->Flake[i].world.x;
      diff.y=snow->Camera.y-snow->Flake[i].world.y;
      diff.z=snow->Camera.z-snow->Flake[i].world.z;
      distSquared=diff.x*diff.x+diff.y*diff.y+diff.z*diff.z;
      if (distSquared>snow->SphereRadiusSquared)
       {
//        if ((snow->NumFlakes>snow->DesiredFlakes)||((fastRand()&31)==0))
        if ((snow->NumFlakes>snow->DesiredFlakes)||((fastRand()&7)==0))
         {
          snow->Flake[i].active=0;
          snow->Flake[i].world.x=0;
          snow->Flake[i].world.y=0;
          snow->Flake[i].world.z=0;
          snow->NumFlakes--;
         }
        else
         {
          snow->Flake[i].world.x=snow->Camera.x+diff.x*.95f;
          snow->Flake[i].world.y=snow->Camera.y+diff.y*.95f;
          snow->Flake[i].world.z=snow->Camera.z+diff.z*.95f;
         }
       }
     }

    x=snow->Flake[i].world.x-snow->Camera.x;
    y=snow->Flake[i].world.y-snow->Camera.y;
    z=snow->Flake[i].world.z-snow->Camera.z;

    snow->Flake[i].distance=dot=x*snow->CameraForward.x+y*snow->CameraForward.y+
        z*snow->CameraForward.z;

    //clipping code
    if (dot<snow->snowNearDist)
     {
      snow->Flake[i].clipped=1;
      continue;
     }

	 float invDot = 1.0f/dot;
    dotY = fabs( x*snow->CameraUp.x + y*snow->CameraUp.y + z*snow->CameraUp.z);
    if ((dotY + snow->maxRadius) * invDot > yRatio)
     {
      snow->Flake[i].clipped=1;
      continue;
     }

    dotX = fabs( x*snow->CameraRight.x + y*snow->CameraRight.y + z*snow->CameraRight.z);
    if ((dotX + snow->maxRadius) * invDot > xRatio)
     {
      snow->Flake[i].clipped=1;
      continue;
     }
   }
}


//---------------------------------------------------------------------------
static void snowLogicFrame(snowGlobals *snow,f32 timediff,TSCamera * camera)
{
  u32 i;
  
	if (!snow->rain)
		{
			if (snow->NumFlakes<snow->DesiredFlakes)
				createAFlake(snow);
			if (snow->NumFlakes<snow->DesiredFlakes*.8)
				for (i=0;i<5;i++)
					createAFlake(snow);
			if (snow->NumFlakes<snow->DesiredFlakes*.5)
				for (i=0;i<25;i++)
					createAFlake(snow);
		}
	else
		while (snow->NumFlakes < snow->DesiredFlakes)
			createAFlake (snow);
			
  updateFlakes(snow,timediff,camera);
}


//---------------------------------------------------------------------------
Snowfall::Snowfall()
{
	id = SnowfallId;
	image.init();
   rain = false;
	suspendRendering = 1;
	intensity = 0.0;
	intensityLimit = 100.0;
	netFlags.set( ScopeAlways );
	netFlags.set( Ghostable );
	
	image.sound = 0;
}

void Snowfall::onRemove()
{
	if (image.sound)
		Sfx::Manager::Stop(manager, image.sound);
	Parent::onRemove();
}

//---------------------------------------------------------------------------
void Snowfall::init()
 {
	image.manager = manager;
	image.bitmap = 0;
	Snow.Wind = wind;
	initSnow(&Snow);
	setIntensity(intensity);
	
	atenuation = 0.0;
	atenGoal = 0.0;
 }

//---------------------------------------------------------------------------
void Snowfall::setNearDist (float value)
{
	if (value < 2)
		value = 2;
		
	if (value > Snow.SphereRadius)
		value = Snow.SphereRadius;
		
	Snow.snowNearDist = value;
	
//	if (value < Snow.SphereRadius)
//	  	Console->printf("snowNearDist %f (on)", value);
//	else
//	  	Console->printf("snowNearDist %f (off)", value);
	
//	if (value >= Snow.SphereRadius)
//		setRendering (false);

	setRendering (value < Snow.SphereRadius);
}
//---------------------------------------------------------------------------
void Snowfall::setIntensity(float value)
{
	intensity = value;
   Snow.DesiredFlakes = intensity * (Snow.MaxFlakes);
	setMaskBits(updateIntensity);
}

//---------------------------------------------------------------------------
void Snowfall::setWind(const Point2F & direction)
{
	Snow.Wind.x    = direction.x;
	Snow.Wind.y    = direction.y;
	if (Snow.rain)
		Snow.Wind.z    = RAINSPEED;
	else
		Snow.Wind.z    = -35;
	wind = Snow.Wind;
	setMaskBits(updateWind);
}

//---------------------------------------------------------------------------
void Snowfall::setRendering(bool v)
{
	suspendRendering = !v;
	setMaskBits(updateRendering);
	
	if (isGhost())
		{
			if (image.sound)
				{
					if (v && precipitation)
						{
							if (atenuation >= atenGoal)
								atenuation -= 0.01;
								
							if (atenuation < atenGoal)
								atenuation += 0.01;
								
							if (atenuation < 0)
								atenuation = 0;
								
							if (atenuation > 1.0)
								atenuation = 1.0;
						}
					else
						if (precipitation)
							{
								if (atenuation < 0.35)
									atenuation += 0.007;
							}
						else
							atenuation = 1.0;
							
					Sfx::Manager *man = Sfx::Manager::find (manager);
					man->selectHandle (image.sound);
					man->setVolume (atenuation);
//				  	Console->printf("attenuation %f", atenuation);
				}
				
//			if (v)
//				{
//					if (!sound)
//						sound = Sfx::Manager::Play(manager, Snow.rain ? IDSFX_RAIN : IDSFX_SNOW);
//				}
//			else
//				if (sound)
//					{
//						Sfx::Manager::Stop(manager, sound);
//						sound = 0;
//					}
		}
}

//---------------------------------------------------------------------------
void Snowfall::setImageType(SimRenderImage::ImageType type)
{
	image.imageType = type;
}

//---------------------------------------------------------------------------
void Snowfall::resetColor()
{
	FillColor = - 1;
}


//------------------------------------------------------------------------------
bool Snowfall::onAdd()
{
   if (!Parent::onAdd()) return (false);
	if (!manager->isServer())
		addToSet(SimRenderSetId);

   Snow.rain = rain;
   if(Snow.rain)
		{
			setImageType (SimRenderImage::Translucent);
	      Snow.MaxFlakes = RAINAMOUNT;
		}
   else
		{
			setImageType (SimRenderImage::Normal);
	      Snow.MaxFlakes = 1024;
		}

   setWind(wind);
   setIntensity(intensity);

	init();
	
	if (Snow.rain)
		{
			Snow.SphereRadius = 110;
			Snow.SphereRadiusSquared = Snow.SphereRadius * Snow.SphereRadius;
			Snow.MaxFlakes = RAINAMOUNT;
		}
	else
		Snow.MaxFlakes = 1024;

   return (true);
}   

//---------------------------------------------------------------------------
bool Snowfall::processQuery(SimQuery* query)
{
	switch (query->type)
	{
		case SimRenderQueryImageType:
		{
			GameBase *player = cg.psc->getControlObject ();

			if (player)
				{
				   SimContainer *root = findObject(player->getManager(), SimRootContainerId, root);
					SimContainer *container = player->getContainer ();
					if (!container)
						container = root->findOpenContainer(player);

					if (container)
						{
							bool shouldRender = true;
							float nearDist = 2;
							
							atenGoal = 0;
							while (container != root && shouldRender && nearDist < Snow.SphereRadius)
								{
									if (container->getType() & SimInteriorObjectType)
										{
											// push nearDist out to edge of bounding box...
											float dist;

		                           if (dynamic_cast<InteriorShape*>(container)) {
											   shouldRender = ((InteriorShape *)container)->getWeatherDistance (player->getTransform(), dist);
		                           }
		                           else {
		                              shouldRender = false;
		                              dist = -1.0f;
		                              AssertFatal(false, "Unsupported interior shape type");
		                           }

											if (shouldRender && dist > 0)
												nearDist = dist;
												
											atenGoal = 0.025;
										}
										
									container = container->getContainer();
								}

							setNearDist (shouldRender ? nearDist : Snow.SphereRadius + 1);
						}
				}

			SimRenderQueryImage *qp = static_cast<SimRenderQueryImage*>(query);
			qp->count = suspendRendering ?	0 : 1;
			qp->image[0] = &image;
   		image.itype = image.imageType;

			return true;
		}
		default:
			return Parent::processQuery(query);
	}
}

//---------------------------------------------------------------------------
void Snowfall::inspectWrite(Inspect *inspector)
{
	Point2F w=wind=Snow.Wind;
   inspector->write(IDITG_SNOW_INTENSITY, (float)intensity);
   inspector->write(IDITG_SNOW_WIND, w);
}   

//---------------------------------------------------------------------------
void Snowfall::inspectRead(Inspect *inspector)
{
	Point2F w;   
   inspector->read(IDITG_SNOW_INTENSITY, (float)intensity);
	if (intensity > 1.0)
		intensity = 1.0;
	if (intensity < 0)
		intensity = 0;
   inspector->read(IDITG_SNOW_WIND, w);
	Snow.Wind.x = w.x;
	Snow.Wind.y = w.y;
	if (Snow.rain)
		Snow.Wind.z = RAINSPEED;
	else
		Snow.Wind.z = -35;
	wind=Snow.Wind;
	suspendRendering = IsEqual(intensity,0.0f);

   if (manager->isServer())
		setMaskBits(updateIntensity|updateWind|updateRendering);
}

//---------------------------------------------------------------------------
bool Snowfall::processArguments(int argc, const char **argv)
{
	if (argc>4)
	{
		Console->printf("Snowfall: intensity wind-x wind-y rain");
		return false;
	}
	
	if (argc>1)
		intensity = atof(argv[0]);
	else
		intensity=0;

	if (!IsEqual(intensity,0.0f))
		suspendRendering=false;

	Snow.Wind.set(0,0,-35);
	if (argc>1)
		Snow.Wind.x = atof(argv[1]);
	if (argc>2)
		Snow.Wind.y = atof(argv[2]);
		
	if (argc > 3 && atoi (argv[3]))
		Snow.rain = true;
	else
		Snow.rain = false;
		
   rain = Snow.rain;
	setIntensity(intensity);
	
	return true;
}


//---------------------------------------------------------------------------
void Snowfall::Image::init()
{
	imageType = SimRenderImage::Normal;
}


//---------------------------------------------------------------------------
void Snowfall::Image::render(TSRenderContext& rc)
 {
  static f32 time,oldtime;
 
  if (rc.getCamera()->getCameraType() == TS::OrthographicCameraType || !precipitation)
  	return;

  static Point2F fTextureVerts[] =
	{
		Point2F(0.0f,1.0f),
		Point2F(0.0f,0.0f),
		Point2F(1.0f,0.0f),
		Point2F(1.0f,1.0f),
	};

  GFXSurface* gfxSurface = rc.getSurface();
  TSPointArray* pa=rc.getPointArray();

	Bool textured = 0;
	if (textured)
	{
  		gfxSurface->setFillMode(GFX_FILL_TEXTURE);
  		gfxSurface->setTransparency(TRUE);
		gfxSurface->setTextureMap(bitmap);
		pa->useTextures(fTextureVerts);
	}
	else
	{
  		gfxSurface->setFillMode(GFX_FILL_CONSTANT);
  		gfxSurface->setTransparency(true);
  		pa->useTextures(false);
	}

  gfxSurface->setHazeSource(GFX_HAZE_NONE);
  gfxSurface->setShadeSource(GFX_SHADE_NONE);
  gfxSurface->setAlphaSource(GFX_ALPHA_NONE);
  gfxSurface->setTexturePerspective(FALSE);

  pa->useBackFaceTest(false);
  pa->useIntensities(false);
  pa->useHazes(false);
  pa->setVisibility(TS::ClipAllVis); // only draw if fully visible, clipping already done

  TMat3F camTrans=rc.getCamera()->getTCW();
  Snow.OldCamera=Snow.Camera;
  Snow.Camera=camTrans.p;
  
  camTrans.getRow(1,&Snow.CameraForward);
  camTrans.getRow(0,&Snow.CameraRight);
  camTrans.getRow(2,&Snow.CameraUp);

  oldtime=time;
  time=manager->getCurrentTime();
  
  snowLogicFrame(&Snow,time-oldtime,rc.getCamera());
  pa->reset(); 

	if (Snow.rain)
		renderRain (rc);
	else
		renderSnow (rc);
		
  gfxSurface->setTransparency(FALSE);
 
 	if (!sound)
		sound = Sfx::Manager::Play(manager, Snow.rain ? IDSFX_RAIN : IDSFX_SNOW);
}
 
void Snowfall::Image::renderRain (TSRenderContext &rc)
{
  TS::VertexIndexPair vip1[7] = { { 0,0 }, {1,1}, {2,2}, {3,3}, {4,4}, {5,5} };
  
  GFXSurface* gfxSurface = rc.getSurface();
  TSPointArray* pa=rc.getPointArray();

	Point3F rightOffset;
	Point3F localWind = Snow.Wind;
	localWind.z = -1;
	localWind.normalize();
	m_cross (Snow.CameraForward, localWind, &rightOffset);
  
	GFXPalette *gfxPalette=gfxSurface->getPalette();
	FillColor=gfxPalette->GetNearestColor(DesiredR/1.5,DesiredG/1.5,DesiredB/1.5);
	gfxSurface->setFillMode(GFX_FILL_CONSTANT);
	gfxSurface->setFillColor(FillColor);
	pa->useIntensities(false);
	if (gfxSurface->getCaps() & GFX_DEVCAP_SUPPORTS_CONST_ALPHA)
		{
	      gfxSurface->setAlphaSource(GFX_ALPHA_CONSTANT);
	      gfxSurface->setConstantAlpha(0.55);
		}

	for (int i=0;i<MAXFLAKES;i++)
		{
			if ((Snow.Flake[i].active==1)&&(Snow.Flake[i].clipped==0))
				{
					pa->reset();
					point[0] = Snow.Flake[i].world;

					point[2] = point[0] + localWind * 2; // Snow.CameraUp;

					point[1] = point[2] - rightOffset * .075; // Snow.CameraRight;
					point[3] = point[2] + rightOffset * .075; // Snow.CameraRight;

					point[2] = point[2] + localWind * .25; // Snow.CameraUp;

					for(int j = 0; j < 4; j++)
						vip1[j].fVertexIndex = pa->addPoint(point[j]);

					pa->drawPoly(4,vip1,0);
		     }
		}

	if (gfxSurface->getCaps() & GFX_DEVCAP_SUPPORTS_CONST_ALPHA)
      gfxSurface->setAlphaSource(GFX_ALPHA_NONE);
}
 
void Snowfall::Image::renderSnow (TSRenderContext &rc)
{
  TS::VertexIndexPair vip1[7] = { { 0,0 }, {1,1}, {2,2}, {3,3}, {4,4}, {5,5} };
  TS::VertexIndexPair vip2[7] = { { 3,3 }, {2,2}, {4,4}, {0,0}, {5,5}, {1,1} };
  f32 mult,a,b,c,x,y,z;
  
  GFXSurface* gfxSurface = rc.getSurface();
  TSPointArray* pa=rc.getPointArray();
  
   for (int i=0;i<MAXFLAKES;i++)
   {
     if ((Snow.Flake[i].active==1)&&(Snow.Flake[i].clipped==0))
     {
      #if COLORRENDER==0
        if (FillColor==-1)
         {
          GFXPalette *gfxPalette=gfxSurface->getPalette();
          FillColor=gfxPalette->GetNearestColor(DesiredR,DesiredG,DesiredB);
         }
        gfxSurface->setFillColor(FillColor);
      #elif COLORRENDER==1
        gfxSurface->setFillColor(16+fastRand()%16);
      #endif

		pa->reset();
	   mult=(MINSIZE+(f32)i/MAXFLAKES*(MAXSIZE-MINSIZE));

		bool farAway;
		
		farAway = Snow.Flake[i].distance>150;

      x=Snow.Flake[i].world.x;
      y=Snow.Flake[i].world.y;
      z=Snow.Flake[i].world.z;

      a=Snow.CameraUp.x*mult;
      b=Snow.CameraUp.y*mult;
      c=Snow.CameraUp.z*mult;

		if (!farAway)
		{
	      point[3].x=x-a;
   	   point[3].y=y-b;
      	point[3].z=z-c;
		}

      point[0].x=x+a;
      point[0].y=y+b;
      point[0].z=z+c;
		
		float width = 0.86;

      a=(Snow.CameraUp.x*.5-Snow.CameraRight.x*width)*mult;
      b=(Snow.CameraUp.y*.5-Snow.CameraRight.y*width)*mult;
      c=(Snow.CameraUp.z*.5-Snow.CameraRight.z*width)*mult;

      point[1].x=x-a;
      point[1].y=y-b;
      point[1].z=z-c;

		if (!farAway)
		{
      	point[4].x=x+a;
   	   point[4].y=y+b;
	      point[4].z=z+c;
		}

      a=(Snow.CameraUp.x*.5+Snow.CameraRight.x*width)*mult;
      b=(Snow.CameraUp.y*.5+Snow.CameraRight.y*width)*mult;
      c=(Snow.CameraUp.z*.5+Snow.CameraRight.z*width)*mult;

      point[2].x=x-a;
      point[2].y=y-b;
      point[2].z=z-c;

		if (!farAway)
		{
	      point[5].x=x+a;
   	   point[5].y=y+b;
      	point[5].z=z+c;
		}
		
		for(int j = 0; j < (farAway ? 3 : 6); j++)
			vip1[j].fVertexIndex = pa->addPoint(point[j]);
			
      if (farAway)
       {
        pa->drawPoly(3,&vip1[0],0);
       }
      else
       {
        if ((i&3)==0)
         {
          pa->drawPoly(3,&vip1[0],0);
          pa->drawPoly(3,&vip1[3],0);
         }
        else
          pa->drawPoly(6,&vip2[0],0);
       }
     }
   }
}

//---------------------------------------------------------------------------
// Standard persist io

IMPLEMENT_PERSISTENT_TAGS( Snowfall, FOURCC('S','N','O','W'), SnowfallPersTag );

//------------------------------------------------------------------------------

void Snowfall::initPersistFields()
{
   addField("intensity", TypeFloat, Offset(intensity, Snowfall));
   addField("wind", TypePoint3F, Offset(wind, Snowfall));
   addField("suspendRendering", TypeBool, Offset(suspendRendering, Snowfall));
   addField("rain", TypeBool, Offset(rain, Snowfall));
}

//------------------------------------------------------------------------------

Persistent::Base::Error Snowfall::read(StreamIO &s,int a,int b)
{
	Persistent::Base::Error error = Parent::read(s, a, b);
	if (error != Persistent::Base::Ok) 
	  return error;

	s.read( sizeof(intensity), (void*) &intensity);
	s.read( sizeof(wind), (void*) &wind);
	s.read( sizeof(suspendRendering), (void*) &suspendRendering);
	s.read( &rain);

	return (s.getStatus() == STRM_OK) ? Ok : WriteError;
}

//------------------------------------------------------------------------------
Persistent::Base::Error Snowfall::write(StreamIO &s,int a,int b)
{
	Persistent::Base::Error error = Parent::write(s, a, b);
	if (error != Persistent::Base::Ok) 
      return error;

	s.write( sizeof(intensity), (void*) &intensity);
	s.write( sizeof(Snow.Wind), (void*) &Snow.Wind);
	s.write( sizeof(suspendRendering), (void*) &suspendRendering);
	s.write (rain);

	return (s.getStatus() == STRM_OK) ? Ok : WriteError;
}

//------------------------------------------------------------------------------
DWORD Snowfall::packUpdate(Net::GhostManager *, DWORD mask, BitStream *stream)
{
	if (mask&updateIntensity)
	{
		stream->writeFlag(true);
		stream->write( sizeof(intensity), (void*) &intensity);
	}
	else
		stream->writeFlag(false);

	if (mask&updateWind)
	{
		stream->writeFlag(true);
		stream->write( sizeof(Snow.Wind), (void*) &Snow.Wind);
	}
	else
		stream->writeFlag(false);

	stream->writeFlag(suspendRendering);
	
	stream->writeFlag (rain);
   return 0;
}

//------------------------------------------------------------------------------
void Snowfall::unpackUpdate(Net::GhostManager *, BitStream *stream)
{
	if (stream->readFlag())
		stream->read( sizeof(intensity), (void*) &intensity);

	if (stream->readFlag())
		stream->read( sizeof(wind), (void*) &wind);

	suspendRendering = stream->readFlag();

	rain = stream->readFlag();
}


//---------------------------------------------------------------------------

