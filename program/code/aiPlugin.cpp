//----------------------------------------------------------------------------
//
//    AIPlugin.cpp
//
//----------------------------------------------------------------------------

#include <sim.h>
#include <simGame.h>

#include "AIPlugin.h"
#include "feardcl.h"
#include "fearGlobals.h"


//----------------------------------------------------------------------------

static const char *intToStr(int d)
{
   static char buf[16];
   sprintf(buf, "%d", d);
   return buf;
}

static const char *floatToStr(float d)
{
   static char buf[32];
   sprintf(buf, "%g", d);
   return buf;
}

static const char trueTxt[] =    "True";
static const char falseTxt[] =   "False";
static const char neg1Txt[] =    "-1";
static const char scan3fTxt[] =  "%f %f %f";
static const char missionCleanup[] = "MissionCleanup";

//-------------------------------------------------------------------------------------

static const char * c_baseRepGetFirst(CMDConsole *, int, int argc, const char * *)
{
   if( argc != 1 || !sg.playerManager )
   {
      Console->printf( "BaseRep::getFirst()" );
      Console->printf( "Like Client::getFirst(), but AI 'reps' are included." );
      return neg1Txt;
   }
   
   PlayerManager::BaseRep * br = sg.playerManager->getBaseRepList();
   if( ! br ) 
      return neg1Txt;
   else
      return intToStr( br->id );
}

static const char * c_baseRepGetNext(CMDConsole *, int, int argc, const char **argv)
{
   if( argc != 2 || !sg.playerManager )
   {
      Console->printf( "BaseRep::getNext( <previous base rep id> );" );
      Console->printf( "Like Client::getNext(), but AI 'reps' are included." );
      return neg1Txt;
   }
   PlayerManager::BaseRep * br = sg.playerManager->findBaseRep(atoi(argv[1]));
   if( !br || !br->nextClient )
      return neg1Txt;
   else
      return intToStr( br->nextClient->id );
}


static const char *c_objectGetName(CMDConsole *, int, int argc, const char **argv)
{
   if( wg->manager && argc == 2 )
      if( SimObject * sobj = wg->manager->findObject(argv[1]) )
         return sobj->getName();
         
   return "";
}


//----------------------------------------------------------------------------

/*
   Notes:
      Each listable callback should be able to give help if invoked with zero arguments
      and ai==NULL.
*/


#if 0
const char * AIPlugin::onGetHelp(int /*argc*/, const char * * /*argv*/ )
{
 static char * help[] = {
  " ",
  "AI players are installed with AI::spawn(), and removed with AI::delete().  Once an AI "
  "player is installed and given a name, commands use that name as their first parameter "
  "and wildcards are allowed to give the same command to multiple AIs. "
  "   Each AI object has a list of directives, a directive is one of the following:",
  "      Waypoint:      point in a path",
  "      Follow:        a player to follow",
  "      Target:        a player to attack",
  "      Guardpoint:    a location to guard",
  "Directives which have an order # are sorted (waypts and guardpts) and only one may "
  "exist for each order #.  Targets are part of the same list but order doesn't at this "
  "time have any meaning for them.  See commands:  AI::DirectiveXXX() to add directives.",
  "      AI::attack() will assign the AI to attack the given player.  ",
  " ",
 };

   for( int i = 0; i < sizeof(help)/sizeof(help[0]); i++ )
      console->printf( help[i] );

   return trueTxt;
}
#endif

AIObj * AIPlugin::getAI( const char * aiName )
{
   AIManager::iterator  itr;
   for( itr = aim->begin(); itr != aim->end(); itr++ )
      if( !stricmp( (*itr)->name, aiName ) )
         return (*itr);
   return 0;
}

const char * AIPlugin::onGetId(int argc, const char * * argv )
{
   if( argc > 1 )
   {   
      AIObj * ai = getAI( argv[1] );
      
      if( ai )
         return intToStr( ai->repId );
      else
         console->printf(" Could not find drone %s.", argv[1] );
   }
   console->printf("AI::getId(<ai name>);");
   console->printf("Returns the rep Id of the given AI.  The rep Id can in many" );
   console->printf("cases be used like a client Id (in those console functions that" );
   console->printf("take a client Id)." );
   return falseTxt;
}

// If the given AI has a target that it is currently going after, return its 
// index, else -1.  
const char * AIPlugin::onGetTarget(int argc, const char * * argv )
{
   if( argc > 1 )
   {
      AIObj * ai = getAI( argv[1] );
      
      if( ai )
         return intToStr( ai->curTargetId() );
      else
         console->printf(" Could not find drone %s.", argv[1] );
   }
   console->printf("AI::GetTarget(<ai name>);");
   console->printf("If the given AI is currently attacking one of its targets, the rep" );
   console->printf("id of that target is returned, else -1 is returned." );
   return falseTxt;
}

// All SimObjects allow the script writer to create member variables on the fly using
// the object Id, so we need this function to get the Id of the AI object.  
const char * AIPlugin::onFetchObject(int argc, const char * * argv )
{
   if( argc > 1 )
   {
      AIObj * ai = getAI( argv[1] );
      if( ai )
         return intToStr( ai->getId() );
      else
         console->printf( "Could not find drone %s.  Usage:", argv[1] );
   }
   console->printf( "%s(<ai name>);", argv[0] );
   console->printf( "Get an object id handle (for creating member variables)." );
   return neg1Txt;
}


const char * AIPlugin::onList(int /*argc*/, const char * * /*argv*/ )
{
   for( AIManager::iterator itr = aim->begin(); itr != aim->end(); itr++ )
   {
      AIObj *ai = *itr;
      Point3F  l = ai->player->getLeadCenter();
      Point3F  r = ai->player->getRot();
      console->printf( "%s  L=(%g,%g,%g)  R=(%g,%g,%g)", ai->name,
                     l.x, l.y, l.z,   r.x, r.y, r.z    );
   }
   return trueTxt;
}


const char * AIPlugin::onSpawnAI(int argc, const char **argv)
{
   if ( argc < 4 )
   {
      console->printf(
         "AI::spawn: <AI name> <armor type> <position> [rotation] [display name] [voice]");
      console->printf( "Create an AI of the given name and armor type at position." );
      console->printf( "If not given, display name is the same as the AI name." );
      console->printf( "Voice defaults to male1 if not supplied here." );
      return falseTxt;
   }

   if( !sg.playerManager || !sg.manager )
   {
      console->printf("Requisite manager(s) not found.");
      return falseTxt;
   }
   
   SimGroup * cleanupGroup = (SimGroup *) sg.manager->findObject(missionCleanup);
   AssertFatal( cleanupGroup, "ai: should know there's a cleanup group by the time." );
   
   if( getAI( argv[1] ) != NULL )
   {
      console->printf("An AI named %s already exists!", argv[1] );
      return falseTxt;
   }

   const char * displayName = NULL;
   const char * voice = stringTable.insert("male1");
   Point3F pos(0, 0, 0), rot(0, 0, 0);
   sscanf( argv[3], scan3fTxt, &pos.x, &pos.y, &pos.z);
   if( argc > 4 ){
      sscanf( argv[4], scan3fTxt, &rot.x, &rot.y, &rot.z);
      if( argc > 5 ){
         displayName = argv[5];
         if( argc > 6 ){
            voice = stringTable.insert( argv[6] );
         }
      }
   }

   Player * player = new Player();
   player->setInitInfo( argv[2], pos, rot );
   player->setAIControlled(true);
   
   player->assignName( displayName ? displayName : argv[1] );
   
   if(!sg.manager->registerObject(player))
      return falseTxt;
   
   // GameBase method, sets map name.  
   player->setName( displayName ? displayName : argv[1] );
   
   AIObj * ai = new AIObj();
   strncpy ( ai->name, argv[1], AIObj::MaxNameLen );
   ai->player = player;
   ai->voice = voice;
   
   if( !sg.manager->registerObject(ai))
   {
      player->deleteObject();
      return falseTxt;
   }
   
   ai->deleteNotify ( player );
   aim->deleteNotify ( ai );
   aim->push_back( ai );
   
   ai->addVariables( console );
   
   cleanupGroup->addObject(ai->player);
   cleanupGroup->addObject(ai);
   
   return trueTxt;
}


// First pass code - only had one target.  
const char * AIPlugin::onAttackPlayer (AIObj * ai, int argc, const char *argv[])
{
   if (argc == 1)
      ai->seekFlag = atoi( argv[0] );
   else
   {
      console->printf("AI::Attack( aiWildcard, clientId )" );
      console->printf("Attack the player having the specified clientId.  No other" );
      console->printf("actor targeting will happen if this target is set (i.e. it's" );
      console->printf("for one target at a time)." );
      console->printf("For implementing a list of targets, see AI::DirectiveTarget()" );
      console->printf("and AI::setAutomaticTargets()" );
      return falseTxt;
   }
   return trueTxt;
}

const char * AIPlugin::onDeleteAI(AIObj * ai, int, const char * * )
{
   if( ai != NULL ){
      // ai->removeThis();
      // This will cause the deletion to happen in the proper sequence.  
      if( ai->player ){
         ai->player->deleteObject();
         ai->player = NULL;
      }
      return trueTxt;
   }
   else{
      console->printf("AI::Delete( <AI wildcard> )" );
      console->printf("Delete any AI which matches the given wildcard specification." );
      return falseTxt;
   }
}

const char * AIPlugin::onGetAICount( int, const char * * )
{
   return intToStr( aim->size() );
}

const char * AIPlugin::onFollowDirective(AIObj * ai, int argc, const char *argv[])
{
   if( argc > 0 )
   {
      AIObj::Follow   follow;
      if( AIObj::findPlayerObject(follow.playerId = atoi(argv[0])) )
      {
         if( argc > 1 )
         {
            follow.getPointAndRadius( argv[1] );
            if( argc>2 && (follow.order=atoi(argv[2])) < 0 )
            {
               console->printf("Error: the order# supplied to a directive "
                     "must be a postive integer. Directive not installed." );
               return falseTxt;
            }
         }
         ai->addFollow ( follow );
         return trueTxt;
      }
      else 
         console->printf("Couldn't find player for Id %d.", follow.playerId );
   }
   else
   {
      console->printf("AI::DirectiveFollow <ai wildcard> <playerID> [Pt/Rad] [Ord]");
      console->printf("Instructs the given AI(s) to follow the specified player.");
      console->printf("Ord is used for sorting directives. The optional point/radius");
      console->printf("(and note that the point can be given alone) tells the AI when");
      console->printf("to stop or move on to his or her next directive.");
   }
   return falseTxt;
}


const char * AIPlugin::onWaypointDirective(AIObj * ai, int argc, const char *argv[])
{
   if( argc > 0 )
   {
      AIObj::Waypoint   waypoint;
      waypoint.getPointAndRadius( argv[0] );
      if( argc > 1 )
         waypoint.order = atoi(argv[1]);
      
      ai->addWaypoint ( waypoint );
   }
   else
   {
      console->printf("AI::DirectiveWaypoint: <ai wildcard> <pt3F [Rad]> [ord]");
      console->printf("Adds the given waypoint to the directive list of any AI that" );
      console->printf("that matches the wildcard specification.  The Radius is an optional" );
      console->printf("addition in the point.  The optional Order number is used for" );
      console->printf("sorting of the directives.  If not given, the waypoint will be" );
      console->printf("put at the end of the directive list(s)." );
      return falseTxt;
   }
   return trueTxt;
}

const char * AIPlugin::onTargetDirective(AIObj * ai, int argc, const char *argv[])
{
   if( argc > 0 )
   {
      AIObj::Target  target;
      
      if( (target.playerId = atoi(argv[0])) != ai->repId )
      {
         if( argc > 1 )
         {
            target.order = atoi(argv[1]);
            //if( argc > 2 )
            //   target.pri = atoi(argv[2]);
         }
         ai->addTarget ( target );
         return trueTxt;
      }
      else// Added self as a target...
         console->printf( "An AI may not have itself as a target, directive not added." );
   }
   else
   {
      console->printf( "AI::DirectiveTarget: <ai wildcard> <player ID> [ord]" );
   }
   return falseTxt;
}

const char * AIPlugin::onTargetDirectiveLaser(AIObj * ai, int argc, const char *argv[])
{
   if( argc > 0 ){
      AIObj::Target  target(AIObj::Target::laser);
      if( (target.playerId = atoi(argv[0])) != ai->repId ){
         if( argc > 1 ){
            target.order = atoi(argv[1]);
            if( argc > 2 )
               target.getPointAndRadius( argv[2] );
         }
         ai->addTarget ( target );
         return trueTxt;
      }
      else// Added self as a target...
         console->printf( "OwnerId %d refers to SELF. Not installed.", ai->repId );
   }
   else{
      console->printf("AI::DirectiveTargetLaser: <ai wildcard> <owner Id> [ord] [point+R]");
      console->printf("Aim at the point targeted by the given Id.  If the owner Id is");
      console->printf("zero then the Ai will look for any laser 'paint' that can be hit.");
      console->printf("The optional point+radius gives a sphere inside that the paint");
      console->printf("must be inside of for the AI to fire (cf. weapons training).");
   }
   return falseTxt;
}
const char * AIPlugin::onTargetDirectivePoint(AIObj * ai, int argc, const char *argv[])
{
   if( argc > 0 )
   {
      AIObj::Target  target(AIObj::Target::point);

      target.getPointAndRadius( argv[0] );
      if( argc > 1 )
         target.order = atoi(argv[1]);
      ai->addTarget ( target );
      return trueTxt;
   }
   else{
      console->printf("AI::DirectiveTargetPoint: <ai wildcard> <point> [ord]" );
      console->printf("Assign the given point as a target.  The AI will fire at this" );
      console->printf("point as long as the target current and LOS exists. First" );
      console->printf("intended use: painting a target with the target laser." );
   }
   return falseTxt;
}


const char * AIPlugin::onGuardDirective(AIObj * ai, int argc, const char *argv[])
{
   ai;   argc;   argv;
   // console->printf("AI::DirectiveGuard: <ai wildcard> <..?..>");
   // console->printf("     [ not implemented ] " );
   return falseTxt;
}


const char * AIPlugin::onRemoveDirective(AIObj * ai, int argc, const char *argv[])
{
   if( argc == 1 )
   {
      if( !ai->removeDirectives( argv[0] ) )
         console->printf("Directive #%s not found for %s", argv[0], ai->name );
      else
         return trueTxt;
   }
   else
   {
      console->printf( "AI::DirectiveRemove( <ai wildcard> <order # key> )" );
   }
      
   return falseTxt;
}

const char * AIPlugin::onListDirectives(AIObj * ai, int argc, const char *argv[])
{
   if( ai != NULL )
   {
      if( argc > 0 ){
         int   mask = atoi(argv[0]);
         console->printf("Listing directives with bitset mask of %d:", mask );
         ai->listDirectives(mask);
      }
      else{
         console->printf("Listing all directives (no bitset mask given):" );
         ai->listDirectives();
      }
      return trueTxt;
   }
   else
   {
      console->printf("AI::DirectiveList( <ai wildcard> [,directive bitset mask] );" );
      console->printf("For each AI matching the given wildcard, this lists its" );
      console->printf("directives.  If a mask is given, then only those which match" );
      console->printf("the given types will be listed. See AI::DirectiveCallback1()" );
      console->printf("for mask definitions." );
      return falseTxt;
   }
}


//-------------------------------------------------------------------------------------
// Regular AI callback methods.
//

const char * AIPlugin::onCallbackDied( AIObj * ai, int argc, const char *argv[] )
{
   if( ai && argc > 0 ){
      ai->installCB( AIObj::AiDied, argv[0] );
      return trueTxt;
   }
   else{
      console->printf( "AI::CallbackDied: <ai wildcard> <script function name>" );
      console->printf( "Install the given death callback on the specified AI objects." );
      return falseTxt;
   }
}
const char * AIPlugin::onCallbackBored( AIObj * ai, int argc, const char *argv[] )
{
   if( ai && argc > 0 ){
      ai->installCB( AIObj::AiBored, argv[0] );
      return trueTxt;
   }
   else{
      console->printf( "AI::CallbackBored: <ai wildcard> <script function name>" );
      console->printf( "Install the given bored function on the specified AI objects.  This" );
      console->printf( "will be called periodically when the AI has no purpose." );
      return falseTxt;
   }
}

const char * AIPlugin::onCallbackAmbient( AIObj * ai, int argc, const char *argv[] )
{
   ai; argc; argv;
   console->printf( "...unimplemented" );
   return falseTxt;
}
const char * AIPlugin::onCallbackPeriodic( AIObj * ai, int argc, const char *argv[] )
{
   if( ai && argc > 0 ){
      float    freqDelay = atof( argv[0] );
      ai->setPeriodicDelay( freqDelay );
      if( argc > 1 )
         ai->installCB( AIObj::AiPeriodic, argv[1] );
      return trueTxt;
   }
   else{
      console->printf( "AI::CallbackPeriodic: <ai wildcard> <freqDelay> [,function name]" );
      console->printf( "Set up a periodic callback with the given frequency (delay between" );
      console->printf( "calls given in seconds).  If no function name is given then the" );
      console->printf( "default of AI::onPeriodic() is called with the (script) name of the AI" );
      console->printf( "as a parameter. For sake of efficiency, the frequency must be at least" );
      console->printf( "one.  Smaller values will disable periodic callbacks for the AIs" );
      console->printf( "specified (they are disabled by default)." );
      return falseTxt;
   }
}

//-------------------------------------------------------------------------------------
// Directive callback methods.  Each directive has three callbacks, with the meaning
//    of each callback depending on the nature of the directive.  For example - 
//       a target directive will have a callback for when a target is first 
//       acquired (callback 1), and a callback2 for when the target is destroyed.  

const char * AIPlugin::installDCB(AIObj * ai, int whichDCB, int argc, const char *argv[])
{
   if( argc > 0 )
   {
      const char * cbWildcard = 0, * errTxt;
      int   dirFilter = 0xffff;
      
      if( argc > 1 ){
         cbWildcard = argv[1];
         if( argc > 2 )
            dirFilter = atoi( argv[2] );
      }
      errTxt = ai->installDCB( whichDCB, argv[0], cbWildcard, dirFilter );
      if( ! errTxt )
         return trueTxt;
      
      console->printf( "ERROR: %s...  Note Usage:", errTxt );
   }
   
   console->printf( 
      "AI::DirectiveCallback%d( <ai wildcard> <CB name> [<CB# wildcard>] [<directive filter>] )",
                                    whichDCB );
   console->printf( "  <CB# wildcard> gives which directive order#(s) to install to." );
   console->printf( "        If none (or 0) is given, the last directive is assumed." );
   console->printf( "  <directive filter> gives a mask to limit directive types:" );
   console->printf( "        Waypoint=%d, Follow=%d, Target=%d.", 
                                 AIObj::WaypointDirective, 
                                 AIObj::FollowDirective,
                                 AIObj::TargetDirective  /* ,AIObj::GuardDirective */
                                 );
   return falseTxt;
}


const char * AIPlugin::onDirectiveCallback1(AIObj * ai, int argc, const char *argv[])
{
   return installDCB( ai, 1, argc, argv );
}
const char * AIPlugin::onDirectiveCallback2(AIObj * ai, int argc, const char *argv[])
{
   return installDCB( ai, 2, argc, argv );
}
const char * AIPlugin::onDirectiveCallback3(AIObj * ai, int argc, const char *argv[])
{
   return installDCB( ai, 3, argc, argv );
}
const char * AIPlugin::onDirectiveCallback4(AIObj * ai, int argc, const char *argv[])
{
   return installDCB( ai, 4, argc, argv );
}

// We provide this method so that script can set internal variables of all AI objects 
//    that match a wildcard name.  If only one argument is given, then print out 
//    the value of the variable.  
const char * AIPlugin::onSetVariable(AIObj * ai, int argc, const char *argv[])
{
   if( argc > 1 ){
      ai->setVar( console, argv[0], argv[1] );
   }
   else if ( argc > 0 ){
      console->printf( "Value of %s for %s = %s", 
                     argv[0], 
                     ai->name, 
                     ai->getVar( console, argv[0] ) 
                     ); 
   }
   else{
      console->printf("AI::setVar( <ai wildcard>, <variable name> [, <value>] )");
      console->printf("Command to set the given AI variable to <value> in all AIs that match");
      console->printf("the wild card spec.  e.g. ai::setVar( \"a*\", iq, 20 ); ");
      console->printf("If no value is provided, the current value is printed.");
      return falseTxt;
   }
   return trueTxt;
}

// AI command to call down to any script function taking a repId as a first parameter.
// Example:
//       AI::CallWithId( "*", Player::setItemCount, GrenadeAmmo, 1000 );
// Will call the following on all AI objects using the proper rep ID:
//       Player::setItemCount ( <Rep Id>, GrenadeAmmo, 100 );
//    
const char * AIPlugin::onCallWithId(AIObj * ai, int argc, const char * argv[])
{
   if( argc > 0 ){
      const char * ArgV[101];
      if( argc <= 100 )
      {
         ArgV[0] = argv[0];
         ArgV[1] = intToStr( ai->repId );
         if( argc > 1 )
            memcpy( & ArgV[2], & argv[1], (argc - 1) * sizeof(argv[0]) );
         console->execute( argc + 1, ArgV );
         return trueTxt;
      }
      else
         console->printf( "%s has too many args (99 is maximum). Not invoked.", argv[0] );
   }
   else{
      console->printf( 
         "AI::CallWithId( <ai wildcard>, <Console command with ID arg> [,arg 1] [...] );"    );
      console->printf( 
         "AI::CallWithId() can be used to invoke a console command for every AI object"   );
      console->printf( 
         "that matches the wildcard spec.  The console command must be one which takes"   );
      console->printf( 
         "a rep Id as its first argument." );
   }
   return falseTxt;
}


//-------------------------------------------------------------------------------------
// These two functions are opposites.  Scripted targets is the default mode and it
// requires the scriptor specify all targets to a given AI.  In Automatic mode
// the AI code will put all different team clients into the target list.  
//
// Scripted is the default since that's what the original game is released with
// and the training missions likely depend on it.  

const char * AIPlugin::onSetAutomaticTargets(AIObj * ai, int, const char * * )
{
   if( ai != NULL )
   {
      ai->setAutomaticTargets();
      return trueTxt;
   }
   else{
      console->printf( "AI::SetAutomaticTargets( <ai wildcard> );"  );
      return falseTxt;
   }
}

const char * AIPlugin::onSetSciptedTargets(AIObj * ai, int, const char * * )
{
   if( ai != NULL )
   {
      ai->setScriptedTargets();
      return trueTxt;
   }
   else{
      console->printf( "AI::SetScriptedTargets( <ai wildcard> );"  );
      console->printf( 
         "Causes the specified AI objects to only attack those targets which they" );
      console->printf( 
         "have been scripted to attack, and none others. This is the default mode." );
      console->printf( 
         "The opposite function is AI::SetAutomaticTargets()" );
      return falseTxt;
   }
}

//-------------------------------------------------------------------------------------

#if INCLUDE_AI_GRAPH_CODE

//-------------------------------------------------------------------------------------

// Add the given node to the weighted graph, and return the index of the node.  
const char * AIPlugin::onGraphAddNode(int argc, const char **argv)
{
   if( argc > 1 )
   {
      Point3F  pos;
      int      maxDirs = AIGraph::MaxDirections;
      
      sscanf(argv[1], scan3fTxt, & pos.x, & pos.y, & pos.z);
      if( argc > 2 )
         maxDirs = atoi( argv[2] );
      
      int index = aim->graph.addNode(pos, maxDirs);
      if( index < 0 )
         console->printf( "Add node failed. Too close to another?" );
      
      return intToStr(index);
   }
   console->printf( "Graph::AddNode( <point> [,max directions] );" );
   return intToStr(-1);
}

const char * AIPlugin::onGraphNodeCount(int, const char * *)
{
   return intToStr( aim->graph.nodeListSize() );
}

const char * AIPlugin::onGraphLoadNode(int argc, const char **argv)
{
   if( argc == 2 )
   {
      int   index = aim->graph.addNode(argv[1]);
      if( index < 0 )
         console->printf( "Load node failed. Too close to another?" );
      return intToStr(index);
   }
   console->printf( "Graph::LoadNode( <node inoformation string> );" );
   return intToStr(-1);
}

const char * AIPlugin::onGraphPrintNode(int argc, const char **argv)
{
   if( argc == 2 )
   {
      int   which = atoi(argv[1]);
      int   total = aim->graph.nodeListSize();
      
      if( which < total && which >= 0 )
      {
         char buff[256];
         console->printf( aim->graph.printNode(buff,which) );
         return trueTxt;
      }
      else
         console->printf( "...%d is out of range...", which );
   }
   console->printf( "Graph::PrintNode( <index of node> );" );
   console->printf( "Prints out relevant information for the given node. Mirrors" );
   console->printf( "the routine Graph::LoadNode()." );
   return falseTxt;
}


#endif



//----------------------------------------------------------------------------

AIPlugin::AIPlugin()
{
   aim = 0;
}
AIPlugin::~AIPlugin()
{
}


void AIPlugin::startFrame()   {}
void AIPlugin::endFrame()     {}

//----------------------------------------------------------------------------

enum CallbackID
{
   SpawnAI,
   List,
   GetAICount,
   GetId,
   GetTarget,
   FetchObject,
   AttackPlayer,
   DeleteAI,
   SetVariable,
   CallWithId,
   CallbackDied,
   CallbackPeriodic,
   
   // Directive commands:  
   FollowDirective,
   WaypointDirective,
   TargetDirective,
   TargetDirectiveLaser,
   TargetDirectivePoint,
   //GuardDirective,
   RemoveDirective,
   ListDirectives,
   DirectiveCallback1,
   DirectiveCallback2,
   DirectiveCallback3,
   DirectiveCallback4,
   
   SetAutomaticTargets,
   SetSciptedTargets,
   
#if INCLUDE_AI_GRAPH_CODE
   GraphAddNode,
   GraphNodeCount,
   GraphLoadNode,
   GraphPrintNode,
#endif
};


void AIPlugin::init()
{
   console->printf("AIPlugin");

   console->addCommand( SpawnAI,             "AI::Spawn",               this, 1);
   console->addCommand( List,                "AI::List",                this, 1);
   console->addCommand( GetAICount,          "AI::GetCount",            this, 1);
   console->addCommand( GetId,               "AI::GetId",               this, 1);
   console->addCommand( GetTarget,           "AI::GetTarget",           this, 1);
   console->addCommand( FetchObject,         "AI::GetObject",           this, 1);
   
   console->addCommand( AttackPlayer,        "AI::Attack",              this, 1);
   console->addCommand( DeleteAI,            "AI::Delete",              this, 1);

   console->addCommand( WaypointDirective,   "AI::DirectiveWaypoint",   this, 1);
   console->addCommand( FollowDirective,     "AI::DirectiveFollow",     this, 1);
   console->addCommand( TargetDirective,     "AI::DirectiveTarget",     this, 1);
   console->addCommand( TargetDirectiveLaser,"AI::DirectiveTargetLaser",this, 1);
   console->addCommand( TargetDirectivePoint,"AI::DirectiveTargetPoint",this, 1);
   //console->addCommand( GuardDirective,      "AI::DirectiveGuard",      this, 1);
   console->addCommand( RemoveDirective,     "AI::DirectiveRemove",     this, 1);
   console->addCommand( ListDirectives,      "AI::DirectiveList",       this, 1);
   
   console->addCommand( CallbackDied,        "AI::CallbackDied",        this, 1);
   console->addCommand( CallbackPeriodic,    "AI::CallbackPeriodic",    this, 1);
   
   console->addCommand( DirectiveCallback1,  "AI::DirectiveCallback1",  this, 1);
   console->addCommand( DirectiveCallback2,  "AI::DirectiveCallback2",  this, 1);
   console->addCommand( DirectiveCallback3,  "AI::DirectiveCallback3",  this, 1);
   console->addCommand( DirectiveCallback4,  "AI::DirectiveCallback4",  this, 1);
   console->addCommand( CallWithId,          "AI::CallWithId",          this, 1);
   console->addCommand( SetVariable,         "AI::SetVar",              this, 1);
   
   console->addCommand( SetAutomaticTargets, "AI::SetAutomaticTargets", this, 1);
   console->addCommand( SetSciptedTargets,   "AI::SetScriptedTargets",  this, 1);
   
#if INCLUDE_AI_GRAPH_CODE
   console->addCommand( GraphAddNode,        "Graph::AddNode",          this, 1);
   console->addCommand( GraphNodeCount,      "Graph::NodeCount",        this, 1);
   console->addCommand( GraphLoadNode,       "Graph::LoadNode",         this, 1);
   console->addCommand( GraphPrintNode,      "Graph::PrintNode",        this, 1);
#endif

   console->addCommand(0,  "BaseRep::getFirst",    c_baseRepGetFirst);
   console->addCommand(0,  "BaseRep::getNext",     c_baseRepGetNext);
   console->addCommand(0,  "Object::getName",      c_objectGetName);
}


#define onCommand(T) \
   case T : return on##T (argc, argv);
   
#define onListableCommand(T) \
   case T : listableCommand = & AIPlugin::on##T;  break;


const char * AIPlugin::consoleCallback(CMDConsole*,int id,int argc,const char *argv[])
{
   const char     * returnTxt = 0;
   HandleCmdFunc  listableCommand = 0;

   if( (aim = AIManager::it) == NULL )
   {
      if( sg.manager )
      {
         SimGroup * cleanupGroup = (SimGroup *) sg.manager->findObject(missionCleanup);
      
         if( cleanupGroup == NULL ){
            console->printf( "No cleanup group yet, can't install AI manager and, "
                     "therefore, cannot execute command %s", argv[0] );
            return returnTxt;
         }
         
         aim = new AIManager();
         AssertFatal( aim, "ai: couldn't new() the AIManager." );
         
         if( ! sg.manager->registerObject(aim) ){
            delete aim;
            aim = 0;
            console->printf( "Couldn't register AI manager...  "
                     "Cannot execute command %s", argv[0] );
            return returnTxt;
         }
         else{//success
            sg.manager->assignId(aim,AIManagerId);
            cleanupGroup->addObject(aim);
         }
      }
      else{
         console->printf("No server manager yet, can't install AI manager thus, "
                     "cannot execute command %s.", argv[0] );
         return returnTxt;
      }
   }

   switch( id )
   {
      onCommand(SpawnAI);                          // create an AI.  
      onCommand(List);                             // list all AIs in manager.  
      onCommand(GetAICount);                       // return list size.
      onCommand(GetId);                            // return rep Id.  
      onCommand(GetTarget);                        // return rep Id.  
      onCommand(FetchObject);                      // return SimObjectId

      onListableCommand(AttackPlayer);
      onListableCommand(DeleteAI);
      
      onListableCommand(CallWithId);
      onListableCommand(SetVariable);
      
      onListableCommand(FollowDirective);          // follow the specified player 
      onListableCommand(WaypointDirective);        // add waypoint to list
      onListableCommand(TargetDirective);          // add Target (player rep Id) to list.
      onListableCommand(TargetDirectiveLaser);     //    Use Laser as target (Pt Ok).  
      onListableCommand(TargetDirectivePoint);     //    Fire at given Point.
      //onListableCommand(GuardDirective);         // guard this point or player.  
      onListableCommand(RemoveDirective);          //    remove using order number.
      onListableCommand(ListDirectives);
      onListableCommand(DirectiveCallback1);
      onListableCommand(DirectiveCallback2);
      onListableCommand(DirectiveCallback3);
      onListableCommand(DirectiveCallback4);
      
      onListableCommand(CallbackDied);
      onListableCommand(CallbackPeriodic);
      onListableCommand(SetAutomaticTargets);
      onListableCommand(SetSciptedTargets);
      
#     if INCLUDE_AI_GRAPH_CODE
      onCommand(GraphAddNode);
      onCommand(GraphNodeCount);
      onCommand(GraphLoadNode);
      onCommand(GraphPrintNode);
#     endif
   }

   if( aim )
   {
      if( listableCommand )
      {
         if( argc > 1 )
         {
            // Execute command an all AIs which match the name spec.  
            AIManager::iterator itr;
            for( itr = aim->begin(); itr != aim->end(); itr++ )
               if( (*itr)->nameMatches( argv[1] ) )
                  returnTxt = (this->*listableCommand)( *itr, argc-2, argv+2 );
         }
         else
         {
            // Just give help:
            returnTxt = (this->*listableCommand)( NULL, 0, 0 );
         }
      }
   }
   
   return returnTxt;
}

