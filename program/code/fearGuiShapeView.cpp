#include <fearguishapeview.h>

#include <g_surfac.h>
#include <fear.strings.h>

#define DEFAULT_NEAR_PLANE  1.0f
#define DEFAULT_FAR_PLANE   1.0E7f
#define DEFAULT_FOV         0.3f

namespace FearGui
{

static const char *gDefaultShapeFile = "logo.dts";

ShapeView::ShapeView()
: camera(NULL),  scene(NULL),    light(NULL), 
  rsShape(NULL), instance(NULL), rDist(0.0f), 
  rotation(0.0f, 0.0f, 0.0f), fAutoRotate(true), 
  fLeftMouseDown(false), fRightMouseDown(false)
{
   szShape[0] = '\0';
}

ShapeView::~ShapeView()
{
}

bool ShapeView::onAdd()
{
   RectF rect;
   float width, height;

   if (Parent::onAdd())
   {
      active = true;
      camera = new TSPerspectiveCamera(
         RectI(Point2I(0, 0),       Point2I(0, 0)),
         RectF(Point2F(0.0f, 0.0f), Point2F(0.0f, 0.0f)),
         256.0f, 1.0E8f);

      scene = new TSSceneLighting;
      scene->setAmbientIntensity(ColorF(0.7f, 0.7f, 0.7f));

      light = new TSLight;
      light->setType(TS::Light::LightDirectional);
      light->setIntensity(1.0f, 1.0f, 1.0f);
      scene->installLight(light);

      return (true);
   }

   return (false);
}

void ShapeView::onRemove()
{
   delete camera;
   delete scene;
   delete light;
   delete instance;

   Parent::onRemove();
}

void ShapeView::onWake()
{
   float width, height;

   if (camera)
   {
      // Set up the viewport
      width  = DEFAULT_NEAR_PLANE * tan(DEFAULT_FOV);
      height = (float)extent.y / (float)extent.x * width;

      camera->setWorldViewport(
         RectF(-width, height, width, -height));
      camera->setNearDist(DEFAULT_NEAR_PLANE);
      camera->setFarDist (getFarPlane());
      camera->setScreenViewport(
         RectI(position.x, position.y, 
               position.x + extent.x, position.y + extent.y));

      // Load the shape
      loadShape(gDefaultShapeFile);

      timer.reset();
   }
}

void ShapeView::loadShape(const char *lpszName)
{
   int iMin;

   if (instance)
   {
      if (! stricmp(lpszName, szShape)) return;
   
      delete instance;
      instance = NULL;
      szShape[0] = '\0';
   }

   if (lpszName == NULL || *lpszName == '\0')
   {
      lpszName = gDefaultShapeFile;
   }

   ResourceManager *rm = SimResource::get(manager);
   rsShape = rm->load(lpszName);

   if (bool(rsShape))
   {
      strcpy(szShape, lpszName);

      instance = new TSShapeInstance(rsShape, *rm);

      rotation.x = -0.25f;
      rotation.z = M_PI - 0.78f;

      last.set(0, 0);
      center = instance->getShape().fCenter;

      camera->lock();

      iMin     = min(extent.x, extent.y);
      rMinDist = camera->
                 projectionDistance(iMin, instance->getShape().fRadius * 2.5f);
      rDefDist = camera->projectionDistance(iMin, 2.0f);
      rMaxDist = camera->projectionDistance(iMin, 4.0f);

      if (rDist == 0)
      {
         rDist = rDefDist;
      }
      else if (rDist < rMinDist)
      {
         rDist = rMinDist;
      }
      else if (rDist > rMaxDist)
      {
         rDist = rMaxDist;
      }

      camera->unlock();
   }
}

Int32 ShapeView::getMouseCursorTag()
{
   if (fRightMouseDown)
   {
      return (IDBMP_CURSOR_ZOOM);
   }
   else if (fLeftMouseDown)
   {
      //return (IDBMP_CURSOR_ROTATE);
      return 0;
   }

   return (Parent::getMouseCursorTag());
}

void ShapeView::onRightMouseDown(const SimGui::Event &event)
{
   Parent::onRightMouseDown(event);

   fRightMouseDown = true;
   fLeftMouseDown  = false;
   last            = event.ptMouse;
}

void ShapeView::onMouseDown(const SimGui::Event &event)
{
   Parent::onMouseDown(event);

   fLeftMouseDown  = true;
   fRightMouseDown = false;
   last            = event.ptMouse;
}

void ShapeView::onRightMouseUp(const SimGui::Event &event)
{
   Parent::onRightMouseUp(event);

   fRightMouseDown = false;
}

void ShapeView::onMouseUp(const SimGui::Event &event)
{
   Parent::onMouseUp(event);

   fLeftMouseDown = false;
}

void ShapeView::onRightMouseDragged(const SimGui::Event &event)
{
   rDist += ((last.y - event.ptMouse.y) * 0.025f);

   if (rDist < rMinDist)
   {
      rDist = rMinDist;
   }
   else if (rDist >= rMaxDist)
   {
      rDist = rMaxDist;
   }

   last = event.ptMouse;
}

void ShapeView::onMouseDragged(const SimGui::Event &event)
{
   if (event.ptMouse.x != last.x)
   {
      rotation.z += ((event.ptMouse.x - last.x) * 0.01f);

      if (rotation.z >= M_2PI)
      {
         rotation.z -= M_2PI;
      }
      else if (rotation.z <= 0)
      {
         rotation.z = M_2PI;
      }
   }
   if (event.ptMouse.y != last.y)
   {
      rotation.x += ((last.y - event.ptMouse.y) * 0.01f);

      if (rotation.x >= M_2PI)
      {
         rotation.x -= M_2PI;
      }
      else if (rotation.x <= 0)
      {
         rotation.x = M_2PI;
      }
   }

   last = event.ptMouse;
}

bool ShapeView::becomeFirstResponder()
{
   return (false);
}

void ShapeView::onPreRender()
{
   if (instance)
   {
      if (fAutoRotate && fLeftMouseDown == false && timer.getElapsed() >= 0.033f)
      {
         timer.reset();
         rotation.z += 0.1f;

         if (rotation.z >= M_2PI)
         {
            rotation.z -= M_2PI;
         }
      }

      setUpdate();
   }
}

void ShapeView::onRender(GFXSurface *sfc, Point2I offset, const Box2I &)
{
   Box2I   box;
   RectI   rect;
   TMat3F  mat, mat2;
   Point3F lightpos, lightaim, screen[4];

   if (instance)
   {
      // Create the camera / light transform
      mat.identity();
      mat.p.set(0.0f, -rDist, 0.0f);

      // Orient rotation around a FIXED point -- looks much nicer
      m_mul(mat, RMat3F(EulerF(rotation.x, rotation.y, -rotation.z)), &mat2);
      
      mat2.flags |= TMat3F::Matrix_HasTranslation | TMat3F::Matrix_HasRotation;

      lightpos = mat2.p;
      mat2.inverse();

      camera->setTWC(mat2);

      // Drop the light
      light->setPosition(lightpos);
      lightaim = center - lightpos; 
      lightaim.normalize();
      light->setAim(lightaim);

      // Set up the render context
      context.setCamera(camera);
      points.reset();
      context.setPointArray(&points);
      context.setSurface(sfc);
      context.setLights(scene);

      // Lock the context and begin drawing
      context.lock();
      sfc->draw3DBegin();

      // Have to clear the background
      screen[0].set(offset.x,            offset.y,            0.0f);
      screen[1].set(offset.x + extent.x, offset.y,            0.0f);
      screen[2].set(offset.x + extent.x, offset.y + extent.y, 0.0f);
      screen[3].set(offset.x,            offset.y + extent.y, 0.0f);

      // Tell our nifty-keen hardware how to draw
      sfc->setHazeSource (GFX_HAZE_NONE);
      sfc->setShadeSource(GFX_SHADE_NONE);
      sfc->setAlphaSource(GFX_ALPHA_NONE);
      sfc->setFillMode   (GFX_FILL_CONSTANT);
      sfc->setFillColor  (Int32(0));
      sfc->setZTest      (GFX_ZWRITE);

      sfc->addVertex(&screen[0]);
      sfc->addVertex(&screen[1]);
      sfc->addVertex(&screen[2]);
      sfc->addVertex(&screen[3]);
      sfc->emitPoly();
      sfc->setZTest(GFX_ZTEST_AND_WRITE);

      // Now render the shape
		TMat3F transform (EulerF (0, 0, 0), -instance->getShape().fCenter);
		camera->pushTransform (transform);
      instance->render(context);
		camera->popTransform ();

      // Done, unlock
      sfc->draw3DEnd();
      context.unlock();
   }

   // We kind of brute-force our draw, so we need to tell anything
   // overlapping us to re-draw
   rect.upperL = offset - 1;
   rect.lowerR = offset + extent + 1;

   sfc->setClipRect(&rect);

   box.fMin = rect.upperL;
   box.fMax = rect.lowerR;

   renderChildControls(sfc, offset, box);
}

void ShapeView::inspectRead(Inspect *inspect)
{
   char szName[256];

   Parent::inspectRead(inspect);

   inspect->read(IDITG_FM_SHAPE, szName);

   loadShape(szName);
}

void ShapeView::inspectWrite(Inspect *inspect)
{
   Parent::inspectWrite(inspect);

   inspect->write(IDITG_FM_SHAPE, szShape);
}

IMPLEMENT_PERSISTENT_TAG(ShapeView, FOURCC('F','G','s','v'));

Persistent::Base::Error ShapeView::read(StreamIO &sio, int iVer, int iUsr)
{
   sio.readString(szShape);
   return (Parent::read(sio, iVer, iUsr));
}

Persistent::Base::Error ShapeView::write(StreamIO &sio, int iVer, int iUsr)
{
   sio.writeString(szShape);
   return (Parent::write(sio, iVer, iUsr));
}

float ShapeView::getFarPlane()
{
   return DEFAULT_FAR_PLANE;
}

};

