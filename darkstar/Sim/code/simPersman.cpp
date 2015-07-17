//--------------------------------------------------------------------------- 


//--------------------------------------------------------------------------- 

#include <sim.h>

#pragma warn -inl
#pragma warn -aus

#include <volstrm.h>
#include <tvector.h>

#include "simCollision.h"
#include "simPersman.h"

//--------------------------------------------------------------------------- 

bool SimPersistManager::createTempVolume (void)
{
	char fname[145];
	
	GetTempPath (134, fname);
	strcat (fname, "fear.tmp");
	
//	GetTempFileName (NULL, "Fear", 0, fname);	// this doesn't work...
	
	if (tempVolume.createVolume (fname))
		return true;
	else
		AssertFatal(0, "Can't open temp volume.");
		
	return false;
}

SimPersistManager::SimPersistManager (SimObjectId _id)
{
	id = _id;

	next_vid = 1;
	
	createTempVolume ();
}

SimPersistManager::~SimPersistManager ()
{
	tempVolume.closeVolume ();

	for (int i = 0; i < 4; i++)
		if (baseVolume[i])
			{
				baseVolume[i]->closeVolume();
				delete baseVolume[i];
				delete volumeName[i];
			}
}

unsigned int SimPersistManager::newVolID (void)
{
	if (next_vid < 0xfffffe)
		{
			next_vid++;
			return (next_vid - 1);
		}

	unsigned int new_id  = 1;
	Bool done = FALSE;

	while (!done)
		{
			iterator i;

			for (i = DIDList.begin(); i != DIDList.end(); i++)
				if (VID_BOTH(i->VolID) == new_id)
					break;

			if (i == DIDList.end())
				done = TRUE;

			if (!done)
				new_id++;
		}

	return (new_id);
}

SimObject* SimPersistManager::addObject (SimObject *obj)
{
	SimPersistAddQuery query;

	if (obj->processQuery (&query))
		{
			Parent::addObject (obj);
			addObject (obj, query.zone, query.flags);
		}

	return (obj);
}

float SimPersistManager::updateTimer (SimObject *sobj)
{
	SimPersistTimeoutQuery query;
	float	interval;

	if (sobj->processQuery (&query))
		interval = query.interval;
	else
		interval = 1.0f;

	return ((float)manager->getCurrentTime() + interval);
}

void SimPersistManager::addObject (SimObject* sobj, SphereF sphere, int flags)
{
	for (iterator i = DIDList.begin(); i != DIDList.end(); i++)
		if (i->sim_obj == sobj)
			return;

	SimPersistTimeoutQuery query;
	float	interval;

	if (sobj->processQuery (&query))
		interval = query.interval;
	else
		interval = 1.0f;

	DIDObj *DID = new DIDObj;

	DID->ID = sobj->getId();
	DID->VolID = newVolID ();
	DID->sphere = sphere;
	DID->flags = flags;
	DID->time = updateTimer (sobj);
	DID->sim_obj = sobj;

	DIDList.push_back (*DID);
}

void SimPersistManager::removeObject (SimObject *obj)
{
	Parent::removeObject (obj);

	for (iterator i = DIDList.begin(); i != DIDList.end(); i++)
		if (i->sim_obj == obj)
			{
				DIDList.erase (i);
				i--;
			}
}

void SimPersistManager::purge (void)
{
	for (iterator itr = DIDList.begin(); itr != DIDList.end(); itr++)
		if (itr->sim_obj)
			manager->deleteObject (itr->sim_obj);

	DIDList.clear ();

	tempVolume.closeVolume ();
	createTempVolume ();

	for (int i = 0; i < 4; i++)
		if (baseVolume[i])
			{
				baseVolume[i]->closeVolume();
				delete baseVolume[i];
				delete volumeName[i];
				baseVolume[i] = NULL;
				volumeName[i] = NULL;
			}
}

SimObject* SimPersistManager::getObject (const char* name)
{
   name;    // fixes param not used WARNING
	return NULL;
}
SimObject* SimPersistManager::getObject (SimObjectId ID)
{
	for (iterator i = DIDList.begin(); i != DIDList.end(); i++)
		if (i->ID == ID)
			if (i->sim_obj)
				return (i->sim_obj);
			else
				return (restoreObject (i->VolID));

	return (NULL);
}

void SimPersistManager::save_vol (VolumeRWStream *volume, unsigned int vid_mask)
{
	// write out header / volume table of contents
	int size;
	int count;
	
	if (!vid_mask)
		{
			count = DIDList.size();
			size = sizeof (count) + (count * sizeof (DIDObj));
			volume->open ("vtoc", STRM_COMPRESS_NONE, size);
			volume->write (count);
			volume->write (count * sizeof (DIDObj), (void *)DIDList.address());
			volume->close ();
		}
	else
		{
			count = 0;
         iterator i;
			for ( i = DIDList.begin(); i != DIDList.end(); i++)
				if (!VID_VOL_PART(i->VolID) || VID_VOL_PART(i->VolID) == vid_mask)
					count++;

			size = sizeof (count) + (count * sizeof (DIDObj));
			volume->open ("vtoc", STRM_COMPRESS_NONE, size);
			volume->write (count);

			for ( i = DIDList.begin(); i != DIDList.end(); i++)
				if (!VID_VOL_PART(i->VolID) || VID_VOL_PART(i->VolID) == vid_mask)
					volume->write (sizeof (DIDObj), (void *)i);

			volume->close ();
		}


	// write each object
	for (iterator i = DIDList.begin(); i != DIDList.end(); i++)
		if (!vid_mask || !VID_VOL_PART(i->VolID) || VID_VOL_PART(i->VolID) == vid_mask)
			{
				if (!i->sim_obj)
					i->sim_obj = restoreObject (i->VolID);

				AssertFatal(i->sim_obj, "Can't find object to save!");

				i->VolID = VID_BOTH(i->VolID);

				volume->open (i->VolID, STRM_COMPRESS_NONE, 0);
				i->sim_obj->store (*volume);
				volume->close ();
			}
}

Bool SimPersistManager::saveVol (char *filename)
{
	//	create volume file
	VolumeRWStream volume;

	if (!volume.createVolume (filename))
		return (FALSE);

	save_vol (&volume, 0);

	volume.closeVolume ();

	return (TRUE);
}

Bool SimPersistManager::saveGame (char *filename)
{
   int i;
	VolumeRWStream saveVol;

	if (!saveVol.createVolume (filename))
		return (FALSE);

	// set up game table of contents
	if (!saveVol.open ("gtoc", STRM_COMPRESS_NONE, 0))
		return (FALSE);

	int vol_count = 0;

	for (i = 0; i < 4; i++)
		if (baseVolume[i])
			vol_count++;

	saveVol.write (vol_count);

	for (i = 0; i < 4; i++)
		if (baseVolume[i])
			{
				int size = strlen (volumeName[i]) + 1;

				saveVol.write (size);
				saveVol.write (size, volumeName[i]);
			}

	save_vol (&saveVol, VID_TEMP_VOL);

	saveVol.closeVolume ();

	return (TRUE);
}

Bool SimPersistManager::loadGame (char *filename)
{
	VolumeRWStream	saveVol;

	purge ();

	if (!saveVol.openVolume (filename))
		return (FALSE);

	if (!saveVol.open ("gtoc"))
		return (FALSE);

	int vol_count;

	saveVol.read (&vol_count);

	for (int i = 0; i < vol_count; i++)
		{
			char	*volname;
			int	num_chars;

			saveVol.read (&num_chars);
			volname = new char[num_chars];
			saveVol.read (num_chars, volname);
			openVol (volname, i);
			delete volname;
		}

	saveVol.closeVolume ();

	AssertFatal (vol_count != 4, "too many volumes in save game.");

	readToTemp (filename);

	// instantiate 'always' objects here...
	for (iterator itr = DIDList.begin(); itr != DIDList.end(); itr++)
		if (itr->flags & FLAG_ALWAYS)
			restoreObject (itr->VolID);

	return (TRUE);
}

Bool SimPersistManager::openVol (char *filename, int index)
{
	//	create volume file

	if (baseVolume[index])
		{
			baseVolume[index]->closeVolume ();	// need to also delete associated objects...
			delete baseVolume[index];
		}

	if (volumeName[index])
		delete volumeName[index];

	volumeName[index] = new char[strlen (filename) + 1];
	strcpy (volumeName[index], filename);

	baseVolume[index] = new VolumeRWStream();

	if (!baseVolume[index]->openVolume (filename))
		return (FALSE);

	// get volume table of contents...
	int DIDCount;
	int old_size = DIDList.size();

	if (!baseVolume[index]->open ("vtoc"))
		return (FALSE);

	baseVolume[index]->read (&DIDCount);

	DIDList.setSize (DIDCount + old_size);
	baseVolume[index]->read (sizeof (DIDObj) * DIDCount, &DIDList.address()[old_size]);

	for (iterator i = &DIDList[old_size]; i != DIDList.end(); i++)
		{
			// make sure each vol ID has the proper current and original volume encoded...
			i->VolID = VID_ID_PART(i->VolID) | (index + 1) << 24 | (index + 1) << 28;

			if (VID_ID_PART(i->VolID) >= next_vid)
				next_vid = VID_ID_PART (i->VolID) + 1;

			i->sim_obj = NULL;
		}

	return (TRUE);
}

Bool SimPersistManager::readToTemp (char *filename)
{
	VolumeRWStream volume;

	if (!volume.openVolume (filename))
		return (FALSE);

	// get volume table of contents...
	int DIDCount;
	int old_size = DIDList.size();

	if (!volume.open ("vtoc"))
		return (FALSE);

	volume.read (&DIDCount);

	DIDList.setSize (DIDCount + old_size);
	volume.read (sizeof (DIDObj) * DIDCount, &DIDList[old_size]);

	Persistent::Base::Error err;
	Persistent::Base* bptr;

	for (iterator i = &DIDList[old_size]; i != DIDList.end(); i++)
		{
			if (VID_ID_PART(i->VolID) >= next_vid)
				next_vid = VID_ID_PART (i->VolID) + 1;

			volume.open (VID_BOTH(i->VolID));
			bptr = Persistent::Base::load(volume,&err);
			volume.close ();
			i->sim_obj = dynamic_cast<SimObject*>(bptr);

			int last_size = DIDList.size ();
			timeout (i, FALSE);
			if (last_size != DIDList.size ())
				i--;

			// resolve collisions between read-only volumes and save games...
			for (iterator j = DIDList.begin(); j != &DIDList[old_size]; j++)
				if (j != i && VID_BOTH (i->VolID) == VID_BOTH (j->VolID))
					{
						DIDList.erase (j);
						i--;
						break;
					}
		}

	volume.closeVolume ();

	// instantiate 'always' objects here...

	return (TRUE);
}

SimObject* SimPersistManager::restoreObject (unsigned int vID)
{
	Persistent::Base::Error err;
	Persistent::Base* bptr;

	if (VID_VOL_PART(vID) != VID_TEMP_VOL)
		{
			int index = (VID_VOL_PART(vID) >> 28) - 1;

			AssertFatal(baseVolume[index], "Trying to load object from NULL volume.");

			if (VID_VOL_PART(vID) == VID_ORG_PART(vID) << 4)
				baseVolume[index]->open (VID_ID_PART(vID));
			else
				baseVolume[index]->open (VID_BOTH(vID));

			bptr = Persistent::Base::load(*baseVolume[index],&err);
			baseVolume[index]->close ();
		}
	else
		{
			if (tempVolume.open (VID_BOTH(vID)))
				bptr = Persistent::Base::load(tempVolume,&err);
			else
				AssertFatal(0, "Object could not be opened!?.");
				
			tempVolume.close ();
		}

	AssertFatal(bptr != 0, "Object does not exist in current volumes.");

	return (dynamic_cast<SimObject*>(bptr));
}

void SimPersistManager::wakeUp (SphereF zone)
{
	for (iterator i = DIDList.begin(); i != DIDList.end(); i++)
		if (i->flags & FLAG_ALWAYS ||
			 (m_distf (zone.center, i->sphere.center) < (zone.radius + i->sphere.radius)))
			{
				if (!i->sim_obj)
					{
						i->sim_obj = restoreObject (i->VolID);

						manager->addObject(i->sim_obj);
						manager->assignId (i->sim_obj, i->ID);
					}

				i->flags |= FLAG_ACTIVE;
			}
		else
			{
				if (i->flags & FLAG_ACTIVE && i->sim_obj)
					i->time = updateTimer (i->sim_obj);

				i->flags &= ~FLAG_ACTIVE;
			}
}

void SimPersistManager::timeout (iterator i, bool in_list)
{
	// timeout with a valid sim_obj - must be going from active to temp storage...
	if (i->sim_obj)
	 	{
			SimPersistTimeoutQuery query;
			int interval;

			if (i->sim_obj->processQuery (&query))
				{
					i->flags |= query.flags;
					interval = query.interval;
				}
			else
				{
					// default = expire object (not store) in 30 seconds...
					interval = 30.0f;
					i->flags |= FLAG_EXPIRE;
				}

			if (in_list && !query.timeout)
				return;

			i->time = (float)(manager->getCurrentTime()) + interval;
			i->VolID = VID_TEMP_VOL | VID_BOTH (i->VolID);

			tempVolume.open (VID_BOTH(i->VolID), STRM_COMPRESS_NONE, 0);
			i->sim_obj->store (tempVolume);
			tempVolume.close ();

			if (in_list)
				manager->deleteObject (i->sim_obj);

			i->sim_obj = NULL;
		}
	else	// timeout without a sim_obj - expiring from temp storage...
		{
			if (i->flags & FLAG_EXPIRE)	// revert to original volume stage...
				{
					if (!VID_ORG_PART (i->VolID))
						DIDList.erase (i);
					else
						i->VolID = VID_BOTH (i->VolID) | (VID_ORG_PART (i->VolID) << 4);

					// should remove entry from temp storage... 
				}
			else
				{
					// for now do nothing... eventually force write of temp RAM to disk.
				}
		}
}

bool SimPersistManager::onAdd()
{
   SimTimeEvent::post(this,(float)manager->getCurrentTime() + 0.1f);
   return Parent::onAdd();
}

bool SimPersistManager::processEvent(const SimEvent *event)
{
	switch (event->type)
		{
			case SimTimeEventType:
				// poll some portion of objects for time out... for now do entire list.
				static int index = 0;
				int range = DIDList.size() / 4;

				if (range < 1)
					range = 1;

				if (index > DIDList.size())
					index = 0;

				for (iterator i = &DIDList[index]; i != &DIDList[index + range] && i != DIDList.end(); i++)
					{
						SimImageTransformQuery query;

						if (i->sim_obj && i->sim_obj->processQuery (&query))
							i->sphere.center = query.transform.p;
						
						if (!(i->flags & FLAG_ACTIVE) && i->time < manager->getCurrentTime())
							{
								int old_size = DIDList.size ();
								timeout (i, TRUE);
								if (old_size != DIDList.size ())
									{
										i--;
										range--;
									}
							}
					}

				index += range;

				SimTimeEvent::post(this,(float)manager->getCurrentTime() + 0.1f);
				break;
		}
	
	return Parent::processEvent(event);
}

bool SimPersistManager::processQuery(SimQuery* query)
{
	switch (query->type){
		case SimLookupQueryType:
			{
				SimLookupQuery *qlook = static_cast<SimLookupQuery*>(query);

				if (qlook->name)	// can't resolve names as it is now...
					qlook->obj = NULL;
				else
					qlook->obj = getObject (qlook->id);
				
				return ((qlook->obj)!=NULL);
			}

		default:
			return false;
	}
   //can never reach here
	//return true;
}
