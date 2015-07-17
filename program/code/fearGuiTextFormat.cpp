//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------
#include "fear.strings.h"
#include "fearGuiTextFormat.h"

namespace FearGui
{

static const int gCtrlVersion = 0;

bool FGTextFormat::onAdd()
{
   if (!Parent::onAdd()) return false;
   
   setFont(0, IDFNT_10_STANDARD); 
   setFont(1, IDFNT_10_HILITE); 
   setFont(2, IDFNT_10_SELECTED); 
   
   return true;
}

void FGTextFormat::setScriptValue(const char *text)
{
   setFont(0, IDFNT_10_STANDARD); 
   setFont(1, IDFNT_10_HILITE); 
   setFont(2, IDFNT_10_SELECTED); 
   
   formatControlString(text, extent.x, false, true);
   resize(position, Point2I(extent.x, getHeight() + 4));
}

Persistent::Base::Error FGTextFormat::write( StreamIO &sio, int a, int b)
{
   sio.write(gCtrlVersion);
   
   return Parent::write(sio, a, b);
}

Persistent::Base::Error FGTextFormat::read( StreamIO &sio, int a, int b)
{
   int version;
   sio.read(&version);
   
   return Parent::read(sio, a, b);
}

IMPLEMENT_PERSISTENT_TAG( FGTextFormat, FOURCC('F','G','t','f'));

};