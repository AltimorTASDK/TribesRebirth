#include <d_caps.h>
#include "SimGuiBitmapCtrl.h"
#include "g_bitmap.h"
#include "g_surfac.h"
#include "commonEditor.strings.h"

namespace FearGui
{

class FGBitmapCtrl : public SimGui::BitmapCtrl
{
   float m_alphaPercentage;

private:
   typedef SimGui::BitmapCtrl Parent;
   
public:

	FGBitmapCtrl(void) : m_alphaPercentage(1.0) { }
   void onRender(GFXSurface *sfc, Point2I offset, const Box2I &updateRect);
   
   void inspectWrite(Inspect *insp);
   void inspectRead(Inspect *insp);

   bool wantsTabListMembership() { return FALSE; }

   DECLARE_PERSISTENT(FGBitmapCtrl);
   Persistent::Base::Error write( StreamIO &sio, int version, int user );
   Persistent::Base::Error read( StreamIO &sio, int version, int user );
};

void FGBitmapCtrl::onRender(GFXSurface* sfc, Point2I offset, const Box2I& updateRect)
{
   if (m_alphaPercentage != 1.0f &&
       sfc->getCaps() & GFX_DEVCAP_SUPPORTS_CONST_ALPHA) {
      sfc->setAlphaSource(GFX_ALPHA_CONSTANT);
      sfc->setConstantAlpha(m_alphaPercentage);
   }

   Parent::onRender(sfc, offset, updateRect);

   if (m_alphaPercentage != 1.0f &&
       sfc->getCaps() & GFX_DEVCAP_SUPPORTS_CONST_ALPHA) {
      sfc->setAlphaSource(GFX_ALPHA_NONE);
   }
}

void FGBitmapCtrl::inspectWrite(Inspect *insp)
{
   Parent::inspectWrite(insp);
   
   insp->write(IDITG_SV_TRANS_LEVEL, m_alphaPercentage);
}   

void FGBitmapCtrl::inspectRead(Inspect *insp)
{
   Parent::inspectRead(insp);
   
   insp->read(IDITG_SV_TRANS_LEVEL, m_alphaPercentage);
}   

Persistent::Base::Error
FGBitmapCtrl::read( StreamIO &sio, int version, int user )
{
   Parent::read(sio, version, user);

   sio.read(&m_alphaPercentage);
   return Ok;
}

Persistent::Base::Error
FGBitmapCtrl::write( StreamIO &sio, int version, int user )
{
   Parent::write(sio, version, user);

   sio.write(m_alphaPercentage);
   return Ok;
}

IMPLEMENT_PERSISTENT_TAG( FGBitmapCtrl,   FOURCC('F','G','b','c') );

};