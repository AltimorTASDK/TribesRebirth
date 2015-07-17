//北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北
//北   
//北  $Workfile:   WinTools.h  $
//北  $Version$
//北  $Revision:   1.00  $
//北    
//北  DESCRIPTION:
//北     Functions to aid in Windows Tools development
//北        
//北  (c) Copyright 1996, Dynamix Inc.   All rights reserved.
//北 
//北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北

#ifndef  _WINTOOLS_H_
#define  _WINTOOLS_H_

#include <windows.h>
#include <types.h>
#include <base.h>
#include <stdarg.h>
#include <commdlg.h>

//北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北
// A function for handling generic dialog boxes

BOOL CALLBACK GenericDlg(HWND, UINT, WPARAM, LPARAM);

//北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北
// a class for handling common dialog box file IO
//
// example:  
//   FileInfo fi;
//   fi.init( HWND_DESKTOP, "bmp", FileInfo::TYPE_DIB, FileInfo::TYPE_PAL, NULL );
//
//   if ( fi.getName( FileInfo::SAVE_MODE, "Save Bitmap to file..." ) )
//      SaveFile( fi.filename );
//
//  fi retains path and file information for future access to the load/save boxes.
//  It can handle multiple file selection
//
//   while ( fi.getName( FileInfo::OPEN_MULTIPLE_MODE, "Multiple Open Bitmaps..." ) )
//   {
//      while ( fi.multipleFileName() )
//         strcpy( buff, fi.filename );
//   }
//

#define MAX_FILENAME_LEN   256
#define MAX_MULTIFILE_LEN  4096

class FileInfo
{
public:
   enum FI_NAME_MODE
   {
      SAVE_MODE,              // to create a "Save..." dialog
      SAVE_AS_MODE,           // to create a "Save as..." dialog
      OPEN_MODE,              // to create an "Open..." dialog
      OPEN_MULTIPLE_MODE,     // to create an "Open..." dialog that allows multi-selection
   };
   
   enum FI_FILE_TYPE
   {
      TYPE_DIB = 1,
      TYPE_PAL,
      TYPE_INI,
      TYPE_FLC,
      TYPE_PBA,
      TYPE_PFT,
      TYPE_GUI,
      TYPE_TAG,
      TYPE_VOL,
      TYPE_DTS,
      TYPE_DML,
      TYPE_PLS,
      TYPE_MIS,
      TYPE_MAC,
      TYPE_BAY,
      TYPE_PLR
   };

   OPENFILENAME ofn;
   char  filename[256];

private:
   void  setFilter( va_list va );
   int   filterSize;
   char  *fnPtr;

public:
   FileInfo();
   ~FileInfo();

   void  __cdecl init( HWND _hWnd, char *_defExt, ... );
   void  free();

   void  setInitDir( const char *initDir );
   bool  getName( FI_NAME_MODE _mode, char *_title );
   char* fileName() { return (filename); }
   bool  multipleFileName();
   void  addFilter( const char *description, const char *extention );
};

//------------------------------------------------------------------------------

inline void FileInfo::setInitDir( const char *initDir )
{
   strcpy( (char*)ofn.lpstrInitialDir, initDir );
}   

#endif   _WINTOOLS_H_