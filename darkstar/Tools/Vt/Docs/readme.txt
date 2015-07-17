**********************************************************************
*                                                                    *
* VT.EXE                                                             *
*                                                                    *
* Volume Tool                                                        *
*                                                                    *
* Last updated 12/27/96                                              *
*                                                                    *
**********************************************************************


Purpose:
--------
To create/modify volume files.

How to use it:
--------------

vt [-n<# of TOC entries>] [compression] <vol file> <add file>

When a volume file is created, the number of table of contents entries
is set.  If no number is specified, a default value of 1000 will be used.
If the volume already exists, any number of TOC entries specified will be
ignored.

VT will not create an empty volume, so a file must be added
when the volume is created.

Each entry in a volume file use a compression method specified by the
following switches:
        compression     switch
        none            (none required - this is the default)
        LZ              -lz
        LZH             -lzh
        RLE             -rle

**********************************************************************
* Vt          Volume Tool v1/23/96                                   *
*                                                                    *
* usage: vt [options] volume [@]file                                 *
*                                                                    *
* options:                                                           *
*  -sp        strip the path of the file when inserting into volume. *
*  -lzh       Set the compression type to LZH                        *
*  -rle       Set the compression type to RLE                        *
*  -cpt       Compact the volume into a new volume named file.       *
*  -nopbm     don't convert .dib's or .bmp's to phoenix bitmaps      *
*  -q         turn on quiet mode. No messages output                 *
*  -------------------------------------------------------------     *
*                                                                    *
*  -dml[#bmp_path]  adds the bitmaps from dml file to volume         *
*                                                                    *
*  -mip[#n[#palette_file]] invokes MIPMAKE to mip the bmp            *
*                                                                    *
*  [@] @ symbol before addfile implies addfile is a script file      *
*       of args to pass to vt (omit volume on all lines).            *
*                                                                    *
*                                                                    *
**********************************************************************
*                                                                    *
* Adds the file to the volume.  If the volume does not exist, it is  *
* created.                                                           *
*                                                                    *
**********************************************************************


History
-------

12/27/96 New switches

-q
Quiet mode. Disables error reporting and other output.

-dml[#bmp_path]
Process DML file. Extracts the bitmap names from a DML file and adds them
to the volume. bmp_path specifies the path to the bitmaps, otherwise the
bitmaps are assumed to be in the same directory as the DML file.

-mip[#n[#palette_file]]
MIPMAKEs a bitmap to a temporary file before adding it to the volume. n and
palette_file are optional arguments for MIPMAKE and are explained by MIPMAKE.

[@]file
Optional @ in fron of file implies file is a script file where each line is
is of the form:
        [options] file

instead of the usual:
        [options] volume file



2/5/96

Added bitmap conversion stuff so it saves bitmaps as GFX in volume.
