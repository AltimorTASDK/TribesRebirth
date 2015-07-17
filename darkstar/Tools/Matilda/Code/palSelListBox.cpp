// palSelListBox.cpp : implementation file
//

#include "stdafx.h"
#include "matilda2.h"
#include "palSelListBox.h"

#include "palHelp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// palSelListBox

palSelListBox::palSelListBox()
{
}

palSelListBox::~palSelListBox()
{
}


BEGIN_MESSAGE_MAP(palSelListBox, CListBox)
	//{{AFX_MSG_MAP(palSelListBox)
	ON_WM_PALETTECHANGED()
	ON_WM_QUERYNEWPALETTE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// palSelListBox message handlers

// Copied wholesale from Matilda 1...
//
void palSelListBox::DrawItem(LPDRAWITEMSTRUCT lpDIS) 
{
	CDC* pDC = CDC::FromHandle(lpDIS->hDC);

	// realize our palette to the DC
	pDC->SelectPalette( gPalette, FALSE );
	pDC->RealizePalette();
	pDC->SelectPalette( &gMapPalette, FALSE );
	int rCnt = pDC->RealizePalette();
	// afxDump << "# of colors realized " << rCnt << "\n";

	// create small rectangle inside of draw area 
	// which is centered vertically and horizontally
	RECT rTarget;
	rTarget.left =  ((5 * lpDIS->rcItem.left) + lpDIS-> rcItem.right) / 6;
	rTarget.right = ((5 * lpDIS->rcItem.right) + lpDIS->rcItem.left) / 6;
	rTarget.top =  ((5 * lpDIS->rcItem.top) + lpDIS-> rcItem.bottom) / 6;
	rTarget.bottom = ((5 * lpDIS->rcItem.bottom) + lpDIS->rcItem.top) / 6;



	if( (lpDIS->itemAction & ODA_DRAWENTIRE) ||
		 ( !(lpDIS->itemState & ODS_SELECTED) && (lpDIS->itemAction & ODA_SELECT) )  	)
		{
		// Paint the color item in the color requested OR
		// Item has been de-selected -- remove frame

		// create a brush for the palette color
		WORD wPalIdx = (WORD)lpDIS->itemData; 
		CBrush brPalItem;
		brPalItem.CreateSolidBrush(PALETTEINDEX(wPalIdx));

		// white filler brush for the background color
		COLORREF cWhite = (COLORREF)RGB(255,255,255);
		CBrush bWhite( cWhite );

		pDC->FillRect(&lpDIS->rcItem, &bWhite);
		pDC->FillRect(&rTarget, &brPalItem);
		}

	if ((lpDIS->itemState & ODS_SELECTED) &&
		(lpDIS->itemAction & (ODA_SELECT | ODA_DRAWENTIRE)))
	{
		// draw a black highlitght box around the selected item
		// create black hilite color
		CBrush brBlackFrame( (COLORREF)RGB(1,1,1) );

		pDC->FrameRect( &lpDIS->rcItem, &brBlackFrame );
	}

}

BOOL palSelListBox::OnQueryNewPalette() 
{
	CClientDC dc(this);	
	return RealizeSysPalette( dc );
}

void palSelListBox::OnPaletteChanged(CWnd* pFocusWnd) 
{
	if (pFocusWnd == this || IsChild(pFocusWnd))
		return;

	OnQueryNewPalette();
}
