//------------------------------------------------------------------------------
//   
//  $Workfile:   WinTools.cpp  $
//  $Version$
//  $Revision:   1.00  $
//    
//  DESCRIPTION:
//     Functions to aid in Windows Tools development
//        
//  (c) Copyright 1996, Dynamix Inc.   All rights reserved.
// 
//------------------------------------------------------------------------------

#include "wintools.h"

//------------------------------------------------------------------------------
//   
// NAME
//   GenericDlg
//   
// DESCRIPTION
//   Handle dialog boxes that simply display a notice
//   
//------------------------------------------------------------------------------

BOOL CALLBACK GenericDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM)
{
   BOOL  msgProcessed;
   RECT  r;

   msgProcessed = FALSE;
   switch (message) 
   {
      case WM_INITDIALOG:         
         GetWindowRect(hDlg, &r);

         r.left = (GetSystemMetrics(SM_CXSCREEN) - (r.right - r.left)) / 2;
         r.top  = (GetSystemMetrics(SM_CYSCREEN) - (r.bottom - r.top)) / 2;
         SetWindowPos(hDlg, NULL, r.left,r.top,0,0, SWP_NOSIZE | SWP_NOZORDER);
         msgProcessed = TRUE;
         break;

      case WM_COMMAND:            
         switch(LOWORD(wParam))
         {
            case IDYES:
            case IDNO:
            case IDOK:
            case IDCANCEL:
               EndDialog(hDlg, wParam);         
               msgProcessed = TRUE;
               break;  

            default:
               break;
         }
         break;
   }
   return (msgProcessed);               
}

//------------------------------------------------------------------------------
//   
// CLASS
//   FileInfo
//   
// DESCRIPTION
//   Methods for handling common dialog file IO boxes
//   
//------------------------------------------------------------------------------

char  *Filter[] =
{
   "NULL",
   "MS Bitmap (*.BMP, *.DIB)|*.BMP;*.DIB|",
   "GFX Palette (*.PAL)|*.PAL|",
   "RM.INI file (*.INI)|*.INI|",
   "Autodesk FLIC (*.FLC)|*.FLC|",
   "GFX Bitmap Array (*.PBA)|*.PBA|",
   "GFX Font (*.PFT)|*.PFT|",
   "Phoenix GUI (*.GUI)|*.GUI|",
   "Phoenix GUI Tags (*.TAG)|*.TAG|",
   "Phoenix Volume (*.VOL)|*.VOL|",
   "Phoenix Shape (*.DTS)|*.DTS|",
   "Phoenix Material List (*.DML)|*.DML|",
   "Phoenix Landscape Script (*.PLS)|*.PLS|",
   "Phoenix Mission File (*.MIS)|*.MIS|",
   "Phoenix Object Macro (*.MAC)|*.MAC|",
   "Phoenix Bayesian Network (*.AI)|*.AI|",
   "Phoenix Landscape Rules (*.PLR)|*.PLR|"
};

//------------------------------------------------------------------------------

FileInfo::FileInfo()
{
   memset(&ofn,0,sizeof(OPENFILENAME));
}   

//------------------------------------------------------------------------------

FileInfo::~FileInfo()
{
   free();
}

//------------------------------------------------------------------------------

void FileInfo::free()
{
   delete [] const_cast<char*>(ofn.lpstrFile);
   delete [] const_cast<char*>(ofn.lpstrDefExt);
   delete [] const_cast<char*>(ofn.lpstrFilter);
}

//------------------------------------------------------------------------------

void __cdecl FileInfo::init( HWND _hWnd, char *_defExt, ... )
{
   free();
   filterSize = 0;

   if (!_defExt) _defExt = "";
   memset(&ofn,0,sizeof(OPENFILENAME));
   ofn.lpstrFile        = new char[MAX_FILENAME_LEN];    // default receive buffer
   ofn.lpstrInitialDir  = new char[MAX_FILENAME_LEN];    // default initial directory
   ofn.lpstrCustomFilter= new char[MAX_FILENAME_LEN];
   ofn.lpstrDefExt      = strnew(_defExt);               // set default extension
   ofn.nFilterIndex     = 1;                             // use first filter as default
   ofn.hwndOwner        = _hWnd;                         // set parent window
   ofn.nMaxFile         = MAX_FILENAME_LEN;
   ofn.nMaxCustFilter   = MAX_FILENAME_LEN;
   ofn.lStructSize      = sizeof(OPENFILENAME);
   ofn.Flags            = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_NOCHANGEDIR | OFN_EXPLORER | OFN_ENABLEHOOK;

   memset(filename,                     0,MAX_FILENAME_LEN);
   memset((char*)ofn.lpstrInitialDir,   0,MAX_FILENAME_LEN);
   memset((char*)ofn.lpstrCustomFilter, 0,MAX_FILENAME_LEN);
   memset(ofn.lpstrFile,                0,MAX_FILENAME_LEN);

   va_list   va;
   va_start( va, _defExt );
   setFilter( va );
   va_end( va );
}   

//------------------------------------------------------------------------------

void FileInfo::setFilter( va_list va )
{
   char     delimiter;                 // string delimiter, usually a " character
   int      filter;                    // argument passed
   unsigned int i,j;                   // string indecies
   char     tmp[2048];

   delete [] const_cast<char*>(ofn.lpstrFilter);

   j = 0;
   while ((filter = va_arg(va,int)) !=0)  // for each argument
   {
      delimiter = Filter[filter][ strlen(Filter[filter])-1 ]; // get the string delimiter
      for ( i=0; i<strlen(Filter[filter]); i++ )     // search entire string for delimiters
      {
         tmp[j++] = Filter[filter][i]; // add string to output string
         if ( tmp[j-1]==delimiter )    // is character a delimiter???
            tmp[j-1]=0;                // yes, then replace it with a NULL character
      }
   }
   filterSize = j;
   ofn.lpstrFilter = new char[filterSize];
   tmp[filterSize]=0;
   memcpy( (char*)ofn.lpstrFilter, tmp, filterSize );
   va_end(va);
}

//------------------------------------------------------------------------------

void FileInfo::addFilter( const char *description, const char *extension )
{
   const char *buff = avar("%s (%s)", description, extension);
   int size = strlen(buff) + strlen(extension) + 2;
   char *newFilter = new char[filterSize+size];
   memcpy( newFilter, ofn.lpstrFilter, filterSize );
   memcpy( newFilter+filterSize, buff, strlen(buff)+1 );
   memcpy( newFilter+filterSize+strlen(buff)+1, extension, strlen(extension)+1 );
   filterSize += size;
   newFilter[filterSize]=0;

   delete [] const_cast<char*>(ofn.lpstrFilter);
   ofn.lpstrFilter = newFilter;
}   

//------------------------------------------------------------------------------
bool FileInfo::multipleFileName()
{
   if ( *fnPtr )
   {
      strcpy( filename, ofn.lpstrInitialDir );
      strcat( filename, "\\" );
      strcat( filename, fnPtr );
      while( *fnPtr++ );
      return true;      
   }
   else
      return false;
}   

//------------------------------------------------------------------------------

bool FileInfo::getName( FI_NAME_MODE _mode, char *_title )
{
   if ( _mode == OPEN_MULTIPLE_MODE )
   {
      ofn.Flags |= OFN_ALLOWMULTISELECT;
      delete [] const_cast<char*>(ofn.lpstrFile);
      ofn.lpstrFile = new char[MAX_MULTIFILE_LEN]; // enlarge buffer for multiple names
      ofn.nMaxFile  = MAX_MULTIFILE_LEN;
      memset((char*)ofn.lpstrFile,0,MAX_MULTIFILE_LEN);
   }
   else
      ofn.Flags &= ~OFN_ALLOWMULTISELECT;

   ofn.lpstrTitle  = _title;                       // set caption (optional)

   if ((_mode<OPEN_MODE)? GetSaveFileName(&ofn):GetOpenFileName(&ofn))
   {
      strcpy( filename, ofn.lpstrFile );           // a copy for the user to have
      memcpy( (char*)ofn.lpstrInitialDir, filename, ofn.nFileOffset );
      (char)(ofn.lpstrInitialDir[ofn.nFileOffset])=0;
      if (_mode!=OPEN_MULTIPLE_MODE)
         strcpy(ofn.lpstrFile, ofn.lpstrFile+ofn.nFileOffset); // chop it for next getName()
      else
         fnPtr = ofn.lpstrFile+ofn.nFileOffset;
      return TRUE;
   }
   return NULL;
}

