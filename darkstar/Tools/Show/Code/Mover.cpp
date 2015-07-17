//--------------------------------------------------------------------------- 


//--------------------------------------------------------------------------- 

#include <ts.h>
#include <base.h>
#include <sim.h>

#include "simGame.h"
#include "simAction.h"
// #include "simEv.h"

#include "mover.h"

#define ROT_STANDARD_VEL (M_2PI/20.0f)
#define POS_STANDARD_VEL (100.0f)

void Eset( RMat3F const &mat, EulerF & e)
{
#define IsEqual(a,b) (fabs(a-b)<=.00001f)
	e.x = m_asin(mat.m[1][2]);

   if( IsEqual(e.x,(float) M_PI/2) || IsEqual(e.x,(float) -M_PI/2) ) 
   {
		e.y = 0;
	   	e.z = m_atan(mat.m[0][0], mat.m[0][1]);
   }
   else 
   {
		e.y = m_atan(mat.m[2][2], -mat.m[0][2]);
     	e.z = m_atan(mat.m[1][1], -mat.m[1][0]);
   }
}

void moverHelper::init()
{
  pos.set(0.0f,0.0f,0.0f);
  posVel.set(0.0f,0.0f,0.0f);
  rot.set(0.0f,0.0f,0.0f);
  rotVel.set(0.0f,0.0f,0.0f);
  orbitRot.set(0.0f,0.0f,0.0f);
  orbitDist = 0.0f; // user should set this
  moveMode = moveRelative;
}

moverHelper::moverHelper()
{
  init();
}

moverHelper::~moverHelper()
{
}

void moverHelper::processAction(int action, float mag) 
{
  switch(action)
  {
	case ViewRoll:
		rotVel.y=mag*ROT_STANDARD_VEL;
		break;
	case ViewPitch:
		rotVel.x=mag*ROT_STANDARD_VEL;
		break;
	case ViewYaw:
		rotVel.z=mag*ROT_STANDARD_VEL;
		break;
	case MoveUp:
		if (moveMode!=moveOrbital)
			posVel.z=mag*POS_STANDARD_VEL;
		break;
	case MoveDown:
		if (moveMode!=moveOrbital)
			posVel.z=-mag*POS_STANDARD_VEL;		
		break;
	case MoveForward:
		posVel.y=mag*POS_STANDARD_VEL;
		break;
	case MoveBackward:
		posVel.y=-mag*POS_STANDARD_VEL;
		break;
	case MoveRight:
		if (moveMode!=moveOrbital)
			posVel.x=mag*POS_STANDARD_VEL;
		break;
	case MoveLeft:
		if (moveMode!=moveOrbital)
			posVel.x=-mag*POS_STANDARD_VEL;
		break;
  }
}

void moverHelper::update(SimTime dt)
{
	if (moveMode==moveOrbital)
	{
	  orbitDist-= dt * posVel.y;
	  orbitRot.x += dt * rotVel.x;
	  orbitRot.y += dt * rotVel.y;
	  orbitRot.z -= dt * rotVel.z;
	  orbitRot.reduce();
	  return;
	}

	// first deal with position offset
	Point3F absPosVel;
	switch(moveMode) 
	{
	  case moveAbsolute:
		  absPosVel.set(posVel.x,posVel.y,posVel.z);
		  break;
	  case moveRelative:
		  m_mul(posVel,RMat3F(rot),&absPosVel);
		  break;
	  case moveSemiRelative:
	  {
		  float zVel = posVel.z;
		  posVel.z=0.0f;
		  m_mul(posVel,RMat3F(rot),&absPosVel);
		  // now get rid of z vel. and set to orig. value
		  float totV = absPosVel.lenf();
		  absPosVel.z = 0.0f;
		  if ((totV>.01f) && (absPosVel.lenf()>.01f)) // safety's sake
		    absPosVel.normalizef(totV);
		  absPosVel.z = zVel;	
		  break;
	  }
	}
	pos.x += dt * absPosVel.x;
	pos.y += dt * absPosVel.y;
	pos.z += dt * absPosVel.z;

 	// now rotate
	EulerF scaledRotV;
	scaledRotV.x = dt * rotVel.x;
	scaledRotV.y = dt * rotVel.y;
	scaledRotV.z = dt * rotVel.z;
	RMat3F rotMat;
	rotMat.set(scaledRotV);
	RMat3F curMat;
	curMat.set(rot);
	RMat3F laterMat;
	m_mul(rotMat,curMat,&laterMat);
	Eset(laterMat,rot);
 
   return;
} // end of moverHelper::update

