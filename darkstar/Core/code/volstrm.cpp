//--------------------------------------------------------------------------------
//   
// Description 
//   
// $Workfile:   volstrm.cpp  $
// $Revision:   2.0  $
// $Author  $
// $Modtime $
//
//--------------------------------------------------------------------------------

#include "streams.h"
#include <stdlib.h>
#include "m_qsort.h"

#define VOL_FILE_ID     FOURCC('P','V','O','L')
#define VOL_FILE_OLDID  FOURCC('V','O','L',' ')
#define VOLUME_IDS      FOURCC('v','o','l','i')    //  Volume id dictionary
#define VOLUME_STRING   FOURCC('v','o','l','s')    //  Volume string table
#define VOLUME_FILE     FOURCC('V','B','L','K')    //  volume file block


char* DWORD_TO_ASCII(DWORD lName, char *ascii)
{
   DWORD nibble;

   for (int i=0; i<8; i++)
   {
      nibble = lName;
      nibble >>= (32-4);
      lName  <<= 4;
      if (nibble < 10) ascii[i] = char(nibble+'0');
      else 				  ascii[i] = char(nibble+'A'-10);
   }
   ascii[8] = '.';
   ascii[9] = '*';
   ascii[10]= 0x0;
   return ( ascii );
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
// Base Volume Stream
//
//--------------------------------------------------------------------------------
// NAME 
//   VolumeStream::VolumeStream()
//   
// DESCRIPTION 
//   Base Volume Constructor
//   
// ARGUMENTS 
//   none
//   
// RETURNS 
//   none
//   
// NOTES 
//   
//--------------------------------------------------------------------------------
VolumeStream::VolumeStream()
{
   rawStringData     = NULL;
   rawStringDataSize = 0;
   stringBlockOffset = 4;

   base              = NULL;
   hFileMap          = NULL;
   hFile             = INVALID_HANDLE_VALUE;
   userStream        = NULL;
}


//--------------------------------------------------------------------------------
// NAME 
//   VolumeStream::~VolumeStream()
//   
// DESCRIPTION 
//   Base volume destructor
//   
// ARGUMENTS 
//   none
//   
// RETURNS 
//   none
//   
// NOTES 
//   
//--------------------------------------------------------------------------------

VolumeStream::~VolumeStream()
{
   closeFile();
   closeMap();
}


//--------------------------------------------------------------------------------
// NAME 
//   Bool VolumeStream::openFile(const char *pVolumeFileName)
//   
// DESCRIPTION 
//   INTERNAL USE ONLY
//   Opens a file with the correct privileges for later file-mapping
//   
// ARGUMENTS 
//   pVolumeFileName - name of file to open
//   
// RETURNS 
//   true  - if open was successful
//   false - otherwise
//   
// NOTES 
//   
//--------------------------------------------------------------------------------
Bool VolumeStream::openFile(const char *pVolumeFileName)
{
   if (readWrite == true) {
      AssertISV(ResourceManager::sm_pManager == NULL ||
                ResourceManager::sm_pManager->isValidWriteFileName(pVolumeFileName) == true,
                avar("Attempted write to file: %s.\n"
                     "File is not in a writable directory.", pVolumeFileName));
   }

   closeFile();
	//Attempt to open the file
	hFile = CreateFile( 
					pVolumeFileName, 							//name of file to open
					(readWrite) ? GENERIC_WRITE|GENERIC_READ : GENERIC_READ, 
					FILE_SHARE_READ, 
					NULL, 
      			(readWrite) ? OPEN_ALWAYS : OPEN_EXISTING, 
      			FILE_ATTRIBUTE_NORMAL, 
      			NULL);

	if (hFile != INVALID_HANDLE_VALUE)
      return (GetFileSize(hFile, NULL) != 0);

	//failed so cleanup
   closeFile();
   return ( false );
}


//--------------------------------------------------------------------------------
// NAME 
//   void VolumeStream::closeFile()
//   
// DESCRIPTION 
//   INTERNAL USE ONLY
//   Close a file.  If the size has changed adjust the EOF to reflect it.
//   
// ARGUMENTS 
//   none
//   
// RETURNS 
//   none
//   Creates a properly flushed and sized file.
//   
// NOTES 
//   
//--------------------------------------------------------------------------------
void VolumeStream::closeFile()
{
   closeMap();
	if ( hFile != INVALID_HANDLE_VALUE ) CloseHandle( hFile );
   hFile = INVALID_HANDLE_VALUE;
}


//--------------------------------------------------------------------------------
// NAME 
//   Bool VolumeStream::createMap(Int32 size)
//   
// DESCRIPTION 
//   Creates a new file-mapping.
//   If the file was Read/Write the map is read/Write
//   The map starts at file position 0 and extends 'size' bytes.
//   
// ARGUMENTS 
//   size - size of map to create
//   
// RETURNS 
//   true  - if map created successfullly 
//   false - otherwise
//   
// NOTES 
//   if size > fileSize the fileSize is adjusted
//   
//--------------------------------------------------------------------------------
Bool VolumeStream::createMap(Int32 size)
{
   if ( hFile == INVALID_HANDLE_VALUE ) return ( false );
   int fileSize = ( size == -1 ) ? GetFileSize(hFile, NULL) : size;

   closeMap();

	//Attempt to create the file mapping
	hFileMap = CreateFileMapping(
	         hFile,								//handle of file
	         NULL,                      	//default security
	         (readWrite) ? PAGE_READWRITE : PAGE_READONLY,   //page level access priv.
	         0,                         	//high 32bits of size
				fileSize,							//low 32bits of size
	         NULL);                     	//don't name the file-map

	if (hFileMap != NULL) 
	{
		//Create pointer to file mapping
	   base = (BYTE*)MapViewOfFile(
	            hFileMap,					//Memory Mapped File Handle
	            (readWrite) ? FILE_MAP_WRITE : FILE_MAP_READ, //Protection
	            0,                      //high 32bit offset into file
	            0,                      //low 32bit offset into file
	            0);                     //length of map in bytes

		if (base != NULL) 
		{
         memRWStream.open(fileSize, base);
         return ( true );
		}
		else
   		AssertWarn(0, "VolumeStream::createMap: MapViewOfFile failed." );
	}
	else
   	AssertWarn(0, "VolumeStream::createMap: CreateFileMapping failed." );

	//failed so cleanup
	closeMap();
	return ( false );
}


//--------------------------------------------------------------------------------
// NAME 
//   void VolumeStream::closeMap()
//   
// DESCRIPTION 
//   close a file mapping
//   
// ARGUMENTS 
//   none
//   
// RETURNS 
//   none
//   
// NOTES 
//   
//--------------------------------------------------------------------------------
void VolumeStream::closeMap()
{
   memRWStream.close();
	if ( base     != NULL ) UnmapViewOfFile( base );
	if ( hFileMap != NULL ) CloseHandle( hFileMap );
   hFileMap = NULL;
   base     = NULL;
}


//--------------------------------------------------------------------------------
// NAME 
//   IDDictionary* VolumeStream::locate( DWORD fileName )
//   
// DESCRIPTION 
//   Given a DWORD ID attempt to locate its dictionary entry
//   
// ARGUMENTS 
//   fileName - DWORD filename ID
//   
// RETURNS 
//   pointer to a dictionary entry if found successfully
//   NULL otherwise
//   
// NOTES 
//   
//--------------------------------------------------------------------------------
VolumeItem * VolumeStream::locate( DWORD fileName )
{
   register Int32  lo, hi, index;
   VolumeItem *pEntry;

   if (idSortedItems.size())
   {
      //do a little binary search
      lo = 0;                                //head of array
      hi = idSortedItems.size() - 1;                      //tail of array
   
      while (lo <= hi)
      {
         index = (lo+hi) >> 1;               //get midpoint, round down
         pEntry = &volumeItems[idSortedItems[index]];
         if (fileName == pEntry->ID) 
            return (pEntry);                 //FOUND, outta here
         else                                //no, we'll lets zero in on it
         {   
         	if (fileName < pEntry->ID) 
         		hi = index-1;
            else lo = index+1;
   	   }
      }
   }
   return ( NULL );   
}


//--------------------------------------------------------------------------------
// NAME 
//   IDDictionary* VolumeStream::locate( const char *pFileName )
//   
// DESCRIPTION 
//   Given a string attempt to locate its dictionary entry
//   
// ARGUMENTS 
//   pFileName - an ascii string 
//   
// RETURNS 
//   pointer to a dictionary entry if found successfully
//   NULL otherwise
//   
// NOTES 
//   
//--------------------------------------------------------------------------------
VolumeItem * VolumeStream::locate( const char *pFileName )
{
   register Int32  lo, hi, index;
   VolumeItem *pEntry;
   Int32       compare;

   if (stringSortedItems.size())
   {
      //do a little binary search
      lo = 0;                                //head of array
      hi = stringSortedItems.size() - 1;     //tail of array
   
      while (lo <= hi)
      {
         index = (lo+hi) >> 1;               //get midpoint, round down
         pEntry = &volumeItems[stringSortedItems[index]];
         compare = stricmp(pFileName, pEntry->string);
			if( compare < 0 )
			   hi = index-1;    // pEntry is greater         
			else if( compare > 0 )
            lo = index+1;    // pEntry is less
			else
            return (pEntry);        // FOUND, outta here!
      }
   }
   return ( NULL );   
}


//--------------------------------------------------------------------------------
// NAME 
//   void* VolumeStream::lock( const char *pFileName )
//   
// DESCRIPTION 
//   Return a pointer to a file block
//   
// ARGUMENTS 
//   pFileName - file name to attempt to acquire a lock on
//   
// RETURNS 
//   void* - if file exists and lock is successful
//   NULL  - otherwise
//   
// NOTES 
//   On a RW stream do not create/open any new blocks while operating
//   on a locked file because the locked
//   
//--------------------------------------------------------------------------------
void* VolumeStream::lock( const char *pFileName )
{
   VolumeItem *pID;
   if ( base )
   {
      pID = locate( pFileName );
      if ( pID && pID->size )
      {
         openSize = pID->size;
         return ( base+(pID->blockOffset+sizeof(DWORD)+sizeof(DWORD)) );
         //compensate for block header (8 bytes)
      }
   }
   openSize = 0;
   return ( NULL );
}


//--------------------------------------------------------------------------------
// NAME 
//   void* VolumeStream::lock( DWORD fileName )
//   
// DESCRIPTION 
//   Return a pointer to a file block
//   
// ARGUMENTS 
//   
//   
// RETURNS 
//   void* - if file exists and lock is successful
//   NULL  - otherwise
//   
// NOTES 
//   
//--------------------------------------------------------------------------------
void* VolumeStream::lock( DWORD fileName )
{
   VolumeItem *pID;
   if ( base )
   {
      pID = locate( fileName );
      if ( pID && pID->size )
      {
         openSize = pID->size;
         return ( base + ( pID->blockOffset+sizeof(BlockStream::Header) ) );
         //compensate for block header (8 bytes)
      }
   }
   openSize = 0;
   return ( NULL );
}


//--------------------------------------------------------------------------------
// NAME 
//   Int32 VolumeStream::getSizeVol( const char *pFileName )
//   
// DESCRIPTION 
//   Get the size of a volume file
//   The size IS NOT  the size as stored in the volume but the actual file size.
//   If compressed the size is the uncompressed size.
//   
// ARGUMENTS 
//   pFileName - name of file you want the size of
//   
// RETURNS 
//   size of the file
//   or -1 on failure
//   
// NOTES 
//   locate() isn't actually const, but we know it doesn't change anything, and
//  this is a logically const action
//--------------------------------------------------------------------------------
Int32 VolumeStream::getSizeVol( const char *pFileName ) const
{
   VolumeItem* p = const_cast<VolumeStream*>(this)->locate( pFileName );
   if ( p )
      return ( p->size );
   else 
      return ( -1 );
}


//--------------------------------------------------------------------------------
// NAME 
//   Int32 VolumeStream::getSizeVol( DWORD fileName )
//   
// DESCRIPTION 
//   Get the size of a volume file
//   The size IS NOT  the size as stored in the volume but the actual file size.
//   If compressed the size is the uncompressed size.
//   
// ARGUMENTS 
//   filename - DWORD filename ID
//   
// RETURNS 
//   size of the file
//   or -1 on failure
//   
// NOTES 
//   locate() isn't actually const, but we know it doesn't change anything, and
//  this is a logically const action
//--------------------------------------------------------------------------------
Int32 VolumeStream::getSizeVol( DWORD fileName ) const
{
   VolumeItem* p = const_cast<VolumeStream*>(this)->locate( fileName ); 
   if ( p )
      return ( p->size );
   else 
      return ( -1 );
}


//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
// Read/Write Volume Stream
//
VolumeRWStream::VolumeRWStream()
{
   pNewFile  = NULL;
   readWrite = true;   
}


VolumeRWStream::~VolumeRWStream()
{
   closeVolume();   
}

Bool VolumeRWStream::compactVolume(const char *pNewVolumeFileName)
{
   VolumeRWStream vs;

   if(!volumeItems.size())
      // this volume is empty
      return(false);

   if(!vs.openVolume(pNewVolumeFileName))
      // unable to create/open target volume
      return(false);

   if (vs.volumeItems.size())
      // target volume already contains stuff!
      return(false);

   for(int i = 0; i < volumeItems.size(); i++)
   {
      VolumeItem &item = volumeItems[i];

      if(item.string)
      {
         open(item.string);
         vs.open(item.string, 
                 StreamCompressType(item.compressType),
                 item.size);
      }
      else
      {
         open(item.ID);
         vs.open(item.ID, 
                 StreamCompressType(item.compressType),
                 item.size);
      }
      int dataSize;
      char *buf = new char[item.size];
	   if (item.compressType == STRM_COMPRESS_NONE) 
	   {
	      dataSize = item.size;
	      read(dataSize, (void *)buf);
	   }
	   else 
	   {
	       char *bptr = buf;
	       while(read(bptr++))
	       	;
          dataSize = bptr - buf;
       }
      vs.write(dataSize, (void *) buf);
      delete [] buf;

      // Make sure there were no errors during the transfer.
	  if (vs.getStatus() != STRM_OK || 
         (getStatus() != STRM_OK && getStatus() != STRM_EOS)) {
         vs.close();
		 close();
		 return FALSE;
      }
      
	  vs.close();
      close();
   }
   return TRUE;
}

//컴컴컴컴컴컴컴컴컴컴 Volume Management Members
//--------------------------------------------------------------------------------
// NAME 
//   Bool VolumeRWStream::openVolume(const char *pVolumeFileName, Int32 dictionarySize, Int32 stringTableSize)
//   
// DESCRIPTION 
//   Open and create a new volume
//   
// ARGUMENTS 
//   pVolumeFileName - filename of volume to be created
//   dictionarySize  - number of dictionary entries to reserve
//   stringTableSize - number of bytes to reserve for the string table ( 0 for no string table)
//   
// RETURNS 
//   true  - if volume sucessfully created
//   false - otherwise
//   
// NOTES 
//   
//--------------------------------------------------------------------------------
Bool VolumeRWStream::createVolume(const char *pVolumeFileName)
{ 
   FileWStream  rwFil;
   BlockRWStream rwSub;

   closeVolume();
   if ( rwFil.open( pVolumeFileName ) )
   {
      // zap anything that may have been in this file
      rwFil.setPosition(0);
      SetEndOfFile(rwFil.getHandle());

      // write the volume file identifier, "PVOL" in the first four bytes
      rwFil.write(VOL_FILE_ID);

      // write the offset of the string block, this is initially eight since
      // the string block will occur next (after this DWORD)
      rwFil.write((UInt32)8);

      // create a string block, initially with size of zero
      rwSub.open(rwFil, VOLUME_STRING, 0, false);
      rwSub.close();

      // create a volume item block initially with size of zero
      rwSub.open(rwFil, VOLUME_IDS, 0, false);
      rwSub.close();
   
      rwFil.close();
      return(openVolume(pVolumeFileName));
   }
   return(false); 
}



//--------------------------------------------------------------------------------
// NAME 
//   Bool VolumeRWStream::openVolume(const char *pVolumeFileName)
//   
// DESCRIPTION 
//   Open an existing volume
//   
// ARGUMENTS 
//   pVolumeFileName - filename of volume to be open
//   
// RETURNS 
//   true  - on sucessful open
//   false - otherwise
//   
// NOTES 
//   
//--------------------------------------------------------------------------------
Bool VolumeRWStream::openVolume(const char *pVolumeFileName)
{
   Bool ok = true;
   BlockRStream rSub;

   closeVolume();
   if (!openFile(pVolumeFileName))
   {
      if (hFile == INVALID_HANDLE_VALUE)
         // error occured during file open
         return(false);
      else
      {
         // file opened ok, but was empty, so lets create a new volume
         closeFile();
         return(createVolume(pVolumeFileName));
      }
   }
   else
   {
      // file was opened ok
      if (createMap())
      {
         // rewind to the beginning of the file
         memRWStream.setPosition(0);

         // read the volume file identifier (should be "PVOL")
         DWORD identifier;
         memRWStream.read(&identifier);

         // Assert if this is an old volume
   		AssertWarn(identifier != VOL_FILE_OLDID, 
   		           "VolumeRWStream::openVolume: Attempt to open volume of obsolete format.");

         // make sure this is a volume file
         if (identifier != VOL_FILE_ID)
         {
            closeFile();
            return(false);
         }

         // make sure this is a volume file
         if (identifier != VOL_FILE_ID)
         {
            closeFile();
            return(false);
         }

         // read the location of the string block (which is just after the data)
         stringBlockOffset = 8;
         memRWStream.read(&stringBlockOffset);

         // fast forward to the strings block, this comes after the
         // data size item and the volume data
         memRWStream.setPosition(stringBlockOffset);
         
         //
         // Read the strings into the raw string area
         //
         if (rSub.open(memRWStream, VOLUME_STRING))
         {
            rawStringDataSize = rSub.getDataSize();
            if (rawStringDataSize)
            {
               rawStringData = new char[rawStringDataSize];
               rSub.read(rawStringDataSize, rawStringData);
            }
            else
               rawStringData = NULL;
         }
         else
            // there should AT LEAST be a block with no data
            ok = false;

         //
         // read the IDs into the raw VolumeItem area
         //
         if (rSub.open(memRWStream, VOLUME_IDS))
         {
            Int32 size = rSub.getDataSize()/sizeof(VolumeItem);
            volumeItems.setSize(size);
            if (size)
               rSub.read(rSub.getDataSize(), volumeItems.address());
            rSub.close();

            // change volume item strings from offsets to actual
            // memory addresses
            for (int i = 0; i < size; i++)
            {
               if ((Int32)volumeItems[i].string == -1 || !rawStringData)
                  // no string associated with this item
                  volumeItems[i].string = NULL;
               else
                  // change from zero-based offset to address
                  volumeItems[i].string += (int)rawStringData;
            }
         }
         else
            // there should AT LEAST be a block with no data
            ok = false;

         rSub.close();
         sortDictionary();

         return(ok);
      }
   } 

   closeVolume();
   return(false);
}


//--------------------------------------------------------------------------------
// NAME 
//   Bool VolumeRWStream::closeVolume()
//   
// DESCRIPTION 
//   Close the volume
//   
// ARGUMENTS 
//   none
//   
// RETURNS 
//   
//   
// NOTES 
//   
//--------------------------------------------------------------------------------
Bool VolumeRWStream::closeVolume()
{ 
   close();

   if (hFile == INVALID_HANDLE_VALUE)
      return(false);
   closeMap();
   fileRWStream.setHandle( hFile );


   // write the string block offset just after the volume file 
   // identifier ("PVOL")
   fileRWStream.setPosition(4);
   fileRWStream.write(stringBlockOffset);

   // jump just past the volume data
   fileRWStream.setPosition(stringBlockOffset);

   //
   // write the strings block
   //
   BlockRWStream stringsBlock;
   char *string;
   int   offset = 0, i;

   stringsBlock.open(fileRWStream, VOLUME_STRING, 0, false);
   for (i = 0; i < volumeItems.size(); i++)
   {
      // change the volumeItem's string to a zero based offset in the
      // file, or -1 if the item doesn't have a string
      string = volumeItems[i].string;
      if (string)
      {
         volumeItems[i].string = (char *)offset;
         stringsBlock.write(strlen(string) + 1, string);
         offset += strlen(string) + 1;
      }
      else
         volumeItems[i].string = (char *)-1;

      // if the string was not in the raw data block (ie it was added
      // to the volume recently) then delete it, since it was allocated
      // separately from the main block
      if ((string < rawStringData) || (string >= rawStringData + rawStringDataSize))
         delete [] string;
   }
   // delete the main strings block
   delete [] rawStringData;
   rawStringData = NULL;
   stringsBlock.close();

   //
   // write the volume items block
   //
   BlockRWStream itemsBlock;
   itemsBlock.open(fileRWStream, VOLUME_IDS, 0, false);
   for (i = 0; i < volumeItems.size(); i++)
      itemsBlock.write(sizeof(VolumeItem), &volumeItems[i]);
   itemsBlock.close();

   // clear out the vectors
   volumeItems.clear();

   closeFile();
   return(true);
}


//--------------------------------------------------------------------------------
// NAME 
//   void VolumeRWStream::dispose( const char *pFileName )
//   
// DESCRIPTION 
//   delete a file from the volume
//   This simply removes the filename it does not attempt to preform any
//   volume compaction
//   
// ARGUMENTS 
//   pFileName - name of file to delete
//   
// RETURNS 
//   
//   
// NOTES 
//   
//--------------------------------------------------------------------------------
void VolumeRWStream::dispose(const char *pFileName)
{ 
   VolumeItem *item = locate(pFileName);
   if (item)
   {
      volumeItems.erase(item);
      sortDictionary();
   }
}


//--------------------------------------------------------------------------------
// NAME 
//   void VolumeRWStream::dispose( DWORD fileName )
//   
// DESCRIPTION 
//   delete a file from the volume
//   This simply removes the filename it does not attempt to preform any
//   volume compaction
//   
// ARGUMENTS 
//   fileName - DWORD fileanme ID to delete
//   
// RETURNS 
//   
//   
// NOTES 
//   
//--------------------------------------------------------------------------------
void VolumeRWStream::dispose( DWORD fileName )
{ 
   VolumeItem *item = locate(fileName);
   if (item)
   {
      volumeItems.erase(item);
      sortDictionary();
   }
}



//--------------------------------------------------------------------------------
// NAME 
//   int VolumeRWStream::compareEntries(const void *_a, const void *_b)
//   
// DESCRIPTION 
//   Support routine used by 'sortDirectory'
//   compares two dictionary entries and sorts them based on filename
//   
//   strings are ALWAYS less than IDs
//   
// ARGUMENTS 
//   
//   
// RETURNS 
//   
//   
// NOTES 
//   
//--------------------------------------------------------------------------------
int __cdecl VolumeStream::compareItemIDs(const void *a, const void *b)
{               
   int i = *(int *)a;
   int j = *(int *)b;

   if ((*sortVolumeItems)[i].ID < (*sortVolumeItems)[j].ID)
      return(-1);
   else if ((*sortVolumeItems)[i].ID > (*sortVolumeItems)[j].ID)
      return(1);
   else 
      return(0);
}
 
int __cdecl VolumeStream::compareItemStrings(const void *a, const void *b)
{
   char *s = (*sortVolumeItems)[*(int *)a].string;
   char *t = (*sortVolumeItems)[*(int *)b].string;

   if (s && t)
      // both items have strings, return comparison of them
      return(stricmp(s, t));
   else if (s)
      // only a has a string, so b is less
      return(1);
   else if (t)
      // only b has a string, so a is less
      return(-1);
   else
      // neither have strings, directly compare ids
      return(compareItemIDs(a, b));
}

//--------------------------------------------------------------------------------
// NAME 
//   void VolumeRWStream::sortDictionary()
//   
// DESCRIPTION 
//   Sort the ID Dictionary by filename
//   
// ARGUMENTS 
//   
//   
// RETURNS 
//   
//   
// NOTES 
//   
//--------------------------------------------------------------------------------

Vector<VolumeItem> *VolumeStream::sortVolumeItems = NULL;

void VolumeStream::sortDictionary()
{
   idSortedItems    .clear();
   stringSortedItems.clear();
   for (int i = 0; i < volumeItems.size(); i++)
   {
      if (volumeItems[i].ID)
         idSortedItems.push_back(i);
      if (volumeItems[i].string)
         stringSortedItems.push_back(i);
   }
   sortVolumeItems = &volumeItems;
   m_qsort(idSortedItems    .address(), idSortedItems.size(),     sizeof(int), compareItemIDs);
   m_qsort(stringSortedItems.address(), stringSortedItems.size(), sizeof(int), compareItemStrings);
}



//컴컴컴컴컴컴컴컴컴컴 Stream Management Members
//--------------------------------------------------------------------------------
// NAME 
//   Bool VolumeRWStream::open( DWORD fileName, StreamCompressType compressType, Int32 size )
//   
// DESCRIPTION 
//   Open a file in the volume
//   
//   If the file exists, the compress is none and the size is the same, open will
//   open the existing volume file.  Otherwise a new volume file will be created
//   
// ARGUMENTS 
//   fileName    - DWORD filename ID to open
//   compressType- streamIO compression type
//   size        - length of file ( 0 can be used if size is unknown)
//   
// RETURNS 
//   true if file sucessfully opened
//   false otherwise
//   
// NOTES 
//   
//--------------------------------------------------------------------------------
Bool VolumeRWStream::open( DWORD fileName, StreamCompressType compressType, Int32 size )
{ 
   close();
   VolumeItem *pID;

   //if the block exists, is uncompressed and is the same size... use it
   if ( compressType ==  STRM_COMPRESS_NONE )
   {
      pID = locate( fileName );
      if ( pID && (UInt32(size) == pID->size) )
         return ( open( pID ) );
   }

   closeMap();
   dispose(fileName);

   volumeItems.setSize(volumeItems.size() + 1);
   pID = &(volumeItems[volumeItems.size() - 1]);
   pID->ID           = fileName;
   pID->string       = NULL;
   pID->blockOffset  = stringBlockOffset;
   pID->size         = size;
   pID->compressType = compressType;
   sortDictionary(); 
   return(open(pID, compressType, size));
}


//--------------------------------------------------------------------------------
// NAME 
//   Bool VolumeRWStream::open( const char *pFileName, StreamCompressType compressType, Int32 size )
//   
// DESCRIPTION 
//   Open a file in the volume
//   
//   If the file exists, the compress is none and the size is the same, open will
//   open the existing volume file.  Otherwise a new volume file will be created
//   
// ARGUMENTS 
//   pFileName   - ascii filename to open
//   compressType- streamIO compression type
//   size        - length of file ( 0 can be used if size is unknown)
//   
// RETURNS 
//   true if file sucessfully opened
//   false otherwise
//   
// NOTES 
//   
//--------------------------------------------------------------------------------
Bool VolumeRWStream::open( const char *pFileName, StreamCompressType compressType, Int32 size )
{ 
   close();
   VolumeItem *pID;

   //if the block exists, is uncompressed and is the same size... use it
   if ( compressType ==  STRM_COMPRESS_NONE )
   {
      pID = locate( pFileName );
      if ( pID && (UInt32(size) == pID->size) )
         return ( open( pID ) );
   }

   closeMap();
   dispose( pFileName );

   volumeItems.setSize(volumeItems.size() + 1);
   pID = &(volumeItems[volumeItems.size() - 1]);
   pID->ID           = 0;
   pID->string       = new char[strlen(pFileName) + 1];
   pID->blockOffset  = stringBlockOffset;
   pID->size         = size;
   pID->compressType = compressType;
   strcpy(pID->string, pFileName);
   sortDictionary(); 
 
   return(open(pID, compressType, size));
}


//--------------------------------------------------------------------------------
// NAME 
//   Bool VolumeRWStream::open( IDDictionary *pID, StreamCompressType compressType, Int32 size )
//   
// DESCRIPTION 
//   INTERNAL
//   Sets up the streams for open/create requests
//   
// ARGUMENTS 
//   
//   
// RETURNS 
//   
//   
// NOTES 
//   
//--------------------------------------------------------------------------------
Bool VolumeRWStream::open(VolumeItem *pID, StreamCompressType compressType, Int32 size)
{
   close();
   if ( pID )
   {
      pNewFile = pID;
      fileRWStream.setHandle( hFile );
      fileRWStream.setPosition(pID->blockOffset);
      blockRWStream.open( fileRWStream, VOLUME_FILE, size, true );
      //attach any compression filers if necessary
      switch( compressType )
      {
	      case STRM_COMPRESS_NONE:
            userStream = &blockRWStream;
            break;

	      case STRM_COMPRESS_RLE:
            rleWStream.attach( blockRWStream );
            userStream = &rleWStream;
            break;

	      case STRM_COMPRESS_LZH:
            lzhWStream.attach( blockRWStream );
            userStream = &lzhWStream;
            break;
      }
      openSize = size;
      return ( userStream->getStatus() == STRM_OK );
   }
   return ( false );
}


//--------------------------------------------------------------------------------
// NAME 
//   Bool VolumeRWStream::open( IDDictionary *pID )
//   
// DESCRIPTION 
//   INTERNAL
//   Sets up the streams for open existing requests
//   
// ARGUMENTS 
//   
//   
// RETURNS 
//   
//   
// NOTES 
//   
//--------------------------------------------------------------------------------
Bool VolumeRWStream::open( VolumeItem *pID )
{ 
   close();
   if ( pID ) 
   {
      if ( !base ) 
         if (!createMap())
            return ( false );

      BlockStream::Header *hdr = (BlockStream::Header*)(base + pID->blockOffset);
      memRWStream.open( hdr->size & ~ALIGN_DWORD, hdr+1 );
      
      //attach any compression filers if necessary
      switch( pID->compressType )
      {
	      case STRM_COMPRESS_NONE:
            userStream = &memRWStream;
            break;

	      case STRM_COMPRESS_RLE:
            rleRStream.attach( memRWStream );
            userStream = &rleRStream;
            break;

	      case STRM_COMPRESS_LZH:
            lzhRStream.attach( memRWStream );
            userStream = &lzhRStream;
            break;
      }
      openSize = hdr->size & ~ALIGN_DWORD;
      return ( userStream->getStatus() == STRM_OK );
   }
   return false; 
}


//--------------------------------------------------------------------------------
// NAME 
//   void VolumeRWStream::close()
//   
// DESCRIPTION 
//   Close a volume file
//   
// ARGUMENTS 
//   none
//   
// RETURNS 
//   none
//   
// NOTES 
//   
//--------------------------------------------------------------------------------
void VolumeRWStream::close()
{ 
   openSize = 0;
   if (userStream && (userStream->getStatus() == STRM_OK ||
   		userStream->getStatus() == STRM_EOS) )
   {
      userStream->flush();
      if ( pNewFile )
      {
         if (!pNewFile->size) 
            pNewFile->size = userStream->getPosition();     //if undefined file size update the size
         userStream->close();
         blockRWStream.close();
         pNewFile = NULL;
         stringBlockOffset += blockRWStream.getBlockSize();
         sortDictionary();
      }
      else
      {
         userStream->close();
         blockRWStream.close();
      }
   }
}


Int32 VolumeRWStream::getCapabilities(StreamCap cap) const
{ 
   if ( userStream )
      return ( userStream->getCapabilities( cap ) );
   AssertFatal(0, "VolumeRWStream::getCapabilities: return value undefined when no file is open.");
   return 0; 
}


Int32 VolumeRWStream::getPosition() const
{ 
   if ( userStream )
      return ( userStream->getPosition() );
   AssertFatal(0, "VolumeRWStream::getPosition: return value undefined when no file is open.");
   return 0; 
}


Bool VolumeRWStream::setPosition(Int32 position)
{
   if ( userStream )
      return ( userStream->setPosition( position ) );
   return false; 
}


Bool VolumeRWStream::flush()
{ 
   if ( userStream )
      return ( userStream->flush() );
   return false; 
}


Bool VolumeRWStream::write(int size,const void* d)
{ 
   if ( userStream )
      return ( userStream->write(size, d) );
   return false; 
}


Bool VolumeRWStream::read(int size,void* d)
{ 
   if ( userStream )
      return ( userStream->read(size, d) );
   return false; 
}






//컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴
// Read-Only Volume Stream
//
VolumeRStream::VolumeRStream()
{ 
   readWrite   = false;
}

VolumeRStream::~VolumeRStream()
{ 
   closeVolume();   

   // delete the search paths
   VectorPtr<SearchPath*>::iterator i= paths.begin();
   for ( ; i != paths.end() ; i++)
   {
      delete [] (*i)->ext;
      delete [] (*i)->path;
      delete (*i);
   }
}


//컴컴컴컴컴컴컴컴컴컴 Volume Management Members
//--------------------------------------------------------------------------------
// NAME 
//   Bool VolumeRStream::openVolume(const char *pVolumeFileName)
//   
// DESCRIPTION 
//   Open a read-only volume stream
//   
// ARGUMENTS 
//   pVolumeFileName - an ascii volume file to open
//   
// RETURNS 
//   true  - if volume scream is sucessfully opened
//   false - otherwise
//   
// NOTES 
//   
//--------------------------------------------------------------------------------
Bool VolumeRStream::openVolume(const char *pVolumeFileName)
{ 
   BlockRStream rSub;

   closeVolume();
   if (openFile( pVolumeFileName ) )
   {
      if ( createMap() )
      {
         Bool ok = true;

         // rewind to the beginning
	      memRWStream.setPosition(0);

        // read the volume file identifier
         DWORD identifier;
         memRWStream.read(&identifier);

         // Assert if this is an old volume
   		AssertWarn(identifier != VOL_FILE_OLDID, 
   		           "VolumeRStream::openVolume: Attempt to open volume of obsolete format.");

          // make sure this is a volume file
         if (identifier != VOL_FILE_ID)
         {
            closeFile();
            return(false);
         }

         // read the location of the string block (which is just after the data)
         stringBlockOffset = 8;
         memRWStream.read(&stringBlockOffset);
         memRWStream.setPosition(stringBlockOffset);

         //                      
         // Read the strings into the raw string area
         //
         if (rSub.open(memRWStream, VOLUME_STRING))
         {
            rawStringDataSize = rSub.getDataSize();
            if (rawStringDataSize)
            {
               rawStringData = new char[rawStringDataSize];
               rSub.read(rawStringDataSize, rawStringData);
            }
            else
               rawStringData = NULL;
         }
         else
            // there should AT LEAST be a block with no data
            ok = false;
    
         //
         // read the IDs into the raw VolumeItem area
         //
         if (rSub.open(memRWStream, VOLUME_IDS))
         {
            Int32 size = rSub.getDataSize()/sizeof(VolumeItem);
            if (size)
            {
               volumeItems.setSize(size);
               rSub.read(rSub.getDataSize(), volumeItems.address());
            }
            rSub.close();

            // change volume item strings from offsets to actual 
            // memory addresses
            for (int i = 0; i < size; i++)
            {
               if ((Int32)volumeItems[i].string == -1 || !rawStringData)
                  // no string associated with this item
                  volumeItems[i].string = NULL;
               else
                  // change from zero-based offset to address
                  volumeItems[i].string += (int)rawStringData;
            }
         }
         else
            // there should AT LEAST be a block with no data
            ok = false;
         rSub.close();
         sortDictionary();

         return(ok);
      }
   } 

   closeVolume();
   return(false);
}


//--------------------------------------------------------------------------------
// NAME 
//   Bool VolumeRStream::closeVolume()
//   
// DESCRIPTION 
//   closes a volumes and releases its resources
//   
// ARGUMENTS 
//   none
//   
// RETURNS 
//   none
//   
// NOTES 
//   
//--------------------------------------------------------------------------------
Bool VolumeRStream::closeVolume()
{ 
   closeFile();
   volumeItems.clear();
   delete [] rawStringData;
   rawStringData = NULL;
   return true; 
}

//컴컴컴컴컴컴컴컴컴컴 Stream Management Members

//--------------------------------------------------------------------------------
// NAME 
//   Bool VolumeRStream::open( IDDictionary *pID )
//   
// DESCRIPTION 
//   OPen a file from inside a volume
//   
// ARGUMENTS 
//   pID - an IDDictionary stream pointer
//   
// RETURNS 
//   true  - if file open sucessfully
//   false - otherwise
//   
// NOTES 
//   
//--------------------------------------------------------------------------------
Bool VolumeRStream::open( VolumeItem *pID )
{ 
   close();
   if ( pID ) 
   {
      BlockStream::Header *hdr = (BlockStream::Header*)(base + pID->blockOffset);
      memRWStream.open( hdr->size & ~ALIGN_DWORD, hdr+1 );
      
      //attach any compression filers if necessary
      switch( pID->compressType )
      {
	      case STRM_COMPRESS_NONE:
            userStream = &memRWStream;
            break;

	      case STRM_COMPRESS_RLE:
            rleRStream.attach( memRWStream );
            userStream = &rleRStream;
            break;

	      case STRM_COMPRESS_LZH:
            lzhRStream.attach( memRWStream );
            userStream = &lzhRStream;
            break;
      }
      openSize = pID->size;//hdr->size & ~ALIGN_DWORD;
      return ( userStream->getStatus() == STRM_OK );
   }
   return false; 
}


//------------------------------------------------------------------------------
StreamIO* VolumeRStream::openStream( const char *pFileName, bool checkDiskFirst )
{
   if (checkDiskFirst)
   {
      if( openFromDisk( pFileName ) || open( locate( pFileName ) ) )
         return (userStream);      
   }
   else
   {
      if( open( locate( pFileName ) ) || openFromDisk( pFileName ) )
         return (userStream);      
   }
   return (NULL);
}   

//------------------------------------------------------------------------------
StreamIO* VolumeRStream::openStream( DWORD fileName )
{
   if( openFromDisk( fileName ) || open( locate( fileName ) ) )
      return (userStream);      
   else
      return (NULL);
}   




//--------------------------------------------------------------------------------
// NAME 
//   Bool VolumeRStream::openFromDisk(const char *pFileName)
//   
// DESCRIPTION 
//   Open a file from disk
//   If a search path has been installed look on disk an attempt to open the file
//   
// ARGUMENTS 
//   pFileName - ascii name of file to open
//   
// RETURNS 
//   true  - if file opened sucessfully
//   false - otherwise
//   
// NOTES 
//   
//--------------------------------------------------------------------------------
Bool VolumeRStream::openFromDisk(const char *pFileName)
{
   WIN32_FIND_DATA findData;
   HANDLE hFind;
   char cwd[MAX_PATH+1];
   char buffer[MAX_PATH+1];
   char *directory;
   const char *path = getSearchPath( pFileName );

   if ( !path ) return ( false );
   GetCurrentDirectory( MAX_PATH, cwd );
   do
   {
      directory = buffer;
      while ( *path && *path != ';' )
         *directory++ = *path++;
      *directory = '\0';
      if ( *path ) 
         path++;
      if ( directory != buffer )
         SetCurrentDirectory( buffer );
      
      hFind = FindFirstFile( pFileName, &findData);
      if ( hFind != INVALID_HANDLE_VALUE )
      {
         FindClose( hFind );
         if  (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
         {
            fileRStream.open(pFileName);
            SetCurrentDirectory( cwd );
            userStream = &fileRStream;
            openSize = findData.nFileSizeLow;
            return ( userStream->getStatus() == STRM_OK );
         }
      }
      SetCurrentDirectory( cwd );
   }while ( *path );
   return ( false );
}


//--------------------------------------------------------------------------------
// NAME 
//   Bool VolumeRStream::openFromDisk(DWORD fileName)
//   
// DESCRIPTION 
//   Open a file from disk
//   If a search path has been installed look on disk an attempt to open the file
//   
// ARGUMENTS 
//   fileName - DWORD name of file to open
//   
// RETURNS 
//   true  - if file opened sucessfully
//   false - otherwise
//   
// NOTES 
//   
//--------------------------------------------------------------------------------
Bool VolumeRStream::openFromDisk(DWORD fileName)
{
   char ascii[11];
   return openFromDisk( DWORD_TO_ASCII( fileName, ascii ) );
}


//------------------------------------------------------------------------------
void VolumeRStream::setSearchPath(const char *path, const char *ext)
{
   VectorPtr<SearchPath*>::iterator i= paths.begin();
   SearchPath *sp = NULL;

   // see if we can find an entry for this extension
   for (; i != paths.end() ; i++)
   {
      // are they both NULL 'wildcards' ?
      if ( (*i)->ext == NULL && ext == NULL)   
         break;

      // do the strings match
      if (ext && (*i)->ext && stricmp(ext, (*i)->ext) == 0)
         break;
   }

   if ( i != paths.end() ) 
      sp = *i;

   if ( sp )
   {  
      // modify existing path entry
      delete [] sp->path;
      sp->path = path ? strnew(path) : NULL;

      // allow for paths to be deleted but don't delete the default 
      // path.  Only clear it. The one with a NULL ext.
      if (sp->ext && !path)
      {
         delete [] sp->ext;
         paths.erase( i );
      }
   }
   else
      if (path)
      {
         // create new path entry
         sp = new SearchPath;
         sp->path = path ? strnew(path) : NULL;
         sp->ext  = ext  ? strnew(ext)  : NULL;
         paths.push_front(sp);
      }
}


//------------------------------------------------------------------------------
const char * VolumeRStream::getSearchPath(const char *file)
{

   const char *ext = strrchr( file, '.' );
   if (ext) 
   {
      ext++;
      if (*ext)
      {
         VectorPtr<SearchPath*>::iterator i= paths.begin();
         for (; i != paths.end() ; i++)
         {
            // default path always matches
            if ( (*i)->ext == NULL )   
               return ((*i)->path);
            // look for match
            if (ext && stricmp(ext, (*i)->ext) == 0)
               return ((*i)->path);
         }
      }
   }
   return (NULL);
}   



//--------------------------------------------------------------------------------
// NAME 
//   Int32 VolumeRStream::getSizeDisk( const char *pFileName )
//   
// DESCRIPTION 
//   Get the size of a file on disk
//   
// ARGUMENTS 
//   pFileName - ascii filename to get the size of
//   
// RETURNS 
//   size of file on success
//   -1 on failure
//   
// NOTES 
//   
//--------------------------------------------------------------------------------
Int32 VolumeRStream::getSizeDisk( const char *pFileName ) const
{
   WIN32_FIND_DATA findData;
   HANDLE hFind;
   char cwd[MAX_PATH+1];
   char buffer[MAX_PATH+1];
   char *directory;
   const char *path = const_cast<VolumeRStream*>(this)->getSearchPath( pFileName); 

   if ( path ) {
	   GetCurrentDirectory( MAX_PATH, cwd );
	   do
	   {
	      directory = buffer;
	      while ( *path && *path != ';' )
	         *directory++ = *path++;
	      *directory = '\0';
	      if ( *path ) 
	         path++;
	      if ( directory != buffer )
	         SetCurrentDirectory( buffer );
	      
	      hFind = FindFirstFile( pFileName, &findData);
         if ( hFind != INVALID_HANDLE_VALUE )
   	      FindClose( hFind );
	      SetCurrentDirectory( cwd );
	      if ( hFind != INVALID_HANDLE_VALUE &&
	           (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
	         )
	         return ( findData.nFileSizeLow );
	   }while ( *path );
   }
   return ( -1 );
}


//--------------------------------------------------------------------------------
// NAME 
//   Int32 VolumeRStream::getSizeDisk( DWORD fileName )
//   
// DESCRIPTION 
//   Get the size of a file on disk
//   
// ARGUMENTS 
//   fileName - DWORD filename to get the size of
//   
// RETURNS 
//   size of file on success
//   -1 on failure
//   
// NOTES 
//   
//--------------------------------------------------------------------------------
Int32 VolumeRStream::getSizeDisk( DWORD fileName ) const
{
   char ascii[11];
   return getSizeDisk( DWORD_TO_ASCII( fileName, ascii ) );
}


//--------------------------------------------------------------------------------
// NAME 
//   void VolumeRStream::close()
//   
// DESCRIPTION 
//   Close an open volume stream.
//   
// ARGUMENTS 
//   none
//   
// RETURNS 
//   none
//   
// NOTES 
//   
//--------------------------------------------------------------------------------
void VolumeRStream::close()
{  
   openSize = 0;
   if ( userStream )
   {
      userStream->close();
      blockRWStream.close();
   }
}


Int32 VolumeRStream::getCapabilities(StreamCap cap) const
{ 
   if ( userStream )
      return ( userStream->getCapabilities( cap ) );
   AssertFatal(0, "VolumeRStream::getCapabilities: return value undefined when no file is open.");
   return 0; 
}


Int32 VolumeRStream::getPosition() const
{ 
   if ( userStream )
      return ( userStream->getPosition() );
   AssertFatal(0, "VolumeRStream::getPosition: return value undefined when no file is open.");
   return 0; 
}


Bool VolumeRStream::setPosition(Int32 position)
{ 
   if ( userStream )
      return ( userStream->setPosition( position ) );
   return false; 
}


Bool VolumeRStream::write(int,const void*)
{ 
   AssertFatal(0, "VolumeRStream::write: cannot write to a read-only stream.");
   return false; 
}


Bool VolumeRStream::read(int size,void* d)
{ 
   if ( userStream )
      return ( userStream->read(size, d) );
   return false; 
}

//--------------------------------------------------------------------------------

bool VolumeStream::findMatches( FindMatch *pFM, bool caseSensitive )
{
   for ( VolumeItemVector::iterator i=volumeItems.begin(); i!=volumeItems.end(); i++ )
     pFM->findMatch(i->string, caseSensitive);
   return ( pFM->isFull() );
}   


//--------------------------------------------------------------------------------
// NAME 
//   FindMatch::FindMatch( const char *_expression, Int32 maxNumMatches )
//   
// DESCRIPTION 
//   Class to match regular expressions (file names)
//   only works with '*','?', and 'chars'
//   
// ARGUMENTS 
//   _expression  -  The regular expression you intend to match (*.??abc.bmp)
//   _maxMatches  -  The maximum number of strings you wish to match.
//   
// RETURNS 
//   
// NOTES 
//   
//--------------------------------------------------------------------------------

FindMatch::FindMatch( Int32 _maxMatches )
{
   expression = NULL;
   maxMatches = _maxMatches;
   matchList.reserve( maxMatches );
}   

FindMatch::FindMatch( char *_expression, Int32 _maxMatches )
{
   expression = NULL;
   setExpression( _expression );
   maxMatches = _maxMatches;
   matchList.reserve( maxMatches );
}   

FindMatch::~FindMatch()
{
   delete [] expression;
   matchList.clear();
}   

void FindMatch::setExpression( const char *_expression )
{
   delete []expression;
   expression = strnew((char*)_expression);
   expression = strupr(expression);
}   

bool FindMatch::findMatch( const char *str, bool caseSensitive )
{
   if ( isFull() )
      return false;

   char nstr[512];
   strcpy( nstr,str );
   strupr(nstr);
   if ( isMatch( expression, nstr, caseSensitive ) )
      matchList.push_back( (char*)str );   
   return true;
}   

bool FindMatch::isMatch( const char *exp, const char *str, bool caseSensitive )
{
   const char  *e=exp;
   const char  *s=str;
   bool  match=true;

   while ( match && *e && *s )
   {
      switch( *e )
      {
         case '*':
               e++;
               match = false;
               while( ((s=strchr(s,*e)) !=NULL) && !match )
               {
                  match = isMatch( e, s, caseSensitive );
                  s++;
               }
               return( match );
         case '?':
            e++;
            s++;
            break;
         default:
            if (caseSensitive) match = ( *e++ == *s++ );
            else match = ( toupper(*e++) == toupper(*s++) );
            
            break;
      }
   }

   if (*e != *s) // both exp and str should be at '\0' if match was successfull 
      match = false; 

   return ( match );
}   



//------------------------------------------------------------------------------
//-------------------------------------- Removed virtual inlines...
//
StreamIO* VolumeStream::openStream( const char *, bool checkDiskFirst)
{
   checkDiskFirst;
   // not supported by default
   return (NULL);
}   

StreamIO* VolumeStream::openStream( DWORD )
{
   return (NULL);
}   

//------------------------------------------------------------------------------
Bool VolumeRWStream::open( const char *pFileName, bool checkDiskFirst )
{
   checkDiskFirst;
   return ( open( locate( pFileName ) ) );
}

Bool VolumeRWStream::open( DWORD fileName )
{
   return ( open( locate( fileName ) ) );
}

Bool VolumeRStream::open( const char *pFileName, bool checkDiskFirst )
{
   if (checkDiskFirst)
      return ( openFromDisk( pFileName ) || open( locate( pFileName ) ) );
   else
      return ( open( locate( pFileName ) ) || openFromDisk( pFileName ) );
}

Bool VolumeRStream::open( DWORD fileName )
{
   return ( openFromDisk( fileName ) || open( locate( fileName ) ) );
}


//------------------------------------------------------------------------------
Int32 VolumeStream::getSize( const char *pFileName ) const
{
   return ( getSizeVol( pFileName) );
}

Int32 VolumeStream::getSize( DWORD fileName ) const
{
   return ( getSizeVol( fileName) );
}

Int32 VolumeStream::getSize() const
{
   return (openSize);
}   

Int32 VolumeRStream::getSize( const char *pFileName ) const
{
   Int32 size = getSizeDisk( pFileName );
   if ( size >= 0 ) return ( size );
   else return getSizeVol( pFileName );
}

Int32 VolumeRStream::getSize( DWORD fileName ) const
{
   Int32 size = getSizeDisk( fileName );
   if ( size >= 0 ) return ( size );
   else return  getSizeVol( fileName );
}


//------------------------------------------------------------------------------
StreamStatus VolumeStream::getStatus() const
{
   if(userStream)
      return userStream->getStatus();
   else
      return STRM_FLTR_DETACHED;
}

