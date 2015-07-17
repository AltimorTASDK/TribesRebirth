#include <fearguiskinview.h>

#include <console.h>
#include <player.h>
#include <g_surfac.h>
#include <fear.strings.h>
#include <d_caps.h>
#include "fearGlobals.h"

#define SV_DEFAULT_NEAR_PLANE  1.0f
#define SV_DEFAULT_FAR_PLANE   1.0E7f
#define SV_DEFAULT_FOV         0.3f


static int g_rgiLArmorAnims[] =
{
   Player::ANIM_TUMBLE_LOOP,
   Player::ANIM_DIE_GRAB_BACK,
};

namespace SimGui
{
   extern Control *findControl(const char *name);
};

namespace FearGui
{

bool SkinView::sm_discoBoogie = false;
bool gbConsoleFunctionAdded = FALSE;

static const char *c_FGSkinSet(CMDConsole *, int, int argc, const char **argv)
{
   if(argc != 4)
   {
      Console->printf("%s(control, skin, gender);", argv[0]);
      return "false";
   }
   SimGui::Control *ctrl = SimGui::findControl(argv[1]);
   
   SkinView *skinCtrl = NULL;
   if (ctrl) skinCtrl = dynamic_cast<SkinView *>(ctrl);

   if (! skinCtrl)
   {
      Console->printf("%s - invalid control %s.", argv[0], argv[1]);
      return "false";
   }
   
   if (! stricmp(argv[3], "MALE"))
   {
      skinCtrl->setSkin(argv[2], TRUE);
   }
   else
   {
      skinCtrl->setSkin(argv[2], FALSE);
   }
   return "TRUE";
}

static const char *c_FGCycleArmor(CMDConsole *, int, int argc, const char **argv)
{
   if(argc != 2)
   {
      Console->printf("%s(control);", argv[0]);
      return "false";
   }
   SimGui::Control *ctrl = SimGui::findControl(argv[1]);

   SkinView *skinCtrl = NULL;
   if (ctrl) skinCtrl = dynamic_cast<SkinView *>(ctrl);

   if (! skinCtrl)
   {
      Console->printf("%s - invalid control %s.", argv[0], argv[1]);
      return "false";
   }
   
   skinCtrl->cycleArmor();
   return "TRUE";
}

SkinView::SkinView()
: camera(NULL),  scene(NULL),    light(NULL), 
  rsShape(NULL), instance(NULL), rDist(0.0f), 
  rotation(0.0f, 0.0f, 0.0f), 
  fLeftMouseDown(false), fRightMouseDown(false),
  thread(NULL)
{
   szShape[0] = '\0';

   m_male = true;
   m_loadedShapeMale = false;
   m_curArmor = Light;
}

SkinView::~SkinView()
{
}

void SkinView::setupViewport()
{
   int iMin;
   float width, height;

   // Set up the viewport
   width  = SV_DEFAULT_NEAR_PLANE * tan(SV_DEFAULT_FOV);
   height = (float)extent.y / (float)extent.x * width;

   camera->setWorldViewport(
      RectF(-width, height, width, -height));

   Point2I blah = localToGlobalCoord(Point2I(0, 0));

   camera->setScreenViewport(
      RectI(blah.x, blah.y, 
            blah.x + extent.x, blah.y + extent.y));

   if (instance)
   {
      camera->lock();
    
      Box3F bbox = instance->getShape().fBounds;
      float boxHeight = (m_male ? 2.65 : 2.5) / 2.0;
      float boxWidth  = (m_male ? 1.7 : 1.5) / 2.0;

      Point3F boxCenter = (bbox.fMin + bbox.fMax) * 0.5;
      
      float xDist = (boxWidth  / width)  * SV_DEFAULT_NEAR_PLANE;
      float yDist = (boxHeight / height) * SV_DEFAULT_NEAR_PLANE;
      rMinDist  = max(xDist, yDist);

      iMin     = min(extent.x, extent.y);

      rShift.set(0, 0, 0);
      rDefDist = rMinDist;
      rMaxDist = camera->projectionDistance(iMin, 10.0f);
      rDist    = rDefDist;

      camera->unlock();
   }
}

bool SkinView::onAdd()
{
   if (Parent::onAdd())
   {
      active = true;
      camera = new TSPerspectiveCamera(
         RectI(Point2I(1, 1),       Point2I(2, 2)),
         RectF(Point2F(1.0f, 1.0f), Point2F(2.0f, 2.0f)),
         256.0f, 1.0E8f);

      camera->setNearDist(SV_DEFAULT_NEAR_PLANE);
      camera->setFarDist (getFarPlane());

      scene = new TSSceneLighting;
      scene->setAmbientIntensity(ColorF(0.7f, 0.7f, 0.7f));

      light = new TSLight;
      light->setType(TS::Light::LightDirectional);
      light->setIntensity(1.0f, 1.0f, 1.0f);
      scene->installLight(light);

      setupViewport();

      m_lastUpdated = wg->currentTime;

      // Add the console function
      if (!gbConsoleFunctionAdded)
      {
         Console->addCommand(0, "FGSkin::set", c_FGSkinSet);
         Console->addCommand(0, "FGSkin::cycleArmor", c_FGCycleArmor);
         Console->addVariable(0, "FGSkin::discoBoogie", CMDConsole::Bool, &sm_discoBoogie);
         gbConsoleFunctionAdded = TRUE;
      }
      return (true);
   }

   return (false);
}

void SkinView::onRemove()
{
   delete camera;
   delete scene;
   delete light;
   delete instance;

   Parent::onRemove();
}

void SkinView::setSkin(const char *lpszSkin, bool fMale)
{
   int                         iCount;
   char                       *lpszTok;
   char                        szDMLFile[256];
   ResourceManager            *resManager;
   Resource<TS::MaterialList>  rsList;
   TS::MaterialList           *list;
   const char                 *lpszGender;
   ResourceObject             *object;

   resManager = SimResource::get(manager);

   m_male = fMale;
   setShape();

   sprintf(szDMLFile, "%s.%s.DML", lpszSkin, szShapeBase);

   object = resManager->find(szDMLFile);

   if (object == NULL || object->resource == NULL)
   {
      // Copy the shape's material list
      list  = new TS::MaterialList;
      *list = *(instance->getShape().getMaterialList());

      iCount = list->getMaterialsCount();

      // Cycle through all of the materials
      for (int i = 0; i < iCount; i ++)
      {
         TS::Material &material = list->getMaterial(i);

         // If this material is a texture ...
         if ((material.fParams.fFlags & TS::Material::MatFlags) == TS::Material::MatTexture)
         {
            // Copy in the new bitmap - this uses the same
            // bitmap for all damage levels, but what the
            // Hell, it's just a preview
            sprintf(material.fParams.fMapFile, 
               "%s.%s.BMP", lpszSkin, szShapeBase);
            if (resManager->findFile(material.fParams.fMapFile) == false) {
               sprintf(material.fParams.fMapFile, 
                  "%s.%s.BMP", "base", szShapeBase);
            }
         }
      }

      resManager->add(ResourceType::typeof(szDMLFile), szDMLFile, (void *)list);
   }

   rsList = resManager->load(szDMLFile, true);
   rsList->load(*resManager, true);
   instance->setMaterialList(rsList);
}

void
SkinView::cycleArmor()
{
   m_curArmor = (m_curArmor + 1) % NumArmors;
   setShape();
   setSkin(Console->getVariable("$PCFG::SkinBase"), m_male);

//   setShape();
}

void SkinView::setShape()
{
   const char* pShapeName;
   const char* pShapeBaseName;
   if (m_male == true) {
      switch (m_curArmor) {
        case Light:  pShapeName = "larmor.dts"; pShapeBaseName = "larmor"; break;
        case Medium: pShapeName = "marmor.dts"; pShapeBaseName = "marmor"; break;
        case Heavy:  pShapeName = "harmor.dts"; pShapeBaseName = "harmor"; break;
        default:
         AssertFatal(0, avar("No such armor: %d", m_curArmor));
      }
   } else {
      switch (m_curArmor) {
        case Light:  pShapeName = "lfemale.dts"; pShapeBaseName = "lfemale"; break;
        case Medium: pShapeName = "mfemale.dts"; pShapeBaseName = "mfemale"; break;
        case Heavy:  pShapeName = "harmor.dts";  pShapeBaseName = "harmor";  break;
        default:
         AssertFatal(0, avar("No such armor: %d", m_curArmor));
      }
   }

   int iMin;

   if (instance)
   {
      if (!stricmp(pShapeName, szShape) && m_loadedShapeMale == m_male) return;
   
      delete instance;
      instance = NULL;
      szShape[0] = '\0';
   }

   if (pShapeName == NULL || *pShapeName == '\0')
   {
      return;
   }

   ResourceManager *rm = SimResource::get(manager);
   rsShape = rm->load(pShapeName);

   if (bool(rsShape))
   {
      strcpy(szShape, pShapeName);
      strcpy(szShapeBase, pShapeBaseName);
      m_loadedShapeMale = m_male;

      instance = new TSShapeInstance(rsShape, *rm);

      rotation.x = -0.25f;
      rotation.z = M_PI;

      last.set(0, 0);
      center = instance->getShape().fCenter;

      setupViewport();

      thread = instance->CreateThread();

      bool set = false;
      if (sm_discoBoogie == true) {
         static char* pAnim;
         if (m_male) {
            switch (m_curArmor) {
              case Light:  pAnim = "celebration 2"; break;
              case Medium: pAnim = "celebration 3";   break;
              case Heavy:  pAnim = "celebration 2"; break;
            }
         } else {
            switch (m_curArmor) {
              case Light:  pAnim = "celebration 1"; break;
              case Medium: pAnim = "celebration 2"; break;
              case Heavy:  pAnim = "celebration 1"; break;
            }
         }
         if (instance->getShape().lookupName(pAnim) != -1)
         {
            thread->SetSequence(pAnim, 0.0f, true);
            thread->setTimeScale(1.0f);
            set = true;
         }
      }

      if (set == false) {
         if (instance->getShape().lookupName("run") != -1)
         {
            thread->SetSequence("run", 0.0f, true);
            thread->setTimeScale(1.0f);
         } else {
            instance->DestroyThread(thread);
            thread = NULL;
         }
      }
   }
}

Int32 SkinView::getMouseCursorTag()
{
   if (fRightMouseDown)
   {
      return (IDBMP_CURSOR_ZOOM);
   }

   return (Parent::getMouseCursorTag());
}

void SkinView::resize(const Point2I &newPos, const Point2I &newExt)
{
   Parent::resize(newPos, newExt);
   setupViewport();
}

void SkinView::onMouseDown(const SimGui::Event &event)
{
   Parent::onMouseDown(event);

   if (sm_discoBoogie) 
      if (event.mouseDownCount > 1)
         cycleArmor();

   fLeftMouseDown  = true;
   fRightMouseDown = false;
   last            = event.ptMouse;
}

void SkinView::onMouseUp(const SimGui::Event &event)
{
   Parent::onMouseUp(event);

   fLeftMouseDown = false;
}

void SkinView::onRightMouseDown(const SimGui::Event &event)
{
   Parent::onRightMouseDown(event);

   fRightMouseDown = true;
   fLeftMouseDown  = false;
   last            = event.ptMouse;
}

void SkinView::onRightMouseUp(const SimGui::Event &event)
{
   Parent::onRightMouseUp(event);

   fRightMouseDown = false;
}

void SkinView::onRightMouseDragged(const SimGui::Event &event)
{
   rDist += ((last.y - event.ptMouse.y) * (10.0f / extent.y));

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

void SkinView::onMouseDragged(const SimGui::Event &event)
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

   last = event.ptMouse;
}

bool SkinView::becomeFirstResponder()
{
   return (false);
}

void SkinView::onPreRender()
{
   setUpdate();
}

void SkinView::onRender(GFXSurface *sfc, Point2I offset, const Box2I &)
{
   DWORD updateSlice = wg->currentTime - m_lastUpdated;
   m_lastUpdated = wg->currentTime;

   if (thread && instance) {
      instance->animateRoot();
      thread->AdvanceTime(float(updateSlice) * 0.001f);
      instance->fRootDeltaTransform.identity();
   }

   Box2I   box;
   RectI   rect;
   TMat3F  mat, mat2;
   Point3F lightpos, lightaim, screen[4];

   bool ghosted = FALSE;
   SimGui::Control *topDialog = root->getDialogNumber(1);
   if (topDialog && (topDialog != getTopMostParent()) && (topDialog->findControlWithTag(IDCTG_DIALOG)))
   {
      ghosted = TRUE;
   }
   if (ghosted) return;
   
   if (instance)
   {
      // Create the camera / light transform
      mat.identity();
      mat.p    = instance->getShape().fCenter;
      mat.p.y -= rDist;
      mat.p.z -= 0.3f - (m_male ? 0.0 : 0.2);

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

      // Tell our nifty-keen hardware how to draw
      sfc->setFillMode   (GFX_FILL_CONSTANT);
      sfc->setHazeSource (GFX_HAZE_NONE);
      sfc->setShadeSource(GFX_SHADE_NONE);
      sfc->setAlphaSource(GFX_ALPHA_NONE);

      if (sfc->getCaps() & GFX_DEVCAP_SUPPORTS_CONST_ALPHA) {
         sfc->clearZBuffer();
      }

      // We have to make sure the camera is in WBuffer mode, this can goof
      //  up in certain scenarios...
      sfc->setZMode(true);
      sfc->setZTest(GFX_ZTEST_AND_WRITE);
      // Now render the shape
      instance->animate();
      instance->render(context);

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

IMPLEMENT_PERSISTENT_TAG(SkinView, FOURCC('F','s','k','v'));

Persistent::Base::Error SkinView::read(StreamIO &sio, int iVer, int iUsr)
{
   sio.readString(szShape);
   return (Parent::read(sio, iVer, iUsr));
}

Persistent::Base::Error SkinView::write(StreamIO &sio, int iVer, int iUsr)
{
   sio.writeString(szShape);
   return (Parent::write(sio, iVer, iUsr));
}

float SkinView::getFarPlane()
{
   return SV_DEFAULT_FAR_PLANE;
}

};
