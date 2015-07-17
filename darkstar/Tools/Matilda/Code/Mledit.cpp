// File: mledit.cpp
// Program: Matilda -- a material list editor
// Authors: John Leavens & Steve Nichols
// History:
//	11/18/95 -- code is checkpointed at external version 1.00;
//		internal SourceSafe version is 2.05
// MLEdit.cpp
// implementation of the MaterialListEdit class, which layers on top
// of TSMaterialList and TSMaterial
//
// version 0.1 -- Thur 11/9/95, Steve Nichols
// version 0.2 -- Mon 11/13, changed name to MLEdit.cpp
// version 0.3 -- Wed 11/15, changes to start WORDegrating Dynamix code
// version 0.4 -- Thur 11/16, added the save/load code

// IDE expects this...
// sn -- for debug version, take this out: 
#include "stdafx.h"

// #if !defined(NO_DYNAMIX)
//	#define NO_DYNAMIX
// #endif

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <g_pal.h>

// special includes for the debug version
#include <stdlib.h>

#include "MLEdit.h"

// headers to do the file i/o 
#include <filstrm.h>
//#include <classio.h>

//#include <ts_init.h>

#include "matsuprt.h"
// support for GFX and TS materials drawing
//#include "mcanvs.h"

// special local definitions for testbed
static char* _apchMaterialNames[] = {"Null", "Palette", "RGB", "Texture"};
static char* _apchLightingNames[] = {"None", "Flat", "_dummy", "Smooth"};
static char* _apchTextureNames[] = {"Transparent", "Translucent", "Perspective", "FlipX", "FlipY", "Rotate"};

// local defs for randomize stuff
static char* _apchBitmapNames[] = {"a", "b", "c", "d", "e"};


// Global instance of the MaterialList class...
//
MaterialListEdit mlEdit;
extern ResourceManager rm;
extern CString pSearchPath;

// function: MaterialListEdit
// description: 
// input: 
// output: 
// returns: 
MaterialListEdit::MaterialListEdit()
{
	// list is not valid
	m_isListValid = false;

	/// save is not advised yet
	m_isSaveAdvised = false;

	// no palette yet
	pGFXPal = NULL;

	// nulls for the material list pointers
	m_pmlCurr = NULL;
	m_pmlOld = NULL;

	// undo operation is not available
	m_isUndoAvail = false;

	// offset is meaningless
	m_iCurrOffset = 0;

	// set the list sizes
	m_iCurrSize = 0;
	m_iOldSize = 0;

	// initialize current filename to empty string
	m_achListFileName[0] = '\0';

	// initialize current pathname to empty string
	m_achListPathName[0] = '\0';

	// initialize current material name to empty string
	m_achMatName[0] = '\0';
}


// function: ~MaterialListEdit()
// description:  destructor for a material list
// input: 
// output: 
// returns: 
MaterialListEdit::~MaterialListEdit()
{
	// deallocate material lists
	if( m_pmlCurr != NULL ) {
		delete m_pmlCurr; m_pmlCurr=NULL;
	}

	if( m_pmlOld != NULL ) {
		delete m_pmlOld; m_pmlOld=NULL;
	}

	if (pGFXPal != NULL) {
		delete pGFXPal;	pGFXPal = NULL;
	}
}


// function: MaterialListEdit -- let the compiler do this one for now
// description: 
// input: 
// output: 
// returns: 
// MaterialListEdit::MaterialListEdit(const MaterialListEdit& mleOld)
// {
// }


// function: operator=
// description: 
// input: 
// output: 
// returns: 
/*** This is problematic w/o an assignment operator for the TSMaterialList class
MaterialListEdit& MaterialListEdit::operator=(const	MaterialListEdit& rhs)
{
	if( this == &rhs )
		return(*this);

	// assign to all data members
	m_isListValid = rhs.m_isListValid;
	m_isSaveAdvised = rhs.m_isSaveAdvised;
	m_mlCurrList = rhs.m_mlCurrList;
	m_mlOldList = rhs.m_mlOldList;
	m_iCurrSize = rhs.m_iCurrSize;
	m_iOldSize = rhs.m_iOldSize;
	m_isUndoAvail = rhs.m_isUndoAvail;
	m_iCurrOffset = rhs.m_iCurrOffset;
	strcpy(m_achListFileName, rhs.m_achListFileName);
	strcpy(m_achListPathName, rhs.m_achListPathName);
	strcpy(m_achMatName, rhs.m_achMatName);

	return(*this);
}
***/

// function: fSetMatListFileName
// description: Set the current material list file name
// input: 
//		szMatFile = new file name
//		szMatPath = full path name
// output: 
// returns: 
//		true -- if successful
//		false -- if not successful (currently not implemented)
bool MaterialListEdit::fSetMatListFileName(const char* szMatFile, const char* szMatPath)
{
	// verify arg
	AssertFatal(szMatFile != NULL, "NULL material filename");
	AssertFatal(szMatPath != NULL, "NULL material pathname");

	// set the Material list name
	strncpy(m_achListFileName, szMatFile, sizeof(m_achListFileName)-1);
	m_achListFileName[ sizeof(m_achListFileName)-1 ] = '\0';

	// set the Material list name
	strncpy(m_achListPathName, szMatPath, sizeof(m_achListPathName)-1);
	m_achListPathName[ sizeof(m_achListPathName)-1 ] = '\0';

	return(true);
}


// function: szGetMatListFileName
// description: returns a pointer to the current material list name
// input: 
// output: 
// returns:
//		pointer to current material list name (if valid)
//		NULL is current material list name is not defined 
const char* MaterialListEdit::szGetMatListFileName() const
{
	return(m_achListFileName);
}

// function: szGetMatListPathName
// description: returns a pointer to the current material list path
// input: 
// output: 
// returns:
//		pointer to current material list name (if valid)
//		NULL is current material list name is not defined 
const char* MaterialListEdit::szGetMatListPathName() const
{
	return(m_achListPathName);
}

// function: szGetBitmapPathName
// description: returns a pointer to the current material bitmap path
// input: 
// output: 
// returns:
//		pointer to current path name (if valid)
//		NULL is current path name is not defined 
const char* MaterialListEdit::szGetBitmapPathName() const
{
	return(m_achBitmapPathName);
}

// function: isMatListFileNameGood
// description: do we have a good filename for material list?
// input: 
// output: 
// returns:
//		pointer to current material list name (if valid)
//		NULL is current material list name is not defined 
bool MaterialListEdit::isMatListFileNameGood() const
{
	if( strlen(m_achListFileName) > 0 && strlen(m_achListPathName) > 0 )
		return(true);
	else
		return(false);
}



// function: fReadMaterialList()
// description: 
// input: 
// output: 
// returns: 
bool MaterialListEdit::fReadMaterialList()
{
	// assume we will not succeed 
	bool isReadSuccessful = false;

	afxDump << "MaterialListEdit::fReadMaterialList() --loading from "
										 << szGetMatListPathName() << "\n";

	// see if we have a filename
	if( isMatListFileNameGood() == true ) {
		// load a material list..
		FileRStream istMatFile;

		// open the file
		istMatFile.open(szGetMatListPathName());

		// if we were successful...
		if( istMatFile.getStatus() == STRM_OK ) {
			// copy material list to old
			if( m_pmlOld != NULL ) {
				delete m_pmlOld; m_pmlOld=NULL;
			}
			m_pmlOld = m_pmlCurr;
			m_iOldSize = m_iCurrSize;

			Persistent::Base::Error  error;

			// load material list...
			m_pmlCurr= (TS::MaterialList *)Persistent::Base::load(istMatFile,&error);


			// close the file
			istMatFile.close();

			//lock all the materials
/*			int iEnd = m_pmlCurr->getMaterialsCount();
			for (int i=0;i < iEnd; i++) 
				m_pmlCurr->getMaterial(i).lock(rm,1);*/

			// if we got a material list
			if( m_pmlCurr != NULL ) {
				// point at detail level 0...
				m_pmlCurr->setDetailLevel(0);

				// update our member variables
				m_isListValid = true;
				m_iCurrOffset = 0;
				m_isSaveAdvised = false;
				m_iCurrSize = m_pmlCurr->getMaterialsCount();

/*				// pass path to TS for bitmaps
				const char* path = szGetMatListPathName();
				char *endCh = strrchr(path, '\\');
				char newPath[256];
				strncpy(newPath, path, endCh - path);
				strncpy(newPath + (endCh - path), "\0", 2);

//function na				TSSetSearchPath(newPath);
				strcpy(m_achBitmapPathName, newPath);
				//m_pmlCurr->lock(pal);
*/
				// we succeeded in read...
				afxDump << "--> load was successful\n";
				isReadSuccessful = true;
			}
			// back out of prev version
			else {
				afxDump << "--> couldn't load file\n";

				// try to recover old list if we can
				if( m_pmlOld != NULL ) {
					m_isUndoAvail = false;
					m_iCurrOffset = 0;
					m_pmlCurr = m_pmlOld;
					m_iCurrSize = m_iOldSize;
				}
				else {
					// make list invalid
					m_isListValid = false;
				}
			}
		}
		// open failed
		else { 
			afxDump << "--> open failed\n";
			isReadSuccessful = false;
		}
	}

	return(isReadSuccessful);
}


// function: fSaveMaterialList
// description: 
// input: 
// output: 
// returns: 
bool MaterialListEdit::fSaveMaterialList()
{
	// assume save will not be successful
	bool isSaveSuccessful = false;

	afxDump << "MaterialListEdit::fSaveMaterialList() -- saving to "
									<< szGetMatListPathName() << "\n";

	// see if we have a filename and a valid materials list
	if( isMatListFileNameGood() == true && isMaterialListValid() == true ) {
		// save a material list..
		FileWStream ostMatFile;

		// open the file
		ostMatFile.open(szGetMatListPathName());

		// if we were successful...
		if( ostMatFile.getStatus() == STRM_OK ) {

		// save material list ... no return value
		m_pmlCurr->store(ostMatFile,0);

			// close the stream
			ostMatFile.close();

			// we succeeded in write...
			afxDump << "--> read was successful\n";
			isSaveSuccessful = true;
			m_isSaveAdvised = false;
		}
		// open failed
		else { 
			// TODO -- pop up dialog for load error
			afxDump << "MaterialListEdit::fSaveMaterialList() -- open failed\n"; 	
			isSaveSuccessful = false;
		}
	}

	return(isSaveSuccessful);
}


// function: fDeleteEntry
// description: 
// input: 
// output: 
// returns: 
bool MaterialListEdit::fDeleteEntry(const Int32 iIndexToDelete)
{
	int ii;						// loop counter	for a list
	int iDetailIndex;			// loop counter for detail level...
	int iNewIndex;				// offset into new list
	int iPrevDetailLevel;		// remember the old detail level...
	// set the return flag to false
	bool isDeleteSuccessful = false;

	// if current list is valid, we can perform a delete
	if( isMaterialListValid() == true ) {
		// see if index is in range...
		if( iIndexToDelete < 0 || iIndexToDelete >= iGetListSize() )
			return(false);

		// save current list
		m_iOldSize = m_iCurrSize;
		if( m_pmlOld != NULL ) {
			delete m_pmlOld; m_pmlOld=NULL;
		}
		m_pmlOld = m_pmlCurr;
		assert(m_pmlOld != NULL);
		m_isUndoAvail = true;

		// remember our detail level
		iPrevDetailLevel = m_pmlOld->getDetailLevel();

		// allocate a material list one larger than old one
		if( m_iCurrSize > 1 )
			m_pmlCurr = new TS::MaterialList(m_iCurrSize-1, m_pmlOld->getDetailsCount() );
		else // to prevent an assertion failure on a 0 length list...
			m_pmlCurr = new TS::MaterialList(1, m_pmlOld->getDetailsCount() );

		m_iCurrSize--;

		// do a member-by-member copy, excluding index
		for( iDetailIndex=0;
					iDetailIndex < m_pmlOld->getDetailsCount();
							iDetailIndex++ ) {
			// set detail level on current and old lists to the same
			m_pmlCurr->setDetailLevel(iDetailIndex);
			m_pmlOld->setDetailLevel(iDetailIndex);

			iNewIndex = 0;
			for(ii=0; ii<m_iOldSize; ii++) {
				// only copy member if it is not one to delete
				if( ii != iIndexToDelete ) {
					(*m_pmlCurr)[iNewIndex] = m_pmlOld->getMaterial(ii);
					iNewIndex++;
				}
			}
		}

		// restore our old detail level
		m_pmlCurr->setDetailLevel(iPrevDetailLevel);
		m_pmlOld->setDetailLevel(iPrevDetailLevel);

		// point to new member at end
		if( iGetCurrIndex() > 0 )
			fSetCurrIndex(iGetCurrIndex() - 1);

		// if new list is non-empty
		if( iGetListSize() > 0 ) {
			// set is-list-valid field
			m_isListValid = true;
			m_isSaveAdvised = true;
		}
		// list is now empty
		else {
			m_isListValid = false;
			m_isSaveAdvised = false;
		}

		// delete succeeded
		isDeleteSuccessful = true;
	}
	// can't delete from an empty list
	else {
		isDeleteSuccessful = false;
	}

	return(isDeleteSuccessful);
}


// function: fAddEntryToEnd
// description: Add a null entry to end of list, increasing list size by one
// input: 
// output: 
// returns: 
bool MaterialListEdit::fAddEntryToEnd()
{
	TS::Material tsmNull;	   	// material to make a dummy param
	int ii;					// loop counter
	int iPrevDetailLevel;	// remember our previous detail level
	int iDetailIndex;		// index the detail level

	// if current list is valid, we can perform an undo
	if( isMaterialListValid() == true ) {
		// save current list
		m_iOldSize = m_iCurrSize;
		if( m_pmlOld != NULL ) {
			delete m_pmlOld; m_pmlOld=NULL;
		}
		m_pmlOld = m_pmlCurr;
		assert(m_pmlOld != NULL);
		m_isUndoAvail = true;

		// save our curent detail level
		iPrevDetailLevel = m_pmlOld->getDetailLevel();

		// allocate a material list one larger than old one
		m_pmlCurr = new TS::MaterialList(m_iCurrSize+1, m_pmlOld->getDetailsCount());
		m_iCurrSize++;

		// make a null member
		tsmNull.fParams.fFlags = (tsmNull.fParams.fFlags 
			& (TS::Material::ShadingFlags | TS::Material::TextureFlags ))
			| TS::Material::MatNull;

		// walk through all detail levels
		for(iDetailIndex=0;
					iDetailIndex < m_pmlOld->getDetailsCount();
							iDetailIndex++ ) {
			// set the current detail level
			m_pmlCurr->setDetailLevel(iDetailIndex);
			m_pmlOld->setDetailLevel(iDetailIndex);

			// do a member-by-member copy
			for(ii=0; ii<m_iOldSize; ii++) {
				(*m_pmlCurr)[ii] = m_pmlOld->getMaterial(ii);
			}

			// null out the last (new) member
			(*m_pmlCurr)[m_iOldSize] = tsmNull;
		}

		// restore the detail level
		m_pmlOld->setDetailLevel(iPrevDetailLevel);
		m_pmlCurr->setDetailLevel(iPrevDetailLevel);

		// point to new member at end
		m_iCurrOffset = m_iCurrSize - 1;

		// set is-list-valid field
		m_isListValid = true;

		// at this point, a save is advised
		m_isSaveAdvised = true;
	} // end if-list-is-valid

	return(true);
}

// function: fCreateNewList
// description: Create a new list w/ one null entry
// input: 
//		iNumDetails = the number of detail levels we will support (default=1)
// output: 
// returns: 
bool MaterialListEdit::fCreateNewList(int iNumDetails)
{
	TS::Material tsmNull;	   	// material to make a dummy param 
	int ii;					// counter for detail level

	// if current list is valid, we can perform an undo
	if( isMaterialListValid() == true ) {
		// save current list
		m_iOldSize = m_iCurrSize;
		if( m_pmlOld != NULL ) {
			delete m_pmlOld; m_pmlOld=NULL;
		}
		m_pmlOld = m_pmlCurr;

		m_isUndoAvail = true;
	}

	// allocate a material list of one 
	m_pmlCurr = new TS::MaterialList(1, iNumDetails);
	m_iCurrSize = 1;

	// point to new member at end
	m_iCurrOffset = 0;

	// create a dummy null member
	tsmNull.fParams.fFlags = (tsmNull.fParams.fFlags 
		& (TS::Material::ShadingFlags | TS::Material::TextureFlags))
		| TS::Material::MatNull;

	// null out each member
	for( ii=0; ii< iNumDetails; ii++ ) {
		m_pmlCurr->setDetailLevel(ii);
		(*m_pmlCurr)[m_iCurrOffset] = tsmNull;
	}

	// reset detail level to 0
	m_pmlCurr->setDetailLevel(0);

	// set is-list-valid field
	m_isListValid = true;

	// at this point, a save is advised
	m_isSaveAdvised = true;

	return(true);
}



bool MaterialListEdit::fUndoLastChange()
{
	return(true);
}


bool MaterialListEdit::fSetCurrIndex(Int32 iNewIndex)
{
	if( 0 <= iNewIndex && iNewIndex < m_iCurrSize ) {
		m_iCurrOffset = iNewIndex;
		return(true);
	}
	else 	// out of range
		return(false);
}


const char* MaterialListEdit::szNameOfMaterial(const Int32 iOffset)
{
	TS::Material tsmUnknown;

	// generate unique material name
	if( isMaterialListValid() == true ) {
		if( 0 <= iOffset && iOffset < iGetListSize() ) { 	
			// get the material from the list
			tsmUnknown = (*m_pmlCurr)[iOffset];

			// assert that we have a valid material type
			AssertFatal( ValidMaterial(tsmUnknown.fParams.fFlags & TS::Material::MatFlags), "invalid material type" );

			if( (tsmUnknown.fParams.fFlags &TS::Material::MatFlags) == TS::Material::MatNull ) {
				sprintf(m_achMatName, "%d %s", iOffset, _apchMaterialNames[(int)(tsmUnknown.fParams.fFlags &TS::Material::MatFlags) ] );
			}
			else if( (tsmUnknown.fParams.fFlags &TS::Material::MatFlags) == TS::Material::MatPalette ) { 
				// assert that we have valid lighting and palette index
				AssertFatal( ValidLighting(tsmUnknown.fParams.fFlags &TS::Material::ShadingFlags), "invalid lighting type" );
				AssertFatal( ValidPalette(tsmUnknown.fParams.fIndex), "invalid palette index" );

				sprintf(m_achMatName, "%d %s, %s (%d)", iOffset,
					_apchMaterialNames[(int)(tsmUnknown.fParams.fFlags &TS::Material::MatFlags) ],
					_apchLightingNames[(int)((tsmUnknown.fParams.fFlags &TS::Material::ShadingFlags) /256) ],
									tsmUnknown.fParams.fIndex );
			}

			else if( (tsmUnknown.fParams.fFlags &TS::Material::MatFlags) == TS::Material::MatTexture ) {
				// build up a string for the textures
				// TODO -- use a const in header for this size
				char achTextures[TMP_BSIZE];

				// assert that we have valid lighting and textures
				AssertFatal( ValidLighting(tsmUnknown.fParams.fFlags &TS::Material::ShadingFlags), "invalid lighting type" );
//z??				AssertFatal( ValidTexture(tsmUnknown.fParams.fFlags &TS::Material::TextureFlags), "invalid texture type" );

				// see about filename
				if( strlen(tsmUnknown.fParams.fMapFile) > 0 ) {
					strncpy(achTextures, tsmUnknown.fParams.fMapFile, sizeof(achTextures)-1);

					

/*					BOOL found=false;
					CString tempPath = pSearchPath;
					CString onePath; */
					CString theFile = tsmUnknown.fParams.fMapFile;
/*			CString oneFullPath;
					int foundindex;
					
					while (!found && !tempPath.IsEmpty()) {

						foundindex = tempPath.ReverseFind(';') + 1;
						onePath= tempPath.Mid(foundindex);
						tempPath= tempPath.Left(foundindex);
			
						//look for file here
						oneFullPath+= onePath+theFile;
						if (GetFileAttributes(oneFullPath) != -1) found= 1;
					} 

					if (!found) { 
						strcat (achTextures," *not in matlist dir*");
					
					}*/


					ResourceObject *obj= rm.load(theFile);
					if(!obj || !obj->resource) strcat (achTextures," *not in matlist dir*");




				}
				else
					strcat (achTextures," *not in matlist dir*");

				// build up whole string
				sprintf(m_achMatName, "%d %s, (%s)", iOffset,
					_apchMaterialNames[(int)(tsmUnknown.fParams.fFlags &TS::Material::MatFlags) ],
									achTextures );
			}
			else if( (tsmUnknown.fParams.fFlags &TS::Material::MatFlags) == TS::Material::MatRGB ) {
				// just check lighting
				AssertFatal( ValidLighting(tsmUnknown.fParams.fFlags &TS::Material::ShadingFlags), "invalid lighting type" );
			
				sprintf(m_achMatName, "%d %s (%d,%d,%d)", iOffset,
					_apchMaterialNames[(int)(tsmUnknown.fParams.fFlags &TS::Material::MatFlags) ],
								tsmUnknown.fParams.fRGB.fRed,
									tsmUnknown.fParams.fRGB.fGreen,
										tsmUnknown.fParams.fRGB.fBlue);
			}

 
			else {
				sprintf(m_achMatName, "%d (unknown?!)", iOffset);
			}
		}
		// else out of range index...
	}
	// no material list to index into
	else {
		m_achMatName[0] = '\0';
	}

	// return a pointer to the material name
	return( m_achMatName );
}

// function: setMatToNull
// description: change current entry to null -- does not operate across
// 		detail levels.

void MaterialListEdit::setMatToNull()
{
	TS::Material tsmNull;	   	// material to make a dummy param

	// afxDump << "MaterialListEdit::setMatToNull()\n";

	// if current list is valid, we can perform an undo
	if( isMaterialListValid() == true ) {
		// undo is not available...
		m_isUndoAvail = false;

		// null out the member
		tsmNull.fParams.fFlags &= (TS::Material::ShadingFlags | TS::Material::TextureFlags);
		tsmNull.fParams.fFlags |= TS::Material::TextureFlags;
		(*m_pmlCurr)[m_iCurrOffset] = tsmNull;

		// at this point, a save is advised
		m_isSaveAdvised = true;
	}
}

// function: setMatToTexture
// description: change current entry to null
 
void MaterialListEdit::setMatToTexture(TS::Material::ShadingType ltKind, int iTextureFlags)
{
	TS::Material tsmTexture;	   	// material to make a dummy param

	// if current list is valid, we can perform an undo
	if( isMaterialListValid() == true ) {
		// undo is not available
		m_isUndoAvail = false;

		// force the texture flags to be in range
		//iTextureFlags &= ~(TS::Material::MatFlags);
		//iTextureFlags |= TS::Material::MatTexture;

		// set members... 
		tsmTexture.fParams.fFlags &= ~(TS::Material::MatFlags);
		tsmTexture.fParams.fFlags |= TS::Material::MatTexture;

		tsmTexture.fParams.fFlags &= ~(TS::Material::TextureFlags);
		tsmTexture.fParams.fFlags |= (enum TS::Material::TextureType)iTextureFlags;

		tsmTexture.fParams.fFlags &= ~(TS::Material::ShadingFlags);
		tsmTexture.fParams.fFlags |= ltKind;

		tsmTexture.fParams.fMapFile[0] = '\0';

		// set the new material
		(*m_pmlCurr)[m_iCurrOffset] = tsmTexture;

		// at this point, a save is advised
		m_isSaveAdvised = true;
	}
}


/* 		setMatToCommonProps ( CommonDmlProperties )
		Apply the given common properties to the current material. 
			(LH 3/98)
*/
void MaterialListEdit::setMatToCommonProps ( const CommonDmlProperties & commProps )
{
	if( isMaterialListValid() == true   &&   m_pmlCurr != NULL )
	{
		m_isUndoAvail = false;

		TS::Material	fillIn = m_pmlCurr->getMaterial(m_iCurrOffset);

		// Puts them in, and returns true if a change happened.  
		if ( commProps.GetParamsFromProps ( fillIn.fParams ) )
		{
			(*m_pmlCurr)[m_iCurrOffset] = fillIn;
			m_isSaveAdvised = true;
		}
	}
}

bool MaterialListEdit::getCommonProps ( CommonDmlProperties & commProps )
{
	if ( isMaterialListValid() == true   &&   m_pmlCurr != NULL )
	{
		TS::Material::Params	p = m_pmlCurr->getMaterial(m_iCurrOffset).fParams;

		commProps.SetPropsToParams ( p );
		
		return true;
	}
	return false;
}



// function: getCurrMat
// description: Get material type (enum MatType) of current material
// input: 
// output: 
// returns: 
TS::Material* MaterialListEdit::getCurrMat()
{
	TS::Material	*pCurrent = NULL;

	// if current list is valid, get current material
	if( isMaterialListValid() == true ) 
	{
		if( m_pmlCurr != NULL )
		{
			pCurrent = new TS::Material;
			if ( pCurrent != NULL )
				*pCurrent = m_pmlCurr->getMaterial(m_iCurrOffset);
		}
	}
	return ( pCurrent );
}


// function: getMatType
// description: Get material type (enum MatType) of current material
// input: 
// output: 
// returns: 
TS::Material::MatType MaterialListEdit::getMatType() const
{
	TS::Material tsmUnknown;	   	// material to fill in

	// set default value to MatNull
	tsmUnknown.fParams.fFlags &= ~(TS::Material::MatFlags);
	tsmUnknown.fParams.fFlags |= TS::Material::MatNull;

	// if current list is valid, we can perform an undo
	if( isMaterialListValid() == true ) {
		// get the material
		if( m_pmlCurr != NULL )
		tsmUnknown = m_pmlCurr->getMaterial(m_iCurrOffset);
	}

	return (enum TS::Material::MatType)(tsmUnknown.fParams.fFlags &TS::Material::MatFlags );
}


// function: getMatLighting
// description: Get lighting type (enum LightingType) of current material
// 	default return is unlit
// input: 
// output: 
// returns: 
TS::Material::ShadingType MaterialListEdit::getMatLighting() const
{
	TS::Material tsmUnknown;	   	// material to fill in

	// default return is unlit
	tsmUnknown.fParams.fFlags &= ~(TS::Material::ShadingFlags);
	tsmUnknown.fParams.fFlags |= TS::Material::ShadingNone;

	// if current list is valid, we can perform an undo
	if( isMaterialListValid() == true ) {
		// get the material
		if( m_pmlCurr != NULL )
		tsmUnknown = m_pmlCurr->getMaterial(m_iCurrOffset);
	}

	return (enum TS::Material::ShadingType)(tsmUnknown.fParams.fFlags &TS::Material::ShadingFlags );
}

// function: getBitMapName
// description: Get a const pointer to the bitmap name.	 Warning, this is not
// a durable object
// input: 
// output: 
// returns: 
//		pointer to a bitmap filename
//		NULL if invalid list or type is not Texture.
char* MaterialListEdit::getBitMapName() const
{
	static TS::Material tsmUnknown;	   	// material to fill in
	char* szFileName;			// pointer at filename

	// return value...
	szFileName = NULL;

	// if current list is valid, 
	if( isMaterialListValid() == true ) {
		// get the material
		if( m_pmlCurr != NULL ) {
			tsmUnknown = m_pmlCurr->getMaterial(m_iCurrOffset);

			// if material type is Texture
			if( (tsmUnknown.fParams.fFlags &TS::Material::MatFlags) == TS::Material::MatTexture ) {
				szFileName = (char*)(&tsmUnknown.fParams.fMapFile[0]);
			}
		}
	}

	return szFileName;
}




// function: setBitMapName
// description: Set the bitmap name for a material
// input: 
//		const pointer to name
// output: 
// returns: none.
void MaterialListEdit::setBitMapName(const char* szNewName)
{
	TS::Material tsmUnknown;	   	// material to fill in

	// if current list is valid, 
	if( isMaterialListValid() == true && szNewName != NULL ) {
		// get the material
		if( m_pmlCurr != NULL ) {
			tsmUnknown = m_pmlCurr->getMaterial(m_iCurrOffset);

			// set the name
			// TODO -- portable way of getting max size?
			tsmUnknown.unload();
			strncpy( tsmUnknown.fParams.fMapFile, szNewName, TS::Material::MapFilenameMax );
			if (tsmUnknown.fParams.fMapFile[0] != '\0') tsmUnknown.load(rm,1);
			
/*			GFXBitmap* bitmap = GFXBitmap::load(szNewName,0);
			if(!bitmap) {
				AfxMessageBox("MaterialListEdit: Error loading bitmap file", MB_OK, 0);
				return;
			}
			//TSTextureManager::registerBmp(tsmUnknown.fMapFile,bitmap);
			//  TSTextureManager deletes the bitmap internally
			//
			//delete bitmap;
*/
			// set the material
			(*m_pmlCurr)[m_iCurrOffset] = tsmUnknown;
		}
	}
}

// function: setMatToPalette
// description: Set a material to palette type, with lighting and index
// input: 
//		ltKind = (enum) unlit, flat, or smooth
//		iPalIndex = index into palette
// output: 
// returns: none.
void MaterialListEdit::setMatToPalette(TS::Material::ShadingType ltKind, Int32 iPalIndex)
{
	TS::Material tsmPalette;	   	// material to make a dummy param

	// if current list is valid, we can perform an undo
	if( isMaterialListValid() == true ) {
		// undo is not available
		m_isUndoAvail = false;

		// set members... 
		tsmPalette.fParams.fFlags= tsmPalette.fParams.fFlags & (TS::Material::MatFlags ^ 0xffff)
			| TS::Material::MatPalette;
		
		tsmPalette.fParams.fFlags= tsmPalette.fParams.fFlags & (TS::Material::ShadingFlags ^ 0xffff)
			| ltKind;


		tsmPalette.fParams.fIndex = iPalIndex;

		// set the new material
		assert(m_pmlCurr != NULL);
		(*m_pmlCurr)[m_iCurrOffset] = tsmPalette;

		// at this point, a save is advised
		m_isSaveAdvised = true;
	}
}

// function: getPaletteIndex
// description: get a palette index
// input: 
// output: 
// returns: index to Palette, or -1 if not a valid reference
int MaterialListEdit::getPaletteIndex() const
{
	TS::Material tsmUnknown;	   	// material to fill in
	// init return value to failure
	int iPaletteIndex = -1;

	// if current list is valid
	if( isMaterialListValid() == true ) {
		// if we have a material of type palette
		if( getMatType() == TS::Material::MatPalette ) {
			// get the material
			tsmUnknown = m_pmlCurr->getMaterial(m_iCurrOffset);

			// return the palette index
			iPaletteIndex = tsmUnknown.fParams.fIndex;
		}
	}
	return(iPaletteIndex);
}

// function: getTextureFlags
// description: get the Or'ed together texture flags
// input: 
// output: 
// returns: texture flags, or 0 if not a valid reference 	
int MaterialListEdit::getTextureFlags() const
{
	TS::Material tsmUnknown;	   	// material to fill in
	// init return value to failure (or empty)
	int iTextureBits = 0;

	// if current list is valid
	if( isMaterialListValid() == true ) {
		// if we have a material of type palette
		if( getMatType() == TS::Material::MatTexture ) {
			// get the material
			tsmUnknown = m_pmlCurr->getMaterial(m_iCurrOffset);

			// return the palette index
			iTextureBits = tsmUnknown.fParams.fFlags & TS::Material::TextureFlags;
		}
	}
	return(iTextureBits);
}

// function: isDetailLevelInRange
// description: Check if a detail level is in the allowed range
// input: 
// output: 
// returns:true if detail level *is* in range,
//		false if not, or if list is invalid	
bool MaterialListEdit::isDetailLevelInRange(int iDetailLevel) const
{
	// assume we will fail
	bool isInRange = false;

	// if list is valid
	if( isMaterialListValid() == true ) {
		// if level between 0 (inclusive) and # of details levels in list
		if( 0 <= iDetailLevel && iDetailLevel < m_pmlCurr->getDetailsCount() )
			isInRange = true;
	}
	return(isInRange);
}

// function: setDetailLevel
// description: Sanity check wrapper for a TSMaterialList method
// input: 
//		iNewLevel = new detail level
// output: 
// returns: none.
void MaterialListEdit::setDetailLevel(const int iNewLevel)
{
	// if we have a valid list
	if( isMaterialListValid() == true ) {
		assert(m_pmlCurr != NULL);
		if(m_pmlCurr != NULL)
			m_pmlCurr->setDetailLevel(iNewLevel);
	}
}

// function: getDetailLevel
// description: Sanity check wrapper for a TSMaterialList method
// input: 
// output: 
// returns: none.
Int32 MaterialListEdit::getDetailLevel() const
{
	// default return is 0
	Int32 iCurrLevel = 0;

	// if we have a valid list
	if( isMaterialListValid() == true ) {
		assert(m_pmlCurr != NULL);
		if(m_pmlCurr != NULL)
			iCurrLevel = m_pmlCurr->getDetailLevel();
	}
	return(iCurrLevel);
}

// function: getNumDetailLevels
// description: Sanity check wrapper for a TSMaterialList method
// input: 
// output: 
// returns: none.
Int32 MaterialListEdit::getNumDetailLevels() const
{

	// default return is 0
	Int32 iCurrLevel = 0;

	// if we have a valid list
	if( isMaterialListValid() == true ) {
		assert(m_pmlCurr != NULL);
		if(m_pmlCurr != NULL)
			iCurrLevel = m_pmlCurr->getDetailsCount();
	}
	return(iCurrLevel);
}

// function: setMatToRGB
// description: set current material to type RGB
// input: 
// output: 
// returns: none.
void MaterialListEdit::setMatToRGB(TS::Material::ShadingType ltKind, const WORD wRed, const WORD wGreen, const WORD wBlue)
{
	TS::Material tsmRGB;	   	// material to fill in

	// if current list is valid
	if( isMaterialListValid() == true ) {
		// check range...
		if( isRGB(wRed) && isRGB(wGreen) && isRGB(wBlue) ) {
			// build up dummy value
			tsmRGB.fParams.fFlags = tsmRGB.fParams.fFlags &(TS::Material::MatFlags ^ 0xffff)
				|TS::Material::MatRGB;

			tsmRGB.fParams.fFlags = tsmRGB.fParams.fFlags &(TS::Material::ShadingFlags ^ 0xffff)
				|ltKind;
			tsmRGB.fParams.fRGB.fRed = (UInt8)wRed;
			tsmRGB.fParams.fRGB.fGreen = (UInt8)wGreen;
			tsmRGB.fParams.fRGB.fBlue = (UInt8)wBlue;

			// set the material
			(*m_pmlCurr)[m_iCurrOffset] = tsmRGB;
		}
	}
}

// function: getRGB
// description: get the RGB values, if material type is RGB.
// input: 
// output: 
// returns: true if successful (if material was rgb), false if not RGB 
//		(the values are unchanged)
bool MaterialListEdit::getRGB(WORD& wRed, WORD& wGreen, WORD& wBlue) const
{
	TS::Material tsmUnknown;	   	// material to fill in
	// assume that we will fail
	bool isRGBvalid = false;

	// if current list is valid
	if( isMaterialListValid() == true ) {
		// get the material entry
		tsmUnknown = m_pmlCurr->getMaterial(m_iCurrOffset);

		// if the material is an RGB
		if( tsmUnknown.fParams.fFlags & TS::Material::MatFlags == TS::Material::MatRGB ) {
			// get the values out
			wRed = tsmUnknown.fParams.fRGB.fRed;
			wGreen = tsmUnknown.fParams.fRGB.fGreen;
			wBlue = tsmUnknown.fParams.fRGB.fBlue;

			isRGBvalid = true;
		}

	}
	return(isRGBvalid);
}


// Code below here is new!!!

static void createIdString(const TS::Material& tsmUnknown, char* szOutput, int nMaxSize);

// function: dumpMaterialList
// description: Given a filename, dump the material list in human
// 		readable format to disk.  Uses standard streams.
// input: 
// output: 
// returns: true if successful (if material was rgb), false if not RGB 
//		(the values are unchanged)
bool MaterialListEdit::dumpMaterialList(const char* pszFileName) const
{
	FILE* fpOutput;				// output file object
	char achOutputLine[128];	// line of output text
	int iOldDetailLevel;		// remember old detail level
	int ii, jj;					// counter for looping
	TS::Material tsmUnknown;		// catch actual material value
	// initialize to failure
	bool isDumpSuccessful = false;

	// check for valid name
	if( pszFileName != NULL ) {
		// try to open
		fpOutput = fopen(pszFileName, "w");
		if( fpOutput != NULL ) {
			// print a test message
			fprintf(fpOutput, "# Contents of material list \"%s\"\n",
									szGetMatListFileName());

			// remember orig. detail level
			iOldDetailLevel = m_pmlCurr->getDetailLevel();

			// walk through the detail levels
			for(ii=0; ii< getNumDetailLevels(); ii++) {
				// set the detail level -- note that setDetailLevel name
				// is already taken in TSMaterialList
				m_pmlCurr->setDetailLevel(ii);

				// walk through the list
				for(jj=0; jj<iGetListSize(); jj++) {
					// get the material
					tsmUnknown = (*m_pmlCurr)[jj];

					// create a string for it
					createIdString(tsmUnknown, achOutputLine, sizeof(achOutputLine));

					// send a formatted string to disk
					fprintf(fpOutput, "%d, %d\t%s\n", ii, jj, achOutputLine);
				}
			}

			// restore detail level
			m_pmlCurr->setDetailLevel(iOldDetailLevel);

			// close the output stream
			fclose(fpOutput);
		}
	}

	// return status
	return(isDumpSuccessful);
}

// create an identifier string
static void createIdString(const TS::Material& tsmUnknown, char* szTextOutput, int nMaxSize)
{
	static char achIdString[1024];
	static char achTexTypes[128];
	int ii;
	char achTextures[TMP_BSIZE];

	// definition seems to be OK


	switch( tsmUnknown.fParams.fFlags &TS::Material::MatFlags ) {
	case TS::Material::MatNull: 
			sprintf(achIdString, "%s",
				_apchMaterialNames[(int)(tsmUnknown.fParams.fFlags &TS::Material::MatFlags) ] );
			break;
	case TS::Material::MatPalette:
		if( ValidLighting(tsmUnknown.fParams.fFlags &TS::Material::ShadingFlags) &&
								ValidPalette(tsmUnknown.fParams.fIndex) ) {
				sprintf(achIdString, "%s %s %d", 
				_apchMaterialNames[(int)(tsmUnknown.fParams.fFlags &TS::Material::MatFlags) ],
				_apchLightingNames[(int)((tsmUnknown.fParams.fFlags &TS::Material::ShadingFlags) / 256) ],
									tsmUnknown.fParams.fIndex );
			}
			else {
				sprintf(achIdString, "corrupted!");
			}
			break;
	case TS::Material::MatTexture:
		if( ValidLighting(tsmUnknown.fParams.fFlags &TS::Material::ShadingFlags) &&
			ValidTexture(tsmUnknown.fParams.fFlags &TS::Material::TextureFlags) ) {
				// see about filename
				if( strlen(tsmUnknown.fParams.fMapFile) > 0 ) {
				//	tsmUnknown.unlock();
					strncpy(achTextures, tsmUnknown.fParams.fMapFile, sizeof(achTextures)-1);
				//	tsmUnknown.lock(rm,1);
				}
				else
					strncpy(achTextures, tsmUnknown.fParams.fMapFile, sizeof(achTextures)-1);

				//don't null out file name if not found
					//strcpy(achTextures, "---");

				// build up string about texture type

//z	textureplain?			if( tsmUnknown.fParams.fFlags &TS::Material::TextureFlags == TS::Material::TexturePlain ) {
//					strcpy(achTexTypes, "plain");
//				}

				if (0) {}
//z
				// something complex
				else {
					// null out string
					achTexTypes[0] = '\0';

					// use a table lookup
					for(ii=0; ii<=5; ii++) {
						if( (tsmUnknown.fParams.fFlags &TS::Material::TextureFlags) & (0x1000* (1<<ii)) ) {
							strcat(achTexTypes, _apchTextureNames[ii]);
							strcat(achTexTypes, "+");
						}
					}

					// take off trailing plus sign
					achTexTypes[ strlen(achTexTypes)-1 ] = '\0';
				}

				// build up whole string
				sprintf(achIdString, "%s %s (%s) %s", 
					_apchMaterialNames[(int)(tsmUnknown.fParams.fFlags &TS::Material::MatFlags) ],
					_apchLightingNames[(int)(0x100 * tsmUnknown.fParams.fFlags &TS::Material::ShadingFlags) ],
									achTextures, achTexTypes );
			}
			else {
				sprintf(achIdString, "corrupted!");
			}
			break;
	case TS::Material::MatRGB:
		if( ValidLighting(tsmUnknown.fParams.fFlags &TS::Material::ShadingFlags) ) {
				// just check lighting
				sprintf(achIdString, "%s %s (%d,%d,%d)", 
					_apchMaterialNames[(int)(tsmUnknown.fParams.fFlags &TS::Material::MatFlags) ],
					_apchLightingNames[(int)(0x100 * tsmUnknown.fParams.fFlags &TS::Material::ShadingFlags) ],
								tsmUnknown.fParams.fRGB.fRed,
									tsmUnknown.fParams.fRGB.fGreen,
										tsmUnknown.fParams.fRGB.fBlue);
			}
			else {
				sprintf(achIdString, "corrupted!");
			}
			break;
		default:
			sprintf(achIdString, "corrupted!");
			break;
	}  // end switch stmt



	// copy into our output string
	strncpy(szTextOutput, achIdString, nMaxSize-1);
	szTextOutput[nMaxSize-1] = '\0';

}



void
MaterialListEdit::copyDetailLevel(int in_levelFrom, int in_levelTo)
{
	if (!isMaterialListValid())
		return;

	if (in_levelFrom >= getNumDetailLevels() || in_levelTo >= getNumDetailLevels() ||
		in_levelFrom == in_levelTo) {
		return;
	}

	int detailOrig = getDetailLevel();

	int numEntries = iGetListSize();

	TS::Material *tempBuffer = new TS::Material[numEntries];

	setDetailLevel(in_levelFrom);
	for (int i = 0; i < numEntries; i++) {
		tempBuffer[i] = (*m_pmlCurr)[i];
	}

	setDetailLevel(in_levelTo);
	for (i = 0; i < numEntries; i++) {
		(*m_pmlCurr)[i] = tempBuffer[i];
	}

	setDetailLevel(detailOrig);

	delete [] tempBuffer;
	tempBuffer = NULL;
}

void
MaterialListEdit::copyToCurrIndex(TS::Material *in_newMat)
{
	m_pmlCurr->getMaterial(m_iCurrOffset) = *in_newMat;
}
