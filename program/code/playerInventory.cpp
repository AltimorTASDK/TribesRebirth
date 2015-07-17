//----------------------------------------------------------------------------
// Script access
//		incInventory
//		decInventory
//		hideItem
//		mountItem
//		unmountItem

//----------------------------------------------------------------------------

#include "player.h"
#include <simResource.h>
#include <SimGame.h>
#include "dataBlockManager.h"
#include "fearGlobals.h"
#include "stringTable.h"
#include "projectile.h"
#include "projectileFactory.h"
#include "tsfx.h"
#include "PlayerManager.h"

//----------------------------------------------------------------------------

Player::ItemImageData::ItemImageData()
{
	shapeFile = NULL;
	firstPerson = true;
	mountPoint = BackpackMount;
	mountOffset.set(0,0,0);
	mountRotation.set(0,0,0);

	ammoType = -1;
	weaponType = -1;
	projectile.type = -1;
	projectile.dataType = -1;
	activateTime = 0.5;
	fireTime = 0.2;
	reloadTime = 1.0;
	spinUpTime = 2.0;
	spinDownTime = 2.0;
	minEnergy = 2;
	maxEnergy = 10;
	mass = 0;

	lightType = Player::NoLight;
	lightRadius = 3;
	lightTime = 1.5;
	lightColor.set(1,1,1);

	sfxActivateTag = -1;
	sfxFireTag = -1;
	sfxReadyTag = -1;
	sfxReloadTag = -1;
	sfxSpinUpTag = -1;
	sfxSpinDownTag = -1;
   activationSequence = -1;
   ambientSequence = -1;
   spinSequence = -1;
   fireSequence = -1;
   reloadSequence = -1;
   noammoSequence = -1;
   readySequence = -1;
}

bool Player::ItemImageData::preload(ResourceManager *rm, bool server, char errorBuffer[256])
{
   server;
   if(shapeFile[0])
   {
      char shapeName[256];
      strcpy(shapeName, shapeFile);
      strcat(shapeName, ".dts");
      shape = rm->load(shapeName);
      if(!bool(shape))
      {
         sprintf(errorBuffer, "Unable to load ItemImageData shape: %s", shapeName);
         return false;
      }
      TSShapeInstance *si = new TSShapeInstance(shape, *rm);
      TS::ShapeInstance::Thread *thread;
      thread = si->CreateThread();

      if(thread)
      {
         activationSequence = thread->GetSequenceIndex("activation");
         ambientSequence = thread->GetSequenceIndex("ambient");
         spinSequence = thread->GetSequenceIndex("spin");
         fireSequence = thread->GetSequenceIndex("fire");
         reloadSequence = thread->GetSequenceIndex("reload");
         noammoSequence = thread->GetSequenceIndex("noammo");
         readySequence = thread->GetSequenceIndex("ready");
         si->DestroyThread(thread);
      }
      delete si;
   }
   return true;
}

void Player::ItemImageData::pack(BitStream* stream)
{
	Parent::pack(stream);

   stream->writeString(shapeFile);
   stream->write(sizeof(projectile), &projectile);
	stream->write(int(mountPoint));
	stream->write(sizeof(mountOffset),&mountOffset);
	stream->write(sizeof(mountRotation),&mountRotation);
	stream->writeFlag(firstPerson);

	stream->write(weaponType);
	stream->write(ammoType);
	stream->write(activateTime);
	stream->write(fireTime);
	stream->write(reloadTime);
	stream->write(spinUpTime);
	stream->write(spinDownTime);
	stream->write(mass);

	sg.dbm->writeBlockId(stream, DataBlockManager::SoundDataType,sfxActivateTag);
	sg.dbm->writeBlockId(stream, DataBlockManager::SoundDataType,sfxFireTag);
	sg.dbm->writeBlockId(stream, DataBlockManager::SoundDataType,sfxReadyTag);
	sg.dbm->writeBlockId(stream, DataBlockManager::SoundDataType,sfxReloadTag);
	sg.dbm->writeBlockId(stream, DataBlockManager::SoundDataType,sfxSpinUpTag);
	sg.dbm->writeBlockId(stream, DataBlockManager::SoundDataType,sfxSpinDownTag);

	stream->write(lightType);
	stream->write(lightRadius);
	stream->write(lightTime);
	stream->write(sizeof(lightColor),&lightColor);

   stream->write(minEnergy);
   stream->write(maxEnergy);
}

void Player::ItemImageData::unpack(BitStream* stream)
{
	Parent::unpack(stream);

	int mount;
   shapeFile = stream->readSTString();
   stream->read(sizeof(projectile), &projectile);
	stream->read(&mount); mountPoint = (MountPoint)mount;
	stream->read(sizeof(mountOffset),&mountOffset);
	stream->read(sizeof(mountRotation),&mountRotation);
	firstPerson = stream->readFlag();

	stream->read(&weaponType);
	stream->read(&ammoType);
	stream->read(&activateTime);
	stream->read(&fireTime);
	stream->read(&reloadTime);
	stream->read(&spinUpTime);
	stream->read(&spinDownTime);
	stream->read(&mass);

	sfxActivateTag = cg.dbm->readBlockId(stream, DataBlockManager::SoundDataType);
	sfxFireTag = cg.dbm->readBlockId(stream, DataBlockManager::SoundDataType);
	sfxReadyTag = cg.dbm->readBlockId(stream, DataBlockManager::SoundDataType);
	sfxReloadTag = cg.dbm->readBlockId(stream, DataBlockManager::SoundDataType);
	sfxSpinUpTag = cg.dbm->readBlockId(stream, DataBlockManager::SoundDataType);
	sfxSpinDownTag = cg.dbm->readBlockId(stream, DataBlockManager::SoundDataType);

	stream->read(&lightType);
	stream->read(&lightRadius);
	stream->read(&lightTime);
	stream->read(sizeof(lightColor),&lightColor);

   stream->read(&minEnergy);
   stream->read(&maxEnergy);
}


//----------------------------------------------------------------------------

Player::ItemTypeEntry::ItemTypeEntry()
{
	count = 0;
	imageSlot = -1;
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

int Player::getItemCount(int type)
{
	return (type >= 0 && type < MaxItemTypes)? itemTypeList[type].count: 0;
}

int Player::incItemCount(int type,int count)
{
	return setItemCount(type,itemTypeList[type].count + count);
}

int Player::decItemCount(int type,int count)
{
	return setItemCount(type,itemTypeList[type].count - count);
}

int Player::setItemCount(int type,int count)
{
	ItemTypeEntry& itemType = itemTypeList[type];
	itemType.count = (count < 0)? 0: count;

	if (!isGhost()) {
		if (!itemType.count && itemType.imageSlot >= 0)
			unmountItem(itemType.imageSlot);
	}
	return itemType.count;
}

int Player::getItemClassCount(const char* itemClass)
{
	int count = 0;
   itemClass = stringTable.insert(itemClass);
	for (int i = 0; i < MaxItemTypes; i++)
		if (itemTypeList[i].count) {
			Item::ItemData& itemData = *getItemData(i);
			if (itemData.className == itemClass)
				count++;
		}
	return count;
}

void Player::resetItemCounts()
{
	for (int i = 0; i < MaxItemTypes; i++) {
		itemTypeList[i].count = 0;
		itemTypeList[i].imageSlot = -1;
	}
}	


//----------------------------------------------------------------------------

bool Player::useItem(int type)
{
	if (!isGhost() && itemTypeList[type].count)
		scriptOnUseItem(type);
	return true;
}

bool Player::dropItem(int type)
{
	if (!isGhost() && itemTypeList[type].count)
		scriptOnDropItem(type);
	return true;
}

bool Player::deployItem(int type)
{
	if (!isGhost() && itemTypeList[type].count) {
		// Check deployment area in front of player
		Point3F pos(0,0,0);
		scriptOnDeployItem(type,pos);
	}
	return true;
}


//----------------------------------------------------------------------------
// Any item with an item image is selectable

bool Player::mountItem(int type,int imageSlot, int team)
{
	Item::ItemData& itemData = *getItemData(type);
	if (itemData.imageId != -1) {
		ItemImageEntry& itemImage = itemImageList[imageSlot];

		if (itemImage.typeId != -1) {
			if (itemImage.typeId == type) {
				// Item already selected
				itemImage.nextId = -2;
				return true;
			}
#if 0
			// No selection if no ammo
			ItemImageData& imageData = *getItemImageData(itemData.imageId);
			if (imageData.ammoType != -1 && !getItemCount(imageData.ammoType))
				return false;
#endif
			if (!isGhost())
				scriptOnUnmountItem(itemImage.typeId);
		}
		//
		itemTypeList[type].imageSlot = imageSlot;
		setMaskBits(VisItemMask | (1 << (ItemImageMaskBits + imageSlot)));
		setImageType(imageSlot,type,team);

		//
		if (!isGhost())
			scriptOnMountItem(type);
		return true;
	}
	return false;
}

bool Player::unmountItem(int imageSlot)
{
	ItemImageEntry& itemImage = itemImageList[imageSlot];
	if (itemImage.typeId != -1) {
		int type = itemImage.typeId;
		itemTypeList[itemImage.typeId].imageSlot = -1;
		//
		setMaskBits(VisItemMask | (1 << (ItemImageMaskBits + imageSlot)));
		setImageType(imageSlot,-1);

		if (!isGhost())
			scriptOnUnmountItem(type);
		return true;
	}
	return false;
}

int Player::getMountedItem(int imageSlot)
{
	return itemImageList[imageSlot].typeId;
}

int Player::getNextMountedItem(int imageSlot)
{
	return itemImageList[imageSlot].nextId;
}

bool Player::isItemFiring(int imageSlot)
{
	return (itemImageList[imageSlot].state == ItemImageEntry::Fire);
}

bool Player::isItemMounted(int type)
{
	return itemTypeList[type].imageSlot != -1;
}

int Player::getItemTeam(int imageSlot)
{
	ItemImageEntry& itemImage = itemImageList[imageSlot];
	if (itemImage.typeId != -1)
		return itemImage.teamId;
	return -1;
}

//----------------------------------------------------------------------------

const char* Player::scriptItemName(int type,const char* callback)
{
	static char name[256];

	// Dat file name 
	sprintf(name,"%s::%s",getItemName(type),callback);
	if (Console->isFunction(name))
		return name;

	// ClassName specified in the dat file
	Item::ItemData* itemData = getItemData(type);
	if (itemData->className && itemData->className[0] != '\0') {
		sprintf(name,"%s::%s",itemData->className,callback);
		if (Console->isFunction(name))
			return name;
	}

	// Generic item
	sprintf(name,"item::%s",callback);
	if (Console->isFunction(name))
		return name;
	return 0;
}

const char* Player::scriptItemThis(int type)
{
	return wg->dbm->lookupBlockName(type,DataBlockManager::ItemDataType);
}

const char* Player::scriptThis()
{
	static char name[10];
	sprintf(name,"%d",getId());
	return name;
}

//----------------------------------------------------------------------------

void Player::scriptOnUseItem(int type)
{
	if (const char* script = scriptItemName(type,"onUse"))
		Console->executef(3,script,scriptThis(),scriptItemThis(type));
}

void Player::scriptOnDropItem(int type)
{
	if (const char* script = scriptItemName(type,"onDrop"))
		Console->executef(3,script,scriptThis(),scriptItemThis(type));
}

void Player::scriptOnDeployItem(int type, const Point3F& pos)
{
	if (const char* script = scriptItemName(type,"onDeploy")) {
		char buff[256];
		sprintf(buff,"%f,%f,%f",pos.x,pos.y,pos.z);
		Console->executef(4,script,scriptThis(),scriptItemThis(type),buff);
	}
}

void Player::scriptOnMountItem(int type)
{
	if (const char* script = scriptItemName(type,"onMount"))
		Console->executef(3,script,scriptThis(),scriptItemThis(type));
}

void Player::scriptOnUnmountItem(int type)
{
	if (const char* script = scriptItemName(type,"onUnmount"))
		Console->executef(3,script,scriptThis(),scriptItemThis(type));
}


//----------------------------------------------------------------------------

void Player::packItemImages(BitStream *stream, DWORD mask)
{
	if (stream->writeFlag(mask & VisItemMask))
		for (int i = 0; i < MaxItemImages; i++)
			if (stream->writeFlag(mask & (1 << ItemImageMaskBits + i))) {
				ItemImageEntry& itemImage = itemImageList[i];
				stream->writeInt(itemImage.typeId+1,ItemTypeBits);
				if (stream->writeFlag(itemImage.teamId != -1))
					stream->writeInt(itemImage.teamId,3);
				stream->writeFlag(itemImage.triggerDown);
				stream->writeFlag(itemImage.ammo);
				stream->writeInt(itemImage.fireCount,3);
				if (mask & InitialUpdate)
					stream->writeFlag(itemImage.state == ItemImageEntry::Fire);
			}
}

void Player::unpackItemImages(BitStream *stream)
{
	if (stream->readFlag())
		for (int i = 0; i < MaxItemImages; i++)
			if (stream->readFlag()) {
				ItemImageEntry& itemImage = itemImageList[i];
				int type = stream->readInt(ItemTypeBits) - 1;
				int team = -1;
				if (stream->readFlag())
					team = stream->readInt(3);
				if (itemImage.typeId != type)
					setImageType(i,type,team);
				itemImage.triggerDown = stream->readFlag();
				itemImage.ammo = stream->readFlag();
				int count = stream->readInt(3);

				if (manager) {
					if (count != itemImage.fireCount) {
						itemImage.fireCount = count;
						setImageState(i,ItemImageEntry::Fire);
					}
					updateImageState(i,0);
				}
				else {
					itemImage.fireCount = count;
					if (stream->readFlag())
						itemImage.state = ItemImageEntry::Fire;
				}
			}
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

Item::ItemData* Player::getItemData(int typeId)
{
	return static_cast<Item::ItemData*>
		(wg->dbm->lookupDataBlock(typeId,DataBlockManager::ItemDataType));
}

Player::ItemImageData* Player::getItemImageData(int imageId)
{
	return static_cast<ItemImageData*>
		(wg->dbm->lookupDataBlock(imageId,DataBlockManager::ItemImageDataType));
}

const char* Player::getItemName(int typeId)
{
	// Names are only valid on the server.
	if (sg.dbm)
		return sg.dbm->lookupBlockName(typeId,DataBlockManager::ItemDataType);
	return "";
}

const char* Player::getImageName(int typeId)
{
	// Names are only valid on the server.
	if (sg.dbm)
		return sg.dbm->lookupBlockName(typeId,DataBlockManager::ItemImageDataType);
	return "";
}

int Player::getItemType(const char* name)
{
	// The name can either be actual name of the data block or
	// it can be it's index number (typeId).
	int typeId = -1;
	while (isspace(*name))
		name++;
	if (isdigit(name[0])) {
		typeId = atoi(name);
		if (typeId < 0 || typeId >= wg->dbm->getDataGroupSize(DataBlockManager::ItemDataType))
			typeId = -1;
	}
	else {
		// The data lookup expects the name to be in the stringTable.
		// Names are only valid on the server.
		if (sg.dbm) {
			const char* str = stringTable.insert(name);
			sg.dbm->lookupDataBlock(str,DataBlockManager::ItemDataType,typeId);
		}
	}
	return typeId;
}


//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

bool Player::getMountTransform(MountPoint mountPoint,TMat3F* mat)
{
	// Returns mount point in world space
	//int node = image.shape->getShape().findNode(NodeMountName[mountPoint]);
	int node = mountNode[mountPoint];
   
	if (node != -1) {
		const TMat3F& nmat = image.shape->getTransform(node);
		if (mountPoint == PrimaryMount || mountPoint == SecondaryMount)
		   mat->set(EulerF(viewPitch, 0, 0), nmat.p);
		else
			m_mul(nmat,getTransform(),mat);
		return true;
	}
	*mat = getTransform();
	return true;
}

void Player::calcImagePullIn()
{
   gunPullIn = 0;
   for(int imageSlot = 0; imageSlot < MaxItemImages; imageSlot++)
   {
	   ItemImageEntry& itemImage = itemImageList[imageSlot];
	   if (itemImage.imageId == -1)
         continue;
	   ItemImageData& imageData = *getItemImageData(itemImage.imageId);
      if (imageData.mountPoint != PrimaryMount)
         continue;
      TMat3F imageTransform, muzzleTransform;
      if(!getImageTransform(imageSlot, &imageTransform) || !getMuzzleTransform(imageSlot, &muzzleTransform))
         return;
      // run a los between imageTransform.p and muzzleTransform.p
   
	   SimContainerQuery cq;
	   cq.id = getId();
	   cq.type = -1;
	   cq.mask = -1 & (~CorpseObjectType);
      m_mul(imageTransform.p, getTransform(), &cq.box.fMin);
      m_mul(muzzleTransform.p, getTransform(), &cq.box.fMax);
      Point3F ovec = cq.box.fMax - cq.box.fMin;
      Point3F nvec = -cq.box.fMin;

      SimCollisionInfo info;
	   SimContainer* root = findObject(manager,SimRootContainerId,(SimContainer*)0);

      if(!root->findLOS(cq,&info))
         return;
   
	   // Should be only one surface.
	   CollisionSurfaceList& surfaceList = info.surfaces;
      Point3F endpos;
   	m_mul(surfaceList[0].position,surfaceList.tWorld,&endpos);

      nvec += endpos;

      Point3F ffdv;
      getTransform().getRow(1, &ffdv);
      gunPullIn = m_dot(nvec, ffdv) - m_dot(ovec, ffdv) - 0.2;

      return;
   }
}

bool Player::getImageTransform(int imageSlot,TMat3F* mat)
{
	// Image transform in object space
	ItemImageEntry& itemImage = itemImageList[imageSlot];
	ItemImageData& imageData = *getItemImageData(itemImage.imageId);

   Point3F offset = imageData.mountOffset;
   if(imageData.mountPoint == PrimaryMount)
      offset.y += gunPullIn;

	TMat3F off(EulerF(imageData.mountRotation.x,
		imageData.mountRotation.y,imageData.mountRotation.z),
		offset);

	int node = mountNode[imageData.mountPoint];
	//int node = image.shape->getShape().findNode(NodeMountName[mountPoint]);
	if (node != -1) {
		const TMat3F& nmat = image.shape->getTransform(node);
		if (!dead && (imageData.mountPoint == PrimaryMount || 
				imageData.mountPoint == SecondaryMount) &&
				(image.shape->getOverride() || aiControlled) ) {
			TMat3F imat;
		   imat.set(EulerF(viewPitch, 0, 0),nmat.p);
			m_mul(off,imat,mat);
		}
		else
			m_mul(off,nmat,mat);
		return true;
	}
	mat->identity();
	return false;
}

bool Player::getMuzzleTransform(int imageSlot,TMat3F* mat)
{
	// Muzzle transform in object space
	ItemImageEntry& itemImage = itemImageList[imageSlot];
	if (itemImage.typeId != -1) {
		int node = itemImage.muzzleNode;
		if(node != -1) {
			TMat3F imat;
			getImageTransform(imageSlot,&imat);
			const TMat3F& mmat = itemImage.image.shape->getTransform(node);
			m_mul(mmat,imat,mat);
		}
		else
			getImageTransform(imageSlot,mat);
		return true;
	}
	return false;
}


//----------------------------------------------------------------------------
// Item Image
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------

Player::ItemImageEntry::ItemImageEntry()
{
	image.shape = 0;
	state = Reset;
	typeId = -1;
	imageId = -1;
	nextId = -2;
	teamId = -1;
	nextTeam = -1;
	animSound = 0;
	spinSound = 0;
	delayTime = 0;
	ammo = false;
	triggerDown = false;
	fireCount = 0;
	lightStart = 0;
   pProjectile = NULL;
	muzzleNode = -1;
	lightStart = 0;
	light.fLight.fType = TSLight::LightInvalid;
}

Player::ItemImageEntry::~ItemImageEntry()
{
	delete image.shape;
}


//----------------------------------------------------------------------------

const char* Player::scriptImageName(int imageSlot,const char* callback)
{
	static char name[256];
	if (itemImageList[imageSlot].imageId == -1)
		return 0;
	ItemImageEntry& itemImage = itemImageList[imageSlot];
	Item::ItemData& itemData = *getItemData(itemImage.typeId);

	// Based off the dat file
	sprintf(name,"%s::%s",getImageName(itemImage.imageId),callback);
	if (Console->isFunction(name))
		return name;

	// Classname specified in the dat file
	if (itemData.className && itemData.className[0]) {
		sprintf(name,"%s::%s",itemData.className,callback);
		if (Console->isFunction(name))
			return name;
	}

	// Based off the default (C++ class name)
	sprintf(name,"ItemImage::%s",callback);
	if (Console->isFunction(name))
		return name;

	return 0;
}


//----------------------------------------------------------------------------

void Player::updateImageMass()
{
	if (!data)
		return;

	float imass = 0;
	for (int i = 0; i < MaxItemImages; i++) {
		ItemImageEntry& itemImage = itemImageList[i];
		if (itemImage.imageId != -1) {
			ItemImageData& imageData = *getItemImageData(itemImage.imageId);
			imass += imageData.mass;
		}
	}
	setMass(data->mass + imass);
}


//----------------------------------------------------------------------------

void Player::setImageType(int imageSlot, int typeId, int team)
{
	ItemImageEntry& itemImage = itemImageList[imageSlot];
	if (itemImage.typeId == typeId) {
		itemImage.nextId = -2;
		return;
	}

	// Delay item changes untill these states are through
	if (!isGhost()) {
		if (typeId != -1 && 
				(itemImage.state == ItemImageEntry::Fire || 
				itemImage.state == ItemImageEntry::Reload)) {
			itemImage.nextId = typeId;
			itemImage.nextTeam = team;
			return;
		}
	}

	// No new type, just unselecting the current item
	if (typeId == -1) {
		resetImageSlot(imageSlot);
		return;
	}

	// Init new shape
	Item::ItemData& itemData = *getItemData(typeId);
	ItemImageData& imageData = *getItemImageData(itemData.imageId);
	resetImageSlot(imageSlot);
	itemImage.typeId = typeId;
	itemImage.imageId = itemData.imageId;
	itemImage.teamId = team;

	itemImage.shapeRes = imageData.shape;

	if (!bool(itemImage.shapeRes)) {
		itemImage.imageId = -1;
		return;
	}

	itemImage.image.shape = new TSShapeInstance(itemImage.shapeRes,
		*wg->resManager );

	// The server needs the shape loaded for muzzle mount nodes
	// but it doesn't need to run any of the animations.
	itemImage.ambientThread = 0;
	itemImage.animThread = 0;
	itemImage.spinThread = 0;
   itemImage.materialList = 0;

	itemImage.muzzleNode = itemImage.image.shape->getShape().findNode("dummyalways muzzle");

	if (isGhost()) {
      // build the material list on the item image
      if(itemImage.teamId != -1)
      {
         const char *skinBase = cg.playerManager->getTeamSkinBase(itemImage.teamId);
         if(strcmp(skinBase, "base"))
         {
            char name[256];
            sprintf(name, "%s_0_%s.dml", imageData.shapeFile, skinBase);
            ResourceObject *ro = wg->resManager->find(name);

            if(!ro || !ro->resource)
            {
               TS::MaterialList *newML = new TS::MaterialList();
               *newML = *(itemImage.image.shape->getShape().getMaterialList());
               int cnt = newML->getMaterialsCount();

               for(int i = 0; i < cnt; i++)
               {
                  TS::Material *material = &newML->getMaterial(i);
                  if ((material->fParams.fFlags & TS::Material::MatFlags) == TS::Material::MatTexture)
                  {
                     static char baseName[64];
                     if(!strncmp("base.", material->fParams.fMapFile, 5))
                     {
                        // test if we have the base material at all
                        sprintf(baseName, "%s.%s", skinBase, material->fParams.fMapFile + 5);
                        if(wg->resManager->findFile(baseName))
                           strcpy(material->fParams.fMapFile, baseName);
                     }
                  }
               }
               wg->resManager->add(ResourceType::typeof(name), name, (void *) newML);
            }
            itemImage.materialList = wg->resManager->load(name, true);
            itemImage.materialList->load(*wg->resManager, true);
            itemImage.image.shape->setMaterialList(itemImage.materialList);
         }
      }
		if (imageData.activationSequence != -1) {
			itemImage.animThread = itemImage.image.shape->CreateThread();
			itemImage.animThread->setTimeScale( 1.0f );
			itemImage.animThread->SetSequence( imageData.activationSequence, 0.0 );
		}

		if (imageData.ambientSequence != -1) {
			itemImage.ambientThread = itemImage.image.shape->CreateThread();
			itemImage.ambientThread->setTimeScale( 1.0f );
			itemImage.ambientThread->SetSequence( imageData.ambientSequence, 0.0 );
		}
		if (imageData.spinSequence != -1) {
			itemImage.spinThread = itemImage.image.shape->CreateThread();
			itemImage.spinThread->setTimeScale( 0 );
			itemImage.spinThread->SetSequence( imageData.spinSequence, 0.0 );
		}
	}

	if (manager) {
		setImageState(imageSlot,ItemImageEntry::Activate);
		updateImageMass();
	}
}

void Player::resetImageSlot(int imageSlot)
{
	// Clear out current image
	ItemImageEntry& itemImage = itemImageList[imageSlot];
	if (itemImage.imageId != -1) {
		ItemImageData& imageData = *getItemImageData(itemImage.imageId);
		if (imageData.weaponType == Sustained &&
				itemImage.state == ItemImageEntry::Fire)
			stopImageFire(imageSlot);
	}
	delete itemImage.image.shape;
	itemImage.image.shape = 0;
   itemImage.materialList = 0;
	itemImage.shapeRes.unlock();
	if (itemImage.animSound) {
		Sfx::Manager::Stop(manager, itemImage.animSound);
		itemImage.animSound = 0;
	}
	if (itemImage.spinSound) {
		Sfx::Manager::Stop(manager, itemImage.spinSound);
		itemImage.spinSound = 0;
	}
	itemImage.typeId = -1;
	itemImage.nextId = -2;
	itemImage.imageId = -1;
	itemImage.teamId = -1;
	itemImage.nextTeam = -1;
	itemImage.state = ItemImageEntry::Reset;
	itemImage.delayTime = 0;
	itemImage.ammo = false;
	itemImage.triggerDown = false;
	itemImage.fireCount = 0;
	itemImage.lightStart = 0;
	itemImage.light.fLight.fType = TSLight::LightInvalid;
	updateImageMass();
}


//----------------------------------------------------------------------------

SimRenderImage* Player::getImage(int imageSlot,TSRenderContext* rc)
{
	ItemImageEntry& itemImage = itemImageList[imageSlot];
	if (itemImage.imageId == -1)
		return 0;
	ItemImageData& imageData = *getItemImageData(itemImage.imageId);

	// Update visiblity thread
	if (isFirstPersonView()) {
		if (!imageData.firstPerson)
			return 0;
		itemImage.image.itype = SimRenderImage::Translucent;
		itemImage.image.sortValue = 0.999f;
	}
	else {
	   itemImage.image.itype = SimRenderImage::Translucent;

	   Point3F pCam;
	   TS::Camera* camera = rc->getCamera();
	   float nd = camera->getNearDist();
	   m_mul(itemImage.image.transform.p, camera->getTWC(), &pCam);
	   itemImage.image.sortValue = (pCam.y <= nd) ? 0.99f : nd / pCam.y;
	}

	// Transform
	TMat3F mat;
	getImageTransform(imageSlot,&mat);
	m_mul(mat,getTransform(),&itemImage.image.transform);

	// Use the player's shadow lighting
	itemImage.image.updateSunOverride(m_attenuateSun,
		m_attenuationFactor,m_attenuationColor);

	return &itemImage.image;
}


//----------------------------------------------------------------------------

bool Player::isImageTriggerDown(int imageSlot)
{
	if (isGhost() || itemImageList[imageSlot].imageId == -1)
		return false;
	return itemImageList[imageSlot].triggerDown;
}

void Player::setImageTriggerDown(int imageSlot)
{
	if (isGhost() || itemImageList[imageSlot].imageId == -1)
		return;
	ItemImageEntry& itemImage = itemImageList[imageSlot];
	ItemImageData& imageData = *getItemImageData(itemImage.imageId);

	if (!itemImage.triggerDown && imageData.weaponType != -1) {
		itemImage.triggerDown = true;
		if (!isGhost()) {
			setMaskBits(VisItemMask | (1 << (ItemImageMaskBits + imageSlot)));
			// If the server is already in the fire state, let's
			// make sure the client is in as well.
			if (itemImage.state == ItemImageEntry::Fire)
				if (imageData.weaponType == Spinning ||
						imageData.weaponType == Sustained)
					itemImage.fireCount = (itemImage.fireCount + 1) & 0x7;
			updateImageState(imageSlot,0);
		}
	}
}

void Player::setImageTriggerUp(int imageSlot)
{
	if (isGhost() || itemImageList[imageSlot].imageId == -1)
		return;
	ItemImageEntry& itemImage = itemImageList[imageSlot];

	if (itemImage.triggerDown) {
		itemImage.triggerDown = false;
		if (!isGhost()) {
			setMaskBits(VisItemMask | (1 << (ItemImageMaskBits + imageSlot)));
			updateImageState(imageSlot,0);
		}
	}
}


//----------------------------------------------------------------------------

void Player::setImageState(int imageSlot, ItemImageEntry::State state)
{
	if (itemImageList[imageSlot].imageId == -1)
		return;
	ItemImageEntry& itemImage = itemImageList[imageSlot];
	ItemImageData& imageData = *getItemImageData(itemImage.imageId);
	float lastDelay = itemImage.delayTime;
	int lastState = itemImage.state;
	itemImage.state = state;

	// Stop any looping sounds or animations use in the last state.
	if (itemImage.animSound && itemImage.animLoopingSound) {
		Sfx::Manager::Stop(manager, itemImage.animSound);
		itemImage.animSound = 0;
	}
	if (itemImage.animThread && itemImage.animThread->getSequence().fCyclic)  {
		itemImage.animThread->setTimeScale(1.0);
		while (itemImage.animThread->getPosition() != 0)
			itemImage.animThread->SetPosition(0);
		itemImage.animThread->setTimeScale(0);
	}

	//
	if (lastState == ItemImageEntry::Fire && imageData.weaponType == Sustained)
		stopImageFire(imageSlot);

	//
	if (itemImage.nextId >= -1 && (lastState == ItemImageEntry::Fire ||
			lastState == ItemImageEntry::Reload)) {
		setImageType(imageSlot,itemImage.nextId,itemImage.nextTeam);
		return;
	}

	switch (itemImage.state) {
		case ItemImageEntry::Fire: {
			itemImage.delayTime = imageData.fireTime;
			if (!isGhost()) {
				setMaskBits(VisItemMask | (1 << (ItemImageMaskBits + imageSlot)));
				itemImage.fireCount = (itemImage.fireCount + 1) & 0x7;
			}
			if (imageData.weaponType == Sustained)
				startImageFire(imageSlot);
			else
				fireImageProjectile(imageSlot);
			setImageSound(imageSlot,imageData.sfxFireTag);
			if (itemImage.animThread && imageData.fireSequence != -1) {
				itemImage.animThread->setTimeScale( 1.0f );
				itemImage.animThread->SetSequence( imageData.fireSequence, 0.0 );
			}
			if (itemImage.spinThread)
				itemImage.spinThread->setTimeScale(1.0f);
			break;
		}
		case ItemImageEntry::Reload: {
			if (imageData.ammoType != -1 && !itemImage.ammo) {
				setImageState(imageSlot,ItemImageEntry::NoAmmo);
				break;
			}
			setImageSound(imageSlot,imageData.sfxReloadTag);
			itemImage.delayTime = imageData.reloadTime;
			if (itemImage.spinThread)
				itemImage.spinThread->setTimeScale(0);
			if (itemImage.animThread && imageData.reloadSequence != -1) {
				itemImage.animThread->setTimeScale( 1.0f );
				itemImage.animThread->SetSequence( imageData.reloadSequence, 0.0 );
			}
			if (!imageData.reloadTime) {
				setImageState(imageSlot,ItemImageEntry::Ready);
				break;
			}
			break;
		}
		case ItemImageEntry::NoAmmo: {
			if (imageData.ammoType != -1)
				scriptOnNoAmmo(imageSlot);
			if (itemImage.spinThread)
				itemImage.spinThread->setTimeScale(0);
			if (itemImage.animThread && imageData.noammoSequence != -1) {
				itemImage.animThread->setTimeScale( 1.0f );
				itemImage.animThread->SetSequence( imageData.noammoSequence, 0.0 );
			}
			break;
		}
		case ItemImageEntry::Idle: {
			if (itemImage.spinThread)
				itemImage.spinThread->setTimeScale(0);
			break;
		}
		case ItemImageEntry::Ready: {
			if (!isGhost() && itemImage.triggerDown) {
				setImageState(imageSlot,ItemImageEntry::Fire);
				break;
			}
			setImageSound(imageSlot,imageData.sfxReadyTag);
			if (itemImage.spinThread)
				itemImage.spinThread->setTimeScale(1.0);
			if (itemImage.animThread && imageData.readySequence != -1) {
				itemImage.animThread->setTimeScale( 1.0f );
				itemImage.animThread->SetSequence( imageData.readySequence, 0.0 );
			}
			break;
		}
		case ItemImageEntry::SpinUp: {
			setImageSound(imageSlot,imageData.sfxSpinUpTag);
			if (lastState == ItemImageEntry::SpinDown) {
				float scale = 1.0f - (lastDelay / imageData.spinDownTime);
				itemImage.delayTime = imageData.spinUpTime * scale;
			}
			else
				itemImage.delayTime = imageData.spinUpTime;
			break;
		}
		case ItemImageEntry::SpinDown: {
			setImageSound(imageSlot,imageData.sfxSpinDownTag);
			if (lastState == ItemImageEntry::SpinUp) {
				float scale = 1.0f - (lastDelay / imageData.spinUpTime);
				itemImage.delayTime = imageData.spinDownTime * scale;
			}
			else
				itemImage.delayTime = imageData.spinDownTime;
			break;
		}
		case ItemImageEntry::Activate: {
			itemImage.delayTime = imageData.activateTime;
			setImageSound(imageSlot,imageData.sfxActivateTag);
			if (itemImage.animThread && imageData.activationSequence != -1) {
				itemImage.animThread->setTimeScale( 1.0f );
				itemImage.animThread->SetSequence( imageData.activationSequence, 0.0 );
			}
			break;
		}
		case ItemImageEntry::Deactivate: {
			break;
		}
	}
}

void Player::updateImageAnimation(int imageSlot, float dt)
{
	if (itemImageList[imageSlot].imageId == -1)
		return;
	ItemImageEntry& itemImage = itemImageList[imageSlot];

	// Animation threads
	if (itemImage.ambientThread)
		itemImage.ambientThread->AdvanceTime(dt);
	if (itemImage.animThread)
		itemImage.animThread->AdvanceTime(dt);
	if (itemImage.spinThread)
		itemImage.spinThread->AdvanceTime(dt);

	// Sound position
	if (itemImage.animSound) {
		Sfx::Manager *man = Sfx::Manager::find(manager);
		if(man)
		{
   		man->selectHandle(itemImage.animSound);
   		man->setTransform(getTransform(), Point3F(0, 0, 0));
      }
	}
}

void Player::updateImageState(int imageSlot,float dt)
{
	if (itemImageList[imageSlot].imageId == -1)
		return;
	ItemImageEntry& itemImage = itemImageList[imageSlot];
	ItemImageData& imageData = *getItemImageData(itemImage.imageId);
	itemImage.delayTime -= dt;

	if (!isGhost()) {
		bool ammo;
		if (imageData.ammoType == -1)
			ammo = energy > imageData.minEnergy;
		else
			ammo = getItemCount(imageData.ammoType) > 0;
		if (ammo != itemImage.ammo) {
			setMaskBits(VisItemMask | (1 << (ItemImageMaskBits + imageSlot)));
			itemImage.ammo = ammo;
		}
	}

	// Image states
	switch (itemImage.state) {
		case ItemImageEntry::Fire: {
			if (imageData.weaponType == Sustained) {
			   updateImageFire(imageSlot);
			   if (imageData.ammoType == -1) {
				   if ((energy -= imageData.maxEnergy * dt) < 0)
				   	energy = 0;
					else
						// Just in case someone puts in a negative 
						// energy value.
						if (energy > data->maxEnergy)
							energy = data->maxEnergy;
            }
         }
			if (itemImage.delayTime <= 0) {
				if (imageData.weaponType == Spinning) {
					if (!itemImage.triggerDown || !itemImage.ammo)
						setImageState(imageSlot,ItemImageEntry::SpinDown);
					else {
						itemImage.delayTime = imageData.fireTime;
						fireImageProjectile(imageSlot);
					}
				}
				else
					if (imageData.weaponType == Sustained) {
						if (!itemImage.triggerDown || !itemImage.ammo)
							setImageState(imageSlot,ItemImageEntry::Reload);
					}
					else
						setImageState(imageSlot,ItemImageEntry::Reload);
			}
			break;
		}
		case ItemImageEntry::Reload: {
			// Must wait the full time before switching states.
			if (itemImage.delayTime <= 0) {
				if (imageData.weaponType == DiscLauncher)
					setImageState(imageSlot,ItemImageEntry::SpinUp);
				else
					setImageState(imageSlot,ItemImageEntry::Ready);
			}
			break;
		}
		case ItemImageEntry::NoAmmo: {
			if (itemImage.ammo) {
				if (imageData.weaponType == Spinning) {
					if (itemImage.triggerDown)
						setImageState(imageSlot,ItemImageEntry::SpinUp);
					else
						setImageState(imageSlot,ItemImageEntry::Idle);
				}
				else
					setImageState(imageSlot,ItemImageEntry::Reload);
			}
			break;
		}
		case ItemImageEntry::Idle: {
			if (!itemImage.ammo)
				setImageState(imageSlot,ItemImageEntry::NoAmmo);
			else
				if (itemImage.triggerDown) {
					if (imageData.weaponType == Spinning)
						setImageState(imageSlot,ItemImageEntry::SpinUp);
					else
						setImageState(imageSlot,ItemImageEntry::Ready);
				}
			break;
		}
		case ItemImageEntry::Ready: {
			if (imageData.weaponType == Spinning) {
				if (!itemImage.triggerDown || !itemImage.ammo)
					setImageState(imageSlot,ItemImageEntry::SpinDown);
			}
			else {
				if (!isGhost() && itemImage.triggerDown)
					setImageState(imageSlot,ItemImageEntry::Fire);
			}
			break;
		}
		case ItemImageEntry::SpinUp: {
			if (imageData.weaponType == DiscLauncher) {
				if (!itemImage.ammo) {
					setImageState(imageSlot,ItemImageEntry::NoAmmo);
					break;
				}
			}
			else
				if (!itemImage.triggerDown || !itemImage.ammo) {
					setImageState(imageSlot,ItemImageEntry::SpinDown);
					break;
				}
			if (itemImage.delayTime <= 0)
				setImageState(imageSlot,ItemImageEntry::Ready);
			else {
				if (itemImage.spinThread) {
					float timeScale = 1.0f - itemImage.delayTime / imageData.spinUpTime;
					itemImage.spinThread->setTimeScale(timeScale);
				}
			}
			break;
		}
		case ItemImageEntry::SpinDown: {
			if (itemImage.triggerDown && itemImage.ammo)
				setImageState(imageSlot,ItemImageEntry::SpinUp);
			else {
				if (itemImage.delayTime <= 0)
					setImageState(imageSlot,ItemImageEntry::Idle);
				else {
					if (itemImage.spinThread) {
						float timeScale = itemImage.delayTime / imageData.spinDownTime;
						itemImage.spinThread->setTimeScale(timeScale);
					}
				}
			}
			break;
		}
		case ItemImageEntry::Reset:
		case ItemImageEntry::Activate: {
			if (itemImage.delayTime <= 0) {
				if (imageData.weaponType == Spinning)
					setImageState(imageSlot,ItemImageEntry::Idle);
				else
					if (!itemImage.ammo)
						setImageState(imageSlot,ItemImageEntry::NoAmmo);
					else
						if (imageData.weaponType == DiscLauncher)
							setImageState(imageSlot,ItemImageEntry::Reload);
						else
							setImageState(imageSlot,ItemImageEntry::Ready);
			}
		}
		case ItemImageEntry::Deactivate: {
			break;
		}
	}
}

void Player::setImageSound(int imageSlot,int soundTag)
{
	if (isGhost() && soundTag != -1) {
		ItemImageEntry& itemImage = itemImageList[imageSlot];
		itemImage.animSound = 
			TSFX::PlayAt(soundTag, getTransform(), Point3F(0, 0, 0));
		Sfx::Manager *man = Sfx::Manager::find(cg.manager);
		if (man) {
			man->selectHandle(itemImage.animSound);
			itemImage.animLoopingSound = man->isLooping();
		}
	}
}


//----------------------------------------------------------------------------

static char ImageSlotBuff[10];

void Player::scriptOnNoAmmo(int imageSlot)
{
	// This callback is executed on both the client and the server
	ItemImageEntry& itemImage = itemImageList[imageSlot];
	if (const char* script = scriptName("onNoAmmo")) {
		sprintf(ImageSlotBuff,"%d",imageSlot);
		Console->executef(4,script,scriptThis(),ImageSlotBuff,
			scriptItemThis(itemImage.typeId));
	}
}


//----------------------------------------------------------------------------

void Player::startImageFire(int imageSlot)
{
	ItemImageEntry& itemImage = itemImageList[imageSlot];
	ItemImageData& imageData = *getItemImageData(itemImage.imageId);

   if (isGhost() == false) {
	   if (imageData.projectile.type == -1) {
	   	if (const char* script = scriptImageName(imageSlot,"onActivate")) {
	   		sprintf(ImageSlotBuff,"%d",imageSlot);
	   		Console->executef(3,script,scriptThis(),ImageSlotBuff);
	   	}
      } else {
		   TMat3F mat,total;

			if (imageData.accuFire && !dead)
				getAimedMuzzleTransform (imageSlot, &total);
			else
				{
					getMuzzleTransform(imageSlot,&mat);
					m_mul(mat,getTransform(),&total);
				}

		   Projectile* pr = createProjectile(imageData.projectile);
		   pr->initProjectile(total,getLinearVelocity(),getId());

         if (pr->isTargetable() == true) {
            Point3F pos, norm;
            SimObject* pLOSObject = findLOSObject(getEyeTransform(), pr->getTargetRange(), -1, &pos, &norm);
            ShapeBase* pShape = dynamic_cast<ShapeBase*>(pLOSObject);

            pr->setTarget(pShape);
         }
         SimGroup *grp = NULL;
         if(SimObject *obj = manager->findObject("MissionCleanup"))
            grp = dynamic_cast<SimGroup*>(obj);
         if(!manager->registerObject(pr)) {
            AssertWarn(0, "unable to register projectile");
            delete pr;
         } else {
            if(grp)
               grp->addObject(pr);
            else
               manager->addObject(pr);
            itemImage.pProjectile = pr;
         }
      }
	}
}

void
Player::updateImageFire(int imageSlot)
{
	ItemImageEntry& itemImage = itemImageList[imageSlot];
	ItemImageData& imageData = *getItemImageData(itemImage.imageId);

   if (isGhost() == false) {
      if (imageData.projectile.type == -1) {
		   if (const char* script = scriptImageName(imageSlot,"onUpdateFire")) {
		   	sprintf(ImageSlotBuff,"%d",imageSlot);
		   	Console->executef(3,script,scriptThis(),ImageSlotBuff);
		   }
      } else if (itemImage.pProjectile != NULL) {
         AssertFatal(itemImage.pProjectile->isSustained() == true,
                     "Projectile isn't a sustained projectile");

		   TMat3F mat,total;

			if (imageData.accuFire && !dead)
				getAimedMuzzleTransform (imageSlot, &total);
			else
				{
					getMuzzleTransform(imageSlot,&mat);
					m_mul(mat,getTransform(),&total);
				}

         itemImage.pProjectile->updateImageTransform(total);
      }
   }
}

void Player::stopImageFire(int imageSlot)
{
	ItemImageEntry& itemImage = itemImageList[imageSlot];
	ItemImageData& imageData = *getItemImageData(itemImage.imageId);

   if (isGhost() == false) {
      if (imageData.projectile.type == -1) {
		   if (const char* script = scriptImageName(imageSlot,"onDeactivate")) {
		   	sprintf(ImageSlotBuff,"%d",imageSlot);
		   	Console->executef(3,script,scriptThis(),ImageSlotBuff);
		   }
      } else {
         AssertFatal(itemImage.pProjectile, "NULL projectile -Dave check this out!");
         if(itemImage.pProjectile) {
            AssertFatal(itemImage.pProjectile->isSustained() == true,
                        "Projectile isn't a sustained projectile");

            itemImage.pProjectile->shutOffProjectile();
            itemImage.pProjectile = NULL;
         }
      }
   }
}	

void Player::fireImageProjectile(int imageSlot)
{
	ItemImageEntry& itemImage = itemImageList[imageSlot];
	ItemImageData& imageData = *getItemImageData(itemImage.imageId);

	if (!isGhost()) {
		if (imageData.projectile.type == -1) {
			if (const char* script = scriptImageName(imageSlot,"onFire")) {
				sprintf(ImageSlotBuff,"%d",imageSlot);
				Console->executef(3,script,scriptThis(),ImageSlotBuff);
			}
		}
		else {
			TMat3F total, mat;

			if (imageData.accuFire && !dead)
				getAimedMuzzleTransform (imageSlot, &total);
			else
				{
					getMuzzleTransform(imageSlot,&mat);
					m_mul(mat,getTransform(),&total);
				}

			Projectile* pr = createProjectile(imageData.projectile);
			pr->initProjectile(total,getLinearVelocity(),getId());

         if (pr->isTargetable() == true) {
            Point3F pos, norm;
            SimObject* pLOSObject = findLOSObject(getEyeTransform(), pr->getTargetRange(), -1, &pos, &norm);
            ShapeBase* pShape = dynamic_cast<ShapeBase*>(pLOSObject);

            pr->setTarget(pShape);
         }

			if (imageData.ammoType != -1)
				decItemCount(imageData.ammoType);
			else {
				float pe;
            float pofm;
				if (imageData.maxEnergy > energy) {
					pe = energy;
               pofm = float(energy) / float(imageData.maxEnergy);
					energy = 0;
				}
				else {
					pe = imageData.maxEnergy;
               pofm = 1.0f;
					energy -= pe;
				}
				pr->setEnergy(pe, pofm);
			}

         SimGroup *grp = NULL;
         if(SimObject *obj = manager->findObject("MissionCleanup"))
            grp = dynamic_cast<SimGroup*>(obj);
         if(!manager->registerObject(pr))
            delete pr;
         else
         {
            if(grp)
               grp->addObject(pr);
            else
               manager->addObject(pr);
         }
		}
	}
	else {
		if (imageData.lightType == WeaponFireLight)
			itemImage.lightStart = manager->getCurrentTime();
	}
}


//----------------------------------------------------------------------------

TSLight* Player::getImageLight(int imageSlot)
{
	ItemImageEntry& itemImage = itemImageList[imageSlot];
	if (itemImage.imageId == -1)
		return 0;

	ItemImageData& imageData = *getItemImageData(itemImage.imageId);
	if (imageData.lightType == NoLight)
		return 0;

	float intensity;
	float delta = manager->getCurrentTime() - itemImage.lightStart;
	switch (imageData.lightType) {
		case ConstantLight:
			intensity = 1.0;
			break;
		case WeaponFireLight: {
			if (delta > imageData.lightTime)
				return 0;
			intensity = 1.0 - delta / imageData.lightTime;
			break;
		}
		case PulsingLight: {
			intensity = 0.5 + 0.5 * sin(M_PI * delta / imageData.lightTime);
			intensity = 0.15 + intensity * 0.85;
			break;
		}
		default:
			return 0;
	}

	itemImage.light.setType(TSLight::LightPoint);
	itemImage.light.setRange(imageData.lightRadius);
	itemImage.light.setIntensity(imageData.lightColor.x * intensity,
		imageData.lightColor.y * intensity,imageData.lightColor.z * intensity);

	// If there is no muzzle node on the shape getMuzzleTransform
	// returns the image origin.  So should work fine for either
	// fire or pulsing lights.
	TMat3F mat,total;
	getMuzzleTransform(imageSlot,&mat);
	m_mul(mat,getTransform(),&total);
	itemImage.light.setPosition(total.p);
	return &itemImage.light;
}
