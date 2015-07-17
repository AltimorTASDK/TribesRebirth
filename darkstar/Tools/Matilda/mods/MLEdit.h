// File: mledit.h
// Program: Matilda -- a material list editor
// Authors: John Leavens & Steve Nichols
// History:
//	11/18/95 -- code is checkpointed at external version 1.00;
//		internal SourceSafe version is 2.05
// MLEdit.h
//
// Header file w/ class declarations for the MaterialListEdit class,
// used to encapsulate the TSMaterialList class
//
// version 0.1 -- Nov. 8, 1995; Steve Nichols
// version 0.2 -- Nov. 10; all list manipulation code is usable
// version 0.3 -- Nov. 12; starting to flesh out MaterialList set/get's
// 		Nov. 13, renamed to MLEdit.h
// version 0.4 -- Nov. 13, bring into line w/ the latest Ts_mat.h header

#if !defined(_MATLISTEDIT_H)
#define _MATLISTEDIT_H

// for now, make sure we are not compiling dynamix code
// #define NO_DYNAMIX

// pull in the MaterialList declarations
#if !defined(NO_DYNAMIX)
	#include <ts_mat.h>
#else
	// this include file fakes some of the defs, for testing before
	// integration with Dynamix code
	// #include "Ts_dummy.h"
#endif // #if !NO_DYNAMIX
#include "matsuprt.h"

// constant that is all texture bits or'ed together
const int ciAllTextureBits = 63; 

// some macros to use in assertions	-- check integrity of members
#define ValidMaterial(m) ((m) == TSMaterial::MatNull || (m) == TSMaterial::MatPalette || (m) == TSMaterial::MatRGB || (m) == TSMaterial::MatTexture)
#define ValidLighting(lt) ((lt) == TSMaterial::LightingUnlit ||(lt) == TSMaterial::LightingFlat ||(lt) == TSMaterial::LightingSmooth)
#define ValidTexture(tt) (0 <= (int)(tt) && (int)(tt) <= ciAllTextureBits)
#define ValidPalette(p) (0 <= p && p < 256)

// class declaration
class MaterialListEdit
{
private:
// ------------
// data members
// ------------
	// is the list usable yet?
	bool m_isListValid;

	// flag if current list and old list are different
	bool m_isUndoAvail;

	// flag whether we need to save before exiting
	bool m_isSaveAdvised;

	// current MaterialList we are working on
	TSMaterialList* m_pmlCurr;

	// save last copy of MaterialList (for undo or cancel)
	TSMaterialList* m_pmlOld;

	// current index into the MaterialList
	Int32 m_iCurrOffset;

	// number of entries in the current Material List
	Int32 m_iCurrSize;

	// number of entries in the old Material List
	Int32 m_iOldSize;

	// current name of the MaterialList file we are working with
	char m_achListFileName[TMP_BSIZE];

	// full path name of MaterialList file
	char m_achListPathName[TMP_BSIZE];

	// name of current material (for identification in list box)
	char m_achMatName[TMP_BSIZE];

	// MaterialList path only, to find bitmap files
	char m_achBitmapPathName[TMP_BSIZE];


// -----------------
// private functions 
// -----------------
 	inline isRGB(WORD rgb) { return( 0 <= rgb && rgb <= 255 ); }

// --------------------
// the public interface
// --------------------
public:
	// constructors and destructors
	MaterialListEdit();
	~MaterialListEdit();
	// MaterialListEdit(const MaterialListEdit&);

	// define an assignment op for reallocs and undo's
	// MaterialListEdit& operator=(const MaterialListEdit& rhs);
	// problematic w/o an = for TSMaterialList

	// pointer to current GFXPalette
	GFXPalette* pGFXPal;

// ----------------------------------------------
// define functions that will manipulate the list
// ----------------------------------------------

	// see if list is valid or not
	bool isMaterialListValid() const {return m_isListValid; }

	// see if a save is advised or not
	bool isSaveAdvised() const {return m_isSaveAdvised; }
	void touchSaveAdvised() { m_isSaveAdvised = true; }

	// set and get default material list filename (and full path)
	bool fSetMatListFileName(const char* szMatFile, const char* szMatPath);
	const char* szGetMatListFileName() const;
	const char* szGetMatListPathName() const;
	const char* szGetBitmapPathName() const;
	bool isMatListFileNameGood() const;

	// read a material list from disk using default filename
	bool fReadMaterialList();
	
	// write a material list to disk using default filename
	bool fSaveMaterialList();

	// delete current entry; crunch list by one
	bool fDeleteEntry(const Int32 iIndexToDelete);

	// increase list by one: add a null entry to end, make that current
	bool fAddEntryToEnd();

	// create a new list with a single null entry
	bool fCreateNewList(int iNumDetails = 1);

	// undo last change by copying old copy of list to new copy
	// TODO: currently not implemented
	bool fUndoLastChange();

	// get and set detail level
	void setDetailLevel(const int iNewLevel); 
	Int32 getDetailLevel() const;
	Int32 getNumDetailLevels() const; 

	// see if a detail level is in range or not
	bool isDetailLevelInRange(int iDetailLevel) const;

	// navigate the material list, return new offset, stop at top or bottom
	bool fSetCurrIndex(Int32 iNewIndex);
	Int32 iGetCurrIndex() const { return m_iCurrOffset; }
	Int32 iGetListSize() const { return m_iCurrSize; }

	// return a text name for the current material (for list box)
	const char* szNameOfMaterial(const Int32 iOffset);

// -----------------------------------------------------------
// define functions that deal specifically with material lists
// if we are building a debug version, don't even declare them
// -----------------------------------------------------------
	// null is a placeholder...no actual attributes to get or set
	void setMatToNull();

	// handle textures
	void setMatToTexture(TSMaterial::LightingType ltKind, int iTextureFlags = (int)(TSMaterial::TexturePlain));
 	int getTextureFlags() const;

	// get pointer to current material
	TSMaterial* getCurrMat();

	// get material type of current material: null, rgb, palette, texture
	TSMaterial::MatType getMatType() const;

	// get the lighting type of the material (rgb, palette, texture)
	TSMaterial::LightingType getMatLighting() const;

	// return the name of the associated bitmap, or NULL if none
	char* getBitMapName() const;

	// set the bitmap file name...
	void setBitMapName(const char* szNewName);

	// Palette type of material
	void setMatToPalette(TSMaterial::LightingType ltKind, Int32 iPalIndex);
	int getPaletteIndex() const;

	// deal w/ RGB type
	void setMatToRGB(TSMaterial::LightingType ltKind, const WORD wRed, const WORD wGreen, const WORD wBlue);
	bool getRGB(WORD& wRed, WORD& wGreen, WORD& wBlue) const;

 	bool dumpMaterialList(const char* pszFileName) const;

	void copyDetailLevel(int in_levelFrom, int in_levelTo);
	void copyToCurrIndex(TSMaterial *in_newMat);

};

// we plan on using a single global instance of a MaterialListEdit
// class in the matilda application

// sn -- turn this on for matilda app
// back on for Matilda2
extern MaterialListEdit mlEdit;

#endif // if !defined _MATLISTEDIT_H
