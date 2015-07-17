//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _LINK_H_
#define _LINK_H_

//Includes

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

#define DeclareLink(ALink) void ALink##LinkDeclare(){}
#define ForceLink(ALink) extern void ALink##LinkDeclare();void ALink##LinkForce(){ALink##LinkDeclare();}

#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_LINK_H_


