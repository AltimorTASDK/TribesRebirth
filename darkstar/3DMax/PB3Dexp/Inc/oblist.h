#ifndef OBLIST_H_
#define OBLIST_H_

#include "scenenum.h"

//-------------------------------------------------------------------

class ObjNameList {
	public:
      class Name 
         {
	      public:
		      TSTR name;
		      Name *next;
		      Name(TSTR n) 
               { 
               name = n; 
               next = NULL; 
               }
	      };

		Name *head;
		Name *tail;
		int			count;
					ObjNameList() 
                  { 
                  head = tail = NULL; 
                  count = 0; 
                  }
					~ObjNameList();
		int		Count() { return count; }
		int		Contains(TSTR &n);
		void		Append(TSTR &n);
		void		MakeUnique(TSTR &n);
};


//-------------------------------------------------------------------
#endif
