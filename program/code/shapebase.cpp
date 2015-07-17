//--------------------------------------------------------------------------- 


//--------------------------------------------------------------------------- 


#include <sim.h>
#include <simResource.h>
#include <console.h>

#include <debris.h>
#include <partDebris.h>

#include "shapeBase.h"
#include "PlayerManager.h"
#include "simTerrain.h"
#include "interiorShape.h"

#include "editor.strings.h"
#include "commonEditor.strings.h"
#include "darkstar.strings.h"
#include "fear.strings.h"
#include "stringTable.h"
#include "fearGlobals.h"
#include "sensorManager.h"
#include "fearPlayerPSC.h"
#include "dataBlockManager.h"
#include "explosion.h"
#include "tsfx.h"
#include "g_bitmap.h"
#include "g_surfac.h"
#include "d_caps.h"
#include "gwDeviceManager.h"
#include "planet.h"


//--------------------------------------------------------------------------- 
//--------------------------------------------------------------------------- 

//--------------------------------------------------------------------------- 

IMPLEMENT_PERSISTENT(ShapeBase);

bool ShapeBase::prefUseCustomSkins = true;
int ShapeBase::prefDamageSkinDetail = 2;
int ShapeBase::prefShadowDetailMask = 0;
float ShapeBase::prefShadowDetailScale = 1.0;
bool ShapeBase::renderBoundingBox = false;
bool ShapeBase::renderLightRays = false;
const DWORD ShapeBase::csm_fadeLength = 2500;

bool g_lowDetailObjectLighting = true;


//--------------------------------------------------------------------------- 

ShapeBase::DamageSkinData::DamageSkinData()
{
   int i;
   for(i = 0; i < BmpCount; i ++)
      bmpName[i] = NULL;
}

void ShapeBase::DamageSkinData::pack(BitStream *stream)
{
   int i;
   for(i = 0; i < BmpCount; i++)
      stream->writeString(bmpName[i]);
}

void ShapeBase::DamageSkinData::unpack(BitStream *stream)
{
   int i;
   for(i = 0; i < BmpCount; i++)
   {
      bmpName[i] = stream->readSTString();
      if(bmpName[i][0])
      {
         char buf[256];
         sprintf(buf, "%s.bmp", bmpName[i]);
         bmps[i] = SimResource::get(cg.manager)->load(buf);
      }
   }
   for(i = 0; i < BmpCount; i++)
      if(!bool(bmps[i]))
         break;
   numBmps = i;
}

ShapeBase::ShapeBaseData::ShapeBaseData()
{
	fileName = 0;
	shieldShapeName = 0;
   damageSkinId = -1;
	sfxShield = -1;
	shadowDetailMask = 0;
	explosionId = -1;
	debrisId = -1;
	maxEnergy = 0;
	maxDamage = 1;
	repairRate = 0.1;
	for (int i = 0; i < MaxSequenceIndex; i++) {
		sequenceSound[i].sequence = 0;
		sequenceSound[i].sound = -1;
	}
}


//---------------------------------------------------------------------------

bool ShapeBase::ShapeBaseData::preload(ResourceManager *rm, bool server, char errorBuffer[256])
{
   server;
   char name[256];
	if (shieldShapeName && shieldShapeName[0]) {
		strcpy(name, shieldShapeName);
		strcat(name, ".dts");
		shieldShape = rm->load(name, true);
      if(!bool(shieldShape))
      {
         sprintf(errorBuffer, "Unable to model: %s.  The server has new art data you are missing.  Check server info for possible download locations.", name);
         return false;
      }
   }
   if(fileName && fileName[0])
   {
	   strcpy (name, fileName);
	   strcat (name, ".dts");
      shape = rm->load(name, true);
      if(!bool(shape))
      {
         sprintf(errorBuffer, "Unable to model: %s.  The server has new art data you are missing.  Check server info for possible download locations.", name);
         return false;
      }
   }
   return GameBase::GameBaseData::preload(rm, server, errorBuffer);
}

void ShapeBase::ShapeBaseData::pack(BitStream *stream)
{
	Parent::pack(stream);
   stream->writeString(fileName);
   stream->writeString(shieldShapeName);
	sg.dbm->writeBlockId(stream, DataBlockManager::SoundDataType,sfxShield);
   stream->writeInt(shadowDetailMask, 8);
	sg.dbm->writeBlockId(stream, DataBlockManager::ExplosionDataType,explosionId);
	sg.dbm->writeBlockId(stream, DataBlockManager::DamageSkinDataType,damageSkinId);
   stream->writeInt(debrisId, 32);
   stream->write(maxEnergy);

	for (int j = 0; j < MaxSequenceIndex; j++) {
		SequenceSound as = sequenceSound[j];
		if (stream->writeFlag(as.sequence)) {
			stream->writeString(as.sequence);
			sg.dbm->writeBlockId(stream, DataBlockManager::SoundDataType,as.sound);
		}
	}
   stream->writeFlag(isPerspective);
}

void ShapeBase::ShapeBaseData::unpack(BitStream *stream)
{
	Parent::unpack(stream);
   char buf[256];
   stream->readString(buf);
   fileName = stringTable.insert(buf);
   stream->readString(buf);
   shieldShapeName = stringTable.insert(buf);
	sfxShield = cg.dbm->readBlockId(stream, DataBlockManager::SoundDataType);
   shadowDetailMask = stream->readInt(8);
   explosionId = cg.dbm->readBlockId(stream, DataBlockManager::ExplosionDataType);
   damageSkinId = cg.dbm->readBlockId(stream, DataBlockManager::DamageSkinDataType);
   debrisId = stream->readInt(32);
   stream->read(&maxEnergy);

	for (int j = 0; j < MaxSequenceIndex; j++) {
		if (stream->readFlag()) {
		   stream->readString(buf);
			sequenceSound[j].sequence = stringTable.insert(buf);
			sequenceSound[j].sound= cg.dbm->readBlockId(stream, DataBlockManager::SoundDataType);
		}
   }

   isPerspective = stream->readFlag();
}


//--------------------------------------------------------------------------- 

ShapeBase::ShapeBase()
 : m_fading(NotFading),
   m_fadeStarted(0),
   m_lastSunUpdated(0)
{
   m_attenuateSun      = false;
   m_attenuationColor  = ColorF(1, 1, 1);
   m_attenuationFactor = 0.0f;
   m_ticks             = 0;
   m_rollColor         = ColorF(1, 1, 1);
   m_rollFactor        = 0.0f;

   curSkin = "base";
   curSkinDamage = 0;

	image.shape = 0;
	image.itype = SimRenderImage::Normal;
	image.castShadow = false;
	data = 0;
	movementStepSize = 0.3f;

	shieldImage.itype = SimRenderImage::Translucent;
	shieldImage.shape = NULL;
	shieldImage.castShadow = false;
	shieldThread = NULL;
	shieldVec.set (1, 0, 0);

   int i;
	for (i = 0; i < MaxScriptThreads; i++) {
		scriptThread[i].sequence = -1;
		scriptThread[i].thread = 0;
		scriptThread[i].sound = 0;
		scriptThread[i].state = SThread::Stop;
		scriptThread[i].atEnd = false;
		scriptThread[i].forward = true;
	}
   for (i = 0; i < MaxSoundChannels; i++)
      soundChannel[i] = 0;

	collisionMask = 0;
	rotation.set(0,0,0);
	destroyable = true;
	blowedUpGood = false;

	damageLevel = 0;
	repairReserve = 0;
	autoRepairRate = 0;
	rechargeRate = 0;
	boundingBox.fMin.set(0,0,0);
	boundingBox.fMax.set(0,0,0);
	SimContainer::setBoundingBox(boundingBox);

	zOffset = 0;
}


ShapeBase::~ShapeBase()
{
	delete image.shape;
	delete shieldImage.shape;
}


//----------------------------------------------------------------------------

void ShapeBase::setTeam(int teamId)
{
   Parent::setTeam(teamId);
   if(manager && bool(image.shape) && isGhost())
      reSkin();
}


const char *ShapeBase::getSkinBase()
{
   // default is to use the team's skin
   // player overrides with its own skin (if it's enabled on the server)

   PlayerManager::BaseRep *owner = cg.playerManager->findBaseRep(getLastOwnerClient());
   if(owner)
      return owner->skinBase;
   return cg.playerManager->getTeamSkinBase(getTeam());
}

void ShapeBase::damageBitmap(GFXBitmap *bmp)
{

   DWORD xPoint = 30149123, yPoint = 3419238501;
   DamageSkinData *dskin = (DamageSkinData *) cg.dbm->lookupDataBlock(data->damageSkinId, DataBlockManager::DamageSkinDataType);
   
   if(dskin && dskin->numBmps)
   {
      GFXSurface *mem = deviceManager.getGFXDeviceManager()->createMemSurface(bmp);
      mem->lock();
      int coverage = 0;
      int area = bmp->getWidth() * bmp->getHeight();
      int targetCoverage = (area * curSkinDamage) >> 2;
      int index = 0;
      int numskipped = 0;
      int i = 1;

      while(coverage < targetCoverage && numskipped < dskin->numBmps)
      {
         GFXBitmap *dbmp = dskin->bmps[index];
         int bmparea = dbmp->getWidth() * dbmp->getHeight();
         dbmp->attribute |= BMA_TRANSPARENT;
         if(dbmp->getWidth() >= bmp->getWidth() || dbmp->getHeight() >= bmp->getHeight())
            numskipped++;
         else
         {
            Point2I drawPoint((xPoint * i) % (bmp->getWidth() - dbmp->getWidth()),
                              (yPoint * i) % (bmp->getHeight() - dbmp->getHeight()));
            mem->drawBitmap2d(dbmp, &drawPoint);
            i++;
            coverage += bmparea;
         }
         index++;
         if(index >= dskin->numBmps)
            index = 0;
      }
      deviceManager.getGFXDeviceManager()->freeMemSurface(mem);
   }
}

void ShapeBase::buildMaterialList()
{
   static char fileName[256];

   sprintf(fileName, "%s_%d_%s.dml", data->fileName, curSkinDamage, prefUseCustomSkins ? curSkin : "base");
   ResourceManager *rm = SimResource::get(manager);
   ResourceObject *ro = rm->find(fileName);

   if(!ro || !ro->resource)
   {
      // construct the material list from curSkin and curDamageLevel
      TS::MaterialList *newML = new TS::MaterialList();
      *newML = *(image.shape->getShape().getMaterialList());
      int cnt = newML->getMaterialsCount();
      for(int i = 0; i < cnt; i++)
      {
         TS::Material *material = &newML->getMaterial(i);
         if ((material->fParams.fFlags & TS::Material::MatFlags) == TS::Material::MatTexture)
         {
            static char baseName[64];
            if(!strncmp("base.", material->fParams.fMapFile, 5) && prefUseCustomSkins)
            {
               // test if we have the base material at all
               sprintf(baseName, "%s.%s", curSkin, material->fParams.fMapFile + 5);
               if(!rm->findFile(baseName))
                  strcpy(baseName, material->fParams.fMapFile);
            }
            else
               strcpy(baseName, material->fParams.fMapFile);

            if(curSkinDamage == 0 || (material->fParams.fFlags & TS::Material::TextureFlags) != 0)
               strcpy(material->fParams.fMapFile, baseName);
            else
            {
               // check if we have a damage texture
               sprintf(material->fParams.fMapFile, "_d%d_%s", curSkinDamage, baseName);
               ro = rm->find(material->fParams.fMapFile);
               if(!ro || !ro->resource)
               {
                  Resource<GFXBitmap> baseTexture = rm->load(baseName, true);
                  // build the damage texture

                  GFXBitmap *bmp = new GFXBitmap();
                  bmp->width = baseTexture->width;
                  bmp->height = baseTexture->height;
                  bmp->stride = baseTexture->stride;
                  bmp->bitDepth = baseTexture->bitDepth;
                  bmp->imageSize = baseTexture->imageSize;
                  bmp->pBitsBase = new BYTE[bmp->imageSize];
                  bmp->pBits = bmp->pBitsBase;
                  bmp->paletteIndex = baseTexture->paletteIndex;
                  bmp->pPalette = NULL;
                  bmp->attribute = baseTexture->attribute;
                  bmp->detailLevels = baseTexture->detailLevels;
                  memcpy(bmp->pBits, baseTexture->pBits, bmp->imageSize);
                  int i;
                  for(i = 0; i < BMA_MAX_MIPMAPS; i++)
                  {
                     if(!baseTexture->pMipBits[i])
                        bmp->pMipBits[i] = NULL;
                     else
                        bmp->pMipBits[i] = baseTexture->pMipBits[i] - baseTexture->pBits + bmp->pBits;
                  }
                  damageBitmap(bmp);

                  rm->add(ResourceType::typeof(material->fParams.fMapFile), material->fParams.fMapFile, (void *) bmp);
               }

            }
         }
      }
      rm->add(ResourceType::typeof(fileName), fileName, (void *) newML);
   }
   materialList = rm->load(fileName, true);
   materialList->load(*rm, true);
}

int ShapeBase::getDamageSkinLevel(int detailLevel)
{
   if(detailLevel == 1 && getDamageLevel() > 0.40)
      return 2;
   else if(detailLevel == 2)
   {
      if(getDamageLevel() > 0.75)
         return 3;
      else if(getDamageLevel() > 0.50)
         return 2;
      else if(getDamageLevel() > 0.25)
         return 1;
   }
   return 0;
}

void ShapeBase::reSkin(bool force)
{
   if(!manager || !image.shape)
      return;

   const char *newSkinBase = getSkinBase();

   int dlevel = getDamageSkinLevel(prefDamageSkinDetail);
   if(!force && dlevel == curSkinDamage && !stricmp(curSkin, newSkinBase))
      return;
   
   curSkinDamage = dlevel;
   curSkin = newSkinBase;
   curSkinDamage  = dlevel;

   if(curSkinDamage == 0 && !stricmp(curSkin, "base"))
   {
      image.shape->setMaterialList((TS::MaterialList *) image.shape->getShape().getMaterialList());
      materialList = 0;
      return;
   }

   buildMaterialList();
   image.shape->setMaterialList(materialList);
}

bool ShapeBase::initResources(GameBase::GameBaseData* dat)
{
   if(!Parent::initResources(dat))
      return false;

	setRot(rotation);
	setPos(lPosition);

	ResourceManager *rm = SimResource::get(manager);
	data = dynamic_cast<ShapeBaseData *>(dat);

   delete shieldImage.shape;
   shieldImage.shape = NULL;
	energy = data->maxEnergy;
	
	if (isGhost() && bool(data->shieldShape)) {
	   shieldImage.shape = new TSShapeInstance(data->shieldShape, *rm);
		if (shieldImage.shape->getShape().lookupName ("Sequence01") != -1)
		{
			shieldThread = shieldImage.shape->CreateThread();
			shieldThread->setTimeScale( 1.0f );
			shieldThread->SetSequence ("Sequence01");
			shieldThread->SetPosition (1.0);
		}
	}

	delete image.shape;
   image.shape = new TSShapeInstance( data->shape, *rm );

   // only skin on the client:
   if(isGhost())
      reSkin();

	image.shape->UpdateSequenceSubscriberLists();

	SimContainer* root = findObject(manager,SimRootContainerId,root);
	root->addObject(this);
	image.root = root;

	for (int i = 0; i < MaxScriptThreads; i++) {
		SThread& st = scriptThread[i];
		if (st.sequence != -1)
			setSequence(i,st.sequence,false);
	}

	movementStepSize = image.shape->getShape().fRadius * 0.45;
	return true;
}

void ShapeBase::onRemove()
{
   Parent::onRemove();
	for (int i = 0; i < MaxScriptThreads; i++)
		stopSound(scriptThread[i]);
}


//----------------------------------------------------------------------------

bool ShapeBase::getCommandStatus(CommandStatus* status)
{
	// Damage level is set to 0, 1 or 2
	float damage = damageLevel / data->maxDamage;
	status->damageLevel = (damage > 0.7)? 2: (damage > 0.3)? 1: 0;
	status->active = true;
	return true;
}


//----------------------------------------------------------------------------

TS::ShapeInstance::Thread *ShapeBase::createThread (int seqNum)
{
	const TSShape &ts_shape = getTSShape();
	if (ts_shape.fSequences.size()) {
		TS::ShapeInstance::Thread *newThread = image.shape->CreateThread();
		newThread->setTimeScale( 1.0f );
		newThread->SetSequence( seqNum );
		return (newThread);
	}
	return 0;
}

void ShapeBase::animateRoot()
{
//	image.shape->animateRoot();
//	if (shieldImage.shape)
//		shieldImage.shape->animateRoot();
}

void ShapeBase::updateEnergy(SimTime t)
{
   energy += rechargeRate * t;
   if (energy > data->maxEnergy)
      energy = data->maxEnergy;
   else if (energy < 0)
      energy = 0;
}

void ShapeBase::updateDamage(SimTime t)
{
	if (repairReserve) {
		float rate = data->repairRate * t;
		if (repairReserve < rate) {
			damageLevel -= repairReserve;
			repairReserve = 0;
		}
		else {
			damageLevel -= rate;
			repairReserve -= rate;
		}
		if (damageLevel < 0) {
			damageLevel = 0;
			repairReserve = 0;
		}
  		setMaskBits(DamageMask);
	}
	if (damageLevel > 0 && autoRepairRate > 0)  {
		if ((damageLevel -= autoRepairRate * 0.032) < 0)
			damageLevel = 0;
  		setMaskBits(DamageMask);
	}
}

float ShapeBase::getEnergyLevel (void)
{
   float returnVal =1.0 - (energy / data->maxEnergy);
   if(returnVal > 1)
      returnVal = 1;
   if(returnVal < 0)
      returnVal = 0;
	return returnVal;
}

void ShapeBase::updateShieldThread (SimTime t)
{
	if (shieldThread)
		shieldThread->AdvanceTime (t);
}

const Point3F& ShapeBase::getAnimTransform()
{
	return image.shape->fRootDeltaTransform.p;
}

void ShapeBase::clearAnimTransform()
{
	image.shape->fRootDeltaTransform.identity();
}


void ShapeBase::setShieldThread (float p)
{
	if (shieldThread)
		shieldThread->SetPosition (p);
		
	Point3F up,r0,r2;
	shieldVec.normalize();
	(shieldVec.x == 0 && shieldVec.y == 0)? up.set (1, 0, 0) : up.set (0, 0, 1);	// "up.set"? hahaha...
	m_cross(shieldVec, up, &r0);
	r0.normalize();
	m_cross(shieldVec, r0, &r2);
	r2.normalize();

	shieldImage.transform.identity();
	shieldImage.transform.setRow (0, r0);
	shieldImage.transform.setRow (1, shieldVec);
	shieldImage.transform.setRow (2, r2);
	shieldImage.transform.p = getBoxCenter();
	shieldImage.transform.p.z += zOffset;

	const Box3F& box = getBoundingBox();
	float ll = (max(box.len_x(),box.len_y()) * 0.5) * 0.70;
	shieldImage.transform.p += shieldVec * ll;

	if (data->sfxShield != -1)
		TSFX::PlayAt(data->sfxShield, getTransform(), Point3F(0, 0, 0));
}

void ShapeBase::setShieldVec(Point3F hv, float zoff)
{
	shieldVec.x = -hv.x;
	shieldVec.y = -hv.y;
	shieldVec.z = -hv.z;
	shieldVec.normalize();
	zOffset = zoff;
	setMaskBits (ShieldMask);
}

void ShapeBase::setDamageLevel (float level)
{
	if (level < 0)
		level = 0;
	else
		if (level >= data->maxDamage)
			level = data->maxDamage;
	if (level != damageLevel) {
		damageLevel = level;
		setMaskBits (DamageMask);
	}
}

bool ShapeBase::repairDamage(float amount)
{
	if (damageLevel != 0 && amount > 0)
		repairReserve += amount;
	return true;
}

void ShapeBase::applyDamage(int type,float value,const Point3F pos,
	const Point3F& vec,const Point3F& mom,int objectId)
{
	if (!isGhost())
		if (const char* script = scriptName("onDamage"))
			Console->evaluatef("%s(%s,%d,%g,\"%g %g %g\",\"%g %g %g\",\"%g %g %g\",%d);",
				script,scriptThis(),type,value,
				pos.x,pos.y,pos.z,
				vec.x,vec.y,vec.z,
				mom.x,mom.y,mom.z,
				objectId);
}

const char* ShapeBase::getDamageState()
{
	return "Enabled";
}

void ShapeBase::setRechargeRate(float rate)
{
	rechargeRate = rate;
	setMaskBits (InfoMask);
}

void ShapeBase::setAutoRepairRate(float rate)
{
	autoRepairRate = rate;
}

void ShapeBase::setEnergy(float ee)
{
	if (ee < 0)
		energy = 0;
	else
		if (ee > data->maxEnergy)
			energy = data->maxEnergy;
		else
			energy = ee;
}

//----------------------------------------------------------------------------

const TMat3F& ShapeBase::getLOSTransform()
{
	static TMat3F mat;
	mat = getTransform();
	mat.p = getBoxCenter();
	return mat;
}

SimObject* ShapeBase::findLOSObject(const TMat3F& mat,float range, int mask, Point3F* pos,Point3F* normal)
{
	SimContainerQuery query;
	query.id = getId();
	query.type = -1;
	query.mask = mask;
	query.detail = SimContainerQuery::DefaultDetail;

	query.box.fMin = mat.p;
	Point3F vec(0,range,0);
	m_mul(vec,mat,&query.box.fMax);

	SimCollisionInfo info;
	SimContainer *root = findObject(manager, SimRootContainerId, root);
	root->findLOS(query, &info);
	if (info.surfaces.size()) {
		m_mul(info.surfaces[0].position,info.surfaces.tWorld,pos);
		m_mul(info.surfaces[0].normal,(RMat3F&)info.surfaces.tWorld,normal);
	}
	else
		*pos->set(0,0,0);
	return info.object;
}


//----------------------------------------------------------------------------

bool ShapeBase::aimedTransform (TMat3F *eyeMat, Point3F startPoint)
{
	Point3F p;
	Point3F n;

	if (findLOSObject (*eyeMat, 1200, -1, &p, &n))
		{
			Point3F forward = p - startPoint;
			if (forward.lenf() >= 2.0)
				{
					forward.normalize ();
					Point3F right;
					m_cross (Point3F (0, 0, 1), forward, &right);
					right.normalize ();
					Point3F up;
					m_cross (right, forward, &up);
					up.normalize ();
					eyeMat->setRow (0, right);
					eyeMat->setRow (1, forward);
					eyeMat->setRow (2, up);
					return true;
				}
	   }

	return false;
}

bool ShapeBase::getAimedMuzzleTransform (int slot, TMat3F *mat)
{
	TMat3F temp;
	if (!getMuzzleTransform (slot, &temp))
		return false;

	m_mul (temp, getTransform(), mat);
	Point3F start = mat->p;
	temp = getEyeTransform ();
	if (aimedTransform (&temp, start))
		{
			*mat = temp;
			mat->p = start;
		}

	return true;
}

//----------------------------------------------------------------------------

bool ShapeBase::setSequence(int slot, int seq,bool reset)
{
	SThread& st = scriptThread[slot];
	if (st.thread && st.sequence == seq && st.state == SThread::Play)
		return true;

	if (seq < MaxSequenceIndex) {
		setMaskBits(ThreadMask | (ThreadNMask << slot));
		st.sequence = seq;
		if (reset) {
			st.state = SThread::Play;
			st.atEnd = false;
			st.forward = true;
		}
		if (image.shape) {
			if (!st.thread)
				st.thread = image.shape->CreateThread();
			st.thread->SetSequence(seq);
			stopSound(st);
			updateSequence(st);
		}
		return true;
	}
	return false;
}

void ShapeBase::updateSequence(SThread& st)
{
   static Random s_randGen;

   float randScale = 1.0f + (2.0f * s_randGen.getFloat() * 0.1f) - 0.1f;

	switch (st.state) {
		case SThread::Stop:
			st.thread->setTimeScale(1);
			st.thread->SetPosition(0);
			// Drop through to pause state
		case SThread::Pause:
			st.thread->setTimeScale(0);
			stopSound(st);
			break;
		case SThread::Play:
			if (st.atEnd) {
				st.thread->setTimeScale(1);
				st.thread->SetPosition(st.forward? 1: 0);
				st.thread->setTimeScale(0);
				stopSound(st);
			}
			else {
				st.thread->setTimeScale(st.forward? randScale: -randScale);
				if (!st.sound)
					startSound(st);
			}
			break;
	}
}	

bool ShapeBase::stopSequence(int slot)
{
	SThread& st = scriptThread[slot];
	if (st.sequence != -1 && st.state != SThread::Stop) {
		setMaskBits(ThreadMask | (ThreadNMask << slot));
		st.state = SThread::Stop;
		updateSequence(st);
		return true;
	}
	return false;
}

bool ShapeBase::pauseSequence(int slot)
{
	SThread& st = scriptThread[slot];
	if (st.sequence != -1 && st.state != SThread::Pause) {
		setMaskBits(ThreadMask | (ThreadNMask << slot));
		st.state = SThread::Pause;
		updateSequence(st);
		return true;
	}
	return false;
}

bool ShapeBase::playSequence(int slot)
{
	SThread& st = scriptThread[slot];
	if (st.sequence != -1 && st.state != SThread::Play) {
		setMaskBits(ThreadMask | (ThreadNMask << slot));
		st.state = SThread::Play;
		updateSequence(st);
		return true;
	}
	return false;
}

bool ShapeBase::setSequenceDir(int slot,bool forward)
{
	SThread& st = scriptThread[slot];
	if (st.sequence != -1) {
		if (st.forward != forward) {
			setMaskBits(ThreadMask | (ThreadNMask << slot));
			st.forward = forward;
			st.atEnd = false;
			updateSequence(st);
		}
		return true;
	}
	return false;
}	


//----------------------------------------------------------------------------

void ShapeBase::stopSound(SThread& thread)
{
	if (thread.sound) {
		Sfx::Manager::Stop(manager, thread.sound);
		thread.sound = 0;
	}
}

void ShapeBase::startSound(SThread& thread)
{
	if (!isGhost() || !thread.thread)
		return;
	stopSound(thread);

	// Find the sound in the data file.
	const char* sname = 
		getTSShape().fNames[thread.thread->getSequence().fName];
	int sfxTag = -1;
	for (int i = 0; i < MaxSequenceIndex; i++)
		if (data->sequenceSound[i].sequence &&
				!stricmp(sname,data->sequenceSound[i].sequence)) {
			sfxTag = data->sequenceSound[i].sound;
			break;
		}

	if (sfxTag != -1)
		thread.sound = TSFX::PlayAt(sfxTag, getTransform(), Point3F(0, 0, 0));
}


//----------------------------------------------------------------------------

void ShapeBase::updateMovement(float slice)
{
	setTimeSlice(slice);

	// New position
	TMat3F tmat;
	Parent::updateMovement(&tmat);

	// New rotation
	RMat3F rmat;
	rmat.set(EulerF(rotation.x,rotation.y,rotation.z));
	(RMat3F&)tmat = rmat;

	if (!Parent::stepPosition(tmat,movementStepSize * 0.3f,movementStepSize)) {
		// New position failed, force rotation
		setRotation(rmat,true);
	}
}

void ShapeBase::updateScriptThreads(SimTime t)
{
	for (int i = 0; i < MaxScriptThreads; i++) {
		SThread& st = scriptThread[i];
		if (st.thread) {
			if (!st.thread->getSequence().fCyclic && !st.atEnd &&
					(st.forward? st.thread->getPosition() >= 1.0:
						st.thread->getPosition() <= 0)) {
				st.atEnd = true;
				updateSequence(st);
				if (!isGhost()) {
					if (image.shape)
						image.shape->animate();
					if (const char* script = scriptName("onEndSequence")) {
						char slot[16];
						sprintf(slot,"%d",i);
						Console->executef(3,script,scriptThis(),slot);
					}
				}
			}
			st.thread->AdvanceTime(t);
		}
	}
}


//----------------------------------------------------------------------------

void ShapeBase::setPos(const Point3F &pos)
{
	TMat3F mat;
	mat.set(EulerF(rotation.x,rotation.y,rotation.z),pos);
	SimMovement::setTransform(mat);
	setMaskBits(PositionMask);
	clearRestFlag();
}

void ShapeBase::setRot(const Point3F &rot)
{
	// No immediate update of rotation at the moment.
	// Will get updated when the object moves
	rotation = rot;
	setMaskBits(RotationMask);
}


//----------------------------------------------------------------------------

float ShapeBase::getRadius ()
{
	const TSShape &ts_shape = ((TSShapeInstance *)(image.shape))->getShape();
	return ts_shape.fRadius;
}

const Point3F &ShapeBase::getCenter ()
{
	const TSShape &ts_shape = ((TSShapeInstance *)(image.shape))->getShape();
	return ts_shape.fCenter;
}

const TSShape &ShapeBase::getTSShape ()
{
	return ((TSShapeInstance *)(image.shape))->getShape();
}

//----------------------------------------------------------------------------


bool ShapeBase::processEvent(const SimEvent* event)
{
	switch (event->type) {
		onEvent (SimObjectTransformEvent);
      case SoundEventType:
      {
         SoundEvent *evt = (SoundEvent *) event;
         Sfx::Manager *man = Sfx::Manager::find(manager);
         if(man)
         {
            if(soundChannel[evt->channel])
            {
               if(man->selectHandle(soundChannel[evt->channel]))
               {
                  man->stop();
                  man->release();
               }
            }
            soundChannel[evt->channel] = TSFX::PlayAt(evt->id, getTransform(), getLinearVelocity());
         }
         return true;
      }
     default:
      return Parent::processEvent(event);
	}
}

bool ShapeBase::onSimObjectTransformEvent(const SimObjectTransformEvent *event)
{
	EulerF rot;
	event->tmat.angles (&rot);
	
	setRot (Point3F (rot.x, rot.y, rot.z));
	setPos (event->tmat.p);
   return true;
}


//----------------------------------------------------------------------------

bool ShapeBase::processQuery(SimQuery* query)
{
	switch (query->type){
		onQuery (SimObjectTransformQuery);
		onQuery (SimRenderQueryImage);
		onQuery (SimImageTransformQuery);

     default:
      return Parent::processQuery(query);
	}
}

bool ShapeBase::onSimObjectTransformQuery (SimObjectTransformQuery* query)
{
   query->tmat = getTransform();
	return true;
}

bool ShapeBase::onSimImageTransformQuery (SimImageTransformQuery* query)
{
   query->transform = getTransform();
	return true;
}

bool ShapeBase::onSimRenderQueryImage (SimRenderQueryImage* query)
{
	TS::Camera* camera = query->renderContext->getCamera();

#if 0
	// See if the shape is inside the viewcone
	SphereF sphere;
	sphere.radius = image.shape->getShape().fRadius;
	m_mul(image.shape->getShape().fCenter,getTransform(),&sphere.center);
	if (camera->testVisibility(sphere) == TS::ClipNoneVis)
		return false;
#endif

	//
   image.setContainerBox(getBoundingBox());
	image.castShadow = Planet::shadows && 
		prefShadowDetailMask & data->shadowDetailMask;
	image.lightDirection = Planet::shadowDirection;
	image.shadowDetailScale = prefShadowDetailScale;

#if 0
	// Old shadow detailing which should now be handled by the
	// new shadow code.
	if (image.castShadow) {
	   camera->pushTransform( getTransform() );
		float adjustedSize = image.shape->getDetailScale() * camera->getPixelScale() *
			camera->transformProjectRadius(image.shape->getShape().fCenter,
				image.shape->getShape().fRadius );
	   camera->popTransform();
		image.castShadow = adjustedSize > prefDefaultShadowSize;
	}
#endif

   if (m_fading != NotFading &&
       (query->renderContext->getSurface()->getCaps() & GFX_DEVCAP_SUPPORTS_CONST_ALPHA) != 0) {
      float factor;
      if (m_fading == FadingOut) {
         factor = float(wg->currentTime - m_fadeStarted) / float(csm_fadeLength);
         // Bail if we're _really faded...
         if (factor > 1.0)
            factor = 1.0;
      } else {
         factor = 1.0f - float(wg->currentTime - m_fadeStarted) / float(csm_fadeLength);
         // Bail if we're _really faded...
         if (factor < 0.0) {
            factor = 0.0;
            m_fading = NotFading;
         }
      }

      if (factor != 0.0) {
         image.shape->setAlphaAlways(true, 1.0f - factor);
         image.alphaLevel = 1.0 * (1.0f - factor);
         image.itype = SimRenderImage::Translucent;
		   image.setSortValue(camera);
      } else {
		   image.setImageType();
		   image.setSortValue(camera);
	   }
   } else {
      image.shape->setAlphaAlways(false);
		image.setImageType();
		image.setSortValue(camera);
	}

   // update sun _once_ for shapebase.  If derived classes, i.e., vehicles, players,
   //  etc. want to do this more frequently, they have to do it themselves...
   //
   if (m_lastSunUpdated == 0) {
      float sunAttenuation;
      ColorF positionalColor;
      bool overrideSun = getPositionalColor(positionalColor, sunAttenuation);
      image.updateSunOverride(overrideSun, sunAttenuation, positionalColor);
      m_lastSunUpdated = wg->currentTime;
   }

	// Update shape & image data
   image.transform = getTransform();

   // Set our haze value...
   SimContainer* cnt = getContainer();
   if (cnt)
      image.hazeValue = cnt->getHazeValue(*query->renderContext,
                                          image.transform.p);
   if(image.hazeValue > 0.2)
      image.castShadow = false;

	// Return the image
	if (!blowedUpGood) {
//	   image.setImageType();
//	   image.setSortValue(camera); // does nothing if not translucent
		query->count = 1;
		query->image[0] = &image;
      image.pOwner = this;
	}
	else
		query->count = 0;
	
	// Return shield if it's active
	if (shieldThread && shieldThread->getPosition() < 1.0) {
	   const TMat3F & twc = camera->getTWC();
		float cd = twc.p.y + 
			shieldImage.transform.p.x * twc.m[0][1] +
			shieldImage.transform.p.y * twc.m[1][1] +
			shieldImage.transform.p.z * twc.m[2][1];
	   float nd = camera->getNearDist();
	   shieldImage.sortValue = (cd<=nd) ? 0.99f : nd/cd;
		query->image[query->count] = &shieldImage;
		query->count++;
	}

	if (image.castShadow && manager)
		image.preRenderShadow (*(query->renderContext), manager->getCurrentTime());

	return true;
}


//----------------------------------------------------------------------------

DebrisPartInfo BuildingDebrisChooser [] =
{
   {
      1,		   // debris id
      150.0f,  // est. mass for this type
      25.0f,   // max radius for this type
      1.0f     // probability of selection
   },
};

void ShapeBase::blowUp()
{
	if (data->explosionId != -1) {
		Explosion* explosion = new Explosion(data->explosionId);
		explosion->setPosition(getBoxCenter());
      Point3F myAxis;
      getTransform().getRow(2, &myAxis);
      explosion->setAxis(myAxis);
		manager->addObject(explosion);
	}
	// Assumes that if there is hulk shape only if there
	// is a debris data block and a debris detail level.
	if (data->debrisId != -1) {
	   // select detail level -- always detail 0 for now
	   TSShapeInstance * shape = (TSShapeInstance *)image.shape;
	   int dl = shape->getShape().findDetail("debris0");
		if (dl != -1)
		   shape->setDetailLevel(dl);
		else {
		   shape->setDetailLevel(0);
			blowedUpGood = true;
		}
	   shape->animate();

	   // find center of shape
	   Point3F tcenter,center;
	   m_mul(shape->getShape().fCenter,shape->fRootDeltaTransform,&tcenter);
	   m_mul(tcenter,getTransform(),&center);

		PartDebrisCloud * cloud = new PartDebrisCloud
			(shape, getTransform(), center, data->debrisId,
		  getLinearVelocity(), 2000, Point3F(0,0,0), 10000);
	   manager->addObject(cloud);
	}
	else
		blowedUpGood = true;
}

void ShapeBase::unBlowUp()
{
	blowedUpGood = false;
	image.shape->resetActiveFlag();
}

	
//----------------------------------------------------------------------------

DWORD ShapeBase::packUpdate(Net::GhostManager *gm, DWORD mask, BitStream *stream)
{
   Parent::packUpdate(gm, mask, stream);

	if (mask & InitialUpdate) {
		// No Shield on intial update
		mask &= ~ShieldMask;
		// Skip all the animation stuff if there is nothing running
		mask &= ~ThreadMask;
		for (int i = 0; i < MaxScriptThreads; i++)
			if (scriptThread[i].sequence != -1) {
				mask |= ThreadMask;
				break;
			}
	}

   if (stream->writeFlag(mask & ShieldMask))
		{
			stream->writeNormalVector(&shieldVec, 7);
			float zOff = zOffset;
			if (zOff > 5)
				zOff = 5;
			if (zOff < -5)
				zOff = -5;
			zOff =  (zOff + 5) / 10;
			stream->writeFloat (zOff, 8);
		}

	if (stream->writeFlag(mask & ThreadMask)) {
		for (int i = 0; i < MaxScriptThreads; i++) {
			SThread& st = scriptThread[i];
			if (stream->writeFlag(st.sequence != -1 && (mask & (ThreadNMask << i)))) {
				stream->writeInt(st.sequence,ThreadSequenceBits);
				stream->writeInt(st.state,2);
				stream->writeFlag(st.forward);
				stream->writeFlag(st.atEnd);
			}
		}
	}

   if (m_fading == FadingIn) {
      // Before we write this bit to the net work, we need to decide if we should stop fading in.
      if (m_fadeStarted + csm_fadeLength <= wg->currentTime)
         m_fading = NotFading;
   }
   if (stream->writeFlag(m_fading != NotFading) != false) {
      stream->writeFlag(m_fading == FadingIn);
   }

   return 0;
}

void ShapeBase::unpackUpdate(Net::GhostManager *gm, BitStream *stream)
{
   Parent::unpackUpdate(gm, stream);

   if (stream->readFlag()) {
		stream->readNormalVector (&shieldVec, 7);
		zOffset = (stream->readFloat (8) * 10) - 5;
		setShieldThread (0);
	}
	if (stream->readFlag())
		for (int i = 0; i < MaxScriptThreads; i++)
			if (stream->readFlag()) {
				SThread& st = scriptThread[i];
				int seq = stream->readInt(ThreadSequenceBits);
				st.state = stream->readInt(2);
				st.forward = stream->readFlag();
				st.atEnd = stream->readFlag();
				if (st.sequence != seq)
					setSequence(i,seq,false);
				else
					updateSequence(st);
			}

   if (stream->readFlag() == true) {
      if (stream->readFlag() == false)
         startFadeOut();
      else
         startFadeIn();
   } else {
      m_fading = NotFading;
   }
}

//----------------------------------------------------------------------------

void ShapeBase::inspectWrite(Inspect* inspector)
{
   Parent::inspectWrite(inspector);

	inspector->write(IDITG_SSP_INDESTRUCTIBLE, !destroyable);
   inspector->write(IDITG_POSITION, getPos() );
   inspector->write(IDITG_ROTATION, getRot() );
}

void ShapeBase::inspectRead(Inspect* inspector)
{
   Parent::inspectRead(inspector);

   Point3F pos;
   Point3F rot;
	bool indestructible;
	inspector->read(IDITG_SSP_INDESTRUCTIBLE, indestructible);
   inspector->read(IDITG_POSITION, pos);
   inspector->read(IDITG_ROTATION, rot);
	destroyable = !indestructible;
	setRot (rot);
   setPos (pos);
}


//---------------------------------------------------------------------------

void ShapeBase::initPersistFields()
{
   Parent::initPersistFields();
   addField("position", TypePoint3F, Offset(lPosition, ShapeBase));
   addField("rotation", TypePoint3F, Offset(rotation, ShapeBase));
   addField("destroyable", TypeBool, Offset(destroyable, ShapeBase));
}

//---------------------------------------------------------------------------

Persistent::Base::Error ShapeBase::read(StreamIO &sio, int, int)
{
	int version;
	sio.read (&version);

	Parent::read (sio, 0, 0);

	if (version > 0)
		sio.read (&destroyable);

	Point3F pos,rot;
	sio.read (sizeof (pos), &pos);
	sio.read (sizeof (rot), &rot);
	setRot(rot);
	setPos(pos);

	return (sio.getStatus() == STRM_OK)? Ok: ReadError;
}


Persistent::Base::Error ShapeBase::write(StreamIO &sio, int, int)
{
	int version = 1;
	sio.write (version);

	if (version > 0)
		Parent::write (sio, 0, 0);

	sio.write (destroyable);
	sio.write (sizeof (Point3F), &getPos());
	sio.write (sizeof (Point3F), &getRot());

	return (sio.getStatus() == STRM_OK)? Ok: WriteError;
}


//------------------------------------------------------------------------------
// Render Image
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------

void ShapeBase::RenderImage::renderBoundingBox(TSRenderContext& rc)
{
   TS::PointArray* pArray = rc.getPointArray();
   Point3F bboxPts[8];
   
   // Draw container bbox
   bboxPts[0].set(m_containerBox.fMin.x, m_containerBox.fMin.y, m_containerBox.fMin.z);
   bboxPts[1].set(m_containerBox.fMin.x, m_containerBox.fMax.y, m_containerBox.fMin.z);
   bboxPts[2].set(m_containerBox.fMin.x, m_containerBox.fMax.y, m_containerBox.fMax.z);
   bboxPts[3].set(m_containerBox.fMin.x, m_containerBox.fMin.y, m_containerBox.fMax.z);
   bboxPts[4].set(m_containerBox.fMax.x, m_containerBox.fMin.y, m_containerBox.fMin.z);
   bboxPts[5].set(m_containerBox.fMax.x, m_containerBox.fMax.y, m_containerBox.fMin.z);
   bboxPts[6].set(m_containerBox.fMax.x, m_containerBox.fMax.y, m_containerBox.fMax.z);
   bboxPts[7].set(m_containerBox.fMax.x, m_containerBox.fMin.y, m_containerBox.fMax.z);
   
   pArray->reset();
   int start = pArray->addPoints(8, bboxPts);
   
   const int color = 253;
   pArray->drawLine(start + 0, start + 1, color);
   pArray->drawLine(start + 1, start + 2, color);
   pArray->drawLine(start + 2, start + 3, color);
   pArray->drawLine(start + 3, start + 0, color);
   pArray->drawLine(start + 4, start + 5, color);
   pArray->drawLine(start + 5, start + 6, color);
   pArray->drawLine(start + 6, start + 7, color);
   pArray->drawLine(start + 7, start + 4, color);
   pArray->drawLine(start + 0, start + 4, color);
   pArray->drawLine(start + 1, start + 5, color);
   pArray->drawLine(start + 2, start + 6, color);
   pArray->drawLine(start + 3, start + 7, color);

   if (renderLightRays == true) {
      Point3F pPosition = pOwner->getBoxCenter();
      Point3F vector;
      pOwner->getTransform().getRow(2, &vector);
      vector.neg();

      float dotX = m_dot(Point3F(pOwner->getBoundingBox().len_x() * 0.5f, 0, 0), vector);
      float dotY = m_dot(Point3F(0, pOwner->getBoundingBox().len_y() * 0.5f, 0), vector);
      float dotZ = m_dot(Point3F(0, 0, pOwner->getBoundingBox().len_z() * 0.5f), vector);

      float useDot = dotX;
      if (fabs(dotY) > fabs(useDot))
         useDot = dotY;
      if (fabs(dotZ) > fabs(useDot))
         useDot = dotZ;

      pPosition -= vector * fabs(useDot);
      Point3F end = pPosition + (vector * (20.0f + fabs(useDot)));

      start = pArray->addPoint(pPosition);
      pArray->addPoint(end);
      pArray->drawLine(start + 0, start + 1, 255);
   }
}


//----------------------------------------------------------------------------

void ShapeBase::RenderImage::render(TSRenderContext& rc)
{
   ColorF oldColor;
   ColorF oldAmbientColor;
   int    sunIndex = -1;

   TS::SceneLighting* pLighting = rc.getLights();
   if (m_attenuateSun == true) {
      // Note that this assumes that the sunlight is the only directional light...
      for (int i = 0; i < pLighting->size(); i++) {
         TS::Light* rLight = (*pLighting)[i];

         if ((rLight->fLight.fType == TS::Light::LightDirectional ||
              rLight->fLight.fType == TS::Light::LightDirectionalWrap) &&
             (rLight->fLight.fRed   != 0.0 ||
              rLight->fLight.fGreen != 0.0 ||
              rLight->fLight.fBlue  != 0.0)) {
            oldColor.set(rLight->fLight.fRed,
                         rLight->fLight.fGreen,
                         rLight->fLight.fBlue);

            ColorF newColor;
            newColor.interpolate(oldColor, m_attenuationColor, m_attenuationFactor);

            rLight->fLight.fRed   = newColor.red;
            rLight->fLight.fGreen = newColor.green;
            rLight->fLight.fBlue  = newColor.blue;

            sunIndex = i;
            break;
         }
      }

      // HACK for ambient light
      //
      float attenIntensity = 0.3 * m_attenuationColor.red   +
                             0.5 * m_attenuationColor.green +
                             0.2 * m_attenuationColor.blue;
      attenIntensity *= m_attenuationFactor;
      oldAmbientColor = pLighting->getAmbientIntensity();
      float newAtten = attenIntensity * 2.0f;
      if (newAtten > 1.0f)
         newAtten = 1.0f;
      ColorF newAmb = oldAmbientColor * newAtten;
      pLighting->setAmbientIntensity(newAmb);
   }

   shape->setPerspectiveScale(pOwner->data->isPerspective ? 1.0 : 0.0);

   TribesShadowRenderImage::render(rc);

   if (m_attenuateSun == true) {
      if (sunIndex != -1) {
         TS::Light* rLight = (*pLighting)[sunIndex];

         rLight->fLight.fRed   = oldColor.red;
         rLight->fLight.fGreen = oldColor.green;
         rLight->fLight.fBlue  = oldColor.blue;
      }
      pLighting->setAmbientIntensity(oldAmbientColor);
   }

   if (ShapeBase::renderBoundingBox)
		renderBoundingBox(rc);
}


//----------------------------------------------------------------------------

void ShapeBase::RenderImage::updateSunOverride(const bool    in_attenuate,
                                   const float   in_factor,
                                   const ColorF& in_color)
{
   m_attenuateSun      = in_attenuate;
   m_attenuationFactor = in_factor;
   m_attenuationColor  = in_color;
}

float
ShapeBase::coverage(Point3F eye)
{
   SimCollisionInfo  collisionInfo;
   SimContainerQuery query;
   query.id     = getId();
	query.type   = -1;
	query.mask   = -1;
	query.detail = SimContainerQuery::DefaultDetail;
   query.box.fMin = eye;
   query.box.fMax = getBoxCenter();
   
   if (!manager)
      return 1.0f;

   SimContainer* pRoot = (SimContainer*)manager->findObject(SimRootContainerId);
   AssertFatal(pRoot != NULL, "No root container found...");

   if (pRoot->findLOS(query, &collisionInfo))
      return 0.0f;
   else
      return 1.0f;
}

void
ShapeBase::updateSunRollAv()
{
   if (m_attenuateSun == true || m_ticks != 0) {
      m_attenuationColor  *= 0.5;
      m_attenuationColor  += m_rollColor * 0.5;
      m_attenuationFactor *= 0.5;
      m_attenuationFactor += m_rollFactor * 0.5;
      if (m_ticks != 0)
         m_ticks--;
   }
}

void
ShapeBase::updateSunOverride(const bool    in_overrideSun,
                             const float   in_sunAttenuation,
                             const ColorF& in_positionalColor)
{
   if (in_overrideSun == true) {
      if (m_attenuateSun == false && m_ticks == 0)
         m_attenuationColor = in_positionalColor;

      m_ticks = 0;

      m_rollFactor = in_sunAttenuation;
      m_rollColor  = in_positionalColor;
   } else {
      if (m_attenuateSun == true) {
         m_ticks = 9;
         m_rollFactor = 0;
      } else {
         if (m_ticks > 0)
            m_ticks--;
      }
   }
   m_attenuateSun = in_overrideSun;
   updateSunRollAv();

   if (m_attenuateSun || m_ticks > 0)
      image.updateSunOverride(m_attenuateSun, m_attenuationFactor, m_attenuationColor);
	else
      image.updateSunOverride(false, 0.0, ColorF(0, 0, 0));
}

bool
ShapeBase::getLOSColor(const Point3F& in_rShootPoint,
                       const Point3F& in_rVec,
                       float          in_dist,
                       ColorF&        out_rColor,
                       float&         out_rFactor,
                       SimContainer*& out_rpContainer)
{
   SimContainerQuery query;
   query.id        = getId();
   query.mask      = SimInteriorObjectType | SimTerrainObjectType;
   query.detail    = SimContainerQuery::DefaultDetail;
   query.box.fMin  = in_rShootPoint;
   query.box.fMax  = in_rShootPoint;
   query.box.fMax += in_rVec * in_dist;

   SimContainer* pRoot = findObject(manager, SimRootContainerId, pRoot);
   AssertFatal(pRoot != NULL, "No root container...");

   SimCollisionInfo cInfo;

   if (pRoot->findLOS(query, &cInfo, SimCollisionImageQuery::High)) {
      SimObject* collObject = cInfo.object;
      AssertFatal(collObject != NULL, "Collision wo/ object?  That doesn't make sense...");

      Point3F worldPos;
      m_mul(cInfo.surfaces[0].position, cInfo.surfaces.tWorld, &worldPos);

      float dist = fabs(m_dot((in_rShootPoint - worldPos), in_rVec));
      if (dist >= in_dist) {
         out_rFactor = 0.0f;
         return false;
      } else if (dist < 0.0f) {
         out_rFactor = 1.0f;
      } else {
         out_rFactor = (in_dist - dist) / in_dist;
      }

      if ((collObject->getType() & SimInteriorObjectType) != 0) {
         InteriorShape* pItrShape = dynamic_cast<InteriorShape*>(collObject);

         if (pItrShape != NULL) {
            if (pItrShape->isMissionLit() == true) {
               out_rpContainer = pItrShape;

               UInt16 lighting;
               if (out_rFactor != 0.0f &&
                   pItrShape->getInstance()->getSurfaceInfo(cInfo.surfaces[0], &lighting) == true) {

                  float red   = float((lighting >> 8) & 0xf) / 15.0f;
                  float green = float((lighting >> 4) & 0xf) / 15.0f;
                  float blue  = float((lighting >> 0) & 0xf) / 15.0f;

                  out_rColor.set(red, green, blue);
                  return true;
               } else {
                  out_rpContainer = NULL;
                  out_rFactor     = 0.0f;
                  return false;
               }
            }
         }
      } else if ((collObject->getType() & SimTerrainObjectType) != 0) {
         SimTerrain* pTerrain = dynamic_cast<SimTerrain*>(collObject);
         AssertFatal(pTerrain != NULL, "Terrain type returned for non-SimTerrain object?");
         out_rpContainer = pTerrain;

         if (out_rFactor != 0.0f) {
            UInt16 lighting;
            Point2F gridPos(cInfo.surfaces[0].position.x, cInfo.surfaces[0].position.y);
            GridFile* pGFile = pTerrain->getGridFile();
            pGFile->getLighting(gridPos, lighting);

            UInt8 r, g, b;
            b = (lighting >> 0) & 0xf;
            g = (lighting >> 4) & 0xf;
            r = (lighting >> 8) & 0xf;

            out_rColor.set(float(r) / 15.0f,
                           float(g) / 15.0f,
                           float(b) / 15.0f);

            return true;
         } else {
            return false;
         }
      } else {
         AssertFatal(0, "Invalid object type returned...");
      }

      out_rpContainer = NULL;
      out_rFactor = 0.0f;
      return false;
   }

   out_rpContainer = NULL;
   out_rFactor = 0.0f;
   return false;
}

bool
ShapeBase::getPositionalColor(ColorF& out_rColor,
                              float&  out_rSunAttenuation)
{
//   Point3F pPosition = getBoxCenter();

   if (g_lowDetailObjectLighting == true) {
      Point3F pPosition = getBoxCenter();
      Point3F vector;
      getTransform().getRow(2, &vector);
      vector.neg();

      float dotX = m_dot(Point3F(getBoundingBox().len_x() * 0.5f, 0, 0), vector);
      float dotY = m_dot(Point3F(0, getBoundingBox().len_y() * 0.5f, 0), vector);
      float dotZ = m_dot(Point3F(0, 0, getBoundingBox().len_z() * 0.5f), vector);

      float useDot = dotX;
      if (fabs(dotY) > fabs(useDot))
         useDot = dotY;
      if (fabs(dotZ) > fabs(useDot))
         useDot = dotZ;

      pPosition -= vector * fabs(useDot);

      SimContainer* pContainer;
      return getLOSColor(pPosition, vector,
                         20.0f + fabs(useDot), out_rColor, out_rSunAttenuation,
                         pContainer);
   } else {
//      struct Result {
//         enum Type {
//            Interior = 0,
//            Terrain  = 1
//         };
//         Type type;
//         float  factor;
//         ColorF color;
//         bool   override;
//      } shotResults[6];
//
//      static Point3F vecs[6];
//      static vecsInitialized = false;
//      if (vecsInitialized == false) {
//         vecs[0].set(0,  0, -1);
//         vecs[1].set(0,  0,  1);
//         vecs[2].set(0, -1,  0);
//         vecs[3].set(0,  1,  0);
//         vecs[4].set(-1, 0,  0);
//         vecs[5].set( 1, 0,  0);
//      }
//
//      int i;
//      for (i = 0; i < 6; i++) {
//         SimContainer* pContainer = NULL;
//
//         shotResults[i].override =
//            getLOSColor(pPosition + (vecs[i] * -0.5),
//                        vecs[i],
//                        20.0f, shotResults[i].color,
//                        shotResults[i].factor,
//                        pContainer);
//         if (shotResults[i].override == true) {
//            AssertFatal(pContainer != NULL, "Container not set on override?");
//
//            if ((pContainer->getType() & SimInteriorObjectType) != 0)
//               shotResults[i].type = Result::Interior;
//            else
//               shotResults[i].type = Result::Terrain;
//         }
//      }
//
//      //int interiorHits = 0;
//      //int terrainHits  = 0;
//      int totalHits    = 0;
//
//      float max0Factor = -1.0;
//      int   max0Index  = -1;
//      float max1Factor = -1.0;
//      int   max1Index  = -1;
//
//      for (i = 0; i < 6; i++) {
//         if (shotResults[i].override == true) {
//            totalHits++;
//            /*  Tinman - removed these (non-effectual) lines to remove compiler warnings
//            if (shotResults[i].type == Result::Interior)
//               interiorHits++;
//            if (shotResults[i].type == Result::Terrain)
//               terrainHits++;
//            */
//
//            if (shotResults[i].factor > max0Factor) {
//               max1Factor = max0Factor;
//               max1Index  = max0Index;
//
//               max0Factor = shotResults[i].factor;
//               max0Index  = i;
//            } else if (shotResults[i].factor > max1Factor) {
//               max1Factor = shotResults[i].factor;
//               max1Index  = i;
//            }
//         }
//      }
//
//      if (totalHits == 0) {
//         out_rColor.set(0, 0, 0);
//         out_rSunAttenuation = 0.0f;
//         return false;
//      } else if (totalHits == 1) {
//         out_rColor          = shotResults[max0Index].color;
//         out_rSunAttenuation = shotResults[max0Index].factor;
//      }
//
//      // Otherwise, interpolate between the two colors based on their factor, final factor
//      //  is second factor of the way between 1 and maxFactor.  May want to tweak this later
//      //  based on container type... HUGE HAIRY HACK!  (But it looks good... :)
//      //
//      float adjustFactor = shotResults[max0Index].factor + shotResults[max1Index].factor;
//      max0Factor /= adjustFactor;
//      max1Factor /= adjustFactor;
//
//      out_rColor = (shotResults[max0Index].color * max0Factor) +
//                   (shotResults[max1Index].color * max1Factor);
//      out_rSunAttenuation = shotResults[max0Index].factor +
//                            ((1.0f - shotResults[max0Index].factor) *
//                             shotResults[max1Index].factor);
      return true;
   }
}

bool
ShapeBase::onAdd()
{
   if (Parent::onAdd() == false)
      return false;

   return true;
}

void
ShapeBase::startFadeOut()
{
   if (isGhost() == false) {
      m_fading = FadingOut;
      m_fadeStarted = wg->currentTime;
      setMaskBits(SBFadeMask);
   } else {
      if (m_fading == NotFading) {
         m_fadeStarted = wg->currentTime;
      } else if (m_fading == FadingIn) {
         DWORD timeDiff = wg->currentTime - m_fadeStarted;
         if (timeDiff > csm_fadeLength) {
            m_fadeStarted = wg->currentTime;
         } else {
            timeDiff = csm_fadeLength - timeDiff;
            m_fadeStarted = wg->currentTime - timeDiff;
         }
      }
      m_fading = FadingOut;
   }
}

void
ShapeBase::startFadeIn()
{
   if (isGhost() == false) {
      m_fading      = FadingIn;
      m_fadeStarted = wg->currentTime;
      setMaskBits(SBFadeMask);
   } else {
      if (m_fading == FadingOut) {
         DWORD timeDiff = wg->currentTime - m_fadeStarted;
         if (timeDiff > csm_fadeLength) {
            m_fadeStarted = wg->currentTime;
         } else {
            timeDiff = csm_fadeLength - timeDiff;
            m_fadeStarted = wg->currentTime - timeDiff;
         }
      } else if (m_fading == NotFading) {
         m_fadeStarted = wg->currentTime;
      }
      m_fading = FadingIn;
   }
}

