#include "stdafx.h"
#include "detail.h"

//--------------------------------------------------------------------

Bool IsTransparent( GFXBitmap * pBmp )
{
   // does this bitmap use color 0, the transparent color?
   for( int y = 0; y < pBmp->getHeight(); y++ )
      {
      BYTE * pPixels = pBmp->getAddress( 0, y );
      for( int x = 0; x < pBmp->getWidth(); x++ )
         if( *pPixels++ == 0 )
            return TRUE;
      }
   return FALSE;
}

//--------------------------------------------------------------------

Material::Material()
{
   fName[0] = 0;
   fRawTextureFile[0] = 0;
   fTextureFile[0] = 0;
   fR = 0;
   fG = 0;
   fB = 0;
   fSmooth = 0;
   fTwoSided = 0;
   fTextured = 0;
   fTextureHasAlpha = 0;
   fTransparent = 0;
   fPerspectiveCorrect = 0;
   fTSMaterialIndex = -1;
	fUsePaletteIndex = 0;
	fPaletteIndex = 0;
}

Material::Material( material3ds *mat3ds, const char *pPath, Int32 detailIndex )
{
   fRawTextureFile[0] = 0;
   fTextureFile[0] = 0;
   fAlphaFile[0] = 0;
   fSmooth = 0;
   fTwoSided = 0;
   fTextured = 0;
   fTextureHasAlpha = 0;
   fTransparent = 0;
   fPerspectiveCorrect = 0;
   fTSMaterialIndex = -1;
	fUsePaletteIndex = 0;
	fPaletteIndex = 0;

   fName[0] = detailIndex + 1;
   strcpy( fName+1, mat3ds->name );
	if( strchr( fName, '#' ) && !strlen( mat3ds->texture.map.name ) )
		{
		fUsePaletteIndex = 1;
		fPaletteIndex = atoi( strchr( fName, '#' ) + 1 );
		}
   fR = (UInt8)(mat3ds->diffuse.r * 255);
   fG = (UInt8)(mat3ds->diffuse.g * 255);
   fB = (UInt8)(mat3ds->diffuse.b * 255);
   fTranslucency = mat3ds->transparency;
   if( mat3ds->selfillumpct > 0.75 )
      fUnlit = 1;
   else
      {
      fUnlit = 0;
      if( mat3ds->shading > 1 )
         fSmooth = 1;
      }
   fTwoSided = mat3ds->twosided;
   if( strlen( mat3ds->texture.map.name ) )
      {
      fTextured = 1;
      fPerspectiveCorrect = strchr( fName, '#' ) ? 1 : 0;
      strcpy( fRawTextureFile, pPath );
      strcat( fRawTextureFile, mat3ds->texture.map.name );
      if( mat3ds->opacity.map.name[0] )
         {
         fTextureHasAlpha = 1;
         strcpy( fAlphaFile, pPath );
         strcat( fAlphaFile, mat3ds->opacity.map.name );
         }
      }
}

//--------------------------------------------------------------------

int Material::operator==( const Material& m )
{
	if( strcmp( fName, m.fName ) )
		return FALSE;

   // no need to test transparent flag or sbd since
   // these are determined by the texture bitmap

	if( ( fR != m.fR ) || ( fG != m.fG ) || ( fB != m.fB ) )
		return FALSE;
	if( fUnlit != m.fUnlit ||
	   fTranslucency != m.fTranslucency ||
	   fSmooth != m.fSmooth || 
		fTwoSided != m.fTwoSided ||
		fTextured != m.fTextured ||
		fTextureHasAlpha != m.fTextureHasAlpha ||
		fPerspectiveCorrect != m.fPerspectiveCorrect ||
		fUsePaletteIndex != m.fUsePaletteIndex ||
		fPaletteIndex != m.fPaletteIndex )
      return FALSE;

   if( fTextured )
      {
   	if( strcmp( fRawTextureFile, m.fRawTextureFile ) )
	   	return FALSE;
      if( fTextureHasAlpha && strcmp( fAlphaFile, m.fAlphaFile ) )
         return FALSE;
      }

   return TRUE;
}

//--------------------------------------------------------------------

void Material::Dump()
{
	static char s[200];

   // write out a summary of the material:
   sprintf( s, "Add Material: %-18s  \r\n", fName );
   if( fUnlit )
      strcat( s, "unlit " );
   if( fSmooth )
      strcat( s, "smooth " );
   else
      strcat( s, "flat   " );

   if( fTextured )
      {
      if( fTransparent )
         strcat( s, "transparent " );
      if( fPerspectiveCorrect )
         strcat( s, "perspective-correct " );
      if( fTextureHasAlpha )
         strcat( s, "alpha " );
      strcat( s, "texture: " );
      strcat( s, fRawTextureFile );
      }
   else if( fUsePaletteIndex )
      {
      char r[100];
      sprintf( r, "Palette: %d", fPaletteIndex );
      strcat( s, r );
      }
	else
      {
      char r[100];
      sprintf( r, "RGB: %d %d %d", fR, fG, fB );
      strcat( s, r );
      }
   sprintf( s + strlen( s ), " translucency: %f", fTranslucency );
   StatusPrintf( s );
}

//--------------------------------------------------------------------

TSMaterial * Material::MakeTSMaterial( TSMaterial *TSMat ) const
{
   memset( TSMat, 0, sizeof( TSMaterial ) );

   if( fUnlit )
      TSMat->fLighting = TSMaterial::LightingUnlit;
   else if( fSmooth )
      TSMat->fLighting = TSMaterial::LightingSmooth;
   else
      TSMat->fLighting = TSMaterial::LightingFlat;

   TSMat->fAlpha = fTranslucency;

   if( fTextured )
      {
      TSMat->fType = TSMaterial::MatTexture;
      TSMat->fTextureType = TSMaterial::TexturePlain;

      if( fTransparent )
         TSMat->fTextureType = TSMaterial::TextureType( TSMat->fTextureType | 
            TSMaterial::TextureTransparent );

      if( fPerspectiveCorrect )
         TSMat->fTextureType = TSMaterial::TextureType( TSMat->fTextureType | 
            TSMaterial::TexturePerspective );

      if( strchr( fTextureFile, '\\' ) )
         strcpy( TSMat->fMapFile, strrchr( fTextureFile, '\\' ) + 1);
      else
         strcpy( TSMat->fMapFile, fTextureFile );

      if( fTextureHasAlpha )
			{
			TSMat->fTextureType = TSMaterial::TextureType( TSMat->fTextureType | 
            TSMaterial::TextureTranslucent );
			}
      }
   else if( fUsePaletteIndex )
		{
		TSMat->fType = TSMaterial::MatPalette;
		TSMat->fIndex = fPaletteIndex;
		}
	else
      {
      TSMat->fType = TSMaterial::MatRGB;

      TSMat->fRGB.fRed = fR;
      TSMat->fRGB.fGreen = fG;
      TSMat->fRGB.fBlue = fB;
      }
   return TSMat;
}

//--------------------------------------------------------------------

void DetailList::Alphatize( GFXBitmap *src, GFXBitmap *alpha )
{
   double scale =  fAlphaNumLevels / 256.0;
   
   for(int j = 0; j < src->getHeight(); j++)
   {
      BYTE *sBits = src->getAddress(0, j);
      BYTE *aBits = alpha->getAddress(0, j);
      for(int i = 0; i < src->getWidth(); i++)
      {
         if( int(aBits[i] * scale) == 0)
            sBits[i] = 0;
         else
            sBits[i] += (BYTE)(int((255-aBits[i]) * scale) * fAlphaNumColors);
      }
   }
}

//--------------------------------------------------------------------

void DetailList::AddMaterial( Material &m )
{
   char s[100];

   // handle too many materials:

	if( fnMats == MAX_MATS )
      throw GeneralError( "Too many materials!" );

   // handle duplicate materials:

   for( int i = 0; i < fnMats; i++ )
      if( !strcmp( m.fName, faMats[i].fName ) )
         {
         if( !(faMats[i] == m) )
            {
            sprintf( s, "Material '%s' conflicts with prior material '%s'",
               m.fName, faMats[i].fName );
            throw GeneralError( s );
            }

         // identical duplicate found
         // return without adding duplicate:
         return;
         }

   // if textured, get the actual texture bitmap and add to sheets:
   if( m.fTextured )
      {
      strcpy( m.fTextureFile, m.fRawTextureFile );
      GFXBitmap * pBmp = GFXBitmap::load( m.fRawTextureFile );
      if( pBmp )
         {
         // if texture is translucent, pack the alpha info into it:
         if( m.fTextureHasAlpha )
            {
            GFXBitmap *pAlpha = GFXBitmap::load( m.fAlphaFile );
            if( !pAlpha )
               {
               delete pBmp;
               sprintf( s, "Alpha file missing: '%s'", m.fAlphaFile );
               throw GeneralError( s );
               }
            Alphatize( pBmp, pAlpha );
            delete pAlpha;

            // save alphatized bitmap with .pab extension:
            if( strchr( m.fTextureFile, '.' ) )
               strcpy( strchr( m.fTextureFile, '.' ), ".pab" );
            else
               strcat( m.fTextureFile, ".pab" );
            pBmp->write( m.fTextureFile );
            }
         m.fTransparent = IsTransparent( pBmp );
         delete pBmp;
         }
      else
         {
         sprintf( s, "Texture file missing: '%s'", m.fRawTextureFile );
         throw GeneralError( s );
         }
      }

   // See if TSMaterial is identical to another:
   m.MakeTSMaterial( &faTSMats[fnTSMats] );
   for( int tm = 0; tm < fnTSMats; tm++ )
      if( !memcmp( &m, &faTSMats[tm], sizeof( TSMaterial ) ) )
         {
         // if same as a previous TSMaterial, don't duplicate
         m.fTSMaterialIndex = tm;
         break;
         }

   // allow no duplicates in TSMaterials:
   if( m.fTSMaterialIndex == -1 )
      m.fTSMaterialIndex = fnTSMats++;

   // add all materials here which are unique (if only by name) in 3dStudio:
   faMats[fnMats++] = m;

   // display material attributes:
   //m.Dump();
}

//--------------------------------------------------------------------

void DetailList::GetMaterials()
{
   Bool success = TRUE;
   fnMats = 0;
           
   Material m;
   AddMaterial( m );

   for( int di = 0; di < fnDetails; di++ )
      success = success && faDetails[di].GetMaterials( this );
   if( !success )
      throw GeneralError( "Unable to continue!" );
}

//--------------------------------------------------------------------

void DetailList::SaveMaterials()
{
	//----------------------------------------------------------------
   // Write out the material list:
   //

   if( fnTSMats )
      {
      TSMaterialList * ml = new TSMaterialList( fnTSMats );
      if( ml )
         {
         // install materials in list:
         for( int i = 0; i < fnTSMats; i++ )
            (*ml)[i] = faTSMats[i];

         // get filename and write material list:
	   	char outFile[256];
		   sprintf(outFile, "%s.%s", fBaseName, MAT_LIST_EXT);
         StatusPrintf( "Writing material list: %s", outFile );
         remove( outFile );
         FileWStream ost( outFile );
         ml->writeItem( ost );

         // we no longer need the material list:
         delete ml;
         }
      else
         StatusPrintf( "Not enough memory to write material list!" );
      }
}

//--------------------------------------------------------------------

int DetailList::FindMaterial( char *name )
{
   // search through our material list for the material with the
   // given name, and if found, return the index, else return 0.
   // (Not really, since if it's not found it is an error.)

   for( int i = 0; i < fnMats; i++ )
      {
      if( !strcmp( name, faMats[i].fName ) )
         return i;
      }
   throw GeneralError( "Undefined material in mesh!" );
   return 0;
}

int DetailList::FindMaterial( int face, mesh3ds *mesh, Int32 detailIndex )
{
   // finds the name of the material used by the face
   // then finds the index of that material in our material list
   // If the face is not associated with any material, it uses
   // the default material.  So we just return 0.

   for( int i = 0; i < mesh->nmats; i++ )
      {
      for( int j = 0; j < mesh->matarray[i].nfaces; j++ )
         if( mesh->matarray[i].faceindex[j] == face )
            {
            char  n[18];
            n[0] = detailIndex + 1;
            strcpy( n+1, mesh->matarray[i].name );
            return FindMaterial( n );
            }
      }
   // face uses default material, so we will use material 0
   return 0;
}

//--------------------------------------------------------------------

Int16 DetailList::GetMaterialTSIndex( int index )
{
   AssertFatal( index >= 0 && index < fnMats, "Invalid material index" );
   return faMats[index].fTSMaterialIndex;
}

//--------------------------------------------------------------------

const Material & DetailList::GetMaterial( int index )
{
   AssertFatal( index >= 0 && index < fnMats, "Invalid material index" );
   return faMats[index];
}

//--------------------------------------------------------------------
