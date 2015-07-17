// 
// bmpinfo.cpp
//

#include <stdio.h>
#include <g_bitmap.h>
#include <g_pal.h>

int main(int argc, char **argv)
{
	GFXBitmap  *bitmap;
	if (argc > 2)
		bitmap = GFXBitmap::load(argv[1]);
	else
		bitmap = GFXBitmap::load(argv[1], BMF_INCLUDE_PALETTE);

	if (!bitmap)
	{
		fprintf(stderr, "BMPINFO: Unable to load \"%s\"\n", argv[1]);
		return(0);
	}
	fprintf(stderr, "width:         %d\n"
	                "height:        %d\n"
					"stride:        %d\n"
					"bitDepth:      %d\n"
					"imageSize:     %d\n"
					"detailLevels:  %d\n",
					bitmap->width,
					bitmap->height,
					bitmap->stride,
					bitmap->bitDepth,
					bitmap->imageSize,
					bitmap->detailLevels);
	if (bitmap->pBits)
		fprintf(stderr, "pBits:         Valid\n");
	else
		fprintf(stderr, "pBits:         NULL\n");
		
	if (bitmap->pPalette)
{
		fprintf(stderr, "Palette:       Included\n");
bitmap->pPalette->write("testpal.ppl");
}
	else
		fprintf(stderr, "Palette:       Not Included\n");
}
