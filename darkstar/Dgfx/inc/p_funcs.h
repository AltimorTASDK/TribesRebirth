//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _P_FUNCS_H_
#define _P_FUNCS_H_

//Includes
#include "d_defs.h"

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

extern GFXFillMode      g_fillMode;
extern GFXShadeSource   g_shadeSource;
extern GFXHazeSource    g_hazeSource;
extern GFXAlphaSource   g_alphaSource;
extern Bool             g_transparent;
extern Bool             g_texturePerspective;

extern float g_shadeLevel;
extern float g_hazeLevel;

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_P_FUNCS_H_
