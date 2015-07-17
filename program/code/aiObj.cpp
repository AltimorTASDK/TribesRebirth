//-----------------------------------------------------------------------------------//
//                                                                                   //
// An AI object controls one Player object.  The AI Manager maintains                //
//    the list of AI objects.  These are the two main classes (The AOIbj, and        //
//       the AIManager list class).  The AIObj class then has within it              //
//       some subsidiary supporting classes, such as:                                //
//                                                                                   //
//    Directive:           a command an AI can follow (path, target, patrol point),  //
//                         kept as a list ("directives") ordered by "order".         //
//    IQVariables:         skill variables.  scaled off of iqScale.                  //
//    Misc math            helper     structures.                                    //
//                                                                                   //
// The command AI::spawn() creates an AIObj/Player pair. See AIPlugin.cpp for        //
//    the commands.  We also create console variables that are installed for each    //
//    AI with the names being of the form:  AI::<ai's given name>::<variable name>   //
//                                                                                   //
// Additional AI support is provided in base/scripts/AI.CS - ideally we script as    //
//    much as possible.                                                              //
//                                                                                   //
// Each object has a state, and there are also "subsidiary" state flags that can     //
//    modify each behavior (for example, an AI can travel on its path while          //
//    shooting at its target, as an evasive tactic, and the boolean tested is set    //
//    independently sort of in parallel).                                            //
//                                                                                   //
//-----------------------------------------------------------------------------------//

#ifdef _BORLANDC_
#pragma warn -inl
#pragma warn -aus
#endif

#ifdef _MSC_VER
#pragma warning(disable : 4710)
#pragma warning(disable : 4101)
#endif

#include <sim.h>
#include <console.h>

#include "fearDcl.h"
#include "DataBlockManager.h"
#include "fearGlobals.h"
#include "PlayerManager.h"
#include "ProjTargetLaser.h"
#include "aiObj.h"

#include "stringTable.h"


//---------------------------------------------------------------------------------------
//    Data and defines
//

Random         AIObj::rand;
const Int32 AIObj::collisionMask = -1 & ~(SimProjectileObjectType    |
                                          SimCameraObjectType        |
                                          ItemObjectType);


// Take a distance, choosing accuracy based off the IQ (unless forceAccurate is 
//    set).  2d distance is in X and Y.  Accuracy can be forced by calling 
//    setFrameAccurate(), which will cause these functions to do accurate math for
//    the remainder of the frame.  For example, things like jetting code would want 
//    accurate math always.  
float AIObj::dist2d( const Point2F & loc1, const Point2F & loc2, bool forceAccurate )
{
   if( forceAccurate || isFrameAccurate() )
      return m_dist( loc1, loc2 );
   else
      return m_distf( loc1, loc2 );
}
float AIObj::dist3d( const Point3F & loc1, const Point3F & loc2, bool forceAccurate )
{
   if( forceAccurate || isFrameAccurate() )
      return m_dist( loc1, loc2 );
   else
      return m_distf( loc1, loc2 );
}


// angle defines for pitching. 
#define   PITCH_SCALE   3.0
static float min_pitch_increment    = 0.01f * PITCH_SCALE; 
static float pitch_increment        = 0.028 * PITCH_SCALE;
static float reduce_pitch_threshold = 0.056 * PITCH_SCALE;


//---------------------------------------------------------------------------------------
//    AIObj construction, deletion, and SimObject() virtual overrides.  
//

AIObj::AIObj() :  seekLoc(0,0,0), lastWaypointLoc(0,0,0),
                  seekOff(3.0, 3.0, 0.0 ), 
                  periodicCbWait(0)
{
   type = AIObjectType;
   didFirstTimeSetup = false;
   
   flags.clear();
   timeHere = 0;
   pathIndex = 0;
   pathDirection = 1;
   seekFlag = 0;
   counter = 0;
   active=  false;
   doneWithPath = false;
   
   memset(name, 0, sizeof(name));
   
   pathType = Circular;
   state = NeedToThink;
   
   player = NULL;
   repId = -1;

   // target variables:   
   attackMode = WhileFollowingTarget;
   currentTarget = NULL;
   currentTargetId = -1;
   lastLosCheckId = 0;
   
   projectileSpeed = 0.0f;
   iqPercent = 100.0 * (iqScale = 0.0);
   wrappedFireAngle = 0;
   jetNavigation = 0.0f;
   
   memset( CBs, 0, sizeof(CBs) );
   
   setDefaultCallbacks();
   
   voice = NULL;
   skin = NULL;
   
   targetingMode = ScriptedTargeting;
   ourTeam = 0;
   
   jetStage = -1;
   jetSaveLinearVel.set();
   
   m_jsPtr = NULL;
}

AIObj::~AIObj()
{
   if( flags.test( ConsoleVarsInstalled ) )
      removeVariables(NULL);
}

//---------------------------------------------------------------------------------------
//    Console variables.  
//
// We set up console variables for the given AI using its name.  The 
//    format of a variable called myVar is:
//       AI::<InstanceName>::myVar.

static const char iqTxt[] =         "iq";
static const char attackModeTxt[] = "attackMode";
static const char pathTypeTxt[] =   "pathType";
static const char spotDistTxt[] =   "spotDist";
static const char seekOffTxt[] =    "seekOff";
static const char tWindupTxt[] =    "triggerWindup";
static const char tPercentTxt[] =   "triggerPct";
static const char smartRangeTxt[] = "smartGuyWeaponRange";
static const char dumbRangeTxt[] =  "dumbGuyWeaponRange";
static const char accuracyTxt[] =   "smartGuyMinAccuracy";
static const char jettingTxt[] =    "jetNavigation";

char * AIObj::makeVarName( char buf[VarNameBufSize], const char * vName )
{
   AssertFatal(strlen(vName)+strlen(name)+10 < VarNameBufSize, "AI::strlen" );
   sprintf( buf, "AI::%s::%s", name, vName );
   return buf;
}

void  AIObj::addVariables( CMDConsole * /*console*/ )
{
   if( flags.test( ConsoleVarsInstalled ) )
      return;
   
   char  varName[VarNameBufSize];
   Console->addVariable(0, 
      makeVarName(varName,iqTxt), CMDConsole::Float, & iqPercent );
   Console->addVariable(0,       varName, AIManager::doRecomputation );
   
   Console->addVariable(0, 
      makeVarName(varName,pathTypeTxt), CMDConsole::Int, & pathType );
   Console->addVariable(0, 
      makeVarName(varName,spotDistTxt), CMDConsole::Float, & iq.spotDistOverride );
   Console->addVariable(0, 
      makeVarName(varName,seekOffTxt), CMDConsole::Point3F, & seekOff);
   Console->addVariable(0, 
      makeVarName(varName,attackModeTxt), CMDConsole::Int, & attackMode );
   
   Console->addVariable(0, 
      makeVarName(varName,tWindupTxt), CMDConsole::Int, & trigger.windup );
   Console->addVariable(0, 
      makeVarName(varName,tPercentTxt), CMDConsole::Unit, & trigger.percent );
      
   Console->addVariable(0, 
      makeVarName(varName,smartRangeTxt), CMDConsole::Float, & iq.smartGuyWeaponRange );
   Console->addVariable(0, varName, AIManager::doRecomputation );
   Console->addVariable(0, 
      makeVarName(varName,dumbRangeTxt),  CMDConsole::Float, & iq.dumbGuyWeaponRange );
   Console->addVariable(0, varName, AIManager::doRecomputation );
   Console->addVariable(0, 
      makeVarName(varName,accuracyTxt),   CMDConsole::Float, & iq.smartGuyMinAccuracy );
   Console->addVariable(0, varName, AIManager::doRecomputation );
   
   Console->addVariable(0, 
      makeVarName(varName,jettingTxt), CMDConsole::Float, & jetNavigation );
      
   flags.set( ConsoleVarsInstalled );
}

bool AIObj::removeVariables( CMDConsole * /*console*/ )
{
   bool  ok = false;
   if( flags.test( ConsoleVarsInstalled ) )
   {
      char  varName[VarNameBufSize];
      
      ok =  Console->removeVariable( makeVarName(varName,iqTxt) );
      ok &= Console->removeVariable( makeVarName(varName,pathTypeTxt) );
      ok &= Console->removeVariable( makeVarName(varName,spotDistTxt) );
      ok &= Console->removeVariable( makeVarName(varName,seekOffTxt) );
      ok &= Console->removeVariable( makeVarName(varName,attackModeTxt) );
      ok &= Console->removeVariable( makeVarName(varName,tWindupTxt) );
      ok &= Console->removeVariable( makeVarName(varName,tPercentTxt) );
      ok &= Console->removeVariable( makeVarName(varName,smartRangeTxt) );
      ok &= Console->removeVariable( makeVarName(varName,dumbRangeTxt) );
      ok &= Console->removeVariable( makeVarName(varName,accuracyTxt) );
      ok &= Console->removeVariable( makeVarName(varName,jettingTxt) );
      
      AssertFatal( ok, "AI: Not Ok" );
      flags.clear( ConsoleVarsInstalled );
   }
   //return a value to remove RB compile warning:
   return ok;
}

void  AIObj::setVar( CMDConsole * console, const char * var, const char * value )
{
   if( flags.test( ConsoleVarsInstalled ) )
   {
      char  varName[VarNameBufSize];
      makeVarName( varName, var );
      console->setVariable( varName, value );
   }
}

const char * AIObj::getVar( CMDConsole * console, const char * var )
{
   if( flags.test( ConsoleVarsInstalled ) )
   {
      char  varName[VarNameBufSize];
      makeVarName( varName, var );
      return console->getVariable( varName );
   }
   else
      return "";
}



//---------------------------------------------------------------------------------
//       Directive construction and addition, removal, info queries, etc.  
//


// Called for each AI when certain console variable are changed.  
void AIObj::doRecomputations(void)
{
   if( (iqScale = iqPercent/100.0) < 0.0 )
      iqScale = 0.0;
   else if( iqScale > 1.0 )
      iqScale = 1.0;

   iq.scale( iqScale );
   
   // losCheck.init( (iq.thinkWait >> 1) | 1 );
   losCheck.init( 5 );
}

#define  havePath()  (findDirective(-1,WaypointDirective|FollowDirective,true) != -1)

AIObj::Waypoint * AIObj::getWaypoint( int order )
{
   int   ind = findDirective( order, WaypointDirective|FollowDirective );
   if( ind >= 0 )
      return directives[ind];
   return 0;
}


// Get the next waypoint beyond the one given by the path index, if there is one.
// Handle the three path types.  
//
// This sets the new path index variable for the AI if next was found.  
AIObj::Waypoint * AIObj::setNextWaypoint( int previousPathIndex )
{
   Waypoint    * w = NULL, * oldWaypoint = NULL;
   int         WaypointMask = WaypointDirective|FollowDirective;
   int         x = findDirective( previousPathIndex, WaypointMask );

   if( x != -1 )
      oldWaypoint = directives[x];
   
   if( ! (pathType == TwoWay) )
   {
      if( x != -1 )
      {
         int   N = directives.size();
         int   L = N - 1;

         while( L-- )                  // Find next waypoint in the other N-1 elements.  
         {
            int   next = (x + 1) % N;
            if( next<x && !circularPath() )
               break;
            if( directives[next].type & WaypointMask )
            {
               w = directives[next];
               break;
            }
            x = next;
         }
      }
      else if( (x = findDirective( previousPathIndex, WaypointMask, true )) != -1 )
         w = directives[x];
      else{
         // handle this in case the current waypoint has been removed but the path is 
         //    cyclic.  
      }
   }
   else
   {
      // we have a two way path. Checking for direction and finding next waypiont
      int   indices[MaximumDirectives], count;
      int   numWaypoints = getDirectiveList( indices, WaypointMask );
      
      for( count = 0; count < numWaypoints; count++ )
         if(directives[indices[count]].order == previousPathIndex)
            break;
            
      bool  found = (count < numWaypoints);
      
      if( numWaypoints > 1 )
      {
         //We don't have a match with previousPathIndex.
         //either the current node has been removed or AI has just been initialized
         if(!found)
         {
            if(directives[numWaypoints - 1].order < previousPathIndex)
            {	
            	//case#1
               //we are at the end of the list
            	pathDirection = -1;
               w = directives[numWaypoints - 1];
            }
            else if(directives[0].order >	previousPathIndex)
            {	
               //case#2 -  we are at the start of the list
               pathDirection = 1;
               w = directives[0];
            }
            else
            {
               //case#3 -  we are somewhere in the middle of the list - find which
               //          waypoints we're between and choose the right one.  
               for(int i = 0; i < numWaypoints - 1; i++)
               {
                  Directive & lo   = directives[indices[i]];
                  Directive & hi   = directives[indices[i+1]];
                  
                  AssertFatal( lo.order < hi.order, "aiObj: directives out of order" );
                  
                  if( hi.order > previousPathIndex && lo.order < previousPathIndex )
                  {
                     w = (pathDirection == -1 ? lo : hi);
                     break;
                  }
            	}
               AssertFatal( w != NULL, "aiObj: should have found a new path dest" );
            }   
         }
         else
         {
	         if(count == (numWaypoints - 1))
	         	pathDirection = -1;
	         else if(count == 0)
	            pathDirection = 1;
            	
	         w = directives[indices[count + pathDirection]];   
         }      
      }
      else if( numWaypoints == 1 )
      {
         // Case where there is only one waypoint in the (TwoWay) path list.  If this 
         // one is different than the last one, then it's our new one.  Otherwise
         // we're done (fall out with w left at NULL).  
         if( ! found )
         {
            AssertFatal( count == 1, "aiObj: bad invariant" );
            w = directives[indices[0]];
         }
      }
   }
   
   m_jsPtr = NULL;
      
   // Note when we're doneWithPath we still retain the pathIndex as it indicates the 
   // last visited place - other directives can be added and then we go from there. 
   if( w )
   {
      pathIndex = w->order;
      if( oldWaypoint )
      {
         lastWaypointLoc = getFollowDestination( oldWaypoint );
         // Set up path following state.  
      }
      else
         lastWaypointLoc = ourCenter();
         
      if( jetNavigation )
      {
         JetSkill    initSkill( lastWaypointLoc, getFollowDestination( w ) );
         m_jetSkill = initSkill;
         m_jsPtr = & m_jetSkill;
      }
   }
   else
      doneWithPath = true; 

   return w;
}


// Get an actual seek location for a (Waypoint | Follow) directive.  
SphereF  AIObj::getFollowDestination( Waypoint * w )
{
   SphereF  sphere;
   
   sphere.radius = 0.0f;
   
   if( w->type & WaypointDirective )
      sphere.center = w->pos;
   else
   {
      Player * p = findPlayerObject( w->playerId );
      // AssertFatal( p, "AI: player id not found" );
      if( p ){
         sphere.center = p->getLeadCenter();
         sphere.center += seekOff;
      }
      else{
         // target might not be found, so seek ourselves for now.  
         sphere.center = ourCenter();
      }
   }
   return sphere;
}


//---------------------------------------------------------------------------------------
//    State handling, decision making.  
//

bool AIObj::timeToThink( bool force )
{
   if(force || ++counter >= iq.thinkWait)
      counter = 0;
   return !counter;
}


// Target method called to see if LOS is possible at all from the given AI to 
// the target in question.  Decision depends on the target type.  
bool AIObj::Target::canGetLosTo( AIObj * fromAi )
{
   if( isPlayer() )
   {
      if( Player * p = findPlayerObject(playerId) )
         if( ! p->isDead() )
         {
            float d = fromAi->spottingDistance();
            if( m_distf(fromAi->ourCenter(), p->getLeadCenter()) < d )
               return true;
         }
      return false;
   }
   return true;      // other target types return true.  
}         


// Set the target for the actor.  Meant to be called at least once per timer tick
// at the beginning of it.  
//
// This performs any target callbacks that were instantiated by the script.  
// 
void AIObj::scanTargetList(void)
{
   Target   * t;
   int      indices[MaximumDirectives], count, x;
   
   if( (count = getDirectiveList( indices, TargetDirective )) != 0 )
   {
      for( x = 0; x < count; x++ )
         if( (t = directives[ indices[x] ])->canGetLosTo( this ) )
            t->flags.set( ValidLOSCandidate );
         else
            t->flags.clear( HaveTargetLOS | HadTargetLOS | ValidLOSCandidate );
         
      if( losCheck.timeToThink() )
      {
         int   lastCheckIndex, nextCheckIndex;
         
         // find last index we did LOS on, if any.  
         lastCheckIndex = -1;
         if( lastLosCheckId )    // find the last one checked, if there, for circling
            for( x = 0; x < count; x++ )
               if( directives[indices[x]].playerId == lastLosCheckId )
               {
                  lastCheckIndex = x;
                  break;
               }
         nextCheckIndex = lastCheckIndex = ((lastCheckIndex + 1) % count);

         // Find next alive target, up through the current, for doing our LOS check on.  
         // Do a circular loop.  
         lastLosCheckId = 0;
         do
         {
            t = directives[ indices[ nextCheckIndex ] ];
            // DO: getTargetLocation():
            //    For   player - just do this code.  
            //          laser  - scan targetable set and check.
            //          point  - see if we have view to this point.  Or just fire at it?
            // if( t->canGetLosTo( this ) )
            if( t->flags.test(ValidLOSCandidate) )
            {
               // found target to do LOS on.  Do the check, perform any needed callbacks
               // and exit the circular loop.  We only check one on each think.  
               
               // DO: This check needs to be different for different target types.  For now
               //    we automatically get LOS to the Laser and Point targets, but we don't
               //    do callbacks on them.  Probably there should be a method, and some
               //    corresponding callbacks.  
               if( t->isPlayer() )
               {
                  if( haveLOS( player, t->findPlayerObj() ) )
                  {
                     if( ! t->flags.test(HaveTargetLOS) )
                     {
                        if( ! t->flags.test(HadTargetLOS) )
                           checkCallback( TargetAcquired, t );
                        else
                           checkCallback( TargetLOSRegained, t );
                     }
                     t->flags.set( HaveTargetLOS | HadTargetLOS );
                  } 
                  else if( t->flags.test(HaveTargetLOS) )
                  {
                     t->flags.clear( HaveTargetLOS );
                     checkCallback( TargetLOSLost, t );
                  }
               }
               else
                  t->flags.set( HaveTargetLOS | HadTargetLOS );
               
               lastLosCheckId = t->playerId;
               break;
            }//find target
            
            nextCheckIndex++;
            
         }while( (nextCheckIndex %= count) != lastCheckIndex );
      }
   }
   
   // Find first alive player that we have LOS to.  Also do checks for target just died.  
   //    We take the nearest of those alive targets which we have LOS to.  
   currentTarget = NULL;
   float bestDist = 314159265358979.0;
   for( x = 0; x < count; x++ )
   {
      if( (t = directives[ indices[x] ])->isPlayer() )
      {
         if( Player * p = findPlayerObject(t->playerId) )
         {
            if( p->isDead() ){
               if( !t->flags.test(TargetWasDead) ){
                  checkCallback( TargetKilled, t );
                  t->flags.set(TargetWasDead);
               }
            }
            else  // alive, see if we have LOS and should make it our target. 
               if( t->flags.test(HaveTargetLOS) )
               {
                  float d = m_distf( p->getLeadCenter(), ourCenter() );
                  if( ! currentTarget || d < bestDist )
                  {
                     currentTarget = t;
                     bestDist = d;
                  }
               }
         }
         else
         {
            // When a player is killed it is no longer owned by it's rep, it appears.  
            // So this callback will also be invoked when a client is dropped.  
            if( !t->flags.test(TargetWasDead) ){
               checkCallback( TargetKilled, t );
               t->flags.set(TargetWasDead);
            }
         }
      }
      else
      {
         // For laser and point targets, the last one will be used.  
         currentTarget = t;
      }
   }
}


// Old scan code for single target given by seekFlag.  New code uses target list with 
// different types of targets.  
Player * AIObj::scanForOldTargets(bool force)
{
   if( seekFlag > 2048  &&  timeToThink(force) )
      if( Player * p = findPlayerObject( seekFlag ) )
         if( !p->isDead() )
            if( dist3d(ourCenter(),p->getLeadCenter()) < iq.spotDistance() )
               if( haveLOS( player, p ) )
                  return p;
   return NULL;
}


void AIObj::followPath(void)
{
   if( currently->haveDest )
      seekLocation ( currently->seekLoc, move );
   
   if( scanForOldTargets() )
      state = AttackStill;
   else if ( currentTarget != NULL )
      state = AttackTarget;
}

void AIObj::attackStill(void)
{
   if( Player * p = scanForOldTargets(true) )
      fireAtPlayerTarget( move, p );
   else
      state = NeedToThink;
}

void AIObj::attackTarget(void)
{
   if( currentTarget ){
      switch( currentTarget->subType ){  
         case  Target::player:{
            Player   * p = currentTarget->findPlayerObj();
            if( p )
               fireAtPlayerTarget( move, p );
            break;
         }
         case  Target::laser:{
            fireAtLaserTarget( move, currentTarget );
            break;
         }
         case  Target::point:{
            fireAtPointTarget( move, currentTarget );
            break;
         }
         default:{
            AssertFatal(0,"ai: bad target type" );
            break;
         }
      }
   }
   else
      state = NeedToThink;
}


// Decision function, though most of the state logic will probably be within the 
//    states themselves.  But otherwise this will be something that can figure
//    things out from scratch.  
// 
void AIObj::rethinkState(bool force)
{
   if( timeToThink(force) )
   {
      if( !doneWithPath && havePath() )
      {
         // pathIndex = 0;
         state = FollowingPath;
      }
      else if ( seekFlag )
         state = AttackStill;
      else if ( currentTarget )
         state = AttackTarget;
      else
         state = DoNothing;
   }
}

AIObj::FrameVariables::FrameVariables()
: seekLoc(-1000000.0f,-1000000.0f,-1000000.0f)
{
   haveDest = false;
}


#if 0
// Track a variable for changes.  i.e. for damage, for which we may want to have the 
//    the changing flag lagged a bit.  
template <class T>
class Track
{
public:
   bool     alreadySet;
   float    counter, howLong;
   T        variable;
   T        velocity;
   
   Track()  { alreadySet = false; }
   
   bool update( T newVar )
   {
      if( ! alreadySet )
      {
         variable = newVar;
         alreadySet = true;
      }
      else
      {
         if( (velocity = newVar-variable) != 0 )
            counter = 10;
      }
      if( --counter < 0 )
         counter = 0;
      return (counter != 0);
   }
}
#endif


bool AIObj::withinRange( const Point3F & destination )
{
   // if( jetNavigation > 0.0 )
#if _JETNAVDEV_
   if( m_jsPtr != NULL )
   {
      float    d3d = dist3d( ourCenter(), destination );
      return ( d3d < 0.2 );
   }
   else
#endif
   {
      float    d2d = dist2d( ourCenter(), destination );
      return ( d2d < 1.2 );
   }
}


// Data we don't want to fetch or compute often within the AI tick can be computed
//    here, such as calling findObject, and so forth.  
// 
// This routine "knows" that the FrameVariables have already been set to the default 
//    values in the construtor above.  
//
void AIObj::doSetupComputations(void)
{
   currently->ourCenter = player->getLeadCenter();
   
   // We want to do this after the player has been added...  
   if ( ! didFirstTimeSetup )
   {
      didFirstTimeSetup = true;
      lastWaypointLoc = ourCenter();
   }

   currentTarget = NULL;
   
   if( player->isDead() )
      seekFlag = 0;
   else{
      if( !seekFlag )
         scanTargetList();
   }

   if( !doneWithPath )
   {
      // figure out our path destination if any.  we may have to check this many 
      //    locations if they are close to each other.  
      Point3F     playerPos = ourCenter();
      int         count = directives.size();
      
      while( count-- )
      {
         Waypoint    * w;
         if( (w = getWaypoint(pathIndex)) == NULL )
            w = setNextWaypoint( pathIndex );

         if( !w )    // nothing found, we're done. 
            break;
         else
         {
            SphereF   sphere = getFollowDestination( w );
            if( w->type==FollowDirective || !withinRange(sphere.center) )
            {
               currently->haveDest = true;
               currently->seekLoc = sphere;
               break;
            }
            
            checkCallback( WaypointReached, w );   // script callback on waypoint arrival
            
            setNextWaypoint(pathIndex);
         }
      }
      if( ! currently->haveDest )
         doneWithPath = true;
   }
}

// Called in a loop after all the ai has been done.  The main reason being in case 
// a spawn is done on the death callback (reentrancy problem).  
void AIObj::doPostLoopComputations(void)
{
   wrappedFireAngle++;
   if( player->isDead() )
   {
      if( !flags.test( AiWasDead ) )
      {
         flags.set( AiWasDead );
         // NOTE: this callback CAN'T respawn - there are re-entrancy problems with it.
         //    So a respawn should just be SCHEDULED.  We should think about moving the 
         //    callback or enforcing no re-entrance.  
         // A possible solution to this problem might be to queue up callbacks and invoke
         //    them at the end of AI manager loop.  
         checkCallback( AiDied );
         
         // Here we change the name of the AI so that a newly spawned one can use
         //    the same name.  This is ugly, and we probably should have had a 
         //    respawning AI mode, but this works.  We need the numberWrap in case
         //    multiple respawns of the same AI are laying around for a while.  Four
         //    billion laying around can't happen.  Yet, that is - maybe Tribes VII?  
         static UInt32 numberWrap;
         removeVariables(NULL);
         char temp[MaxNameLen + 20];
         sprintf( temp, "Corpse%d %s", numberWrap++, name );
         strncpy( name, temp, MaxNameLen - 1 );
      }
   }
   
   if( periodicCbWait >= TicksPerSecond )
      if( periodicCbWait.timeToThink() )
         checkCallback( AiPeriodic );
}

void AIObj::doPreLoopComputations(void)
{
   currentTargetId = -1;
   trigger.preLoop();
   clrFrameAccurate();
   ourTeam = getBaseRepPtr(repId)->team;
}


bool AIObj::onServerProc(void)
{
   PlayerMove     * pm = & move;
   PlayerMove     stopPM;
   move = stopPM;
   switch ( state )
   {
      case  NeedToThink:
         rethinkState(true);
         break;
      case  FollowingPath:
         followPath();
         break;
      case  AttackStill:
         attackStill();
         break;
      case  AttackTarget:
         attackTarget();
         break;
      case  DoNothing:
         rethinkState(false);
         break;
   }

   player->serverUpdateMove( pm, 1 );

   return true;
}


//---------------------------------------------------------------------------------------
//    Basic AI machinery.  Path and shot seeking, shot leading, LOS, places 
//       we can step, math helper functions.  
//

// patch to m_atan() with check for singularity.  
static float arcTangent( float x, float y )
{
   return( (IsZero(x) && IsZero(y)) ? 0.0 : m_atan(x, y) );
}


#define  MaxMisfireScale      2.2
#define  AngleNoiseAmount     0.3
// #define  ResetAngleHowOften   0.037
#define  ResetAngleHowOften   0.001

// 
// Return a "doping" offset to add to the target location for less intelligent AI.  
//
// Decide if we're going to hit. 
//    If so - 
//       Aim within the box: further out for dumber guys though, and our angle from 
//       the middle of the guy is maintained (using wrappedFireAngle). 
//    If not
//       Aim outisde of the box.  Use the wrapping angle, with a little bit of noise
//       added, so that the guy is aiming in the same wrong area, about. 
//       The distance away from the center that we use should be randomly chosen
//       with the worst guys aiming somewhere between (X * R) and (2 * X * R) 
//       units away from the center.  X is a factor which is 1 for the smartest 
//       guys, and some larger value for the dumbest, probably 2 or 3 (see
//          MaxMisfireScale).  
//
// Once we know how far off from the center to fire, we fire off at an angle in the
//    plane which is _facing_ the firer.  Hence upVector and sideVector.  
// 
Point3F  AIObj::dopeOffset( Player * p, const Point3F * loc )
{
   Point3F  targCenter = loc ? *loc : p->getLeadCenter();
   
   Point3F  vecToTarg = targCenter;
   vecToTarg -= ourCenter();
   
   // Handle unlikely case where target location is the same.  This did turn up
   // but it was with two objects mounted on the same vehicle (and getLeadCenter()
   // returns _mount_ location when there is one).  
   if( vecToTarg.isZero() )
      vecToTarg.set(1.0);     // i.e. 1,0,0
   else
      vecToTarg.normalize();
      
   Point3F     upVector ( 0.0, 0.0, 1.0 ), sideVector;
	m_cross( upVector, vecToTarg, & sideVector );
   
   float    range = dist3d( targCenter, ourCenter() );
   float    percent = iq.getWeaponAccuracy( range );
   bool     weHit = randPct( percent );

   // figure out unit distance from center to aim at.   This is then scaled 
   float unitDistance;
   if( weHit )
   {
      unitDistance = 1.0 - iqScale;
   }
   else
   {
      float misfireScale = scaleBetween( MaxMisfireScale, 1.0, iqScale );
      unitDistance = misfireScale + rand.getFloat( misfireScale );
   }

   // Get our angle away from the center.  The wrapped value is always rotating around
   // a circle, and we add just a little bit of noise, plus occasionally (every 20 frame
   //    or so) we completely change the wrapping value.  The wrapping value simulates
   //    the way someone fires in the same general area relative to their target.
   float angle = (float(wrappedFireAngle) * M_2PI) / 256.0f;
   angle += rand.getFloat( - AngleNoiseAmount, + AngleNoiseAmount );
   if( randPct( ResetAngleHowOften ) )
      wrappedFireAngle += 0x70;

      
      // all guys are deadly in close.   
   static float adjust_circle_min = 1.7;
   static float adjust_circle_max = 22.0;
   if ( range < adjust_circle_max )
   {
      float    pctAway;
      if( range <= adjust_circle_min )
         unitDistance = 0.0;
      else
      {
         pctAway = getPercentBetween( range, adjust_circle_min, adjust_circle_max );
         
         pctAway *= pctAway;
         //Console->printf("Scaled at --> %f", pctAway );
         unitDistance = scaleBetween( 0.0f, unitDistance, pctAway );
      }
   }
   
   // Got our angle and our unitDistance from the center to fire, so figure out the
   //    actual offset.  We scale the offset differently for Up than Sideways, based
   //    off of dimensions of the target.  
   float w = loc ? 0.6 : p->boundingBox.len_x() / 2.0;
   float h = loc ? 1.2 : p->boundingBox.len_z() / 2.0;
   upVector    *= (h * m_sin(angle) * unitDistance);
   sideVector  *= (w * m_cos(angle) * unitDistance);
   
   return (upVector + sideVector);
}


// Return a Z rotation from source to dest.  
float AIObj::getDesiredRot( const Point3F & dest, const Point3F & source )
{
   float angle = arcTangent( dest.x-source.x, dest.y-source.y ) - M_PI/2;
   return angle;
}

// Return an X rotation in the range [-pi/2, +pi/2].  arcTangent() needs no 
// adjustment here.  
float AIObj::getDesiredPitch( const Point3F & dest, const Point3F & source, bool fastMath )
{
   //Point2F  & xyDst = dest, & xySrc = source;
   //float horizontalDist = fastMath ? m_distf(xyDst,xySrc) : m_dist(xyDst,xySrc);
   float horizontalDist = dist2d( dest, source, fastMath );
   
   return arcTangent( horizontalDist, dest.z - source.z );
}


// Given a direction we're facing, and a direction we would like to go, set the PlayerMove 
// action variables to move in that direction.  
//    Q: this works even if angles are anywhere (i.e. 2*Pi*N equivalents), right?  
void AIObj::setMovementAction ( PlayerMove & pm, const AngleSeek & heading, float speed )
{
   float    diffAngle =  heading.desired - heading.current;
   float    lrAction =   m_sin( diffAngle ) * speed;
   float    fbAction =   m_cos( diffAngle ) * speed;
   
   if( lrAction > 0.0 )
      pm.leftAction = lrAction, pm.rightAction = 0.0;
   else
      pm.rightAction = -lrAction, pm.leftAction = 0.0;
      
   if( fbAction > 0.0 )
      pm.forwardAction = fbAction, pm.backwardAction = 0.0;
   else
      pm.backwardAction = -fbAction, pm.forwardAction = 0.0;
}


// Put the given rotation value into the range [rangeMin, rangeMin + 2PI].  
float AIObj::wrapRotation ( float r, float rangeMin/*=0*/ )
{
   while ((r - rangeMin) > M_2PI)
      r -= float( M_2PI );
   while ((r - rangeMin) < 0)
      r += float( M_2PI );
   return r;
}

// Set the speed of the currently selected weapon of the player we are controlling.
// Return true if a weapon does exist and has a positive speed.  
//    Do we want to just do this when it's changed?  
bool AIObj::setProjectileSpeed(void)
{
   ProjectileDataType   primary, secondary;
   
   if( player->getWeaponAmmoType(primary, secondary) )
   {
      if (primary.type != -1)
      {
         DataBlockManager *dbm = (DataBlockManager *)manager->findObject(DataBlockManagerId);
         Projectile::ProjectileData* ptr = static_cast<Projectile::ProjectileData *>
               (dbm->lookupDataBlock(primary.dataType,primary.type));
               
         if( ptr )
            if( !IsZero(projectileSpeed = ptr->getTerminalVelocity()) )
               return true;
      }
   }
   projectileSpeed = 0.0f;
   return false;
}

Point3F AIObj::leadPosition( Point3F pos, Point3F vel )
{
   AssertFatal( !IsZero(projectileSpeed), "AIOBJ: zero proj speed" );
   float dist = dist3d(ourCenter(), pos);
   vel *= (dist / projectileSpeed);
   return ( pos += vel );
}


AIObj::AngleSeek  AIObj::getAngleSeekZ ( const Point3F & seekLoc )
{
   Point3F     playerPos = ourCenter();
   AngleSeek   Z ( wrapRotation( getDesiredRot( seekLoc, playerPos ) ) );
   
   // Get the player Z rot (getRot() is in ShapeBase).  Normalize the player 
   //    Zrot 2PI higher so we can do the next loop which helps us find the actual
   //    difference.  We basically want to find a "version" of the destination
   //    angle which is within PI in either direction of where we're pointed now, 
   //       so the loop "comes up" on this from below.  
   Z.current = wrapRotation ( player->getRot().z, M_2PI );
   float    pi = M_PI+0.000001f;  // paranoia. else not convinced loop always exits
   
   while(  (Z.absDiff = fabs( Z.diff = Z.desired - Z.current ))  >  pi  )
      Z.desired += M_2PI;

   return Z;
}

// Pitch up or down at the given target. We assume that we are roughly aimed in that 
//    direction.  
AIObj::AngleSeek   AIObj::pitchAtTarget ( const Point3F & seekLoc, PlayerMove & pm )
{
   Point3F     playerPos = ourCenter();
   AngleSeek   X (getDesiredPitch( seekLoc, playerPos ));
   X.current = player->getViewPitch();
   
   float       diff = X.desired - X.current;
   X.absDiff = fabs(diff);

   float    increment = pitch_increment;
   if ( X.absDiff < reduce_pitch_threshold ){
      // DO: use incrementor class.  Also: still need "doping" vector.  Also: I guess
      //       we need to dope the _turn_ offset of the player for aiming.  That is,
      //       the sidewise dope needs to not change too much, be kept around a little.
      increment = pitch_increment * (X.absDiff / reduce_pitch_threshold);
      if( increment < min_pitch_increment )  
         increment = min_pitch_increment;
      if( increment > X.absDiff )  
         increment = X.absDiff;
   }
   
   pm.pitch = (diff < 0 ?  -increment : increment);
   return  X;
}

void AIObj::fireAtPlayerTarget ( PlayerMove & pm, Player * p )
{
   if( p )
   {
      Point3F  seekLoc;
      
      // need to do something different if we can't get a projectile speed, go to 
      //    path following or evasives.. 
      if( setProjectileSpeed() )
      {
         Point3F  enemyVelocity = p->getLeadVelocity();
         // Account for (subtract out) our own velocity.  
         enemyVelocity -= player->getLeadVelocity();
         
         seekLoc = leadPosition( p->getLeadCenter(), enemyVelocity );
      }
      else
         seekLoc = p->getLeadCenter();
         
      seekLoc += dopeOffset(p); 

      AngleSeek   Z = turnToLoc( seekLoc, pm );
      if( Z < 0.08f ){
         // At this point we could also think about running or jetting - we should have
         //    some code that can decide if it's in "safe" region for doing this.  
         if( pitchAtTarget( seekLoc, pm ) < 0.08f )
         {
            if ( !p->isDead() )
               pm.trigger = trigger.press();
         }
      }

      // This variable gives the target currently being attacked, for reference from
      // the console.  It is cleared every frame (to -1) in the preLoop, and so it is 
      // only set to a meaningful Id when the code gets here.  
      currentTargetId = currentTarget->playerId;
      
      if( attackMode == WhileFollowingPath ){
         if( currently->haveDest ){
#if _JETNAVDEV_
            if( m_jsPtr != NULL )
            // if( jetNavigation > 0.0 )
               jetTowardLoc( SphereF(currently->seekLoc), pm );
            else
#endif
               moveTowardLoc( SphereF(currently->seekLoc), pm );
         }
      }
      else if( attackMode == WhileFollowingTarget ){
         //Point3F  loc = seekOff;
         //loc += p->getLeadCenter();
         Point3F  loc = p->getLeadCenter();
         
         float d = dist2d( loc, ourCenter() );
         static float  keep_away = 17;
         if( d < keep_away )
         {
            loc -= ourCenter();
            loc.normalize();
            loc *= -keep_away;
            loc += p->getLeadCenter();
         }
         
         // DO: occasionally try to jet, or jet over, maybe it depends on IQ and the 
         //    jet variable is used differently, or we can have a negative value (which 
         //    distinguishes this type of jetting from navigation jetting).  
         moveTowardLoc( loc, pm );
      }else{
         // attack mode is standing still.  
      }
   }
   else
   {
      // by default we could rethink, if we haven't found a target or if we're out of
      //    ammo for the currently selected weapon.  
   }
}

void AIObj::fireAtLaserTarget ( PlayerMove & pm, Target * t )
{
   bool  followPath = (attackMode==WhileFollowingPath && currently->haveDest);
   bool  aimingAtLaserPoint = false;

   //this is actually an IF, but want to use BREAKs below.
   while ( setProjectileSpeed() )
   {
      GameBase * pControl = player;
      
      if( pControl == NULL )
         return;
      
      ProjectileDataType primaryWeapon, secondaryWeapon;
      if( pControl->getWeaponAmmoType(primaryWeapon, secondaryWeapon) == false )
         break;

      if( primaryWeapon.type == -1 && secondaryWeapon.type == -1 )
         break;

      // Get the block and player managers...
      AssertFatal(manager != NULL, "ai: need manager for laser targetting");
      DataBlockManager *dbm = (DataBlockManager *) manager->findObject(DataBlockManagerId);

      //-------------------------------------- Get the data blocks
   	Projectile::ProjectileData * ptrPrimary = NULL;
      if (primaryWeapon.type != -1)
   	   ptrPrimary = static_cast<Projectile::ProjectileData *>
   	   	          (dbm->lookupDataBlock(primaryWeapon.dataType, primaryWeapon.type));
      if (ptrPrimary == NULL)
         break;

      SimSet * targetSet = static_cast<SimSet*>(manager->findObject(TargetableSetId));
      AssertFatal( targetSet != NULL, "ai: can't find targetable set" );
      
      Player   * laserOwner = findPlayerObject( t->playerId );
      if( laserOwner == NULL )
         break;

      for( Vector<SimObject*>::iterator itr = targetSet->begin(); itr != targetSet->end(); itr++ )
      {
         TargetLaser * tl = dynamic_cast<TargetLaser *>(*itr);
         
         if( tl && tl->wasShotBy( laserOwner ) )
         {
            Point3F  targetPoint, eyePoint, primaryVector, secondaryVector;
            bool     primaryInRange, secondaryInRange, showPrimary, showSecondary;
            int      team;
            
            if( ! tl->getTarget( & targetPoint, & team ) )
               continue;
               
            Point3F gravityVec;
            float gravity = 0.0f;
            if( pControl->getForce(SimMovementGravityForce, &gravityVec) == true )
               gravity = -(gravityVec.len());
         
            eyePoint = pControl->getEyeTransform().p;

            showPrimary = ptrPrimary->getTargetVector( eyePoint, targetPoint, gravity,
                                       & primaryVector, & primaryInRange, false );
                                       
            if( showPrimary == false ) // Secondary never visible if primary isn't...
               continue;
                                       
            showSecondary = ptrPrimary->getTargetVector( eyePoint, targetPoint, gravity,
                                       & secondaryVector, & secondaryInRange, true );
                                       
            // DO:  we need to do some LOS checking here to see which vector to choose, if 
            //    either.  Also: need a console variable fireWait in the trigger.  
            Point3F  seekLoc;
            if( showSecondary && secondaryInRange )
               seekLoc = secondaryVector;
            else if( primaryInRange )
               seekLoc = primaryVector;
            else 
               break;


            seekLoc *= 50.0;
            seekLoc += eyePoint;       // get actual location to seek at.  
            
            static float   zThresh = 0.001;
            static float   xThresh = 0.001;

            // We want to do accurate aiming math here.  
            setFrameAccurate(); 
            AngleSeek   Z = turnToLoc( seekLoc, pm );
            if( Z < zThresh )
            {
               if( pitchAtTarget( seekLoc, pm ) < xThresh )
               {
                  bool     press = trigger.press();
                  
                  if ( press ){
                     pm.trigger = true;
                  }
                  //else we can travel?
               }
            }
            aimingAtLaserPoint = true;
            
         }//is proper owner.
      }//targetSet iteration. 
      break;
   }//if have projectiles.
   
   if( followPath )
   {
      if( ! aimingAtLaserPoint )
         seekLocation ( SphereF(currently->seekLoc), pm );
      // moveTowardLoc( SphereF(ourCenter()), pm );
   }
}

void AIObj::fireAtPointTarget ( PlayerMove & pm, Target * t )
{
   Point3F  seekLoc = t->pos;
   // seekLoc += dopeOffset( NULL, & seekLoc );
   AngleSeek   Z = turnToLoc( seekLoc, pm );
   if( Z < 0.08f )
   {
      if( pitchAtTarget( seekLoc, pm ) < 0.08f )
      {
         pm.trigger = trigger.press();
      }
   }
}


// get a speed based on distance from the desired destination (and on skill).  
//    these numbers need to take into account speed.  
#define  ArrivalDistance      0.2
#define  SlowdownDistance     1.7
float AIObj::getSpeed(float dist)
{
   if( dist < SlowdownDistance )
   {
      if( dist > ArrivalDistance )
      {
         float pct = (dist-ArrivalDistance) / (SlowdownDistance-ArrivalDistance);
         float minSpeed = iq.maxSpeed / 3;
         
         if( minSpeed > 0.25 )
            minSpeed = 0.25;
            
         return scaleBetween( minSpeed, iq.maxSpeed, pct );
      }
      return 0.0;
   }
   return iq.maxSpeed;
}


#if   _JETNAVDEV_

static float   lineAboveBegin = 0.75, lineAboveEnd = 0.2;
float AIObj::jetSlantUpwards( const Point3F & dst, const Point3F & src, PlayerMove & pm )
{
   // Algorithm: 
   //    Try to maintain a certain height above the line, which converges to 0 as we 
   //    get near the desired point.  Other than that we pulse in X and Y to maintain 
   //    the velocity in that direction - and we slow THAT target velocity down as 
   //       we get close.  
   
   // compute height above line we're supposed to travel.
   float distSoFar = dist2d( ourCenter(), src );
   float totalDist = dist2d( dst, src );
   float pctOfWay = distSoFar / totalDist;
   float expectedZ = scaleBetween( src.z, dst.z, pctOfWay );
   float zDiff = ourCenter().z - expectedZ;
   
   pm;
   
   // float desiredAbove = scaleBetween( lineAboveBegin, lineAboveEnd, pctOfWay );
   // if( zDiff < desiredAbove )
   // {
   //    pm.jetting = true;
   // }
   // else
   // {
      // need to adjust horizontal velocity.  
   // }
   
   distSoFar;
   totalDist;
   pctOfWay;
   expectedZ;
   zDiff;
   
   return 0.0;
}

float AIObj::jetSlantDownward( const Point3F & dst, const Point3F & src, PlayerMove & pm )
{
   src;
   dst;
   pm;
   return 0.0;
}

namespace Test
{
   float    startScale = 0.2;
   float    midScale = 0.2;
   float    endingSpeed = 0.1;
};

//
// Jet test.  The drone attempts to jet between the destination loc and the last waypoint
// location by hopping to a point above the midpoint (specified by jetNavigation 
// variable).  
//
float AIObj::jetTowardLoc( const SphereF & destLoc, PlayerMove & pm )
{
   setFrameAccurate();
   
   if( m_jsPtr != NULL )
   {
      if( updateJetNavigation( *m_jsPtr, pm ) )
         m_jsPtr = NULL;
         
      return 0.0f;
   }

   Point3F     playerPos   = ourCenter();
   Point3F     endpoint    = destLoc;
   Point3F     startpoint  = lastWaypointLoc;
   Point3F     midpoint    = lastWaypointLoc;
   
   midpoint += endpoint;
   midpoint /= 2.0;
   midpoint.z += jetNavigation;
   
   float       totalD2 = dist2d( endpoint, startpoint );
   float       remainingD2 = dist2d( playerPos, endpoint );
   float       halfD = totalD2 / 2.0;
   
   float       distToEndPoint = dist3d( playerPos, endpoint );
   float       distToMidPoint = dist3d( playerPos, midpoint );
   
   // Basic approach.  We want to 'conservatively' choose a velocity vector at each
   // stage of the way and then try to seek it using the jet forces available to us, 
   // and averaging the force based on errors in the actual one we could apply.  
   // Conservative means we always seek a velocity that isn't in danger of "getting out
   // of hand" - or, more accurately, won't change too much from frame to frame.  
   // Then, hopefully, our 'micro-tuned' adjustments will work.  We will then generate 
   // intermediate jetting paths on the fly which have constraints on entrance and
   // exit speeds so as to give a good arc.  
   // 
   if( distToEndPoint > 1.0 )    // assure non-zero seek vector for normalization below
   {
      int      i;
   
      if( fabs(remainingD2-halfD) < 0.3 )
      {
         // track how close we come to our midpoint.  
      }
      
      // Values for figuring out target velocities.  
      float maxLateralSpeed = player->data->maxJetForwardVelocity;
      // static float startScale = 0.9, midScale = 0.6, endingSpeed = 2;
      
      // Figure out constraints for whichever path half we're on.  
      float startSpeed, endSpeed, percent;
      Point3F  targetPoint;
      if( remainingD2 > (halfD + 0.1) )
      {
         // first half.  
         targetPoint = midpoint;
         
         percent = getPercentBetween( distToMidPoint, m_dist(startpoint,midpoint), 0 );
         // percent = distToMidPoint / m_dist( startpoint, midpoint );
         
         startSpeed = Test::startScale * maxLateralSpeed;
         endSpeed =   Test::midScale * maxLateralSpeed;
         
         if( jetStage != 1 ) 
         {  // We need some correct state init, but this will work for our test
            jetStage = 1;
            jetSaveLinearVel = player->getLinearVelocity();
         }
      }
      else
      {  // second half
         targetPoint = endpoint;
         percent = getPercentBetween( distToEndPoint, m_dist(midpoint,endpoint), 0 );
         // percent = distToEndPoint / m_distf( midpoint, endpoint );
         startSpeed = Test::midScale * maxLateralSpeed;
         endSpeed = Test::endingSpeed;
         
         jetStage = 2;
      }
      
      float desiredSpeed = scaleBetween( startSpeed, endSpeed, percent );

      Point3F  targetVector = targetPoint;      
      targetVector -= playerPos;
      Point3F  desiredVelocity = targetVector.normalize();
      desiredVelocity *= desiredSpeed;
      
      // We want to figure out how much our last frame's force deviated from 
      // what we want.  The deviation is the distance from the velocity vector
      // we're trying to seek - or it's endpoint if we're near that.  This 
      // deviation gives us an error vector to roll into our consideration of which
      // force best takes us where we want to go this frame.  
      Point3F  actualForce = player->getLinearVelocity() - jetSaveLinearVel;
      if( actualForce.lenf() > 10.0 )
      {
         // We're not in control here....  is this how to handle it...?  
         actualForce.set();
      }
      // float    oldSpeed = jetSaveLinearVel.len();
      float    dotProd = m_dot( actualForce, targetVector );
      if( dotProd > desiredSpeed )
         dotProd = desiredSpeed;
      Point3F  pointOnVec = targetVector * dotProd;
      Point3F  adjustedTargetVel = desiredVelocity + (pointOnVec - actualForce);
      jetSaveLinearVel = player->getLinearVelocity();
      
      // Get forces available.  This method gives us up to four possible jet
      // forces that can result.  The second parameter is a unit lateral velocity
      // vector, or NULL if none can be determined. 
      targetVector.z = 0;
      float latSpd = targetVector.len();
      Point3F possibleForces[4], * unitLateralVel = NULL;
      if( latSpd > 0.001 )
      {
         targetVector.x /= latSpd;     // normalize
         targetVector.y /= latSpd;
         unitLateralVel = & targetVector;
      }
      int nForces = player->possibleJetForces( possibleForces, unitLateralVel );
      
      // Find the the force that is closest to what we desire, and configure
      // the input.  
      float    bestDiff = m_dist( possibleForces[0], adjustedTargetVel );
      int      bestForce = 0;
      for( i = 1; i < nForces; i++ )
      {
         float diff = m_dist( possibleForces[i], adjustedTargetVel );
         if( diff < bestDiff )
         {
            bestDiff = diff;
            bestForce = i;
         }
      }
      AngleSeek  Z = getAngleSeekZ( targetPoint );
      switch( bestForce )
      {
         case  0:    // no jetting
            break;
         case  1:    // jetting straight up
            pm.jetting = true;
            break;
         case  2:    // jetting in the specified lateral direction
            pm.jetting = true;
            setMovementAction ( pm, Z, 1.0 );
            break;
         case  3:    // jetting opposite to the specified direction
            pm.jetting = true;
            Z.desired += M_PI;
            setMovementAction ( pm, Z, 1.0 );
            break;
      }
      // Z.current += pm.turnRot;
   }
   
   // Occasionally, give them full energy.  We need to figure out a way to do this 
   // BUT to track how much is used as well, and then we can see what kinds of jumps
   // are reasonable.  
   if( randPct( 0.90 ) )
   {
      float maxE = player->data->maxEnergy;
      player->setEnergy( maxE );
   }
   
   return distToEndPoint;
}


#endif


// Run without turning towards this location.  Make use of any turnRot already in 
// the PlayerMove to pre-adjust where we consider the AI to be currently pointing. 
float AIObj::moveTowardLoc( const SphereF & seekLoc, PlayerMove & pm )
{
   Point3F     playerPos = ourCenter();
   float       dist = dist3d ( seekLoc, playerPos );
   
   if( dist > ArrivalDistance )
   {
      AngleSeek  Z = getAngleSeekZ( seekLoc );
      Z.current += pm.turnRot;
      setMovementAction ( pm, Z, getSpeed(dist) );
   }
   return dist;
}

// Turn towards the given location, set the pm rotation field, and return the 
// results as an AngleSeek.  Does not set other fields of pm.  
// Used for both navigation and shot aiming.
AIObj::AngleSeek  AIObj::turnToLoc ( const Point3F & seekLoc, PlayerMove & pm )
{
   AngleSeek   Z = getAngleSeekZ( seekLoc );
   float    increment = iq.turnInc.getInc( Z.absDiff );
   pm.turnRot =  Z.diff < 0 ?  -increment : increment;
   return Z;
}


static float   max_speed = 1.0f;
static float   close_dist_outer = 5.0f;
static float   close_dist_inner = 1.0f;
static float   start_run_threshold = 0.08;

float AIObj::seekLocation ( const SphereF & sphere, PlayerMove & pm )
{
   Point3F     playerPos = ourCenter();
   float       dist = dist3d( sphere.center, playerPos );
   
   if( dist <= close_dist_inner ) 
      return dist;
      
   AngleSeek   seekZ = turnToLoc( sphere, pm );

#if _JETNAVDEV_
   // if( jetNavigation > 0.0 )
   if( m_jsPtr != NULL )
      jetTowardLoc( sphere, pm );
   else
#endif
      setMovementAction ( pm, seekZ, getSpeed(dist) );
   
   return dist;
}


//-------------------------------------------------------------------------------------
//    AIManager (+ onAdd, onRemove, etc.)

void AIObj::onDeleteNotify(SimObject *object)
{
   if(object == player)
   {
      // This is where the deletion process gets initiated.  
      player = NULL;
   }
   Parent::onDeleteNotify(object);
}

bool AIObj::onAdd()
{
   if (!Parent::onAdd())
      return false;

   repId = sg.playerManager->getFreeId();
   
   const char * theName = player->getName();
   
   sg.playerManager->clientAdded( theName, 
         voice, 
         stringTable.insert("base"), 
         ! strnicmp(voice,"male",4), 
         this );
   
   if( stricmp(theName,name) )
      flags.set(HaveColorName);
   
   player->setOwnerClient( repId );

   return (active = true);
}

void AIObj::onRemove()
{
   active = false;
   sg.playerManager->clientDropped( repId );
   Parent::onRemove();
}

void AIObj::removeThis(void)
{
   active = false;
   if( player ){
      player->deleteObject();
      player = NULL;
   }
   // This actually causes a deletion call - is this Ok?  
   deleteObject();
}

// 
AIManager *    AIManager::it = NULL;
bool           AIManager::recomputePulse = true;

AIManager::AIManager()
{
   AssertFatal( ! it, "AI: can only have one manager" );
   it = this;
   cyclingVar = 0;
   recomputePulse = true;
}
AIManager::~AIManager()
{
   it = 0;
   AssertFatal( !size(), "AI manager list should be empty when deleted" );
}

// Console callback to force recomputations when certain values are changed.  
const char * AIManager::doRecomputation(CMDConsole*,int,int, const char * *)
{
   recomputePulse = true;
   return 0;
}

bool AIManager::onAdd()
{
   if (!Parent::onAdd())
      return false;
   return true;
}

// This is where we remove the pointer from our list.  
void AIManager::onDeleteNotify(SimObject *object)
{
   for( iterator itr = begin(); itr != end(); itr++ )
      if( static_cast<AIObj *>(object) == *itr )
      {
         erase(itr);
         break;
      }
      
   // should we assert that it was in fact found (not at end?). 
   Parent::onDeleteNotify(object);
}

// We put only the manager in the timer set, and do our own iteration.  
//
// The list management (in terms of objects being added and deleted) here could likely be 
// cleaner.  
//
void AIManager::doServerProcesses(DWORD /*curTime*/ )
{
   AIList      deleteList;
   AIObj       *ai;
   iterator    itr;

   for( itr = begin(); itr != end(); itr++ )
   {
      if( (ai = * itr)->active )
      {
         if( ai->player )
            ai->doPreLoopComputations();
         else
            deleteList.push_back( ai );
      }
   }

   // Perform the removals from the manager's list.  
   while( ! deleteList.empty() )
   {
      deleteList.last()->removeThis();
      deleteList.pop_back();
   }

   // Perform some time sliced operations - for anything which seems inefficient.  
   if( list.size() )
   {
      ai = list [ cyclingVar++ % list.size() ];
      cyclingVar &= 0x7FFF;
      
      if( ai->active && ai->player )
         ai->timeSlicedOps();
   }
   
   for( itr = begin(); itr != end(); itr++ )
   {
      if( (ai = * itr)->active && ai->player )
      {
         AIObj::FrameVariables   thisFrame;
         ai->currently = &thisFrame;
         if( recomputePulse )
            ai->doRecomputations();
         ai->doSetupComputations();
         ai->onServerProc();
         ai->currently = NULL;
      }
   }
   recomputePulse = false;
   
   for( itr = begin(); itr != end(); itr++ )
      if( (ai = * itr)->active && ai->player )
         ai->doPostLoopComputations();
}

