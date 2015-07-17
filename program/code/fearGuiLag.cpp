
#include "simResource.h"
#include "SimGuiCtrl.h"
#include "console.h"
#include "g_bitmap.h"
#include "g_surfac.h"
#include "fearCSDelegate.h"
#include "fear.strings.h"
#include "fearGlobals.h"
#include "netPacketStream.h";
namespace FearGui
{

class FGLagIndicator : public SimGui::Control
{
private:
   typedef SimGui::Control Parent;
   
   Resource<GFXBitmap> bmp;
   static DWORD mLagTimeout;
   
public:
   bool onAdd(void);
   void onPreRender(void);
   void onRender(GFXSurface *sfc, Point2I offset, const Box2I &updateRect);
   
   bool wantsTabListMembership() { return FALSE; }

   DECLARE_PERSISTENT(FGLagIndicator);
};

DWORD FGLagIndicator::mLagTimeout = 700;

bool FGLagIndicator::onAdd(void)
{
   if (! Parent::onAdd()) return FALSE;
   
   //load the lag bitmap
   bmp = SimResource::get(manager)->load("H_Lag.BMP");
   AssertFatal(bmp.operator bool(), "Unable to load H_Lag.bmp");
   bmp->attribute |= BMA_TRANSPARENT; 
   extent.set(bmp->getWidth(), bmp->getHeight());
   
   //add the pref var
	Console->addVariable(0, "pref::LagTimeout", CMDConsole::Int, &mLagTimeout);
   
   return TRUE;   
}

void FGLagIndicator::onPreRender(void)
{
   if (! parent) return;
   position.set(parent->extent.x - 1 - extent.x, 0);
   
   if (cg.csDelegate && cg.packetStream && cg.packetStream->getStreamMode() != Net::PacketStream::PlaybackMode)
   {
      DWORD curTime = GetTickCount();
      DWORD csDelegateTime = cg.csDelegate->getLastReceivedTick();
      if (DWORD(curTime - csDelegateTime) >= mLagTimeout)
      {
         setVisible(TRUE);
      }
      else setVisible(FALSE);
   }
}

void FGLagIndicator::onRender(GFXSurface* sfc, Point2I offset, const Box2I &)
{
   sfc->drawBitmap2d(bmp, &offset);
}

IMPLEMENT_PERSISTENT_TAG( FGLagIndicator,   FOURCC('F','G','l','g') );

};