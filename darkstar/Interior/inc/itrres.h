//-----------------------------------------------------------------------------


//------------------------------------------------------------------------------


#ifndef _ITRRES_H_
#define _ITRRES_H_

#include <ResManager.h>

//------------------------------------------------------------------------------

class ResourceTypeITRGeometry: public ResourceType
{
public:   
   ResourceTypeITRGeometry(const char *ext = ".dig"):
      ResourceType( ResourceType::typeof(ext) ) { }
   void* construct(StreamIO *stream,int) {
		Persistent::Base::Error err;
		Persistent::Base* bptr = Persistent::Base::load(*stream,&err);
		AssertFatal(bptr != 0,
			"ReourceTypeITRGeometry:: Could not load file");
		ITRGeometry* geometry = dynamic_cast<ITRGeometry*>(bptr);
		AssertFatal(geometry != 0,
			"ReourceTypeITRGeometry:: File does not contain a ITRGeometry");
   	return geometry;
   }
   void destruct(void *p) {
   	delete (ITRGeometry*)p;
   }
};

class ResourceTypeITRLighting: public ResourceType
{
public:   
   ResourceTypeITRLighting(const char *ext = ".dil"):
      ResourceType( ResourceType::typeof(ext) ) { }
   void* construct(StreamIO *stream,int) {
		Persistent::Base::Error err;
		Persistent::Base* bptr = Persistent::Base::load(*stream,&err);
		AssertFatal(bptr != 0,
			"ReourceTypeITRLighting:: Could not load file");

      // attempt to make this a missionlit version
      ITRMissionLighting* missionLighting = dynamic_cast<ITRMissionLighting*>(bptr);
      if(missionLighting)
         return(missionLighting);

		ITRLighting* lighting = dynamic_cast<ITRLighting*>(bptr);
		AssertFatal(lighting != 0,
			"ReourceTypeITRLighting:: File does not contain a ITRGeometry");
   	return lighting;
   }
   void destruct(void *p) {
    	delete (ITRLighting*)p;
   }
};

class ResourceTypeITRShape: public ResourceType
{
public:   
   ResourceTypeITRShape(const char *ext = ".dis"):
      ResourceType( ResourceType::typeof(ext) ) { }
   void* construct(StreamIO *stream,int) {
		Persistent::Base::Error err;
		Persistent::Base* bptr = Persistent::Base::load(*stream,&err);
		AssertFatal(bptr != 0,
			"ReourceTypeITRShape:: Could not load file");
		ITRShape* shape = dynamic_cast<ITRShape*>(bptr);
		AssertFatal(shape != 0,
			"ReourceTypeITRShape:: File does not contain a ITRGeometry");
   	return shape;
   }
   void destruct(void *p) {
   	delete (ITRShape*)p;
   }
};


#endif

