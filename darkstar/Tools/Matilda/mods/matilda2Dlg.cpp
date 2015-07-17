// matilda2Dlg.cpp : implementation file
//

#include <stdafx.h>
#include <winreg.h>

#include <types.h>

#include <stdio.h>
#include <filstrm.h>
#include "matilda2.h"
#include "matilda2Dlg.h"
#include "palHelp.h"
#include "ChangeLighting.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

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
			delete [] m_copyBuffer[i];
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
	
	CString buffer;
	buffer.Format("%d", m_currDetailLevel);
	m_detailEdit.SetWindowText(buffer);


	// Get the last palette loaded from the system registry, and load it.  If there is
	// no key set, pass NULL to the loadPalette function, and 
	//
	HKEY	regKey;
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Matilda2", 0, KEY_QUERY_VALUE, &regKey) != ERROR_SUCCESS) {
		loadPalette(NULL);
	} else {
		unsigned char *fileNameBuffer = new unsigned char[2048];
		UInt32 receiveNumBytes = 2048;
		DWORD type;
		int result = RegQueryValueEx(regKey, "PaletteFile", 0, &type, fileNameBuffer, &receiveNumBytes);
		loadPalette((const char *)fileNameBuffer);
		delete [] fileNameBuffer;
		RegCloseKey(regKey);
	}

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
	TSMaterial::LightingType lightingType;

	int currSel = m_matList.GetCurSel();
	if (currSel == LB_ERR || (mlEdit.isMaterialListValid() != TRUE)) {
		return;
	}

	mlEdit.fSetCurrIndex(currSel);
	
	if (mlEdit.getMatType() == TSMaterial::MatRGB) {
		mlEdit.getRGB(red, green, blue);
	}
	if (mlEdit.getMatType() == TSMaterial::MatNull) {
		lightingType = TSMaterial::LightingUnlit;
	} else {
		lightingType = mlEdit.getMatLighting();
	}
	
	int response = rgbDlg.DoModal(&red, &green, &blue, &lightingType);

	if (response == IDOK) {
		mlEdit.setMatToRGB(lightingType, (UInt8)red, (UInt8)green, (UInt8)blue);
		UpdateListMember(currSel);
		m_matList.SetCurSel(currSel);
	}
}

void CMatilda2Dlg::OnEditasPalette() 
{
	palEditDlg	palDlg;

	UInt32						 palIndex;
	TSMaterial::LightingType	 lightingType;

	int currSel = m_matList.GetCurSel();
	if (currSel == LB_ERR || (mlEdit.isMaterialListValid() != TRUE)) {
		return;
	}

	mlEdit.fSetCurrIndex(currSel);
	
	if (mlEdit.getMatType() == TSMaterial::MatPalette) {
		palIndex = mlEdit.getPaletteIndex();
	}
	if (mlEdit.getMatType() == TSMaterial::MatNull) {
		lightingType = TSMaterial::LightingUnlit;
	} else {
		lightingType = mlEdit.getMatLighting();
	}
	
	int response = palDlg.DoModal(&palIndex, &lightingType);

	if (response == IDOK) {
		mlEdit.setMatToPalette(lightingType, palIndex);
		UpdateListMember(currSel);
		m_matList.SetCurSel(currSel);
	}
}

void CMatilda2Dlg::OnEditasTexture() 
{
	TEXEditDlg texDlg;
	
	CString						 buffer;
	TSMaterial::LightingType	 lightingType;
	UInt32						 flags = 0;

	int currSel = m_matList.GetCurSel();
	if (currSel == LB_ERR || (mlEdit.isMaterialListValid() != TRUE)) {
		return;
	}

	mlEdit.fSetCurrIndex(currSel);
	
	if (mlEdit.getMatType() == TSMaterial::MatTexture) {
		flags  = mlEdit.getTextureFlags();
		buffer = mlEdit.getBitMapName();
	}
	if (mlEdit.getMatType() == TSMaterial::MatNull) {
		lightingType = TSMaterial::LightingUnlit;
	} else {
		lightingType = mlEdit.getMatLighting();
	}
	
	int result = texDlg.DoModal(buffer, &lightingType, &flags);

	if (result == IDOK) {
		mlEdit.setMatToTexture(lightingType, flags);
		mlEdit.setBitMapName(buffer);
		UpdateListMember(currSel);
		m_matList.SetCurSel(currSel);
	}
}


void CMatilda2Dlg::OnEditasNull() 
{
	int currSel = m_matList.GetCurSel();

	if (currSel == LB_ERR || mlEdit.isMaterialListValid() != TRUE) {
		return;
	}
	
	mlEdit.fSetCurrIndex(currSel);
	mlEdit.setMatToNull();
	UpdateListMember(currSel);
	m_matList.SetCurSel(currSel);
}


void CMatilda2Dlg::UpdateList()
{
	if (mlEdit.isMaterialListValid() == FALSE) {
		return;
	}

	AssertFatal(m_currDetailLevel >= 0 &&
				m_currDetailLevel < mlEdit.getNumDetailLevels(),
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
	CFileDialog fileDialog(TRUE, 
						 NULL, 
						 NULL,
						 OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
                         "Material List (*.DML)|*.DML|All files (*.*)|*.*||");
	int result = fileDialog.DoModal();
	if (result == IDCANCEL) {
		return;
	}

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

void CMatilda2Dlg::OnFileExportToText() 
{
	const char* szDefaultFile;
	static const char achDefaultExt[] = "*.dmt";
	static const char achDefFileType[] = "dmt";

	// if we don't have a good material list, bail now
	if( mlEdit.isMaterialListValid() == false )
		return;

	// use a default extension
	szDefaultFile = achDefaultExt;
	CFileDialog fileDlg(FALSE, 
						szDefaultFile, 
						NULL,
                        OFN_OVERWRITEPROMPT,
	                    "Material Text (*.DMT)|*.DMT|All files (*.*)|*.*||");

	int result = fileDlg.DoModal();
	if (result == IDCANCEL) {
		return;
	}

	char fileName[512];
	strcpy(fileName, fileDlg.GetPathName());

	FileWStream fws(fileName);
	if (fws.getStatus() != STRM_OK) {
		AfxMessageBox("Error opening file...");
		return;
	}

	int saveCurrMaterial = mlEdit.iGetCurrIndex();
	int saveCurrDetail   = mlEdit.getDetailLevel();
	for (int i = 0; i < mlEdit.iGetListSize(); i++) {
		mlEdit.fSetCurrIndex(i);

		CString str;
		str.Format("%4d ", i);
		fws.write(strlen(str), str);

		for (int j = 0; j < mlEdit.getNumDetailLevels(); j++) {
			mlEdit.setDetailLevel(j);
			TSMaterial *pMat = mlEdit.getCurrMat();
			if (pMat->fType == TSMaterial::MatTexture) {
				str.Format("Texture %-13s %c%c%c%c%c%c%c ", pMat->fMapFile,
					(pMat->fTextureType & TSMaterial::TextureFlipX) ? '1' : '0',
					(pMat->fTextureType & TSMaterial::TextureFlipY) ? '1' : '0',
					(pMat->fTextureType & TSMaterial::TextureRotate) ? '1' : '0',
					(pMat->fTextureType & TSMaterial::TextureTransparent) ? '1' : '0',
					(pMat->fTextureType & TSMaterial::TextureTranslucent) ? '1' : '0',
					(pMat->fTextureType & TSMaterial::TexturePerspective) ? '1' : '0',
					((char)(pMat->fLighting)) + '0');
			} else {
				str.Format("NonTexture  <DO_NOT_EDIT>     ");
			}
			fws.write(strlen(str), str);
		}

		fws.write(strlen("\n"), "\n");
	}
	mlEdit.fSetCurrIndex(saveCurrMaterial);
	mlEdit.setDetailLevel(saveCurrDetail);
}


void CMatilda2Dlg::OnFileImportFromText() 
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
	CFileDialog fileDialog(TRUE, 
						 NULL, 
						 NULL,
						 OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
                         "Material Text (*.DMT)|*.DMT|All files (*.*)|*.*||");
	int result = fileDialog.DoModal();
	if (result == IDCANCEL) {
		return;
	}

	FILE *inFile = fopen(fileDialog.GetPathName(), "r");
	if (!inFile) {
		AfxMessageBox("Error opening file...");
		return;
	}

	int saveCurrMaterial = mlEdit.iGetCurrIndex();
	int saveCurrDetail   = mlEdit.getDetailLevel();

	char buff[256];
	char buff2[256];
	int  matIndex;

	for (int mat = 0; mat < mlEdit.iGetListSize(); mat++) {
		fscanf(inFile, "%d", &matIndex);
		
		if (matIndex != mat) {
			AssertFatal(0, "Error, invalid .dmt file!");
		}

		if (matIndex < 0 || matIndex >= mlEdit.iGetListSize()) {
			AssertFatal(0, "Error, invalid .dmt file");
			exit(-1);
		}

		mlEdit.fSetCurrIndex(matIndex);

		for (int i = 0; i < mlEdit.getNumDetailLevels(); i++) {
			mlEdit.setDetailLevel(i);
			char setFlipX;
			char setFlipY;
			char setRotate;
			char setTransparent;
			char setTranslucent;
			char setPerspective;
			char bogusLighting;

			fscanf(inFile, "%s %s ", buff, buff2);

			if (!strcmp("Texture", buff)) {
				// this is pretty darn bogus, but it works...
				//
				fscanf(inFile, "%c%c%c%c%c%c%c", &setFlipX, &setFlipY, &setRotate,
				&setTransparent, &setTranslucent, &setPerspective, &bogusLighting);

				TSMaterial *pMat = mlEdit.getCurrMat();
				AssertFatal(pMat->fType == TSMaterial::MatTexture,
					"Error, attempted to load a texture into a non-texture type");
				
				TSMaterial::LightingType	 lightingType;
				UInt32						 flags = 0;
				flags = ((setFlipX == '1') ? TSMaterial::TextureFlipX : 0) |
						((setFlipY == '1') ? TSMaterial::TextureFlipY : 0) |
						((setRotate == '1') ? TSMaterial::TextureRotate : 0) |
						((setTransparent == '1') ? TSMaterial::TextureTransparent : 0) |
						((setTranslucent == '1') ? TSMaterial::TextureTranslucent : 0) |
						((setPerspective == '1') ? TSMaterial::TexturePerspective : 0);
				lightingType = TSMaterial::LightingType(bogusLighting - '0');
	
				mlEdit.setMatToTexture(lightingType, flags);
				mlEdit.setBitMapName(buff2);
				UpdateListMember(matIndex);
			} else {
				// do nothing if this is not a texture.
			}
		}
		fscanf(inFile, "\n", buff);
	}
	mlEdit.fSetCurrIndex(saveCurrMaterial);
	mlEdit.setDetailLevel(saveCurrDetail);
	mlEdit.touchSaveAdvised();

	fclose(inFile);

	UpdateList();
}



void CMatilda2Dlg::OnDblclkMaterialList() 
{
	int editMat = m_matList.GetCurSel();

	if (editMat == LB_ERR) {
		return;
	}

	if (mlEdit.isMaterialListValid()) {
		TSMaterial *matToEdit;
		
		mlEdit.fSetCurrIndex(editMat);

		matToEdit = mlEdit.getCurrMat();

		TSMaterial::LightingType	lightType	= matToEdit->fLighting;
		TSMaterial::MatType			matType		= matToEdit->fType;
		UInt32						palIndex	= (UInt32)matToEdit->fIndex;
		UInt32						texFlags	= matToEdit->fFlags;
		WORD						red			= matToEdit->fRGB.fRed;
		WORD						green		= matToEdit->fRGB.fGreen;
		WORD						blue		= matToEdit->fRGB.fBlue;
		CString						texName		= mlEdit.getBitMapName();

		RGBEditDlg	rgbDlg;
		palEditDlg palDlg;
		TEXEditDlg texDlg;

		switch (matType) {
		case TSMaterial::MatNull:
			return;
			break;

		case TSMaterial::MatRGB:
			
			if (rgbDlg.DoModal(&red, &green, &blue, &lightType) == IDOK) {
				mlEdit.setMatToRGB(lightType, red, green, blue);
				UpdateListMember(editMat);
			}
			break;

		case TSMaterial::MatPalette:
			if (palDlg.DoModal(&palIndex, &lightType) == IDOK) {
				mlEdit.setMatToPalette(lightType, palIndex);
				UpdateListMember(editMat);
			}
			break;

		case TSMaterial::MatTexture:
			if (texDlg.DoModal(texName, &lightType, &texFlags) == IDOK) {
				mlEdit.setMatToTexture(lightType, texFlags);
				mlEdit.setBitMapName(texName);
				UpdateListMember(editMat);
			}
			break;
		}
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
	m_preview.Invalidate();
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
		GetSystemPaletteEntries(dc, 0, 255, mlEdit.pGFXPal->color);

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
			HKEY	regKey;
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

			GetDlgItem(ID_CURRENTPALETTE)->SetWindowText(in_palName);
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
	return realizeSysPalette( dc );
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

	m_copyBuffer = new TSMaterial*[mlEdit.getNumDetailLevels()];
	for (int j = 0; j < mlEdit.getNumDetailLevels(); j++) {
		m_copyBuffer[j] = new TSMaterial[numMarked];
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
	TSMaterial::LightingType lightType;
	
	if (changeLight.DoModal(&lightType) == IDCANCEL)
		return;

	for (int i = 0; i < size; i++) {
		if (m_matList.GetSel(i) != 0) {
			TSMaterial *temp = NULL;
			mlEdit.fSetCurrIndex(i);
			temp = mlEdit.getCurrMat();
			temp->fLighting = lightType;
			mlEdit.copyToCurrIndex(temp);
		}
	}
	UpdateList();
	mlEdit.fSetCurrIndex(origIndex);
}
