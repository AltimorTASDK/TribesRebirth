//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _GOGLTX_H_
#define _GOGLTX_H_

//Includes
#include <windows.h>
#include <gl/gl.h>
#include <base.h>
#include "g_types.h"
#include "gOGLSfc.h"

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

class  GFXBitmap;
struct GFXBitmap::CacheInfo;
struct GFXLightMap;
class  GFXPalette;

namespace OpenGL {

//------------------------------------------------------------------------------
//-------------------------------------- TextureCache
//
class Surface::TextureCache
{
   static char const* const sm_pMultiTextureExtStringARB;
   static char const* const sm_pMultiTextureExtStringSGI;
   static char const* const sm_pPackedPixelsExtString;
   static char const* const sm_pEdgeClampString;

   static UInt32 sm_cacheMagic;

   // Extension function structures.
   //
  public:
   struct {
      void (__stdcall* glMTexCoord2fv) (GLenum, GLfloat*);
      void (__stdcall* glMTexCoord4fv) (GLenum, GLfloat*);
      void (__stdcall* glSelectTexture)(GLenum);
   } m_multiTexExtSGI;

   struct {
      void (__stdcall* glMTexCoord2fv) (GLenum, GLfloat*);
      void (__stdcall* glMTexCoord4fv) (GLenum, GLfloat*);
      void (__stdcall* glSelectTexture)(GLenum);
   } m_multiTexExtARB;

   bool m_supportsPackedPixels;
   bool m_supportsEdgeClamp;

   enum GeneralState {
      Disabled = 0,
      Enabled,
      Undetermined
   };
  
  private:
   void dumpOGLTextureNormal(const GFXBitmap*, const bool clamp);
   void dumpOGLTextureNormalDyn(const GFXBitmap*, const bool clamp);
   void dumpOGLTextureAlpha(const GFXBitmap*, const bool clamp);
   void dumpOGLTextureTransparent(const GFXBitmap*, const bool clamp);
   void dumpOGLLightmap(GFXLightMap*);

   void refreshOGLTextureNormal(const GFXBitmap*, const bool clamp);
   void refreshOGLTextureAlpha(const GFXBitmap*, const bool clamp);
   void refreshOGLTextureTransparent(const GFXBitmap*, const bool clamp);

   void dumpOGLTexturePacked(const GFXBitmap*, bool transparent, const bool clamp);
   void dumpOGLTexturePackedDyn(const GFXBitmap*, bool transparent, const bool clamp);
   void refreshOGLTexturePacked(const GFXBitmap*, bool transparent, const bool clamp);
   void dumpOGLLightmapPacked(GFXLightMap*);

   bool isFromCurrentRound(const GFXBitmap::CacheInfo&) const;
   bool isFromCurrentRoundLM(const GFXBitmap::CacheInfo&) const;
   bool isCurrentDynamic(const GFXBitmap::CacheInfo& in_rCI) const;
   GLuint getTexName(const GFXBitmap::CacheInfo& in_rCI) const;

   GLuint chooseDynamicName(GFXBitmap::CacheInfo& in_rCacheInfo,
                            const UInt32          in_arenaIndex);

  public:
   void touchEntryIfNecessary(GFXBitmap::CacheInfo& in_rCacheInfo);

  protected:
   static UInt32 sm_currFlushNum;

   UInt32        m_currFrameKey;

   GeneralState m_texUnitState[2];
   GLenum       m_texUnitMode[2];
   GLenum       m_srcBlend;
   GLenum       m_dstBlend;
   GeneralState m_transpEnabled;
   GeneralState m_zTestEnabled;
   GeneralState m_zWriteEnabled;
   GLenum       m_zMode;
   GeneralState m_fogEnabled;

   int          m_currentTexUnit;
   int          m_currentTexName[2];
   void setTexUnit(int u);

   UInt8*       m_pTranslationBuffer;

   typedef Vector<GLuint> TextureNameVector;
   TextureNameVector m_currNormalNames;

   struct DynamicCacheEntry {
      DynamicCacheEntry* pPrev;
      DynamicCacheEntry* pNext;

      UInt32             frameKey;

      UInt32             wrapNumber;
      GLuint             texName;
   };

   typedef Vector<DynamicCacheEntry> DCEntryVector;
   
   static const UInt32 csm_sizeIndices[6];
   static const UInt32 csm_cacheDSize[6];
   
   DCEntryVector     m_dynamicEntries[6];

   DynamicCacheEntry m_freeListSentrys[6];
   DynamicCacheEntry m_inuseSentryHeads[6];
   DynamicCacheEntry m_inuseSentryTails[6];
   
   void refreshOGLTexture(const GFXBitmap*);

   DynamicCacheEntry* getCacheEntry(const GFXBitmap::CacheInfo& in_rCI);
   UInt32             getEntryArena(const GFXBitmap::CacheInfo& in_rCI);
   UInt32             getArena(const UInt32 in_dim) const;

   //-------------------------------------- Palette elements
  public:
   void setPalette(GFXPalette* in_pPalette);
   void setGamma(const float in_gamma);

   struct MPCacher {
      GLubyte data[256*4];
      GLubyte dataTransparent[256*4];
      GLushort dataPacked[256];
      GLushort dataPackedTransparent[256];
      GLenum  packedType;
      GLenum  packedTransparentType;
      Int32   paletteIndex;
   };

   float       m_gamma;

  private:
   GFXPalette* m_pPalette;

   MPCacher m_multiPalettes[16];

   MPCacher* getMPCache(const Int32 in_palIndex);

   void determineExtensions();

   Surface* m_pSurface;
  public:
   TextureCache(Surface*);
   ~TextureCache();

   bool supportsSGIMultiTexture() const { return m_multiTexExtSGI.glMTexCoord2fv != NULL; }
   bool supportsARBMultiTexture() const { return m_multiTexExtARB.glMTexCoord2fv != NULL; }
   bool supportsPackedPixels()    const { return m_supportsPackedPixels;                  }
   bool supportsEdgeClamp()       const { return m_supportsEdgeClamp;                     }

   void cacheBitmap(const GFXBitmap*      in_pBitmap,
                    GFXBitmap::CacheInfo& in_rCacheInfo,
                    int                   in_texUnit,
                    const bool            in_clampMode);
   void refreshBitmap(const GFXBitmap*      in_pBitmap,
                      GFXBitmap::CacheInfo& in_rCacheInfo);
   bool setTexture(GFXBitmap::CacheInfo& in_rCacheInfo, int in_texUnit = 0);

   bool isCurrent(GFXBitmap::CacheInfo& in_rCacheInfo);
   bool isCurrentLM(GFXBitmap::CacheInfo& in_rCacheInfo);

   // For lightmaps
   //
   bool setLightmap(GFXBitmap::CacheInfo& in_rCacheInfo, int in_texUnit = 0);
   void cacheLightmap(GFXLightMap*          io_pLightmap,
                      GFXBitmap::CacheInfo& in_rCacheInfo,
                      int                   in_texUnit);

   void flushLightMap(GFXBitmap::CacheInfo& in_rCacheInfo);

   void enableTexUnits(const bool in_texUnit0,
                       const bool in_texUnit1 = false);
   void setTexMode(const GLenum in_mode0,
                   const GLenum in_mode1 = GL_REPLACE);
   void setBlendMode(const GLenum in_srcFactor,
                     const GLenum in_dstFactor);
   void setTransparent(const bool in_transp);
   void enableZBuffering(const bool   in_enableTest,
                         const bool   in_enableWrite,
                         const GLenum in_compare);
   void enableFog(const bool in_enable);

   void flushCache();
   void clearState();   

   void incFrameKey() { m_currFrameKey++; }

   // querys
  public:
   bool isTexturing() const { return m_texUnitState[0] == Enabled; }
};


//------------------------------------------------------------------------------
//-------------------------------------- HandleCache
//
struct HandleCacheEntry
{
   enum HandleFlags { LightMapValid = 1 };

   GFXTextureHandle  handle;
   UInt32            flags;
   HandleCacheEntry* hashLink;
   HandleCacheEntry* next;
   HandleCacheEntry* prev;

   Point2F coordScale, coordTrans;

   Point2F m_col0;
   Point2F m_col1;
   Point2F m_col2;

   bool isTerrain;
   UInt32 frameKey;

   const GFXBitmap* pBmp;
   GFXLightMap*     pLMap;
   int              size;
   GFXCacheCallback cb; // if it has one of these, we have to regen
                        // the bitmap when it's recached.

   GFXBitmap::CacheInfo  bitmapCacheInfo;
   GFXBitmap::CacheInfo* pLightmapCacheInfo;

   HandleCacheEntry();
   ~HandleCacheEntry();
};

class HandleCache
{
  private:
   int                  m_entryTableSize;
   int                  m_numHandles;
   HandleCacheEntry*    m_currentEntry;
   HandleCacheEntry**   m_entryTable;

   HandleCacheEntry     m_freeListTail;
   HandleCacheEntry     m_freeListHead;

   HandleCacheEntry*    m_array;

   void touch(HandleCacheEntry *entry);
   UInt32 HashHandle(GFXTextureHandle);
   Bool HandleEqual(const GFXTextureHandle&, const GFXTextureHandle&);

  public:
   HandleCache(int numHandles, int handleTableSize);
   ~HandleCache();

   void  HashInsert(HandleCacheEntry *);

   HandleCacheEntry* getFreeEntry(Surface::TextureCache*);
   HandleCacheEntry* getCurrentEntry() { return m_currentEntry; }

   GFXLightMap* getLightMap();
   void setLightMap(int lightScale, GFXLightMap *map);
   void setTextureMap(const GFXBitmap *bmp);
   void setMipLevel(int mipLevel);
   Bool setTextureHandle(GFXTextureHandle tex);
   void flush();
};

//------------------------------------------------------------------------------
//-------------------------------------- Public inlines
//
inline void
Surface::TextureCache::setBlendMode(const GLenum in_srcFactor,
                                    const GLenum in_dstFactor)
{
   if (in_srcFactor != m_srcBlend ||
       in_dstFactor != m_dstBlend) {
      glBlendFunc(in_srcFactor, in_dstFactor);
   }

   m_srcBlend = in_srcFactor;
   m_dstBlend = in_dstFactor;
}

inline void
Surface::TextureCache::setTransparent(const bool in_transp)
{
   GeneralState desired = in_transp ? Enabled : Disabled;

   if (desired != m_transpEnabled) {
      if (in_transp == true) {
         glEnable(GL_ALPHA_TEST);
         glAlphaFunc(GL_GREATER, GLclampf(0.65));
      } else {
         glDisable(GL_ALPHA_TEST);
      }
   }
   m_transpEnabled = desired;
}

inline void
Surface::TextureCache::enableFog(const bool in_enable)
{
   GeneralState desired = in_enable ? Enabled : Disabled;
   if (m_fogEnabled != desired) {
      m_fogEnabled = desired;

      if (in_enable)
         glEnable(GL_FOG);
      else
         glDisable(GL_FOG);
   }
}


inline void
Surface::TextureCache::enableZBuffering(const bool   in_enableTest,
                                        const bool   in_enableWrite,
                                        const GLenum in_compare)
{
   GeneralState desiredTest  = in_enableTest  ? Enabled : Disabled;
   GeneralState desiredWrite = in_enableWrite ? Enabled : Disabled;

   if (desiredTest != m_zTestEnabled) {
      if (in_enableTest)
         glEnable(GL_DEPTH_TEST);
      else
         glDisable(GL_DEPTH_TEST);
      m_zTestEnabled = desiredTest;
   }

   if (desiredWrite != m_zWriteEnabled) {
      if (in_enableWrite)
         glDepthMask(GL_TRUE);
      else
         glDepthMask(GL_FALSE);
      m_zWriteEnabled = desiredWrite;
   }


   if (in_compare != m_zMode) {
      glDepthFunc(in_compare);
      m_zMode = in_compare;
   }
}

inline void
Surface::TextureCache::clearState()
{
   m_texUnitState[0]    = Undetermined;
   m_texUnitState[1]    = Undetermined;
   m_texUnitMode[0]     = -1;
   m_texUnitMode[1]     = -1;
   m_srcBlend           = -1;
   m_dstBlend           = -1;
   m_transpEnabled      = Undetermined;
   m_zTestEnabled       = Undetermined;
   m_zWriteEnabled      = Undetermined;
   m_zMode              = -1;
   m_fogEnabled         = Undetermined;

   m_currentTexUnit    = -2;
   m_currentTexName[0] = -1;
   m_currentTexName[1] = -1;
}

inline void
Surface::TextureCache::setTexUnit(int u)
{
   const GLuint SGIS_TEXTURE_0 = 0x835E;
   const GLuint SGIS_TEXTURE_1 = 0x835F;
   AssertFatal(u == 0 || (supportsSGIMultiTexture() && u == 1), "oob tex unit");

   if (u != m_currentTexUnit) {
      m_currentTexUnit = u;

      if (supportsSGIMultiTexture())
         m_multiTexExtSGI.glSelectTexture(u == 0 ? SGIS_TEXTURE_0 : SGIS_TEXTURE_1);
   }
}

inline Surface::TextureCache::MPCacher*
Surface::TextureCache::getMPCache(const Int32 in_palIndex)
{
   if (m_multiPalettes[0].paletteIndex  == in_palIndex) return &m_multiPalettes[0];
   if (m_multiPalettes[1].paletteIndex  == in_palIndex) return &m_multiPalettes[1];
   if (m_multiPalettes[2].paletteIndex  == in_palIndex) return &m_multiPalettes[2];
   if (m_multiPalettes[3].paletteIndex  == in_palIndex) return &m_multiPalettes[3];
   if (m_multiPalettes[4].paletteIndex  == in_palIndex) return &m_multiPalettes[4];
   if (m_multiPalettes[5].paletteIndex  == in_palIndex) return &m_multiPalettes[5];
   if (m_multiPalettes[6].paletteIndex  == in_palIndex) return &m_multiPalettes[6];
   if (m_multiPalettes[7].paletteIndex  == in_palIndex) return &m_multiPalettes[7];
   if (m_multiPalettes[8].paletteIndex  == in_palIndex) return &m_multiPalettes[8];
   if (m_multiPalettes[9].paletteIndex  == in_palIndex) return &m_multiPalettes[9];
   if (m_multiPalettes[10].paletteIndex == in_palIndex) return &m_multiPalettes[10];
   if (m_multiPalettes[11].paletteIndex == in_palIndex) return &m_multiPalettes[11];
   if (m_multiPalettes[12].paletteIndex == in_palIndex) return &m_multiPalettes[12];
   if (m_multiPalettes[13].paletteIndex == in_palIndex) return &m_multiPalettes[13];
   if (m_multiPalettes[14].paletteIndex == in_palIndex) return &m_multiPalettes[14];
   if (m_multiPalettes[15].paletteIndex == in_palIndex) return &m_multiPalettes[15];

   return &m_multiPalettes[0];
}

} // namespace OpenGL

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_GOGLTX_H_
