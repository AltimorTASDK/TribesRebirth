//
// palmap.h
//

#ifndef _PALMAP_H_
#define _PALMAP_H_

#include "CSelections.h"

class Palmap
{
  public:
	CSelections	*m_pShadeIncludes;
	CSelections	*m_pHazeIncludes;
	CSelections	*m_pTranslucencyIncludes;
	CSelections	*m_pAllowedColorMatches;

	CSelections	 m_shadeDefaultIncludes;
	CSelections	 m_hazeDefaultIncludes;
	CSelections	 m_transDefaultIncludes;
	CSelections	 usesShadeDefault;
	CSelections	 usesHazeDefault;
	CSelections	 usesTransDefault;

	int		     m_hazeToIndex;
	int	         m_hazeLevels;
	int	         m_shadeLevels;
	int          m_transLevels;
	int          m_transOffset;
	int          m_transRange;
	BOOL         m_firstColorTransparent;
	BOOL         m_calcTrans;
	double       m_gammaCorrect;

	Palmap();
	~Palmap();
	static Palmap *load(const char *in_filename);
	Bool read(const char *in_filename);	
};

#endif