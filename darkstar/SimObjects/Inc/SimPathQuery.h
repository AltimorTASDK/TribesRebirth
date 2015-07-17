#ifndef _H_SIMPATHQUERY
#define _H_SIMPATHQUERY

#include <sim.h>

class SimPathQuery : public SimQuery
{
   public :
      int  iPathID;
      Int8 iNumWaypoints;

      SimPathQuery()
      {
         type = SimPathQueryType;
      }
};

#endif // _H_SIMPATHQUERY
