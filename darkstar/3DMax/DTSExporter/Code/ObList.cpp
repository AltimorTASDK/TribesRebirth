#include <MAX.H>

#include "OBList.H"
#include "SceneEnum.H"

//-------------------------------------------------------------------

ObjNameList::~ObjNameList() 
{
    while(head) 
      {
        Name *next = head->next;
        delete head;
        head = next;
        }
    head = tail = NULL;
    count = 0;    
}

int ObjNameList::Contains(TSTR &n) 
   {
    int index = 0;
    for( Name *e = head; e; e = e->next )
      {
        if(e->name == n)
            return index;
        index++;
        }
    return -1;
}

void ObjNameList::Append(TSTR &n) 
{
    Name *entry = new Name(n);
    if(tail)
        tail->next = entry;
    tail = entry;
    if(!head)
        head = entry;
    count++;    
    }

void ObjNameList::MakeUnique(TSTR &n) 
{
    // First make it less than 10 chars.
    if( n.Length() > 10 ) 
      n.Resize(10);
    
    if( Contains(n) < 0 ) 
      {
        Append(n);
        return;
        }
    // Make it unique and keep it 10 chars or less
    for(int i = 0; i < 100000; ++i) 
      {
        char buf[12];
        sprintf(buf,"%d",i);
        TSTR num(buf);
        TSTR work = n;
        int totlen = num.Length() + work.Length();
        if(totlen > 10)
            work.Resize(10 - (totlen - 10));
        work = work + num;
        if(Contains(work) < 0)  
         {
            Append(work);
            n = work;
            return;
            }
        }
    // Forget it!
}

//-------------------------------------------------------------------
