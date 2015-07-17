// SClrDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSelectColorDialog dialog

#include "Matcanvs.h"
#include "g_pal.h"


typedef enum
{
  BACKGROUND = 0,
  GRID_ORIGIN,
  ADDITIVE_BRUSH,
  SUBTRACTIVE_BRUSH,
  SELECTED_BRUSH,
  DEFAULT_BRUSH,
  LINE_GRID,
  DOT_GRID,
  BOUNDING_BOX,
  VOLUME_BRUSH,
  TOTAL_COLORS
} ColorItemType;


class CSelectColorDialog : public CDialog
{
// Construction
public:
	CSelectColorDialog(CWnd* pParent = NULL);   // standard constructor
	~CSelectColorDialog();

// Dialog Data
	//{{AFX_DATA(CSelectColorDialog)
	enum { IDD = IDD_PICKCOLORDIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSelectColorDialog)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSelectColorDialog)
	afx_msg void OnPaint();
	virtual BOOL OnInitDialog();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSelchangeColorselect();
	afx_msg void OnColorDefaults();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	CMatCanvas*	    m_pFrame;
	GFXPalette*     palette;
    int             current_selection;
    int             selectedcolor[TOTAL_COLORS];

protected:
  void DrawOutlineRect();
  void EraseOutlineRect();

};
