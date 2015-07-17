// Entity.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEntity window
#ifndef _ENTITY_H_
#define _ENTITY_H_

#include "thredprimitives.h"
#include "rendercamera.h"
#include "thredparser.h"
#include "ts_vertex.h"

#define CLASSNAME "classname"

enum EntityStyles { ENTITY_S_ORIGIN, ENTITY_S_BRUSH };
			
#define ENTITY_SELECTED		BIT_0
#define ENTITY_ACTIVE		BIT_1
	
class CEntity
{
// Construction
public:
	void UpdateOriginFirst();	// DPW - 10/23
	int IsEntityInBox(ThredBox& Box);
	double GetDistance(ThredPoint Point);
	void Export(CStdioFile& OutFile);
	void Serialize(CThredParser& Parser);
	void SetValueByKey(const char* Key, const char* Value);
	void MoveEntity(double X, double Y, double Z);
	void DoneMoveEntity(double GridSize);
	void RenderEntity(CRenderCamera* Camera, COLORREF Colour, TSRenderContext* rc, int ViewType);
	CString ValueByKey(const char *Key);
	void UpdateOrigin();
    CEntity &operator=(CEntity &Entity );  // Right side is the argument.
	CString GetName();
	CEntity();
	int IsActive();

	// whether this is a brush type
	// entity or an origin type entity
	EntityStyles EntityStyle;

	// some flags such as active or selected
	int mFlags;
    int mNumStates;

	// which group this entity belongs to.
	int mGroup;
	COLORREF mEntityColour;

	// the origin if it's an origin type
	ThredPoint mOrigin;
	ThredPoint mRenderOrigin;

    CEntity*    NextSelectedEntity;
    CEntity*    PrevSelectedEntity;

	// Put a group name for brushes if it is a brush type
	// the way this will work is that when you associate
	// a brush with an entity that brush will be created
	// into a new group and that group associated with
	// the entity.  When you add a group to the entity
	// if the entity already has a brush the group
	// will take that brushes group.  If not the brus
	// will take the groups name.
	CString BrushGroup;

	// array of key names
	CStringArray mKeyArray;

	// array of values
	CStringArray mValueArray;

	// whether we are active
	//int mActive;

protected:
    TS::VertexIndexPair         pairs[32];
    Point2F                     texture_indices[32];

};

/////////////////////////////////////////////////////////////////////////////
// make an entity array
typedef CArray<CEntity, CEntity&> CEntityArray;


#endif
