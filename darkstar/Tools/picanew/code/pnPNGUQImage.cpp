//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------
#include <stdio.h>
#include "pnPNGUQImage.h"
#include "pnPopTable.h"
#include <png.h>

//class PNPNGUQImage : public PNUnquantizedImage {
//  public:
//   PNPNGUQImage();
//   ~PNPNGUQImage();
//
//   void registerColors(PNPopularityTable& in_rPopTable,
//                       const UInt32       in_weight);
//   bool loadFile(const char* in_pFileName);
//};

PNPNGUQImage::PNPNGUQImage()
 : m_pFinalPixels(NULL),
   m_hasAlpha(false)
{
   //
}

PNPNGUQImage::~PNPNGUQImage()
{
   delete [] m_pFinalPixels;
   m_pFinalPixels = NULL;

   m_hasAlpha = false;
}

const PALETTEENTRY*
PNPNGUQImage::getFinalPixels() const
{
   return m_pFinalPixels;
}

void
PNPNGUQImage::registerColors(PNPopularityTable& in_rPopTable,
                             const UInt32       in_weight)
{
   AssertFatal(m_pFinalPixels != NULL, "No image loaded yet...");

   if (m_hasAlpha == true) {
      in_rPopTable.registerRGBAColors(m_pFinalPixels,
                                      getWidth() * getHeight(),
                                      in_weight);
   } else {
      in_rPopTable.registerRGBAColors(m_pFinalPixels,
                                      getWidth() * getHeight(),
                                      in_weight);
   }
}

bool
PNPNGUQImage::loadFile(const char* in_pFileName)
{
   //----------------------------------------------------
   // Sample PNG Reading code taken from LIBPNG example.c
   //----------------------------------------------------

   // Open the file for reading...
   //
   FILE* fp = fopen(in_pFileName, "rb");
   if (fp == NULL)
      return false;

   // Check the signature on the PNG file...
   //
   char buf[8];
   if (fread(buf, 1, 8, fp) != 8) {
      fclose(fp);
      return false;
   }
   if (png_sig_cmp(buf, (png_size_t)0, 8) != 0) {
      fclose(fp);
      return false;
   }


   /* Create and initialize the png_struct with the desired error handler
    * functions.  If you want to use the default stderr and longjump method,
    * you can supply NULL for the last three parameters.  We also supply the
    * the compiler header file version, so that we know if the application
    * was compiled with a compatible version of the library.  REQUIRED
    */
   png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
                                                NULL, NULL, NULL);

   if (png_ptr == NULL) {
      fclose(fp);
      return false;
   }

   /* Allocate/initialize the memory for image information.  REQUIRED. */
   png_infop info_ptr = png_create_info_struct(png_ptr);
   if (info_ptr == NULL) {
      fclose(fp);
      png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
      return false;
   }
   
   /* Set error handling if you are using the setjmp/longjmp method (this is
    * the normal method of doing things with libpng).  REQUIRED unless you
    * set up your own error handlers in the png_create_read_struct() earlier.
    */
   if (setjmp(png_ptr->jmpbuf)) {
      /* Free all of the memory associated with the png_ptr and info_ptr */
      png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
      fclose(fp);
      /* If we get here, we had a problem reading the file */
      return false;
   }

   /* Set up the input control if you are using standard C streams */
   png_init_io(png_ptr, fp);

   /* we have already read some of the signature */
   png_set_sig_bytes(png_ptr, 8);


   png_uint_32 width, height;
   int bit_depth, color_type, interlace_type;

   /* The call to png_read_info() gives us all of the information from the
    * PNG file before the first IDAT (image data chunk).  REQUIRED
    */
   png_read_info(png_ptr, info_ptr);

   png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
       &interlace_type, NULL, NULL);


   /**** Set up the data transformations you want.  Note that these are all
    **** optional.  Only call them if you want/need them.  Many of the
    **** transformations only work on specific types of images, and many
    **** are mutually exclusive.
    ****/
   

   /* Extract multiple pixels with bit depths of 1, 2, and 4 from a single
    * byte into separate bytes (useful for paletted and grayscale images).
    */
   png_set_packing(png_ptr);

   /* Expand paletted colors into true RGB triplets */
   if (color_type == PNG_COLOR_TYPE_PALETTE)
      png_set_expand(png_ptr);
   
   /* Expand paletted or RGB images with transparency to full alpha channels
    * so the data will be available as RGBA quartets.
    */
   if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
      png_set_expand(png_ptr);

//   /* Add filler (or alpha) byte (before/after each RGB triplet) */
//   png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);

   /* Use RGB triples for greyscales... */
   if (color_type == PNG_COLOR_TYPE_GRAY ||
       color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
      png_set_gray_to_rgb(png_ptr);

   /* tell libpng to strip 16 bit/color files down to 8 bits/color */
   png_set_strip_16(png_ptr);

   /* read the updated info structure */
   png_read_update_info(png_ptr, info_ptr);

   /* The easiest way to read the image: */
   if (png_get_rowbytes(png_ptr, info_ptr) != width * 4) {
      AssertWarn(0, avar("bd: %d, ct: %d, w/h (%d/%d)", bit_depth, color_type, width, height));

      AssertFatal(png_get_rowbytes(png_ptr, info_ptr) == width * 4,
                  avar("huh? rowbytes wrong on %s (%d: expected %d)",
                       in_pFileName,
                       png_get_rowbytes(png_ptr, info_ptr),
                       width * 4));
   }

   png_bytep* row_pointers = new png_bytep[height];
   for (png_uint_32 row = 0; row < height; row++) {
      row_pointers[row] = new png_byte[png_get_rowbytes(png_ptr, info_ptr)];
   }

   /* Now it's time to read the image.  One of these methods is REQUIRED */
   png_read_image(png_ptr, row_pointers);

   /* read rest of file, and get additional chunks in info_ptr - REQUIRED */
   png_read_end(png_ptr, info_ptr);

   //--------------------------------------
   //--------------------------------------
   // COPY THE DATA INTO THE PN Format...
   //--------------------------------------
   m_hasAlpha = (color_type & PNG_COLOR_MASK_ALPHA) != 0;
   setImageDimensions(width, height);
   m_pFinalPixels = new PALETTEENTRY[getWidth() * getHeight()];

   UInt32 currentPixelIndex = 0;
   for (UInt32 y = 0; y < getHeight(); y++) {
      png_bytep currentRow = row_pointers[y];
      for (UInt32 x = 0; x < getWidth(); x++) {
         m_pFinalPixels[currentPixelIndex].peRed   = currentRow[(x * 4) + 0];
         m_pFinalPixels[currentPixelIndex].peGreen = currentRow[(x * 4) + 1];
         m_pFinalPixels[currentPixelIndex].peBlue  = currentRow[(x * 4) + 2];
         
         if (m_hasAlpha)
            m_pFinalPixels[currentPixelIndex].peFlags = currentRow[(x * 4) + 3];
         else
            m_pFinalPixels[currentPixelIndex].peFlags = 0;

         currentPixelIndex++;
      }
   }
   AssertFatal(currentPixelIndex == getWidth() * getHeight(), "Huh?");

   // Delete the row pointers, we're done with them..
   //
   for (UInt32 i = 0; i < getHeight(); i++) {
      delete [] row_pointers[i];
      row_pointers[i] = NULL;
   }
   delete [] row_pointers;

   /* clean up after the read, and free any memory allocated - REQUIRED */
   png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);

   /* close the file */
   fclose(fp);

   setImageName(in_pFileName);
   return true;
}

