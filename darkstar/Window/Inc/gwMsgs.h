//------------------------------------------------------------------------------
// Description 
//    Message Map macros.  May need the fancier ones for convenience later...
//
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#ifndef _GWMSGS_H_
#define _GWMSGS_H_

//Includes

#ifdef __BORLANDC__
#pragma option -a4
#endif
#ifdef _MSC_VER
#pragma pack(push,4)
#endif

#define ON_COMMAND(id, pMemberFn) \
   { WM_COMMAND, 0, (WORD)id, (WORD)id, GWParamSig_v_v, (GWMsgMapFn)&pMemberFn },

#define ON_COMMAND_RANGE(id, idLast, pMemberFn)             \
   { WM_COMMAND, 0, (WORD)id, (WORD)idLast, GWParamSig_v_w, \
     (GWMsgMapFn)&pMemberFn },

#define ON_WM_SYSCOMMAND(pMemberFn)             \
   { WM_SYSCOMMAND, 0, 0, 0, GWParamSig_v_wl,   \
     (GWMsgMapFn)(void (GW_MSG_CALL GWWindow::*)(WPARAM, LPARAM))&pMemberFn },

#define ON_NOTIFY(notifyCode, id, pMemberFn)                                      \
   { WM_NOTIFY, (WORD)(int)notifyCode, (WORD)id, (WORD)id, GWParamSig_v_pNMHDRpL, \
     (GWMsgMapFn)(void (GW_MSG_CALL GWWindow::*)(NMHDR*, LRESULT*))&pMemberFn },

#define ON_NOTIFY_RANGE(notifyCode, id, lastId, pMemberFn)                       \
   { WM_NOTIFY, (WORD)(int)notifyCode, (WORD)id, (WORD)lastId,                   \
     GWParamSig_v_pNMHDRpL,                                                      \
     (GWMsgMapFn)(void (GW_MSG_CALL GWWindow::*)(NMHDR*, LRESULT*))&pMemberFn },

#define ON_CONTROL(notifyCode, id, pMemberFn)                           \
   { WM_COMMAND, (WORD)notifyCode, (WORD)id, (WORD)id, GWParamSig_v_v,  \
     (GWMsgMapFn)&pMemberFn },

#define ON_CONTROL_RANGE(notifyCode, id, idLast, pMemberFn)                \
   { WM_COMMAND, (WORD)notifyCode, (WORD)id, (WORD)idLast, GWParamSig_v_w, \
     (GWMsgMapFn)(void (GW_MSG_CALL GWWindow::*)(WORD))&pMemberFn },

#define ON_MESSAGE(message, pMemberFn)                                        \
   { message, 0, 0, 0, GWParamSig_v_wl,                                       \
   (GWMsgMapFn)(void (GW_MSG_CALL GWWindow::*)(WPARAM, LPARAM))&pMemberFn },


//-------------------------------------- BUTTON MESSAGES
#define ON_BN_CLICKED(id, pMemberFn)    \
   ON_CONTROL(BN_CLICKED, id, pMemberFn)
#define ON_BN_DOUBLECLICKED(id, pMemberFn)    \
   ON_CONTROL(BN_DOUBLECLICKED, id, pMemberFn)


//-------------------------------------- EDIT CONTROL MESSAGES
#define ON_EN_KILLFOCUS(id, pMemberFn)     \
   ON_CONTROL(EN_KILLFOCUS, id, pMemberFn)
#define ON_EN_UPDATE(id, pMemberFn)     \
   ON_CONTROL(EN_UPDATE, id, pMemberFn)
#define ON_EN_CHANGE(id, pMemberFn)     \
   ON_CONTROL(EN_CHANGE, id, pMemberFn)

//-------------------------------------- LISTBOX CONTROL MESSAGES
#define ON_LBN_ERRSPACE(id, pMemberFn)    \
	ON_CONTROL(LBN_ERRSPACE, id, pMemberFn)
#define ON_LBN_SELCHANGE(id, pMemberFn)    \
	ON_CONTROL(LBN_SELCHANGE, id, pMemberFn)
#define ON_LBN_DBLCLK(id, pMemberFn)    \
	ON_CONTROL(LBN_DBLCLK, id, pMemberFn)
#define ON_LBN_SELCANCEL(id, pMemberFn)    \
	ON_CONTROL(LBN_SELCANCEL, id, pMemberFn)
#define ON_LBN_SETFOCUS(id, pMemberFn)    \
	ON_CONTROL(LBN_SETFOCUS, id, pMemberFn)
#define ON_LBN_KILLFOCUS(id, pMemberFn)    \
	ON_CONTROL(LBN_KILLFOCUS, id, pMemberFn)


#ifdef __BORLANDC__
#pragma option -a.
#endif
#ifdef _MSC_VER
#pragma pack(pop)
#endif

#endif //_GWMSGS_H_

