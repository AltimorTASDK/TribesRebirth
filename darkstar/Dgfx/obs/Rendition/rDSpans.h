//------------------------------------------------------------------------------
// Description: Redline Fifo command words for DSpans commands... 
//    
// $Workfile: $
// $Revision: $
// $Author:   $
// $Modtime:  $
//
//------------------------------------------------------------------------------

#ifndef _RDSPANS_H_
#define _RDSPANS_H_

//Includes

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

#define V_FIFO_DSPAN_C    62
#define V_FIFO_DSPAN_S    63
#define V_FIFO_DSPAN_H    64
#define V_FIFO_DSPAN_UV   65
#define V_FIFO_DSPAN_UVS  66
#define V_FIFO_DSPAN_UVH  67
#define V_FIFO_DSPAN_UVQ  68
#define V_FIFO_DSPAN_UVQS 69
#define V_FIFO_DSPAN_UVQH 70

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_RDSPANS_H_
