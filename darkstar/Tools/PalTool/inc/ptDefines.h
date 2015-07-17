//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _DEFINES_H_
#define IDD_PALTOOL	   1000
#define IDD_SHADEHAZE	2000
#define IDD_ABOUT	      3000

#define IDC_HAZE_LEVELS	      101
#define IDC_HAZE_COLOR	      103
#define IDC_VIEW	            104
#define IDC_EDIT_RGB_MATCH	   100
#define IDC_EDIT_HAZE_TO      102
#define IDC_EDIT_SHADE	      105
#define IDC_EDIT_HAZE	      106
#define IDC_SURFACE	         113
#define IDC_RGB	            114
#define IDC_GENERATE	         115
#define IDC_DO_SHADE	         116
#define IDC_DO_HAZE	         117
#define IDC_HAZE_MAX	         118
#define IDC_SHADE_MAX	      119
#define IDC_GAMMA	            120
#define IDC_HAZE_LEVLES	      121
#define IDC_SHADE_LEVELS      122
#define IDC_USE_LUV	         123
#define IDC_APPLY	            124
#define IDC_PROGRESS	         125
#define IDC_TABCONTROL	      126
                                
                                
#define IDM_MENU	      1000       
#define IDM_OPEN        1001       
#define IDM_SAVE        1002
#define IDM_SAVE_AS     1003
#define IDM_OPEN_MAP    1011       
#define IDM_SAVE_MAP    1012
#define IDM_SAVE_AS_MAP 1013
#define IDM_EXIT        1004


#define SZ  (10)       // size of palette pixel
#define SP   (4)       // space between palette pixels
#define XOFS (8)       // offset of palette from left of surface
#define YOFS (SZ+SP)   // offset of palette from top of surface

#define SHW ((SZ+SP)*19)   // Haze/Shade width in pixels
#define SHH ((SZ+SP)*16)   // Haze/Shade height in pixels
#define XOFSS ((SZ+SP)*18)
#define YOFSS (YOFS) 


#endif //_DEFINES_H_