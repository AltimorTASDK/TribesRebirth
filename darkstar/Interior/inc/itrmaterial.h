//----------------------------------------------------------------------------


//----------------------------------------------------------------------------

#ifndef _ITRMATERIAL_H_
#define _ITRMATERIAL_H_

#include <ts_mat.h>
#include <tvector.h>


//----------------------------------------------------------------------------

struct ITRMaterial
{
	GFXBitmap* texture;
};


class ITRMaterialList
{
 	Vector<ITRMaterial> mList;

public:

	static ITRMaterialList* load(const char* file);
	ITRMaterial& operator[](int index);
};

inline ITRMaterial& ITRMaterialList::operator[](int index)
{
	return mList[index];
}	

inline ITRMaterialList* ITRMaterialList::load(const char* file)
{
	TSMaterialList* ts = TSMaterialList::load(file,0,false);
	AssertFatal(ts != 0,"ITRMaterialList::load: Error loading file");

	char filename[256];
	strcpy(filename,file);
	char* cp = &filename[strlen(filename) - 1];
	while (cp > filename && *cp != '\\' && *cp != '/')
		--cp;
	++cp;

	ITRMaterialList* ml = new ITRMaterialList;
	ml->mList.setSize(ts->getMaterialsCount());
	for (int i = 0; i < ml->mList.size(); i++) {
		TSMaterial mat = (*ts)[i];
		if (mat.fType == TSMaterial::MatTexture) {
			strcpy(cp,mat.fMapFile);
			ml->mList[i].texture = GFXBitmap::load(filename);
			AssertFatal(ml->mList[i].texture != 0,
				"ITRMaterialList::load: Error loading bitmap");
		}
		else
			ml->mList[i].texture = 0;
	}

	delete ts;
	return ml;
}	



#endif
