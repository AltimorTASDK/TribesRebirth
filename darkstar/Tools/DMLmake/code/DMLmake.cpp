//------------------------------------------------------------------------------
// Description 
//    Creates a DML file from a list of BMPs
//
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include <base.h>
#include <filstrm.h>
#include <ts_material.h>

#include <windows.h>

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#define MAX_NUM_ARGS 1024  


#define AssertExit(b,m) if (!(b))     \
                        {             \
                           printf(m); \
                           exit(1);   \
                        }


static char *sgFileList[MAX_NUM_ARGS];
static UInt32 sgNumFiles = 0;


//------------------------------------------------------------------------------
// NAME 
//    void addFileToList(char filename)
//    
// DESCRIPTION 
//    
//    
// ARGUMENTS 
//    
//    
// RETURNS 
//    
//    
// NOTES 
//    
//------------------------------------------------------------------------------
void addFileToList(char *filename)
{
   HANDLE h;
   WIN32_FIND_DATA findData;

   // allow for wildcards by using FindFirst
   h = FindFirstFile(filename, &findData);
   if (h != INVALID_HANDLE_VALUE)
   {
      do
      {
         sgFileList[sgNumFiles] = new char[strlen(findData.cFileName) + 1];
         AssertExit(sgFileList[sgNumFiles], "Ran out of memory!");
         strcpy(sgFileList[sgNumFiles], findData.cFileName);
               
         sgNumFiles++;

      } while (FindNextFile(h, &findData));
   }
   else 
   {
      // The first file specifies the output file.
      // If the output file does not currently exist,
      // FindFirstFile will fail.
      if (sgNumFiles == 0) 
      {
         sgFileList[sgNumFiles] = new char[strlen(filename) + 1];
         AssertExit(sgFileList[sgNumFiles], "Ran out of memory!");
         strcpy(sgFileList[sgNumFiles], filename);
         sgNumFiles++;
      }
      else
      {
         printf("No files matching %s exists\n",filename);
         AssertExit(0," ");
      }
   }
   FindClose(h);
}


//------------------------------------------------------------------------------
// NAME 
//    void loadResponseFile(char *filename)
//    
// DESCRIPTION 
//    For each argument in the responseFile, store it in the sgFileList. 
//    
// ARGUMENTS 
//
//    
// RETURNS 
//
//    
// NOTES 
//
//------------------------------------------------------------------------------
void loadResponseFile(char *responseFile)
{
   FileRStream file;
   char *sourceBuf;   
   Bool result;

   // read the entire file into memory
   result = file.open(responseFile);
   AssertExit(result, "Could not open response file");
   
   sourceBuf = new char[file.getSize()];
   AssertExit(sourceBuf, "Ran out of memory!");

   file.read(file.getSize(), sourceBuf);
   file.close();

   // now, put each filename into the fileList
   char filename[MAX_PATH];
   int strLength;
   char *bufWalk = sourceBuf;

   while (1 == sscanf(bufWalk, "%s", &filename)) 
   {
     
      bufWalk = strstr(bufWalk, filename);
      AssertExit(bufWalk, "Parse failed!");
      strLength = strlen(filename);
      bufWalk += strLength;

      addFileToList(filename);
   }   

   // clean up and return
   delete[] sourceBuf;
}   


//------------------------------------------------------------------------------
// NAME 
//    TSMaterialList *createMatList(const char *bmpFileList[], UInt32 numFiles)
//    
// DESCRIPTION 
//    Creates a material list from the list of bmp files and returns it.
//    
// ARGUMENTS 
//    
//    
// RETURNS 
//    
//
// NOTES 
//    
//------------------------------------------------------------------------------
TSMaterialList *createMatList(char *bmpFileList[], UInt32 numFiles)
{
   TSMaterialList *matList;
   TSMaterial mat;
   FileRStream file;
   Bool result;
   char buffer[200];

   mat.fParams.fAlpha = 0;
   mat.fParams.fFlags = TSMaterial::MatTexture | TSMaterial::ShadingNone;

   matList = new TSMaterialList(numFiles);
   AssertExit(matList,"Ran out of memory!");

   for (int i = 0; UInt32(i) < numFiles; i++)
   {
      AssertExit(strlen(bmpFileList[i]) < TSMaterial::MapFilenameMax,
                     "Bmp file name exceeds limit!");

      // make sure the bmp file exists.
      result = file.open(bmpFileList[i]);
      if (result == false)
      {
         printf("Cannot open %s",bmpFileList[i]);
         exit(0);
      }
      file.close();

      printf("Writing %s to the dml\n",bmpFileList[i]);

      strcpy( mat.fParams.fMapFile, bmpFileList[i]);
      &bmpFileList[i];
      matList->setMaterial(i, mat);
   }
   
   return matList;
}   


//------------------------------------------------------------------------------
// NAME 
//    void saveMatList(char *filename, TSMaterialList *matList)
//    
// DESCRIPTION 
//    Writes out the material list to the specified file.
//    
// ARGUMENTS 
//    
//    
// RETURNS 
//    
//    
// NOTES 
//    
//------------------------------------------------------------------------------
void saveMatList(char *filename, TSMaterialList *matList)
{
   FileWStream file;
   Bool result;

   result = file.open(filename);
   AssertExit(result, "Can't write to specified DML file");

   matList->store(file);

   file.close();
}   


//------------------------------------------------------------------------------
// NAME 
//    void printUsage(void)
//    
// DESCRIPTION 
//    
//    
// ARGUMENTS 
//    
//    
// RETURNS 
//    
//    
// NOTES 
//    
//------------------------------------------------------------------------------
void printUsage(void)
{
   printf("\nUsage: dmlmake <file list>\n");
   printf("  The first name in the file list specifies the\n");
   printf("  dml file that will be created.   The remainder\n");
   printf("  of the list should specify all the BMP files\n");
   printf("  you want included in the dml.\n\n");
   printf("  At any position in the file list, you may include\n");
   printf("  one or more response files.  A response file is\n");
   printf("  designated by a '@' character before its name.\n\n");
   printf("  The args contained within the response file\n");
   printf("  is substituted in the position the response file\n");
   printf("  occurs in the command line\n\n"); 
   printf("Example usage:\n");
   printf("  dmlmake out.dml foo.bmp cat.bmp\n");
   printf("  dmlmake @reponse\n");
   printf("  dmlmake out.dml @reponse\n\n");
   exit(0);   
}   


//------------------------------------------------------------------------------
// NAME 
//    void main(int argc, char *argv[])
//    
// DESCRIPTION 
//    
//    
// ARGUMENTS 
//    
//    
// RETURNS 
//    
//    
// NOTES 
//    
//------------------------------------------------------------------------------
int main(int argc, char *argv[]) 
{
   // Initialize a sgFileList from the command line args.
   // The first file in the list denotes the name of the
   // new dml file to create.
   if (argc == 1)
      printUsage();

   for (UInt32 i = 1; i < UInt32(argc); i++) 
   {
      AssertExit(sgNumFiles < MAX_NUM_ARGS, "Too many arguments!");

      if (*argv[i] == '@')
      {
         loadResponseFile(&(argv[i][1]));
      }
      else
      {
         addFileToList(argv[i]);
      }  
   }

   AssertExit(sgNumFiles > 1, "No bitmap files found!");
   
   // Create and save a material list from the file list. 
   TSMaterialList *matList;
  
   matList = createMatList(sgFileList + 1, sgNumFiles - 1);
   saveMatList(sgFileList[0], matList);   
   
   return 0;
}