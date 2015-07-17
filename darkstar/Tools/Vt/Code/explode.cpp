// Program to break apart a volume file.

#include <typeinfo.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>
#include <stdio.h>
#include <iostream.h>

#include <streams.h>

///////////////////////////////////////////////////////////////////////////////
// Need our own class so we can get access to the VolumeItemVector memeber:
class MyVStream : public VolumeRStream
{
public:
	int getNoNamedFiles () const
	{
		return volumeItems.size();
	}
	const char *getItemName (int item) const
	{
		return volumeItems[item].string;
	}
};


// Process an individual volume file:
void process_volume (const char *pvol, char *out_name_buf, int ndir_part, bool blist_only);

// main recursive function that looks goes down through directories:
void process_name (
				   const char *name, 
				   const char *dir,
				   char *out_name_buf, 
				   int ndir_part, 
				   bool blist_only, 
				   bool brecurse
				   );
///////////////////////////////////////////////////////////////////////////////
int __cdecl main(int argc, char **argv)
{

   bool brecurse = false, blist_only = false;
   

   // Process the switches:
   for (int n = 2; n < argc; n++)
   {
		if (*argv[n] == '-' || *argv[n] == '/')
		{
			switch (toupper	(*(argv[n] + 1)))
			{
				case 'L':
					blist_only = true;
					break;
				case 'R':
					brecurse = true;
					break;
			}
		}
		else break;
   }


   if(((int) blist_only) + argc - n != 1)
   {
      printf("Usage: explode <volfile> [switches] <target path>.\n");
	  printf("\n\tProgram explodes the contents of a darkstar volume.\n");
	  printf("\tvolfile may contain wildcards.\n");
	  printf("\tSwitches are:\n\t-l\tlist file names only\n\t-r\trecurse\n");

      return 1;
   }

   char buf[_MAX_FNAME],outpath[_MAX_PATH];

   // make sure the exploding path is terminated with a backslash:
   if (!blist_only)
   {
	   strcpy (buf, argv[n]);
	   n = strlen (buf);
	   if (buf[n-1] != '\\')
	   {
			buf[n++] = '\\';
			buf[n] = 0;
	   }

	   _fullpath (outpath, buf, _MAX_PATH);

	   DWORD dw = ::GetFileAttributes (outpath);
	   if (dw == 0xffffffff)
	   {
			printf ("Error: Output path %s does not exist.\n",argv[2]);
			return 1;
	   }
	   if (!(dw & FILE_ATTRIBUTE_DIRECTORY))
	   {
			printf ("Error: Output path %s is not a directory.\n",argv[2]);
			return 1;
	   }
	   printf ("Using output path path %s\n", outpath);
   }


	// Examine the starting the filename to extract the directory to
    // start in:

   _fullpath (outpath, argv[1], _MAX_PATH);

   char dir[_MAX_DIR];
   char ext[_MAX_EXT];
   char file_name[_MAX_FNAME];
   char dir_name[_MAX_PATH];

    _splitpath( outpath, dir_name, dir, file_name, ext );
	strcat (dir_name, dir);
	strcat (file_name, ext);

   process_name (file_name, dir_name, buf, n, blist_only, brecurse);


   return 0;
}
///////////////////////////////////////////////////////////////////////////////
void process_name (
				   const char *name, 
				   const char *dir,
				   char *out_name_buf, 
				   int ndir_part, 
				   bool blist_only, 
				   bool brecurse
				   )
{
	//printf ("Processing %s in dir %s.\n", name, dir);

	HANDLE hfind;
	WIN32_FIND_DATA  findData;
	char buf[_MAX_FNAME];

	// Make sure the directory name is null terminated:
	strcpy (buf, dir);
	int l = strlen(buf);
	if (buf[l-1] != '\\')
	{
		buf[l++] = '\\';
		buf[l] = 0;
	}

	// First, do all the volume files:
	strcpy (buf + l, name);
	hfind = ::FindFirstFile (buf, &findData);

	if (hfind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;

			strcpy (buf + l, findData.cFileName);

			process_volume (buf,  out_name_buf, ndir_part, blist_only);

		} while (::FindNextFile (hfind, &findData));
		::FindClose (hfind);
	}

	if (!brecurse) return;

	// Now, do subdirectories:
	strcpy (buf+l, "*.*");
	if ((hfind = ::FindFirstFile (buf, &findData)) == INVALID_HANDLE_VALUE) return;
	do
	{
		if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) continue;

		if (strcmp (findData.cFileName, ".") == 0 || strcmp(findData.cFileName, "..") == 0) continue;

		strcpy (buf + l, findData.cFileName);

		// Recurse here:
		process_name (
					name, 
					buf,
					out_name_buf,
					ndir_part, 
					blist_only, 
					brecurse
					);

	} while (::FindNextFile (hfind, &findData));
	::FindClose (hfind);
}
///////////////////////////////////////////////////////////////////////////////
void process_volume (const char *pvol, char *out_name_buf, int ndir_part, bool blist_only)
{
   MyVStream vol;

   if(!vol.openVolume(pvol))
   {
	   printf ("Error: could not open volume file %s.\n", pvol);
      return;
   }


   for (int i = 0; i < vol.getNoNamedFiles(); i++)
   {

	   if (blist_only)
	   {
			printf ("%s is in volume %s\n", vol.getItemName(i), pvol);
	   }
	   else
	   {
		   FileRWStream f = FileRWStream();

		   strcpy (out_name_buf + ndir_part, vol.getItemName(i));

		   if (::GetFileAttributes(out_name_buf) != 0xffffffff)
		   {
				printf ("Warning: file %s already exists.\n", out_name_buf);
		   }

		   if (f.open(out_name_buf))
		   {
			   vol.open(vol.getItemName(i));
   
			   Int8 c;
			   int k = 0;
  
			   while(vol.read(&c))
			   {
				  f.write(c);
				  k++;
			   }
   
			   vol.close();
			   f.close();
			   printf ("%s of %i bytes created from volume.\n", vol.getItemName(i), k, pvol);
		   }
		   else
		   {
			   printf ("Error: could not create output file %s from volume %s\n", vol.getItemName(i), pvol);
		   }
	}
   }
   
   vol.closeVolume();
}
///////////////////////////////////////////////////////////////////////////////
