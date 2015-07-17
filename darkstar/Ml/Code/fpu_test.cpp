//------------------------------------------------------------------------------
// Description 
//    
// $Workfile$
// $Revision$
// $Author  $
// $Modtime $
//
//------------------------------------------------------------------------------

#include <ml.h>
#include <stdio.h>
#include <stdlib.h>
#include <g_timer.h>

char* toStr(Point3F &p)
{
   char *str = new char[40];
   sprintf(str, "p(%4.2f, %4.2f, %4.2f)", p.x, p.y, p.z);
   return (str);
}   


void main()
{
   const int ITER = 100000;
   int time;
   float f = .5169;


   //-------------------------------------- m_sqrt
   {
      gfx_push_time();
      for (int i=0; i<ITER; i++)
         m_sqrt(f);
      time = gfx_pop_time();
      printf("m_sqrt(f) ~%d cycles\n", time/ITER);   
   }      
   //-------------------------------------- m_sqrtf
   {
      gfx_push_time();
      for (int i=0; i<ITER; i++)
         m_sqrtf(f);
      time = gfx_pop_time();
      printf("m_sqrtf(f) ~%d cycles\n", time/ITER);   
   }
   //-------------------------------------- ERROR
   {
      float maxError = 0.0;
      for (float f=0.01; f < 100.0; f+=.1)
      {
         float v  = m_sqrt(f);
         float vf = m_sqrtf(f);
         float error = fabs(v-vf)/v;
         if (error > maxError)
            maxError = error;
      }
      printf("  maxError %2.5f\n", maxError);
   }
   printf("\n");
   
   //------------------------------------------------------------------------------
   Point3F p(1.3, 100.333, -10.11);
   //-------------------------------------- point3F.len()
   {
      gfx_push_time();
      for (int i=0; i<ITER; i++)
         p.len();
      time = gfx_pop_time();
      printf("Point3F.len() ~%d cycles\n", time/ITER);   
   }
   //-------------------------------------- point3F.lenf()
   {
      gfx_push_time();
      for (int i=0; i<ITER; i++)
         p.lenf();
      time = gfx_pop_time();
      printf("Point3F.lenf() ~%d cycles\n", time/ITER);   
   }
   //-------------------------------------- ERROR
   {
      randomize();
      float maxError = 0.0;
      for (int i=0; i<ITER; i++)
      {
         Point3F p(rand(),rand(),rand()); 
         float v  = p.len();
         float vf = p.lenf();
         float error = fabs(v-vf)/v;
         if (error > maxError)
            maxError = error;
      }
      printf("  maxError %2.5f\n", maxError);
   }
   printf("\n");
   
   //------------------------------------------------------------------------------
   //-------------------------------------- point3F.normalize()
   {
      gfx_push_time();
      for (int i=0; i<ITER; i++)
         p.normalize();
      time = gfx_pop_time();
      printf("Point3F.normalize() ~%d cycles\n", time/ITER);   
   }
   //-------------------------------------- point3F.normalizef()
   {
      gfx_push_time();
      for (int i=0; i<ITER; i++)
         p.normalizef();
      time = gfx_pop_time();
      printf("Point3F.normalizef() ~%d cycles\n", time/ITER);   
   }
   //-------------------------------------- ERROR
   {
      srand(0);
      float maxError = 0.0;
      for (int i=0; i<ITER; i++)
      {
         Point3F p(random(10)+.01,random(10)+.01,random(10)+.01); 
         Point3F p1(p);
         p.normalize();
         p1.normalizef();
         float v  = p.len();
         float vf = p1.len();
         float error = fabs(v-vf);
         if (error > maxError)
            maxError = error;
      }
      printf("  maxError %2.5f\n", maxError);
   }
   printf("\n");
   
   //------------------------------------------------------------------------------
   {
      gfx_push_time();
      for (int i=0; i<ITER; i++)
         m_log2((float)123.4);
      time = gfx_pop_time();
      printf("m_log2() ~%d cycles\n", time/ITER);   

      gfx_push_time();
      for (int i=0; i<ITER; i++)
         m_log2f((float)123.4);
      time = gfx_pop_time();
      printf("m_log2f() ~%d cycles\n", time/ITER);   
   }
   printf("\n");

   //------------------------------------------------------------------------------
   {
      gfx_push_time();
      for (int i=0; i<ITER; i++)
         m_pow2((float)11.4);
      time = gfx_pop_time();
      printf("m_pow2() ~%d cycles\n", time/ITER);   

      gfx_push_time();
      for (int i=0; i<ITER; i++)
         m_pow2f((float)11.4);
      time = gfx_pop_time();
      printf("m_pow2f() ~%d cycles\n", time/ITER);   
   }
   printf("\n");

   //------------------------------------------------------------------------------
   {
      gfx_push_time();
      for (int i=0; i<ITER; i++)
         m_fog(.5,.3);
      time = gfx_pop_time();
      printf("m_fog() ~%d cycles\n", time/ITER);   
   }
}   