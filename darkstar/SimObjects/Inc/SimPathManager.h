//----------------------------------------------------------------------------
// $Workfile$     SimPathManager.h
// $Revision$     1.0
// $Author  $     Robert Mobbs (robert.mobbs@dynamix.com)
// $Modtime $     05/17/98
//
// Copyright (c) 1998 Dynamix Incorporated.  All rights reserved
//----------------------------------------------------------------------------

#ifndef _H_SIMPATHMANAGER
#define _H_SIMPATHMANAGER

#include <sim.h>
#include <tvector.h>

class SimPathEvent;
class SimPathManager : public SimNetObject
{
   public:
      class Waypoint
      {
         public:
            Vector3F aVector;
            Vector3F lVector;
            EulerF   rotation;
            Point3F  position;

            Waypoint();
            ~Waypoint();
            Waypoint(Waypoint *);
            Waypoint(Point3F, EulerF);
            void operator= (const Waypoint &);
            void operator= (const Waypoint *);
            bool operator==(const Waypoint &) const;
            bool operator==(const Waypoint *) const;
            bool operator!=(const Waypoint *) const;
            bool operator!=(const Waypoint &) const;
      };

      class Path
      {
         public :
            Int32              iPathID;
            bool               fLoop;
            Vector<Waypoint *> waypoints;

            Path();
            ~Path();
            Path(Path *);
            Path(const Path &);
            Path(Int32, bool);

            void operator= (const Path &);
            void operator= (const Path *);
            bool operator==(const Path &) const;
            bool operator==(const Path *) const;
            bool operator!=(const Path *) const;
            bool operator!=(const Path &) const;
      };

   private :
      Vector<Path *> paths;

   public :
      SimPathManager();
      ~SimPathManager();

      bool  updateClients(const SimPathEvent *);
      bool  updateClient(Net::PacketStream *);
      bool  processEvent(const SimEvent *);
      void  deletePath(Int32);
      void  fullReset();
      bool  onSimPathEvent(const SimPathEvent *);
      bool  notifyClients();
      Int32 addPath(Int32, bool);
      Int32 addWaypoint(Int32, Point3F, EulerF);
      Int32 getWaypoint(Int32, Int32, Waypoint **);
};

class SimPathEvent : public SimEvent
{
   public:
      SimPathManager::Path *path;
      Int32                 iPathID;

      SimPathEvent();
      ~SimPathEvent();
      SimPathEvent(Int32);
      SimPathEvent(const SimPathEvent *);
      SimPathEvent(SimPathManager::Path *, int);

      void pack  (SimManager *, Net::PacketStream *, BitStream *);
      void unpack(SimManager *, Net::PacketStream *, BitStream *);

      DECLARE_PERSISTENT(SimPathEvent);
};

#endif