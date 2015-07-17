//
// palmap.cpp
//

#include "palmap.h"

Palmap::Palmap()
{
	int i;

	m_pShadeIncludes		= new CSelections[256];
	m_pHazeIncludes			= new CSelections[256];
	m_pTranslucencyIncludes = new CSelections[256];
	m_pAllowedColorMatches  = new CSelections;

	for (i = 0; i < 256; i++) 
	{
		usesShadeDefault.markSelected(i, true);
		usesHazeDefault.markSelected (i, true);
		usesTransDefault.markSelected(i, true);
	}
		
	m_hazeLevels            =  0;
	m_hazeToIndex	        = -1;
	m_shadeLevels           =  0;
	m_transLevels           =  0;
	m_transOffset           = -1;
	m_transRange            = -1;
	m_firstColorTransparent = FALSE;
	m_calcTrans             = FALSE;
	m_gammaCorrect          = 0.0;
}

Palmap::~Palmap()
{
	if (m_pShadeIncludes != NULL)
		delete [] m_pShadeIncludes;
	if (m_pHazeIncludes != NULL)
		delete [] m_pHazeIncludes;
	if (m_pTranslucencyIncludes != NULL)
		delete [] m_pTranslucencyIncludes;
	if (m_pAllowedColorMatches != NULL)
		delete m_pAllowedColorMatches;
}

Palmap *Palmap::load(const char *in_filename)
{
	Palmap *palmap = new Palmap;

	if (!palmap->read(in_filename))
	{
		delete palmap;
		return(NULL);
	}
	return(palmap);
}

Bool Palmap::read(const char *in_filename)
{
	// The world's most bogus file format.
	//
	FileRStream fileStream(in_filename);
	if (fileStream.getHandle() == INVALID_HANDLE_VALUE) 
		return(FALSE);

	char test[4];
	fileStream.read(4, test);

	if (strncmp(test, "DPMP", 4) != 0) 
	{
		fileStream.close();
		return(FALSE);
	}

	int pathLength;
	char nameBuffer[512];
	fileStream.read(&pathLength);
	if (pathLength != 0) 
		fileStream.read(pathLength, nameBuffer);

	int i;
	for (i = 0; i < 256; i++) 
		m_pShadeIncludes[i].read(fileStream);
	for (i = 0; i < 256; i++) 
		m_pHazeIncludes[i].read(fileStream);
	for (i = 0; i < 256; i++) 
		m_pTranslucencyIncludes[i].read(fileStream);

	m_pAllowedColorMatches->read(fileStream);
	m_shadeDefaultIncludes.read(fileStream);
	m_hazeDefaultIncludes. read(fileStream);
	m_transDefaultIncludes.read(fileStream);

	usesShadeDefault.read(fileStream);
	usesHazeDefault. read(fileStream);
	usesTransDefault.read(fileStream);

	fileStream.read(&m_hazeToIndex);
	fileStream.read(&m_hazeLevels);
	fileStream.read(&m_shadeLevels);
	fileStream.read(&m_transLevels);
	fileStream.read(&m_transOffset);
	fileStream.read(&m_transRange);
	fileStream.read(&m_firstColorTransparent);
	fileStream.read(&m_calcTrans);
	fileStream.read(sizeof(double), (UInt8 *)&m_gammaCorrect);
	fileStream.close();

	return(TRUE);
}

