// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include "inspectWindow.h"
#include "registry.h"
#include <simTagDictionary.h>
#include <simTreeView.h>
#include <console.h>
#include <stdio.h>
#include "tagDictionary.h"

//------------------------------------------------------------------------------
//                       InspectWindow functions
//------------------------------------------------------------------------------

GWWindowClass InspectWindow::InspectWindowClass;
bool InspectWindow::classRegistered = false;

//------------------------------------------------------------------------------
InspectWindow::InspectWindow(GWWindow *parent, char *name, RECT &r, SimTagDictionary *std)
{
   Point2I pos(r.left,r.top);
   Point2I size(r.right-r.left, r.bottom-r.top);
   init(parent, name, pos, size, std );
}   

InspectWindow::InspectWindow(GWWindow *parent, char *name, Point2I &pos, Point2I &size, SimTagDictionary *std)
{
   init(parent, name, pos, size, std );
}   

void InspectWindow::init(GWWindow *parent, char *name, Point2I& pos, Point2I& size, SimTagDictionary * /*std*/)
{
   // save the initial window parameters, we'll
   // actually create the window when we get
   // a simAddNotifyEvent.
   _parent = parent;
   windowName = new char[strlen(name) + 1];
   if (windowName)
   {
      if (name)
         strcpy(windowName, name);
      else
         windowName[0] = '\0';
   }

   initPos = pos;
   initSize = size;

   toolbarHWnd = NULL;
   toolbarHeight = 0;
   numWindows = 0;
   resizeHWnd = NULL;
   resizeOnInit = true;
   
   applyBtnHWnd = NULL;
   inspectObj = NULL;
   
   minDesSize.set(0,0);
   minValSize.set(0,0);
   contentHeight = 0;
   contentYOrig = 0;
   clientSize = size;

}   

//------------------------------------------------------------------------------
InspectWindow::~InspectWindow()
{
   delete[] windowName;
}   

//------------------------------------------------------------------------------
bool InspectWindow::onAdd()
{          
   if(!Parent::onAdd())
      return false;

   // create the window inspector 
   createWin();  
   
   // initialize messaging structures to query and send events 
   // to the inspectObject.
   // Both inspect query and event must be evaluated immediately by
   // the inspectObject since we are using a single event
   // and query structure for all of our queries and events

   // make sure that the common controls to create Edit boxes, buttons,
   // and list boxes are loaded
   InitCommonControls();
   
   return true;
}   

//------------------------------------------------------------------------------
void InspectWindow::onRemove()
{
   if (inspectObj)
      clearNotify(inspectObj);

   // destroy any child windows we have
   removeChildWindows();

   // the window is destoryed in the base class' destructor      
   Parent::onRemove();
}

//------------------------------------------------------------------------------
void InspectWindow::onDeleteNotify(SimObject *object)
{
   if (inspectObj == object)
      inspectObj = NULL;
   Parent::onDeleteNotify(object);
}

//------------------------------------------------------------------------------

bool InspectWindow::setPrefFlag( const char * name, const bool value )
{
   if( !name || !strlen( name ) )
      return( false );
      
   int val = value;
   
   Registry reg;
   
   // get to where we want and write the flag
   if (reg.open("/HKEY_CURRENT_USER/Software/Dynamix/SimEngine/GWPrefs"))
      if( reg.write( name, val ) )
         return( true );
               
   return( false );
}

//------------------------------------------------------------------------------

bool InspectWindow::getPrefFlag( const char * name, bool & value )
{
   if( !name || !strlen( name ) )
      return( false );
      
   int val;
   
   Registry reg;
   
   // get to where we want and read the flag
   if (reg.open("/HKEY_CURRENT_USER/Software/Dynamix/SimEngine/GWPrefs"))
      if( reg.read( name, &val ) )
      {
         value = (bool)val;
         return( true );
      }
               
   return( false );
}

//------------------------------------------------------------------------------
void InspectWindow::createWin()
{
   if (!classRegistered)
   {
      InspectWindowClass.registerClass(CS_HREDRAW | CS_VREDRAW, 0, 0, (HBRUSH)(COLOR_BTNFACE + 1));
      classRegistered = true;
   }


   if ( GWWindow::createEx( WS_EX_TOOLWINDOW, &InspectWindowClass, windowName,
                          InspectWindowStyle,
                          initPos,
                          initSize,
                          _parent, 0, 0, true, true ) )
   {
      show(SW_SHOW);
      update();
   }
   else
   {
      AssertFatal(0, "could not create InspectWindow window");
   }
   
   // grab the resize flag from the registry
   getPrefFlag( "MissionEditor::resizeOnInit", resizeOnInit );
   
   initDisplay();                                                     
}

//------------------------------------------------------------------------------
void InspectWindow::createChildWindows()
{
   if (inspectObj)
   {
      inspector.clear();

      inspectObj->inspectWrite(&inspector);
      {
         // first create windows for every member 
         // returned in the inspector query and
         // obtain the minimum member window sizes
         inspector.top();
         Inspect::Type type;
         SimTag des;

         minDesSize.set(0,0);
         minValSize.set(0,0);
         contentHeight = 0;
         
         InspectMemberWin *mb = NULL;

         while ((type = inspector.peekNextType()) != Inspect::Type_Invalid)
         {
            des = inspector.peekNextDes();
            switch (type)
            {
               //--------------------------------------
               case Inspect::Type_Divider: {
                  inspector.readDivider();
                  mb = new Divider_MemberWin;
                  if (mb)
                     static_cast<Divider_MemberWin *>(mb)->create(getHandle(), hInstance); 
                  break;
               }

               //--------------------------------------
               case Inspect::Type_ActionBtn: {
                  Inspect_ActionBtn val;

                  inspector.readActionButton(des, &val.recipient, &val.simMessage);
                  mb = new ActionBtn_MemberWin;
                  if (mb)
                     static_cast<ActionBtn_MemberWin *>(mb)->create(getHandle(), hInstance, des, val, manager); 
                  break;
               }

               //--------------------------------------
               case Inspect::Type_Point3F: {
                  Point3F val;

                  inspector.read(des, val);
                  mb = new Point3F_MemberWin;
                  if (mb)
                     static_cast<Point3F_MemberWin *>(mb)->create(getHandle(), hInstance, des, val); 
                  break;
               }

               //--------------------------------------
               case Inspect::Type_Point2F: {
                  Point2F val;

                  inspector.read(des, val);
                  
                  mb = new Point2F_MemberWin;
                  if (mb)
                     static_cast<Point2F_MemberWin *>(mb)->create(getHandle(), hInstance, des, val); 
                  break;
               }

               //--------------------------------------
               case Inspect::Type_Point3I: {
                  Point3I val;

                  inspector.read(des, val);
                  
                  mb = new Point3I_MemberWin;
                  if (mb)
                     static_cast<Point3I_MemberWin *>(mb)->create(getHandle(), hInstance, des, val); 
                  break;
               }

               //--------------------------------------
               case Inspect::Type_Point2I: {
                  Point2I val;

                  inspector.read(des, val);
                  mb = new Point2I_MemberWin;
                  if (mb)
                     static_cast<Point2I_MemberWin *>(mb)->create(getHandle(), hInstance, des, val); 
                  break;
               }

               //--------------------------------------
               case Inspect::Type_List: {
                  switch (inspector.peekNextListType())
                  {
                     case Inspect::Type_SimTagValPair: {
                        Int32 stvLen = inspector.peekNextListSize();
                        Inspect::SimTagValPair *stvList = new Inspect::SimTagValPair[stvLen];
                        Int32 val;                        

                        inspector.read(des, val, stvLen, stvList);
                        mb = new SimTagValPair_MemberWin;
                        if (mb)
                           static_cast<SimTagValPair_MemberWin *>(mb)->create(getHandle(), hInstance, des, val, stvLen, stvList); 
                        break;
                     }
                     case Inspect::Type_IStringValPair: {
                        Int32 svLen = inspector.peekNextListSize();
                        Inspect::IStringValPair *svList = new Inspect::IStringValPair[svLen];
                        Int32 val;

                        inspector.read(des, val, svLen, svList);
                        mb = new IStringValPair_MemberWin;
                        if (mb)
                           static_cast<IStringValPair_MemberWin *>(mb)->create(getHandle(), hInstance, des, val, svLen, svList); 
                        break;
                     }
                     default:
                        AssertFatal(0, "Unknown list type!");      
                  }
                  break;
               }

               //--------------------------------------
               case Inspect::Type_TagRange: {
                  Inspect_TagRange tag;

                  inspector.read(des, &tag.listDefines, &tag.minTag, &tag.maxTag, tag.initTag );
                  if (tag.listDefines)
                  {
                     mb = new Show_TagRange_Define_MemberWin;
                     if (mb)
                        static_cast<Show_TagRange_Define_MemberWin *>(mb)->create(getHandle(), hInstance, des, tag.minTag, tag.maxTag, tag.initTag); 
                  }
                  else
                  {
                     mb = new Show_TagRange_IString_MemberWin;
                     if (mb)
                        static_cast<Show_TagRange_IString_MemberWin *>(mb)->create(getHandle(), hInstance, des, tag.minTag, tag.maxTag, tag.initTag); 
                  }
                  break;
               }

               //--------------------------------------
               case Inspect::Type_Tag: {
                  Inspect_Tag tag;

                  inspector.read(des, &tag.listDefines, tag.filter, tag.tagVal );
                  if (tag.listDefines)
                  {
                     mb = new Show_Tag_Define_MemberWin;
                     if (mb)
                        static_cast<Show_Tag_Define_MemberWin *>(mb)->create(getHandle(), hInstance, des, tag.filter, tag.tagVal); 
                  }
                  else
                  {
                     mb = new Show_Tag_IString_MemberWin;
                     if (mb)
                        static_cast<Show_Tag_IString_MemberWin *>(mb)->create(getHandle(), hInstance, des, tag.filter, tag.tagVal); 
                  }
                  break;
               }

               //--------------------------------------
               case Inspect::Type_Bool: {
                  Bool val;

                  inspector.read(des, val);
                  mb = new Bool_MemberWin;
                  if (mb)
                     static_cast<Bool_MemberWin *>(mb)->create(getHandle(), hInstance, des, val); 
                  break;
               }

               //--------------------------------------
               case Inspect::Type_RealF: {
                  RealF val;

                  inspector.read(des, val);
                  mb = new RealF_MemberWin;
                  if (mb)
                     static_cast<RealF_MemberWin *>(mb)->create(getHandle(), hInstance, des, val); 
                  break;
               }

               //--------------------------------------
               case Inspect::Type_Int32: {
                  Int32 val;
                  Inspect::Display display;

                  inspector.read(des, val, &display);
                  mb = new Int32_MemberWin;
                  if (mb)
                     static_cast<Int32_MemberWin *>(mb)->create(getHandle(), hInstance, des, val, display); 
                  break;
               }

               //--------------------------------------
               case Inspect::Type_IString:{
                  char val[Inspect::MAX_STRING_LEN + 1];

                  inspector.read(des, val);
                  mb = new IString_MemberWin;
                  if (mb)
                     static_cast<IString_MemberWin *>(mb)->create(getHandle(), hInstance, des, val); 
                  break;
               }

               //--------------------------------------
               case Inspect::Type_Description:{
                  inspector.read(des);
                  mb = new Description_MemberWin;
                  if (mb)
                     static_cast<Description_MemberWin *>(mb)->create(getHandle(), hInstance, des); 
                  break;
               }

               //--------------------------------------
               case Inspect::Type_StringDescription:{
                  const char* pString;
                  inspector.readStringDes(pString);
                  mb = new StringDescription_MemberWin;
                  if (mb)
                     static_cast<StringDescription_MemberWin *>(mb)->create(getHandle(), hInstance, pString); 
                  break;
               }

               case Inspect::Type_StringIndexDescription:{
                  const char* pString;
                  int         index;
                  inspector.readStringIndexDes(pString, index);
                  mb = new StringIndexDescription_MemberWin;
                  if (mb)
                     static_cast<StringIndexDescription_MemberWin *>(mb)->create(getHandle(), hInstance, pString, index); 
                  break;
               }

               default:
                  MessageBox(0, "Unknown Inspect type", "InspectEditWin error", MB_OK|MB_ICONHAND );
            }
            
            if (mb)
            {
               memberList.push_back(mb);

               // get minimum window sizes
               Point2I size = mb->getDesMinSize();
               if (minDesSize.x < size.x) minDesSize.x = size.x;
               if (minDesSize.y < size.y) minDesSize.y = size.y;

               size = mb->getValMinSize();
               if (minValSize.x < size.x) minValSize.x = size.x;
               if (minValSize.y < size.y) minValSize.y = size.y;
            
               contentHeight += (mb->getCurSize()).y;
            } 
              
         } // while

         // create the toolbar and apply button
         createToolbar();
         
         contentHeight += InspectMemberWin::DBL_BORDER_SPACING + toolbarHeight;
      } 
      
      // go through and count all the child windows created
      Vector<InspectMemberWin *>::iterator iter;
      InspectMemberWin *mb;
      numWindows = 0;
      for( iter = memberList.begin(); iter != memberList.end(); iter++ )
      {
         mb = *iter;
         if( mb->desHWnd )
            numWindows++;
         if( mb->valHWnd )
            numWindows++;
      }
   }   
}   

//------------------------------------------------------------------------------
void InspectWindow::createToolbar()
{
   static TBBUTTON tbButtons[] = { { 0, 0, TBSTATE_ENABLED, TBSTYLE_SEP, 0L, 0 } };

   int numButtons = sizeof( tbButtons ) / sizeof( TBBUTTON );
   
   // create the window
   toolbarHWnd = ::CreateToolbarEx( getHandle(), 
      WS_CHILD | WS_BORDER | WS_VISIBLE | TBSTYLE_TOOLTIPS | WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
      CCS_NOMOVEY | CCS_TOP | CCS_NOPARENTALIGN, 0, 0, getHInstance(), 0,
      tbButtons, numButtons, 16, 16, 16, 16, sizeof( TBBUTTON ) );

   applyBtnHWnd = ::CreateWindow( "BUTTON", "Apply",  WS_CLIPSIBLINGS | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 
      4, 0, 60, 24, toolbarHWnd, ( HMENU )0, hInstance, NULL );
   
   resizeHWnd = ::CreateWindow( "BUTTON", "Resize", WS_CLIPSIBLINGS | WS_VISIBLE | WS_CHILD |
      BS_AUTOCHECKBOX, 70, 0, 80, 24, toolbarHWnd, ( HMENU )0, hInstance, NULL );

   lockedHWnd = ::CreateWindow( "BUTTON", "Locked", WS_CLIPSIBLINGS | WS_VISIBLE | WS_CHILD |
      BS_AUTOCHECKBOX, 156, 0, 80, 24, toolbarHWnd, ( HMENU )0, hInstance, NULL );
      
   // get the height of the toolbar
   RECT rect;
   ::GetWindowRect( toolbarHWnd, &rect );
   toolbarHeight = rect.bottom - rect.top;
   
   SetWindowPos( toolbarHWnd, HWND_TOPMOST, 0, 0, clientSize.x, toolbarHeight, 0 );
}

//------------------------------------------------------------------------------
void InspectWindow::positionChildWindows(int vertOrig)                                                  
{
   if (memberList.empty())
      return;

   // position all the struct member windows
   Vector<InspectMemberWin *>::iterator iter;
   InspectMemberWin *mb;
   Box2I bbox;
   int curY = vertOrig;

   // get a defer window pos handle + toolbar window
   HDWP deferHandle = ::BeginDeferWindowPos( numWindows + 1 );
   
   // have to use MoveWindow to really move the toolbar
   ::MoveWindow( toolbarHWnd, 0, 0, clientSize.x, toolbarHeight, true );
   
   // position the toolbar
   ::DeferWindowPos( deferHandle, toolbarHWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOREDRAW );
   
   curY += toolbarHeight + InspectMemberWin::BORDER_SPACING;

   // position all the windows and set the z-order
   for (iter = memberList.begin(); iter != memberList.end(); iter++)
   {
      mb = *iter;

      bbox.fMin.x = 0;
      bbox.fMin.y = curY;
      curY += (mb->getCurSize()).y;
      bbox.fMax.x = clientSize.x;
      bbox.fMax.y = curY;
      
      mb->position(bbox, minDesSize);    
      if (mb->desHWnd)
         ::DeferWindowPos( deferHandle, mb->desHWnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOREDRAW );
      if (mb->valHWnd)
         ::DeferWindowPos( deferHandle, mb->valHWnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOREDRAW );
   }
   
   ::EndDeferWindowPos( deferHandle );
   
   // make sure the toolbar window is clear
   ::InvalidateRect( toolbarHWnd, NULL, true );
}   

//------------------------------------------------------------------------------
void InspectWindow::removeChildWindows()
{
   while (! memberList.empty())
   {
      InspectMemberWin *member = memberList.front();
      memberList.pop_front();
      delete member;
   }

   if( toolbarHWnd )
   { 
      // grab the resize flag from the registry
      setPrefFlag( "MissionEditor::resizeOnInit", resizeOnInit );
      
      DestroyWindow( applyBtnHWnd );
      DestroyWindow( resizeHWnd );
      DestroyWindow( lockedHWnd );
      DestroyWindow( toolbarHWnd );
      applyBtnHWnd = NULL;
      resizeHWnd = NULL;
      lockedHWnd = NULL;
      toolbarHWnd = NULL;
   }

   minDesSize.set(0,0);
   minValSize.set(0,0);
   toolbarHeight = 0;
   contentHeight = 0;
   contentYOrig = 0;
} 

//------------------------------------------------------------------------------
void InspectWindow::initDisplay()
{
   // check if the window is already up ( valid toolbar window )
   if( !resizeOnInit )
   {
      // create the child windows
      removeChildWindows();
      createChildWindows();
   }
   else
   {
      // create the child windows
      removeChildWindows();
      createChildWindows();
      
      // position them in the window
      if (memberList.empty() )
      {
         clientSize.set(100,100);
      }
      else
      {
         clientSize.x = minDesSize.x + minValSize.x;
         clientSize.y = (contentHeight > 600) ? 600 : contentHeight;
      }
      
      setClientSize(clientSize);
   }

   // set / clear the check on the resize option
   if( toolbarHWnd )
   {
      SendMessage( resizeHWnd, BM_SETCHECK, resizeOnInit ? BST_CHECKED : BST_UNCHECKED, 0 );
      SendMessage( lockedHWnd, BM_SETCHECK, ( inspectObj->isLocked() ) ? BST_CHECKED : BST_UNCHECKED, 0 );
      EnableWindow( applyBtnHWnd, !inspectObj->isLocked() );
   }
      
   // update the window
   onWindowPosChanged(NULL);
   update();
}   

//------------------------------------------------------------------------------
BOOL InspectWindow::onWindowPosChanging(LPWINDOWPOS lpwpos)
{
   RECT minWindRect;
    
   minWindRect.left = 0;
	minWindRect.top = 0;
	minWindRect.right = minDesSize.x + minValSize.y + InspectMemberWin::SCROLLBAR_WIDTH;
	minWindRect.bottom = (contentHeight > 100) ? 100 : contentHeight;
   
   AdjustWindowRect(&minWindRect, InspectWindowStyle, FALSE);
   minWindRect.right -= minWindRect.left;
   minWindRect.bottom -= minWindRect.top;
   
   if (lpwpos->cx < minWindRect.right)
      lpwpos->cx = minWindRect.right;

   if (lpwpos->cy < minWindRect.bottom)
      lpwpos->cy = minWindRect.bottom;

   return 0;   
}

//------------------------------------------------------------------------------
void InspectWindow::onWindowPosChanged(const LPWINDOWPOS)
{
   RECT clientRect;

   GetClientRect(getHandle(), &clientRect);
   
   clientSize.x = clientRect.right;
   clientSize.y = clientRect.bottom;

   int pos= (clientSize.y >= contentHeight) ? 0 : GetScrollPos(getHandle(), SB_VERT);
   onVScroll(0, SB_THUMBTRACK, pos);
}   

//------------------------------------------------------------------------------
void InspectWindow::onVScroll(HWND, UINT nScrollCode, int pos)
{
   if (clientSize.y >= contentHeight)
   {
      EnableScrollBar(getHandle(), SB_VERT, ESB_DISABLE_DOWN | ESB_DISABLE_UP);
      contentYOrig = 0;
   }
   else
   {
      EnableScrollBar(getHandle(), SB_VERT, ESB_ENABLE_BOTH);

      if ( (SB_THUMBTRACK != nScrollCode) && (SB_THUMBPOSITION != nScrollCode) )
      {
         pos = GetScrollPos(getHandle(), SB_VERT);    
         
         Int32 maxInvisibleRegion =  contentHeight - clientSize.y;
         Int32 lineUpDownInc = 10;
         lineUpDownInc = (Int32)(((float)lineUpDownInc / (float)maxInvisibleRegion) * 100.0f);
         
         Int32 pageUpDownInc = (clientSize.y > 20) ? (clientSize.y - 10) : 10;
         pageUpDownInc = (Int32)(((float)pageUpDownInc / (float)maxInvisibleRegion) * 100.0f);
         
         switch (nScrollCode)
         {
            case SB_BOTTOM:
               pos = 100;
               break;
            case SB_TOP:
               pos = 0;
               break; 
            case SB_LINEUP:
               pos-= lineUpDownInc;
               break;
            case SB_PAGEUP:
               pos-= pageUpDownInc;
               break;
            case SB_LINEDOWN:
               pos+= lineUpDownInc;
               break;
            case SB_PAGEDOWN:
               pos+= pageUpDownInc;
               break; 
         }
         if (pos < 0) pos = 0;
         if (pos > 100) pos = 100;
      }

      contentYOrig = (int) ((float)(contentHeight - clientSize.y) * ( (float)pos / -100.0f ));    
      SetScrollPos(getHandle(), SB_VERT, pos, true);
   }
   positionChildWindows(contentYOrig);
   update();
}  

//------------------------------------------------------------------------------
void InspectWindow::onClose()
{
   GWWindow::onClose();
   if (manager)
      manager->deleteObject(this);   
}   

//------------------------------------------------------------------------------
void InspectWindow::onCommand(int, HWND hwndCtl, UINT codeNotify)
{
   if (codeNotify != BN_CLICKED)
      return;

   if (memberList.empty() || !inspectObj)
      return;

   Vector<InspectMemberWin *>::iterator iter;
   InspectMemberWin *mb;

   if( ( hwndCtl != applyBtnHWnd ) && ( hwndCtl != resizeHWnd ) && ( hwndCtl != lockedHWnd ) )
   {
      // see if it's an action button
      for (iter = memberList.begin(); iter != memberList.end(); iter++)
      {
         mb = *iter; 
         switch ( mb->getType() )
         {
            case Inspect::Type_ActionBtn: {
               if (mb->valHWnd == hwndCtl)
               {
                  static_cast<ActionBtn_MemberWin *>(mb)->doAction();
               	initDisplay();
               	return;
               }
               break;
            }
         }
      }
      return;
   }
   
   // check for the resize checkbox
   if( hwndCtl == resizeHWnd )
   {
      // set the resize flag
      resizeOnInit = false;
      if( SendMessage( resizeHWnd, BM_GETCHECK, 0, 0 ) == BST_CHECKED )
         resizeOnInit = true;
      return;
   }

   // check for locked
   if( hwndCtl == lockedHWnd )
   {
      bool b = ( SendMessage( lockedHWnd, BM_GETCHECK, 0, 0 ) == BST_CHECKED );
      
      inspectObj->setLocked( b );
      EnableWindow( applyBtnHWnd, !b );

      // update the parent tree view if there is one
      SimTreeView * pTreeView = dynamic_cast< SimTreeView * >( _parent );
      if( pTreeView )
      {
         HTREEITEM hItem = pTreeView->findItem( inspectObj );
         
         if( hItem )
         {
            pTreeView->refresh( hItem );
            pTreeView->setModified();
         }
      }
      
      return;
   }

   // apply button was pushed - check if locked
   if( inspectObj->isLocked() )
      return;
   
   // construct a SimInspectEvent from 
   // current values of the inspector members
   inspector.clear();
   
   Bool noErr = true;
   for (iter = memberList.begin(); iter != memberList.end(); iter++)
   {
      mb = *iter;

      SimTag des = mb->getDes();

      switch ( mb->getType() )
      {
         case Inspect::Type_Description: {
            inspector.write(des);
            break;
         }
         case Inspect::Type_StringDescription: {
            const char* pString =
               static_cast<StringDescription_MemberWin *>(mb)->getVal();
            inspector.writeStringDes(pString);
            break;
         }
         case Inspect::Type_StringIndexDescription: {
            const char* pString =
               static_cast<StringIndexDescription_MemberWin *>(mb)->getStringVal();
            int index = 
               static_cast<StringIndexDescription_MemberWin *>(mb)->getIndexVal();
            inspector.writeStringIndexDes(pString, index);
            break;
         }
         case Inspect::Type_IString: {
            char buffer[Inspect::MAX_STRING_LEN + 1];
            noErr = static_cast<IString_MemberWin *>(mb)->getVal(buffer, Inspect::MAX_STRING_LEN + 1 );
            if (noErr)
               inspector.write(des, buffer);
            break;
         }
         case Inspect::Type_Int32: {
            Int32 val;
            noErr = static_cast<Int32_MemberWin *>(mb)->getVal(val);
            if (noErr)
               inspector.write(des, val);
            break;
         }
         case Inspect::Type_RealF: {
            RealF val;
            noErr = static_cast<RealF_MemberWin *>(mb)->getVal(val);
            if (noErr)
               inspector.write(des, val);
            break;
         }
         case Inspect::Type_Bool: {
            Bool val;
            noErr = static_cast<Bool_MemberWin *>(mb)->getVal(val);
            if (noErr)
               inspector.write(des, val);
            break;
         }
         case Inspect::Type_Tag: {
            Int32 val;
            Show_Tag_Define_MemberWin *sd = dynamic_cast<Show_Tag_Define_MemberWin *>(mb);
            if (sd)
            {
               sd->getVal(val);
               inspector.write(des, true, NULL, val);
               noErr = true;
               break;
            }
            Show_Tag_IString_MemberWin *ss = dynamic_cast<Show_Tag_IString_MemberWin *>(mb);
            if (ss)
            {
               ss->getVal(val);
               inspector.write(des, false, NULL, val);
               noErr = true;
               break;
            }
            noErr = false;
            break;
         }
         case Inspect::Type_TagRange: {
            Int32 val;
            Show_TagRange_Define_MemberWin *sd = dynamic_cast<Show_TagRange_Define_MemberWin *>(mb);
            if (sd)
            {
               sd->getVal(val);
               inspector.write(des, true, 0, 1, val);
               noErr = true;
               break;
            }
            Show_TagRange_IString_MemberWin *ss = dynamic_cast<Show_TagRange_IString_MemberWin *>(mb);
            if (ss)
            {
               ss->getVal(val);
               inspector.write(des, false, 0, 1, val);
               noErr = true;
               break;
            }
            noErr = false;
            break;
         }
         case Inspect::Type_SimTagValPair: {
            Int32 val, stvLen;
            noErr = static_cast<SimTagValPair_MemberWin *>(mb)->getVal(val, stvLen);
            if (noErr)
               inspector.write(des, val, stvLen, NULL);
            break;
         }
         case Inspect::Type_IStringValPair: {
            Int32 val, svLen;
            noErr = static_cast<IStringValPair_MemberWin *>(mb)->getVal(val, svLen);
            if (noErr)
               inspector.writeIStringPairList(des, val, svLen, NULL);
            break;
         }
         case Inspect::Type_Point2I: {
            Point2I val;
            noErr = static_cast<Point2I_MemberWin *>(mb)->getVal(val);
            if (noErr)
               inspector.write(des, val);
            break;
         }
         case Inspect::Type_Point3I: {
            Point3I val;
            noErr = static_cast<Point3I_MemberWin *>(mb)->getVal(val);
            if (noErr)
               inspector.write(des, val);
            break;
         }
         case Inspect::Type_Point2F: {
            Point2F val;
            noErr = static_cast<Point2F_MemberWin *>(mb)->getVal(val);
            if (noErr)
               inspector.write(des, val);
            break;
         }
         case Inspect::Type_Point3F: {
            Point3F val;
            noErr = static_cast<Point3F_MemberWin *>(mb)->getVal(val);
            if (noErr)
               inspector.write(des, val);
            break;
         }
         case Inspect::Type_ActionBtn: {
            noErr = static_cast<ActionBtn_MemberWin *>(mb)->getVal();
            if (noErr)
               inspector.writeActionButton(des, NULL, 0);
            break;
         }
         case Inspect::Type_Divider: {
            inspector.writeDivider();
            break;
         }
         default:
            MessageBox(0, "Unknown Inspect Type, could not construct event", "InspectEditWin error", MB_OK|MB_ICONHAND );
      }

      if (! noErr) 
         break;
   }

   inspector.top();
   if (noErr)
      inspectObj->inspectRead(&inspector);

	initDisplay();

}   

//------------------------------------------------------------------------------
void InspectWindow::setObjToInspect(SimObject *obj, const char *lpszClass)
{
   if (!manager)
      return;

   if (inspectObj)
      clearNotify(inspectObj);
   inspectObj = obj;
   if (inspectObj)
   {
      deleteNotify(inspectObj);
      if (lpszClass)
      {
         char szName [256];
         char szTitle[256];

         if (obj->getName())
         {
            strcpy(szName, obj->getName());
         }
         else
         {
            sprintf(szName, "%d", obj->getId());
         }

         sprintf(szTitle, "%s - %s [%s]", windowName, lpszClass, szName);            
         SetWindowText(getHandle(), szTitle);
      }
      else
      {
         SetWindowText(getHandle(), windowName);
      }
      initDisplay();
   }
}

//------------------------------------------------------------------------------
//                       InspectMemberWin functions
//------------------------------------------------------------------------------

char *InspectMemberWin::STR_NONE = "<NONE>";

//------------------------------------------------------------------------------
// convert the buffer to a float number checking if actually changed first
double InspectMemberWin::atof( const char * buffer, const double orig_val )
{
   char buf[ 64 ];
   sprintf( buf, "%f", orig_val );
   if( !stricmp( buf, buffer ) )
      return( orig_val );
      
   return( ::atof( buffer ) );
}

// get a float string from a string of comma seperated floats ( for use with atof( buf, val )
bool InspectMemberWin::getFloatString( const char * buffer, const unsigned int index,      
   char * floatBuf, const unsigned int floatBufSize )
{
   if( !buffer || !floatBuf || !floatBufSize )
      return( false );
   
   char * ps = const_cast<char *>(buffer);
   
   // get to the substring we want
   for( unsigned int i = 0; i < index; i++ )
   {
      ps = ::strchr( ps, ',' );
      if( !ps )
         return( false );
         
      // get to the next char
      ps++;
   }
   
   // remove all whitespace
   while( ( *ps == ' ' ) || ( *ps == '\t' ) )
      ps++;   
   
   unsigned int count = 0;
   
   // now go through and copy the string
   while( *ps && ( *ps != ',' ) )
   {
      floatBuf[count] = *ps;
      
      // advance
      *ps++;
      count++;
      
      if( count >= floatBufSize )
      {
         floatBuf[0] = '\0';
         return( false );
      }
   }
   
   // terminate
   floatBuf[count] = '\0';
   
   return( true );
}
         

//------------------------------------------------------------------------------
void InspectMemberWin::displayErrDialog(const char *message)
{
   AssertFatal(message, "need message");

   char fieldDes[Inspect::MAX_STRING_LEN + 1];
   fieldDes[0] = '\0';
   if (desHWnd)
   {
      GetWindowText(desHWnd, fieldDes, Inspect::MAX_STRING_LEN);
   }

   char buffer[Inspect::MAX_STRING_LEN + 30];

   sprintf(buffer, "In field: %s, %s", fieldDes, message);
   MessageBox(0, buffer, "Input Error", MB_OK|MB_ICONHAND );
} 

//------------------------------------------------------------------------------
//                       Description_MemberWin functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void Description_MemberWin::create(const HWND parent, HINSTANCE hInst, const SimTag fieldDes)
{
   HDC hdc;
   SIZE size;

   des = fieldDes;

   // initially create small windows at root, we'll position them later
   desMinSize.set(10, 10);
   valMinSize.set(10, 10);
   curSize.set(20, 20);

   // get the member description string from SimTag Dictionary
   const char* memberDes = NULL;
   char buffer[256];
   memberDes = tagDictionary.idToString(des);      

   if (!memberDes)
   {
      sprintf(buffer, "Undefined Inspect Tag %d", des);
      memberDes = buffer;
   }

   // create the window for the member description
   desHWnd = ::CreateWindow("STATIC", memberDes, WS_CHILD | WS_CLIPSIBLINGS | SS_RIGHT, 
                            0, 0, desMinSize.x, desMinSize.y, parent, (HMENU)0, hInst, NULL);

   if (! desHWnd)
      return;

   // get the minimum window size for the member description 
   hdc = GetDC(desHWnd);
   if (hdc)
   {
      GetTextExtentPoint32(hdc, memberDes, strlen(memberDes), &size);
      desMinSize.set(size.cx + DBL_BORDER_SPACING, size.cy + DBL_BORDER_SPACING);
   }
   ReleaseDC(desHWnd, hdc);

   // get bounding size for both min values
   curSize.x = valMinSize.x + desMinSize.x;
   curSize.y = (valMinSize.y > desMinSize.y) ? valMinSize.y : desMinSize.y;
}   

//------------------------------------------------------------------------------
void Description_MemberWin::position(Box2I &bbox, Point2I &recommendedDesSize)
{
   curSize.x = bbox.fMax.x - bbox.fMin.x;
   curSize.y = bbox.fMax.y - bbox.fMin.y;

   SetWindowPos(desHWnd, 0, 
                bbox.fMin.x + BORDER_SPACING, 
                bbox.fMin.y + BORDER_SPACING, 
                recommendedDesSize.x - DBL_BORDER_SPACING, 
                curSize.y - DBL_BORDER_SPACING, 
                SWP_NOZORDER | SWP_SHOWWINDOW);
} 


//------------------------------------------------------------------------------
//                       StringDescription_MemberWin functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void StringDescription_MemberWin::create(const HWND parent, HINSTANCE hInst,
                                         const char* string)
{
   HDC hdc;
   SIZE size;

   // initially create small windows at root, we'll position them later
   desMinSize.set(10, 10);
   valMinSize.set(10, 10);
   curSize.set(20, 20);

   pString = string;

   // create the window for the member description
   desHWnd = ::CreateWindow("STATIC", pString, WS_CHILD | WS_CLIPSIBLINGS | SS_RIGHT, 
                            0, 0, desMinSize.x, desMinSize.y, parent, (HMENU)0, hInst, NULL);

   if (! desHWnd)
      return;

   // get the minimum window size for the member description 
   hdc = GetDC(desHWnd);
   if (hdc)
   {
      GetTextExtentPoint32(hdc, pString, strlen(pString), &size);
      desMinSize.set(size.cx + DBL_BORDER_SPACING, size.cy + DBL_BORDER_SPACING);
   }
   ReleaseDC(desHWnd, hdc);

   // get bounding size for both min values
   curSize.x = valMinSize.x + desMinSize.x;
   curSize.y = (valMinSize.y > desMinSize.y) ? valMinSize.y : desMinSize.y;
}   

//------------------------------------------------------------------------------
void StringDescription_MemberWin::position(Box2I &bbox, Point2I &recommendedDesSize)
{
   curSize.x = bbox.fMax.x - bbox.fMin.x;
   curSize.y = bbox.fMax.y - bbox.fMin.y;

   SetWindowPos(desHWnd, 0, 
                bbox.fMin.x + BORDER_SPACING, 
                bbox.fMin.y + BORDER_SPACING, 
                recommendedDesSize.x - DBL_BORDER_SPACING, 
                curSize.y - DBL_BORDER_SPACING, 
                SWP_NOZORDER | SWP_SHOWWINDOW);
} 


//------------------------------------------------------------------------------
//                       StringDescription_MemberWin functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void StringIndexDescription_MemberWin::create(const HWND parent, HINSTANCE hInst,
                                              const char* string,
                                              int         index)
{
   HDC hdc;
   SIZE size;

   // initially create small windows at root, we'll position them later
   desMinSize.set(10, 10);
   valMinSize.set(10, 10);
   curSize.set(20, 20);

   strIdxDes.string = string;
   strIdxDes.index  = index;

   char temp[512];
   sprintf(temp, "%s (%d)", string, index);

   // create the window for the member description
   desHWnd = ::CreateWindow("STATIC", temp, WS_CHILD | WS_CLIPSIBLINGS | SS_RIGHT, 
                            0, 0, desMinSize.x, desMinSize.y, parent, (HMENU)0, hInst, NULL);
   if (! desHWnd)
      return;

   // get the minimum window size for the member description 
   hdc = GetDC(desHWnd);
   if (hdc)
   {
      GetTextExtentPoint32(hdc, temp, strlen(temp), &size);
      desMinSize.set(size.cx + DBL_BORDER_SPACING, size.cy + DBL_BORDER_SPACING);
   }
   ReleaseDC(desHWnd, hdc);

   // get bounding size for both min values
   curSize.x = valMinSize.x + desMinSize.x;
   curSize.y = (valMinSize.y > desMinSize.y) ? valMinSize.y : desMinSize.y;
}   

//------------------------------------------------------------------------------
void StringIndexDescription_MemberWin::position(Box2I &bbox, Point2I &recommendedDesSize)
{
   curSize.x = bbox.fMax.x - bbox.fMin.x;
   curSize.y = bbox.fMax.y - bbox.fMin.y;

   SetWindowPos(desHWnd, 0, 
                bbox.fMin.x + BORDER_SPACING, 
                bbox.fMin.y + BORDER_SPACING, 
                recommendedDesSize.x - DBL_BORDER_SPACING, 
                curSize.y - DBL_BORDER_SPACING, 
                SWP_NOZORDER | SWP_SHOWWINDOW);
} 


//------------------------------------------------------------------------------
//                       Divider_MemberWin functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void Divider_MemberWin::create(const HWND parent, HINSTANCE hInst)
{
   // initially create small windows at root, we'll position them later
   desMinSize.set(10, 6);
   valMinSize.set(10, 6);
   curSize.set(20, 6);

   // create the window for the member description
   desHWnd = ::CreateWindow("STATIC", NULL, WS_CHILD | WS_CLIPSIBLINGS | SS_GRAYRECT, 
                            0, 0, desMinSize.x, desMinSize.y, parent, (HMENU)0, hInst, NULL);
   valHWnd = ::CreateWindow("STATIC", NULL, WS_CHILD | WS_CLIPSIBLINGS | SS_WHITERECT, 
                            0, 0, desMinSize.x, desMinSize.y, parent, (HMENU)0, hInst, NULL);
}   

//------------------------------------------------------------------------------
void Divider_MemberWin::position(Box2I &bbox, Point2I &)
{
   curSize.x = bbox.fMax.x - bbox.fMin.x;
   curSize.y = bbox.fMax.y - bbox.fMin.y;

   SetWindowPos(desHWnd, 0, 
                bbox.fMin.x + BORDER_SPACING, 
                bbox.fMin.y + BORDER_SPACING, 
                curSize.x - DBL_BORDER_SPACING, 
                1, 
                SWP_NOZORDER | SWP_SHOWWINDOW); 
   SetWindowPos(valHWnd, 0, 
                bbox.fMin.x + BORDER_SPACING, 
                bbox.fMin.y + BORDER_SPACING + 1, 
                curSize.x - DBL_BORDER_SPACING, 
                1, 
                SWP_NOZORDER | SWP_SHOWWINDOW);  
} 


//------------------------------------------------------------------------------
//                       IString_MemberWin functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void IString_MemberWin::create(const HWND parent, HINSTANCE hInst, const SimTag fieldDes, const char *val)
{
   HDC hdc;
   SIZE size;

   des = fieldDes;

   // initially create small windows at root, we'll position them later
   desMinSize.set(10, 10);
   valMinSize.set(10, 10);
   curSize.set(20, 20);

   // get the member description string from SimTag Dictionary
   const char* memberDes = NULL;
   char buffer[256];
   memberDes = tagDictionary.idToString(des);      

   if (!memberDes)
   {
      sprintf(buffer, "Undefined Inspect Tag %d", des);
      memberDes = buffer;
   }

   // create the window for the member description
   desHWnd = ::CreateWindow("STATIC", memberDes, WS_CHILD | WS_CLIPSIBLINGS | SS_RIGHT, 
                            0, 0, desMinSize.x, desMinSize.y, parent, (HMENU)0, hInst, NULL);

   if (! desHWnd)
      return;

   // get the minimum window size for the member description 
   hdc = GetDC(desHWnd);
   if (hdc)
   {
      GetTextExtentPoint32(hdc, memberDes, strlen(memberDes), &size);
      desMinSize.set(size.cx + DBL_BORDER_SPACING, size.cy + DBL_BORDER_SPACING);
   }
   ReleaseDC(desHWnd, hdc);

   // create window for the member value
   valHWnd = ::CreateWindow("EDIT", val, WS_CHILD | WS_CLIPSIBLINGS | ES_LEFT | WS_BORDER | ES_AUTOHSCROLL, 
                            10, 0, valMinSize.x, valMinSize.y, parent, (HMENU)0, hInst, NULL);                                                 

   if (! valHWnd)
   {
      desMinSize.set(10,10);
      DestroyWindow(desHWnd);
      return;   
   }

   // get the minimum window size for the member value
   if (val)
   {
      hdc = GetDC(valHWnd);
      if (hdc)
      {
         GetTextExtentPoint32(hdc, val, strlen(val), &size);
         valMinSize.set(size.cx + DBL_BORDER_SPACING + EDIT_BORDER, size.cy + DBL_BORDER_SPACING);
      }
      ReleaseDC(valHWnd, hdc);
   }

   // cap the minValues.
   if (valMinSize.x > 250) valMinSize.x = 250;
   if (valMinSize.x < 150) valMinSize.x = 150;

   // get bounding size for both min values
   curSize.x = valMinSize.x + desMinSize.x;
   curSize.y = (valMinSize.y > desMinSize.y) ? valMinSize.y : desMinSize.y;
}   

//------------------------------------------------------------------------------
void IString_MemberWin::position(Box2I &bbox, Point2I &recommendedDesSize)
{
   curSize.x = bbox.fMax.x - bbox.fMin.x;
   curSize.y = bbox.fMax.y - bbox.fMin.y;

   SetWindowPos(desHWnd, 0, 
                bbox.fMin.x + BORDER_SPACING, 
                bbox.fMin.y + BORDER_SPACING, 
                recommendedDesSize.x - DBL_BORDER_SPACING, 
                curSize.y - DBL_BORDER_SPACING, 
                SWP_NOZORDER | SWP_SHOWWINDOW);
   SetWindowPos(valHWnd, 0, 
                bbox.fMin.x + recommendedDesSize.x + BORDER_SPACING, 
                bbox.fMin.y + BORDER_SPACING,
                bbox.fMax.x - recommendedDesSize.x - DBL_BORDER_SPACING,
                curSize.y - DBL_BORDER_SPACING,  
                SWP_NOZORDER | SWP_SHOWWINDOW);
}   

//------------------------------------------------------------------------------
Bool IString_MemberWin::getVal(char *buffer, int bufLen)
{
   if (valHWnd)
   {
      int txtLen = GetWindowTextLength(valHWnd);
      if ( bufLen < (txtLen + 1) ) 
      {
         char buffer[256];
         sprintf(buffer, "the string must not exceed %d chars", bufLen - 1);
         displayErrDialog(buffer);
         return false;
      }

      if (txtLen == 0)
         buffer[0] = '\0';
      else
         GetWindowText(valHWnd, buffer, bufLen);
      return true;
   }
   return false;
}   

//------------------------------------------------------------------------------
//                       Int32_MemberWin functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void Int32_MemberWin::create(const HWND parent, HINSTANCE hInst, const SimTag fieldDes, const Int32 val, const Inspect::Display _display)
{
   display = _display;

   char buffer[80];
   if (display == Inspect::Hex)
      sprintf(buffer,"0x%x",val);
   else
      sprintf(buffer,"%d",val);
   buffer[79] = '\0';
   Parent::create(parent, hInst, fieldDes, buffer);
}   

//------------------------------------------------------------------------------
void Int32_MemberWin::position(Box2I &bbox, Point2I &recommendedDesSize)
{
   Parent::position(bbox, recommendedDesSize);
}

//------------------------------------------------------------------------------
Bool Int32_MemberWin::getVal(Int32 &val)
{
   char buffer[Inspect::MAX_STRING_LEN + 1];
   if (Parent::getVal(buffer, Inspect::MAX_STRING_LEN + 1))
   {
      if (display == Inspect::Hex)
      {
         if ( (buffer[0] != '0') || (buffer[1] != 'x') || (!isxdigit(buffer[2])) )
         {
            displayErrDialog("must specify a hex number with a leading 0x");
            return false;
         }
         sscanf(buffer, "0x%x", &val);  
      }
      else
      {
         if ( !isdigit(buffer[0]) && (buffer[0] != '-') )
         {
            displayErrDialog("must specify an integer");
            return false;   
         }
         val = atoi(buffer);
      }
      return true;
   }
   return false;
}  

//------------------------------------------------------------------------------
//                       RealF_MemberWin functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void RealF_MemberWin::create(const HWND parent, HINSTANCE hInst, const SimTag fieldDes, const RealF val)
{
   orig_val = val;
   char buffer[80];
   sprintf(buffer,"%f",val);
   buffer[79] = '\0';
   Parent::create(parent, hInst, fieldDes, buffer);
}   

//------------------------------------------------------------------------------
void RealF_MemberWin::position(Box2I &bbox, Point2I &recommendedDesSize)
{
   Parent::position(bbox, recommendedDesSize);
}

//------------------------------------------------------------------------------
Bool RealF_MemberWin::getVal(RealF &val)
{
   char buffer[Inspect::MAX_STRING_LEN + 1];
   if (Parent::getVal(buffer, Inspect::MAX_STRING_LEN + 1))
   {
      if ( !isdigit(buffer[0]) && (buffer[0] != '-') && (buffer[0] != '.') )
      {
         displayErrDialog("must specify a floating point number");
         return false;   
      }
      val = (RealF)atof( buffer, orig_val );
      return true;
   }
   return false;
}


//------------------------------------------------------------------------------
//                       Bool_MemberWin functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void Bool_MemberWin::create(const HWND parent, HINSTANCE hInst, const SimTag fieldDes, const Bool val)
{
   HDC hdc;
   SIZE size;

   des = fieldDes;

   // initially create small windows at root, we'll position them later
   desMinSize.set(10, 10);
   valMinSize.set(10, 10);
   curSize.set(20, 20);

   // get the member description string from SimTag Dictionary
   const char* memberDes = NULL;
   char buffer[256];
   memberDes = tagDictionary.idToString(des);      

   if (!memberDes)
   {
      sprintf(buffer, "Undefined Inspect Tag %d", des);
      memberDes = buffer;
   }

   // create the window for the member description
   desHWnd = ::CreateWindow("STATIC", memberDes, WS_CHILD | WS_CLIPSIBLINGS | SS_RIGHT, 
                            0, 0, desMinSize.x, desMinSize.y, parent, (HMENU)0, hInst, NULL);

   if (! desHWnd)
      return;

   // get the minimum window size for the member description 
   hdc = GetDC(desHWnd);
   if (hdc)
   {
      GetTextExtentPoint32(hdc, memberDes, strlen(memberDes), &size);
      desMinSize.set(size.cx + DBL_BORDER_SPACING, size.cy + DBL_BORDER_SPACING);
   }
   ReleaseDC(desHWnd, hdc);

   // create window for the member value
   valHWnd = ::CreateWindow("BUTTON", "", WS_CHILD | WS_CLIPSIBLINGS | BS_AUTOCHECKBOX, 
                            10, 0, valMinSize.x, valMinSize.y, parent, (HMENU)0, hInst, NULL);                                                 

   if (! valHWnd)
   {
      desMinSize.set(10,10);
      DestroyWindow(desHWnd);
      return;   
   }

   // set the initial value for the check box
   if (val)
      SendMessage(valHWnd, BM_SETCHECK, 1, 0);
   else
      SendMessage(valHWnd, BM_SETCHECK, 0, 0);

   // get the minimum window size for the member value
   valMinSize.x = desMinSize.y + DBL_BORDER_SPACING;
   valMinSize.y = desMinSize.y + DBL_BORDER_SPACING;

   curSize.x = valMinSize.x + desMinSize.x;
   curSize.y = valMinSize.y;
}   

//------------------------------------------------------------------------------
void Bool_MemberWin::position(Box2I &bbox, Point2I &recommendedDesSize)
{
   curSize.x = bbox.fMax.x - bbox.fMin.x;
   curSize.y = bbox.fMax.y - bbox.fMin.y;

   SetWindowPos(desHWnd, 0, 
                bbox.fMin.x + BORDER_SPACING, 
                bbox.fMin.y + BORDER_SPACING, 
                recommendedDesSize.x - DBL_BORDER_SPACING, 
                curSize.y - DBL_BORDER_SPACING, 
                SWP_NOZORDER | SWP_SHOWWINDOW);
   SetWindowPos(valHWnd, 0, 
                bbox.fMin.x + recommendedDesSize.x + BORDER_SPACING, 
                bbox.fMin.y + BORDER_SPACING,
                curSize.y - DBL_BORDER_SPACING, 
                curSize.y - DBL_BORDER_SPACING,
                SWP_NOZORDER | SWP_SHOWWINDOW);
}

//------------------------------------------------------------------------------
Bool Bool_MemberWin::getVal(Bool &val)
{
   if (valHWnd)
   {
      val = (Bool) SendMessage(valHWnd, BM_GETCHECK, 0, 0);

      return true;
   }
   return false;
}   

//------------------------------------------------------------------------------
//                       Show_Tag_Define_MemberWin functions
//------------------------------------------------------------------------------

void Show_Tag_Define_MemberWin::create(const HWND parent, HINSTANCE hInst, const SimTag fieldDes, const Inspect::IString filter, const Int32 val)
{
   HDC hdc;
   SIZE size;

   des = fieldDes;

   // initially create small windows at root, we'll position them later
   desMinSize.set(10, 10);
   valMinSize.set(10, 10);
   curSize.set(20, 20);

   // get the member description string from SimTag Dictionary
   const char* memberDes = NULL;
   char buffer[256];
   memberDes = tagDictionary.idToString(des);      

   if (!memberDes)
   {
      sprintf(buffer, "Undefined Inspect Tag %d", des);
      memberDes = buffer;
   }

   // create the window for the member description
   desHWnd = ::CreateWindow("STATIC", memberDes, WS_CHILD | WS_CLIPSIBLINGS | SS_RIGHT, 
                            0, 0, desMinSize.x, desMinSize.y, parent, (HMENU)0, hInst, NULL);

   if (! desHWnd)
      return;

   // get the minimum window size for the member description 
   hdc = GetDC(desHWnd);
   if (hdc)
   {
      GetTextExtentPoint32(hdc, memberDes, strlen(memberDes), &size);
      desMinSize.set(size.cx + DBL_BORDER_SPACING, size.cy + DBL_BORDER_SPACING);
   }
   ReleaseDC(desHWnd, hdc);

   // create window for the member value
   valHWnd = ::CreateWindow("COMBOBOX", 0, WS_CHILD | WS_CLIPSIBLINGS | WS_VSCROLL | CBS_SORT | CBS_DROPDOWNLIST, 
                            10, 0, valMinSize.x, valMinSize.y, parent, (HMENU)0, hInst, NULL);                                                 

   if (! valHWnd)
   {
      desMinSize.set(10,10);
      DestroyWindow(desHWnd);
      return;   
   }

   valMinSize.set(80 + DBL_BORDER_SPACING + EDIT_BORDER, desMinSize.y + DBL_BORDER_SPACING);
   // fill up the list box with all available defines that match
   // the tagDefSearchStr and get the minimum window size
   hdc = GetDC(valHWnd);
       
   if (filter)
   {
      Vector<int> v;
      char *searchStrBeg = filter;
      char *searchStrEnd;
      bool done = false;
      while (! done)
      {
         // filter can have multiple search strings separated by commas
         searchStrEnd = searchStrBeg;
         while (*searchStrEnd != ',')
         {
            if (*searchStrEnd == '\0')
            {
               done = true;
               break;
            }
            searchStrEnd++;
         }
         *searchStrEnd = '\0';

         v.clear();
         tagDictionary.findDefines(v, searchStrBeg);
         for (Vector<int>::iterator iter = v.begin(); iter != v.end(); iter++)
         {
            const char *fontName = tagDictionary.idToDefine(*iter);
            SendMessage(valHWnd, CB_ADDSTRING, 0, (LPARAM)fontName);         

            // get minimum size
            if (hdc)
            {
               GetTextExtentPoint32(hdc, fontName, strlen(fontName), &size);
               size.cx += DBL_BORDER_SPACING + 30; // 30 for the button and scroll bar of combobox
               size.cy += DBL_BORDER_SPACING + 8;  // 8 for the border around the combobox
               if (size.cx > valMinSize.x)
                  valMinSize.x = size.cx;
               if (size.cy > valMinSize.y)
                  valMinSize.y = size.cy;
            }
         }
         searchStrBeg = searchStrEnd + 1;
      }
   }

   // add None as a choice
   SendMessage(valHWnd, CB_ADDSTRING, 0, (LPARAM)STR_NONE);         

   // get minimum size
   if (hdc)
   {
      GetTextExtentPoint32(hdc, STR_NONE, strlen(STR_NONE), &size);
      size.cx += DBL_BORDER_SPACING + 30; // 30 for the button and scroll bar of combobox
      size.cy += DBL_BORDER_SPACING + 8;  // 8 for the border around the combobox
      if (size.cx > valMinSize.x)
         valMinSize.x = size.cx;
      if (size.cy > valMinSize.y)
         valMinSize.y = size.cy;
   }

   ReleaseDC(valHWnd, hdc);   

   // highlight the initial value
   const char *fontName = tagDictionary.idToDefine(val);
   SendMessage(valHWnd, CB_SELECTSTRING, -1, (LPARAM) fontName); 

   // cap minimum valMinSize
   if (valMinSize.x > 250) valMinSize.x = 250;

   // get bounding size for both min values
   curSize.x = valMinSize.x + desMinSize.x;
   curSize.y = (valMinSize.y > desMinSize.y) ? valMinSize.y : desMinSize.y;
}   

//------------------------------------------------------------------------------
void Show_Tag_Define_MemberWin::position(Box2I &bbox, Point2I &recommendedDesSize)
{
   curSize.x = bbox.fMax.x - bbox.fMin.x;
   curSize.y = bbox.fMax.y - bbox.fMin.y;

   SetWindowPos(desHWnd, 0, 
                bbox.fMin.x + BORDER_SPACING, 
                bbox.fMin.y + BORDER_SPACING, 
                recommendedDesSize.x - DBL_BORDER_SPACING, 
                curSize.y - DBL_BORDER_SPACING, 
                SWP_NOZORDER | SWP_SHOWWINDOW);
   SetWindowPos(valHWnd, 0, 
                bbox.fMin.x + recommendedDesSize.x + BORDER_SPACING, 
                bbox.fMin.y + BORDER_SPACING,
                bbox.fMax.x - recommendedDesSize.x - DBL_BORDER_SPACING,
                curSize.y * 10, // room for 10 items on pop up  
                SWP_NOZORDER | SWP_SHOWWINDOW);
}   

//------------------------------------------------------------------------------
Bool Show_Tag_Define_MemberWin::getVal(Int32 &val)
{
   if (valHWnd)
   {
      // get the name of the currently selected font
      int index = SendMessage(valHWnd, CB_GETCURSEL, 0, 0);
      if (index == CB_ERR)
      {
         // STR_NONE selected
         val = 0;
         return true; 
      }
      int txtLen = SendMessage(valHWnd, CB_GETLBTEXTLEN, index, 0);
      char *buffer = new char[txtLen + 1];
      buffer[0] = '\0';
      SendMessage(valHWnd, CB_GETLBTEXT, index, (LPARAM)buffer);
      
      // get font define id for the currently selected font
      if ((stricmp(STR_NONE, buffer) == 0) )
         val = 0;
      else
         val = tagDictionary.defineToId(stringTable.insert(buffer));

      return true;
   }
   return false;
}


//------------------------------------------------------------------------------
//                       Show_Tag_IString_MemberWin functions
//------------------------------------------------------------------------------

void Show_Tag_IString_MemberWin::create(const HWND parent, HINSTANCE hInst, const SimTag fieldDes, const Inspect::IString filter, const Int32 val)
{
   HDC hdc;
   SIZE size;

   des = fieldDes;

   // initially create small windows at root, we'll position them later
   desMinSize.set(10, 10);
   valMinSize.set(10, 10);
   curSize.set(20, 20);

   // get the member description string from SimTag Dictionary
   const char* memberDes = NULL;
   char buffer[256];
   memberDes = tagDictionary.idToString(des);      

   if (!memberDes)
   {
      sprintf(buffer, "Undefined Inspect Tag %d", des);
      memberDes = buffer;
   }

   // create the window for the member description
   desHWnd = ::CreateWindow("STATIC", memberDes, WS_CHILD | WS_CLIPSIBLINGS | SS_RIGHT, 
                            0, 0, desMinSize.x, desMinSize.y, parent, (HMENU)0, hInst, NULL);

   if (! desHWnd)
      return;

   // get the minimum window size for the member description 
   hdc = GetDC(desHWnd);
   if (hdc)
   {
      GetTextExtentPoint32(hdc, memberDes, strlen(memberDes), &size);
      desMinSize.set(size.cx + DBL_BORDER_SPACING, size.cy + DBL_BORDER_SPACING);
   }
   ReleaseDC(desHWnd, hdc);

   // create window for the member value
   valHWnd = ::CreateWindow("COMBOBOX", 0, WS_CHILD | WS_CLIPSIBLINGS | WS_VSCROLL | CBS_SORT | CBS_DROPDOWNLIST, 
                            10, 0, valMinSize.x, valMinSize.y, parent, (HMENU)0, hInst, NULL);                                                 

   if (! valHWnd)
   {
      desMinSize.set(10,10);
      DestroyWindow(desHWnd);
      return;   
   }

   valMinSize.set(80 + DBL_BORDER_SPACING + EDIT_BORDER, desMinSize.y + DBL_BORDER_SPACING);
   // fill up the list box with all available tag strings and get the minimum window size
   hdc = GetDC(valHWnd);
       
   
   if (filter)
   {
      Vector<int> v;
      char *searchStrBeg = filter;
      char *searchStrEnd;
      bool done = false;
      while (! done)
      {
         // filter can have multiple search strings separated by commas
         searchStrEnd = searchStrBeg;
         while (*searchStrEnd != ',')
         {
            if (*searchStrEnd == '\0')
            {
               done = true;
               break;
            }
            searchStrEnd++;
         }
         *searchStrEnd = '\0';

         v.clear();
         tagDictionary.findDefines(v, searchStrBeg);
         for (Vector<int>::iterator iter = v.begin(); iter != v.end(); iter++)
         {
            const char *tagStr = tagDictionary.idToString(*iter);
            SendMessage(valHWnd, CB_ADDSTRING, 0, (LPARAM)tagStr);         

            // get minimum size
            if (hdc)
            {
               GetTextExtentPoint32(hdc, tagStr, strlen(tagStr), &size);
               size.cx += DBL_BORDER_SPACING + 30; // 30 for the button and scroll bar of combobox
               size.cy += DBL_BORDER_SPACING + 10; // 10 for the border around the combobox
               if (size.cx > valMinSize.x)
                  valMinSize.x = size.cx;
               if (size.cy > valMinSize.y)
                  valMinSize.y = size.cy;
            }
         }
         searchStrBeg = searchStrEnd + 1;
      }
   }
   
   // add none as a choice
   SendMessage(valHWnd, CB_ADDSTRING, 0, (LPARAM)STR_NONE);         

   // get minimum size
   if (hdc)
   {
      GetTextExtentPoint32(hdc, STR_NONE, strlen(STR_NONE), &size);
      size.cx += DBL_BORDER_SPACING + 30; // 30 for the button and scroll bar of combobox
      size.cy += DBL_BORDER_SPACING + 10; // 10 for the border around the combobox
      if (size.cx > valMinSize.x)
         valMinSize.x = size.cx;
      if (size.cy > valMinSize.y)
         valMinSize.y = size.cy;
   }
   
   ReleaseDC(valHWnd, hdc);   

   // highlight the initial value
   const char *tagStr = tagDictionary.idToString(val);
   SendMessage(valHWnd, CB_SELECTSTRING, -1, (LPARAM) tagStr); 

   // cap minimum valMinSize
   if (valMinSize.x > 250) valMinSize.x = 250;

   // get bounding size for both min values
   curSize.x = valMinSize.x + desMinSize.x;
   curSize.y = (valMinSize.y > desMinSize.y) ? valMinSize.y : desMinSize.y;
}   

//------------------------------------------------------------------------------
void Show_Tag_IString_MemberWin::position(Box2I &bbox, Point2I &recommendedDesSize)
{
   curSize.x = bbox.fMax.x - bbox.fMin.x;
   curSize.y = bbox.fMax.y - bbox.fMin.y;

   SetWindowPos(desHWnd, 0, 
                bbox.fMin.x + BORDER_SPACING, 
                bbox.fMin.y + BORDER_SPACING, 
                recommendedDesSize.x - DBL_BORDER_SPACING, 
                curSize.y - DBL_BORDER_SPACING, 
                SWP_NOZORDER | SWP_SHOWWINDOW);
   SetWindowPos(valHWnd, 0, 
                bbox.fMin.x + recommendedDesSize.x + BORDER_SPACING, 
                bbox.fMin.y + BORDER_SPACING,
                bbox.fMax.x - recommendedDesSize.x - DBL_BORDER_SPACING,
                curSize.y * 10, // room for 10 items on pop up  
                SWP_NOZORDER | SWP_SHOWWINDOW);
}   

//------------------------------------------------------------------------------
Bool Show_Tag_IString_MemberWin::getVal(Int32 &val)
{
   if (valHWnd)
   {
      // get the name of the currently selected font
      int index = SendMessage(valHWnd, CB_GETCURSEL, 0, 0);
      if (index == CB_ERR)
      {
         val = 0;
         return true; 
      }
      int txtLen = SendMessage(valHWnd, CB_GETLBTEXTLEN, index, 0);
      char *buffer = new char[txtLen + 1];
      buffer[0] = '\0';
      SendMessage(valHWnd, CB_GETLBTEXT, index, (LPARAM)buffer);
      
      // get str define id for the currently selected font
      if ((stricmp(buffer, STR_NONE) == 0) )
      {
         val = 0;
         return true;
      }

      Vector<int> v;
      tagDictionary.findStrings(v, buffer);
      for (Vector<int>::iterator iter = v.begin(); iter != v.end(); iter++)
      {
         if (strcmp(buffer, tagDictionary.idToString(*iter) ) == 0)
         {
            val = *iter;
            return true;
         }
      }
   }
   return false;
}

//------------------------------------------------------------------------------
//                       Show_TagRange_Define_MemberWin functions
//------------------------------------------------------------------------------

void Show_TagRange_Define_MemberWin::create(const HWND parent, HINSTANCE hInst, const SimTag fieldDes, const Int32 minTag, const Int32 maxTag, const Int32 val)
{
   HDC hdc;
   SIZE size;

   des = fieldDes;

   AssertFatal(minTag <= maxTag, "Min tag must be less or equal to maxTag");

   // initially create small windows at root, we'll position them later
   desMinSize.set(10, 10);
   valMinSize.set(10, 10);
   curSize.set(20, 20);

   // get the member description string from SimTag Dictionary
   const char* memberDes = NULL;
   char buffer[256];
   memberDes = tagDictionary.idToString(des);      

   if (!memberDes)
   {
      sprintf(buffer, "Undefined Inspect Tag %d", des);
      memberDes = buffer;
   }

   // create the window for the member description
   desHWnd = ::CreateWindow("STATIC", memberDes, WS_CHILD | WS_CLIPSIBLINGS | SS_RIGHT, 
                            0, 0, desMinSize.x, desMinSize.y, parent, (HMENU)0, hInst, NULL);

   if (! desHWnd)
      return;

   // get the minimum window size for the member description 
   hdc = GetDC(desHWnd);
   if (hdc)
   {
      GetTextExtentPoint32(hdc, memberDes, strlen(memberDes), &size);
      desMinSize.set(size.cx + DBL_BORDER_SPACING, size.cy + DBL_BORDER_SPACING);
   }
   ReleaseDC(desHWnd, hdc);

   // create window for the member value
   valHWnd = ::CreateWindow("COMBOBOX", 0, WS_CHILD | WS_CLIPSIBLINGS | WS_VSCROLL | CBS_SORT | CBS_DROPDOWNLIST, 
                            10, 0, valMinSize.x, valMinSize.y, parent, (HMENU)0, hInst, NULL);                                                 

   if (! valHWnd)
   {
      desMinSize.set(10,10);
      DestroyWindow(desHWnd);
      return;   
   }

   valMinSize.set(80 + DBL_BORDER_SPACING + EDIT_BORDER, desMinSize.y + DBL_BORDER_SPACING);
   // fill up the list box with all available defines that match
   // the tagDefSearchStr and get the minimum window size
   hdc = GetDC(valHWnd);
       
   Vector<int> v;

   v.clear();
   tagDictionary.findIDs(v, minTag, maxTag);
   for (Vector<int>::iterator iter = v.begin(); iter != v.end(); iter++)
   {
      const char *fontName = tagDictionary.idToDefine(*iter);
      SendMessage(valHWnd, CB_ADDSTRING, 0, (LPARAM)fontName);         

      // get minimum size
      if (hdc)
      {
         GetTextExtentPoint32(hdc, fontName, strlen(fontName), &size);
         size.cx += DBL_BORDER_SPACING + 30; // 30 for the button and scroll bar of combobox
         size.cy += DBL_BORDER_SPACING + 8;  // 8 for the border around the combobox
         if (size.cx > valMinSize.x)
            valMinSize.x = size.cx;
         if (size.cy > valMinSize.y)
            valMinSize.y = size.cy;
      }
   }
   
   // add None as a choice
   SendMessage(valHWnd, CB_ADDSTRING, 0, (LPARAM)STR_NONE);         

   // get minimum size
   if (hdc)
   {
      GetTextExtentPoint32(hdc, STR_NONE, strlen(STR_NONE), &size);
      size.cx += DBL_BORDER_SPACING + 30; // 30 for the button and scroll bar of combobox
      size.cy += DBL_BORDER_SPACING + 8;  // 8 for the border around the combobox
      if (size.cx > valMinSize.x)
         valMinSize.x = size.cx;
      if (size.cy > valMinSize.y)
         valMinSize.y = size.cy;
   }

   ReleaseDC(valHWnd, hdc);   

   // highlight the initial value
   const char *fontName = tagDictionary.idToDefine(val);
   SendMessage(valHWnd, CB_SELECTSTRING, -1, (LPARAM) fontName); 

   // cap minimum valMinSize
   if (valMinSize.x > 250) valMinSize.x = 250;

   // get bounding size for both min values
   curSize.x = valMinSize.x + desMinSize.x;
   curSize.y = (valMinSize.y > desMinSize.y) ? valMinSize.y : desMinSize.y;
}

//------------------------------------------------------------------------------
//                       Show_TagRange_String_MemberWin functions
//------------------------------------------------------------------------------

void Show_TagRange_IString_MemberWin::create(const HWND parent, HINSTANCE hInst, const SimTag fieldDes, const Int32 minTag, const Int32 maxTag, const Int32 val)
{
   HDC hdc;
   SIZE size;

   des = fieldDes;

   // initially create small windows at root, we'll position them later
   desMinSize.set(10, 10);
   valMinSize.set(10, 10);
   curSize.set(20, 20);

   // get the member description string from SimTag Dictionary
   const char* memberDes = NULL;
   char buffer[256];
   memberDes = tagDictionary.idToString(des);      

   if (!memberDes)
   {
      sprintf(buffer, "Undefined Inspect Tag %d", des);
      memberDes = buffer;
   }

   // create the window for the member description
   desHWnd = ::CreateWindow("STATIC", memberDes, WS_CHILD | WS_CLIPSIBLINGS | SS_RIGHT, 
                            0, 0, desMinSize.x, desMinSize.y, parent, (HMENU)0, hInst, NULL);

   if (! desHWnd)
      return;

   // get the minimum window size for the member description 
   hdc = GetDC(desHWnd);
   if (hdc)
   {
      GetTextExtentPoint32(hdc, memberDes, strlen(memberDes), &size);
      desMinSize.set(size.cx + DBL_BORDER_SPACING, size.cy + DBL_BORDER_SPACING);
   }
   ReleaseDC(desHWnd, hdc);

   // create window for the member value
   valHWnd = ::CreateWindow("COMBOBOX", 0, WS_CHILD | WS_CLIPSIBLINGS | WS_VSCROLL | CBS_SORT | CBS_DROPDOWNLIST, 
                            10, 0, valMinSize.x, valMinSize.y, parent, (HMENU)0, hInst, NULL);                                                 

   if (! valHWnd)
   {
      desMinSize.set(10,10);
      DestroyWindow(desHWnd);
      return;   
   }

   valMinSize.set(80 + DBL_BORDER_SPACING + EDIT_BORDER, desMinSize.y + DBL_BORDER_SPACING);
   // fill up the list box with all available tag strings and get the minimum window size
   hdc = GetDC(valHWnd);
       
   
   Vector<int> v;
      
   v.clear();
   tagDictionary.findIDs(v, minTag, maxTag );
   for (Vector<int>::iterator iter = v.begin(); iter != v.end(); iter++)
   {
      const char *tagStr = tagDictionary.idToString(*iter);
      SendMessage(valHWnd, CB_ADDSTRING, 0, (LPARAM)tagStr);         

      // get minimum size
      if (hdc)
      {
         GetTextExtentPoint32(hdc, tagStr, strlen(tagStr), &size);
         size.cx += DBL_BORDER_SPACING + 30; // 30 for the button and scroll bar of combobox
         size.cy += DBL_BORDER_SPACING + 10; // 10 for the border around the combobox
         if (size.cx > valMinSize.x)
            valMinSize.x = size.cx;
         if (size.cy > valMinSize.y)
            valMinSize.y = size.cy;
      }
   }
   
   // add none as a choice
   SendMessage(valHWnd, CB_ADDSTRING, 0, (LPARAM)STR_NONE);         

   // get minimum size
   if (hdc)
   {
      GetTextExtentPoint32(hdc, STR_NONE, strlen(STR_NONE), &size);
      size.cx += DBL_BORDER_SPACING + 30; // 30 for the button and scroll bar of combobox
      size.cy += DBL_BORDER_SPACING + 10; // 10 for the border around the combobox
      if (size.cx > valMinSize.x)
         valMinSize.x = size.cx;
      if (size.cy > valMinSize.y)
         valMinSize.y = size.cy;
   }
   
   ReleaseDC(valHWnd, hdc);   

   // highlight the initial value
   const char *tagStr = tagDictionary.idToString(val);
   SendMessage(valHWnd, CB_SELECTSTRING, -1, (LPARAM) tagStr); 

   // cap minimum valMinSize
   if (valMinSize.x > 250) valMinSize.x = 250;

   // get bounding size for both min values
   curSize.x = valMinSize.x + desMinSize.x;
   curSize.y = (valMinSize.y > desMinSize.y) ? valMinSize.y : desMinSize.y;
}   

//------------------------------------------------------------------------------
//                       SimTagValPair_MemberWin functions
//------------------------------------------------------------------------------

void SimTagValPair_MemberWin::create(const HWND parent, HINSTANCE hInst, const SimTag fieldDes, const Int32 val, const Int32 _stvLen, Inspect::SimTagValPair *_stvList)
{
   HDC hdc;
   SIZE size;

   stvLen = _stvLen;
   stvList = _stvList;
   des = fieldDes;

   // initially create small windows at root, we'll position them later
   desMinSize.set(10, 10);
   valMinSize.set(10, 10);
   curSize.set(20, 20);

   // get the member description string from SimTag Dictionary
   const char* memberDes = NULL;
   char buffer[256];
   memberDes = tagDictionary.idToString(des);      

   if (!memberDes)
   {
      sprintf(buffer, "Undefined Inspect Tag %d", des);
      memberDes = buffer;
   }

   // create the window for the member description
   desHWnd = ::CreateWindow("STATIC", memberDes, WS_CHILD | WS_CLIPSIBLINGS | SS_RIGHT, 
                            0, 0, desMinSize.x, desMinSize.y, parent, (HMENU)0, hInst, NULL);

   if (! desHWnd)
      return;

   // get the minimum window size for the member description 
   hdc = GetDC(desHWnd);
   if (hdc)
   {
      GetTextExtentPoint32(hdc, memberDes, strlen(memberDes), &size);
      desMinSize.set(size.cx + DBL_BORDER_SPACING, size.cy + DBL_BORDER_SPACING);
   }
   ReleaseDC(desHWnd, hdc);

   // create window for the member value
   valHWnd = ::CreateWindow("COMBOBOX", 0, WS_CHILD | WS_CLIPSIBLINGS | WS_VSCROLL | WS_VISIBLE | CBS_SORT | CBS_DROPDOWNLIST, 
                            10, 0, valMinSize.x, valMinSize.y, parent, (HMENU)0, hInst, NULL);                                                 

   if (! valHWnd)
   {
      desMinSize.set(10,10);
      DestroyWindow(desHWnd);
      return;   
   }

   valMinSize.set(80 + DBL_BORDER_SPACING + EDIT_BORDER, desMinSize.y + DBL_BORDER_SPACING);
   // fill up the list box with all available tag strings and get the minimum window size
   hdc = GetDC(valHWnd);
   
   if (stvLen)
   {
      for (int k = 0; k < stvLen; k++)
      {
         const char *tagStr = tagDictionary.idToString(stvList[k].tag);
         AssertFatal( tagStr, "invalid tag" );
         SendMessage(valHWnd, CB_ADDSTRING, 0, (LPARAM)tagStr);         

         // get minimum size
         if (hdc)
         {
            GetTextExtentPoint32(hdc, tagStr, strlen(tagStr), &size);
            size.cx += DBL_BORDER_SPACING + 30; // 30 for the button and scroll bar of combobox
            size.cy += DBL_BORDER_SPACING + 10; // 10 for the border around the combobox
            if (size.cx > valMinSize.x)
               valMinSize.x = size.cx;
            if (size.cy > valMinSize.y)
               valMinSize.y = size.cy;
         }
      }
   }
   
   // get minimum size
   if (hdc)
   {
      GetTextExtentPoint32(hdc, STR_NONE, strlen(STR_NONE), &size);
      size.cx += DBL_BORDER_SPACING + 30; // 30 for the button and scroll bar of combobox
      size.cy += DBL_BORDER_SPACING + 10; // 10 for the border around the combobox
      if (size.cx > valMinSize.x)
         valMinSize.x = size.cx;
      if (size.cy > valMinSize.y)
         valMinSize.y = size.cy;
   }
   
   ReleaseDC(valHWnd, hdc);   

   // highlight the initial value
   bool found = false;
   int j;
   for ( j = 0; j < stvLen; j++)
   {
      if (stvList[j].val == val)
      {
         found = true;
         break;
      }
   }
   
   if(found)
   {
      const char *tagStr = tagDictionary.idToString(stvList[j].tag);
      AssertFatal( tagStr, "invalid tag" );
      SendMessage(valHWnd, CB_SELECTSTRING, -1, (LPARAM) tagStr); 
   }
   
   // cap minimum valMinSize
   if (valMinSize.x > 250) valMinSize.x = 250;

   // get bounding size for both min values
   curSize.x = valMinSize.x + desMinSize.x;
   curSize.y = (valMinSize.y > desMinSize.y) ? valMinSize.y : desMinSize.y;
}   

//------------------------------------------------------------------------------
void SimTagValPair_MemberWin::position(Box2I &bbox, Point2I &recommendedDesSize)
{
   curSize.x = bbox.fMax.x - bbox.fMin.x;
   curSize.y = bbox.fMax.y - bbox.fMin.y;

   SetWindowPos(desHWnd, 0, 
                bbox.fMin.x + BORDER_SPACING, 
                bbox.fMin.y + BORDER_SPACING, 
                recommendedDesSize.x - DBL_BORDER_SPACING, 
                curSize.y - DBL_BORDER_SPACING, 
                SWP_NOZORDER | SWP_SHOWWINDOW);
   SetWindowPos(valHWnd, 0, 
                bbox.fMin.x + recommendedDesSize.x + BORDER_SPACING, 
                bbox.fMin.y + BORDER_SPACING,
                bbox.fMax.x - recommendedDesSize.x - DBL_BORDER_SPACING,
                curSize.y * 10, // room for 10 items on pop up  
                SWP_NOZORDER | SWP_SHOWWINDOW);
}   

//------------------------------------------------------------------------------
Bool SimTagValPair_MemberWin::getVal(Int32 &val, Int32 &_stvLen)
{
   if (valHWnd)
   {
      // get the name of the currently selected font
      int index = SendMessage(valHWnd, CB_GETCURSEL, 0, 0);
      if (index == CB_ERR)
      {
         val = 0;
         return true; 
      }
      int txtLen = SendMessage(valHWnd, CB_GETLBTEXTLEN, index, 0);
      char *buffer = new char[txtLen + 1];
      buffer[0] = '\0';
      SendMessage(valHWnd, CB_GETLBTEXT, index, (LPARAM)buffer);
      
      for (int i = 0; i < stvLen; i++)
      {
         AssertFatal( tagDictionary.idToString(stvList[i].tag), "invalid tag" );
         if (strcmp(buffer, tagDictionary.idToString(stvList[i].tag) ) == 0)
         {
            _stvLen = stvLen;
            val = stvList[i].val;
            return true;
         }
      }
      displayErrDialog("must specify a value");
   }
   return false;
}

//------------------------------------------------------------------------------
//                       IStringValPair_MemberWin functions
//------------------------------------------------------------------------------

void IStringValPair_MemberWin::create(const HWND parent, HINSTANCE hInst, const SimTag fieldDes, const Int32 val, const Int32 _svLen, Inspect::IStringValPair *_svList)
{
   HDC hdc;
   SIZE size;

   svLen  = _svLen;
   svList = _svList;
   des = fieldDes;

   // initially create small windows at root, we'll position them later
   desMinSize.set(10, 10);
   valMinSize.set(10, 10);
   curSize.set(20, 20);

   // get the member description string from SimTag Dictionary
   const char* memberDes = NULL;
   char buffer[256];
   memberDes = tagDictionary.idToString(des);      

   if (!memberDes)
   {
      sprintf(buffer, "Undefined Inspect Tag %d", des);
      memberDes = buffer;
   }

   // create the window for the member description
   desHWnd = ::CreateWindow("STATIC", memberDes, WS_CHILD | WS_CLIPSIBLINGS | SS_RIGHT, 
                            0, 0, desMinSize.x, desMinSize.y, parent, (HMENU)0, hInst, NULL);

   if (! desHWnd)
      return;

   // get the minimum window size for the member description 
   hdc = GetDC(desHWnd);
   if (hdc)
   {
      GetTextExtentPoint32(hdc, memberDes, strlen(memberDes), &size);
      desMinSize.set(size.cx + DBL_BORDER_SPACING, size.cy + DBL_BORDER_SPACING);
   }
   ReleaseDC(desHWnd, hdc);

   // create window for the member value
   valHWnd = ::CreateWindow("COMBOBOX", 0, WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE | CBS_DROPDOWNLIST, 
                            10, 0, valMinSize.x, valMinSize.y, parent, (HMENU)0, hInst, NULL);                                                 

   if (! valHWnd)
   {
      desMinSize.set(10,10);
      DestroyWindow(desHWnd);
      return;   
   }

   valMinSize.set(80 + DBL_BORDER_SPACING + EDIT_BORDER, desMinSize.y + DBL_BORDER_SPACING);
   // fill up the list box with all available tag strings and get the minimum window size
   hdc = GetDC(valHWnd);
   
   if (svLen)
   {
      for (int k = 0; k < svLen; k++)
      {
         const char *tagStr = svList[k].string;
         SendMessage(valHWnd, CB_ADDSTRING, 0, (LPARAM)tagStr);         

         // get minimum size
         if (hdc)
         {
            GetTextExtentPoint32(hdc, tagStr, strlen(tagStr), &size);
            size.cx += DBL_BORDER_SPACING + 30; // 30 for the button and scroll bar of combobox
            size.cy += DBL_BORDER_SPACING + 10; // 10 for the border around the combobox
            if (size.cx > valMinSize.x)
               valMinSize.x = size.cx;
            if (size.cy > valMinSize.y)
               valMinSize.y = size.cy;
         }
      }
   }
   
   // get minimum size
   if (hdc)
   {
      GetTextExtentPoint32(hdc, STR_NONE, strlen(STR_NONE), &size);
      size.cx += DBL_BORDER_SPACING + 30; // 30 for the button and scroll bar of combobox
      size.cy += DBL_BORDER_SPACING + 10; // 10 for the border around the combobox
      if (size.cx > valMinSize.x)
         valMinSize.x = size.cx;
      if (size.cy > valMinSize.y)
         valMinSize.y = size.cy;
   }
   
   ReleaseDC(valHWnd, hdc);   

   // highlight the initial value
   bool found = false;
   int j;
   for ( j = 0; j < svLen; j++)
   {
      if (svList[j].val == val)
      {
         found = true;
         break;
      }
   }
   
   if(found)
   {
      const char *tagStr = svList[j].string;
      SendMessage(valHWnd, CB_SELECTSTRING, -1, (LPARAM) tagStr);
   } 

   // cap minimum valMinSize
   if (valMinSize.x > 250) valMinSize.x = 250;

   // get bounding size for both min values
   curSize.x = valMinSize.x + desMinSize.x;
   curSize.y = (valMinSize.y > desMinSize.y) ? valMinSize.y : desMinSize.y;
}   

//------------------------------------------------------------------------------
void IStringValPair_MemberWin::position(Box2I &bbox, Point2I &recommendedDesSize)
{
   curSize.x = bbox.fMax.x - bbox.fMin.x;
   curSize.y = bbox.fMax.y - bbox.fMin.y;

   SetWindowPos(desHWnd, 0, 
                bbox.fMin.x + BORDER_SPACING, 
                bbox.fMin.y + BORDER_SPACING, 
                recommendedDesSize.x - DBL_BORDER_SPACING, 
                curSize.y - DBL_BORDER_SPACING, 
                SWP_NOZORDER | SWP_SHOWWINDOW);
   SetWindowPos(valHWnd, 0, 
                bbox.fMin.x + recommendedDesSize.x + BORDER_SPACING, 
                bbox.fMin.y + BORDER_SPACING,
                bbox.fMax.x - recommendedDesSize.x - DBL_BORDER_SPACING,
                curSize.y * 10, // room for 10 items on pop up  
                SWP_NOZORDER | SWP_SHOWWINDOW);
}   

//------------------------------------------------------------------------------
Bool IStringValPair_MemberWin::getVal(Int32 &val, Int32 &_svLen)
{
   if (valHWnd)
   {
      // get the name of the currently selected font
      int index = SendMessage(valHWnd, CB_GETCURSEL, 0, 0);
      if (index == CB_ERR)
      {
         val = 0;
         return true; 
      }
      int txtLen = SendMessage(valHWnd, CB_GETLBTEXTLEN, index, 0);
      char *buffer = new char[txtLen + 1];
      buffer[0] = '\0';
      SendMessage(valHWnd, CB_GETLBTEXT, index, (LPARAM)buffer);
      
      for (int i = 0; i < svLen; i++)
      {
         if (strcmp(buffer, svList[i].string) == 0)
         {
            _svLen = svLen;
            val = svList[i].val;
            return true;
         }
      }
      displayErrDialog("must specify a value");
   }
   return false;
}

//------------------------------------------------------------------------------
//                       Point2I_MemberWin functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void Point2I_MemberWin::create(const HWND parent, HINSTANCE hInst, const SimTag fieldDes, const Point2I val)
{
   char buffer[160];
   sprintf(buffer,"%d, %d", val.x, val.y);
   Parent::create(parent, hInst, fieldDes, buffer);
}   

//------------------------------------------------------------------------------
void Point2I_MemberWin::position(Box2I &bbox, Point2I &recommendedDesSize)
{
   Parent::position(bbox, recommendedDesSize);
}

//------------------------------------------------------------------------------
Bool Point2I_MemberWin::getVal(Point2I &val)
{
   char buffer[160];
   if (Parent::getVal(buffer, 160))
   {
      if ( !isdigit(buffer[0]) && (buffer[0] != '-') )
      {
         displayErrDialog("must specify an integer");
         return false;   
      }
      if (sscanf(buffer, "%d, %d", &val.x, &val.y) != 2)
      {
         displayErrDialog("for Point2I type, must include two integer values separated by commas");
         return false;
      }
      return true;
   }
   return false;
}

//------------------------------------------------------------------------------
//                       Point3I_MemberWin functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void Point3I_MemberWin::create(const HWND parent, HINSTANCE hInst, const SimTag fieldDes, const Point3I val)
{
   char buffer[240];
   sprintf(buffer,"%d, %d, %d", val.x, val.y, val.z);
   Parent::create(parent, hInst, fieldDes, buffer);
}   

//------------------------------------------------------------------------------
void Point3I_MemberWin::position(Box2I &bbox, Point2I &recommendedDesSize)
{
   Parent::position(bbox, recommendedDesSize);
}

//------------------------------------------------------------------------------
Bool Point3I_MemberWin::getVal(Point3I &val)
{
   char buffer[240];
   if (Parent::getVal(buffer, 240))
   {
      if ( !isdigit(buffer[0]) && (buffer[0] != '-') )
      {
         displayErrDialog("must specify an integer");
         return false;   
      }
      if (sscanf(buffer, "%d, %d, %d", &val.x, &val.y, &val.z) != 3)
      {
         displayErrDialog("for Point3I type, must include three integer values separated by commas");
         return false;
      }
      return true;
   }
   return false;
}

//------------------------------------------------------------------------------
//                       Point2F_MemberWin functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void Point2F_MemberWin::create(const HWND parent, HINSTANCE hInst, const SimTag fieldDes, const Point2F val)
{
   orig_val = val;
   char buffer[160];
   sprintf(buffer,"%f, %f", val.x, val.y);
   Parent::create(parent, hInst, fieldDes, buffer);
}   

//------------------------------------------------------------------------------
void Point2F_MemberWin::position(Box2I &bbox, Point2I &recommendedDesSize)
{
   Parent::position(bbox, recommendedDesSize);
}

//------------------------------------------------------------------------------
Bool Point2F_MemberWin::getVal(Point2F &val)
{
   char buffer[160];
   if (Parent::getVal(buffer, 160))
   {
      if ( !isdigit(buffer[0]) && (buffer[0] != '-') )
      {
         displayErrDialog("must specify an integer");
         return false;   
      }
      if (sscanf(buffer, "%f, %f", &val.x, &val.y) != 2)
      {
         displayErrDialog("for Point2F type, must include two real numbers separated by commas");
         return false;
      }
      
      // check the float values
      char bufA[64];
      char bufB[64];

      // doh!      
      if( !getFloatString( buffer, 0, bufA, sizeof( bufA ) ) ||
         !getFloatString( buffer, 1, bufB, sizeof( bufB ) ) )
         return( false );
      
      val.x = atof( bufA, orig_val.x );
      val.y = atof( bufB, orig_val.y );
      
      return true;
   }
   return false;
}

//------------------------------------------------------------------------------
//                       Point3F_MemberWin functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void Point3F_MemberWin::create(const HWND parent, HINSTANCE hInst, const SimTag fieldDes, const Point3F val)
{
   orig_val = val;
   char buffer[240];
   sprintf(buffer,"%f, %f, %f", val.x, val.y, val.z);
   Parent::create(parent, hInst, fieldDes, buffer);
}   

//------------------------------------------------------------------------------
void Point3F_MemberWin::position(Box2I &bbox, Point2I &recommendedDesSize)
{
   Parent::position(bbox, recommendedDesSize);
}

//------------------------------------------------------------------------------
Bool Point3F_MemberWin::getVal(Point3F &val)
{
   char buffer[240];
   if (Parent::getVal(buffer, 240))
   {
      if ( !isdigit(buffer[0]) && (buffer[0] != '-') )
      {
         displayErrDialog("must specify an integer");
         return false;   
      }
      if (sscanf(buffer, "%f, %f, %f", &val.x, &val.y, &val.z) != 3)
      {
         displayErrDialog("for Point3F type, must include three real numbers separated by commas");
         return false;
      }
   
      // check the float values
      char bufA[64];
      char bufB[64];
      char bufC[64];

      // doh!      
      if( !getFloatString( buffer, 0, bufA, sizeof( bufA ) ) ||
         !getFloatString( buffer, 1, bufB, sizeof( bufB ) ) ||
         !getFloatString( buffer, 2, bufC, sizeof( bufC ) ) )
         return( false );
      
      val.x = atof( bufA, orig_val.x );
      val.y = atof( bufB, orig_val.y );
      val.z = atof( bufC, orig_val.z );
      
      return true;
   }
   return false;
}

//------------------------------------------------------------------------------
//                       ActionBtn Member Win functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void ActionBtn_MemberWin::create(const HWND parent, HINSTANCE hInst, const SimTag fieldDes, const Inspect_ActionBtn &actBtn, const SimManager *mngr)
{
   HDC hdc;
   SIZE size;

   des = fieldDes;
   recipient = (actBtn.recipient) ? actBtn.recipient->getId() : 0;
   simMessage = actBtn.simMessage;
   manager = (SimManager *)mngr;

   // initially create small windows at root, we'll position them later
   desMinSize.set(10, 10);
   valMinSize.set(10, 10);
   curSize.set(20, 20);

   // get the member description string from SimTag Dictionary
   const char* memberDes = NULL;
   char buffer[256];
   memberDes = tagDictionary.idToString(des);      

   if (!memberDes)
   {
      sprintf(buffer, "Undefined Inspect Tag %d", des);
      memberDes = buffer;
   }

   // create the window for the member description
   desHWnd = ::CreateWindow("STATIC", " ", WS_CHILD | WS_CLIPSIBLINGS | SS_RIGHT, 
                            0, 0, desMinSize.x, desMinSize.y, parent, (HMENU)0, hInst, NULL);

   if (! desHWnd)
      return;

   // get the minimum window size for the member description 
   desMinSize.set(DBL_BORDER_SPACING, DBL_BORDER_SPACING);

   // create window for the member value
   valHWnd = ::CreateWindow("BUTTON",memberDes, WS_CHILD | WS_CLIPSIBLINGS | BS_PUSHBUTTON, 
                                 0, 0, desMinSize.x, desMinSize.y, parent, (HMENU)0, hInst, NULL);

   if (! valHWnd)
   {
      desMinSize.set(10,10);
      DestroyWindow(desHWnd);
      return;   
   }

   // get the minimum window size for the member value
   hdc = GetDC(valHWnd);
   if (hdc)
   {
      GetTextExtentPoint32(hdc, memberDes, strlen(memberDes), &size);
      size.cx += 20;
      size.cy += 8;
      valMinSize.set(size.cx + DBL_BORDER_SPACING + EDIT_BORDER, size.cy + DBL_BORDER_SPACING);
   }
   ReleaseDC(valHWnd, hdc);

   updateBtnState();

   // cap the minValues.
   if (valMinSize.x > 250) valMinSize.x = 250;
   if (valMinSize.x < 150) valMinSize.x = 150;

   // get bounding size for both min values
   curSize.x = valMinSize.x + desMinSize.x;
   curSize.y = (valMinSize.y > desMinSize.y) ? valMinSize.y : desMinSize.y;
}   

//------------------------------------------------------------------------------
void ActionBtn_MemberWin::position(Box2I &bbox, Point2I &recommendedDesSize)
{
   curSize.x = bbox.fMax.x - bbox.fMin.x;
   curSize.y = bbox.fMax.y - bbox.fMin.y;

   SetWindowPos(desHWnd, 0, 
                bbox.fMin.x + BORDER_SPACING, 
                bbox.fMin.y + BORDER_SPACING, 
                recommendedDesSize.x - DBL_BORDER_SPACING, 
                curSize.y - DBL_BORDER_SPACING, 
                SWP_NOZORDER | SWP_SHOWWINDOW);
   SetWindowPos(valHWnd, 0, 
                bbox.fMin.x + recommendedDesSize.x + BORDER_SPACING, 
                bbox.fMin.y + BORDER_SPACING,
                bbox.fMax.x - recommendedDesSize.x - DBL_BORDER_SPACING,
                curSize.y - DBL_BORDER_SPACING,  
                SWP_NOZORDER | SWP_SHOWWINDOW);
}   

//------------------------------------------------------------------------------
Bool ActionBtn_MemberWin::getVal()
{
   updateBtnState();
   return true;
}   

//------------------------------------------------------------------------------
void ActionBtn_MemberWin::updateBtnState()
{
   if (recipient && !manager->findObject(recipient))   
      recipient = 0;
   
   EnableWindow(valHWnd, (bool)recipient);
}   

//------------------------------------------------------------------------------
void ActionBtn_MemberWin::doAction()
{
   updateBtnState();
   if (recipient)
   {

      SimMessageEvent simMesgEvent;
      simMesgEvent.message = simMessage;
      ( manager->findObject(recipient) )->processEvent( &simMesgEvent );
   }
}
