//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _SIMGUITEXTLIST_H_
#define _SIMGUITEXTLIST_H_

#include <g_font.h>
#include <simGuiArrayCtrl.h>

namespace SimGui
{
         
class DLLAPI TextList : public ArrayCtrl
{
private:
   typedef ArrayCtrl Parent;

protected:
	Resource<GFXFont> hFont;
	Resource<GFXFont> hFontHL;

   Int32 fontNameTag;
   Int32 fontNameTagHL;
   Int32 textVPosDelta;

public:
   struct Entry {
      Int32 stdTag;  // sim tag dictionary tag.
      char *text;
      void *data; // subclass specific entry data
   };
   
   Vector<Entry> textList;
   bool enumerate;
   bool noDuplicates;

protected:
   enum Constants
   {
      ENTRY_H_MARGIN = 10,
      ENTRY_V_MARGIN = 4,
   };

   enum ScrollConst
   {
      UP = 0,
      DOWN = 1,
   };

   Entry createEntry(Int32 stdTag, const char *text);

public:
   TextList();

   const char *getScriptValue();
   void setScriptValue(const char *value);

   virtual void clearEntry(TextList::Entry *entry);

	void setFont(Int32 tag);
	void setFontHL(Int32 tag);

   Int32 getNumEntries();


   // pos is 0 based, ie 0 to (size - 1)
   void addEntryFront(Int32 stdTag);
   void addEntryFront(const char *text);  // text is copied

   void addEntryBack(Int32 stdTag);
   void addEntryBack(const char *text);   // text is copied

   void removeEntryFront();
   void removeEntryBack();
   
   const char* getSelectedText();
   Int32 getSelectedTag();
   void deleteSelected();

   void onPreRender();
   void onRenderCell(GFXSurface *sfc, Point2I offset, Point2I cell, bool selected, bool mouseOver);

   bool onAdd();
   void onWake();

   void clear();
   void sort();

   Int32 findEntry(const char *text);

   void inspectRead(Inspect *);
   void inspectWrite(Inspect *);
   void onRemove();

   bool becomeFirstResponder();

   DECLARE_PERSISTENT(TextList);
   Persistent::Base::Error write( StreamIO &sio, int, int );
   Persistent::Base::Error read( StreamIO &sio, int, int );
};

};

#endif //_SIMGUITEXTLIST_H_
