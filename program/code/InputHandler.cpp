#include <sim.h>
#include <simaction.h>
#include <timer.h>
#include <inputhandler.h>

InputHandler::InputHandler()
{
   rValue            = 0.0f;
   rTarget           = 0.0f;
   diFocus           = NULL;
   fAnalogStick      = false;
   rAnalogStickRange = 0.25f;

   timer.reset();
}

InputHandler::~InputHandler()
{
   // This space for rent
}

void InputHandler::sample(float rDelta, int iDevice)
{
   if (iDevice & SimActionTypeDigital)
   {
      sampleDigital(max(min(rDelta, 1.0f), -1.0f), !(iDevice & SimActionTypePOV));
   }
   else if (iDevice & SimActionTypeDelta)
   {
      sampleMouse(rDelta);
   }
   else if (iDevice & SimActionTypeAnalog)
   {
      sampleAnalog(max(min(rDelta, 1.0f), -1.0f));
   }
}

void InputHandler::sampleDigital(float rDelta, bool fIsKeyboard)
{
   fIsKeyboard;
   
   diFocus = &diDigital;

   // If the value was zero ...
   if (rDelta == 0.0f)
   {
      reset();
   }
   // If it was a positive value ...
   else
   {
      rTarget = m_limit_range(rDelta, 1.0f);
   }
}

void InputHandler::sampleAnalog(float rDelta)
{
   if (fAnalogStick)
   {
      if (fabs(rDelta - rAnalogStickValue) > rAnalogStickRange)
      {
         fAnalogStick = false;
      }
      else
      {
         return;
      }
   }

   if (diFocus == &diAnalog || rDelta != 0.0f)
   {
      diFocus = &diAnalog;
      rTarget = m_limit_range(rDelta, 1.0f);
   }
}

void InputHandler::sampleMouse(float rDelta)
{
   diFocus = &diMouse;
   rTarget = m_limit_range(rDelta, 1.0f);
}

void InputHandler::reset()
{
   rValue = rTarget = 0.f;
}

float InputHandler::getInput()
{
   float rTime = timer.getElapsed();

   timer.reset();

   if (diFocus)
   {
      // Mice are read as absolute motion
      if (diFocus == &diMouse)
      {
         rValue  = rTarget;
         rTarget = 0.0f;
      }
      // All others are read as "velocity" motion
      else
      {
         rValue = m_limit_range(rTarget * rTime, 1.0f);
      }

      return (rValue);
   }

   return (0.0f);
}
