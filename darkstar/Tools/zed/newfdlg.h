// NewFDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// NewFileDlg dialog

#include "THREDDoc.h"
#include "filename.h"

#define MAX_BMP_FILES 500

class NewFileDlg : public CDialog
{
// Construction
public:
   bool getTribesPath(char * buffer, DWORD size);
	NewFileDlg(UINT dialogId, CWnd* pParent = NULL);   // standard constructor

   UINT id;
   
// Dialog Data
	//{{AFX_DATA(NewFileDlg)
	enum { CreateIDD = IDD_NEWFILEDLG,
          LoadIDD = IDD_PALMATDIALOG };
	CString	m_strPalette;
	CString	m_strName;
	CString	m_strBitmaps;
	CString	m_strBitmapsVol;
	CString	m_strPaletteVol;
	//}}AFX_DATA

	CTHREDDoc* m_pDoc;
   char    m_filelist[MAX_BMP_FILES*256];
   int     new_mat_count;
   FileName m_TextureFile;
   FileName m_PaletteFile;
   FileName m_PaletteVol;
   FileName m_TextureVol;

   // temp
   char  m_TexturePath[256];
   char  m_TextureName[64];
   char  m_PalettePath[256];
   char  m_PaletteName[64];
   

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(NewFileDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(NewFileDlg)
	afx_msg void OnArray();
	afx_msg void OnPalette();
   afx_msg void OnPaletteVol();
   afx_msg void OnTextureVol();
   afx_msg void OnDefault();
	afx_msg void OnOPEN();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
