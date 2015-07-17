// AsgnText.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// AssignTexture dialog

#include "matcanvs.h"
#include "g_pal.h"
#include "bspnode.h"
#include "ConstructiveBsp.h"


#define MAX_TICS_PER_DEGREE 2


class CTHREDDoc;

class AssignTexture : public CDialog
{
// Construction
public:
	AssignTexture(CWnd* pParent = NULL);   // standard constructor
	~AssignTexture();
    void SetIndex(int index);
    int GetIndex();

// Dialog Data
	//{{AFX_DATA(AssignTexture)
	enum { IDD = IDD_ASSIGN_TEXTURE };
	CSpinButtonCtrl	m_spinControl;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(AssignTexture)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

public:
   static int        m_lastX;
   static int        m_lastY;
	int               material_count;
	GFXPalette*       palette;
	TSMaterialList*   material_list;
    double            rotation;
    int               temp_index;

   bool              doneBsp;
    CWnd*             parentview;
    CBspNode*         node;
    CBspNode*         topnode;
    CConstructiveBsp* bsptree;
    CTHREDDoc*        pDoc;

    double            uval;
    double            vval;
    double            rotate_amount;

    int               stretchit;
    void TextureTheNodes(CBspNode* curr_node, CThredBrush* brush, int which_poly, int which_material);
    void ShiftNodesU(CBspNode* curr_node, CThredBrush* brush, int which_poly, double val);
    void ShiftNodesV(CBspNode* curr_node, CThredBrush* brush, int which_poly, double val);


// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(AssignTexture)
	afx_msg void OnPaint();
	virtual void OnCancel();
	afx_msg void OnChangeMatIndex();
	virtual BOOL OnInitDialog();
	afx_msg void OnDeltaposMatSpin(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnUpbutton();
	afx_msg void OnDownbutton();
	afx_msg void OnRadio100();
	afx_msg void OnRadio25();
	afx_msg void OnRadio50();
	afx_msg void OnFliphoriz();
	afx_msg void OnFlipvert();
	afx_msg void OnStretchcheck2();
	virtual void OnOK();
	afx_msg void OnRadio10();
	afx_msg void OnAmbientcheck();
	afx_msg void OnClearambientbut();
	afx_msg void OnSetambientbut();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	// DPW - needed to display textures
	CMatCanvas*	m_pFrame1;
	int         material_index1;
    
    double      deltau;
    double      deltav;
    double      lastu;
    double      lastv;
    double      u_val;
    double      v_val;
    float       r_delta;
    float       last_rotation;
    int         lastrot;
    UInt8      last_scaleshift;

    void RotateNodes(CBspNode* curr_node, CThredBrush* brush, int which_poly, Point2F center, double val);
    void FlipNodesH(CBspNode* curr_node, CThredBrush* brush, int which_poly, Point2F center);
    void FlipNodesV(CBspNode* curr_node, CThredBrush* brush, int which_poly, Point2F center);
    void FindTextureCenter(CBspNode* curr_node, CThredBrush* brush, int which_poly);
    void ClearHandles(CBspNode* curr_node, CThredBrush* brush, int which_poly);

};
