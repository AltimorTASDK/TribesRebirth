#ifndef _EXCEPT_H_
#define _EXCEPT_H_

class AllocError 
{
};

class MeshError
{
public:
   char name[40];
   char type[200];
   MeshError( char *meshname, char *typename )
      {
      strncpy( name, meshname, sizeof(name) );
      name[sizeof(name)-1] = 0;
      strncpy( type, typename, sizeof(type) );
      type[sizeof(type)-1] = 0;
      }
};

class GeneralError
{
public:
   char type[200];
   GeneralError( char *typename )
      {
      strncpy( type, typename, sizeof(type) );
      type[sizeof(type)-1] = 0;
      }
};

#endif
