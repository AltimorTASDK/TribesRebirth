#ifndef _H_SIMGUICTRL
#define _H_SIMGUICTRL

#include "simGuiBase.h"
#include "simGuiCanvas.h"
#include <simEv.h>
#include "inspect.h"

namespace SimGui
{

class Canvas;

class DLLAPI Control : public SimGroup, public Responder
{
private:
   typedef SimGroup Parent;

public:
	enum horizSizingOptions
	{
		horizResizeRight = 0, // fixed on the left and width
		horizResizeWidth, // fixed on the left and right
		horizResizeLeft, // fixed on the right and width
      horizResizeCenter,
      horizResizeRelative, //resize relative
	};
	enum vertSizingOptions
	{
		vertResizeBottom = 0, // fixed on the top and in height
		vertResizeHeight, // fixed on the top and bottom
		vertResizeTop, // fixed in height and on the bottom
      vertResizeCenter,
      vertResizeRelative, // resize relative
	};

   enum Flags
   {
      Visible = 1, DeleteOnLoseContent = 2, ArrowsDontMakeFirstResponder = 4,
   };

	BYTE mbOpaque;
	BYTE mbBoarder;
	BYTE fillColor;
	BYTE selectFillColor;
	BYTE ghostFillColor;
	BYTE boarderColor;
	BYTE selectBoarderColor;
	BYTE ghostBoarderColor;

protected:
   Control *parent;
   Canvas *root;
   DWORD tag;
	Int32 horizSizing;
	Int32 vertSizing;
   Inspect_Str consoleVariable;
   Inspect_Str consoleCommand;
   Inspect_Str altConsoleCommand;
   Int32 mouseCursorTag;
	Int32 helpTag;
   const char *lpszHelpText;
   
   // console variable mapping stuff
   void setVariable(const char *value);
   void setIntVariable(int value);
   void setFloatVariable(float value);

   const char* getVariable(void);
   int getIntVariable(void);
   float getFloatVariable(void);

   virtual void variableChanged(const char *newValue);
   void onNameChange(const char *name);
public:
   BitSet32 flags;
   Int32 vLineScroll;
   Int32 hLineScroll;

	Point2I position; // control bbox in it's own reference frame is (0,0) to (extent.x, extent.y)
                     // position is the upper left corner in parent's coord system
   Point2I extent;   // size of control

   virtual Point2I getMinExtent();
   const Point2I& getPosition() { return position; }
   const Point2I& getExtent() { return extent; }
   const Control* getParent() { return parent; }
   const Control* getTopMostParent();

   DECLARE_PERSISTENT( Control );

   Control();
   ~Control();

   bool processArguments(int argc, const char **argv);

   Persistent::Base::Error write( StreamIO &sio, int, int );
   Persistent::Base::Error read( StreamIO &sio, int, int );
   static Control* load(SimManager *manager, const char *name);

   void inspectRead(Inspect *insp);
   void inspectWrite(Inspect *insp);

	SimObject* addObject(SimObject*);
	void removeObject(SimObject*);

   void setConsoleVariable(const char *variable);
	void setConsoleCommand(const char *newCmd);
	const char * getConsoleCommand(void);

   virtual void onRender(GFXSurface *sfc, Point2I offset, const Box2I &updateRect);
   void renderChildControls(GFXSurface *sfc, Point2I offset, const Box2I &updateRect);

   bool deleteOnLoseContent() { return flags.test(DeleteOnLoseContent); }

	void getSizingOptions(Int32 *horz, Int32 *vert)
			{ *horz = horizSizing; *vert = vertSizing; }

	void setSizingOptions(Int32 horz, Int32 vert)
			{ horizSizing = horz; vertSizing = vert; }

   virtual void editResized(); // called if the editor resized/moved you
	virtual void resize(const Point2I &newPosition, const Point2I &newExtent);
	virtual void childResized(Control *child);
   virtual void parentResized(const Point2I &oldParentExtent, const Point2I &newParentExtent);

   Point2I globalToLocalCoord(const Point2I &src);
   Point2I localToGlobalCoord(const Point2I &src);

   void setCanvas(Canvas *newCanvas);
   Canvas* getCanvas() { return root; }

   virtual bool pointInControl(Point2I &parentCoordPoint);
   virtual Control* findHitControl(const Point2I &pt);
   Control* findControlWithTag(DWORD tag);
   Control* findRootControlWithTag(DWORD tag);

   Control* findNamedControl(StringTableEntry name);
   Control* findRootNamedControl(StringTableEntry name);

   virtual const char *getScriptValue();
   virtual void setScriptValue(const char *value);

   DWORD getTag();
   void setHelpTag(DWORD);
   void setHelpText(const char *);
   void setTag(DWORD newTag);
   void messageSiblings(DWORD message);

   virtual void setVisible(bool visState);
   void setBranchVisible(bool visState, bool updateRoot = true);
   inline bool isVisible(void) { return flags.test(Visible); }

   void setUpdateRegion(Point2I pos, Point2I ext);
   void setUpdate();

   bool onAdd();
   void onRemove();
   void preRender();
   void awaken(); // called when this control and it's children have
                  // been wired up.
   void close(); // called when this control and it's children are about
                  // to be removed from the content control stack
   
   void setMouseCursorTag(Int32 mouseTag) {mouseCursorTag = mouseTag;}
   virtual Int32 getMouseCursorTag() {return mouseCursorTag;}
   virtual Int32 getHelpTag(float) {return helpTag;}
   virtual const char *getHelpText(float) { return lpszHelpText; }

   virtual void onPreRender(); // do special pre render processing
   virtual void onWake(); // do special processing on wakeup.
   virtual void onClose(); // do special processing on close.
   void addControl(Control *ctrl); // add a control substructure

   virtual bool wantsTabListMembership();
   void mouseLock() { root->mouseLock(this); }
   void mouseUnlock() { if(root->getMouseLockedControl() == this) root->mouseUnlock(); }

   // allows controls to be included in scroll views
   virtual Int32 scroll_Line_H(RectI &clientRect);
   virtual Int32 scroll_Line_V(RectI &clientRect);
   virtual Int32 scroll_Page_H(RectI &clientRect);
   virtual Int32 scroll_Page_V(RectI &clientRect);

   // Called for bottommost content control, allows TSCtrl to lock in double
   //  mode, if necessary.  Most other controls should _not_ override these.
   virtual void lockDevice(GFXDevice*);
   virtual void unlockDevice(GFXDevice*);
};

};

#endif