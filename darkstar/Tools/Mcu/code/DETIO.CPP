#include "stdafx.h"
#define DL_TABLE_INSTANCE  // instantiate classio table
#include "detail.h"

//---------------------------------------------------------------------------

void DLInit()
{
   ClassItem::loadTable( ClassDLTypeList, ClassDLTypeListSize );
}

//---------------------------------------------------------------------------
// I/O methods
//---------------------------------------------------------------------------

void Detail::read( StreamIO& sio )
{
   sio.read( sizeof( fName ), fName );
   sio.read( sizeof( fShrink), &fShrink );
   sio.read( sizeof( fScale ), &(fScale) );
   sio.read( sizeof( fDetailSize ), &(fDetailSize) );
   sio.read( &fSort );
}

void Detail::write( StreamIO& sio )
{
   sio.write( sizeof( fName ), fName );
   sio.write( sizeof( fShrink ), &fShrink );
   sio.write( sizeof( fScale ), &fScale );
   sio.write( sizeof( fDetailSize ), &fDetailSize );
   sio.write( fSort );
}

//-------------------------------------------------------------------------

ClassItem* COREAPI loadDetailListItemOld(StreamIO& sio, UInt32 flag )
{
    DetailList* p = new(ClassItem::CurrentPool) DetailList;
	p->classRead(sio, 0);
	return p;
}

ClassItem* COREAPI loadDetailListItem(StreamIO& sio, UInt32 flag )
{
	UInt32 version;
	sio.read( &version );
	DetailList* p = new(ClassItem::CurrentPool) DetailList;
	p->classRead(sio, version);
	return p;
}

void COREAPI DetailList::classRead(StreamIO& sio, UInt32 flag)
{
	UInt32 version = flag;
   if( version > 0 )
      {
		sio.read(&fMergePointTolerance);
		sio.read(&fMergeNormalTolerance);
      }
	else
      {
		fMergePointTolerance = 0.0f;
		fMergeNormalTolerance = 0.0f;
      }
   sio.read(&fnDetails);
   sio.read(&fAlphaNumLevels);
   sio.read(&fAlphaNumColors);

   AssertFatal( fnDetails < MAX_DETAILS,
      "DetailList::classRead: Too many details" );
	for( int i = 0; i < fnDetails; i++ )
      faDetails[i].read( sio );

}

void COREAPI DetailList::classWrite(StreamIO& sio, UInt32 flag)
{
   const UInt32 version = 1;
   sio.write( version );
	sio.write( fMergePointTolerance );
	sio.write( fMergeNormalTolerance );
	sio.write( fnDetails );
   sio.write( fAlphaNumLevels );
   sio.write( fAlphaNumColors );
   for( int i = 0; i < fnDetails; i++ )
      faDetails[i].write( sio );
}

UInt32 COREAPI DetailList::classMemSize()
{
	return sizeof(DetailList);
}

ClassType COREAPI DetailList::classType()
{
	return CLS_DetailList;
}

UInt32 COREAPI DetailList::classSize()
{
	return 	sizeof( fMergePointTolerance ) +
            sizeof( fMergeNormalTolerance ) +
			   sizeof (fnDetails) +
            sizeof( fAlphaNumLevels ) +
            sizeof( fAlphaNumColors ) +
				fnDetails * sizeof (Detail);
}

void COREAPI DetailList::classDump(Int32 detailLevel,Int32 indent)
{
	if( !detailLevel )	// if low enough detail level, don't show the rest
		return;

	printf( "%*ccount: %s\n", indent, ' ', fnDetails );
	printf( "%*cAlpha Levels: %s\n", indent, ' ', fAlphaNumLevels );
	printf( "%*cAlpha Colors: %s\n", indent, ' ', fAlphaNumColors );
	printf( "%*cPoint Merge Tolerance: %f\n", indent, ' ', fMergePointTolerance );
	printf( "%*cNormal Merge Tolerance: %f\n", indent, ' ', fMergeNormalTolerance );
   indent++;
   for( int i = 0; i < fnDetails; i++ )
      {
   	printf( "%*cname: %s\n", indent, ' ', faDetails[i].fName );
   	printf( "%*cshrink: %f\n", indent, ' ', faDetails[i].fShrink );
   	printf( "%*cscale: %f\n", indent, ' ', faDetails[i].fScale  );
      }
}

//---------------------------------------------------------------------------
