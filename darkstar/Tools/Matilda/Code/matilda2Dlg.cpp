// matilda2Dlg.cpp : implementation file
//

#include <stdafx.h>
#include <winreg.h>

#include <types.h>

#include "matilda2.h"
#include "matilda2Dlg.h"
#include "palHelp.h"
#include "ChangeLighting.h"
#include "NullEditDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CMatilda2App theApp;
HWND    ghDlg;          // Handle to main dialog box
HACCEL  ghAccelTable;   // Handle to accelerator table
BOOL WantStretched=false;
 

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMatilda2Dlg dialog

CMatilda2Dlg::CMatilda2Dlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMatilda2Dlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMatilda2Dlg)
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_currDetailLevel = 0;
	gPalette          = NULL;

	m_copyBuffer   = NULL;
	m_numCopyItems = 0;
}


CMatilda2Dlg::~CMatilda2Dlg()
{
	if (gPalette != NULL) {
		delete gPalette;
		gPalette = NULL;
	}
	
	if (m_copyBuffer != NULL) {
		for (int i = 0; i < mlEdit.getNumDetailLevels(); i++) {
			delete [] m_copyBuffer[i]; m_copyBuffer[i]=NULL;
		}
		delete [] m_copyBuffer;
		m_copyBuffer = NULL;
	}
}


void CMatilda2Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMatilda2Dlg)
	DDX_Control(pDX, ID_DETAIL_SLIDER, m_detailSlider);
	DDX_Control(pDX, ID_CURRENTDETAIL_EDIT, m_detailEdit);
	DDX_Control(pDX, ID_MATERIAL_LIST, m_matList);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMatilda2Dlg, CDialog)
	//{{AFX_MSG_MAP(CMatilda2Dlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(ID_EDITAS_RGB, OnEditasRgb)
	ON_BN_CLICKED(ID_EDITAS_PALETTE, OnEditasPalette)
	ON_BN_CLICKED(ID_EDITAS_TEXTURE, OnEditasTexture)
	ON_LBN_DBLCLK(ID_MATERIAL_LIST, OnDblclkMaterialList)
	ON_LBN_SELCHANGE(ID_MATERIAL_LIST, OnSelchangeMaterialList)
	ON_BN_CLICKED(ID_EDITAS_NULL, OnEditasNull)
	ON_BN_CLICKED(ID_APPEND_NEW, OnAppendNew)
	ON_WM_HSCROLL()
	ON_WM_PALETTECHANGED()
	ON_WM_QUERYNEWPALETTE()
	ON_BN_CLICKED(ID_DELETE_SELECTED, OnDeleteSelected)
	ON_BN_CLICKED(ID_COPY_DETAILLEVEL, OnCopyDetaillevel)
	ON_BN_CLICKED(ID_COPY_COPYSELECTED, OnEditCopy)
	ON_BN_CLICKED(ID_COPY_PASTECURSOR, OnEditPaste)
	ON_BN_CLICKED(ID_COPY_PASTETOEND, OnEditPastetoend)
	ON_BN_CLICKED(IDC_CHECK1, OnCheck1)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMatilda2Dlg message handlers

BOOL CMatilda2Dlg::OnInitDialog()
{
	CRect rcCanvas;
	CWnd* pCanvas = GetDlgItem(ID_PREVIEW_FRAME);
	pCanvas->GetWindowRect(rcCanvas);
	ScreenToClient(rcCanvas);
	m_preview.Create(this, rcCanvas, ID_PREVIEW_FRAME);

	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	CString strAboutMenu;
	strAboutMenu.LoadString(IDS_ABOUTBOX);
	if (!strAboutMenu.IsEmpty())
	{
		pSysMenu->AppendMenu(MF_SEPARATOR);
		pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
		//load the accelerator table
	ghDlg = m_hWnd;



	CString cmdLine= theApp.m_lpCmdLine;
	cmdLine = cmdLine.SpanExcluding( "\"");

	//check for palette in registry
	CString palName= theApp.GetProfileString( "Matilda3", "Palette");
	if (!palName.IsEmpty()) {
		// load new palette
		loadPalette( (const char*)(LPCTSTR)palName );

		// override the static text for palette name field in dialog
		SetDlgItemText(ID_CURRENTPALETTE, (const char*)(LPCTSTR)palName);

	}


	if (!cmdLine.IsEmpty()) {
		OpenTheFile((char *)(LPCTSTR)cmdLine);
	}
	

	CString buffer;
	buffer.Format("%d", m_currDetailLevel);
	m_detailEdit.SetWindowText(buffer);

	// Get the last palette loaded from the system registry, and load it.  If there is
	// no key set, pass NULL to the loadPalette function, and 
	//
/*	HKEY	regKey;
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Matilda2", 0, KEY_QUERY_VALUE, &regKey) != ERROR_SUCCESS) {
		loadPalette(NULL);
	} else {
		unsigned char *fileNameBuffer = new unsigned char[2048];
		UInt32 receiveNumBytes = 2048;
		DWORD type;
		int result = RegQueryValueEx(regKey, "PaletteFile", 0, &type, fileNameBuffer, &receiveNumBytes);
		loadPalette((const char *)fileNameBuffer);
		delete [] fileNameBuffer; fileNameBuffer=NULL;
		RegCloseKey(regKey);
	}*/

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CMatilda2Dlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	} else if ((nID & 0xFFF0) == SC_CLOSE) {
		OnFileExit();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CMatilda2Dlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		m_preview.Refresh();
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CMatilda2Dlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}


void CMatilda2Dlg::OnEditasRgb() 
{
	RGBEditDlg rgbDlg;
	
	WORD red		= 0;
	WORD green		= 0;
	WORD blue		= 0;
	TS::Material::ShadingType lightingType;

	int currSel = m_matList.GetCurSel();
	if (currSel == LB_ERR || (mlEdit.isMaterialListValid() != TRUE)) {
		return;
	}

	mlEdit.fSetCurrIndex(currSel);
	
	if (mlEdit.getMatType() == TS::Material::MatRGB) {
		mlEdit.getRGB(red, green, blue);
	}
	if (mlEdit.getMatType() == TS::Material::MatNull) {
		lightingType = TS::Material::ShadingNone;
	} else {
		lightingType = mlEdit.getMatLighting();
	}

	mlEdit.getCommonProps ( rgbDlg.commonProps );
	
	int response = rgbDlg.DoModal(&red, &green, &blue, &lightingType);

	if (response == IDOK) {
		mlEdit.setMatToRGB(lightingType, (UInt8)red, (UInt8)green, (UInt8)blue);
		mlEdit.setMatToCommonProps ( rgbDlg.commonProps );
		UpdateListMember(currSel);
		m_matList.SetCurSel(currSel);
	}
}

void CMatilda2Dlg::OnEditasPalette() 
{
	palEditDlg	palDlg;

	UInt32						 palIndex;
	TS::Material::ShadingType	 lightingType;

	int currSel = m_matList.GetCurSel();
	if (currSel == LB_ERR || (mlEdit.isMaterialListValid() != TRUE)) {
		return;
	}

	mlEdit.fSetCurrIndex(currSel);
	
	if (mlEdit.getMatType() == TS::Material::MatPalette) {
		palIndex = mlEdit.getPaletteIndex();
	}
	if (mlEdit.getMatType() == TS::Material::MatNull) {
		lightingType = TS::Material::ShadingNone;
	} else {
		lightingType = mlEdit.getMatLighting();
	}
	mlEdit.getCommonProps ( palDlg.commonProps );
	
	int response = palDlg.DoModal(&palIndex, &lightingType);

	if (response == IDOK) {
		mlEdit.setMatToPalette(lightingType, palIndex);
		mlEdit.setMatToCommonProps ( palDlg.commonProps );
		UpdateListMember(currSel);
		m_matList.SetCurSel(currSel);
	}
}

void CMatilda2Dlg::OnEditasTexture() 
{
	TEXEditDlg texDlg;
	
	CString						buffer;
	TS::Material::ShadingType	lightingType;
	UInt32						flags = 0;
			
	int currSel = m_matList.GetCurSel();
	if (currSel == LB_ERR || (mlEdit.isMaterialListValid() != TRUE)) {
		return;
	}

	mlEdit.fSetCurrIndex(currSel);

	if (mlEdit.getMatType() == TS::Material::MatTexture) {
		flags  = mlEdit.getTextureFlags();
		buffer = mlEdit.getBitMapName();
	}
	if (mlEdit.getMatType() == TS::Material::MatNull) {
		lightingType = TS::Material::ShadingNone;
	} else {
		lightingType = mlEdit.getMatLighting();
	}
	mlEdit.getCommonProps ( texDlg.commonProps );
			
	int result = texDlg.DoModal(buffer, &lightingType, &flags );

	if (result == IDOK) {
		mlEdit.setMatToTexture(lightingType, flags);

		if (!buffer.IsEmpty()) {
			ResourceObject *obj= rm.load(buffer);
			if (!obj || !obj->resource) {
				AfxMessageBox("MaterialListEdit: Error loading bitmap file", MB_OK, 0);
			}
			else {
				mlEdit.setBitMapName(buffer);
				mlEdit.setMatToCommonProps ( texDlg.commonProps );
				UpdateListMember(currSel);
				m_matList.SetCurSel(currSel);
			}
		}
	}
}


void CMatilda2Dlg::OnEditasNull() 
{
	NullEditDlg		nulDlg;

	int currSel = m_matList.GetCurSel();
	if (currSel == LB_ERR || mlEdit.isMaterialListValid() != TRUE) 
		return;
	
	mlEdit.getCommonProps ( nulDlg.commonProps );

	int result = nulDlg.DoModal();

	if (result == IDOK) {
		mlEdit.fSetCurrIndex(currSel);
		mlEdit.setMatToNull();
		mlEdit.setMatToCommonProps ( nulDlg.commonProps );
		UpdateListMember(currSel);
		m_matList.SetCurSel(currSel);
	}
}


void CMatilda2Dlg::UpdateList()
{
	if (mlEdit.isMaterialListValid() == FALSE) {
		return;
	}


	//zmod added >= and <= in following assertion
	AssertFatal(m_currDetailLevel >= 0 &&
				m_currDetailLevel <= mlEdit.getNumDetailLevels(),
				"Error: invalid detail Level...:");
	
	int currSel = m_matList.GetCurSel();
	
	mlEdit.setDetailLevel(m_currDetailLevel);

	// TODO, find a more efficient way to do this...
	//
	m_matList.ResetContent();

	for (int i = 0; i < mlEdit.iGetListSize(); i++) {
		m_matList.AddString(mlEdit.szNameOfMaterial(i));
	}

	if (currSel != LB_ERR) {
		m_matList.SetCurSel(currSel);
	}

	//z?
	m_preview.installMaterial(mlEdit.getCurrMat(), false);
	m_preview.Invalidate();
}


void CMatilda2Dlg::UpdateListMember(int in_index)
{
	UpdateList();
	m_matList.SetSel(in_index);
	m_matList.SetCaretIndex(in_index, TRUE);
}


void CMatilda2Dlg::OnFileExit() 
{
	// TODO: Add your command handler code here
	if (mlEdit.isMaterialListValid() && mlEdit.isSaveAdvised()) {
		int result = AfxMessageBox("Unsaved work.  Save?",
								   MB_YESNOCANCEL|MB_ICONQUESTION);
		if (result == IDYES) {
			int result = OnFileSave();
			if (result == IDCANCEL) {
				return;
			}
		} else if (result == IDCANCEL) {
			// Return to editing...
			//
			return;
		}
	}
	// Exit the editor...
	OnOK();
}


void CMatilda2Dlg::OnFileNew() 
{

	if (mlEdit.isMaterialListValid() && mlEdit.isSaveAdvised()) {
		int result = AfxMessageBox("Unsaved work.  Save?",
								   MB_YESNOCANCEL|MB_ICONQUESTION);
		if (result == IDYES) {
			int result = OnFileSave();
			if (result == IDCANCEL) {
				return;
			}
		} else if (result == IDCANCEL) {
			// Exit without trashing the material list...
			//
			return;
		}
	}
	// Create the new material list...
	//
	detailLevelsDlg detailDlg;
	int levels;

	// Get the number of levels in the new list...
	//
	if (detailDlg.DoModal(&levels) == IDCANCEL) {
		return;
	}

	mlEdit.fCreateNewList(levels);
	mlEdit.fSetCurrIndex(0);
	
	//reset the search path
	pSearchPath="";

	// Set the detail Slider to the appropriate number of positions...
	//
	m_detailSlider.SetRangeMin(0);
	m_detailSlider.SetRangeMax(levels - 1);

	UpdateList();
}

int CMatilda2Dlg::OnFileSave() 
{
	if (mlEdit.isMaterialListValid() && mlEdit.isSaveAdvised() &&
		mlEdit.isMatListFileNameGood() == TRUE) {
		if (mlEdit.isMatListFileNameGood()) {
			if(mlEdit.fSaveMaterialList() == TRUE){
				return IDOK;
			} else {
				AfxMessageBox("Error Saving Material List!", MB_OK|MB_ICONEXCLAMATION);
				return IDCANCEL;				
			}
		} else {
			return OnFileSaveas();
		}
	}
	// No material list to save, so I guess it's ok... :)
	//
	return IDOK;
}

int CMatilda2Dlg::OnFileSaveas() 
{
	const char* szDefaultFile;
	static const char achDefaultExt[] = "*.dml";
	static const char achDefFileType[] = "dml";

	// if we don't have a good material list, bail now
	if( mlEdit.isMaterialListValid() == false )
		return IDCANCEL;
		// instantiate a file dialog box w/ last known name
	if( mlEdit.isMatListFileNameGood() == true )
	{
		szDefaultFile = mlEdit.szGetMatListFileName();
	}
	else // use a default extension
	{
		szDefaultFile = achDefaultExt;
	}
	CFileDialog fileDlg(FALSE, 
						 szDefaultFile, 
						 NULL,
                         OFN_OVERWRITEPROMPT,
	                     "Material List (*.DML)|*.DML|All files (*.*)|*.*||");

	int result = fileDlg.DoModal();

	if (result == IDCANCEL) {
		return result;
	}

	mlEdit.fSetMatListFileName(fileDlg.GetFileName(), fileDlg.GetPathName());

	result = mlEdit.fSaveMaterialList();

	if (result == TRUE) {
		return IDOK;
	} else {
		AfxMessageBox("Error Saving Material List!", MB_OK|MB_ICONEXCLAMATION);
		return IDCANCEL;
	}
}


void CMatilda2Dlg::OnFileOpen() 
{

	if (mlEdit.isMaterialListValid() && mlEdit.isSaveAdvised()) {
		int result = AfxMessageBox("Unsaved work.  Save?",
								   MB_YESNOCANCEL|MB_ICONQUESTION);
		if (result == IDYES) {
			int result = OnFileSave();
			if (result == IDCANCEL) {
				return;
			}
		} else if (result == IDCANCEL) {
			// Exit without trashing the material list...
			//
			return;
		}
	}
	//init the mat list in preparation for loading a new one
	mlEdit.fCreateNewList(1);
	mlEdit.fSetCurrIndex(0);
	UpdateList();
	
	CFileDialog fileDialog(TRUE, 
						 NULL, 
						 NULL,
						 OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
                         "Material List (*.DML)|*.DML|All files (*.*)|*.*||");
	int result = fileDialog.DoModal();
	if (result == IDCANCEL) {
		return;
	}


//set the search path to directory
//	char tempPath[512];
//	strncpy (tempPath, fileDialog.GetPathName(), fileDialog.m_ofn.nFileOffset);
	pSearchPath=fileDialog.GetPathName();
    int i = pSearchPath.ReverseFind('\\');
	int i2 = pSearchPath.ReverseFind('/');
	if (i2 != -1) i=i2;
	pSearchPath=pSearchPath.Left(i+1);



   rm.setSearchPath(pSearchPath);
//     rm.add( NULL, pSearchPath );
//	vrStream.setSearchPath((char *)(LPCTSTR)pSearchPath);

	mlEdit.fSetMatListFileName(fileDialog.GetFileTitle(), fileDialog.GetPathName());
	if (mlEdit.fReadMaterialList() != TRUE) {
		AfxMessageBox("Error reading Material List!", MB_OK|MB_ICONEXCLAMATION);
		return;
	}
	
	mlEdit.fSetCurrIndex(0);
	// Set the detail Slider to the appropriate number of positions...
	//
	m_detailSlider.SetRangeMin(0);
	m_detailSlider.SetRangeMax(mlEdit.getNumDetailLevels() - 1);

	UpdateList();
}


//opens the file if given a full path
void CMatilda2Dlg::OpenTheFile(const char *path) 
{
	CString theFile;
	CString thePath= path;
	int index= thePath.ReverseFind('\\');
	int index2= thePath.ReverseFind('/');
	if (index2 != -1) index=index2;

	if (index == -1) {
		theFile=thePath;
		thePath="";
	} else {
		theFile= thePath.Mid(index+1);
		thePath= thePath.Left(index+1);
	}

	char curDir[512];
	if (thePath.IsEmpty()) {
		::GetCurrentDirectory(512,curDir);
		pSearchPath= curDir;
		pSearchPath+= '\\';
	} else	pSearchPath=thePath;

//	vrStream.setSearchPath((char *)(LPCTSTR)pSearchPath);

	//init the mat list in preparation for loading a new one
	mlEdit.fCreateNewList(1);
	mlEdit.fSetCurrIndex(0);
	UpdateList();

	mlEdit.fSetMatListFileName(theFile, thePath+theFile);
	if (mlEdit.fReadMaterialList() != TRUE) {
		AfxMessageBox("Error reading Material List!", MB_OK|MB_ICONEXCLAMATION);
		return;
	}
	
	mlEdit.fSetCurrIndex(0);
	// Set the detail Slider to the appropriate number of positions...
	//
	m_detailSlider.SetRangeMin(0);
	m_detailSlider.SetRangeMax(mlEdit.getNumDetailLevels() - 1);

	UpdateList();
}



void CMatilda2Dlg::OnDblclkMaterialList() 
{
	int editMat = m_matList.GetCurSel();

	if (editMat == LB_ERR) {
		return;
	}

	if (mlEdit.isMaterialListValid()) {
		TS::Material *matToEdit;
		
		mlEdit.fSetCurrIndex(editMat);

		// Note the returned pointer is to a temporary copy created
		//	using new.  
		matToEdit = mlEdit.getCurrMat();

		TS::Material::ShadingType	lightType	= (enum TS::Material::ShadingType)(matToEdit->fParams.fFlags &TS::Material::ShadingFlags);
		TS::Material::MatType		matType		= (enum TS::Material::MatType)(matToEdit->fParams.fFlags &TS::Material::MatFlags);

		UInt32						palIndex	= (UInt32)matToEdit->fParams.fIndex;
		UInt32						texFlags	= matToEdit->fParams.fFlags;
		WORD						red			= matToEdit->fParams.fRGB.fRed;
		WORD						green		= matToEdit->fParams.fRGB.fGreen;
		WORD						blue		= matToEdit->fParams.fRGB.fBlue;
		CString						texName		= mlEdit.getBitMapName();
		
		RGBEditDlg		rgbDlg;
		palEditDlg		palDlg;
		TEXEditDlg		texDlg;
		NullEditDlg		nulDlg;

		switch (matType) {
		case TS::Material::MatNull:
			mlEdit.getCommonProps ( nulDlg.commonProps );
			if ( nulDlg.DoModal () == IDOK ) {
				mlEdit.setMatToCommonProps ( nulDlg.commonProps );
				UpdateListMember ( editMat );
			}
			break;

		case TS::Material::MatRGB:
			mlEdit.getCommonProps ( rgbDlg.commonProps );
			if (rgbDlg.DoModal(&red, &green, &blue, &lightType) == IDOK) {
				mlEdit.setMatToRGB(lightType, red, green, blue);
				mlEdit.setMatToCommonProps ( rgbDlg.commonProps );
				UpdateListMember(editMat);
			}
			break;

		case TS::Material::MatPalette:
			mlEdit.getCommonProps ( palDlg.commonProps );
			if (palDlg.DoModal(&palIndex, &lightType) == IDOK) {
				mlEdit.setMatToPalette(lightType, palIndex);
				mlEdit.setMatToCommonProps ( palDlg.commonProps );
				UpdateListMember(editMat);
			}
			break;

		case TS::Material::MatTexture:
			mlEdit.getCommonProps ( texDlg.commonProps );
			if (texDlg.DoModal(texName, &lightType, &texFlags ) == IDOK)
			{
				if (!texName.IsEmpty()) {
					ResourceObject *obj= rm.load(texName);
					if (!obj || !obj->resource) {
						AfxMessageBox (
							"MaterialListEdit: Error loading bitmap file", 
							MB_OK, 0);
					}
					else {
						mlEdit.setMatToTexture(lightType, texFlags);
						mlEdit.setBitMapName(texName);
						mlEdit.setMatToCommonProps ( texDlg.commonProps );
						UpdateListMember(editMat);
					}
				}

			}
			break;
		}

		delete matToEdit;
	}
}

void CMatilda2Dlg::OnSelchangeMaterialList() 
{
	int newSelection = m_matList.GetCurSel();

	if (newSelection == LB_ERR) {
		return;
	}

	mlEdit.fSetCurrIndex(newSelection);
	m_preview.installMaterial(mlEdit.getCurrMat(), false);

	CString matText;
	m_matList.GetText(m_matList.GetCurSel(),matText);
	if (matText.Find("*not in") == -1) m_preview.Invalidate();

}

void CMatilda2Dlg::OnAppendNew() 
{
	if (mlEdit.isMaterialListValid() != TRUE) {
		return;
	}

	mlEdit.fAddEntryToEnd();
	mlEdit.fSetCurrIndex(mlEdit.iGetListSize() - 1);
	UpdateList();
	LRESULT test = m_matList.SetCurSel(m_matList.GetCount() - 1);
	m_matList.Invalidate();
	// ENTER CODE TO SWITCH PREVIEW...
}


// Since there is only one slider in the window, the detail slider, this is
// all compressed...
//
void CMatilda2Dlg::OnHScroll(UINT /*nSBCode*/, UINT /*nPos*/, CScrollBar* pScrollBar) 
{
	m_currDetailLevel = ((CSliderCtrl *)pScrollBar)->GetPos();

	CString newLevel;
	newLevel.Format("%d", m_currDetailLevel);

	m_detailEdit.SetWindowText(newLevel);
	mlEdit.setDetailLevel(m_currDetailLevel);

	UpdateList();
//	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}


BOOL CMatilda2Dlg::loadPalette(const char *in_palName)
{
	if (in_palName == NULL) {
		// Load system Palette...
		//
		if (mlEdit.pGFXPal) {
			delete mlEdit.pGFXPal;
		}
		mlEdit.pGFXPal = new GFXPalette;
		
		CClientDC dc(this);
		GetSystemPaletteEntries(dc, 0, 255, mlEdit.pGFXPal->palette[0].color);

		// delete old palette if it already exist
		if( gPalette != NULL ) {
			delete gPalette;
			gPalette = NULL;
		}

		// instantiate new palette
		gPalette = new CPalette;
		if( gPalette->CreatePalette( (LOGPALETTE*)mlEdit.pGFXPal )  == 0) {
			afxDump << " Cannot Load Palette \n";
			return FALSE;
		}
		m_preview.setPalette(mlEdit.pGFXPal);


		GetDlgItem(ID_CURRENTPALETTE)->SetWindowText("System Default");

	} else {
		// Load the palette file...
		//
		if (mlEdit.pGFXPal) {
			delete mlEdit.pGFXPal;
		}
		mlEdit.pGFXPal = GFXPalette::load(in_palName);
		if (mlEdit.pGFXPal == NULL) {
			AfxMessageBox("Error Loading palette!  Palette Specified in the Registry is not valid.  Select Another...");
			OnPaletteSetcurrentpalette();
		} else {
			// load dynamix palette into a logical palette and set NOCOLLAPSE flag
			LOGPALETTE* pDynPal = ( LOGPALETTE*)&(mlEdit.pGFXPal->version);
			LOGPALETTE* pPal = (LOGPALETTE*) malloc(sizeof(LOGPALETTE) 
					               	+ 236 * sizeof(PALETTEENTRY));

			pPal->palVersion = 0x300;  // Windows 3.0
			pPal->palNumEntries = 236; // table size

			// dynamix fills in their pals w/ 10 dummy arguments
			for (int i=0; i<236; i++)  {
				pPal->palPalEntry[i].peRed = 	pDynPal->palPalEntry[i+10].peRed;
				pPal->palPalEntry[i].peGreen = pDynPal->palPalEntry[i+10].peGreen;
				pPal->palPalEntry[i].peBlue = pDynPal->palPalEntry[i+10].peBlue;
				pPal->palPalEntry[i].peFlags = PC_NOCOLLAPSE;
			}

			// delete old palette if it already exist
			if( gPalette != NULL ) {
				delete gPalette;
				gPalette = NULL;
			}

			// instantiate new palette
			gPalette = new CPalette;
			if( gPalette->CreatePalette( pPal )  == 0) {
				afxDump << " Cannot Load Palette \n";
				return FALSE;
			}
			free( pPal );

			// Since the preview window in the main dialog isn't constructing/destructing all the
			// time like the subsidiary dialogs, we need to reset the palette whenever we reload...
			//
			m_preview.setPalette(mlEdit.pGFXPal);

			// Now we need to mark in the registry the palette file that we opened successfully..
			//
		/*	HKEY	regKey;
			DWORD   disposition = strlen(in_palName) + 1;
			if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Matilda2", 0, KEY_ALL_ACCESS, &regKey) == ERROR_SUCCESS) {
				// Do nuthin...
				//
			} else {
				RegCreateKeyEx(HKEY_LOCAL_MACHINE, "Matilda2", 0, NULL,
								REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,
								NULL, &regKey, &disposition);
			}
			int result = RegSetValueEx(regKey, "PaletteFile", 0, REG_SZ, (const unsigned char *)in_palName, strlen(in_palName) + 1);
			RegCloseKey(regKey);

			GetDlgItem(ID_CURRENTPALETTE)->SetWindowText(in_palName);*/
		}
	}
	// Force a redraw...
	//
	OnQueryNewPalette();
	Invalidate();

	return TRUE;
}

void CMatilda2Dlg::OnPaletteChanged(CWnd* pFocusWnd) 
{
	// if we get the focus realize our palette
	if (pFocusWnd == this || IsChild(pFocusWnd))
		return;

	OnQueryNewPalette();
}


BOOL CMatilda2Dlg::OnQueryNewPalette() 
{
	CClientDC dc(this);
	return RealizeSysPalette( dc );
	//m_preview.setPalette(mlEdit.pGFXPal);
	//return TRUE;
}

void CMatilda2Dlg::OnDeleteSelected() 
{
	int currSelected = 0;

	if (!mlEdit.isMaterialListValid()) {
		return;
	}
	if (mlEdit.iGetListSize() > 1) {
		bool *markBuffer = new bool[m_matList.GetCount()];
		int numMarked = 0;
		for (int i = 0; i < m_matList.GetCount(); i++) {
			if (m_matList.GetSel(i)) {
				markBuffer[i] = true;
				numMarked++;
			} else {
				markBuffer[i] = false;
			}
		}
		
		if (numMarked == 0)
			return;

		if (numMarked == mlEdit.iGetListSize()) {
			AfxMessageBox("Error: Cannot delete all list entries");
		}

		for (int j = (mlEdit.iGetListSize()-1); j >= 0; j--) {
			if (markBuffer[j])
				mlEdit.fDeleteEntry(j);
		}

		delete [] markBuffer;
		UpdateList();
	}
	else {
		AfxMessageBox("Cannot delete last entry...", MB_OK|MB_ICONEXCLAMATION);
	}
}


void CMatilda2Dlg::OnPaletteSetcurrentpalette() 
{
	int iPalExitStat;

	// use a file dialog to prompt for the palette
	CFileDialog dlgPalette(	TRUE, 
							NULL, 
							NULL,
                           	OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
       	"Palette Files (*.PPL,*.PAL,*.WPL)|*.PPL;*.PAL;*.WPL|All files (*.*)|*.*||");

	iPalExitStat = dlgPalette.DoModal();

	// if user exited w/ by clicking ok
	if( iPalExitStat == IDOK )
	{
		// override the static text for palette name field in dialog
		SetDlgItemText(ID_CURRENTPALETTE, (const char*)(dlgPalette.GetFileName()));

		// load new palette
		loadPalette( dlgPalette.GetPathName() );


		//update the registry
		theApp.WriteProfileString("Matilda3","Palette",dlgPalette.GetPathName());

		// invalidate the dialog box to force a redraw
		Invalidate();
	}
}

void CMatilda2Dlg::OnCopyDetaillevel() 
{
	copyDetailDlg copyDlg;

	int copyFrom, copyTo;

	int result = copyDlg.DoModal(&copyFrom, &copyTo);

	if (result == IDOK) {
		if (copyFrom >= mlEdit.getNumDetailLevels() || copyTo >= mlEdit.getNumDetailLevels() ||
			copyFrom == copyTo) {
			AfxMessageBox("Error: invalid copy operation...");
			return;
		}

		mlEdit.copyDetailLevel(copyFrom, copyTo);
	}
}

void CMatilda2Dlg::OnEditCopy() 
{
	if (!mlEdit.isMaterialListValid())
		return;
	
	if (m_copyBuffer != NULL) {
		for (int k = 0; k < mlEdit.getNumDetailLevels(); k++) {
			delete [] m_copyBuffer[k];
		}
		delete [] m_copyBuffer;
		m_copyBuffer = NULL;
	}

	bool *markBuffer = new bool[m_matList.GetCount()];
	int numMarked = 0;
	for (int i = 0; i < m_matList.GetCount(); i++) {
		if (m_matList.GetSel(i)) {
			markBuffer[i] = true;
			numMarked++;
		} else {
			markBuffer[i] = false;
		}
	}

	if (numMarked == 0)
		return;

	m_copyBuffer = new TS::Material*[mlEdit.getNumDetailLevels()];
	for (int j = 0; j < mlEdit.getNumDetailLevels(); j++) {
		m_copyBuffer[j] = new TS::Material[numMarked];
	}

	int origDetail = mlEdit.getDetailLevel();
	int origIndex  = mlEdit.iGetCurrIndex();


	for (int detLevel = 0; detLevel < mlEdit.getNumDetailLevels(); detLevel++) {
		mlEdit.setDetailLevel(detLevel);
		int currIndex = 0;
		for (i = 0; i < m_matList.GetCount(); i++) {
			
			mlEdit.fSetCurrIndex(i);
			if (markBuffer[i] == true) {
				m_copyBuffer[detLevel][currIndex++] = *(mlEdit.getCurrMat());
			}
		}
	}
	
	m_numCopyItems = numMarked;

	mlEdit.setDetailLevel(origDetail);
	mlEdit.fSetCurrIndex(origIndex);

	delete [] markBuffer;
}

void CMatilda2Dlg::OnEditPaste() 
{
	if (!mlEdit.isMaterialListValid())
		return;

	if (m_copyBuffer == NULL) {
		return;
	}

	int origDetail = mlEdit.getDetailLevel();
	int origIndex  = mlEdit.iGetCurrIndex();

	int insertPoint = origIndex;

	// Make sure there are enough entries after the copy point...
	if ((insertPoint + m_numCopyItems) > mlEdit.iGetListSize()) {
		int size = mlEdit.iGetListSize();
		for (int i = 0; i < (size - (m_numCopyItems - insertPoint)); i++) {
			mlEdit.fAddEntryToEnd();
		}
	}

	for (int i = 0; i < mlEdit.getNumDetailLevels(); i++) {
		mlEdit.setDetailLevel(i);
		for (int k = 0; k < m_numCopyItems; k++) {
			mlEdit.fSetCurrIndex(insertPoint + k);
			mlEdit.copyToCurrIndex(&m_copyBuffer[i][k]);
		}
	}

	mlEdit.setDetailLevel(origDetail);
	mlEdit.fSetCurrIndex(origIndex);
	UpdateList();
}

void CMatilda2Dlg::OnEditPastetoend() 
{
	if (!mlEdit.isMaterialListValid())
		return;

	if (m_copyBuffer == NULL) {
		return;
	}

	int origIndex = mlEdit.iGetCurrIndex();

	int insertPoint = mlEdit.iGetListSize();

	for (int i = 0; i < m_numCopyItems; i++) {
		mlEdit.fAddEntryToEnd();
	}

	mlEdit.fSetCurrIndex(insertPoint);

	OnEditPaste();

	mlEdit.fSetCurrIndex(origIndex);
}


void CMatilda2Dlg::OnChangelightingflags() 
{
	if (!mlEdit.isMaterialListValid())
		return;
	
	int origIndex = mlEdit.iGetCurrIndex();
	int size = mlEdit.iGetListSize();
	
	CChangeLighting changeLight;
	TS::Material::ShadingType lightType;
	
	if (changeLight.DoModal(&lightType) == IDCANCEL)
		return;

	for (int i = 0; i < size; i++) {
		if (m_matList.GetSel(i) != 0) {
			TS::Material *temp = NULL;
			mlEdit.fSetCurrIndex(i);
			temp = mlEdit.getCurrMat();

			temp->fParams.fFlags &= ~(TS::Material::ShadingFlags);
			temp->fParams.fFlags |= lightType;

			mlEdit.copyToCurrIndex(temp);
		}
	}
	UpdateList();
	mlEdit.fSetCurrIndex(origIndex);
}

void CMatilda2Dlg::OnCheck1() 
{
	// TODO: Add your control notification handler code here
	WantStretched= !WantStretched;
	m_preview.Invalidate();
	
}
