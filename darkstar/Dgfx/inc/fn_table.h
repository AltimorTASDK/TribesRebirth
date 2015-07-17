//±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±
//±   
//± Description 
//±   
//± $Workfile$
//± $Revision$
//± $Author  $
//± $Modtime $
//±
//±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±±

#ifndef _FN_TABLE_H_
#define _FN_TABLE_H_

#include "fn_all.h"

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

struct FunctionTable;

extern FunctionTable rclip_table;
extern FunctionTable glide_table;
extern FunctionTable powersgl_table;
extern FunctionTable opengl_table;

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_FN_TABLE_H_
