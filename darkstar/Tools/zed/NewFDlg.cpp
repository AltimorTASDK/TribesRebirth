// NewFDlg.cpp : implementation file
//

#include "stdafx.h"
#include "thred.h"
#include "NewFDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const char TribesRegKey[] = "Software\\Dynamix\\Starsiege Tribes";
const char DefaultPalette[] = "lush.day.ppl";
const char DefaultPaletteVol[] = "lushworld.vol";
const char DefaultDML[] = "human1.dml";
const char DefaultDMLVol[] = "human1dml.vol";

/////////////////////////////////////////////////////////////////////////////
// NewFileDlg dialog


NewFileDlg::NewFileDlg(UINT dialogId, CWnd* pParent /*=NULL*/)
	: CDialog(dialogId, pParent)
{
   id = dialogId;
   
	//{{AFX_DATA_INIT(NewFileDlg)
	m_strPalette = _T("");
   if(dialogId == CreateIDD)
   	m_strName = _T("");
	m_strBitmaps = _T("");
   m_strPaletteVol = _T("");
   m_strBitmapsVol = _T("");
	//}}AFX_DATA_INIT
}


void NewFileDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(NewFileDlg)
	DDX_Text(pDX, IDC_EDPAL, m_strPalette);
   if(id == CreateIDD)
   	DDX_Text(pDX, IDC_EDFILENAME, m_strName);
	DDX_Text(pDX, IDC_EDARRAY, m_strBitmaps);
   DDX_Text(pDX, IDC_EDPALVOL, m_strPaletteVol);
   DDX_Text(pDX, IDC_EDMATVOL, m_strBitmapsVol);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(NewFileDlg, CDialog)
	//{{AFX_MSG_MAP(NewFileDlg)
	ON_BN_CLICKED(IDC_ARRAY, OnArray)
	ON_BN_CLICKED(IDC_PALETTE, OnPalette)
   ON_BN_CLICKED(IDC_PALETTEVOL, OnPaletteVol)
   ON_BN_CLICKED(IDC_TEXTUREVOL, OnTextureVol)
   ON_BN_CLICKED(IDC_Defaults, OnDefault)
	ON_BN_CLICKED(OPEN, OnOPEN)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// NewFileDlg message handlers

//------------------------------------------------------------------
// returns true if able to get the app path from the registry
bool NewFileDlg::getTribesPath(char * buffer, DWORD size)
{
   bool retVal = false;
   
   // check for the reg key and path
   HKEY key;
   if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, TribesRegKey, 0, KEY_READ, 
      &key) == ERROR_SUCCESS)
   {
      if(RegQueryValueEx(key, "PATH", NULL, NULL, (unsigned char*)buffer, &size) ==
         ERROR_SUCCESS)
      {
         // tack on a '\' if need be
         if(strlen(buffer) > 1)
            if(buffer[strlen(buffer-1)] != '\\')
               strcat(buffer, "\\");
         retVal = true;
      }
      
      RegCloseKey(key);
   }

   return(retVal);
}

void NewFileDlg::OnDefault()
{
   char tribesPath[1024];
   
   // load up the key from the registry...
   if(getTribesPath(tribesPath, sizeof(tribesPath)))
   {
      // fill in the strings
      m_strPalette.Format(DefaultPalette);
      m_strBitmaps.Format(DefaultDML);
      m_strPaletteVol.Format("%sbase\\%s", tribesPath, DefaultPaletteVol);
      m_strBitmapsVol.Format("%sbase\\%s", tribesPath, DefaultDMLVol);

   	if(!UpdateData(false))
         return;
   }
}

void NewFileDlg::OnArray() 
{
	// Browse for a material list
	UpdateData(TRUE);
	CString title = "Material List File";
	CString filter = "Material List Files (*.dml)|*.dml|All Files|*.*||";

	if(m_pDoc->LoadBrowser(m_strBitmaps, filter, title))
		UpdateData(FALSE);
}

void NewFileDlg::OnPalette() 
{
	// Browse for a palette
	UpdateData(TRUE);
	CString title = "Import Palette File";
	CString filter =
	"Palette Files (*.ppl)|*.ppl|All Files|*.*||";
	if(m_pDoc->LoadBrowser(m_strPalette, filter, title))
		UpdateData(FALSE);
}

void NewFileDlg::OnPaletteVol()
{
   UpdateData(true);
   CString title = "Palette Volume File";
   CString filter = "Volume Files (*.vol)|*.vol|All Files|*.*||";
   if(m_pDoc->LoadBrowser(m_strPaletteVol, filter, title))
      UpdateData(false);
}

void NewFileDlg::OnTextureVol()
{
   UpdateData(true);
   CString title = "Texture Volume File";
   CString filter = "Volume Files (*.vol)|*.vol|All Files|*.*||";
   if(m_pDoc->LoadBrowser(m_strBitmapsVol, filter, title))
      UpdateData(false);
}

void NewFileDlg::OnOPEN() 
{
    int i;

	// TODO: Add your control notification handler code here
	if(!UpdateData(TRUE))
		return;
	
   if(id == CreateIDD)
   {
	   char* name;
	   char nameBuf[256];
	   if(SearchPath(".", (LPCTSTR)m_strName, NULL, 256, nameBuf, &name)) {
		   if(AfxMessageBox("File exists, overwrite?", MB_OKCANCEL, 0) == IDOK) {
			   if(!DeleteFile((LPCTSTR)m_strName)) {
				   AfxMessageBox("Couldn't delete old file", MB_OK, 0);
				   return;
			   }
		   }
	   }
   }

   // set the filenames
   m_PaletteVol = m_strPaletteVol;
   m_TextureVol = m_strBitmapsVol;
   m_PaletteFile = m_strPalette;
   m_TextureFile = m_strBitmaps;

	CDialog::OnOK();	
}

void NewFileDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}

BOOL NewFileDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	UpdateData(FALSE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
