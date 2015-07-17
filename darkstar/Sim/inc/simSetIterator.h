//-----------------------------------------------------------------------------

// "Iterator" which will iterate through all the objects
// in a set including all of it's children.
//
// Use like this:
// 	for (SimSetIterator itr(manager); *itr; ++itr)
//			doSomethingUsefull(*itr);

//-----------------------------------------------------------------------------


#ifndef _SimSetITERATOR_H
#define _SimSetITERATOR_H


//-----------------------------------------------------------------------------

class SimSetIterator
{
	struct Entry {
		SimSet* set;
		SimSet::iterator itr;
	};
	class Stack: public Vector<Entry> {
	public:
		void push_back(SimSet*);
	};
	Stack stack;

public:
	SimSetIterator(SimSet*);
	SimObject* operator++();
	SimObject* operator*() {
		return stack.empty()? 0: *stack.last().itr;
	}
};


#endif
