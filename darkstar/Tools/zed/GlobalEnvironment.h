// GlobalEnvironment.h : header file
//

#ifndef _GLOBALENVIRONMENT_H_
#define _GLOBALENVIRONMENT_H_

// Defines that are Global in nature
#define AXIS_X	0x1
#define AXIS_Y	0x2
#define AXIS_Z	0x4


/////////////////////////////////////////////////////////////////////////////
// CGlobalEnvironment window

class CGlobalEnvironment
{
public:
	CString& GetPalettePath();
	CString& GetWadPath();
	CGlobalEnvironment();
	void DoDialog();
	int mAxis;
   bool  mLockTexturePositions;
	CString mWadPath;
	CString mPalettePath;
};

/////////////////////////////////////////////////////////////////////////////
extern CGlobalEnvironment Globals;

#endif
