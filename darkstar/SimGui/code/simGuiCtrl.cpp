#include "simGuiCtrl.h"
#include "g_surfac.h"
#include "inspect.h"
#include "editor.strings.h"
#include "r_clip.h"
#include "console.h"
#include "simResource.h"

namespace SimGui
{

static const BYTE gControlVersion = 4;

Control::Control()
{
   position.set(0,0);
   extent.set(50,50);
   root = NULL;
   parent = NULL;
   horizSizing = horizResizeRight;
   vertSizing = vertResizeBottom;
   tag = 0;
   ownObjects = true;
   flags = Visible | DeleteOnLoseContent;
   vLineScroll = 5;
   hLineScroll = 5;
   mouseCursorTag = 0;
   helpTag = 0;
   lpszHelpText = NULL;

   consoleVariable[0] = 0;
   consoleCommand[0] = 0;
   altConsoleCommand[0] = 0;
   
   mbOpaque = FALSE;
   fillColor = 0;
   mbBoarder = FALSE;
   boarderColor = 0;
   stable = true;
}

Control::~Control()
{
   if (lpszHelpText)
   {
      delete [] (char*)lpszHelpText;
      lpszHelpText = NULL;
   }
}

void Control::onRemove()
{
   if(root)
      root->invalidateControl(this);
   Parent::onRemove();
}

void Control::setVisible(bool visState)
{
   flags.set(Visible, visState);
   if(root)
      root->rebuildTabList();
   setUpdate();
}

void Control::setBranchVisible(bool visState, bool updateRoot)
{
   flags.set(Visible, visState);
   for(iterator i = begin(); i != end(); i++)
   {
      Control *ctrl = (Control *)(*i);
      ctrl->setBranchVisible(visState, false);
   }
   
   if(updateRoot)
   {
      if (root) root->rebuildTabList();
      setUpdate();
   }
}

Persistent::Base::Error Control::write( StreamIO &sio, int version, int user )
{
   //write the version first
   BYTE versionByte = gControlVersion;
   sio.write(versionByte);
   
   //write the two free bytes left
   DWORD unused = 0;
   sio.write(unused);
   
   sio.write(mbOpaque);
   sio.write(fillColor);
   sio.write(selectFillColor);
   sio.write(ghostFillColor);
   sio.write(mbBoarder);
   sio.write(boarderColor);
   sio.write(selectBoarderColor);
   sio.write(ghostBoarderColor);

   BYTE len = strlen(consoleCommand);
   sio.write(len);
   if(len)
      sio.write(len, consoleCommand);
   
   len = strlen(altConsoleCommand);
   sio.write(len);
   if(len)
      sio.write(len, altConsoleCommand);
      
   position.write(sio);
   extent.write(sio);
   sio.write(flags.mask());
   sio.write(tag);
   sio.write(horizSizing);
   sio.write(vertSizing);
   
   sio.write(helpTag);

   sio.write(Inspect::MAX_STRING_LEN + 1, consoleVariable);
   return Parent::write(sio, version, user);
}

Persistent::Base::Error Control::read( StreamIO &sio, int version, int user)
{
   //first read in the version number
   BYTE versionByte;
   sio.read(&versionByte);
   
   //if the version byte was zero, read in an empty 6 bytes
   if (versionByte == 0)
   {
   
      DWORD temp;
      sio.read(&temp);
      sio.read(&temp);
      
      BYTE temp2;
      sio.read(&temp2);
      sio.read(&temp2);
      
      mbOpaque = mbBoarder = 0;
   }
   
   //else if the version >= 1, read in the flags for opaque and boarder, and their colors
   else if (versionByte == 1)
   {
      DWORD temp;
      sio.read(&temp);
      
      sio.read(&mbOpaque);
      sio.read(&fillColor);
      sio.read(&mbBoarder);
      sio.read(&boarderColor);
   }
   
   else if (versionByte >= 2)
   {
      DWORD temp;
      sio.read(&temp);
      
      sio.read(&mbOpaque);
      sio.read(&fillColor);
      sio.read(&selectFillColor);
      sio.read(&ghostFillColor);
      sio.read(&mbBoarder);
      sio.read(&boarderColor);
      sio.read(&selectBoarderColor);
      sio.read(&ghostBoarderColor);
   }

   BYTE len;
   sio.read(&len);
   if(len)
   {
      sio.read(len, consoleCommand);
      consoleCommand[len] = 0;
   }
   
   if (versionByte >= 3)
   {
      sio.read(&len);
      if(len)
      {
         sio.read(len, altConsoleCommand);
         altConsoleCommand[len] = 0;
      }
   }
   
   position.read(sio);
   extent.read(sio);
   UInt32 mask;
   sio.read(&mask);
   flags = mask;
   sio.read(&tag);
   sio.read(&horizSizing);
   sio.read(&vertSizing);
   
   if (versionByte >= 4)
   {
      sio.read(&helpTag);
   }

   sio.read(Inspect::MAX_STRING_LEN + 1, consoleVariable);
   return Parent::read(sio, version, user);
}

Control* Control::load(SimManager *manager, const char *name)
{
   Persistent::Base::Error err;
   Control *control = NULL;
   if (manager)
   {
      control = (Control*)Persistent::Base::fileLoad(name, &err);
      if (!control)
      {
         ResourceManager *rm = SimResource::get(manager);
         if (rm)
         {
            void *data = rm->lock( name );
            if (data)
            {
               MemRWStream sio(rm->getSize(name), data);
               control = (Control*)Persistent::Base::load(sio, &err);
            }
         }
      }
      if (err != Persistent::Base::Ok)
      {
         delete control;
         control = NULL;   
      }
   }
   return (control);
}   

void Control::editResized()
{

}

void Control::inspectWrite(Inspect *insp)
{
   Parent::inspectWrite(insp);
   insp->write( IDITG_POS_TOP_LEFT,      position );
   insp->write( IDITG_EXTENT_DIM,        extent );
   insp->write( IDITG_CONTROL_ID,        true, "*",(Int32) tag);

   insp->writeSimTagPairList(IDITG_HORIZONTAL_SIZING, horizSizing, 5, 
      IDITG_HORIZ_RESIZE_RIGHT, horizResizeRight,
      IDITG_HORIZ_RESIZE_WIDTH, horizResizeWidth,
      IDITG_HORIZ_RESIZE_LEFT, horizResizeLeft,
      IDITG_HORIZ_RESIZE_CENTER, horizResizeCenter,
      IDITG_HORIZ_RESIZE_RELATIVE, horizResizeRelative);
   insp->writeSimTagPairList(IDITG_VERTICAL_SIZING, vertSizing, 5, 
      IDITG_VERT_RESIZE_BOTTOM, vertResizeBottom,
      IDITG_VERT_RESIZE_HEIGHT, vertResizeHeight,
      IDITG_VERT_RESIZE_TOP, vertResizeTop,
      IDITG_VERT_RESIZE_CENTER, vertResizeCenter,
      IDITG_VERT_RESIZE_RELATIVE, vertResizeRelative);

   insp->write( IDITG_VISIBLE,           bool(flags.test(Visible)) );
   insp->write( IDITG_DELETE_ON_LOSE,    bool(flags.test(DeleteOnLoseContent)));
   insp->write( IDITG_CONSOLE_VAR,       consoleVariable);
   insp->write( IDITG_CONSOLE_CMD,       consoleCommand);
   insp->write( IDITG_ALT_CONSOLE_CMD,   altConsoleCommand);
                   
   insp->write( IDITG_OPAQUE,            bool(mbOpaque) );
   Int32 temp = fillColor;
   insp->write( IDITG_FILL_COLOR,        temp );
   temp = selectFillColor;
   insp->write( IDITG_SELECT_FILL_COLOR, temp );
   temp = ghostFillColor;
   insp->write( IDITG_GHOST_FILL_COLOR,  temp );
   
   insp->write( IDITG_BOARDER,           bool(mbBoarder) );
   temp = boarderColor;
   insp->write( IDITG_BOARDER_COLOR,        temp );
   temp = selectBoarderColor;
   insp->write( IDITG_SELECT_BOARDER_COLOR, temp );
   temp = ghostBoarderColor;
   insp->write( IDITG_GHOST_BOARDER_COLOR,  temp );
   
   insp->write( IDITG_HELP_TAG,        true, "IDHELP*",(Int32) helpTag);
}   

void Control::inspectRead(Inspect *insp)
{
   Parent::inspectRead(insp);
   Box2I b;
   insp->read( IDITG_POS_TOP_LEFT,      position);  
   insp->read( IDITG_EXTENT_DIM,        extent);  
   
   Int32 temp;
   insp->read( IDITG_CONTROL_ID,        NULL, NULL, temp);
   tag = DWORD(temp);

   insp->readSimTagPairList(IDITG_HORIZONTAL_SIZING, horizSizing);
   insp->readSimTagPairList(IDITG_VERTICAL_SIZING, vertSizing);

   bool visible, deleteOnLose, Opaque, Boarder;
   insp->read( IDITG_VISIBLE,           visible);
   insp->read( IDITG_DELETE_ON_LOSE,    deleteOnLose);
   insp->read( IDITG_CONSOLE_VAR,       consoleVariable);
   insp->read( IDITG_CONSOLE_CMD,       consoleCommand);
   insp->read( IDITG_ALT_CONSOLE_CMD,   altConsoleCommand);
               
   insp->read( IDITG_OPAQUE,            bool(Opaque) );
   mbOpaque = Opaque;
   insp->read( IDITG_FILL_COLOR,        temp );
   fillColor = temp;
   insp->read( IDITG_SELECT_FILL_COLOR, temp );
   selectFillColor = temp;
   insp->read( IDITG_GHOST_FILL_COLOR,  temp );
   ghostFillColor = temp;
   
   insp->read( IDITG_BOARDER,            bool(Boarder) );
   mbBoarder = Boarder;
   insp->read( IDITG_BOARDER_COLOR,        temp );
   boarderColor = temp;
   insp->read( IDITG_SELECT_BOARDER_COLOR, temp );
   selectBoarderColor = temp;
   insp->read( IDITG_GHOST_BOARDER_COLOR,  temp );
   ghostBoarderColor = temp;
   
   insp->read( IDITG_HELP_TAG,        NULL, NULL, temp);
   setHelpTag((DWORD)temp);
   
   flags.set(Visible, visible);
   flags.set(DeleteOnLoseContent, deleteOnLose);

   if(consoleVariable[0])
   {
      const char *var = Console->getVariable(consoleVariable);
      variableChanged(var);
   }
   setUpdate();
}

void Control::setHelpTag(DWORD dwNewTag)
{
   helpTag = dwNewTag;

   if (lpszHelpText)
   {
      delete [] (char*)lpszHelpText;
      lpszHelpText = NULL;
   }
}

void Control::setHelpText(const char *lpszText)
{
   if (lpszHelpText)
   {
      delete [] (char*)lpszHelpText;
      lpszHelpText = NULL;
   }

   if (lpszText)
   {
      lpszHelpText = strnew(lpszText);
   }

   helpTag = 0;
}

void Control::onNameChange(const char *)
{
   if(manager)
      addToSet(NamedGuiSetId);
}

bool Control::onAdd()
{
   if(!Parent::onAdd())
      return false;
   if(getName())
      addToSet(NamedGuiSetId);
   if(tag)
      addToSet(TaggedGuiSetId);
   if(consoleVariable[0])
   {
      const char *var = Console->getVariable(consoleVariable);
      variableChanged(var);
   }
   return true;
}

bool Control::processArguments(int argc, const char **argv)
{
   if(!argc)
      return true;
   if(argc >= 4)
   {
      // x, y, w, h
      position.x = atoi(argv[0]);
      position.y = atoi(argv[1]);
      extent.x = atoi(argv[2]);
      extent.y = atoi(argv[3]);
   }
   if(argc >= 5)
      setConsoleVariable(argv[4]);
   if(argc >= 6)
      setConsoleCommand(argv[5]);
   return true;
}

void Control::variableChanged(const char *newValue)
{
   // do nothing;
   newValue;
}

void Control::setConsoleVariable(const char *variable)
{
   if (variable)
   {
      strncpy(consoleVariable, variable, Inspect::MAX_STRING_LEN);
      consoleVariable[Inspect::MAX_STRING_LEN] = '\0';
   }
   else
   {
      consoleVariable[0] = '\0';
   }
}   

void Control::setConsoleCommand(const char *newCmd)
{
   if (newCmd)
   {
      strncpy(consoleCommand, newCmd, Inspect::MAX_STRING_LEN);
      consoleCommand[Inspect::MAX_STRING_LEN] = '\0';
   }
   else
   {
      consoleCommand[0] = '\0';
   }
}   

const char * Control::getConsoleCommand(void)
{
	return consoleCommand;
}

void Control::setVariable(const char *value)
{
   if(consoleVariable[0])
      Console->setVariable(consoleVariable, value);
}

void Control::setIntVariable(int value)
{
   if(consoleVariable[0])
      Console->setIntVariable(consoleVariable, value);
}

void Control::setFloatVariable(float value)
{
   if(consoleVariable[0])
      Console->setFloatVariable(consoleVariable, value);
}

const Control* Control::getTopMostParent(void)
{
   Control *topMostParent = this;
   while (topMostParent->parent)
   {
      topMostParent = topMostParent->parent;
   }
   return topMostParent;
}

const char * Control::getVariable(void)
{
   if(consoleVariable[0])
      return Console->getVariable(consoleVariable);
   else return NULL;
}

int Control::getIntVariable(void)
{
   if(consoleVariable[0])
      return Console->getIntVariable(consoleVariable);
   else return 0;
}

float Control::getFloatVariable(void)
{
   if(consoleVariable[0])
      return Console->getFloatVariable(consoleVariable);
   else return 0.0f;
}

SimObject* Control::addObject(SimObject *object)
{
   Control *control = dynamic_cast<Control *>(object);
   AssertFatal(control, "Control::addObject: atempted to add NON Control to set");
	Parent::addObject(object);
   control->parent = this;
   control->setCanvas(root);
   control->setNextResponder(this);
   return (object);
}

void Control::removeObject(SimObject *object)
{
   Control *control = dynamic_cast<Control *>(object);
	Parent::removeObject(object);
   if(control)
   {
      control->parent = NULL;
      control->setCanvas(NULL);
      control->setNextResponder(NULL);
   }
}

bool Control::pointInControl(Point2I &parentCoordPoint)
{
   int xt = parentCoordPoint.x - position.x;
   int yt = parentCoordPoint.y - position.y;
   return xt < extent.x && yt < extent.y && (xt | yt) >= 0;
}

Control* Control::findHitControl(const Point2I &pt)
{
   Point2I ptemp = pt;
   iterator i; // find in z order (last to first)
   for(i = end();;)
   {
      if(i == begin())
         break;
      i--;
      Control *ctrl = dynamic_cast<Control *>(*i);
      if(ctrl->flags.test(Visible) && ctrl->pointInControl(ptemp))
      {
         ptemp.x -= ctrl->position.x;
         ptemp.y -= ctrl->position.y;
         return ctrl->findHitControl(ptemp);
      }
   }
   return this;
}

Control* Control::findControlWithTag(DWORD in_tag)
{
   iterator i; // find in z order (last to first)
   for(i = end();;)
   {
      if(i == begin())
         break;
      i--;
      Control *ctrl = dynamic_cast<Control *>(*i);
      if(ctrl->tag == in_tag)
         return ctrl;
      Control *rctrl = ctrl->findControlWithTag(in_tag);
      if(rctrl)
         return rctrl;
   }
   return NULL;
}

Control* Control::findNamedControl(StringTableEntry name)
{
   iterator i; // find in z order (last to first)
   for(i = end();;)
   {
      if(i == begin())
         break;
      i--;
      Control *ctrl = dynamic_cast<Control *>(*i);
      if(ctrl->getName() == name)
         return ctrl;
      Control *rctrl = ctrl->findNamedControl(name);
      if(rctrl)
         return rctrl;
   }
   return NULL;
}

Control* Control::findRootControlWithTag(DWORD in_tag)
{
   Control *prev, *curr;
   curr = this;
   prev = this->parent;
   while (prev)
   {
      curr = prev;
      prev = curr->parent;
   }
   
   return curr->findControlWithTag(in_tag);
}

Control* Control::findRootNamedControl(StringTableEntry name)
{
   Control *prev, *curr;
   curr = this;
   prev = this->parent;
   while (prev)
   {
      curr = prev;
      prev = curr->parent;
   }
   
   return curr->findNamedControl(name);
}

const char *Control::getScriptValue()
{
   return NULL;
}

void Control::setScriptValue(const char *value)
{
   value;
}

void Control::setCanvas(Canvas *newCanvas)
{
   root = newCanvas;
   iterator i;
   for(i = begin(); i != end(); i++)
   {
      Control *ctrl = dynamic_cast<Control *>(*i);
      ctrl->setCanvas(newCanvas);
   }
}

Point2I Control::localToGlobalCoord(const Point2I &src)
{
   Point2I ret = src;
   ret.x += position.x;
   ret.y += position.y;
   Control *walk = parent;
   while(walk)
   {
      ret.x += walk->position.x;
      ret.y += walk->position.y;
      walk = walk->parent;
   }
   return ret;
}

Point2I Control::globalToLocalCoord(const Point2I &src)
{
   Point2I ret = src;
   ret.x -= position.x;
   ret.y -= position.y;
   Control *walk = parent;
   while(walk)
   {
      ret.x -= walk->position.x;
      ret.y -= walk->position.y;
      walk = walk->parent;
   }
   return ret;
}

DWORD Control::getTag()
{
   return tag;
}   

void Control::setTag(DWORD newTag)
{
   if (manager)
   {
      if(!tag && newTag)
         addToSet(TaggedGuiSetId);
      else if(tag && !newTag)
         removeFromSet(TaggedGuiSetId);
   }
   tag = newTag;
}   

void Control::parentResized(const Point2I &oldParentExtent, const Point2I &newParentExtent)
{
   Point2I newPosition = position;
   Point2I newExtent = extent;

	int deltaX = newParentExtent.x - oldParentExtent.x;
 	int deltaY = newParentExtent.y - oldParentExtent.y;

	if(horizSizing == horizResizeCenter)
	   newPosition.x = (newParentExtent.x - extent.x) >> 1;
	else if(horizSizing == horizResizeWidth)
		newExtent.x += deltaX;
	else if(horizSizing == horizResizeLeft)
      newPosition.x += deltaX;

	if(vertSizing == vertResizeCenter)
	   newPosition.y = (newParentExtent.y - extent.y) >> 1;
	else if(vertSizing == vertResizeHeight)
		newExtent.y += deltaY;
	else if(vertSizing == vertResizeTop)
      newPosition.y += deltaY;

	resize(newPosition, newExtent);
}

void Control::resize(const Point2I &newPosition, const Point2I &newExtent)
{
   iterator i;
   for(i = begin(); i != end(); i++)
   {
      Control *ctrl = dynamic_cast<Control *>(*i);
      ctrl->parentResized(extent, newExtent);
   }
   extent = newExtent;
   position = newPosition;

	if(parent)
		parent->childResized(this);
}

void Control::childResized(Control *child)
{
   child;
   // default to doing nothing...
}

void Control::renderChildControls(GFXSurface *sfc, Point2I offset, const Box2I &updateRect)
{
   // offset is the upper-left corner of this control in screen coordinates
   // updateRect is the intersection rectangle in screen coords of the control
   // hierarchy.  This can be set as the clip rectangle in most cases.
   RectI clipRect(updateRect.fMin, updateRect.fMax);
   clipRect.lowerR -= 1; // inset by 1 for GFX

   iterator i;
   for(i = begin(); i != end(); i++)
   {
      Control *ctrl = (Control *)(*i);
      if(ctrl->flags.test(Visible))
      {
         Point2I newOffset = offset;
         Point2I xt = ctrl->extent;
         newOffset += ctrl->position;
         xt += newOffset;
         Box2I bbox(newOffset, xt);
         
         RectI childClip(bbox.fMin.x, bbox.fMin.y, bbox.fMax.x - 1, bbox.fMax.y - 1);

         if(rectClip(&childClip, &clipRect))
         {
            sfc->setClipRect(&childClip);
            bbox.intersect(updateRect);
            ctrl->onRender(sfc, newOffset, bbox);
         }
      }
   }
}

void Control::onRender(GFXSurface *sfc, Point2I offset, const Box2I &updateRect)
{
   //if opaque, fill the update rect with the fill color
   if (mbOpaque)
   {
      sfc->drawRect2d_f(&RectI(offset, Point2I(offset.x + extent.x - 1, offset.y + extent.y - 1)), fillColor);
   }
   
   //if there's a boarder, draw the boarder
   if (mbBoarder)
   {
      sfc->drawRect2d(&RectI(offset, Point2I(offset.x + extent.x - 1, offset.y + extent.y - 1)), boarderColor);
   }
   
   renderChildControls(sfc, offset, updateRect);
}

Point2I Control::getMinExtent()
{
   return Point2I(4,4);
}

void Control::messageSiblings(DWORD message)
{
   Control::iterator i;
   for(i = parent->begin(); i != parent->end(); i++)
   {
      Control *ctrl = dynamic_cast<Control *>(*i);
      if(ctrl != this)
         ctrl->onMessage(this, message);
   }
}

void Control::awaken(void)
{
   iterator i;
   for(i = begin(); i != end(); i++)
   {
      Control *ctrl = dynamic_cast<Control *>(*i);
      ctrl->awaken();
   }
   onWake();
}

void Control::onWake(void)
{
   // do nothing.
}

void Control::close(void)
{
   iterator i;
   for(i = begin(); i != end(); i++)
   {
      Control *ctrl = dynamic_cast<Control *>(*i);
      ctrl->close();
   }
   onClose();
}

void Control::onClose(void)
{
   // do nothing.
}

void Control::preRender(void)
{
   iterator i;
   for(i = begin(); i != end(); i++)
   {
      Control *ctrl = (Control *) (*i);
      ctrl->preRender();
   }
   onPreRender();
}

void Control::onPreRender(void)
{
   // do nothing.
}

void Control::addControl(Control *ctrl)
{
   addObject(ctrl); // wire it up...
   root->rebuildTabList();
   ctrl->awaken();
}

bool Control::wantsTabListMembership()
{
   return false;
}

Int32 Control::scroll_Line_H(RectI &)
{
   // 0.1 percent  - this is normal scroll ctrl value
   return (Int32)(0.1f * extent.x);
}          

Int32 Control::scroll_Line_V(RectI &)
{
   return (Int32)(0.1f * extent.y);
}  
 
Int32 Control::scroll_Page_H(RectI &)
{
   return (Int32)(0.2f * extent.x);
}   

Int32 Control::scroll_Page_V(RectI &)
{
   return (Int32)(0.2f * extent.y);
}  

void Control::setUpdateRegion(Point2I pos, Point2I ext)
{
   Point2I upos = localToGlobalCoord(pos);
   if(root)
      root->addUpdateRegion(upos, ext);
}

void Control::setUpdate()
{
   setUpdateRegion(Point2I(0,0), extent);
}

void
Control::lockDevice(GFXDevice* io_pDev)
{
   io_pDev->lock();
}

void
Control::unlockDevice(GFXDevice* io_pDev)
{
   io_pDev->unlock();
}

};