//---------------------------------------------------------------------------
//
//	$Workfile:   vt.cpp  $
// $Version$
//	$Revision:   3.0.1  $
// $Version$
//	$Date:   04 Dec 1995 10:06:10  $
//	$Log:   R:\darkstar\develop\tools\vt\vcs\vt.cpv  $
//
//         Rev 3.0.1 24 March 1998 DAVE MOORE / MITCH SHAW
//	   Rev 3.0   03 June 1997 LOUIE MCCRADY
//	   Rev 2.0   03 Dec 1996 DAVE SCOTT
//	   Rev 1.0   04 Dec 1995 10:06:10   CAEDMONI
//	Initial revision.
//
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Volume Tool
//---------------------------------------------------------------------------

#include <typeinfo.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>
#include <stdio.h>
#include <iostream.h>
#ifdef MSVC
#include <io.h>
#else
#include <dir.h>
#endif
#include <ts_material.h>

#include <tvector.h>
#include <streams.h>
#include <g_bitmap.h>
#include <ctype.h>  // needed for isspace()

//--------------------------------------------------------------------------

const char TokenSep='#';
const int BufLength= 16000;

struct brokenPath {
	char theDrive[3];	 // drive; includes colon (:)
	char theDir[256];  // dir; includes leading and trailing backslashes (\)
	char theFile[256]; // name
	char theExt[256];  // ext; includes leading dot (.)
};

char *tempBuf, *bmpPath, *numDetails, *palFile;
Vector<char *> fileAdds;

BOOL isDML=false;
BOOL isStrip=false;
BOOL isMip=false;
BOOL isBat=false;
BOOL isTrace=true;
int numErrors=0;

char volumeFile[256];
char addFile[256];
char *addName;
// hey, this is a global hack but so is the rest of the program.
#define  MAX_RANGES  10    // only allow 10 range settings on palette
int rangeIndex = 0;

char palRange[MAX_RANGES][512];

VolumeRWStream * pVol = 0;
StreamCompressType compressType = STRM_COMPRESS_NONE;
Bool compacting = FALSE;
Bool nopbm = FALSE;

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

void Usage()
{
   cout << "**********************************************************************\n";
   cout << "* Vt          Version 3.0.1                                          *\n";
   cout << "*                                                                    *\n";
   cout << "* usage: vt [options] volume [@]file                                 *\n";
   cout << "*                                                                    *\n";
   cout << "* options:                                                           *\n";
   cout << "*  -sp        strip the path of the file when inserting into volume. *\n";
   cout << "*  -lzh       Set the compression type to LZH                        *\n";
   cout << "*  -rle       Set the compression type to RLE                        *\n";
   cout << "*  -cpt       Compact the volume into a new volume named file.       *\n";
   cout << "*  -nopbm     don't convert .dib's or .bmp's to phoenix bitmaps      *\n";
   cout << "*  -q         turn on quiet mode. No messages output                 *\n";
   cout << "*  -------------------------------------------------------------     *\n";
   cout << "*                                                                    *\n";
   cout << "*  -dml[#bmp_path]  adds the bitmaps from dml file to volume         *\n";
   cout << "*                                                                    *\n";
   cout << "*  -mip[#n[#palette_file]] invokes MIPMAKE to mip the bmp            *\n";
   cout << "*  -r(min,max) set mipmap palette range(s) restrictions              *\n";
   cout << "*                                                                    *\n";
   cout << "*  [@] @ symbol before addfile implies addfile is a script file      *\n";
   cout << "*       of args to pass to vt (omit volume on all lines).            *\n";
   cout << "*                                                                    *\n";
   cout << "*                                                                    *\n";
   cout << "**********************************************************************\n";
   cout << "*                                                                    *\n";
   cout << "* Adds the file to the volume.  If the volume does not exist, it is  *\n";
   cout << "* created.                                                           *\n";
   cout << "*                                                                    *\n";
   cout << "**********************************************************************\n";
}

void CleanUp()
{
	if (!tempBuf) delete[] tempBuf;
	if (!bmpPath) delete[] bmpPath;
	if (!numDetails) delete[] numDetails;
	if (!palFile) delete[] palFile;
   tempBuf= bmpPath= numDetails= palFile= NULL;

}
void InitVars()
{
	CleanUp();

	isDML=false;
	isMip=false;
   isBat=false;

	addFile[0]= '\0';
	addName= NULL;
}


////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////// stuff merged in from vinfo //////////////////////////////

/////////////////////////////////////////////////////////////////////////////

class VolumeInfoStream : public VolumeRStream
{
 public:
   Vector<VolumeItem> &getVolumeItems() { return(volumeItems); }
   int getStringBlockOffset()           { return(stringBlockOffset); }
};

/////////////////////////////////////////////////////////////////////////////

BOOL DumpVol()
{

   VolumeInfoStream vol;
   if (!vol.openVolume(volumeFile))
   {
      numErrors++;
      isTrace = true;
      fprintf(stderr, "unable to open \"%s\"\n", volumeFile);
      exit(numErrors);
   }

   Vector<VolumeItem> &volumeItems = vol.getVolumeItems();
   if (isTrace) fprintf(stderr, "Volume %s: stringBlockOffset = %d\n", volumeFile,
           vol.getStringBlockOffset());

   for (int i = 0; i < volumeItems.size(); i++)
      if (isTrace) fprintf(stderr,
              "%3d: %20s at offset %d for %d bytes\n",
              i,
              volumeItems[i].string,
              volumeItems[i].blockOffset,
              volumeItems[i].size);
   if (isTrace) fprintf(stderr, "\n");

   return (true);
}

//------------------------------------------------------------------------
// Parse a single command line argument
void ParseArgument (char * argv, bool alreadyKnowVol=false)
{
      if (!argv) return;
		if (argv[0] == '-')
      {
         if (argv[1]=='q')
         	isTrace=false;
         else if (!strncmp(argv, "-mip", 4)) 
         {
         	isMip= true;
            if ( (strlen(argv) > 5) && (argv[4]==TokenSep) )
            {
	  				if (numDetails)
	  				{
   	         	delete[] numDetails;
      	         numDetails=NULL;
         	   }
			   	numDetails = new char[2];
               numDetails[0]= argv[5];
               numDetails[1]= '\0';

               if ( (strlen(argv) > 7) && (argv[6]== TokenSep))
               {
		  				if (palFile)
		  				{
      		      	delete[] palFile;
            		   numDetails=NULL;
		            }
					   palFile = new char[256];
                  strcpy(palFile, &argv[7]);
               }
            }
         }
         else if (!strncmp(argv, "-r(", 2)) 
         {
            AssertFatal( rangeIndex<MAX_RANGES, avar("Maximum number of color ranges(%i) exceeded",MAX_RANGES) );
            strcpy(palRange[rangeIndex], argv);
            rangeIndex++;
         }
         else if (!strncmp(argv, "-dml", 4)) 
         {
         	isDML=true;
            if ( (strlen(argv) > 5) && (argv[4]=='#'))
            {
   				if (bmpPath)
   				{
               	delete[] bmpPath;
                  bmpPath=NULL;
               }
				   bmpPath = new char[BufLength];

            	char *ptr= strchr(argv,TokenSep);
               strcpy(bmpPath, ++ptr);
               int length=strlen(bmpPath);
               if (bmpPath[length-1] != '\\') 
                  strcat(bmpPath,"\\");
            }
         }
         else if(!strcmp(argv, "-lz"))
         {
            compressType = STRM_COMPRESS_LZ;
            if (isTrace) printf("Compressing with lempel-ziv.\n");
         }
         else if(!strcmp(argv, "-lzh"))
         {
            compressType = STRM_COMPRESS_LZH;
            if (isTrace) printf("Compressing with lempel-ziv huffman.\n");
         }
         else if(!strcmp(argv, "-rle"))
         {
            compressType = STRM_COMPRESS_RLE;
            if (isTrace) printf("Compressing with run-length encoding.\n");
         }
         else if(!strcmp(argv, "-nopbm"))
         {
            nopbm = TRUE;
         }
         else if(argv[1] == 'n')
         {
            if (isTrace)
              printf("vt: \"-n\" option is obsolete, ignoring\n"); 
         }

         else if(!strcmp(argv, "-sp"))
         {
            isStrip = TRUE;
         }
         else if(argv[1] == 's')
         {
            if (isTrace)
               printf("vt: \"-s\" option is obsolete, ignoring\n");
         }
         else if(!strcmp(argv, "-cpt"))
         {
            compacting = TRUE;
            if (isTrace) printf("Compacting volume into new volume file.\n");
         }
         else
            Usage();
      }
      else if( !alreadyKnowVol && !volumeFile[0] )
      {
         strcpy( volumeFile, argv );
      }
      else if( !addFile[0] )
      {
      	//test for batch file here
         if (argv[0]=='@' && !isBat) 
         {
           isBat = true;
           argv++;
         }

         addName = addFile;
         strcpy( addFile, argv );
         if(isStrip)
         {
            addName = strrchr(addFile, '\\');
            if(!addName)
               addName = addFile;
            else
               addName++;
         }

         brokenPath pathHolder;
#ifdef MSVC
         _splitpath( addFile, pathHolder.theDrive, pathHolder.theDir, pathHolder.theFile, pathHolder.theExt );
#else
         fnsplit(addFile, pathHolder.theDrive, pathHolder.theDir, pathHolder.theFile, pathHolder.theExt);
#endif
         /*
         // Check for wildcards
         if ((pathHolder.theFile && (strchr(pathHolder.theFile, '*') || strchr(pathHolder.theFile, '?'))) ||
             (pathHolder.theExt  && (strchr(pathHolder.theExt,  '*') || strchr(pathHolder.theExt,  '?'))))
         {
            if (isBat)
            {
               numErrors ++;
               isTrace = true;
               cout << "error: batch files cannot contain wildcards" << endl;
               return;
            }

         }
         */

         if (isDML) 
         {
            if (!bmpPath) 
            {
               bmpPath = new char[BufLength];
            	strcpy(bmpPath,pathHolder.theDrive);
   	         strcat(bmpPath,pathHolder.theDir);
            }
         }

      }
      else 
      {
      	numErrors++;
         isTrace = true;
			printf("error: argument '%s' not understood", argv);
      }

}

//------------------------------------------------------------------------
// Check for command line flags.

void GetOptions( int argc, char **argv )
{
   isDML=false;
   isMip=false;
   isStrip=false;
   volumeFile[0] = 0;
   addFile[0] = 0;

	for (int a = 1; a < argc; ++a) {
     ParseArgument(argv[a]);
   }

  if( !volumeFile[0] )
     Usage();

}


//--------------------------------------------------------------------
// Get volume

bool GetVolume()
{
   pVol = new VolumeRWStream();

   // openVolume() will create a new volume if volumeFile doesn't exist
   return (pVol->openVolume(volumeFile));
}


Bool CompactVolume()
{
   if (pVol->compactVolume(addFile))
   {
      if (isTrace) printf("done\n");
      return(true);
   }
   else
   {
   	numErrors++;
      isTrace = true;
      printf("Error during compaction\n");
      return(false);
   }
}

Bool isBitmap(char *name)
{
   if(nopbm)
      return FALSE;
   char *ext = strrchr(name, '.');
   if(ext && (!strcmp(ext, ".bmp") ||
         !strcmp(ext, ".BMP") ||
         !strcmp(ext, ".dib") ||
         !strcmp(ext, ".DIB")))
         return TRUE;
   return FALSE;
}

//--------------------------------------------------------------------
// Close the volume


void CloseVolume()
{
   pVol->closeVolume();
}


//--------------------------------------------------------------------
// Copy the file name and shortname to the vector
bool CopyFile(char *afile, char *shortname=NULL)
{
	int iSize = 3 + strlen(afile) + ((shortname==NULL) ? 0:strlen(shortname));
	char *holdingBuf= new char [iSize];

   strcpy(holdingBuf,afile);
	strcat(holdingBuf, "#");
   if (shortname) strcat(holdingBuf, shortname);

   //check for duplicate
   bool found= false;
   int iEnd= fileAdds.size();
   for (int i=0; i< iEnd; i++)
   	if (strcmpi(holdingBuf, fileAdds[i]) == 0) {
      	found= true;
         break;
      }

   if (!found) {
   	fileAdds.push_back(holdingBuf);
   } else {
   }

   return true;
}


//--------------------------------------------------------------------
// Copy the file to the volume

bool DoCopyFile(char *afile, char *shortname=NULL)
{                                         
   if ( afile==NULL && shortname==NULL )
      return ( true );
   bool result = TRUE;
   UInt32 size = 0;
   char * buff = 0;
   Bool useBitmap = FALSE;

   //------------------
   // get source file:

   Bool packedAsPhoensixBitmap = false;

   if(isBitmap(afile))
   {
      GFXBitmap *bmp = GFXBitmap::load(afile);

      if(!bmp)
      {
      	numErrors++;
         isTrace = true;
         printf("Error loading bitmap file '%s'.\n", afile);
         return FALSE;
      }
      packedAsPhoensixBitmap = true;
      bmp->write("vttmp.bmp",0);
      useBitmap = TRUE;
      delete bmp;
   }

   FileRStream ist;
   if(useBitmap)
      ist.open("vttmp.bmp");
   else
      ist.open( afile );

   if( ist.getStatus() != STRM_OK )
   {
      numErrors++;
      isTrace = true;
      printf( "Error opening file: %s\n", shortname ? shortname : afile ); 
      result = FALSE;
   }
   else
   {
      size = ist.getSize();
      buff = new char[size];
      if( !ist.read( size, buff ) )
      {
         numErrors++;
         isTrace = true;
         printf( "Error reading from file: %s\n", afile );
         result = FALSE;
      }
      ist.close();
   }
   if( !result )
      return result;

   char *memBuf = new char[size * 3 + 1000];

   StreamIO *mstrm = new MemRWStream(size * 3 + 1000, memBuf);
   StreamIO *cprsdStream = 0;

   if(compressType == STRM_COMPRESS_LZH)
      cprsdStream = new LZHWStream(*mstrm);
   else if(compressType == STRM_COMPRESS_RLE)
      cprsdStream = new RLEWStream(*mstrm);

   if(!cprsdStream)
   {
      cprsdStream = mstrm;
   }
   cprsdStream->write(size, (void *) buff);

   //------------------
   // write to volume:
   brokenPath pathHolder;

#ifdef MSVC
   if (shortname) _splitpath(shortname, pathHolder.theDrive, pathHolder.theDir, pathHolder.theFile, pathHolder.theExt);
   else _splitpath( afile, pathHolder.theDrive, pathHolder.theDir, pathHolder.theFile, pathHolder.theExt );
#else
   if (shortname) fnsplit(shortname, pathHolder.theDrive, pathHolder.theDir, pathHolder.theFile, pathHolder.theExt);
   else fnsplit(afile, pathHolder.theDrive, pathHolder.theDir, pathHolder.theFile, pathHolder.theExt);
#endif

   if (tempBuf) {
   	delete[] tempBuf;
      tempBuf=NULL;
   }
   tempBuf = new char[BufLength];

   if (isStrip) {
   	strcpy(tempBuf, pathHolder.theFile);
      strcat(tempBuf, pathHolder.theExt);
   }
   else
   {
   	strcpy(tempBuf,pathHolder.theDir);
      strcat(tempBuf,pathHolder.theFile);
      strcat(tempBuf, pathHolder.theExt);
   }

		if (isTrace) 
		{
         char *addString = new char[BufLength];
         if (strlen(tempBuf) > 40) 
         {
         	strcpy(addString, "..\\");
            strcat(addString, pathHolder.theFile);
         } 
         else 
            strcpy (addString, tempBuf);

         if (!size)
         {
            numErrors++;
            isTrace = true;
            printf("Added: %s %d bytes@ %d%%", addString, mstrm->getPosition(), 100 );
            printf( "warning: file size of ZERO '%s'\n", addString );
         }
         else
            printf("Added: %s %d bytes@ %d%%", addString, mstrm->getPosition(), (mstrm->getPosition()*100)/size );

         if (packedAsPhoensixBitmap) printf("(Phx)");
         printf("\n");

         delete[] addString;
      }


   if( pVol->open( tempBuf, compressType, size ) )
   {
      if( !pVol->write( size, buff ) )
      {
         numErrors++;
         isTrace = true;
         printf( "Error writing to volume\n" );
         result = FALSE;
      }
      pVol->close();
   }
   else
   {
      numErrors++;
      isTrace = true;
      printf( "Error opening item %s in volume.\n", afile );
      result = FALSE;
   }
   return result;
}
//--------------------------------------------------------------------------

void mipInvoke(char * filesToMip)
{
	char comLine[16000];
   char suffix[10];
   static int tempFileNo=0;

   sprintf( comLine, "MIPMAKE -ovttmp -i%i ", tempFileNo );

   if (numDetails) strcat(comLine, numDetails);
   strcat(comLine, " ");
   if (palFile) strcat(comLine, avar(" -p%s",palFile));
   if (rangeIndex)
   {
      for ( int i=0; i<rangeIndex; i++ )
      {
         strcat(comLine, " ");
         strcat(comLine, palRange[i]);  // append palette range (min,max)
      }
   }
   strcat(comLine, " ");
   strcat(comLine,filesToMip);

   STARTUPINFO StartupInfo = {0};
   PROCESS_INFORMATION ProcessInfo;
   int dwCreationF = 0;
   if (!isTrace) dwCreationF = DETACHED_PROCESS;

   StartupInfo.cb = sizeof(STARTUPINFO);

   if (CreateProcess(NULL, comLine, NULL, NULL, FALSE,
                     dwCreationF, NULL, NULL, &StartupInfo, &ProcessInfo))
   {
       WaitForSingleObject(ProcessInfo.hProcess, INFINITE);
       /* Process has terminated */
   }
   else
   {
       /* Process has given us problems */
       numErrors++;
       isTrace = true;
       printf("error: could not execute mipmake\n");
   }

   char *pFile = strtok( filesToMip, " " );
   while ( pFile )
   {
      strcpy(comLine,"vttmp");
      strcat(comLine,itoa( tempFileNo++, suffix, 10 ));
      CopyFile(comLine, pFile );
      pFile = strtok( NULL, " " );
   }
}
//--------------------------------------------------------------------------

void doDML()
{
	TS::MaterialList *m_pmlList;
   char suffix[10];

  	// load material list...
 	FileRStream istMatFile;
 	istMatFile.open(addFile);
 	if( istMatFile.getStatus() == STRM_OK )
 	{
      CopyFile(addFile);
 	   Persistent::Base::Error  error;
      m_pmlList= (TS::MaterialList *)Persistent::Base::load(istMatFile,&error);
      istMatFile.close();

      if (tempBuf)
      {
         delete[] tempBuf;
         tempBuf=NULL;
      }
      tempBuf = new char[BufLength];

      int iEnd=m_pmlList->getMaterialsCount();
      TS::Material theMaterial;

      if (isMip)
      {
         tempBuf[0] = 0;
         for (int i =0; i< iEnd;i++)
         {
        	   theMaterial= m_pmlList->getMaterial(i);
            if (theMaterial.fParams.fMapFile[0])
            {
               strcat(tempBuf, bmpPath);
               strcat(tempBuf, theMaterial.fParams.fMapFile);
               strcat(tempBuf, " ");
            }
         }
         mipInvoke(tempBuf);
      }
      else
      {
         for (int i =0; i< iEnd;i++)
         {
            theMaterial= m_pmlList->getMaterial(i);
            if (theMaterial.fParams.fMapFile[0])
            {
               //add bitmap to volume here
               strcpy(tempBuf, bmpPath);
               strcat(tempBuf, theMaterial.fParams.fMapFile);
               CopyFile(tempBuf);
            }
         }
      }
   }
   else
   {
	   numErrors++;
      isTrace = true;
      printf("error: could not open DML file '%s'\n", addFile);
   }
}

//--------------------------------------------------------------------------
void processBat()
{
   char oneLine[512],tempLine[512];
   char* token;
   FILE *stream;

   if ((stream = fopen(addFile, "r"))== NULL)
   {
	   numErrors++;
      isTrace = true;
      printf("error: could not open script file '%s'\n", addFile);
   }

   while (!feof(stream))
   {
		int x = 0;
   	fgets(tempLine,509,stream);          // strip beginning and end of line
      int z = strlen(tempLine) - 1;          // of spaces -- v3.0.1
      while (isspace(tempLine[x])) x++;      // extras spaces caused weirdness
      while (isspace(tempLine[z])) z--;

      int bb=0;
      for (int aa=x;aa<=z;aa++) oneLine[bb++]=tempLine[aa];
      oneLine[bb]='\0';

//      if ( oneLine[strlen(oneLine)-1] == '\n') oneLine[strlen(oneLine)-1]= '\0';
      if (strlen(oneLine) > 2) {
         InitVars();

         token =strtok(oneLine, " ");
         ParseArgument(token, true);
         while (token) {
         	token= strtok(NULL, " ");
	         ParseArgument(token, true);
         }

   		if (isDML) {
	        doDML();
      	}
	      else if (isMip) {
   	      mipInvoke(addFile);
      	}
	      else {
 		      CopyFile(addFile);
		   }
      }
   }

   fclose(stream);
}


//--------------------------------------------------------------------------
void FlushVector()
{
 	char *toAdd;
   char *afile, *shortname;

	while (!fileAdds.empty()) {
		toAdd = fileAdds.front();
		fileAdds.pop_front();

      afile= strtok(toAdd, "#");
      shortname= strtok(NULL, "#");

    	DoCopyFile(afile, shortname);

   	delete[] toAdd;
	}

}


//--------------------------------------------------------------------------

int __cdecl main(int argc,char** argv)
{
   AssertSetFlags(ASSERT_NO_DIALOG);
   AssertSetFlags(1);


   InitVars();
   GetOptions( argc, argv );
   if( !volumeFile[0]) return (false);

   if (!addFile[0] ) return DumpVol();

	if (false == GetVolume())
   {
      cout << "error: cannot open/create volume " << volumeFile << endl;
      return (0);
   }

   if(compacting)
      numErrors += (int)CompactVolume();
   else
      if (isBat) {
         processBat();
      }
   	else if (isDML) {
        doDML();
      }
      else if (isMip) {
      	mipInvoke(addFile);
      }
      else {
       numErrors += (int)CopyFile(addFile);
      }

   FlushVector();
   CloseVolume();
   CleanUp();

   //clean up temporary files
#ifdef MSVC
   struct _finddata_t fileinfo;
   long hFile = _findfirst( "*vttmp*", &fileinfo );
   if( hFile != -1 )
   {
      do
      {
	 remove(fileinfo.name);
      }
      while( !_findnext( hFile, &fileinfo ) );
      _findclose( hFile );
   }
#else
   struct ffblk ffblk;
   bool done= false;
   while (!done)
   {
	   done = findfirst("*vttmp*",&ffblk,0);
      if (!done) remove(ffblk.ff_name);
   }
#endif

   return ( numErrors );
}
