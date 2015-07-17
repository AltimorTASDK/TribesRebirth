//================================================================
//   
//	$Workfile:   m_temp.cpp  $
//	$Revision:   1.2  $
// $Version: 1.6.2 $
//	$Date:   10 Nov 1995 11:36:42  $
// $Log:   R:\darkstar\develop\core\ml\vcs\m_temp.cpv  $
//	
//================================================================

#if defined(__BORLANDC__) && (__BORLANDC__ >= 0x500)

#pragma option -Jg

#include "ts_shadow.h"


template class TS::TPoly<TS::TPolyVertex>;
template class TS::TPoly<TS::TexturedPolyVertex>;
template class SortableVectorPtr<TS::Poly*>;
template class Vector<TS::Face>;

#endif
