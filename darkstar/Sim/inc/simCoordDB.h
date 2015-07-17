
#ifndef _SIMCOORDDB_H_
#define _SIMCOORDDB_H_

#include <simContainer.h>

//--------------------------------------------------------------------------- 
// Base class for coordinate based spatial database
// doesn't fill in any of the SimcontainerDatabase methods
// purpose is to supply dimension list methods and object maintainance
// that are common to x, xy, and xyz db's
//--------------------------------------------------------------------------- 

class SimCoordinateDB : public SimContainerDatabase
{
	protected:

	struct ObjectListEntry
	{
		SimContainer * object; // if object==0, then entry unused;
		int xminIndex, xmaxIndex;// hard-coded for x,y, and z
		int yminIndex, ymaxIndex;
		int zminIndex, zmaxIndex;
		int tag;
	};

	typedef Vector<ObjectListEntry*> ObjectList;

	// list of objects and containers
	ObjectList objectList;
	SimContainerList containerList;
	int unusedObjEntries;

	float containerFarDist;

	// this db functions needs to be defined here (used by add)
	virtual bool findContained(const Box3F & box, SimContainerList *) = 0;

	struct DimensionEntry
	{
		ObjectListEntry * entry;
		float val;
		int * index; // holds its own index in dimension list
		             // points to x/y/z minIndex in an ObjectListEntry
		UInt32 bitIndex;
		UInt32 bit;
	};

	class DimensionList : public Vector<DimensionEntry*>
	{
		public:

		// add 2 entries into list -- 1 for min, 1 for max
		void addEntries(float minVal, float maxVal,
	                            int & minIndex, int & maxIndex,
	                            ObjectListEntry * objEntry);

		// remove min and max entries from list
		void removeEntries(int minIndex, int maxIndex);

		// update min and max entries
		void updateEntries(int & minIndex, int & maxIndex,
                                   float minVal, float maxVal,
                                   int newMinIndex, int newMaxIndex);

		// update a single entry
		void updateEntry(int & index, int newIndex, float val);

		// return list of containables that overlap with min,max on this dim
      // used by SimXDB
		void findIntersections(const Box3F & box,
                             UInt32 queryID, UInt32 typeMask,
                             UInt32 * bitList,
                             SimContainerList * iList);
      // method to test intersection on y & z coordinates -- used by above method
      bool isIntersectingYZ(ObjectListEntry * entry,
                            UInt32 queryID, UInt32 typeMask,
                            float minY, float maxY,
                            float minZ, float maxZ);

		// return list of containables that overlap with min,max on this dim
      // used by SimXYDB
		void findIntersections(const Box3F & box,
                             DimensionList & yList,
                             UInt32 queryID, UInt32 typeMask,
                             UInt32 * bitList,
                             SimContainerList * iList);
      // method to test intersection on y & z coordinates -- used by above method
      bool isIntersectingYZ(ObjectListEntry * entry,
                            UInt32 queryID, UInt32 typeMask,
                            int minY, int maxY,
                            float minZ, float maxZ);

		// return list of containables that overlap with min,max on this dim
      // used by render methods -- this routine doesn't know which dimension it's on
      // and only checks on that dimension
		void findIntersections(float min, float max,
                             UInt32 queryID, UInt32 typeMask,
                             UInt32 * bitList,
                             SimContainerList * iList);

		// used by SimXYDB
		bool findContained1(float minVal,float maxVal, int tag);
		bool findContained2(float minVal,float maxVal, int tag);
		void findContained3(float minVal,float maxVal, int tag,
                                    SimContainerList * outList);

		// used by SimXDB
		void findContained(float minVal,float maxVal, int tag,
                                   SimContainerList * outList);

		int findPosition(float val,
    	                         int loIndex, int hiIndex);

		void findPositions(float minVal, float maxVal,
		                           int & minIndex, int & maxIndex);

		void findMinMaxIndices(float minVal, float maxVal,
	                                   int & minIndex, int & maxIndex);

		void findMinMaxIterators(float minVal, float maxVal,
                                         DimensionList::iterator & minPos,
                                         DimensionList::iterator & maxPos);

		bool testEntry(float val, int index);

		// debugging routine
		void checkList();

		~DimensionList();
	};

	// for fast intersection/containment checking
	Vector<UInt32> bitList;
	int currentTag;

	public:
	SimCoordinateDB() { unusedObjEntries=0; containerFarDist = -1; }
	~SimCoordinateDB();

	void setFarDist(float fd) { containerFarDist = fd; }

	// tag and bitList for fast intersection/containment checking
	UInt32 * getBitList() { return bitList.address(); }
	int getInsertionTag();

	// management of contained items...
	ObjectListEntry * addItem(SimContainer *);
	bool remove(SimContainer *);
	bool removeAll(SimContainer * parent);
   void scope(SimContainerScopeContext & sc);
};

//--------------------------------------------------------------------------- 
// coordinate based spatial databases:
//--------------------------------------------------------------------------- 

class SimXDB : public SimCoordinateDB
{
	DimensionList xList;

	public:
	bool add(SimContainer*);
	bool remove(SimContainer*);
	void update(SimContainer*);
	bool findIntersections(const SimContainerQuery&,SimContainerList* list);
	SimContainer* findOpenContainer(const SimContainerQuery&);
	SimContainer* findOpenContainer(const SimContainerQuery&,
	                                    int xminIndex, int xmaxIndex);
	bool findContained(const Box3F &, SimContainerList *);
	bool findContained(const SimContainerQuery&, SimContainerList *);
	DimensionList & getXList() { return xList; }

	void render(SimContainerRenderContext & rc);
};

class SimXYDB : public SimCoordinateDB
{
	DimensionList xList;
	DimensionList yList;

	public:
	bool add(SimContainer*);
	bool remove(SimContainer*);
	void update(SimContainer*);
	bool findIntersections(const SimContainerQuery&,SimContainerList* list);
	SimContainer* findOpenContainer(const SimContainerQuery&);
	SimContainer* findOpenContainer(const SimContainerQuery&,
	                                    int xminIndex, int xmaxIndex,
	                                    int yminIndex, int ymaxIndex);
	bool findContained(const Box3F&, SimContainerList *);
	bool findContained(const SimContainerQuery&, SimContainerList *);
	DimensionList & getXList() { return xList; }
	DimensionList & getYList() { return yList; }

	void render(SimContainerRenderContext& rc);
};

#endif


