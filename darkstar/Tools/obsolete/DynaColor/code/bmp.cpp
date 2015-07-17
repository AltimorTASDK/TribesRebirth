#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include "dy_types.h"
#include "bmp.h"
#include "globs.h"

u8* InitBmp(bmptype* bmp,u32 bits,void* pal,void* data)
{
  if (bits == 8)
    bmp->numColors = 256;
  else if (bits == 24)
    bmp->numColors = 0;
  else
    return("support for 8 or 24 bits only ");

  bmp->widthBytes = ((bmp->bi.biWidth*bits/8+3)&0xfffc); /* pad to multiple of 4 */
  bmp->bi.biBitCount = (s16)bits;
  bmp->bi.biSizeImage = bmp->widthBytes * bmp->bi.biHeight;
  bmp->bi.biClrUsed = bmp->numColors;
  bmp->bi.biClrImportant = bmp->numColors;

  if (pal || !bmp->numColors) {
    bmp->palFreeFlag = 0;
    bmp->pal = (bmprgb *)pal;
  }
  else {
    bmp->palFreeFlag = 1;
    bmp->pal = (bmprgb*)malloc(sizeof(bmprgb)*bmp->numColors);
    if (bmp->pal == 0) {
      return("can't allocate image pal ");
    }
  }

  if (data) {
    bmp->dataFreeFlag = 0;
    bmp->data = (unsigned char *)data;
  }
  else {
    bmp->dataFreeFlag = 1;
    bmp->data = (u8*)malloc(bmp->bi.biSizeImage);
    if (bmp->data == 0) {
      return("can't allocate image data ");
    }
  }

  return(0);
}


void FreeBmp(bmptype* bmp)
{
  if (bmp->pal) {
    if (bmp->palFreeFlag)
     free(bmp->pal);
    bmp->pal = 0;
  }
  if (bmp->data) {
    if (bmp->dataFreeFlag)
     free(bmp->data);
    bmp->data = 0;
  }
}


u8* LoadBmp(u8* fname,bmptype* bmp,void* pal,void* data)
{
  FILE *fp;
  bmpfileheader bf;
  u8 *err;


  if ((fp = fopen(fname,"rb")) == NULL)
    return("can't open file ");

  // read file header
  fread ( &bf, sizeof(bf), 1, fp );

  // read bitmap info
  fread ( &bmp->bi, sizeof(bmp->bi), 1, fp );

  if ( bmp->bi.biSize != 40 )
    return("bad header size ");
  if (bmp->bi.biCompression != 0)
    return("can't process compressed files ");

  err = InitBmp(bmp,bmp->bi.biBitCount,pal,data);
  if (err)
    return(err);

  /* Read in the color table (if any) */
  if (bmp->numColors)
    fread(bmp->pal,sizeof(bmprgb),bmp->numColors,fp);

  fseek(fp,bf.bfOffBits,SEEK_SET);
  fread(bmp->data,1,bmp->bi.biSizeImage,fp);

  fclose(fp);
  return(0);
}


u8* SaveBmp(u8* fname,bmptype* bmp)
{
  FILE *fp;
  bmpfileheader bf;

  if ((fp = fopen(fname,"wb")) == NULL)
    return("can't open dest ");

  bf.bfType = MAKEWORD('B','M');
  bf.bfReserved1 = 0;
  bf.bfReserved2 = 0;
  bf.bfOffBits = sizeof (bf) + bmp->bi.biSize +
      sizeof(bmprgb) * bmp->numColors;
  bf.bfSize = bf.bfOffBits + bmp->bi.biSizeImage;

  fwrite (&bf,sizeof(bf),1,fp);
  fwrite (&bmp->bi,sizeof(bmp->bi),1,fp);
  if (bmp->numColors)
    fwrite(bmp->pal,sizeof(bmprgb),bmp->numColors,fp);
  fwrite(bmp->data,1,bmp->bi.biSizeImage,fp);

  fclose(fp);
  return(0);
}
