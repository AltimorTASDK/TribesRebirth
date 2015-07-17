//---------------------------------------------------------------------------
//	$Workfile:   ts_mesh.h  $
//	$Revision:   2.8  $
// $Version: $
//	$Date:   15 Sep 1995 10:27:12  $
//	$Log:   R:\darkstar\develop\ts\vcs\ts_mesh.h_v  $
//	
//---------------------------------------------------------------------------

#include "ts_transform.h"

namespace TS
{
   //---------------------------------------------------------------------------
	TMat3F & Transform::interpolate( Transform const &src1, 
														  Transform const &src2,
							   			  			  RealF interpParam, TMat3F *mat )
	{
		Point3F trans;
		QuatF quat;
   	trans.interpolate( src1.fTranslate, src2.fTranslate, interpParam );
      Transform::interpolate( src1.fRotate, src2.fRotate, interpParam, &quat );
      mat->set( quat, trans );
#if TRANS_USE_SCALE			
		Point3F scale;
		scale.interpolate( src1.fScale, src2.fScale, interpParam );
		mat->preScale( scale );			
#endif
		return *mat;
	}									
   //---------------------------------------------------------------------------
};  // namespace TS
