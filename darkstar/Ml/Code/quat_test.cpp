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

char* toStr(QuatF &q)
{
   char *str = new char[40];
   sprintf(str, "q(%4.2f, %4.2f, %4.2f: %4.2f)", q.x, q.y, q.z, q.w);
   return (str);
}   


void main()
{
   const int ITER = 100000;
   int time;
   

//   EulerF e(0.1, 0.2, 0.3);
//   QuatF quat;
//   QuatF a;
//   QuatF b;
//   QuatF c;
//
//   a.set(1.0,2.0,0.0,0.0);
//   b.set(1.0,2.0,0.0,0.0);
//   m_mul(a,b,&c);
//   printf("%s \n",toStr(a));
//   printf("%s \n",toStr(b));
//   return;
//
      EulerF e(0.1, 0.2, 0.3);
      QuatF quat;
      quat.set(e);


   //-------------------------------------- TEST x
   {
      EulerF e(0.1, 0, 0);
      RMat3F rmat;
      QuatF quat;

      gfx_push_time();
      for (int i=0; i<ITER; i++)
         rmat.set(e);
      time = gfx_pop_time();
      printf("RMat.set(Euler x) ~%d cycles\n", time/ITER);   

      gfx_push_time();
      for (int i=0; i<ITER; i++)
         quat.set(e);
      time = gfx_pop_time();
      printf("Quat.set(Euler x) ~%d cycles\n", time/ITER);   
   }
   printf("\n");   
   //-------------------------------------- TEST xy
   {
      EulerF e(0.1, 0.2, 0);
      RMat3F rmat;
      QuatF quat;

      gfx_push_time();
      for (int i=0; i<ITER; i++)
         rmat.set(e);
      time = gfx_pop_time();
      printf("RMat.set(Euler xy) ~%d cycles\n", time/ITER);   

      gfx_push_time();
      for (int i=0; i<ITER; i++)
         quat.set(e);
      time = gfx_pop_time();
      printf("Quat.set(Euler xy) ~%d cycles\n", time/ITER);   
   }
   printf("\n");   
   //-------------------------------------- TEST xyz
   {
      EulerF e(0.1, 0.2, 0.3);
      RMat3F rmat;
      QuatF quat;

      gfx_push_time();
      for (int i=0; i<ITER; i++)
         rmat.set(e);
      time = gfx_pop_time();
      printf("RMat.set(Euler xyz) ~%d cycles\n", time/ITER);   

      gfx_push_time();
      for (int i=0; i<ITER; i++)
         quat.set(e);
      time = gfx_pop_time();
      printf("Quat.set(Euler xyz) ~%d cycles\n", time/ITER);   
   }
   printf("\n");   

   //------------------------------------------------------------------------------
   //-------------------------------------- TEST quat.set(RMat)
   {
      EulerF e2(0.3, 0.5, 0.6);
      RMat3F rmat(e);
      QuatF  quat(e);

      gfx_push_time();
      for (int i=0; i<ITER; i++)
         quat.set(rmat);
      time = gfx_pop_time();
      printf("quat.set(RMat) ~%d cycles\n", time/ITER);   
   }

   //-------------------------------------- TEST rmat.set(quat)
   {
      EulerF e2(0.3, 0.5, 0.6);
      RMat3F rmat(e);
      QuatF  quat(e);

      gfx_push_time();
      for (int i=0; i<ITER; i++)
         rmat.set(quat);
      time = gfx_pop_time();
      printf("rmat.set(quat) ~%d cycles\n", time/ITER);   
   }
   printf("\n");   


   //------------------------------------------------------------------------------
   //-------------------------------------- TEST v*rot
   {
      Vector3F result, v(1,2,3);
      EulerF e(0.1, 0.2, 0.3);
      RMat3F rmat(e);
      QuatF quat(e);

      gfx_push_time();
      for (int i=0; i<ITER; i++)
         m_mul(v, rmat, &result);
      time = gfx_pop_time();
      printf("m_mul(pt, rmat) ~%d cycles\n", time/ITER);   

      gfx_push_time();
      for (int i=0; i<ITER; i++)
         m_mul(v, quat, &result);
      time = gfx_pop_time();
      printf("m_mul(pt, quat) ~%d cycles\n", time/ITER);   
   }
   printf("\n");   

   //-------------------------------------- TEST rot*rot
   {
      EulerF e(0.1, 0.2, 0.3);
      RMat3F rmat(e);
      RMat3F rmat2(e);
      RMat3F rmat3;
      QuatF quat(e);
      QuatF quat2(e);
      QuatF quat3;

      gfx_push_time();
      for (int i=0; i<ITER; i++)
         m_mul(rmat, rmat2, &rmat3);
      time = gfx_pop_time();
      printf("m_mul(rmat, rmat) ~%d cycles\n", time/ITER);   

      gfx_push_time();
      for (int i=0; i<ITER; i++)
         m_mul(quat, quat2, &quat3);
      time = gfx_pop_time();
      printf("m_mul(quat, quat) ~%d cycles\n", time/ITER);   
   }
   printf("\n");   


   //-------------------------------------- TEST inverse
   {
      EulerF e(0.1, 0.2, 0.3);
      RMat3F rmat(e);
      QuatF quat(e);

      gfx_push_time();
      for (int i=0; i<ITER; i++)
         rmat.inverse();
      time = gfx_pop_time();
      printf("rmat.inverse() ~%d cycles\n", time/ITER);   

      gfx_push_time();
      for (int i=0; i<ITER; i++)
         quat.inverse();
      time = gfx_pop_time();
      printf("quat.inverse() ~%d cycles\n", time/ITER);   
   }
   printf("\n");   

   //-------------------------------------- TEST interp
   {
      EulerF e(0.1, 0.2, 0.3);
      EulerF e2(0.3, 0.5, 0.6);
      QuatF quat(e);
      QuatF quat2(e);

      float t=0.5;
      gfx_push_time();
      for (int i=0; i<ITER; i++)
         quat.interpolate(quat, quat2, t);
      time = gfx_pop_time();
      printf("quat.slerp() ~%d cycles\n", time/ITER);   
   }
   printf("\n");   



}   