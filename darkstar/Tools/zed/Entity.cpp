// Entity.cpp : implementation file
//

#include "stdafx.h"
#include "thred.h"
#include "Entity.h"
#include "Thredparser.h"
#include "ts_PointArray.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEntity

CEntity::CEntity()
{
	// start of with new pairs but room for 10.
	mKeyArray.SetSize(0, 10);
	mValueArray.SetSize(0, 10);

	// default is an origin entity
	EntityStyle = ENTITY_S_ORIGIN;

	// start off active
	mFlags = 0;
	mFlags |= ENTITY_ACTIVE;

	// start off with no group
	mGroup = -1;
	// DPW mEntityColour = RGB(255,255,255);

	// DPW - Needed for TS compatibility
	mEntityColour = 255;
    mNumStates = 0;

    NextSelectedEntity = NULL;
    PrevSelectedEntity = NULL;

	// make a string
	char stringing[] = "0011001100mijrof1100";
}


// this this active
int CEntity::IsActive()
{
	return (mFlags & ENTITY_ACTIVE);
}


// this function will return the classname
// of the entity.
CString CEntity::GetName()
{
	return ValueByKey(CLASSNAME );
}

// copy over the shit... MAKE SURE WE UPDATE THIS!!!
CEntity& CEntity::operator=(CEntity& Entity )
{

	EntityStyle = Entity.EntityStyle;
	mFlags = Entity.mFlags;
	mGroup = Entity.mGroup;
	mOrigin = Entity.mOrigin;
    mNumStates = Entity.mNumStates;
	mRenderOrigin = Entity.mRenderOrigin;
	BrushGroup = Entity.BrushGroup;
    NextSelectedEntity = Entity.NextSelectedEntity;
    PrevSelectedEntity = Entity.PrevSelectedEntity;
	for(int Current = 0; Current < Entity.mKeyArray.GetSize(); Current++ ) {
		mKeyArray.Add(Entity.mKeyArray[Current] );
		mValueArray.Add(Entity.mValueArray[Current] );
	}

	return *this;
}


// this function will return the classname
// of the entity.
CString CEntity::ValueByKey(const char *Key)
{
	int CurrentString;
	int NumberOfStrings = mKeyArray.GetSize();

	// go through the array of entities
	for(CurrentString = 0; CurrentString < NumberOfStrings; CurrentString++ ) {
		if(!mKeyArray[CurrentString].CompareNoCase(Key) )
			return mValueArray[CurrentString];
	}

	// we should have found it!
	return CString("");
}

// update our origin
void CEntity::UpdateOrigin()
{
	// find our origin
	CString OriginStr = ValueByKey("pos");

	// if we empty leave
	if(OriginStr.IsEmpty() ) {
		EntityStyle = ENTITY_S_BRUSH;
		return;
	}
	else {
		EntityStyle = ENTITY_S_ORIGIN;
	}

	// get our x y z
	int x, y, z;
	sscanf(OriginStr, "%d %d %d", &x, &y, &z);

	// assign them 
	mOrigin.X = (double)x;
	mOrigin.Y = (double)y;
	mOrigin.Z = (double)z;
	mRenderOrigin = mOrigin;
}

// set the value
void CEntity::SetValueByKey(const char* Key, const char* Value)
{
	int CurrentString;
	int NumberOfStrings = mKeyArray.GetSize();

	// go through the array of entities
	for(CurrentString = 0; CurrentString < NumberOfStrings; CurrentString++ ) {
		if(!mKeyArray[CurrentString].CompareNoCase(Key) ) {
			mValueArray[CurrentString] = Value;
			break;
		}
	}
}

// this function will export an entity to qualke
void CEntity::Export(CStdioFile& OutFile)
{
	CString OutString;

	// this function will write out the entity
	// to a .MAP file.
	int Size = mKeyArray.GetSize();

	// write out an entity opening
	OutFile.WriteString("{\n");

	// write out the key's and values
	for(int Cur = 0; Cur < Size; Cur++ ) {
		// check if we have an origin
//		if(!mKeyArray[Cur].CompareNoCase("origin") ) {
		if(!mKeyArray[Cur].CompareNoCase("pos") ) {
			OutString.Format("\"%s\"\t\"%d %d %d\"\n", (LPCTSTR)mKeyArray[Cur],
				(int)mOrigin.X, (int)mOrigin.Z, (int)mOrigin.Y);
		}
		else {
			OutString.Format("\"%s\"\t\"%s\"\n",
				(LPCTSTR)mKeyArray[Cur], (LPCTSTR)mValueArray[Cur] );
		}		 
		OutFile.WriteString(OutString );
	}

}

// calculate the distance from our origin to this
// point in 2d space
double CEntity::GetDistance(ThredPoint Point)
{
	double x1, y1, x2, y2;

	// don't check us because we aren't real
	if(EntityStyle != ENTITY_S_ORIGIN )
		return 100000;

	// check which vars to use
	if(Point.X == 0 ) {
		x1 = mOrigin.Y;
		y1 = mOrigin.Z;
		x2 = Point.Y;
		y2 = Point.Z;
	}
	else if(Point.Y == 0 ) {
		x1 = mOrigin.X;
		y1 = mOrigin.Z;
		x2 = Point.X;
		y2 = Point.Z;
	}
	else {
		x1 = mOrigin.Y;
		y1 = mOrigin.X;
		x2 = Point.Y;
		y2 = Point.X;
	}

	// calculate our deltas
	double xdistance = x2-x1;
	double ydistance = y2-y1;
	return sqrt((xdistance*xdistance) + (ydistance*ydistance) );
}

#define CENTITYTYPE "CEntity"
#define CENTSTYLE "eStyle"
#define CENTORIGIN "eOrigin"
#define CENTPAIRCOUNT "ePairCount"
#define CENTFLAGS "eFlags"
#define CENTGROUP "eGroup"
#define CENTKEY "K"
#define CENTVALUE "V"
#define CENDENTTYPE "End CEntity"
void CEntity::Serialize(CThredParser& Parser)
{
	int Count;
	//CThredParser Parser(&ar, CENDENTTYPE);
	Parser.SetEnd(CENDENTTYPE);

	if(Parser.IsStoring ) {
		// we want to write our our transaction type
		Parser.WriteString(CENTITYTYPE);
		Parser.WriteInt(CENTSTYLE, EntityStyle );
		Parser.WritePoint(CENTORIGIN, mOrigin);

		// key/value count
		Count = mKeyArray.GetSize();

		Parser.WriteInt(CENTFLAGS, mFlags );
		Parser.WriteInt(CENTGROUP, mGroup );

		// the count of key/value pairs
		Parser.WriteInt(CENTPAIRCOUNT, Count );

		// go through and write out
		for(int Cur = 0; Cur < Count; Cur++ ) {
			Parser.WriteString(CENTKEY, mKeyArray[Cur]);
			Parser.WriteString(CENTVALUE, mValueArray[Cur]);
		}

		// we want to write our our transaction type
		Parser.WriteString(CENDENTTYPE);

	}
	else {
		// we want to write our our transaction type
		if(!Parser.GetKey(CENTITYTYPE) ) 
			return;

		// get our entity style
		int eStyle;
		Parser.GetKeyInt(CENTSTYLE, eStyle );
		EntityStyle = (EntityStyles)eStyle;
		Parser.GetKeyPoint(CENTORIGIN, mOrigin);


		// get our group and flags
		Parser.GetKeyInt(CENTFLAGS, mFlags );
		Parser.GetKeyInt(CENTGROUP, mGroup );

		// key/value count
		Parser.GetKeyInt(CENTPAIRCOUNT, Count);

		// go through and write out
		CString InString;
		while(Count-- ) {
			Parser.GetKeyString(CENTKEY, InString);
			mKeyArray.Add(InString );
			Parser.GetKeyString(CENTVALUE, InString);
			mValueArray.Add(InString );
		}
		Parser.GotoEnd();

	}

}

// check whether or not this entity is within this
// bounding box
int CEntity::IsEntityInBox(ThredBox& Box)
{
	ThredPoint NewOrig = mOrigin;

	// make a new origin that is zero in the same
	// axis as the box so when tested it works
	if(Box.mSmallest.X == 0 )
		NewOrig.X = 0;
	if(Box.mSmallest.Y == 0 )
		NewOrig.Y = 0;
	if(Box.mSmallest.Z == 0 )
		NewOrig.Z = 0;

	// check to see man....
	return Box.CheckPointIntersection(NewOrig);
}

//======================================================================
// This function will render the entity.
// using the renderentityorigin
//======================================================================
void CEntity::RenderEntity(CRenderCamera* Camera, COLORREF Colour, TSRenderContext* rc, int ViewType)
{
   Point2I	start, end;

   // don't draw us because we aren't real ;)
   if(EntityStyle != ENTITY_S_ORIGIN )
      return;

   // if we are not active don't draw
   if(!( mFlags & ENTITY_ACTIVE ) )
	   return;


   if( (ViewType == ID_VIEW_SOLIDRENDER ) ||
      ( ViewType == ID_VIEW_TEXTUREVIEW ) )
   {
      const float offset = 10.0;
      Camera->DrawDiamond( mRenderOrigin, offset, Colour, rc );
   }
   else
   {
      // our point
      ThredPoint TranPoint = mRenderOrigin;
      ThredPoint FakePoint;
      CPoint point, scalepoint;

      // translate and draw
      Camera->TranslateToScreen(point, TranPoint, rc);

      // create transfer a fake point for scale
      FakePoint.X = mRenderOrigin.X + 8;
      FakePoint.Y = mRenderOrigin.Y + 8;
      FakePoint.Z = mRenderOrigin.Z + 8;
   
      // do the scale
      Camera->TranslateToScreen(scalepoint, FakePoint, rc);
   
      // get a size		
      int xsize = scalepoint.x - point.x;

		// DPW - Needed for TS compatibility
		start.x = point.x-xsize;
		start.y = point.y-xsize;
		end.x = point.x+xsize;
		end.y = point.y+xsize;
		Camera->DrawLine (&start, &end, 3, Colour, rc);

		// draw another one
		start.x = point.x-xsize;
		start.y = point.y+xsize;
		end.x = point.x+xsize;
		end.y = point.y-xsize;
		Camera->DrawLine (&start, &end, 3, Colour, rc);
    }
}

// move the entity
void CEntity::MoveEntity(double X, double Y, double Z )
{
	// leave cause we don't have an origin to move.
	if(EntityStyle != ENTITY_S_ORIGIN )
		return;

	// change the stuff
	if(Globals.mAxis & AXIS_X ) {
		mRenderOrigin.X -= X;
	}
	if(Globals.mAxis & AXIS_Y ) {
		mRenderOrigin.Y -= Y;
	}
	if(Globals.mAxis & AXIS_Z ) {
		mRenderOrigin.Z -= Z;
	}

	// update to ints
	//mRenderOrigin.X = (double)((int)mRenderOrigin.X);
	//mRenderOrigin.Y = (double)((int)mRenderOrigin.Y);
	//mRenderOrigin.Z = (double)((int)mRenderOrigin.Z);
	mRenderOrigin.X = mRenderOrigin.X;
	mRenderOrigin.Y = mRenderOrigin.Y;
	mRenderOrigin.Z = mRenderOrigin.Z;
}

//==================================================================
// When we are done moving the entity we want to snap it to the
// grid etc.
//==================================================================
void CEntity::DoneMoveEntity(double GridSize)
{

	// Snap to the damn grid
	if(Globals.mAxis & AXIS_X ) 
		mRenderOrigin.X = RoundFloat((mRenderOrigin.X/GridSize) ) * GridSize;
	if(Globals.mAxis & AXIS_Y ) 
		mRenderOrigin.Y = RoundFloat((mRenderOrigin.Y/GridSize) ) * GridSize;
	if(Globals.mAxis & AXIS_Z ) 
		mRenderOrigin.Z = RoundFloat((mRenderOrigin.Z/GridSize) ) * GridSize;

	// if we are pusing the shit key we want to move only on greatest
	// on the axis we have the most movement on
	if(GetAsyncKeyState(VK_SHIFT) & 0x8000) {
		double XDis, YDis, ZDis;
		XDis = fabs(mRenderOrigin.X - mOrigin.X);
		YDis = fabs(mRenderOrigin.Y - mOrigin.Y);
		ZDis = fabs(mRenderOrigin.Z - mOrigin.Z);
		if(XDis > YDis ) {
			if(XDis > ZDis ) 
				mOrigin.X = mRenderOrigin.X;
			else
				mOrigin.Z = mRenderOrigin.Z;
		}
		else {
			if(YDis > ZDis )
				mOrigin.Y = mRenderOrigin.Y;
			else
				mOrigin.Z = mRenderOrigin.Z;
		}
		mRenderOrigin = mOrigin;
	}
	else mOrigin = mRenderOrigin;


	// now update the string
	// find our origin
	CString OriginStr; 

	// format
	OriginStr.Format("%d %d %d", (int)mOrigin.X, (int)mOrigin.Y, (int)mOrigin.Z);

	// set the new value
//	SetValueByKey("origin", OriginStr);
	SetValueByKey("pos", OriginStr);

}

// DPW - 10/23
// update our origin
void CEntity::UpdateOriginFirst()
{
	// find our origin
//	CString OriginStr = ValueByKey("origin");
	CString OriginStr = ValueByKey("pos");

	// if we empty leave
	if(OriginStr.IsEmpty() ) {
		EntityStyle = ENTITY_S_BRUSH;
		return;
	}
	else {
		EntityStyle = ENTITY_S_ORIGIN;
	}

	// get our x y z
	int x, y, z;
	sscanf(OriginStr, "%d %d %d", &x, &y, &z);

	// assign them 
	mOrigin.X = (double)x;
	mOrigin.Y = (double)z;
	mOrigin.Z = (double)y;
	mRenderOrigin = mOrigin;

	// format
	OriginStr.Format("%d %d %d", (int)mOrigin.X, (int)mOrigin.Y, (int)mOrigin.Z);

	// set the new value
//	SetValueByKey("origin", OriginStr);
	SetValueByKey("pos", OriginStr);
}