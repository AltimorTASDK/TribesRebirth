//================================================================
//   
// $Workfile:   volstrm.h  $
// $Version$
// $Revision:   2.0  $
//	
// DESCRIPTION:
//
// (c) Copyright Dynamix Inc.   All rights reserved.
//================================================================

#ifndef _VOLSTRM_H_
#define _VOLSTRM_H_

#include <base.h>
#include <string.h>
#include <tVector.h>
#include "streams.h"

#ifdef __BORLANDC__
#pragma option -a1
#endif
#ifdef _MSC_VER
#pragma pack(push,1)
#endif

struct VolumeItem
{
   DWORD  ID;
   char  *string;
   DWORD  blockOffset;
   UInt32  size;
   UInt8  compressType;
};

class   FindMatch
{
   char     *expression;
   Int32    maxMatches;
public:
   static bool isMatch( const char *exp, const char *string, bool caseSensitive = FALSE );
   Vector   <char *> matchList;

	    FindMatch( Int32 _maxMatches=256 );
	    FindMatch( char *_expression, Int32 _maxMatches=256 );
	    ~FindMatch();

   bool   findMatch( const char *string, bool caseSensitive = FALSE );
   Int32  numMatches() const { return(matchList.size()); }
   bool   isFull() const { return (matchList.size() >= maxMatches); }
   void   setExpression( const char *_expression );
   void   clear()   { matchList.clear(); }
};

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif
 
#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif


char* DWORD_TO_ASCII(DWORD lName, char *ascii);

//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 Base Volume Stream
//
class VolumeStream: public StreamIO
{

protected:
   friend class ResourceManager; // hack, but res manager reads this directly

   UInt32              stringBlockOffset;
   int                 rawStringDataSize;
   char               *rawStringData;
   typedef Vector<VolumeItem> VolumeItemVector;
   VolumeItemVector    volumeItems;
   Vector<int>         idSortedItems;
   Vector<int>         stringSortedItems;

   BYTE   *base;
   HANDLE hFileMap;
   HANDLE hFile;
   int    openSize;

   StreamIO       *userStream;
   MemRWStream    memRWStream;
   BlockRWStream  blockRWStream;
   RLERStream     rleRStream;
   LZHRStream     lzhRStream;
   Bool readWrite;

   //컴컴컴컴컴컴컴컴컴컴 File Mapping Members
   Bool openFile(const char *pVolumeFileName );
   void closeFile();
   Bool createMap(Int32 size = -1);
   void closeMap();

   //컴컴컴컴컴컴컴컴컴컴 Volume Support Members
   VolumeItem* locate( DWORD fileName );
   VolumeItem* locate( const char *pFileName );
   Int32 getSizeVol( const char *pFileName ) const;
   Int32 getSizeVol( DWORD fileName ) const;

   static Vector<VolumeItem> *sortVolumeItems;
   static int __cdecl compareItemIDs(const void *_a, const void *_b);
   static int __cdecl compareItemStrings(const void *_a, const void *_b);

   void sortDictionary();

public:
	VolumeStream();
	virtual ~VolumeStream();

   StreamStatus getStatus() const;

   virtual Bool open( const char *pFileName, bool checkDiskFirst=true   ) = 0;
	virtual Bool open( DWORD fileName) = 0;
	virtual StreamIO* openStream( const char *pFileName, bool checkDiskFirst=true );
	virtual StreamIO* openStream( DWORD fileName );
	virtual void close() = 0;

   void* lock()  { return (NULL); }
   void* lock( const char *pFileName );
   void* lock( DWORD fileName );

   virtual Int32 getSize( const char *pFileName ) const;
   virtual Int32 getSize( DWORD fileName ) const;
   Int32 getSize() const;

   bool  findMatches( FindMatch *pFM, bool caseSensitive = FALSE );
};


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 Read/Write Volume Stream
//
class VolumeRWStream: public VolumeStream
{
private:
   UInt32 MAX_STRING_TABLE_SIZE;
   VolumeItem *pNewFile;

   FileRWStream   fileRWStream;
   RLEWStream     rleWStream;
   LZHWStream     lzhWStream;

   Bool open( VolumeItem *pID, StreamCompressType compressType, Int32 size );
   Bool open( VolumeItem *pEntry );

public:
	VolumeRWStream();
	~VolumeRWStream();

   //컴컴컴컴컴컴컴컴컴컴 Volume Management Members
	Bool createVolume(const char *pVolumeFileName);
	Bool openVolume(const char *pVolumeFileName);
	Bool closeVolume();
	Bool compactVolume(const char *pNewVolumeFileName);

   void dispose( const char *pFileName );
	void dispose( DWORD fileName);

   //컴컴컴컴컴컴컴컴컴컴 Stream Management Members
	Bool open( const char *pFileName, StreamCompressType compressType, Int32 size = 0 );
	Bool open( DWORD fileName, StreamCompressType compressType, Int32 size = 0 );
	Bool open( const char *pFileName, bool checkDiskFirst=true  );
	Bool open( DWORD fileName );
	void close();

	Int32 getCapabilities(StreamCap) const;
	Int32 getPosition() const;
	Bool setPosition(Int32);
	Bool flush();

	using VolumeStream::write;
	using VolumeStream::read;

	Bool write(int size,const void* d);
	Bool read (int size,void* d);
};


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴 Read-Only Volume Stream
class VolumeRStream: public VolumeStream
{
private:
   Bool open( VolumeItem *pEntry );
   FileRStream fileRStream;
   struct SearchPath
   {
      char *ext;
      char *path;
   };
   VectorPtr<SearchPath*> paths;

   Bool openFromDisk( const char *pFileName );
	Bool openFromDisk( DWORD fileName);

   Int32 getSizeDisk( const char *pFileName ) const;
   Int32 getSizeDisk( DWORD fileName ) const;

   const char* getSearchPath(const char *file);

public:
	VolumeRStream();
	~VolumeRStream();

   //컴컴컴컴컴컴컴컴컴컴 Volume Management Members
	Bool openVolume(const char *pVolumeFileName);
	Bool closeVolume();
   void setSearchPath(const char *path, const char *ext=NULL);

   //컴컴컴컴컴컴컴컴컴컴 Stream Management Members
	Bool open( const char *pFileName, bool checkDiskFirst=true );
	Bool open( DWORD fileName );
	StreamIO* openStream( const char *pFileName, bool checkDiskFirst=true  );
	StreamIO* openStream( DWORD filename );
	void close();

	Int32 getCapabilities(StreamCap) const;
	Int32 getPosition() const;
	Bool setPosition(Int32);

   using VolumeStream::getSize;
   Int32 getSize( const char *pFileName ) const;
   Int32 getSize( DWORD fileName ) const;

	using VolumeStream::read;

	Bool write(int size,const void* d);
	Bool read (int size,void* d);
};

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif
 
#endif //_VOLSTRM_H_
