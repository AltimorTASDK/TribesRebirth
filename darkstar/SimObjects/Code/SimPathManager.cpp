//----------------------------------------------------------------------------
// $Workfile$     SimPathManager.cpp
// $Revision$     1.0
// $Author  $     Robert Mobbs (robert.mobbs@dynamix.com)
// $Modtime $     05/17/98
//
// Copyright (c) 1998 Dynamix Incorporated.  All rights reserved
//----------------------------------------------------------------------------

#include <simpath.h>
#include <netpacketstream.h>
#include <netghostmanager.h>
#include <neteventmanager.h>
#include <simpathmanager.h>

///////////////////////////////////////////////////////////////////////////////
// SimPathManager methods
///////////////////////////////////////////////////////////////////////////////
SimPathManager::SimPathManager()
{
   id = SimPathManagerId;
}

SimPathManager::~SimPathManager()
{
   Vector<Path *>::iterator iter;

   // Delete all of our paths
   for (iter = paths.begin(); iter < paths.end(); iter ++)
   {
      delete (*iter);
   }
}

Int32 SimPathManager::addPath(Int32 iPathID, bool fLoop)
{
   // If they don't already have a path ID, add the path and assign them one
   if (iPathID == -1)
   {
      iPathID = paths.size();
      paths.push_back(new Path(iPathID, fLoop));
   }

   // Otherwise this is a path update / delete
   else
   {
      // Make sure they are requesting an in-bounds path
      if (paths.size() > iPathID)
      {
         // If the path already exists, delete it
         if (paths[iPathID])
         {
            delete paths[iPathID];
         }

         // Add in the path
         paths[iPathID] = new Path(iPathID, fLoop);
      }
   }

   return (iPathID);
}

Int32 SimPathManager::addWaypoint(Int32 iPathID, Point3F position, EulerF rotation)
{
   if (iPathID < 0 || iPathID >= paths.size())
   {
      return (-1);
   }

   paths[iPathID]->waypoints.push_back(new Waypoint(position, rotation));

   return (paths[iPathID]->waypoints.size() - 1);
}

Int32 SimPathManager::getWaypoint(Int32 iPathID, Int32 iWaypoint, Waypoint **waypoint)
{
   Path     *path;
   Waypoint *dest;

   // Check bounds on the path
   if (iPathID >= 0 && iPathID < paths.size())
   {
      // Get rid of repeated access
      path = paths[iPathID];

      // Have to check b/c someone may be editing the mission as we run
      if (path == NULL)
         return (0);

      // If this is a looping path, need to mod waypoint
      if (path->fLoop)
         iWaypoint %= path->waypoints.size();

      // Check bounds on the waypoint
      if (iWaypoint < path->waypoints.size() && iWaypoint >= 0)
      {
         // Get the waypoint
         (*waypoint) = path->waypoints[iWaypoint];

         // Find the next waypoint
         if (iWaypoint < (path->waypoints.size() - 1))
            dest = path->waypoints[iWaypoint + 1];
         else
            dest = path->waypoints[0];

         // Build the linear movement vector to next waypoint
         (*waypoint)->lVector.x = dest->position.x - (*waypoint)->position.x;
         (*waypoint)->lVector.y = dest->position.y - (*waypoint)->position.y;
         (*waypoint)->lVector.z = dest->position.z - (*waypoint)->position.z;

         // Build the angular movement vector to next waypoint
         (*waypoint)->aVector.x = dest->rotation.x - (*waypoint)->rotation.x;
         (*waypoint)->aVector.y = dest->rotation.y - (*waypoint)->rotation.y;
         (*waypoint)->aVector.z = dest->rotation.z - (*waypoint)->rotation.z;
      }
      return (paths[iPathID]->waypoints.size());
   }

   return (-1);
}

bool SimPathManager::updateClient(Net::PacketStream *nps)
{
   // Cycle through all of the paths
   for (int iIndex = 0; iIndex < paths.size(); iIndex ++)
      nps->getEventManager()->
        postRemoteEvent(new SimPathEvent(new Path(paths[iIndex]), iIndex));

   return (true);
}

bool SimPathManager::updateClients(const SimPathEvent *event)
{
   SimSet *set;

   // Find the packet stream set
   if ((bool)(set = static_cast<SimSet *>(manager->findObject(PacketStreamSetId))))
   {
      SimSet::iterator iter;

      // Cycle through all clients and update them
      for (iter = set->begin(); iter < set->end(); iter ++)
      {
         Net::PacketStream *nps = dynamic_cast<Net::PacketStream *>(*iter);

         // Only send the update if the client is a ghost
         if (nps && nps->getGhostManager()->
             getCurrentMode() == Net::GhostManager::GhostNormalMode)
         {
            SimPathEvent *newEvent = new SimPathEvent(event);
            nps->getEventManager()->postRemoteEvent(newEvent);
         }
      }
   }
   return (true);
}

void SimPathManager::deletePath(Int32 iPathID)
{
   // Delete our representation of the object -- clients will be notified
   // by timer event, as we must be in edit mode
   if (iPathID >= 0 && iPathID < paths.size())
   {
      delete paths[iPathID];
      paths[iPathID] = NULL;
   }
}

void SimPathManager::fullReset()
{
   paths.clear();
}

bool SimPathManager::processEvent(const SimEvent *event)
{
   switch (event->type)
   {
      onEvent(SimPathEvent);
   }

   return (false);
}

bool SimPathManager::onSimPathEvent(const SimPathEvent *event)
{
   // Sanity check
   if (event->iPathID >= 0)
   {
      // If the ID of the path is less than the size, it is either
      // a delete or an add/change
      if (event->iPathID < paths.size())
      {
         // Get rid of the old path
         if (paths[event->iPathID])
         {
            delete paths[event->iPathID];
            paths[event->iPathID] = NULL;
         }

         // If we were passed a new one, put it in the vector
         if (event->path)
         {
            paths[event->iPathID] = new Path(event->path);
         }
      }
      else
      {
         int i = paths.size();

         paths.setSize(event->iPathID + 1);

         while (i < event->iPathID)
            paths[i ++] = NULL;

         paths.last() = event->path ? new Path(event->path) : NULL;
      }

      if (!isGhost())
      {
         updateClients(event);
      }
   }

   // No matter what the result, we still handled the event
   return (true);
}

///////////////////////////////////////////////////////////////////////////////
// SimPathManager::Path methods
///////////////////////////////////////////////////////////////////////////////
SimPathManager::Path::Path()
{
}

SimPathManager::Path::Path(const Path &path)
{
   iPathID = path.iPathID;
   fLoop   = path.fLoop;

   for (int iIndex = 0; iIndex < path.waypoints.size(); iIndex ++)
   {
      waypoints.push_back(new Waypoint(path.waypoints[iIndex]));
   }
}

SimPathManager::Path::Path(Int32 iPathID, bool fLoop)
{
   this->iPathID = iPathID;
   this->fLoop   = fLoop;
}

SimPathManager::Path::Path(Path *path)
{
   iPathID = path->iPathID;
   fLoop   = path->fLoop;

   for (int iIndex = 0; iIndex < path->waypoints.size(); iIndex ++)
   {
      waypoints.push_back(new Waypoint(path->waypoints[iIndex]));
   }
}

SimPathManager::Path::~Path()
{
   for (int iIndex = 0; iIndex < waypoints.size(); iIndex ++)
   {
      delete (waypoints[iIndex]);
   }
}

void SimPathManager::Path::operator=(const Path &path)
{
   iPathID = path.iPathID;
   fLoop   = path.fLoop;

   for (int iIndex = 0; iIndex < path.waypoints.size(); iIndex ++)
   {
      waypoints.push_back(new Waypoint(path.waypoints[iIndex]));
   }
}

void SimPathManager::Path::operator=(const Path *path)
{
   iPathID = path->iPathID;
   fLoop   = path->fLoop;

   for (int iIndex = 0; iIndex < path->waypoints.size(); iIndex ++)
   {
      waypoints.push_back(new Waypoint(path->waypoints[iIndex]));
   }
}

bool SimPathManager::Path::operator==(const Path &path) const
{
   if (iPathID != path.iPathID ||
       fLoop   != path.fLoop   ||
       path.waypoints.size() != waypoints.size())
   {
      return (false);
   }

   for (int iIndex = 0; iIndex < waypoints.size(); iIndex ++)
   {
      if (waypoints[iIndex] != path.waypoints[iIndex])
         return (false);
   }

   return (true);
}

bool SimPathManager::Path::operator==(const Path *path) const
{
   if (iPathID != path->iPathID ||
       fLoop   != path->fLoop   ||
       path->waypoints.size() != waypoints.size())
   {
      return (false);
   }

   for (int iIndex = 0; iIndex < waypoints.size(); iIndex ++)
   {
      if (waypoints[iIndex] != path->waypoints[iIndex])
         return (false);
   }

   return (true);
}

bool SimPathManager::Path::operator!=(const Path *path) const
{
   return (!(this == path));
}

bool SimPathManager::Path::operator!=(const Path &path) const
{
   return (!(*this == path));
}

///////////////////////////////////////////////////////////////////////////////
// SimPathManager::Waypoint methods
///////////////////////////////////////////////////////////////////////////////
SimPathManager::Waypoint::Waypoint(Point3F position, EulerF rotation)
{
   this->rotation = rotation;
   this->position = position;
   lVector.set(0.0f, 0.0f, 0.0f);
   aVector.set(0.0f, 0.0f, 0.0f);
}

SimPathManager::Waypoint::Waypoint(Waypoint *waypoint)
{
   rotation = waypoint->rotation;
   position = waypoint->position;
   lVector  = waypoint->lVector;
   aVector  = waypoint->aVector;
}

SimPathManager::Waypoint::Waypoint()
{
}

SimPathManager::Waypoint::~Waypoint()
{
}

void SimPathManager::Waypoint::operator=(const Waypoint &waypoint)
{
   rotation = waypoint.rotation;
   position = waypoint.position;
   lVector  = waypoint.lVector;
   aVector  = waypoint.aVector;
}

void SimPathManager::Waypoint::operator=(const Waypoint *waypoint)
{
   rotation = waypoint->rotation;
   position = waypoint->position;
   lVector  = waypoint->lVector;
   aVector  = waypoint->aVector;
}

bool SimPathManager::Waypoint::operator==(const Waypoint &waypoint) const
{
   if (rotation != waypoint.rotation ||
       position != waypoint.position ||
       lVector  != waypoint.lVector  ||
       aVector  != waypoint.aVector)
   {
      return (false);
   }

   return (true);
}

bool SimPathManager::Waypoint::operator==(const Waypoint *waypoint) const
{
   if (rotation != waypoint->rotation ||
       position != waypoint->position ||
       lVector  != waypoint->lVector  ||
       aVector  != waypoint->aVector)
   {
      return (false);
   }

   return (true);
}

bool SimPathManager::Waypoint::operator!=(const Waypoint *waypoint) const
{
   return (!(this == waypoint));
}

bool SimPathManager::Waypoint::operator!=(const Waypoint &waypoint) const
{
   return (!(*this == waypoint));
}

///////////////////////////////////////////////////////////////////////////////
// SimPathEvent methods
///////////////////////////////////////////////////////////////////////////////
SimPathEvent::SimPathEvent(SimPathManager::Path *path, int iPathID)
{
   this->path       = path;
   this->iPathID    = iPathID;
   type             = SimPathEventType;
   address.objectId = SimPathManagerId;
}

SimPathEvent::SimPathEvent()
{
   path             = NULL;
   iPathID          = -1;
   type             = SimPathEventType;
   address.objectId = SimPathManagerId;
}

SimPathEvent::SimPathEvent(const SimPathEvent *event)
{
   if (event->path)
      path = new SimPathManager::Path(event->path);
   else
      path = NULL;

   iPathID          = event->iPathID;
   type             = SimPathEventType;
   address.objectId = SimPathManagerId;
}

SimPathEvent::SimPathEvent(Int32 iPathID)
{
   path             = NULL;
   this->iPathID    = iPathID;
   type             = SimPathEventType;
   address.objectId = SimPathManagerId;
}

SimPathEvent::~SimPathEvent()
{
   if (path)
      delete path;
}

void SimPathEvent::pack(SimManager *mgr, Net::PacketStream *nps, BitStream *bstrm)
{
   SimEvent::pack(mgr, nps, bstrm);

   // Write the path ID
   bstrm->write(iPathID);

   // Signal to unpack
   bstrm->writeFlag(path ? true : false);

   // If we have no path, this is a delete event.  Otherwise, this is
   // an add/change event
   if (path)
   {
      bstrm->writeFlag(path->fLoop);
      bstrm->write(path->waypoints.size());

      for (int iIndex = 0; iIndex < path->waypoints.size(); iIndex ++)
      {
         bstrm->write(sizeof(SimPathManager::Waypoint), 
                     (void *)path->waypoints[iIndex]);
      }
   }
}

void SimPathEvent::unpack(SimManager *mgr, Net::PacketStream *nps, BitStream *bstrm)
{
   if(!verifyNotServer(mgr))
      return;

   Int32                    iSize;
   SimPathManager::Waypoint wp;

   SimEvent::unpack(mgr, nps, bstrm);

   // Read the path ID
   bstrm->read(&iPathID);

   // See if they passed a path.  If not, this is a delete event.  If so,
   // this is an add/change event
   if (bstrm->readFlag())
   {
      path          = new SimPathManager::Path;
      path->iPathID = iPathID;
      path->fLoop   = bstrm->readFlag();

      bstrm->read(&iSize);

      for (int iIndex = 0; iIndex < iSize; iIndex ++)
      {
         bstrm->read(sizeof(SimPathManager::Waypoint), (void *)&wp);
         path->waypoints.push_back(new SimPathManager::Waypoint(wp));
      }
   }
   else
   {
      path = NULL;
   }
}

IMPLEMENT_PERSISTENT_TAG(SimPathEvent, SimPathEventType);
