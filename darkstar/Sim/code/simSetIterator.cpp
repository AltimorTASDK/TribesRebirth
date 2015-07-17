//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------


#include <simbase.h>
#include "SimSetIterator.h"


//-----------------------------------------------------------------------------

inline void SimSetIterator::Stack::push_back(SimSet* set)
{
	increment();
	last().set = set;
	last().itr = set->begin();
}


//-----------------------------------------------------------------------------

SimSetIterator::SimSetIterator(SimSet* set)
{
	if (!set->empty())
		stack.push_back(set);
}


//-----------------------------------------------------------------------------

SimObject* SimSetIterator::operator++()
{
	SimSet* set;
	if ((set = dynamic_cast<SimSet*>(*stack.last().itr)) != 0) {
		if (!set->empty()) {
		   stack.push_back(set);
			return *stack.last().itr;
		}
	}

	while (++stack.last().itr == stack.last().set->end()) {
		stack.pop_back();
		if (stack.empty())
			return 0;
	}
	return *stack.last().itr;
}	

