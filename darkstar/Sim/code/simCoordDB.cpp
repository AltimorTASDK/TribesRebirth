//---------------------------------------------------------------------
// SimCoordinateDB
//---------------------------------------------------------------------

#include <simCoordDB.h>

SimCoordinateDB::ObjectListEntry * 
SimCoordinateDB::addItem(SimContainer * object)
{
	// If the new object is open, add to
   // container list (open list) in addition to object list
	if (object->isOpen() && object->getContainerMask())
		// add to container list (in addition to objectList)
      // this is so that when we search for containers
      // that might hold an item, e.g. on update, we only
      // have to loop through the containers (now called open containers)
		containerList.push_back(object);

	ObjectListEntry * entry;
	int entryIndex;
	if (unusedObjEntries)
	{
		ObjectList::iterator itr2 = objectList.begin();
		entryIndex = 0;
		while ((*itr2)->object)
		{
			itr2++;
			entryIndex++;
		}
		entry = *itr2;
		unusedObjEntries--;
	}
	else
	{
		objectList.increment();
		objectList.last() = entry = new ObjectListEntry;
		entryIndex = objectList.size()-1;
		// make sure bitList is big enough
		bitList.setSize((entryIndex>>5)+1);
		bitList.last() = (UInt32) 0;
	}
	entry->object = object;
	object->setContainerData(entryIndex);
	return entry;
}

bool 
SimCoordinateDB::remove(SimContainer * object)
{
	int objectIndex = object->getContainerData();
	ObjectListEntry * entry = objectList[objectIndex];

	entry->object = 0;
	object->setContainerData(-1);
	unusedObjEntries++;

   // if object is open, remove from containerList
   // don't check containerMask (as on addItem) since
   // mask may have changed
	if (object->isOpen() && !containerList.empty())
	{
		SimContainerList::iterator itr;
		for (itr = containerList.begin();
		     itr!=containerList.end() && *itr != object;
			  itr++) ;
      if (itr!=containerList.end())
      {
   		*itr = containerList.last();
	   	containerList.decrement();         
      }
	}

	return true;
}

bool 
SimCoordinateDB::removeAll(SimContainer * parent)
{
	// Remove the objects and place then into our parent.
	for (ObjectList::iterator itr = objectList.begin(); 
	     itr != objectList.end();
	     itr++ )
	{
		SimContainer * object = (*itr)->object;
		if (object)
		{
			if (parent && !object->isDeleted())
				parent->addObject(object);
         else
   			object->getContainer()->removeObject(object);
		}
	}

   return true;
}


SimCoordinateDB::~SimCoordinateDB()
{
	for (int i=0; i<objectList.size(); i++)
		delete objectList[i];
}


SimCoordinateDB::DimensionList::~DimensionList()
{
	for (int i=0; i<size(); i++)
		delete (*this)[i];
}

void 
SimCoordinateDB::DimensionList::addEntries(float minVal, float maxVal,
	                                       int & minIndex, int & maxIndex,
	                                       ObjectListEntry * objEntry)
{
	// add a pair of entries to the db

	minIndex=maxIndex=-1; // don't have a "previous" value
	findMinMaxIndices(minVal,maxVal,minIndex,maxIndex);

//	AssertFatal(testDimensionEntry(minVal,minIndex,list),"oops min");
//	AssertFatal(testDimensionEntry(maxVal,maxIndex,list),"oops max");

	// increase list size by 2
	int sz = size();
	setSize(sz+2);
	memmove(&(*this)[maxIndex+2],&(*this)[maxIndex],
	        sizeof(DimensionEntry*)*(sz-maxIndex));
	memmove(&(*this)[minIndex+1],&(*this)[minIndex],
	        sizeof(DimensionEntry*)*(maxIndex-minIndex));

	// get bitIndex and bit for object
	int objIndex = objEntry->object->getContainerData();
	UInt32 bitIndex = objIndex >> 5;
	UInt32 bit      = 0x01 << (objIndex & 0x1f);

	DimensionEntry * minEntry = (*this)[minIndex] = new DimensionEntry;
	minEntry->val = minVal;
	minEntry->entry = objEntry;
	minEntry->index = &minIndex;
	minEntry->bitIndex = bitIndex;
	minEntry->bit = bit;

	// maxIndex increase because min of insertion
	DimensionEntry * maxEntry = (*this)[++maxIndex] = new DimensionEntry;
	maxEntry->val = maxVal;
	maxEntry->entry = objEntry;
	maxEntry->index = &maxIndex;
	maxEntry->bitIndex = bitIndex;
	maxEntry->bit = bit;

	// now we need to update indices
	// everything between min and max goes up 1, everything after max goes up 2
	DimensionList::iterator itr  = &(*this)[minIndex+1];
	DimensionList::iterator itr2 = &(*this)[maxIndex];
	for (;itr != itr2;itr++)
		(*(*itr)->index)++;
	for (itr++;itr!=end();itr++)
		(*(*itr)->index) += 2;
}

void 
SimCoordinateDB::DimensionList::removeEntries(int minIndex, int maxIndex)
{
	// first, get rid of entries data structures
	// eventually might want to have some repository for these
	delete (*this)[minIndex];
	delete (*this)[maxIndex];

	// set pointers to 0 to ease debugging (not that we'll need any)
	(*this)[minIndex]=0;
	(*this)[maxIndex]=0;

	// move the list items
	memmove(&(*this)[minIndex],&(*this)[minIndex+1],
	        sizeof(DimensionEntry*)*(maxIndex-minIndex-1));
	memmove(&(*this)[maxIndex-1],&(*this)[maxIndex+1],
	        sizeof(DimensionEntry*)*(size()-maxIndex-1));
	setSize(size()-2);

	// now we need to update indices
	// everything between entry presently occupying minIndex to entry
	// presently occupying maxIndex-1 goes down by 1
	// everything from maxIndex on goes down by 2
	DimensionList::iterator itr  = &(*this)[minIndex];
	DimensionList::iterator itr2 = &(*this)[maxIndex-1];
	for (;itr != itr2;itr++)
		(*(*itr)->index)--;
	for (;itr!=end();itr++)
		(*(*itr)->index) -= 2;
}


// A note on updateEntries:
// The old min and max indices are held in minIndex and maxIndex (which reference
// the object list entry).  The new indicies are held in newMinIndex and newMaxIndex
// (the are assumed to have been looked up by findMinMaxIndices by the caller).
// There are 6 possible orderings of new and old entries.  Let a and b be the old
// min and max indicies, and let c and d be the new.  The the possible orderings are:
//----------------------------------------------------------------------
// 1)                         a                 b
//                                     c                 d
//----------------------------------------------------------------------
// 2)                         a                 b
//                                 c    d
//----------------------------------------------------------------------
// 3)                         a                 b
//                        c                         d
//----------------------------------------------------------------------
// 4)                         a                 b
//                        c          d
//----------------------------------------------------------------------
// 5)                         a                 b
//                                                  c*  d
//----------------------------------------------------------------------
// 6)                         a                 b
//                      c  d**
//----------------------------------------------------------------------
// In the first 4 cases we can update the min and max independently.
// The later two, however, require special (but simple) attention.
// In particular, in case 5 min must be updated first whereas in case
// 6 max must be updated first (key is that a and b are automatically
// changed if one update effects the other, but c and d are not).
// [Note:  case 6 still done min first for other reasons (so that min and
// max can be kept in order), but we adjust indices accordingly].
// *  In case 5, if c=b, c considered less than b (actual value is)
// ** In case 6, if d=a, d considered less than a (actual value is)

void 
SimCoordinateDB::DimensionList::updateEntries(int & minIndex, int & maxIndex,
                                              float minVal, float maxVal,
                                              int newMinIndex, int newMaxIndex)
{
	AssertFatal(minIndex<maxIndex,"updateEntries:  oops1");

	if (newMaxIndex<=minIndex)
	{
		// case 6 above
		updateEntry(minIndex,newMinIndex,minVal);
		updateEntry(maxIndex,newMaxIndex+1,maxVal);
	}
	else
	{
		// cases 1-5
		updateEntry(minIndex,newMinIndex,minVal);
		updateEntry(maxIndex,newMaxIndex,maxVal);
	}

	AssertFatal(minIndex<maxIndex,"updateEntries:  oops2");
}

void 
SimCoordinateDB::DimensionList::updateEntry(int & index, int newIndex, float val)
{
	// first, does the index for this entry have to change?
	if (newIndex==index || newIndex==index+1)
	{
		(*this)[index]->val = val;
		return;
	}

	DimensionList::iterator itr, prevPos, endPos;
	DimensionEntry * de;

	if (val>(*this)[index]->val)
	{
		newIndex--; // decrement because below everything will move down one

		de = (*this)[index];
		prevPos = &(*this)[index];
		endPos  = &(*this)[newIndex];
      AssertFatal(index<=newIndex,"SimCoordDB: corrupt dimension list");
		for (itr=prevPos; itr++!=endPos; prevPos++)
		{
			*prevPos=*itr;
			(*(*prevPos)->index)--;
		}
		*endPos=de;
		de->val = val;
		index=newIndex; // same as de->index=newIndex
	}
	else
	{
		de = (*this)[index];
		prevPos = &(*this)[index];
		endPos  = &(*this)[newIndex];
      AssertFatal(index>=newIndex,"SimCoordDB: corrupt dimension list");
		for (itr=prevPos; itr--!=endPos; prevPos--)
		{
			*prevPos=*itr;
			(*(*prevPos)->index)++;
		}
		*endPos=de;
		de->val = val;
		index=newIndex; // same as de->index=newIndex
	}
}

//------------------------------------------------------------------------------

// utility function used by findIntersections -- returns true if object in entry
// intersects supplied y and z ranges and if it matches other criteria...
inline bool
SimCoordinateDB::DimensionList::isIntersectingYZ(SimCoordinateDB::ObjectListEntry * entry,
                                                 UInt32 queryID, UInt32 typeMask,
                                                 int minY, int maxY,
                                                 float minZ, float maxZ)
{
   SimContainer * obj = entry->object;
   if (!obj->isOpen() && (queryID==obj->getId() || !(typeMask&obj->getType())))
      return false;

   if (entry->yminIndex >= maxY || entry->ymaxIndex < minY)
      return false;

   const Box3F & box = entry->object->getBoundingBox();
   if (box.fMin.z >= maxZ || box.fMax.z <= minZ)
      return false;
      
   return true;
}

// return a list (iList) of objects that intersect box
// this routine assumes this list is the xList and that the yList is passed in
void 
SimCoordinateDB::DimensionList::findIntersections(const Box3F & box,
                                                  SimCoordinateDB::DimensionList & yList,
                                                  UInt32 queryID, UInt32 typeMask,
                                                  UInt32 * bitList,
                                                  SimContainerList * ilist)
{
	DimensionList::iterator minPos,maxPos, itr, endPos;
	findMinMaxIterators(box.fMin.x,box.fMax.x,minPos,maxPos);

   int ymin = -1, ymax = -1;
	yList.findMinMaxIndices(box.fMin.y,box.fMax.y,ymin,ymax);
   
   float zmin, zmax;
   zmin = box.fMin.z;
   zmax = box.fMax.z;
   
	for (itr=begin(); itr!=minPos; itr++)
		bitList[(*itr)->bitIndex] ^= (*itr)->bit; // toggle bit

	for (itr=minPos; itr!=maxPos; itr++)
	{
		// toggle the bit -- if that clears it, add object to list
		UInt32 & bit = bitList[(*itr)->bitIndex];
		bit ^= (*itr)->bit;
		if ( !(bit & (*itr)->bit) )
		{
         ObjectListEntry * entry = (*itr)->entry;
         if (isIntersectingYZ(entry,queryID,typeMask,ymin,ymax,zmin,zmax))
			{
		  		ilist->increment();
	  			ilist->last() = entry->object;
			}
		}
	}

	endPos = end();
	for (itr=maxPos; itr!=endPos; itr++)
		// if bit set, add object to list and clear the bit
		if ( (bitList[(*itr)->bitIndex] & (*itr)->bit) )
		{
			bitList[(*itr)->bitIndex] ^= (*itr)->bit; // clear by toggling
         ObjectListEntry * entry = (*itr)->entry;
         if (isIntersectingYZ(entry,queryID,typeMask,ymin,ymax,zmin,zmax))
			{
		  		ilist->increment();
	  			ilist->last() = entry->object;
			}
		}
}

//------------------------------------------------------------------------------

// utility function used by findIntersections -- returns true if object in entry
// intersects supplied y and z ranges and if it matches other criteria...
inline bool
SimCoordinateDB::DimensionList::isIntersectingYZ(SimCoordinateDB::ObjectListEntry * entry,
                                                 const UInt32 queryID, const UInt32 typeMask,
                                                 float minY, float maxY,
                                                 float minZ, float maxZ)
{
   SimContainer * obj = entry->object;
   if (!obj->isOpen() && (queryID==obj->getId() || !(typeMask&obj->getType())))
      return false;

   const Box3F & box = entry->object->getBoundingBox();

   if (box.fMin.y >= maxY || box.fMax.y <= minY ||
       box.fMin.z >= maxZ || box.fMax.z <= minZ)
      return false;
      
   return true;
}

// return a list (iList) of objects that intersect box
// this routine assumes this list is the xList but doesn't use yList like above version
void 
SimCoordinateDB::DimensionList::findIntersections(const Box3F & box,
                                                  UInt32 queryID, UInt32 typeMask,
                                                  UInt32 * bitList,
                                                  SimContainerList * ilist)
{
	DimensionList::iterator minPos,maxPos, itr, endPos;
	findMinMaxIterators(box.fMin.x,box.fMax.x,minPos,maxPos);

   float ymin, ymax;
	ymin = box.fMin.y;
   ymax = box.fMax.y;
   
   float zmin, zmax;
   zmin = box.fMin.z;
   zmax = box.fMax.z;
   
	for (itr=begin(); itr!=minPos; itr++)
		bitList[(*itr)->bitIndex] ^= (*itr)->bit; // toggle bit

	for (itr=minPos; itr!=maxPos; itr++)
	{
		// toggle the bit -- if that clears it, add object to list
		UInt32 & bit = bitList[(*itr)->bitIndex];
		bit ^= (*itr)->bit;
		if ( !(bit & (*itr)->bit) )
		{
         ObjectListEntry * entry = (*itr)->entry;
         if (isIntersectingYZ(entry,queryID,typeMask,ymin,ymax,zmin,zmax))
			{
		  		ilist->increment();
	  			ilist->last() = entry->object;
			}
		}
	}

	endPos = end();
	for (itr=maxPos; itr!=endPos; itr++)
		// if bit set, add object to list and clear the bit
		if ( (bitList[(*itr)->bitIndex] & (*itr)->bit) )
		{
			bitList[(*itr)->bitIndex] ^= (*itr)->bit; // clear by toggling
         ObjectListEntry * entry = (*itr)->entry;
         if (isIntersectingYZ(entry,queryID,typeMask,ymin,ymax,zmin,zmax))
			{
		  		ilist->increment();
	  			ilist->last() = entry->object;
			}
		}
}

//------------------------------------------------------------------------------

// old version of findIntersections -- still used by render
// this method doesn't know which dimension list it is, and only checks this dim
void 
SimCoordinateDB::DimensionList::findIntersections(float min, float max,
                                                  UInt32 queryID, UInt32 typeMask,
                                                  UInt32 * bitList,
                                                  SimContainerList * ilist)
{
	DimensionList::iterator minPos,maxPos, itr, endPos;
	findMinMaxIterators(min,max,minPos,maxPos);

	for (itr=begin(); itr!=minPos; itr++)
		bitList[(*itr)->bitIndex] ^= (*itr)->bit; // toggle bit

	for (itr=minPos; itr!=maxPos; itr++)
	{
		// toggle the bit -- if that clears it, add object to list
		UInt32 & bit = bitList[(*itr)->bitIndex];
		bit ^= (*itr)->bit;
		if ( !(bit & (*itr)->bit) )
		{
			SimContainer * obj = (*itr)->entry->object;
			if ( obj->isOpen() ||
			     ( queryID!=obj->getId() && typeMask&obj->getType()) )
			{
		  		ilist->increment();
	  			ilist->last() = (*itr)->entry->object;
			}
		}
	}

	endPos = end();
	for (itr=maxPos; itr!=endPos; itr++)
		// if bit set, add object to list and clear the bit
		if ( (bitList[(*itr)->bitIndex] & (*itr)->bit) )
		{
			bitList[(*itr)->bitIndex] ^= (*itr)->bit; // clear by toggling
			SimContainer * obj = (*itr)->entry->object;
			if ( obj->isOpen() ||
			     ( queryID!=obj->getId() && typeMask&obj->getType()) )
			{
		  		ilist->increment();
  				ilist->last() = (*itr)->entry->object;
			}
		}
}
//------------------------------------------------------------------------------

bool 
SimCoordinateDB::DimensionList::findContained1(float minVal,float maxVal, int tag)
{
	// assign tag to any entry between minVal and maxVal,
	// unless entry->tag == tag, then increment tag

	DimensionList::iterator minPos,maxPos;
	findMinMaxIterators(minVal,maxVal,minPos,maxPos);

	DimensionList::iterator itr;

	for (itr=minPos; itr!=maxPos; itr++)
	{
		int & etag = (*itr)->entry->tag;
		if (etag == tag)
			etag++;
		else
			etag = tag;
	}

	return minPos!=maxPos;

}

bool 
SimCoordinateDB::DimensionList::findContained2(float minVal,float maxVal, int tag)
{
	// increment tag of any entry between minVal and maxVal

	DimensionList::iterator minPos,maxPos;
	findMinMaxIterators(minVal,maxVal,minPos,maxPos);

	DimensionList::iterator itr;

	bool gotOne = false;
	for (itr=minPos; itr!=maxPos; itr++)
		if (++(*itr)->entry->tag == tag)
			gotOne = true;

	return gotOne;
}

void 
SimCoordinateDB::DimensionList::findContained3(float minVal,float maxVal, int tag,
                                               SimContainerList * outList)
{
	// for any entry between minVal and maxVal,
	// if tag==entry->tag, add to outList
	// o.w., increment

	DimensionList::iterator minPos,maxPos;
	findMinMaxIterators(minVal,maxVal,minPos,maxPos);

	DimensionList::iterator itr;

	for (itr=minPos; itr!=maxPos; itr++)
	{
		int & etag = (*itr)->entry->tag;
		if (etag == tag)
		{
			outList->increment();
			outList->last() = (*itr)->entry->object;
		}
		else
			etag++;
	}
}

void 
SimCoordinateDB::DimensionList::findContained(float minVal,float maxVal, 
                                              int tag, SimContainerList * outList)
{
	// for any entry between minVal and maxVal,
	// if tag==entry->tag, add to outList
	// o.w., set entry->tag to tag

	DimensionList::iterator minPos,maxPos;
	findMinMaxIterators(minVal,maxVal,minPos,maxPos);

	DimensionList::iterator itr;

	for (itr=minPos; itr!=maxPos; itr++)
	{
		int & etag = (*itr)->entry->tag;
		if (etag == tag)
		{
			outList->increment();
			outList->last() = (*itr)->entry->object;
		}
		else
			etag=tag;
	}
}


// list[loIndex] < val <= list[hiIndex]
// return index s.t. list[index-1] < val <= list[index]
// assume list[-1] = - infinity and list[size] = infinity
int 
SimCoordinateDB::DimensionList::findPosition(float val, int loIndex, int hiIndex)
{
	while (loIndex+1 != hiIndex)
	{
		int pivotIndex = loIndex + ((hiIndex-loIndex) >> 1);
		if (val <= (*this)[pivotIndex]->val)
			hiIndex = pivotIndex;
		else
			loIndex = pivotIndex;
	}
	return hiIndex;
}

void 
SimCoordinateDB::DimensionList::findPositions(float minVal, float maxVal,
		                                      int & minIndex, int & maxIndex)
{
	// Start with and maintain:
	//    minVal >  list[minIndex-1].val, and
	//    maxVal <= list[maxIndex].val,
	// where we assume that:
	//    list[-1].val = - infinity and list[size].val = + infinity
	// Found spots for min and max when:
	//    minVal <= list[minIndex].val, and
	//    maxVal >  list[maxIndex-1].val

	float currentMinVal;
	float preMaxVal;

	if (maxIndex)
	{
		currentMinVal = (*this)[minIndex]->val;
		preMaxVal  = (*this)[maxIndex-1]->val;
	}
	else
	{
		currentMinVal = -1.0E30f;
		preMaxVal = 1.0E30f;
	}

	int pivotIndex;
	float pivotVal;
	while ( minIndex != maxIndex  && 
	        (maxVal   <= preMaxVal ||
	        minVal   >  currentMinVal) )
	{
		pivotIndex = minIndex + ((maxIndex-minIndex) >> 1);
		pivotVal = (*this)[pivotIndex]->val;
		if (minVal > pivotVal)
		{
			minIndex = pivotIndex+1;
			if (minIndex != maxIndex)
				currentMinVal = (*this)[minIndex]->val;
		}
		else if (maxVal <= pivotVal)
		{
			maxIndex=pivotIndex;
			if (maxIndex > minIndex)
				preMaxVal = (*this)[maxIndex-1]->val;
			else
				maxIndex=minIndex;
		}
		else
		{
			// search for min and max separately
			minIndex = findPosition(minVal,minIndex-1,pivotIndex);
			maxIndex = findPosition(maxVal,pivotIndex,maxIndex);
			break;
		}
	}
}

void 
SimCoordinateDB::DimensionList::findMinMaxIndices(float minVal, float maxVal,
	                                              int & minIndex, int & maxIndex)
{
	enum { minFound = 0x01, maxFound = 0x02 };
	int logic = 0;

	// choose starting minIndex -- use previous value for guess
	if (minIndex==-1)
		minIndex=0;
	else if (minIndex==0 || minVal > (*this)[minIndex-1]->val)
	{
		if (minIndex==size() || minVal <= (*this)[minIndex]->val)
			logic |= minFound;
		// check next entry too under assumption that order of list changes infrequently
		else if (++minIndex==size() || minVal <= (*this)[minIndex]->val)
			logic |= minFound;
		// else, starting value of minIndex will be current value
	}
	else
		minIndex=0;

	// choose starting maxIndex -- use previous value for guess
	if (maxIndex==-1)
		maxIndex=size();
	else if (maxIndex==size() || maxVal <= (*this)[maxIndex]->val)
	{
		if (maxIndex==0 || maxVal > (*this)[maxIndex-1]->val)
			logic |= maxFound;
		// else, starting value of maxIndex will be current value
	}
	else
	{
		// try next biggest value on assumption that index normally doesn't change
		if (++maxIndex==size() || maxVal <= (*this)[maxIndex]->val)
			logic |= maxFound;
		else
			maxIndex = size();
	}

	switch (logic)
	{
		case 0 :
			// search for both
			findPositions(minVal,maxVal,minIndex,maxIndex);
			break;
		case minFound :
			// just search for max
			maxIndex = findPosition(maxVal,minIndex-1,maxIndex);
			break;
		case maxFound :
			// just search for min
			minIndex = findPosition(minVal,minIndex-1,maxIndex);
			break;
		// case:  min and max found, our work is done
	}
}

void 
SimCoordinateDB::DimensionList::findMinMaxIterators(float minVal, float maxVal,
                                                    DimensionList::iterator & minPos,
                                                    DimensionList::iterator & maxPos)
{
	int minIndex=-1,maxIndex=-1;
	findMinMaxIndices(minVal,maxVal,minIndex,maxIndex);

	minPos = &(*this)[minIndex];
	if (maxIndex==size())
		maxPos = end();
	else
		maxPos = &(*this)[maxIndex];
}

bool 
SimCoordinateDB::DimensionList::testEntry(float val, int index)
{
	return ( (index==size() || val <= (*this)[index]->val) &&
	         (index==0 || (*this)[index-1]->val < val) );
}

void 
SimCoordinateDB::DimensionList::checkList()
{
	for (int i=0; i < size(); i++)
	{
		AssertFatal(i==0 || (*this)[i-1]->val <= (*this)[i]->val,"non-mono");
		AssertFatal(*((*this)[i]->index)==i,"index out of synch");
	}
}

int 
SimCoordinateDB::getInsertionTag()
{
	currentTag += 5;
	return currentTag;
}

void
SimCoordinateDB::scope(SimContainerScopeContext & sc)
{
	// Scope all the objects and sub-containers.
	// This method is a little different then the normal database 
	// functions in that it recursively operates on nested open
	// containers.
	//
   SimNetObject::CameraInfo *ci = sc.getInfo();

	for (ObjectList::iterator itr = objectList.begin();
			itr != objectList.end(); itr++ )
	{
      SimContainer * object = (*itr)->object;

      if (!object)
         continue;

      if ((object->getBoxCenter() - ci->pos).len() > ci->visibleDistance)
         continue;

		if (object->isOpen())
		{
			if (sc.getTag() != object->getTag())
				object->scope(sc);
		}
		else
			if (object->isScopeable())
				sc.scopeObject(object);
	}
}


//--------------------------------------------------------------------------- 
// coordinate based spatial databases:
//--------------------------------------------------------------------------- 

//
// SimXDB
//

bool 
SimXDB::add(SimContainer * object)
{
	ObjectListEntry * entry = addItem(object);

	const Box3F & box = object->getBoundingBox();
	xList.addEntries(box.fMin.x,box.fMax.x,
	                 entry->xminIndex,entry->xmaxIndex,
	                 entry);

	return true;
}

bool 
SimXDB::remove(SimContainer*object)
{
	int objectIndex = object->getContainerData();
	ObjectListEntry * entry = objectList[objectIndex];

	xList.removeEntries(entry->xminIndex,entry->xmaxIndex);

	return SimCoordinateDB::remove(object);
}

void 
SimXDB::update(SimContainer * object)
{
	ObjectListEntry * entry = objectList[object->getContainerData()];

	// find new indices of object's new bounding box
	// will be used by findContainer and updateEntries
	// Note: new indices really just reference slot that min and
	// max bounds fall into.  When the update is done, they move
	// up or down 1.

	const Point3F & boxmin = object->getBoundingBox().fMin;
	const Point3F & boxmax = object->getBoundingBox().fMax;
	int xminIndex = entry->xminIndex;
	int xmaxIndex = entry->xmaxIndex;
	xList.findMinMaxIndices(boxmin.x,boxmax.x,xminIndex,xmaxIndex);

	SimContainerQuery query;
	query.id = object->getId();
	query.type = object->getType();
	query.mask = object->getContainerMask();
	query.box = object->getBoundingBox();
	query.detail = SimContainerQuery::SystemDetail;

	SimContainer* cnt = findOpenContainer(query,xminIndex,xmaxIndex);

	if (cnt)
	{
		cnt->addObject(object);
      return;
	}

	// Stays in this container, but update dimension lists
	xList.updateEntries(entry->xminIndex,entry->xmaxIndex,
	                    boxmin.x,boxmax.x,
	                    xminIndex,xmaxIndex);
}

bool 
SimXDB::findIntersections(const SimContainerQuery & query, SimContainerList* iList)
{
	int startSize = iList->size();
   xList.findIntersections(query.box,query.id,query.mask,bitList.address(),iList);
	return iList->size()!=startSize;
}

SimContainer* 
SimXDB::findOpenContainer(const SimContainerQuery& query)
{
	int xminIndex=-1,xmaxIndex=-1;
	const Point3F & boxmin = query.box.fMin;
	const Point3F & boxmax = query.box.fMax;
	xList.findMinMaxIndices(boxmin.x,boxmax.x,xminIndex,xmaxIndex);

	// do the work below
	return findOpenContainer(query,xminIndex,xmaxIndex);

}

SimContainer* 
SimXDB::findOpenContainer(const SimContainerQuery& query,int xminIndex, int xmaxIndex)
{
	const Box3F & queryBox = query.box;

	for (SimContainerList::iterator itr = containerList.begin();
	     itr != containerList.end();
	     itr++)
	{
		SimContainer * cnt = *itr;
		if (query.id==cnt->getId() || !(cnt->getContainerMask()&query.type))
			continue;

		ObjectListEntry & ole = *objectList[cnt->getContainerData()];
		const Box3F & cntBox = ole.object->getBoundingBox();
		if (ole.xminIndex <  xminIndex &&
		    ole.xmaxIndex >= xmaxIndex &&
		    cntBox.fMin.y <  queryBox.fMin.y &&
		    cntBox.fMax.y >  queryBox.fMax.y &&
		    cntBox.fMin.z <  queryBox.fMin.z &&
		    cntBox.fMax.z >  queryBox.fMax.z)
			return cnt;
	}

	return 0;

}

// get list of objects in this container that are contained within box
bool
SimXDB::findContained(const Box3F & box, SimContainerList * containedList)
{
	int tag = getInsertionTag();
	const Point3F & boxmin = box.fMin;
	const Point3F & boxmax = box.fMax;

	xList.findContained(boxmin.x,boxmax.x,tag,containedList);

	for (SimContainerList::iterator itr = containedList->begin();
	     itr != containedList->end();
		 itr++)
	{
		ObjectListEntry & ole = *objectList[(*itr)->getContainerData()];
		const Box3F & objBox = ole.object->getBoundingBox();
		if (objBox.fMin.y < boxmin.y ||
		    objBox.fMax.y > boxmax.y ||
		    objBox.fMin.z < boxmin.z ||
		    objBox.fMax.z > boxmax.z)
		{
			// not contained on other dimensions
			*itr = containedList->last();
			containedList->decrement();
			itr--;
		}
	}
	
	return !containedList->empty();
}

bool 
SimXDB::findContained(const SimContainerQuery & query, SimContainerList * containedList)
{
   // TODO: specialize for query
   const Box3F & box = query.box;

	int tag = getInsertionTag();
	const Point3F & boxmin = box.fMin;
	const Point3F & boxmax = box.fMax;

	xList.findContained(boxmin.x,boxmax.x,tag,containedList);

	for (SimContainerList::iterator itr = containedList->begin();
	     itr != containedList->end();
		 itr++)
	{
		ObjectListEntry & ole = *objectList[(*itr)->getContainerData()];
		const Box3F & objBox = ole.object->getBoundingBox();
		if (objBox.fMin.y < boxmin.y ||
		    objBox.fMax.y > boxmax.y ||
		    objBox.fMin.z < boxmin.z ||
		    objBox.fMax.z > boxmax.z)
		{
			// not contained on other dimensions
			*itr = containedList->last();
			containedList->decrement();
			itr--;
		}
	}
	
	return !containedList->empty();
}

void
SimXDB::render(SimContainerRenderContext & rc)
{
	// get camera
	TSCamera * camera = rc.currentState().renderContext->getCamera();

	// set up box to hold viewcone
	Box3F vcBox;
	camera->getViewconeBox( vcBox, containerFarDist);

	SimContainerList list;
	list.clear();

	// viewcone bbox is really big, but still might be able to eliminate something based on x-coord
	UInt32 * bitList = getBitList();
	xList.findIntersections(vcBox.fMin.x,vcBox.fMax.x,rc.id,rc.mask,bitList,&list);

	const int size = objectList.size();
	for (SimContainerList::iterator itr = list.begin();
			itr != list.end(); itr++ )
	{
		SimContainer * object = *itr;

		const Box3F & box = object->getBoundingBox();

		int vis = camera->testVisibility(box, containerFarDist);

		if (vis==TS::ClipNoneVis)
			continue;

      if(camera->getCameraType() != TS::OrthographicCameraType)
      {
         // now make sure that some part of object is w/in containerFarDist
         // -- camera checks z-depth, we're checking distance as the crow flies here
         Point3F radii = box.fMax-box.fMin; radii *= 0.5f;
         Point3F center = box.fMin + box.fMax; center *= 0.5f;
         Point3F offset = center - camera->getTCW().p;

         // find closest position on box to camera center
         Point3F closestPosition = offset;

         // following 3 if/else statements walk to box
         // corner that is closest to camera position
         if (offset.x > 0.0f)
            closestPosition.x -= radii.x;
         else
            closestPosition.x += radii.x;

         if (offset.y > 0.0f)
            closestPosition.y -= radii.y;
         else
            closestPosition.y += radii.y;

         if (offset.z > 0.0f)
            closestPosition.z -= radii.z;
         else
            closestPosition.z += radii.z;

         // is closest position w/in containerFardist
         if (m_dot(closestPosition,closestPosition) > containerFarDist * containerFarDist)
            continue; // vis = TS::ClipNoneVis
      }
		// push visibility so that sub-containers inherit our vis flags
		camera->pushVisibility( vis );

		if (object->isOpen())
		{
			if (rc.getTag() != object->getTag())
				object->render(rc);
		}
		else
			rc.render(object);

		camera->popVisibility();
      
      AssertFatal(size == objectList.size(),
         "object changed containers during render cycle");
	}
}

//
// SimXYDB
//

bool 
SimXYDB::add(SimContainer * object)
{
	ObjectListEntry * entry = addItem(object);

	const Box3F & box = object->getBoundingBox();
	xList.addEntries(box.fMin.x,box.fMax.x,
	                 entry->xminIndex,entry->xmaxIndex,
	                 entry);

	yList.addEntries(box.fMin.y,box.fMax.y,
	                 entry->yminIndex,entry->ymaxIndex,
	                 entry);

	return true;
}

bool 
SimXYDB::remove(SimContainer*object)
{
	int objectIndex = object->getContainerData();
	ObjectListEntry * entry = objectList[objectIndex];

	xList.removeEntries(entry->xminIndex,entry->xmaxIndex);
	yList.removeEntries(entry->yminIndex,entry->ymaxIndex);

	return SimCoordinateDB::remove(object);
}

void
SimXYDB::update(SimContainer * object)
{
	ObjectListEntry * entry = objectList[object->getContainerData()];

	// find new indices of object's new bounding box
	// will be used by dbFindOpenContainer and updateDimensionEntries
	// Note: new indices really just reference slot that min and
	// max bounds fall into.  When the update is done, they move
	// up or down 1.

	const Point3F & boxmin = object->getBoundingBox().fMin;
	const Point3F & boxmax = object->getBoundingBox().fMax;
	int xminIndex = entry->xminIndex;
	int xmaxIndex = entry->xmaxIndex;
	int yminIndex = entry->yminIndex;
	int ymaxIndex = entry->ymaxIndex;
	xList.findMinMaxIndices(boxmin.x,boxmax.x,xminIndex,xmaxIndex);
	yList.findMinMaxIndices(boxmin.y,boxmax.y,yminIndex,ymaxIndex);

	SimContainerQuery query;
	query.id = object->getId();
	query.type = object->getType();
	query.mask = object->getContainerMask();
	query.box = object->getBoundingBox();
	query.detail = SimContainerQuery::SystemDetail;

	SimContainer* cnt = findOpenContainer(query,xminIndex,xmaxIndex,yminIndex,ymaxIndex);

	if (cnt)
	{
		cnt->addObject(object);
      return;
	}

	// Stays in this container -- now update dimension lists
	xList.updateEntries(entry->xminIndex,entry->xmaxIndex,
	                    boxmin.x,boxmax.x,
	                    xminIndex,xmaxIndex);
	yList.updateEntries(entry->yminIndex,entry->ymaxIndex,
	                    boxmin.y,boxmax.y,
	                    yminIndex,ymaxIndex);
}

bool 
SimXYDB::findIntersections(const SimContainerQuery & query, SimContainerList* iList)
{
	int startSize = iList->size();
	xList.findIntersections(query.box,yList,query.id,query.mask,bitList.address(),iList);
	return iList->size()!=startSize;
}

SimContainer* 
SimXYDB::findOpenContainer(const SimContainerQuery& query)
{
	int xminIndex=-1,xmaxIndex=-1;
	int yminIndex=-1,ymaxIndex=-1;
	const Point3F & boxmin = query.box.fMin;
	const Point3F & boxmax = query.box.fMax;
	xList.findMinMaxIndices(boxmin.x,boxmax.x,xminIndex,xmaxIndex);
	yList.findMinMaxIndices(boxmin.y,boxmax.y,yminIndex,ymaxIndex);

	// do the work below
	return findOpenContainer(query,xminIndex,xmaxIndex,yminIndex,ymaxIndex);

}

SimContainer* 
SimXYDB::findOpenContainer(const SimContainerQuery& query,int xminIndex, int xmaxIndex,
                           int yminIndex, int ymaxIndex)
{
	const Box3F & queryBox = query.box;

	for (SimContainerList::iterator itr = containerList.begin();
	     itr != containerList.end();
		 itr++)
	{
		SimContainer * cnt = *itr;
		if (query.id==cnt->getId() || !(cnt->getContainerMask()&query.type))
			continue;

		ObjectListEntry & ole = *objectList[(*itr)->getContainerData()];
		const Box3F & cntBox = ole.object->getBoundingBox();
		if (ole.xminIndex <  xminIndex &&
		    ole.xmaxIndex >= xmaxIndex &&
		    ole.yminIndex <  yminIndex &&
		    ole.ymaxIndex >= ymaxIndex &&
		    cntBox.fMin.z <  queryBox.fMin.z &&
		    cntBox.fMax.z >  queryBox.fMax.z)
			return cnt;
	}

	return 0;

}

// get list of objects in this container that are contained within box
bool 
SimXYDB::findContained(const Box3F & box, SimContainerList * containedList)
{
	int tag = getInsertionTag();
	const Point3F & boxmin = box.fMin;
	const Point3F & boxmax = box.fMax;

	xList.findContained1(boxmin.x,boxmax.x,tag-2);
	yList.findContained3(boxmin.y,boxmax.y,tag,containedList);

	for (SimContainerList::iterator itr = containedList->begin();
	     itr != containedList->end();
		 itr++)
	{
		ObjectListEntry & ole = *objectList[(*itr)->getContainerData()];
		const Box3F & objBox = ole.object->getBoundingBox();
		if (objBox.fMin.z < boxmin.z ||
		    objBox.fMax.z > boxmax.z)
		{
			// not contained on z dimension
			*itr = containedList->last();
			containedList->decrement();
			itr--;
		}
	}
	
	return !containedList->empty();
}

bool 
SimXYDB::findContained(const SimContainerQuery & query, SimContainerList * containedList)
{
  // TODO: specialize for query 
  const Box3F & box = query.box;

	int tag = getInsertionTag();
	const Point3F & boxmin = box.fMin;
	const Point3F & boxmax = box.fMax;

	xList.findContained1(boxmin.x,boxmax.x,tag-2);
	yList.findContained3(boxmin.y,boxmax.y,tag,containedList);

	for (SimContainerList::iterator itr = containedList->begin();
	     itr != containedList->end();
		 itr++)
	{
		ObjectListEntry & ole = *objectList[(*itr)->getContainerData()];
		const Box3F & objBox = ole.object->getBoundingBox();
		if (objBox.fMin.z < boxmin.z ||
		    objBox.fMax.z > boxmax.z)
		{
			// not contained on z dimension
			*itr = containedList->last();
			containedList->decrement();
			itr--;
		}
	}
	
	return !containedList->empty();
}

void
SimXYDB::render(SimContainerRenderContext & rc)
{
	// get camera
	TSCamera * camera = rc.currentState().renderContext->getCamera();

	// set up box in rc to hold viewcone
	Box3F vcBox;
	camera->getViewconeBox( vcBox, containerFarDist);

	SimContainerList list;
	list.clear();

	// viewcone bbox is really big -- do our best to find a range of x or y that eliminates something
	UInt32 * bitList = getBitList();
	if (vcBox.fMax.x-vcBox.fMin.x < vcBox.fMax.y-vcBox.fMin.y)
	   xList.findIntersections(vcBox.fMin.x,vcBox.fMax.x,rc.id,rc.mask,bitList,&list);
	else
	   yList.findIntersections(vcBox.fMin.y,vcBox.fMax.y,rc.id,rc.mask,bitList,&list);

	const int size = objectList.size();
	for (SimContainerList::iterator itr = list.begin();
			itr != list.end(); itr++ )
	{
		SimContainer * object = *itr;

		const Box3F & box = object->getBoundingBox();

		int vis = camera->testVisibility(box, containerFarDist);

		if (vis==TS::ClipNoneVis)
			continue;
      if(camera->getCameraType() != TS::OrthographicCameraType)
      {
         // now make sure that some part of object is w/in containerFarDist
         // -- camera checks z-depth, we're checking distance as the crow flies here
         Point3F radii = box.fMax-box.fMin; radii *= 0.5f;
         Point3F center = box.fMin + box.fMax; center *= 0.5f;
         Point3F offset = center - camera->getTCW().p;

         // find closest position on box to camera center
         Point3F closestPosition = offset;

         // following 3 if/else statements walk to box
         // corner that is closest to camera position
         if (offset.x > 0.0f)
            closestPosition.x -= radii.x;
         else
            closestPosition.x += radii.x;

         if (offset.y > 0.0f)
            closestPosition.y -= radii.y;
         else
            closestPosition.y += radii.y;

         if (offset.z > 0.0f)
            closestPosition.z -= radii.z;
         else
            closestPosition.z += radii.z;

         // is closest position w/in containerFardist
         if (m_dot(closestPosition,closestPosition) > containerFarDist * containerFarDist)
            continue; // vis = TS::ClipNoneVis
      }

		camera->pushVisibility(vis);

		if (object->isOpen())
		{
			if (rc.getTag() != object->getTag())
				object->render(rc);
		}
		else
			rc.render(object);

		camera->popVisibility();

      AssertFatal(size == objectList.size(),
         "object changed containers during render cycle");
	}
}




