//--------------------------------------------------------------------------- 


//--------------------------------------------------------------------------- 

//#pragma warn -inl
//#pragma warn -aus

#include <ts.h>
#include <base.h>
#include <sim.h>

#include "genericActions.h"

//--------------------------------------------------------------------------- 

char *actionTable[] = 
{
    "None",         "Pause",          "Fire",      "StartFire",     "EndFire", 
    "SelectWeapon", "MoveYaw",        "MovePitch", "MoveRoll",      "MoveForward",
    "MoveBackward", "MoveLect",       "MoveRight", "MoveUp",        "MoveDown",
    "ViewYaw",      "ViewPitch",      "ViewRoll",  "IncreaseSpeed", "DecreaseSpeed",
    "SetSpeed",     "SetDetailLevel", NULL
};

//--------------------------------------------------------------------------- 


