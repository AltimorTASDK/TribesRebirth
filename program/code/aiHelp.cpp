//-----------------------------------------------------------------------------------//
//                                                                                   //
//       aiHelp.cpp                                                                  //
//                                                                                   //
//    Subsidiary AIObj methods, moved here mostly for file size convenience.         //
//                                                                                   //
//-----------------------------------------------------------------------------------//

#include <sim.h>
#include <console.h>

#include "fearGlobals.h"
#include "aiObj.h"


//----------------------------------------------------------------------------
// Miscellaneous utility methods.  
//

Player * AIObj::findPlayerObject( int id, const char * name /*=NULL*/ )
{
   if (name && sg.manager)
      if( SimObject* sobj = sg.manager->findObject(name) )
         if( Player* pobj = dynamic_cast<Player*>(sobj) )
            return pobj;

   if( sg.playerManager && id != -1 )
      if( PlayerManager::ClientRep *p = sg.playerManager->findBaseRep(id) )
         return p->ownedObject ? dynamic_cast<Player *>(p->ownedObject) : NULL;
         
   return 0;
}

Player * AIObj::Target::findPlayerObj(void)
{
   if( subType == player )
      return AIObj::findPlayerObject( playerId );
   return 0;
}




AIObj::Directive::Directive()  :  pos(0,0,0) 
{
   flags.clear();
   pri = -1;
   order = -1;
   playerId = -1;
   radius = 0.0f;
   memset( CBs, 0, sizeof(CBs) );
}


int AIObj::Directive::operator< (const Directive & dir) const 
{
   return ( order < dir.order );
}

void AIObj::Directive::getPointAndRadius( const char * in_PointAndRadius )
{
   Point3F  P ( 0, 0, 0 );
   float    R = 0.0f;
   sscanf( in_PointAndRadius, "%f %f %f %f", & P.x, & P.y, & P.z, & R );
      
   pos      = P;
   radius   = R;
}

bool AIObj::nameMatches(const char * expr) const
{
   if( FindMatch::isMatch( expr, name, false ) )
      return true;
   else if( flags.test(HaveColorName) )
      return FindMatch::isMatch( expr, player->getName(), false );
   else
      return false;
}


// find directive with the given order key, use only types given by typeFilter.  
//  getBeyond can be set to find whatever is >= the supplied order.  The default
//    parameters are AnyType and getBeyond=false.  
int AIObj::findDirective( int order, int typeFilter, bool getBeyond )
// defaults:                              anyType           false
{
   for( int i = 0; i < directives.size(); i++ )
   {
      Directive   & dir = directives[i];
      if( typeFilter & dir.type )
         if(  dir.order == order  ||  (getBeyond && dir.order>order)  )
            return i;
   }
   return -1;
}


PlayerManager::ClientRep * AIObj::getBaseRepPtr( int id )
{
   PlayerManager::ClientRep * p = NULL;
   if( sg.playerManager && id != -1 )
      p = sg.playerManager->findBaseRep(id);
   return p;
}


void AIObj::eraseDirectives( Vector<int> & eraseList )
{
   while( ! eraseList.empty() )
   {
      int index = findDirective( eraseList.last() );
      eraseList.pop_back();
      AssertFatal( index >= 0, "aiHelp: directive to erase not found" );
      directives.erase( index );
   }
}


bool AIObj::isEnemy( PlayerManager::BaseRep * br )
{
   if( br->id != repId )   // can't target self..  
      if( ourTeam != br->team || sg.playerManager->getNumTeams() <= 1 )
         return true;
         
   return false;
}


#define  AUTO_ORDER_RESERVE      (0x0Bacb000 << 3)
#define  AUTO_ORDER_MASK         (0x0FFFF000 << 3)
#define  isAutoTarget(ord)       (((ord) & AUTO_ORDER_MASK) == AUTO_ORDER_RESERVE)
#define  autoOrderId(ord)        ((ord) & 0xFFF)
#define  autoOrderIdx(id)        ((id) - 2048)

// This is called for ONE AI object (only) each time the server pass does all AI.  
// We're using it to update our target list if we're in "automatic" target 
// mode (which means we put all enemies in the list).  
//    In order to track which of the existing clients we have on the list, we 
// use a bit flag array - one for each possible ClientRep or BaseRep.  
void AIObj::timeSlicedOps(void)
{
   if( targetingMode == AutomaticTargeting )
   {
      // int            ourTeam = getBaseRepPtr(repId)->team;
      Vector<int>    targetsToRemove;
   
      // 1. Scan our existing target list and CULL OUT any that aren't valid 
      //    targets anymore (no rep found, or now same team).  
      DirectiveList::iterator itr;
      for( itr = directives.begin(); itr != directives.end(); itr++ )
      {
         if( itr->type == TargetDirective && isAutoTarget(itr->order) )
         {
            AssertFatal( autoOrderId(itr->order) == itr->playerId, 
                  "aiHelp: Id not correctly encoded into order for autotargets" );
            AssertFatal( targetIdTracker[ autoOrderIdx( itr->playerId ) ], 
                  "aiHelp: Flag array doesn't jibe with auto target list" );
         
            PlayerManager::ClientRep * target = getBaseRepPtr(itr->playerId);
            if( (target == NULL)  ||  !isEnemy(target) )
            {
               targetIdTracker.set( autoOrderIdx( itr->playerId ), 0 );
               targetsToRemove.push_back( itr->order );
            }
         }
      }
      eraseDirectives( targetsToRemove );
      
      // 2. Now scan the existing Base Rep list, and ADD TARGETS that aren't 
      //    already in our list.  
      bool                       somethingAdded = false;
      PlayerManager::BaseRep *   maybe = sg.playerManager->getBaseRepList();
      
      while( maybe )
      {
         if( isEnemy(maybe)  &&  !targetIdTracker[autoOrderIdx(maybe->id)] )
         {
            Target   target;
            target.order = AUTO_ORDER_RESERVE | (target.playerId = maybe->id);
            addDirective( target, true );      // (second param suppresses sort)
            targetIdTracker.set( autoOrderIdx(maybe->id), 1 );
            somethingAdded = true;
         }
         maybe = maybe->nextClient;
      }
      if( somethingAdded )
         directives.sort();
   }
}

void AIObj::setAutomaticTargets(void)
{
   if( targetingMode == ScriptedTargeting )
   {
      targetingMode = AutomaticTargeting;
      targetIdTracker.clear();
   }
}

void AIObj::setScriptedTargets(void)
{
   if( targetingMode == AutomaticTargeting )
   {
      // Change the mode, and remove any auto-targets from the list.  
      targetingMode = ScriptedTargeting;
      Vector<int>    targetsToRemove;
      DirectiveList::iterator itr;
      for( itr = directives.begin(); itr != directives.end(); itr++ )
         if( itr->type == TargetDirective && isAutoTarget(itr->order) )
            targetsToRemove.push_back( itr->order );

      eraseDirectives( targetsToRemove );
   }
}


//
// Add the directive to the list.  Sort the list, unless we're replacing an existing
//    object of like order number.  
// If order# not given, then directive goes at end and order number is higher
//    multiple of 100.  Order numbers are always positive.
//
// 1/18/99:  Now that we have "reserved" order numbers in place for the automatic 
//    targeting, we need to check for the presence of them.  Users must keep their
//    order numbers low.  
//
#define  MAX_USER_ORDER_NUMBER      (AUTO_ORDER_RESERVE >> 1)
void AIObj::addDirective ( Directive & newDirective, bool suppressSorting/*=false*/ )
{
   newDirective.setDefaultCallbacks();
   
   if( newDirective.order > 0 )
   {
      int   index = findDirective( newDirective.order );
      if( index >= 0 )
         directives[ index ] = newDirective;
      else{
         directives.push_back( newDirective );
         if( ! suppressSorting )
            directives.sort();
      }
   }
   else
   { 
      if( directives.empty() )
         newDirective.order = 100;
      else
      {
         int   o = directives.last().order;
      
         if( o >= MAX_USER_ORDER_NUMBER )
         {
            // Must search for last order number to postpend to.  
            int   i;
            for( i = directives.size() - 1; i >= 0; i-- )
            {
               o = directives[i].order + 150;
               if( o < MAX_USER_ORDER_NUMBER )
                  break;
            }
            if( i < 0 )
               o = 100;
         }
         newDirective.order = (o - o % 100);
      }
      
      // int   o = directives.empty() ? 100 : (directives.last().order + 150);
      // newDirective.order = (o - o % 100);
      
      directives.push_back( newDirective );
      if( ! suppressSorting )
         directives.sort();
   }
   
   if( newDirective.type & (WaypointDirective|FollowDirective) )
      doneWithPath = false;
}




// Fetch a list of the indices of the directives which match the given type mask 
// spec.  This is helpful for performing circular list operations....  
// Return how many were found.  
int  AIObj::getDirectiveList( int indices[MaximumDirectives], int ofTheGivenTypes )
{
   int   count = 0;
   int   index = 0;
   DirectiveList::iterator itr;
   
   for( itr = directives.begin(); itr != directives.end(); itr++ )
   {
      if( itr->type & ofTheGivenTypes ){
         indices[ count++ ] = index;
         AssertFatal( count <= MaximumDirectives, "aiHelp: too many directives" );
      }
      index++;
   }
   
   return count;
}

void AIObj::Directive::printf(void)
{
   const char * typeStr = "unknown";
   const char * subtStr = "";
   
   switch(type){
      case  WaypointDirective:
         typeStr = "Waypoint";
         break;
      case  FollowDirective:
         typeStr = "Follow";
         break;
      case  TargetDirective:
         typeStr = "Target";
         if(subType==Target::player)         subtStr = "(player)";
         else if(subType==Target::laser)     subtStr = "(laser)";
         else if(subType==Target::point)     subtStr = "(point)";
         else subtStr = "(what?)";
         break;
      case  GuardDirective:
         typeStr = "Guard";
         break;
   }
   Console->printf( "Ord=%d  Type=%s%s", order, typeStr, subtStr );
}
int AIObj::listDirectives( int typeFilter )
{
   int   indices[MaximumDirectives];
   int   N = getDirectiveList( indices, typeFilter );
   for( int i = 0; i < N; i++ )
      directives[indices[i]].printf();
   return N;
}


// Remove all directives whose order numbers match this wildcard spec.  
//
bool AIObj::removeDirectives( const char * ordExpr )
{
   DirectiveList::iterator itr;
   bool     moreRemain, retVal = false;

   do 
   {
      moreRemain = false;
      for( itr = directives.begin(); itr != directives.end(); itr++ )
      {
         char number[12];
         itoa( itr->order, number, 10 );
         if( FindMatch::isMatch(ordExpr, number) )
         {
            directives.erase( itr );
            retVal = moreRemain = true;
            break;
         }
      }
   }while( moreRemain );
   return retVal;
}

int   AIObj::countDirectives ( int ofTheGivenTypes )
{
   int   count = 0;
   DirectiveList::iterator itr;
   for( itr = directives.begin(); itr != directives.end(); itr++ )
      count += (itr->type & ofTheGivenTypes) != 0;
   return count;
}


//-------------------------------------------------------------------------------------
//       Callbacks



// Install the directive callback on all directives matching the specs.
//   This can filter on order number and on directive type.  
// If an error occurs, return an error message for the console command to print.
//    A NULL return values indicates success.  
const char * 
AIObj::installDCB( int dcb, const char * func, const char * ordExpr, int dirMask )
{
   if( --dcb < 0 || dcb >= NumCallbacks )
      return avar("Callback # must be in the range [%d,%d]", 1, NumCallbacks);

   DirectiveList::iterator    itr;
   DCB   function = NULL;
   
   // int   indices[MaximumDirectives];
   // int   count = getDirectiveList( indices, dirMask );
   
   for( itr = directives.begin(); itr != directives.end(); itr++ )
      if( itr->type & dirMask )
      {
         bool  matches = true;
         
         if( ordExpr ){
            char number[12];
            itoa( itr->order, number, 10 );
            matches = FindMatch::isMatch( ordExpr, number );
         }
         
         if( matches ){
            if( function == NULL )
               function = stringTable.insert(func);
            itr->CBs[ dcb ] = function;
         }
      }
   return 0;      // no error.  
}

#if 0
// DO fix for installing on last with 0.  
const char * 
AIObj::installDCB( int dcb, const char * func, const char * ordExpr, int dirMask )
{
   if( --dcb < 0 || dcb >= NumCallbacks )
      return avar("Callback # must be in the range [%d,%d]", 1, NumCallbacks);

   DCB   function = NULL;
   
   int   indices[MaximumDirectives];
   int   count = getDirectiveList( indices, dirMask );
   
   for( int i = 0; i < count; i++ )
   {
      Directive   dir = directives[indices[i]];
      
      if( dir.type & dirMask )
      {
         bool  matches = true;
         
         if( ordExpr ){
            char number[12];
            itoa( itr->order, number, 10 );
            matches = FindMatch::isMatch( ordExpr, number );
         }
         
         if( matches ){
            if( function == NULL )
               function = stringTable.insert(func);
            itr->CBs[ dcb ] = function;
         }
      }
   }
   return 0;      // no error.  
}
#endif

// Install CB for the AI.  
void AIObj::installCB( UInt16 whichCallback, const char * func )
{
   AssertFatal( whichCallback < NumCallbacks, "aiHelp: bad callback number" );
   CBs [ whichCallback ] = func ? stringTable.insert( func ) : NULL;
}


// Each new AI object has these callbacks set up by default.  
void AIObj::setDefaultCallbacks(void)
{
   CBs[AiDied]       = "AI::onDroneKilled";
   CBs[AiBored]      = 0;
   CBs[AiPeriodic]   = "AI::onPeriodic";
}

// Each Directive that is added can have certain default callbacks added.  
void AIObj::Directive::setDefaultCallbacks(void)
{
   memset( CBs, 0, sizeof(CBs) );
   switch( type )
   {
      case  WaypointDirective:
         CBs[WaypointReached] = 0;
         CBs[WaypointIntermittent] = 0;
         CBs[WaypointQueryFace] = 0;
         break;
      
      case  TargetDirective:
         CBs[TargetAcquired] =      "AI::onTargetLOSAcquired";
         CBs[TargetKilled] =        "AI::onTargetDied";
         CBs[TargetLOSLost] =       "AI::onTargetLOSLost";
         CBs[TargetLOSRegained] =   "AI::onTargetLOSRegained";
         break;
         
      case  FollowDirective:
         CBs[FollowDestReached] = 0;
         CBs[FollowOffsetReached] = 0;
         break;
      
      case  GuardDirective:
         CBs[GuardBored] = 0;
         break;
   }
}

//
// Patch through to the callback.  Each directive has up to four callbacks, as well
// as the AI object itself.  See the enums for the meanings of them.  
//
// This function also parses out parameters for the callbacks and so this is a 
// good place to put/view documentation on all the callbacks.  
// 
bool AIObj::checkCallback( UInt16 whichCallback, Directive * dir /*=NULL*/ )
{
   AssertFatal( whichCallback < NumCallbacks, "aiHelp: bad callback index" );
   
   DCB   * callbacks = dir ? dir->CBs : CBs;
   
   if( callbacks[ whichCallback ] )
   {
      char     commandBuff[314];
      char *   curBuf = commandBuff;
      char *   argv[14];
      int      N = 0;
      
      argv[N++] = const_cast<char *>(callbacks[ whichCallback ]);
      argv[N++] = name;

      if( dir )
      {
         switch( dir->type )
         {
            case  TargetDirective:     // Pass in AI name and the target's rep ID.  
               // curBuf += 
               sprintf( argv[N++] = curBuf, "%d", dir->playerId );
               break;
         }
      }
      
      Console->execute( N, (const char * *)argv );
      return true;
   }
   else
      return false;
}



bool AIObj::haveLOS( Player * from, Player * to )
{
   SimContainer *root = findObject(manager, SimRootContainerId, root);
   AssertFatal(root != NULL, "AIObj:  have root?");
   
   Point3F  m_endPoint = to->getEyeTransform().p;
   Point3F  m_startPoint = from->getEyeTransform().p;

   SimCollisionInfo  collisionInfo;
   SimContainerQuery collisionQuery;

   // Setting id to source makes it not get included in intersection.
   collisionQuery.id       = from->getId();
   collisionQuery.type     = -1;
   collisionQuery.mask     = collisionMask;
   collisionQuery.detail   = SimContainerQuery::DefaultDetail;
   collisionQuery.box.fMin = m_startPoint;
   collisionQuery.box.fMax = m_endPoint;
   
   if ( !root->findLOS(collisionQuery, &collisionInfo) ||
                           static_cast<Player *>(collisionInfo.object) == to ) 
      return true;
   else
      return false;
}



//-----------------------------------------------------------------------------------
//       Incrementor


float AIObj::Incrementor::getInc(float absDiff)
{
   float    inc = increment;
   if ( absDiff < reduce )
   {
      inc *= (absDiff / reduce);
      if( inc < min_inc )  
         inc = min_inc;
      if( inc > absDiff )
         inc = absDiff;
   }
   return inc;
}


void AIObj::Incrementor::init(float inc)
{
   increment = inc;
   reduce = 2.0 * inc;
   min_inc = 0.1 * inc;
}



//
// Given (0 <= percent <= 1).  Return true that percentage of the time. 
//
bool AIObj::randPct( float percent )
{
   if( percent < 0.0 )
      percent = 0.0;
   else if( percent > 1.0 )
      percent = 1.0;

   bool retval = (rand.getFloat() < percent);

#if 0
   // Gather some stats to check our random's behavrior.  
   static   int      totalCalls = 0;
   static   int      totalTrue = 0;
   static   float    totalPct = 0.0;
   
   totalPct += percent;
   totalCalls++;
   if( retval )
      totalTrue++;
   
   float expectedPct = (totalPct / (float)totalCalls) * 100.0;
   float actualPct = ((float)totalTrue / (float)totalCalls) * 100.0;
   
   if( fabs( expectedPct - actualPct ) > 0.3 )
      rand.getFloat();     // avoid BCC warn.  
#endif

   return retval;
}


//-------------------------------------------------------------------------------------
//       IQVariables.  
// 

#define  BestSpotDist            150.0
#define  WorstSpotDist           50.0

#define  BestTurnInc             0.07
#define  WorstTurnInc            0.05

#define  BestPitchInc            0.03
#define  WorstPitchInc           0.008

#define  BestRunSpeed            1.0
#define  WorstRunSpeed           0.4

#define  BestMinVertDope         0.0
#define  BestMaxVertDope         0.2
#define  WorstMinVertDope        1.3
#define  WorstMaxVertDope        4.0

AIObj::IQVariables::IQVariables(bool isBest)
{
   spotDistOverride = -1.0;

   if( isBest ){
      thinkWait = bestThinkWait;
      spotDist = BestSpotDist;
      maxSpeed = BestRunSpeed;
      
      pitchInc.init( BestPitchInc );      // these control how fast or well the AI hones
      turnInc.init( BestTurnInc );        //   in on locations or targets.  
      
      vertDope[0] = BestMinVertDope;      // we control AI accuracy by "doping" otherwise
      vertDope[1] = BestMaxVertDope;      // good shots.  
   }
   else{// default
      thinkWait = worstThinkWait;
      spotDist = WorstSpotDist;
      maxSpeed = WorstRunSpeed;
      
      pitchInc.init( WorstPitchInc );
      turnInc.init( WorstTurnInc );
      
      vertDope[0] = WorstMinVertDope;
      vertDope[1] = WorstMaxVertDope;
   }
   
   smartGuyWeaponRange = 145.0;
   dumbGuyWeaponRange = 55.0;
   smartGuyMinAccuracy = 0.2;
}

void AIObj::IQVariables::scale(float unitScale /* in [0,1] */)
{
   IQVariables    best(true);
   IQVariables    worst(false);
   
   thinkWait = scaleBetween( worst.thinkWait, best.thinkWait, unitScale );
   spotDist = scaleBetween( worst.spotDist,  best.spotDist,  unitScale );
   maxSpeed = scaleBetween( worst.maxSpeed,  best.maxSpeed,  unitScale );
   
   pitchInc.init( scaleBetween( WorstPitchInc, BestPitchInc, unitScale ) );
   turnInc.init( scaleBetween( WorstTurnInc, BestTurnInc, unitScale ) );
   
   for( int i = 0; i < 2; i++ )
   {
      vertDope[i] = scaleBetween( worst.vertDope[i], best.vertDope[i], unitScale );
   }
   
   curWeaponRange = scaleBetween( dumbGuyWeaponRange, smartGuyWeaponRange, unitScale );
   curWeaponMinPct = scaleBetween( 0.0f, smartGuyMinAccuracy, unitScale );
   curWeaponMaxPct = unitScale;
}

// Get a hit percentage for the given range.  
float AIObj::IQVariables::getWeaponAccuracy( float range )
{
   float    pct = getPercentBetween( range, 0.0f, curWeaponRange );
   pct = scaleBetween( curWeaponMaxPct, curWeaponMinPct, pct );
   // static float   close_pct = 0.8;
   // static float   close_range = 16;
   //static float   close_range = 0;
   // if( range < close_range )
   //    if( pct < close_pct )
   //       pct = close_pct;
   return pct;
}


float AIObj::IQVariables::spotDistance(void)
{
   if( spotDistOverride < 0.0 )
      return spotDist;
   else
      return spotDistOverride;
}



//-------------------------------------------------------------------------------------
//    Trigger 
// 


bool AIObj::Trigger::press(void)
{
   wasPressed = 24;
   if( counter < windup )
      return true;
   else
      return AIObj::randPct(percent);
}

// must call this once at start of IQ frame.
void AIObj::Trigger::preLoop(void)
{
   if( wasPressed )
   {
      if( ++counter > windup )
         counter = windup;
         
      if( --wasPressed <= 0 )
         wasPressed = 0;
   }
   else 
      counter = 0;
}
