//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _SIMGUICOMBOBOX_H_
#define _SIMGUICOMBOBOX_H_

#include "simGuiSimpleText.h"

namespace SimGui
{

class ScrollCtrl;
class ComboBox;
class ArrayCtrl;

class DLLAPI ComboPopUp : public Control
{
private:
   typedef Control Parent;

protected:
   ComboBox *parentCb;
   ScrollCtrl *sc;
   ArrayCtrl *ac;
   Point2I dim;

public: 
   ComboPopUp(ComboBox *_parentCb);
   void setPos(Int32 x, Int32 y);
   virtual void setDim(Int32 w, Int32 hMin, Int32 hMax);
   void getDim(Point2I &p);
   void onRender(GFXSurface *sfc, Point2I offset, const Box2I &updateRect);
   void onMouseDown(const Event &event);
   void onMouseMove(const Event &event);
   void onMouseDragged(const Event &event);
   void onMouseUp(const Event &event);
   DWORD onMessage(SimObject *sender, DWORD msg);
   ArrayCtrl * getArrayCtrl() {return ac;}
   ScrollCtrl * getScrollCtrl() {return sc;}

   bool onAdd();
   void onRemove();
   void onDeleteNotify(SimObject *object);
};

class DLLAPI ComboBox : public SimpleText
{
private:
   typedef SimpleText Parent;

protected:
   ComboPopUp *popUpCtrl;
   Bool popUpActive;

public:   
   ComboBox() {active = true; popUpActive = false;}

   void onRender(GFXSurface *sfc, Point2I offset, const Box2I &updateRect);
   void onAction();

   virtual void openPopUpCtrl();
   virtual void closePopUpCtrl();

   ArrayCtrl * getArrayCtrl();
   virtual void updateFromArrayCtrl();

   DECLARE_PERSISTENT(ComboBox);

   bool onAdd();
   void onRemove();
   void onDeleteNotify(SimObject *object);
};

};
#endif //_SIMGUICOMBOBOX_H_
