// TEXEditDlg.cpp : implementation file
//

#include "stdafx.h"
#include "matilda2.h"
#include "TEXEditDlg.h"
#include "palhelp.h"
#include "gfxres.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// TEXEditDlg dialog


TEXEditDlg::TEXEditDlg(CWnd* pParent /*=NULL*/)
	: CDialog(TEXEditDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(TEXEditDlg)
	//}}AFX_DATA_INIT
	m_previewIsValid = false;
}


void TEXEditDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(TEXEditDlg)
	DDX_Control(pDX, ID_TEX_PERSPECTIVE, m_perspective);
	DDX_Control(pDX, ID_TEX_TRANSPARENT, m_transparent);
	DDX_Control(pDX, ID_TEX_TRANSLUCENT, m_translucent);
	DDX_Control(pDX, ID_TEX_ROTATE, m_rotate);
	DDX_Control(pDX, ID_TEX_HAZED, m_hazed);
	DDX_Control(pDX, ID_TEX_FLIPY, m_flipY);
	DDX_Control(pDX, ID_TEX_FLIPX, m_flipX);
		CommonPropertyDataExchange ( pDX, 0 );
	DDX_Control(pDX, ID_TEX_FILENAME, m_fileNameEdit);
	DDX_Control(pDX, ID_TEX_RADIOUNLIT, m_lightingUnlit);
	DDX_Control(pDX, ID_TEX_RADIOFLAT, m_lightingFlat);
	DDX_Control(pDX, ID_TEX_RADIOSMOOTH, m_lightingSmooth);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(TEXEditDlg, CDialog)
	//{{AFX_MSG_MAP(TEXEditDlg)
	ON_BN_CLICKED(ID_TEX_RADIOFLAT, OnTexRadioflat)
	ON_BN_CLICKED(ID_TEX_RADIOSMOOTH, OnTexRadiosmooth)
	ON_BN_CLICKED(ID_TEX_RADIOUNLIT, OnTexRadiounlit)
	ON_BN_CLICKED(ID_TEX_SELECTFILE, OnTexSelectfile)
	ON_BN_CLICKED(ID_TEX_FLIPX, OnTexFlipx)
	ON_BN_CLICKED(ID_TEX_FLIPY, OnTexFlipy)
		CommonPropertyMessageMap ( 0 )
	ON_BN_CLICKED(ID_TEX_PERSPECTIVE, OnTexPerspective)
	ON_BN_CLICKED(ID_TEX_ROTATE, OnTexRotate)
	ON_BN_CLICKED(ID_TEX_TRANSLUCENT, OnTexTranslucent)
	ON_BN_CLICKED(ID_TEX_TRANSPARENT, OnTexTransparent)
	ON_WM_PALETTECHANGED()
	ON_WM_QUERYNEWPALETTE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

int
TEXEditDlg::DoModal(CString &io_fileName, TS::Material::ShadingType *io_lightingType,
					UInt32 *io_flags )
{
	m_lightingType = *io_lightingType;
	m_fileName     =  io_fileName;
	m_texType      = *io_flags;
	
	// set members... 
	m_tempMaterial.fParams.fFlags = m_tempMaterial.fParams.fFlags & ~(TS::Material::MatFlags)
		|TS::Material::MatTexture;

	m_tempMaterial.fParams.fFlags = m_tempMaterial.fParams.fFlags & ~(TS::Material::TextureFlags)
		|(enum TS::Material::TextureType)(*io_flags);

	m_tempMaterial.fParams.fFlags = m_tempMaterial.fParams.fFlags & ~(TS::Material::ShadingFlags)
		|*io_lightingType;

	//m_tempMaterial.unlock();
	strncpy(m_tempMaterial.fParams.fMapFile, io_fileName, TS::Material::MapFilenameMax);
	//m_tempMaterial.lock(rm,1);

	if (strlen(m_tempMaterial.fParams.fMapFile) != 0) {
		m_previewIsValid = true;
	} else {
		m_previewIsValid = false;
	}
		
		/*GFXBitmap* bitmap = GFXBitmap::load(m_tempMaterial.fParams.fMapFile,0);
		if(!bitmap) {
			m_previewIsValid = false;
		} else {
			//TSTextureManager::registerBmp(m_tempMaterial.fMapFile,bitmap);
			m_previewIsValid = true;
		}
	}*/

	int returnVal = CDialog::DoModal();

	 io_fileName     = m_fileName;
	*io_lightingType = m_lightingType;
	*io_flags        = m_texType;
	
	return returnVal;
}


/////////////////////////////////////////////////////////////////////////////
// TEXEditDlg message handlers

void TEXEditDlg::OnTexRadiosmooth() 
{
	m_lightingFlat.SetCheck(0);
	m_lightingUnlit.SetCheck(0);
	m_lightingType = TS::Material::ShadingSmooth;

	m_tempMaterial.fParams.fFlags = m_tempMaterial.fParams.fFlags & (TS::Material::ShadingFlags ^ 0xffff)
		|m_lightingType;
	if (m_previewIsValid == true) {
		m_preview.installMaterial(&m_tempMaterial, true);
		Invalidate();
	}
}

void TEXEditDlg::OnTexRadioflat() 
{
	m_lightingSmooth.SetCheck(0);
	m_lightingUnlit.SetCheck(0);
	m_lightingType = TS::Material::ShadingFlat;
	
	m_tempMaterial.fParams.fFlags = m_tempMaterial.fParams.fFlags & (TS::Material::ShadingFlags ^ 0xffff)
		|m_lightingType;
	if (m_previewIsValid == true) {
		m_preview.installMaterial(&m_tempMaterial, true);
		Invalidate();
	}
}

void TEXEditDlg::OnTexRadiounlit() 
{
	m_lightingSmooth.SetCheck(0);
	m_lightingFlat.SetCheck(0);
	m_lightingType = TS::Material::ShadingNone;

	m_tempMaterial.fParams.fFlags = m_tempMaterial.fParams.fFlags & (TS::Material::ShadingFlags ^ 0xffff)
		|m_lightingType;
	if (m_previewIsValid == true) {
		m_preview.installMaterial(&m_tempMaterial, true);
		Invalidate();
	}
}


BOOL TEXEditDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
   InitCommonProps();

	switch (m_lightingType) {
	case TS::Material::ShadingSmooth:
		m_lightingSmooth.SetCheck(1);
		break;

	case TS::Material::ShadingFlat:
		m_lightingFlat.SetCheck(1);
		break;
	
	case TS::Material::ShadingNone:
		m_lightingUnlit.SetCheck(1);
		break;
	}
	
	if (m_texType & TS::Material::TextureTransparent)
		m_transparent.SetCheck(1);
	
	if (m_texType & TS::Material::TextureTranslucent)
		m_translucent.SetCheck(1);

/*
	if (m_texType & TS::Material::TexturePerspective)
		m_perspective.SetCheck(1);

	if (m_texType & TS::Material::TextureFlipX)
		m_flipX.SetCheck(1);

	if (m_texType & TS::Material::TextureFlipY)
		m_flipY.SetCheck(1);

	if (m_texType & TS::Material::TextureRotate)
		m_rotate.SetCheck(1);
*/

	m_fileNameEdit.SetWindowText(m_fileName);

	CRect rcCanvas;
	CWnd* pCanvas = GetDlgItem(ID_TEX_PREVIEW);
	pCanvas->GetWindowRect(rcCanvas);
	ScreenToClient(rcCanvas);
	m_preview.Create(this, rcCanvas, ID_TEX_PREVIEW);

	if (m_previewIsValid == true) {
		m_preview.installMaterial(&m_tempMaterial, true);	//zmod true
		Invalidate();
	}


	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void TEXEditDlg::OnTexSelectfile() 
{
	char filterBuffer[] = "Windows BMPs\0*.BMP;*.DIB\0All Files\0*.*\0";
	char fileNameBuffer[512] = {'\0'};
	char tempPath[512];

	CFileDialog fileDlg(TRUE,NULL,NULL,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_FILEMUSTEXIST,NULL,this);
	fileDlg.m_ofn.lpstrFilter = filterBuffer;
	fileDlg.m_ofn.lpstrFile   = fileNameBuffer;
	fileDlg.m_ofn.nMaxFile    = 512;
	
	int result = fileDlg.DoModal();

	if ((result == IDOK) && (fileNameBuffer != "")) {
		m_fileName = fileNameBuffer + fileDlg.m_ofn.nFileOffset;
		m_fileNameEdit.SetWindowText(m_fileName);


		if (!pSearchPath.IsEmpty()) pSearchPath += ';';
		strncpy (tempPath, fileNameBuffer, fileDlg.m_ofn.nFileOffset);
		pSearchPath += tempPath;
		vrStream.setSearchPath((char *)(LPCTSTR)pSearchPath);


		m_tempMaterial.unload();
    	strncpy(m_tempMaterial.fParams.fMapFile, m_fileName, TS::Material::MapFilenameMax);
		if (m_tempMaterial.fParams.fMapFile[0] != '\0') m_tempMaterial.load(rm,1);
		
		if (strlen(m_tempMaterial.fParams.fMapFile) != 0) {
			m_previewIsValid = true;
			m_preview.installMaterial(&m_tempMaterial, true);
			Invalidate();
		}

	}
}

void TEXEditDlg::OnTexFlipx() 
{
/* //z no flipped textures
	if (m_flipX.GetCheck() == 1) {
		m_texType |= TS::Material::TextureFlipX;
		m_tempMaterial.fFlags = m_texType;
		m_tempMaterial.fTextureType = (TS::Material::TextureType)m_texType;
	} else {
		m_texType &= ~(TS::Material::TextureFlipX);
		m_tempMaterial.fFlags = m_texType;
		m_tempMaterial.fTextureType = (TS::Material::TextureType)m_texType;
	}
	if (m_previewIsValid == true) {
		m_preview.installMaterial(&m_tempMaterial, true);
		Invalidate();
	}
*/
}

void TEXEditDlg::OnTexFlipy() 
{
/* //z no flipped textures
	if (m_flipY.GetCheck() == 1) {
		m_texType |= TS::Material::TextureFlipY;
		m_tempMaterial.fFlags = m_texType;
		m_tempMaterial.fTextureType = (TS::Material::TextureType)m_texType;
	} else {
		m_texType &= ~(TS::Material::TextureFlipY);
		m_tempMaterial.fFlags = m_texType;
		m_tempMaterial.fTextureType = (TS::Material::TextureType)m_texType;
	}
	if (m_previewIsValid == true) {
		m_preview.installMaterial(&m_tempMaterial, true);
		Invalidate();
	}
*/
}

void TEXEditDlg::OnTexPerspective() 
{
/* //z no perspective texture
	if (m_perspective.GetCheck() == 1) {
		m_texType |= TS::Material::TexturePerspective;
		m_tempMaterial.fFlags = m_texType;
		m_tempMaterial.fTextureType = (TS::Material::TextureType)m_texType;
	} else {
		m_texType &= ~(TS::Material::TexturePerspective);
		m_tempMaterial.fFlags = m_texType;
		m_tempMaterial.fTextureType = (TS::Material::TextureType)m_texType;
	}
*/
}

void TEXEditDlg::OnTexRotate() 
{
/* //z no rotated textures
	if (m_rotate.GetCheck() == 1) {
		m_texType |= TS::Material::TextureRotate;
		m_tempMaterial.fFlags = m_texType;
		m_tempMaterial.fTextureType = (TS::Material::TextureType)m_texType;
	} else {
		m_texType &= ~(TS::Material::TextureRotate);
		m_tempMaterial.fFlags = m_texType;
		m_tempMaterial.fTextureType = (TS::Material::TextureType)m_texType;
	}
	if (m_previewIsValid == true) {
		m_preview.installMaterial(&m_tempMaterial, true);
		Invalidate();
	}
*/
}

void TEXEditDlg::OnTexTranslucent() 
{
	if (m_translucent.GetCheck() == 1) {
		m_texType |= TS::Material::TextureTranslucent;
		m_tempMaterial.fParams.fFlags &= ~(TS::Material::TextureFlags);
		m_tempMaterial.fParams.fFlags |= m_texType;

	//zz ftexturetype na	m_tempMaterial.fTextureType = (TS::Material::TextureType)m_texType;

	} else {
		m_texType &= ~(TS::Material::TextureTranslucent);
		m_tempMaterial.fParams.fFlags &= ~(TS::Material::TextureFlags);
		m_tempMaterial.fParams.fFlags |= m_texType;
	//z ftexturetype na	m_tempMaterial.fTextureType = (TS::Material::TextureType)m_texType;
	}
}

void TEXEditDlg::OnTexTransparent() 
{
	if (m_transparent.GetCheck() == 1) {
		m_texType |= TS::Material::TextureTransparent;
		m_tempMaterial.fParams.fFlags &= ~(TS::Material::TextureFlags);
		m_tempMaterial.fParams.fFlags |= m_texType;
//z ftexturetype na		m_tempMaterial.fTextureType = (TS::Material::TextureType)m_texType;

	} else {
		m_texType &= ~(TS::Material::TextureTransparent);
		m_tempMaterial.fParams.fFlags &= ~(TS::Material::TextureFlags);
		m_tempMaterial.fParams.fFlags |= m_texType;
//z ftexturetype na		m_tempMaterial.fTextureType = (TS::Material::TextureType)m_texType;
	}
}

BOOL TEXEditDlg::OnQueryNewPalette() 
{
//	CClientDC dc(this);	
	return FALSE;
}

void TEXEditDlg::OnPaletteChanged(CWnd* pFocusWnd) 
{
	if (pFocusWnd == this || IsChild(pFocusWnd))
		return;

	OnQueryNewPalette();
}



CommonPropertyMethodsDefined(TEXEditDlg)

