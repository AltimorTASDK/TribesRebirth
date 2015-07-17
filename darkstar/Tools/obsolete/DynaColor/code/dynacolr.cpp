#include <stdio.h>       
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <direct.h>
#include <DOS.h>
#include <IO.h>

#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#include "types.h"
#include "globs.h"
#include "bmp.h"
#include "tga.h"


static char StartDir[MAX_PATH];
static char OutDir[MAX_PATH];
static char OutFile[MAX_PATH];

char  fDrive [_MAX_DRIVE],
      fDir   [_MAX_DIR],
      fFile  [_MAX_FNAME],
      fExt   [_MAX_EXT];

static u32 NumFiles = 0;
static int ChooseFlag = 0;
static int RenderFlag = 1;
static int OverFlag = 0;
static int QuietFlag = 0;
static char msgbuff[ 255 ];
static bmptype PalType;
static int ObtainedPalType = 0;

void initAVI(void);
void exitAVI(void);
// Borland 5.02 can't compile vfw.h  
//void loadAVI(u8* sourceFile,u8* renderFile);

// prototypes from choose.c
u8 findClosestMatch(u8colorVec *colorPtr);
void scanForColors(void);
void chooseColors(void);
u8 findShould(u8colorVec *color,u8 would,u32 x,u32 y);
void popularityScan(void);
void kill(s32 red,s32 green,s32 blue);
void popularityChoose(void);

static bool isPalFile( const char *finame );
static u8* SavePal( const char *finame, bmptype* pBMP );
static u8* LoadPal( const char *finame, bmptype* bmp,void* pal,void* data);

void error( const char *format, ... )
 {
  va_list argp;

  va_start( argp, format );
  vsprintf( msgbuff, format, argp );
  va_end( argp );

  fprintf( stderr, "ERROR: %s\n", msgbuff );
// Borland 5.02 can't compile vfw.h  
//  exitAVI();
  exit( 2 );
 }



void message( const char *format, ... )
 {
  if( !QuietFlag )
  {
   va_list argp;
  
   va_start( argp, format );
   vsprintf( msgbuff, format, argp );
   va_end( argp );
  
   printf( "%s", msgbuff );
  }
 }


// compares two colors and returns 
//   Neg if c_1 is less intense then c_2
//   0 if they are the same intensity
//   Pos if c_1 is more intense then c_2
// uses a grayscale formula to determine the
// intensity of a R,G,B color.
int color_cmp(const void *c_1, const void *c_2) 
{
   u8colorVec *c1, *c2;
   f32 intensity_1, intensity_2;
   
   c1 = (u8colorVec *)c_1;
   c2 = (u8colorVec *)c_2;
 
   intensity_1 = 0.299f * (f32)c1->r + 0.587f * (f32)c1->g + 0.114f * (f32)c1->b; 
   intensity_2 = 0.299f * (f32)c2->r + 0.587f * (f32)c2->g + 0.114f * (f32)c2->b; 
 
   return (int)(intensity_2 - intensity_1);
}

void choosePalette()
{
  u32 i;

  message("Choosing 256 color palette...\n");

  Rendering=0;
  BaseOffset=ChooseBaseOffset;
  TotalColors=ChooseTotalColors*(100-PercentageForPopularity)/100;
  for (i=0;i<17;i++)
    ColorRange[i]=TotalColors*i*PercentageForRegions/1600+BaseOffset;

  chooseColors();
  if (TotalColors>0)
   for (i=BaseOffset;i<BaseOffset+TotalColors;i++)
     kill(Pal[i].r,Pal[i].g,Pal[i].b);
 
  BaseOffset=ChooseBaseOffset+ChooseTotalColors*(100-PercentageForPopularity)/100;
  TotalColors=ChooseTotalColors-ChooseTotalColors*(100-PercentageForPopularity)/100;
  popularityChoose();  

  if (SortPalette) {
    qsort(Pal + ChooseBaseOffset, ChooseTotalColors, sizeof(u8colorVec), color_cmp);   
  }
}


void renderColors(u8* name)
{
   u8 *maskPtr;
   u8 *destPtr;
   u32 x,y;
   u8colorVec color;
   u32 i;
   u32 paddedWidth = (Width+3)&0xFFFFFFFC; // RST

   if ( RenderData )
      free (RenderData);
   RenderData=(u8 *)malloc(paddedWidth*Height); // RST
   if (RenderData==NULL)
      error("out of memory",0);

   Pointer=Data;
   maskPtr=Mask;
   destPtr=RenderData;
   Seed=0;
   prevErrorRed=prevErrorGreen=prevErrorBlue=0;
   for (y=0;y<Height;y++) {
      if ((y&31)==0) {
         message("Rendering %s...%ld%% done.%-20\r",name,(y+ 1)*100/(Height),"");
      } 
      for (x=0;x<Width;x++) {
         u8 would;
      
         color.b=*Pointer++;
         color.g=*Pointer++;
         color.r=*Pointer++;

         if (zeroColorOn)
         {
            if ((color.b == zeroColor.b) && (color.g == zeroColor.g) && (color.r == zeroColor.r)) 
            {
               *destPtr++=0;
               continue;
            }
         }

         if (!Mask || *maskPtr++) {
            would=findClosestMatch(&color);
            *destPtr++=findShould(&color,would,x,y);
         }  
         else
            *destPtr++=0;
      }
      destPtr+=(paddedWidth-Width);      // RST
      Pointer+=(PaddedWidth-(Width*3));  // RST
   }

   for (i=0;i<256;i++) {
      *(RenderPal+i*4)=0;
      *(RenderPal+i*4+1)=0;
      *(RenderPal+i*4+2)=0;
   }
 
   for (i=BaseOffset;i<BaseOffset+TotalColors;i++) {
      *(RenderPal+i*4)=Pal[i].b;
      *(RenderPal+i*4+1)=Pal[i].g;
      *(RenderPal+i*4+2)=Pal[i].r;
   }
}

void loadPal(u8* fName)
{
   bmptype type1;
   u8 tmpPal [1024];
   u32 i;
   u8* err;

   if (LoadAPalette == 1)
     return;

   LoadAPalette=1;
   //ChooseAPalette=0;

   if (RenderABitmap==0)
      message("Loading 256 color palette: %s\n",fName);

   if ( isPalFile(fName) )
      err = LoadPal(fName,&type1,tmpPal,0);
   else  // else load palette from .bmp
      err = LoadBmp(fName,&type1,tmpPal,0);

   if ( err ) error (err);
   if ( type1.bi.biBitCount != 8 )
      error ("%s: palette bmp must be 8-bit",fName);

   for (i=0;i<256;i++) {
      Pal[i].b=*(tmpPal+i*4);
      Pal[i].g=*(tmpPal+i*4+1);
      Pal[i].r=*(tmpPal+i*4+2);
   }

   FreeBmp(&type1);
}


void doBmp(u8* name)
{
   u32 i;

   if (RenderABitmap==1) {
      for (i=0;i<(256*256*32);i++)
         Memory[i]=65535;


      Rendering=1;
      BaseOffset=RenderBaseOffset;
      TotalColors=RenderTotalColors;
      renderColors(name);
   }
   else
   if (ChooseAPalette==1) {
      message("Scanning %s\r",name);
      // scan the original colors
      scanForColors();
      popularityScan();

      if (ApplyShade) {
         u8 *data_copy;

         // if we are also applying the haze,
         // we need to save the original data
         // and restore it after we are done
         // applying shade
         if (ApplyHaze) {
            data_copy = (u8 *)malloc(PaddedWidth * Height);
            if (data_copy == NULL) 
               error("Could not allocate memory");
            memcpy(data_copy, Data, PaddedWidth * Height); 
         }
      
         blend_uniform(&Shade);
         scanForColors(); 
         popularityScan();
       
         if (ApplyHaze) {
            memcpy(Data, data_copy, PaddedWidth * Height);
            free(data_copy);
         }
      }

      if (ApplyHaze) {
         blend_uniform(&Haze);
         scanForColors();
         popularityScan();
      }
      
   }
}                                               


void loadBmp(u8* sourceName,u8* destName)
 {
  bmptype type;
  u8* err;

  err = LoadBmp(sourceName,&type,0,0);
  if ((PalOutName) && (! ObtainedPalType))
  {
      ObtainedPalType = 1;
      memcpy(&PalType,&type,sizeof(bmptype));
  
  }

  if ( err )
   error (err);
  if ( type.bi.biBitCount != 24 )
   error ("%s: source bmp must be 24-bit",sourceName);

  Data=type.data;
  Mask=0;
  Width=type.bi.biWidth;                             
  Height=type.bi.biHeight;
  PaddedWidth = (((Width*3)+3)&0xFFFFFFFC); // because file is padded - RST

  doBmp (sourceName);
  if (RenderABitmap==1)
   {
    bmptype type2 = type;
    message("Saving %-40s\n",destName);
    
    err = InitBmp(&type2,8,RenderPal,RenderData);
    if ( err )
     error(err);
    err = SaveBmp(destName,&type2);
    if ( err )
     error(err);

    FreeBmp(&type2);
   }
  else
    message("\n");

  FreeBmp(&type);
 }


void loadTga(u8* sourceName,u8* destName)
 {
  TGA tga;
  u8* err;

  LoadTGA(sourceName,&tga);
  Data=tga.Data;
  Mask=0;
  Width=tga.Head.ImageWidth;
  Height=tga.Head.ImageHeight;
  PaddedWidth = Width*3; // because file is NOT padded - RST

  doBmp (sourceName);
  if (RenderABitmap==1)
   {
    bmptype type2;
    type2.bi.biSize = 40;
    type2.bi.biWidth = tga.Head.ImageWidth;
    type2.bi.biHeight = tga.Head.ImageHeight;
    type2.bi.biPlanes = 1;
    type2.bi.biBitCount = 8;
    type2.bi.biCompression = 0;
    type2.bi.biSizeImage = tga.Head.ImageWidth * tga.Head.ImageHeight;
    type2.bi.biXPelsPerMeter = 0;
    type2.bi.biYPelsPerMeter = 0;
    type2.bi.biClrUsed = 256;
    type2.bi.biClrImportant = 256;
    type2.widthBytes = tga.Head.ImageWidth;
    type2.numColors = 256;
    type2.palFreeFlag = 0;
    type2.pal = 0;
    type2.dataFreeFlag = 0;
    type2.data = 0;

    message("Saving %-40s\n",destName);

    err = InitBmp(&type2,8,RenderPal,RenderData);
    if ( err )
     error(err);
    err = SaveBmp(destName,&type2);
    if ( err )
     error(err);
    FreeBmp(&type2);
   }
  else
    message("\n");

  FreeTGA(&tga);
 }


void initGamma(void)
 {
  u32 i;

  Gamma[0] = 0;
  for (i = 1; i < 256; i++)
    Gamma[i] = (s32)(pow((f32)i/255.0f,GammaVal) * 255.0f);
 }


void parseArg(char* argPtr)
 {
  s32 first,last;
  s32 r, g, b, start_intensity;
  static int doOnce = 1;

  if ( *argPtr == '-'  ||  *argPtr == '/' )
  {
   argPtr++;
   if ( strnicmp (argPtr,"out:",4) == 0 )
    {
     strncpy ( OutDir, &argPtr[4], sizeof(OutDir) );
    }
   else
//   if ( strnicmp (argPtr,"over",4) == 0 )
//    {
//     OverFlag = (argPtr[4]!='-');
//    }
//   else
   if ( strnicmp (argPtr,"palOut:",7) == 0 )
    {
      PalOutName = (char *)malloc(strlen(&argPtr[7]) + 1);
      if (PalOutName == NULL) 
         error("Out of memory");
      strcpy(PalOutName,&argPtr[7]);
    }
   else
   if ( strnicmp (argPtr,"pal:",4) == 0 )
    {
      loadPal (&argPtr[4]);
    }
   else
      if ( strnicmp (argPtr,"choose",6) == 0 )
    {
      switch (argPtr[6]) 
      {
         case '(':
            sscanf  ( &argPtr[6], "(%d-%d)", &first, &last );
            if ( first < 0 || last < 0 || last < first || first>255 || last>255 )
               error ( "Invalid colors: %s", argPtr );
            ChooseBaseOffset=first;
            ChooseTotalColors=last-first+1;
            ChooseFlag=1;
            ChooseAPalette=1;
            break;
         case '-':
            ChooseBaseOffset=0;
            ChooseTotalColors=0;
            ChooseFlag=0;
            ChooseAPalette=0;
            break;
         default:
            error("Invalid format: %s",argPtr);
      }
    }
   else
   if ( strnicmp (argPtr,"render",6) == 0 )
    {
       switch (argPtr[6])
      {
         case '(':
            sscanf  ( &argPtr[6], "(%d-%d)", &first, &last );
            if ( first < 0 || last < 0 || last < first || first>255 || last>255 )
               error ( "Invalid colors: %s", argPtr );

            RenderBaseOffset=first;
            RenderTotalColors=last-first+1;
            RenderFlag = 1;
            break;
         case '- ':
            RenderFlag = 0;
            break;
         default:
            error("Invalid format: %s",argPtr);
      }
    }
   else
   if ( strnicmp (argPtr,"windows",7) == 0 )
    {
     ChooseAPalette=1;
     ChooseBaseOffset=10;
     ChooseTotalColors=236;
     ChooseFlag=1;

     RenderBaseOffset=10;
     RenderTotalColors=236;
    }
   else
   if ( strnicmp (argPtr,"dither",6) == 0 )
    {
     sscanf  ( &argPtr[6], "(%d-%d)", &first, &last );
     if ( first < 0 || last < 0 || last < first )
      error ( "Invalid dither: %s", argPtr );

     MinDither=first;
     MaxDither=last;
    }
   else
   if ( strnicmp (argPtr,"frames",6) == 0 )
    {
     sscanf  ( &argPtr[6], "(%d-%d)", &first, &last );
     if ( first < 0 || last < 0 || last < first )
      error ( "Invalid frames: %s", argPtr );

     MinAviFrame=first;
     MaxAviFrame=last;
    }
   else
   if ( strnicmp (argPtr,"exact",5) == 0 )
    {
     ExactMatchPassThrough = (argPtr[5]!='-');
    }
   else
   if ( strnicmp (argPtr,"quiet",5) == 0 )
    {
     QuietFlag = (argPtr[5]!='-');
    }
   else
   if ( strnicmp (argPtr,"black",5) == 0 )
    {
     sscanf  ( &argPtr[5], "(%d)", &first );
     if ( first < 0 )
      error ( "Invalid black threshold: %s", argPtr );

     BlackThreshold=first;
    }
   else
   if ( strnicmp (argPtr,"method",6) == 0 )
    {
     sscanf  ( &argPtr[6], "(%d)", &first );
     if ( first < 0 )
      error ( "Invalid method: %s", argPtr );

     Method=first;
    }
   else
   if ( strnicmp (argPtr,"regions",7) == 0 )
    {
     sscanf  ( &argPtr[7], "(%d)", &first );
     if ( first < 0 || first > 100 )
      error ( "Invalid percentage for regions: %s", argPtr );

     PercentageForRegions=first;
    }
   else
   if ( strnicmp (argPtr,"popularity",10) == 0 )
    {
     sscanf  ( &argPtr[10], "(%d)", &first );
     if ( first < 0 || first > 100 )
      error ( "Invalid percentage for popularity: %s", argPtr );

     PercentageForPopularity=first;
    }
   else
   if ( strnicmp (argPtr,"rot",3) == 0 )
    {
     int cnt = sscanf  ( &argPtr[3], "(%d)", &first );
     if ( cnt != 1 )
      error ( "Invalid rotate frames: %s", argPtr );

     RotateFrames=first;
    }
   else
   if ( strnicmp (argPtr,"trim",4) == 0 )
    {
     int cnt = sscanf  ( &argPtr[4], "(%d)", &first );
     if ( cnt != 1 )
      error ( "Invalid trim frames: %s", argPtr );

     TrimFrames=first;
    }
   else
   if ( strnicmp (argPtr,"cut",3) == 0 )
    {
     sscanf  ( &argPtr[3], "(%d)", &first );
     if ( first < 0 )
      error ( "Invalid cut threshold: %s", argPtr );

     CutFlag=1;
     CutThreshold=first;
    }
   else
   if ( strnicmp (argPtr,"compress",8) == 0 )
    {
     CompressFlag = (argPtr[8]!='-');
    }
   else
   if ( strnicmp (argPtr,"sort",4) == 0)
    {
      SortPalette = 1;
    }
   else  
   if ( strnicmp (argPtr,"kill",4) == 0 )
    {
      int cnt;
      cnt = sscanf (&argPtr[4], "(%d,%d,%d)", &r, &g, &b);
      
      if (cnt != 3)
        error("Invalid color value: %s", argPtr);
      if ( (r < 0) || (r > 255) || (g < 0) || (g > 255) || (b < 0) || (b > 255) )
       error("Invalid color component value: %s\n range is 0-255", argPtr);
   
      IgnoreColorOn = 1;
      IgnoreColor.r = r;
      IgnoreColor.g = g;
      IgnoreColor.b = b;
    }
   else
   if ( strnicmp (argPtr,"zeroColor",9) == 0 )
    {
      int cnt;
      cnt = sscanf (&argPtr[9], "(%d,%d,%d)", &r, &g, &b);
      
      if (cnt != 3)
        error("Invalid color value: %s", argPtr);
      if ( (r < 0) || (r > 255) || (g < 0) || (g > 255) || (b < 0) || (b > 255) )
       error("Invalid color component value: %s\n range is 0-255", argPtr);
   
      zeroColorOn = 1;
      zeroColor.r = r;
      zeroColor.g = g;
      zeroColor.b = b;
    }
   else
   if ( strnicmp (argPtr,"gamma",5) == 0 )
    {
      int cnt;
      cnt = sscanf (&argPtr[5], "(%f)",&GammaVal);
      
      if ((cnt != 1) || (GammaVal < 0)) 
        error("Invalid gamma value: %s", argPtr);
         
      initGamma();
    }
   else
   if ( strnicmp (argPtr,"shade",5) == 0)
    {
     int cnt = sscanf( &argPtr[5], "(%d,%d,%d,%d)", &r, &g, &b, &start_intensity);
     if (cnt != 4) 
       error("Invalid shade format: %s\n (note: don't use spaces)", argPtr);
     if ( (r < 0) || (r > 255) || (g < 0) || (g > 255) || (b < 0) || (b > 255) )
       error("Invalid color component value: %s\n range is 0-255", argPtr);
     if ( (start_intensity < 0) || (start_intensity > 100) )
       error("Invalid intensity value: %s\n range is 0-100", argPtr);
 
     ApplyShade = 1;
     Shade.r = (u8)r;
     Shade.g = (u8)g;
     Shade.b = (u8)b;
     Shade.start_intensity = (float)start_intensity / 100.0f;
    }
   else
   if ( strnicmp (argPtr,"haze",4) == 0)
    {
     int cnt = sscanf( &argPtr[4], "(%d,%d,%d,%d)", &r, &g, &b, &start_intensity);
     if (cnt != 4) 
       error("Invalid haze format: %s\n (note: don't use spaces)", argPtr);
     if ( (r < 0) || (r > 255) || (g < 0) || (g > 255) || (b < 0) || (b > 255) )
       error("Invalid color component value: %s\n range is 0-255", argPtr);
     if ( (start_intensity < 0) || (start_intensity > 100) )
       error("Invalid intensity value: %s\n range is 0-100", argPtr);
 
     ApplyHaze = 1;
     Haze.r = (u8)r;
     Haze.g = (u8)g;
     Haze.b = (u8)b;
     Haze.start_intensity = (float)start_intensity / 100.0f;
    }
   else
    error ( "Unknown switch: %s", argPtr );
  }
  else
   {
      WIN32_FIND_DATA data;
      HANDLE hFind = FindFirstFile(argPtr, &data);
      if (hFind != INVALID_HANDLE_VALUE)
      {
         do
         {
            u8 SourceFile [_MAX_PATH];
            u8 DestFile [_MAX_PATH];


            _splitpath (argPtr,fDrive,fDir,NULL,NULL);
            _splitpath (data.cFileName,NULL,NULL,fFile,fExt);
            _makepath (SourceFile,fDrive,fDir,fFile,fExt);

            if (RenderABitmap)
            {
               if (doOnce)
               {
                  doOnce = 0;

                  // Create Directory
                  mkdir (OutDir);
                  chdir (OutDir);
                  getcwd (OutDir,sizeof(OutDir));
                  chdir (StartDir);
               }
               sprintf(DestFile,"%s\\%s",OutDir,data.cFileName);

               if (OverFlag == 0)
               {
                  FILE *file;
                  if ((file = fopen(DestFile, "rb")) != NULL);
                  {
                     fclose(file);
                     error("cannot over-write file: %s (use -over)", DestFile);
                  }
                  
               }
            }
            else
            {
               DestFile[0] = 0;
            }

            NumFiles++;
            if (ChooseAPalette || RenderABitmap)
            {
               if ( !stricmp (fExt,".bmp") )
               {
                  loadBmp (SourceFile,DestFile);
               }
               else if ( !stricmp (fExt,".tga") )
               {
                  if (RenderABitmap)
                  {
                     // Force DestFile extention to .bmp
                     _splitpath (DestFile,fDrive,fDir,fFile,NULL);
                     strcpy (fExt,".bmp");
                     _makepath (DestFile,fDrive,fDir,fFile,fExt);
                  }

                  loadTga (SourceFile,DestFile);
               }
               else if ( !stricmp (fExt,".avi") )
               {
// Borland 5.02 can't compile vfw.h  
//                  loadAVI (SourceFile,DestFile);
               }
               else
               {
                  error ("unknown file type: %s",argPtr);
               }
            }

         } while (FindNextFile(hFind,&data));

         FindClose(hFind);
      }
      else
         error("cannot find file: %s",argPtr);
   }
}


void parseFile(char *fileName)
{
   FILE *filePtr = fopen(fileName,"rt");
   char  fileBuff [200];

   if (!filePtr)
      error("Unable to open list file: %s", fileName);

   while(fgets (fileBuff, sizeof (fileBuff)-1, filePtr)) {
      char *ptr;
      int len;

      // Strip any comments
      if ((ptr = strstr (fileBuff,"//")) != NULL)
         *ptr = 0;
   
      // Strip trailing whitespace
      for (len = strlen (fileBuff)-1; len > 0 && fileBuff[len] <= ' ';  len--)
         fileBuff [len] = 0;

      // Strip leading whitespace
      for (ptr = fileBuff; *ptr && *ptr <= ' '; ptr++)
         ;

      // Check for two args on a line
      if (strchr (ptr,' ') != NULL)
         error("Only one arg per line (%s)", fileBuff);

      if (strlen(ptr))
         parseArg(ptr);
   }

   fclose(filePtr);
}


void usage(void)
{
   puts("Usage: Dynacolr [opts] <files>");
   puts("  convert artwork from 24-bit to 8-bit");
   puts("Args:");
   puts("  -quiet[-]              be quiet");
   puts("  -out:<out-dir>         set output directory (default=out)");
   //   puts("  -over[-]               overwrite existing output files");
   puts("  -pal:<bmp-file>        load existing 256 color palette");
   puts("  -palOut:<bmp-file>     writes the new palette to the specified");
   puts("                           bitmap file.");
   puts("                           If no path is specified, then the file");
   puts("                           is saved to the -out directory");
   puts("  -choose(a-b)           choose palette colors a-b");
   puts("  -choose-               do not choose any palette colors");
   puts("  -render(a-b)           render palette colors a-b");
   puts("  -render-               do not render any palette colors");
   puts("  -windows               same as -choose(10-245) -render(10-245)");
   puts("  -dither(a-b)           set dither range a-b");
   puts("  -frames(a-b)           set avi frame range a-b");
   //  puts("  -cut(n)                set avi frame cut threshold");
   //  puts("  -rot(n)                set avi frame rotation");
   //  puts("  -trim(n)               set avi frame trim");
   puts("  -gamma(a)              set gamma correction value");
   puts("                          default is 2.35");
   puts("                          set to 1.0 to disable gamma correction");
   puts("  -sort                  sorts the palette colors being rendered");
   puts("  -zeroColor(r,g,b)      maps all specified (r,g,b) in source");
   puts("                          to zero color (index 0)");
   puts("  -kill(r,g,b)           ignores the specified color");     
   puts("  -shade(r,g,b,i)        blend (r,g,b) shade with intensity (i)");
   puts("                          over the image before reducing colors");
   puts("  -haze(r,g,b,i)         blend (r,g,b) haze with intensity (i)");
   puts("                          over the image before reducing colors"); 
   puts("  -exact[-]              set exact match pass through flag");
   puts("  -black(n)              set black threshold (default=0)");
   puts("  -method(n)             set method number (default=3)");
   puts("  -regions(n)            set percentage for regions (default=50)");
   puts("  -popularity(n)         set percentage for popularity (default=50)");
   puts("  <file.bmp>             convert microsoft bitmap");
   puts("  <file.avi>             convert AVI movie");
   puts("  @<file>                read args from file");
   puts("Examples:");
   puts("  dynacolr foo.bmp       convert 24-bit bmp to 256 color");
   puts("  dynacolr foo.avi       convert 24-bit avi to 256 color");

// Borland 5.02 can't compile vfw.h  
//   exitAVI();
   exit( 1 );
}


void main(u32 argC, u8* argV[])
{
   u32 i,j;

   strcpy(OutDir,"out");
   getcwd(StartDir,MAX_PATH);

// Borland 5.02 can't compile vfw.h  
//   initAVI();
   puts("DynaColr 1.2, 10/10/96, Copyright(c) 1996, Rhett Anderson, All Rights Reserved.");

   MinDither=1;
   MaxDither=18;
   BlackThreshold=0;
   Method=3;
   PercentageForRegions=50;
   PercentageForPopularity=50;
   ExactMatchPassThrough=1;
   LoadAPalette=0;
   ChooseAPalette=1;

   RotateFrames=0;
   TrimFrames=0;
   CutFlag=0;
   CompressFlag=1;
  
   MinAviFrame = 0;
   MaxAviFrame = 9999;

   ChooseBaseOffset=0;
   ChooseTotalColors=256;
  
   RenderBaseOffset=0;
   RenderTotalColors=256;

   SortPalette = 0;
   ApplyShade = 0;
   ApplyHaze = 0;    
   GammaVal=2.35f;
   IgnoreColorOn = 0;
   zeroColorOn = 0;
   OverFlag = 1;
   RenderFlag = 1;

   PalOutName = NULL;

   initGamma();

   for (i=0;i<(256*256*32);i++)
      Memory[i]=0;
  
   for (i=0;i<(64*64*64);i++)
      Memory2[i]=(f32)(0);

   RenderABitmap=0;
   for ( j = 0; j < 2; j++ ) {
      NumFiles = 0;
      for ( i = 1; i < argC; i++ ) {
         if ( argV[i][0] == '@' )
            parseFile(&argV[i][1]);
         else
            parseArg(argV[i]);
      }

      if (NumFiles == 0)
         usage();

      if (RenderABitmap==0 && ChooseAPalette==1) 
         choosePalette ();
   
      if (PalOutName) 
      {
         char *palPath;
         u8  *err;

         if (! ObtainedPalType)
            error("Can not produce a palette when no data was scanned");

         if ((NULL == strrchr(PalOutName, '\\')) && (NULL == strrchr(PalOutName,'/'))) 
         {  
            palPath = (char *)malloc(strlen(PalOutName) + strlen(OutDir) + 2);
            if (palPath == NULL)
               error("Out of memory");
            sprintf(palPath,"%s\\%s",OutDir,PalOutName);
         }
         else 
         {
            palPath = (char *)malloc(strlen(PalOutName) + 1);
            if (palPath == NULL)
               error("Out of memory");
            strcpy(palPath,PalOutName);
         }
  
         message("Saving the palette file: %s",palPath);
         PalType.bi.biWidth = 100;
         PalType.bi.biHeight = 2;
         
         for (i=0;i<256;i++) {
            *(RenderPal+i*4)=Pal[i].b;
            *(RenderPal+i*4+1)=Pal[i].g;
            *(RenderPal+i*4+2)=Pal[i].r;
         }

         err = InitBmp(&PalType,8,RenderPal,NULL);
         if (err) error(err);

         if ( isPalFile(palPath) )
            err = SavePal( palPath, &PalType );
         else  // save as an MS .bmp
            err = SaveBmp(palPath,&PalType);

         if (err) error(err);
         FreeBmp(&PalType);
         free(palPath);
         free(PalOutName);
         PalOutName = NULL;
      } 

      if (RenderFlag)
         RenderABitmap=1;
      else
         break;
   }  

   message("Done.\n");
// Borland 5.02 can't compile vfw.h  
//   exitAVI();
   exit( 0 );
}

//--------------------------------------------------------------------------
// determines if filename has a .pal extension

bool  isPalFile( const char *finame )
{
   char *pChr = strrchr( (char*)finame, '.' );
   if ( pChr )
      return ( !strcmpi( pChr, ".pal" ) );
   else
      return ( false );
}   

//--------------------------------------------------------------------------
// saves Microsoft .Pal file

struct RiffHdr
{
   char  hdr[4];
   int   fsize;
   char  typ[4];
   char  dta[4];
   int   dsize;
   int   version;
};

u8* SavePal( const char *finame, bmptype* pBMP )
{
   PALETTEENTRY tmpPal [1024];
   FILE *fp;
   RiffHdr palette = { "RIFF",1040,"PAL ","data",4*256+4,0x1000300};

   if ((fp = fopen(finame,"wb")) == NULL)
      return("can't open dest ");

   for ( int i=0; i<256; i++ )
   {
      tmpPal[i].peBlue  = pBMP->pal[i].b;
      tmpPal[i].peGreen = pBMP->pal[i].g;
      tmpPal[i].peRed   = pBMP->pal[i].r;
      tmpPal[i].peFlags = NULL;
   }

   fwrite (&palette,sizeof(RiffHdr),1,fp);
   fwrite(tmpPal,sizeof(PALETTEENTRY),256,fp);

   fclose(fp);
   return(0);
}   

//--------------------------------------------------------------------------
// loads a Micrsoft .Pal file

u8* LoadPal( const char *fname, bmptype* bmp,void* pal,void* data)
{

   data;                      // unused parameter
   PALETTEENTRY tmpPal [256];
   PALETTEENTRY *dcPal = (PALETTEENTRY*)pal;
   FILE *fp;
   RiffHdr palette;

   if ((fp = fopen(fname,"rb")) == NULL)
      return("can't open file ");

   fread ( &palette, sizeof(palette), 1, fp );
   fread ( &tmpPal, sizeof(PALETTEENTRY), 256, fp );

   for ( int i=0; i<256; i++ )
   {
      dcPal[i].peBlue  = tmpPal[i].peRed;
      dcPal[i].peGreen = tmpPal[i].peGreen;
      dcPal[i].peRed   = tmpPal[i].peBlue;
      dcPal[i].peFlags = NULL;
   }

   bmp->bi.biBitCount = 8;    // fake out their bit-depth detection
   bmp->dataFreeFlag = 0;     // didn't allocate any bits
   bmp->palFreeFlag = 0;      // didn't allocate palette

   fclose(fp);
   return(0);
}

