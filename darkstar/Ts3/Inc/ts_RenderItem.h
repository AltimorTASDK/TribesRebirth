//---------------------------------------------------------------------------

//	$Workfile:   ts_mesh.h  $
//	$Revision:   2.8  $
// $Version$
//	$Date:   15 Sep 1995 10:27:12  $
//	$Log:   R:\darkstar\develop\ts\vcs\ts_mesh.h_v  $
//	

//---------------------------------------------------------------------------

#ifndef _TS_RENDERITEM_H_
#define _TS_RENDERITEM_H_

#include <types.h>

namespace TS
{
   //---------------------------------------------------------------------------
   //------------------------------------------------------------------

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

   //------------------------------------------------------------------

   class RenderContext;

   class DLLAPI RenderItem
      {
   public:
      virtual ~RenderItem() {};
      virtual void render( RenderContext &rc ) = 0;
      };

   //------------------------------------------------------------------
#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif


};  // namespace TS

//
typedef TS::RenderItem TSRenderItem;


#endif
