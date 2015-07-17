//--------------------------------------------------------------------------- 

#ifndef _SIMPARTDEBRIS_H_
#define _SIMPARTDEBRIS_H_

#include <SimDebris.h>
#include <ts_partInstance.h>

//---------------------------------------------------------------------------
// The following structure is used to map parts to an id corresponding to their
// size.  A list of these structures is passed to SimPartDebris constructor.
// List should be sorted in order of smallest to largest debris type.  If part
// can fit in ith entry, ith id is used with probability p, o.w. i+1 entry
// is tested.  List shoud be terminated with catch-all entry.  Note:  list is
// assumed to be static, because debris holds onto it for later use.
// DetailFactor determines the extent to which detailScale affects this piece.
// A value of 0 means it doesn't, 1 means it does fully.
struct SimDebrisPartInfo
{
    public:
    int id;
    float mass;
    float radius;
    float p;
    float detailScaleMin;
    float * shapeDetailScale;
};

//---------------------------------------------------------------------------


class SimPartDebrisCloud;

class SimPartDebris : public SimDebris
{
    friend SimPartDebrisCloud;

    SimDebrisPartInfo * chooser;

    bool onSimMessageEvent(const SimMessageEvent *);

public:
    SimPartDebris(TSPartInstance *, int _id, float timeOut);
    SimPartDebris(TSPartInstance *, SimDebrisPartInfo *, float timeOut);
    void initShape();
};

class SimPartDebrisCloud : public SimDebrisCloud
{
    typedef SimDebrisCloud Parent;

    TMat3F transform;
    TSShapeInstance * cannabalize;

    int debrisId;
    SimDebrisPartInfo * chooser;

    SimPartDebris ** list;
    int listSize;

    bool useBreakP;
    float breakP;

    void addDebris(SimPartDebris ** list, int count);
    void generateDebris();

public:
    SimPartDebrisCloud(TSShapeInstance *, const TMat3F &, const Point3F & pos, int id,
                       const Point3F & v, float outJ, const Point3F & inJ,
                       float spawnTO);

    SimPartDebrisCloud(TSShapeInstance *, const TMat3F &, const Point3F & pos, SimDebrisPartInfo *,
                       const Point3F & v, float outJ, const Point3F & inJ,
                       float spawnTO);

    SimPartDebrisCloud(SimPartDebris ** list, int listSize);

    void setBreakProbability(float p);

    bool onAdd();
};


#endif