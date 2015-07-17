//
// m_qsort.h
//

#ifndef _M_QSORT_H_
#define _M_QSORT_H_

#include <stdlib.h>

// extern AngleF DLLAPI m_reduce( const AngleF a );

extern void __cdecl m_qsort(void *base, size_t nelem, size_t width, 
   int (_USERENTRY *fcmp)(const void *, const void *));

#endif