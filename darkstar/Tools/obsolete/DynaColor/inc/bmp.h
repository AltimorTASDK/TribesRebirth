#ifndef BMP_H
#define BMP_H   1

typedef struct {
  u8 b;   
  u8 g;
  u8 r;
  u8 reserved; /* should be 0 */
} bmprgb;

#include <pshpack2.h>
typedef struct {
  s16 bfType;
  s32 bfSize;
  s16 bfReserved1;
  s16 bfReserved2;
  u32 bfOffBits;
} bmpfileheader;
#include <poppack.h>

typedef struct {
  s32 biSize;
  s32 biWidth;
  s32 biHeight;
  s16 biPlanes;
  s16 biBitCount;
  s32 biCompression;
  s32 biSizeImage;
  s32 biXPelsPerMeter;
  s32 biYPelsPerMeter;
  s32 biClrUsed;
  s32 biClrImportant;
} bmpinfoheader;

typedef struct {
  bmpinfoheader bi;
  u32 widthBytes;
  u32 numColors;
  u32 palFreeFlag;
  bmprgb* pal;
  u32 dataFreeFlag;
  u8* data;
} bmptype;

u8* InitBmp(bmptype* bmp,u32 bits,void* pal,void* data);
u8* LoadBmp(u8* fname,bmptype* bmp,void* pal,void* data);
u8* SaveBmp(u8* fname,bmptype* bmp);
void FreeBmp(bmptype* bmp);

#endif /* BMP_H */
