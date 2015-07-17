//-----------------------------------------------------------------------------------//
//                                                                                   //
//       aiSkill.cpp                                                                 //
//                                                                                   //
//-----------------------------------------------------------------------------------//

#include <sim.h>
#include "aiObj.h"

namespace Tweak
{
   float    startScale = 0.2;
   float    midScale = 0.2;
   float    endingScale = 0.04;
   int      jumpWait = 10;
   
   float    stoppedSpeed = 0.3;
};


// Basic method for getting from point A to point B, with constraints determined by 
// the "path planner".  This routine won't necessarily work if the starting state
// of the player is off to much, such as having too great a velocity in another 
// direction.  
float AIObj::jetBetween( 
   const Point3F & A, const Point3F & B, 
   float startVelPct, float endVelPct, 
   PlayerMove & pm, 
   JetSkill & js
)
{
   setFrameAccurate();

   Point3F  playerPos   = ourCenter();
   // float    total2D = dist2d( A, B );
   float    remain2D = dist2d( playerPos, B );
   float    total3D = dist3d( A, B );
   float    remain3D = dist3d( playerPos, B );
   
   if( remain3D > 0.01 )    // assure non-zero seek vector for normalization below
   {
      int      i;
   
      // Values for figuring out target velocities.  
      float    maxLateralSpeed = player->data->maxJetForwardVelocity;
      float    startSpeed = startVelPct * maxLateralSpeed;
      float    endSpeed = endVelPct * maxLateralSpeed;
      float    percent = getPercentBetween( remain3D, total3D, 0.0f );
      Point3F  targetPoint = B;
      
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
      Point3F  actualForce = player->getLinearVelocity() - js.saveLinearVel;
      if( actualForce.lenf() > 10.0 )
      {
         // We're not in control here....  is this how to handle it...?  
         actualForce.set();
      }
      float    dotProd = m_dot( actualForce, targetVector );
      if( dotProd > desiredSpeed )
         dotProd = desiredSpeed;
      Point3F  pointOnVec = targetVector * dotProd;
      Point3F  adjustedTargetVel = desiredVelocity + (pointOnVec - actualForce);
      js.saveLinearVel = player->getLinearVelocity();
      
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
      // js.expectedForce = ;
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
   
   return remain2D;
}


// default virtual skill execution
bool AIObj::Skill::doIt( AIObj * ai, PlayerMove & pm )
{
   ai;
   pm;
   
   return false;
}


bool AIObj::updateJetNavigation( JetSkill & js, PlayerMove & pm )
{
   bool     doneFlag = false;

   Point3F  lvel = player->getLinearVelocity();
   float    speed = lvel.lenf();
         
   // Process has stages...  
   //
   if( js.stage == JetSkill::Jump )
   {
      // jump.  we could wait until we're stopped for this, or TURN to dest.  
      
      AngleSeek   Z = turnToLoc( js.points[js.pathLength-1], pm );
      
      if( Z < 0.08f )
      {
         if( speed < Tweak::stoppedSpeed )
         {
            pm.jumpAction = true;
            js.incStage();
            js.jumpCount = 0;
            js.jumpWait = Tweak::jumpWait;
         }
      }
   }
   else if ( js.stage == JetSkill::Jumped )
   {
      // jet straight up.  
      pm.jetting = true;
      if( js.jumpCount++ > js.jumpWait )
      {
         js.incStage();
      }
   }
   else if( js.stage >= JetSkill::StartFlight && js.stage < JetSkill::StartFlight + js.pathLength )
   {
      int         indA = js.stage - JetSkill::StartFlight;
      int         indB = indA + 1;
      Point3F &   ptA = js.points[indA];
      Point3F &   ptB = js.points[indB];
      float       velA = js.vels[indA], velB = js.vels[indB], remainD;
      bool        firstTime = js.stageWas(JetSkill::Jumped);

      if( firstTime )
      {
         js.saveLinearVel = player->getLinearVelocity();
      }
      
      remainD = jetBetween( ptA, ptB, velA, velB, pm, js );
      
      // we can check for INCREASE to this for possible state reset.  
      if( remainD < 1.0 )
      {
         js.incStage();
      }
      
      if( speed < 0.03 )
         doneFlag = true;
   }
   else 
   {
      doneFlag = true;
   }
   
   // Occasionally, give them full energy.  We need to figure out a way to do this 
   // BUT to track how much is used as well, and then we can see what kinds of jumps
   // are reasonable.  
   if( randPct( 0.99 ) )
   {
      float maxE = player->data->maxEnergy;
      player->setEnergy( maxE );
   }
   else
   {
      // how often does this happen?  
      // bogus code... 
      if( speed > 100000 )
         doneFlag = true;
   }
   
   return doneFlag;
}


//
// Set up a path based on these, using the percentage fields above.  
//
AIObj::JetSkill::JetSkill( const Point3F & startpoint, const Point3F & endpoint )
{
   setStage(Jump);

   points[0] = startpoint;
   points[2] = endpoint;
   
   Point3F     midpoint    = startpoint;
   midpoint += endpoint;
   midpoint /= 2.0;
   // midpoint.z += jetNavigation;
   midpoint.z += 40.0;
   points[1] = midpoint;
   
   pathLength = 3;
   
   vels[0] = Tweak::startScale;
   vels[1] = Tweak::midScale;
   vels[2] = Tweak::endingScale;
}



/*
   - set up based on what we have (two segments, plus needed other sub-states).  
   - add in different paths
   - experiment with different metrics for "arrival" at each point.  try to find the 
         paths that work.  for example, one path might be to go up-over-down, with 
         arrival at the second of the two intermediate nodes being 
   - stay in one place up the air: THAT would be a good option.  
 
   Notes:
   
   Running jumps will probably only happen from flat terrain - where we can figure out
   and anticipate the launch velocity reliably.  
   
   Idea - 'shield pack' drone could perhaps jet better.  The seek point above landing,
      cut to zero lateral velocity,  FALL on target point and sheild before impact.  

   Once we find a path method or methods that work, we'll use those and not worry about 
   'perfect' jetting, such as leaping up anywhere (maybe while running) and doing
   fine tuned adjustments along the way.  We can require "positioning" and preparation
   in other words.  In open spaces or when under seige (radical evasives) they
   may do this though.  Otherwise they will try to plan, and then detect if the plan
   fails (such as a mortor blows them off the nice path), and then re-land, and
   seek to plan again.  
   
   
   
*/

