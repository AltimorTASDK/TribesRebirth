//北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北
//北   
//北  $Workfile:   ObjStrip.cpp  $
//北  $Author:     Robert Mobbs $
//北  $Revision:   1.00  $
//北    
//北  DESCRIPTION:
//北        Utility to strip data from OBJ files.  The program
//北        will determine, on the fly, whether the OBJ file is
//北        COFF or OMF format.
//北        
//北  (c) Copyright 1998, Dynamix Inc.   All rights reserved.
//北 
//北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北

#include <stdio.h>     // standard utilities
#include <time.h>      // for formatting COFF timedate
#include <filstrm.h>   // to handle file I/O
#include <objstrip.h>  // local include

//北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北
// the main program

int main(int argc, char *argv[])
{
    int  iIndex = 1;

    // If we have more than one argument we need to check for
    // the verbose flag
    if (argc > 1)
    {
        if ((argv[1][0] == '-' || argv[1][0] == '/') &&
            (argv[1][1] == 'v' || argv[1][1] == 'V'))
        {
            iIndex = 2;
        }
    }

    // If we're out of arguments, print usage
    if (iIndex == argc)
    {
        (void)printf("\nUtility to strip data segments from OBJ files.\n");
        (void)printf("usage: %s [-v] <infile> [outfile]\n", argv[0]);
        return (1);
    }

    // Otherwise strip
    return strip(argv[iIndex], (((iIndex + 1) == argc) ? 
                 NULL : argv[iIndex + 1]), (iIndex > 1));
}

//北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北北
// the main decomposing function

int strip(LPTSTR lpszInFile, LPTSTR lpszOutFile, BOOL fVerbose)
{
    int           offset = 0;
    int           _segOffset;
    char          szBuf[4096];
    BYTE          len;
    int           _size;
    bool          deps = FALSE;
    LPTSTR        lpszBuffer;
    OMFHEADER     omfheader;
    COMENT        coment;
    SECTIONHEADER sectionheader;
    COFFHEADER    coffheader;
    PUBDEF        pubdef;
    LEDATA        ledata;
    FileRStream   frs;
    FileWStream   fws;

    if (FALSE == frs.open(lpszInFile))
    {
        (void)printf("fatal: unable to open input file \"%s\"\n",
            lpszInFile);
        return (1);
    }

    if (lpszOutFile)
    {
        if (FALSE == fws.open(lpszOutFile))
        {
            (void)printf("fatal: unable to create output file \"%s\"\n",
                lpszOutFile);
        }
    }

    // We're going to try to guess what format this file is
    frs.read(sizeof(OMFHEADER), &omfheader);

    // If the first record is an OMF THEADR or LHEADR, then this file
    // is an OMF file
    if (omfheader.type == OMF_TYPE_THEADR || omfheader.type == OMF_TYPE_LHEADR)
    {
        frs.read(sizeof(BYTE), &len);
        frs.read(len + 1,      &szBuf);

        if (fVerbose) 
        {
            (void)printf("FILETYPE: OMF\n%6.6X THEADR  %s\n", offset, szBuf);
        }

        while(frs.read(sizeof(OMFHEADER), &omfheader))
        {
            switch (omfheader.type)
            {
                case OMF_TYPE_COMENT:
                    frs.read(sizeof(COMENT), &coment);
                    _size = 0;

                    switch(coment.Class)
                    {
                        case COMENT_TYPE_TRANSLATOR:
                            frs.read(sizeof(BYTE), &_size);
                            frs.read(_size + 1, szBuf);
                            if (fVerbose) 
                            {
                                (void)printf("%6.6X COMENT  class: %3.3Xh,"
                                    "  Compiler:  %s\n", offset, 
                                    coment.Class, szBuf);
                            }
                            break;

                        case COMENT_TYPE_SOURCEFILE:
                            frs.read(sizeof(BYTE), &szBuf);
                            frs.read(sizeof(BYTE), &_size);
                            frs.read(_size + 1, szBuf);

                            szBuf[_size] = 0;

                            frs.read(sizeof(DWORD), &_size);
                            if (fVerbose) 
                            {
                                (void)printf("%6.6X COMENT  class: %3.3Xh,"
                                             " Source File:  %s\n", offset, 
                                             coment.Class, szBuf);
                            }
                            break;

                        case COMENT_TYPE_DEPENDFILE:
                            if (omfheader.size == 3)
                            {
                                frs.read(sizeof(BYTE), &_size);
                            }
                            else
                            {
                                frs.read(sizeof(DWORD), szBuf);
                                frs.read(sizeof(BYTE), &_size);
                                frs.read(_size + 1, szBuf);
                                if (!deps)
                                {
                                    if (fVerbose) 
                                    {
                                        (void)printf("%6.6X COMENT  class: "
                                            "%3.3Xh, Dependencies\n", offset, 
                                            coment.Class);
                                    }
                                    deps = TRUE;
                                }
                                if (fVerbose) 
                                {
                                    (void)printf("               file:  %s\n", 
                                        szBuf);
                                }
                            }
                            break;

                        default:
                            if (fVerbose) 
                            {
                                (void)printf("            Unknown Comment Type");
                            }

                            frs.setPosition(frs.getPosition() + 
                                            omfheader.size - 2);
                    }
                    break;

                case OMF_TYPE_PUBDEF:
                    frs.read(sizeof(PUBDEF), &pubdef);
                    frs.read(pubdef.size, szBuf);
                    frs.read(sizeof(DWORD), &_segOffset);
                    
                    szBuf[pubdef.size] = 0;
                    if (fVerbose) 
                    {
                        (void)printf("%6.6X PUBDEF  %-15.30s  Seg:%i, Offset:"
                            "%4.4X\n", offset, szBuf, pubdef.seg, _segOffset);
                    }
                    break;

                case OMF_TYPE_LEDATA:
                    frs.read(sizeof(LEDATA), &ledata);
                    lpszBuffer = new char[omfheader.size];
                    frs.read(omfheader.size - sizeof(LEDATA), lpszBuffer);
                    if (fVerbose) 
                    {
                        (void)printf("%6.6X LEDATA                   "
                            "Seg:%i, Offset:%4.4X  Size:%i\n", offset, 
                            ledata.segdef, ledata.offset, 
                            omfheader.size - 4);
                    }
                    if (lpszOutFile)
                    {
                        fws.write(omfheader.size - 4, lpszBuffer);
                    }
                    delete [] lpszBuffer;
                    break;

                case OMF_TYPE_LNAMES:
                    if (fVerbose) 
                    {
                        (void)printf("%6.6X LNAMES\n", offset);
                    }
                    frs.setPosition(frs.getPosition() + omfheader.size);
                    break;

                case OMF_TYPE_SEGDEF:
                    if (fVerbose) 
                    {
                        (void)printf("%6.6X SEGDEF\n", offset);
                    }
                    frs.setPosition(frs.getPosition() + omfheader.size);
                    break;

                case OMF_TYPE_GRPDEF:
                    if (fVerbose) 
                    {
                        (void)printf("%6.6X GRPDEF\n", offset);
                    }
                    frs.setPosition(frs.getPosition() + omfheader.size);
                    break;

                case OMF_TYPE_MODE32:
                    if (fVerbose) 
                    {
                        (void)printf("%6.6X MODE32\n", offset);
                    }
                    frs.setPosition(frs.getPosition() + omfheader.size);
                    break;
      
                default:
                    if (fVerbose) 
                    {
                        (void)printf("%6.6X UNDEFINED BLOCK ID:%3.3X\n", 
                            offset, omfheader.type);
                    }
                    frs.setPosition(frs.getPosition() + omfheader.size);
                    break;
            }
            offset += omfheader.size;
        }
    }

    else
    {
        // Go back to the beginning of the file to get COFF header
        frs.setPosition(0);
        frs.read(sizeof(COFFHEADER), &coffheader);

        // Print the time at which this file was created
        if (fVerbose)
        {
            (void)printf("FILETYPE: COFF\nDATETIME: %s\n", 
                         strupr(asctime(localtime(&coffheader.f_timdat))));
        }

        offset = sizeof(COFFHEADER);

        // Read all of the sections
        while ((coffheader.f_nscns --) > 0)
        {
            // Get next section
            frs.read(sizeof(SECTIONHEADER), &sectionheader);

            // If we are in verbose mode, print this section's info
            if (fVerbose)
            {
                (void)printf("SECT: %6.6X\tTYPE: %s\tDATA: %6.6X\tSIZE: %4d\n",
                    offset, strupr(sectionheader.s_name), 
                    sectionheader.s_scnptr, sectionheader.s_size);
            }

            // Keep track of position in file
            offset += sizeof(SECTIONHEADER);

            // The only entry with the IMAGE_SCN_LNK_INFO flag set is
            // the .DRECTVE entry, which can be ignored
            if (!(sectionheader.s_flags & IMAGE_SCN_LNK_INFO) &&
                 (lpszOutFile != NULL) &&
                 (sectionheader.s_scnptr > 0) && (sectionheader.s_size > 0))
            {
                // Skip to this section's data segment
                frs.setPosition(sectionheader.s_scnptr);
                
                // Chunk the data
                lpszBuffer = new char[sectionheader.s_size];
                frs.read (sectionheader.s_size, lpszBuffer);
                fws.write(sectionheader.s_size, lpszBuffer);
                delete [] lpszBuffer;

                // Move to the next section
                frs.setPosition(offset);
            }
        }
    }
    return (0);
}