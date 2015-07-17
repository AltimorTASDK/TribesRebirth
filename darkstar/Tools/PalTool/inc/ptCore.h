#ifndef _PTCORE_H_
#define _PTCORE_H_



#include <core.h>
#include <gw.h>
#include <tBitVector.h>
#include <commdlg.h>
#include <palMap.h>

#define  STATE_EDIT        0x01  // toggle edit/view mode
#define  STATE_HAZE        0x02     
#define  STATE_SHADE       0x04  
#define  STATE_HAZE_TO     0x08  
#define  STATE_RGB_MATCH   0x10  
#define  STATE_MASK        (STATE_HAZE| STATE_SHADE| STATE_HAZE_TO| STATE_RGB_MATCH)
#define  STATE_RENDERING   0x80   


#endif   // _PTCORE_H_
