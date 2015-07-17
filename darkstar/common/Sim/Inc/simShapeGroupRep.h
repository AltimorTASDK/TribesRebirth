//------------------------------------------------------------------------------
// Description Represents a shape group on the client and server side.  Note
//              that it only contains SimShapes (for now).
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _SIMSHAPEGROUPREP_H_
#define _SIMSHAPEGROUPREP_H_

//Includes
#include <tVector.h>
#include <tAlgorithm.h>
#include "simNetObject.h"
#include "simContainer.h"

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

//-------------------------------------- Forward Decls.
class SimShape;
typedef VectorPtr<SimShape*> SimShapeList;
class SimTimerEvent;
class SimInspectQuery;

//------------------------------------------------------------------------------
class SimShapeGroupRep : public SimContainer
{
   typedef SimNetObject Parent;

   static int sm_fileVersion;

  protected:
   enum NetMaskFlags {
      ObjectsChangedMask = 1 << 0,
      InitialUpdate      = 1 << 31
   };
   
   SimShapeList m_objectList;
   
   bool        m_isInTimerSet;
   Vector<int> m_unresolvedGhostIds;
   
   // Damage forwarding control
   //
   bool m_globallyConserveDamage;      // G-/P+
   
  public:
   SimShapeGroupRep();
   ~SimShapeGroupRep();
   
   void damageForward(const SimShape*, const double);

   void initFromCurr();  // Sent by group when this object is added
   void removeAll();     // Sent by group when this object is removed
   
   virtual void _addObject(SimObject*);
   virtual void _removeObject(SimObject*);
   
   bool processEvent(const SimEvent*);
	bool onAdd();
   void onDeleteNotify(SimObject*);
	void inspectRead(Inspect *inspector);
	void inspectWrite(Inspect *inspector);

   bool onSimTimerEvent(const SimTimerEvent*);
   
   void addToTimerSet();
   void removeFromTimerSet();
   void processUnresolvedGhosts();
   
   DWORD packUpdate(Net::GhostManager *gm, DWORD mask, BitStream *stream);
   void  unpackUpdate(Net::GhostManager *gm, BitStream *stream);
   DECLARE_PERSISTENT(SimShapeGroupRep);
	Persistent::Base::Error read(StreamIO &, int version, int user);
	Persistent::Base::Error write(StreamIO &, int version, int user); 
   int version();

   // STL Style interface...
   typedef SimShapeList::iterator iterator;
   typedef SimShapeList::value_type value;
   SimShape* front() { return m_objectList.front();  }
   bool      empty() { return m_objectList.empty();  }
   int       size()  { return m_objectList.size();   }
   iterator  begin() { return m_objectList.begin();  }
   iterator  end()   { return m_objectList.end();    }
   value operator[] (int index) { return m_objectList[index];   }
};

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_SIMSHAPEGROUPREP_H_
