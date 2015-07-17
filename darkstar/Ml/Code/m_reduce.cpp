#include "ml.h"

AngleF DLLAPI 
m_reduce( const AngleF a )
{
   RealF temp = m_AngleF_reduce(a);
   if (temp < 0.0)
      temp += RealF_2Pi;
   return temp;
}

AngleD DLLAPI 
m_reduce( const AngleD a )
{
   RealD temp = m_AngleD_reduce(a);
   if (temp < 0.0)
      temp += RealD_2Pi;
   return temp;
}

