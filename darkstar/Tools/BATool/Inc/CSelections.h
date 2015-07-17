#ifndef _CSELECTIONS_H_
#define _CSELECTIONS_H_

#include "types.h"
#include "filstrm.h"

class CSelections {
   public:
	bool m_selections[256];

   public:
	CSelections();

	void markSelected(int in_index, bool in_state) { m_selections[in_index] = in_state; }
	bool isSelected(int in_index) const { return m_selections[in_index]; }

	void copyArray(bool *in_array);

	void write(FileWStream &in_fileStream);
	void read(FileRStream &in_fileStream);

	UInt32 *pack();
	void    unpack(UInt32 *in_packedSel);
};

#endif
