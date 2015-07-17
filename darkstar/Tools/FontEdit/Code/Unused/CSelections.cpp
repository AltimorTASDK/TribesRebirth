//#include "stdafx.h"

#include "CSelections.h"
#include <stdlib.h>

CSelections::CSelections()
{
	memset(m_selections, 0, sizeof(bool) * 256);
}

UInt32 *
CSelections::pack()
{
	UInt32 *packed = new UInt32[256/32];

	for (int i = 0; i < 256; i++) {
		if (m_selections[i] == 0) {
			packed[i/32] &= ~(1 << (i % 32));
		} else {
			packed[i/32] |= (1 << (i % 32));
		}
	}

	return packed;
}

void
CSelections::unpack(UInt32 *in_packedSel)
{
	for	(int i = 0; i < 256; i++) {
		if (in_packedSel[i/32] & (1 << (i % 32))) {
			m_selections[i] = 1;
		} else {
			m_selections[i] = 0;
		}
	}	
	
//	memset(m_selections, 0, sizeof(bool) * 256);
}

void
CSelections::copyArray(bool *in_array)
{
	memcpy(m_selections, in_array, 256 * sizeof(bool));
}

void
CSelections::write(FileWStream &in_fileStream)
{
//	char tempBuf[256];
//	for (int i = 0; i < 256; i++) {
//		tempBuf[i] = (m_selections[i] != 0) ? 1 : 0;
//	}

	UInt32 *packed = pack();
	in_fileStream.write(8, packed);
	delete [] packed;
}

void
CSelections::read(FileRStream &in_fileStream)
{
	UInt32 tempBuf[256/32];

	in_fileStream.read(256/32, tempBuf);
	unpack(tempBuf);
}
