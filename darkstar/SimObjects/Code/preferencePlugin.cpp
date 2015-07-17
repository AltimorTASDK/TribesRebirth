//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include <sim.h>
#include <simPrefs.h>
#include <simConsolePlugin.h>
#include <preferencePlugin.h>

//------------------------------------------------------------------------------
void PreferencePlugin::init()
{
	console->printf("PreferencePlugin");

	console->addCommand(NewPrefObject,  "newPreferenceObject",  this);
	console->addCommand(Notify,         "prefNotify",  this);
	console->addCommand(Close,          "prefClose",  this);
	console->addCommand(Save,           "prefSave",   this);
	console->addCommand(Restore,        "prefRestore",   this);
	console->addCommand(SetInt,         "prefSetInt",  this);
	console->addCommand(SetFloat,       "prefSetFloat",  this);
	console->addCommand(SetString,      "prefSetString",  this);
}


//----------------------------------------------------------------------------
void PreferencePlugin::startFrame()
{
}

void PreferencePlugin::endFrame()
{
}


//----------------------------------------------------------------------------
const char *PreferencePlugin::consoleCallback(CMDConsole *, int id,int argc,const char *argv[])
{
   if (!manager) return "False";

	switch(id) 
	{
      //--------------------------------------     
      case NewPrefObject:
         if (argc == 3)
         {
            SimPreference *obj = new SimPreference;
            manager->addObject( obj );
            manager->assignName( obj, argv[1] );
            obj->setRoot( argv[1] );
            if(obj->open())
               break;
         }
         console->printf("newPreferenceObject objName registryPath");
         return "False";
         
      //--------------------------------------     
      case Notify:
         if (argc == 2)
         {
            SimPreference *obj = dynamic_cast<SimPreference*>( manager->findObject(argv[1]) );               
            if (obj)
               obj->notify();
            else
               console->printf("no preference object named %s", argv[1]);
            if(obj)
               break;
         }
         console->printf("prefNotify objName");
         return "False";
        
      //--------------------------------------     
      case Close:
         if (argc == 2)
         {
            SimPreference *obj = dynamic_cast<SimPreference*>( manager->findObject(argv[1]) );               
            if (obj)
               obj->close();
            else
               console->printf("no preference object named %s", argv[1]);
            if(obj)
               break;
         }
         console->printf("prefClose objName");
         return "False";
      
      //--------------------------------------     
      case Save:
         if (argc == 2)
         {
            SimPreference *obj = dynamic_cast<SimPreference*>( manager->findObject(argv[1]) );               
            if (obj)
               obj->save();
            else
               console->printf("no preference object named %s", argv[1]);
            if(obj)
               break;
         }
         console->printf("prefSave objName");
         return "False";
      
      //--------------------------------------     
      case Restore:
         if (argc == 2)
         {
            SimPreference *obj = dynamic_cast<SimPreference*>( manager->findObject(argv[1]) );               
            if (obj)
               obj->restore();
            else
               console->printf("no preference object named %s", argv[1]);
            if(obj)
               break;
         }
         console->printf("prefRestore objName");
         return "False";
      
      //--------------------------------------     
      case SetInt:
         if (argc == 5)
         {
            SimPreference *obj = dynamic_cast<SimPreference*>( manager->findObject(argv[1]) );               
            if (obj)
               if ( *argv[2] == '0' )
               {
                  int value;
                  if ( !obj->get( argv[3], &value ) )
                     obj->set( argv[3], atoi(argv[4]) );
               }
               else
                  obj->set( argv[3], atoi(argv[4]) );
            else
               console->printf("no preference object named %s", argv[1]);
            if(obj)
               break;
         }
         console->printf("PrefSetInt objName bool(overwrite) keyName int");
         return "False";
      
      //--------------------------------------     
      case SetFloat:
         if (argc == 5)
         {
            float value;
            SimPreference *obj = dynamic_cast<SimPreference*>( manager->findObject(argv[1]) );               
            if (obj)
            {
               sscanf(argv[4], "%f", &value);
               if ( *argv[2] == '0' )
               {
                  if ( !obj->get( argv[3], &value ) )
                     obj->set( argv[3], value );
               }
               else
                  obj->set( argv[3], value );
            }
            else
               console->printf("no preference object named %s", argv[1]);
            if(obj)
               break;
         }
         console->printf("PrefSetFloat objName bool(overwrite) keyName float");
         return "False";
      
      //--------------------------------------     
      case SetString:
         if (argc == 5)
         {
            SimPreference *obj = dynamic_cast<SimPreference*>( manager->findObject(argv[1]) );               
            if (obj)
            {
               if ( *argv[2] == '0' )
               {
                  char buff[256];
                  if ( !obj->getString( argv[3], buff, 256 ) )
                     obj->setString( argv[3], argv[4] );
               }
               else
                  obj->setString( argv[3], argv[4] );
            }
            else
               console->printf("no preference object named %s", argv[1]);
            if(obj)
               break;
         }
         console->printf("PrefSetString bool(overwrite) keyName string");
         return "False";
	}
	return 0;
}

