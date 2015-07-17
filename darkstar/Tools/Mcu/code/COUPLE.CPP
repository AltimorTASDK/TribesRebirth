/*
 * Based on C code from the article
 * "Fast Collision Detection of Moving Convex Polyhedra"
 * by Rich Rabbitz, rrabbitz%pgn138fs@serling.motown.ge.com
 * in "Graphics Gems IV", Academic Press, 1994
 */
#include "stdafx.h"
#include "couple.h"

#define GET(u,i,j,s) (*(u+i*s+j))
#define GET3(u,i,j,k,s) (*(u+i*(s*2)+(j*2)+k))

/****************************************************************************
 *
 *   Alternate function to compute the point in a polytope closest to the
 *   origin in 3-space. The polytope size m is restricted to 1 < m <= 4.
 *   This function is called only when comp_sub_dist fails.
 *
 *   On Entry:
 *	pPoints  - table of 3-element points containing polytope's vertices.
 *	stopIndex - number of sets to test.
 *	D_P	   - array of determinants for each set.
 *	Di_P	   - cofactors for each set.
 *	Is	      - indices for each set.
 *	c2	      - row size offset.
 *
 *   On Exit:
 *
 *   Function Return :
 *	the index of the set that is numerically closest to eq. (14).
 *
 ****************************************************************************/

int subDistAlternate(Point3D *pPoints, int stopIndex, RealD *D_P, RealD Di_P[][4], int *Is, int c2)
{
   RealD	  sum, v_aff, best_v_aff;

   Bool first = TRUE;
   int best_s = -1;
   for( int s = 0; s < stopIndex; s++ )
      {
      Bool pass = TRUE;
      if (D_P[s] > 0.0)
         {
	      for( int i = 1; i <= Is[s*c2]; i++ )
            {
	         int is = Is[s*c2+i];
	         if (Di_P[s][is] <= 0.0)
	            pass = FALSE;
	         }
         }
      else
	      pass = FALSE;

      if (pass)
         {
         /*** Compute equation (33) in Gilbert ***/

	      int k = Is[s*c2+1];
	      sum = 0;
	      for (int i = 1; i <= Is[s*c2]; i++)
            {
	         int is = Is[s*c2+i];
	         sum += Di_P[s][is] * m_dot( pPoints[is], pPoints[k] );
	         }
	      v_aff = sqrt( sum / D_P[s] );
	      if (first)
            {
	         best_s = s;
	         best_v_aff = v_aff;
	         first = FALSE;
	         }
	      else
            {
	         if (v_aff < best_v_aff)
               {
	            best_s = s;
	            best_v_aff = v_aff;
	            }
	         }
         }
      }
   if( best_s == -1 )
      {
      StatusPrintf("backup failed\n");
      exit(0);
      }
   return best_s;
}

/****************************************************************************
 *
 *   Function to compute the point in a polytope closest to the origin in
 *   3-space. The polytope size m is restricted to 1 < m <= 4.
 *
 *   On Entry:
 *	  pPoints  - table of 3-element points containing polytope P's vertices.
 *	  nPoints  - number of points in P.
 *	  pJo  - table of indices for storing Dj_P cofactors in Di_P.
 *	  pIs  - indices into P for all sets of subsets of P.
 *	  pIsC - indices into P for complement sets of Is.
 *   near_pnt - an empty array of size 3.
 *   pNearIndexes - an empty array of size 4.
 *   lambda - an empty array of size 4.
 *
 *   On Exit:
 *   pNearPoint - the point in P closest to the origin.
 *   pNearIndexes - indices for a subset of P which is affinely independent.
 *		See eq. (14).
 *   lambda - the lambda as in eq. (14).
 *
 *   Function Return :
 *	the number of entries in near_indx and lambda.
 *
 ****************************************************************************/

int compSubDist(Point3D *pPoints, int nPoints, int *pJo, int *pIs, int *pIsC,
   Point3D *pNearPoint, int *pNearIndexes, RealD lambda[])
{
   RealD	    D_P[15];
   RealD       Di_P[15][4];
   static int	    combinations[5] = {0,0,3,7,15};

   int stopIndex = combinations[nPoints];    /** how many subsets in P **/
   int c1 = nPoints;
   int c2 = nPoints + 1;		    /** row offsets for IsC and Is **/

   /** Initialize Di_P for singletons **/

   Di_P[0][0] = Di_P[1][1] = Di_P[2][2] = Di_P[3][3] = 1.0;
   int s = 0;
   Bool pass = FALSE;

   while ((s < stopIndex) && (!pass))
      {	/* loop through each subset */
      D_P[s] = 0.0;
      Bool fail = FALSE;
      for( int i = 1; i <= pIs[s*c2]; i++ )
         {	/** loop through all Is **/
	      int is = pIs[s*c2+i];
	      if (Di_P[s][is] > 0.00001)			/** Condition 2 Theorem 2 **/
	         D_P[s] += Di_P[s][is];		/** sum from eq. (16)	  **/
	      else
	         fail = TRUE;
         }

      for( int j = 1; j <= pIsC[s*c1]; j++)
         {	/** loop through all IsC **/
	      RealD Dj_P = 0;
         int k = pIs[s*c2+1];
	      int isp = pIsC[s*c1+j];
         Point3D x = pPoints[k];
         x -= pPoints[isp];		  /** Wk - Wj  eq. (18) **/

	      for (i = 1; i <= pIs[s*c2]; i++)
            {
	         int is = pIs[s*c2+i];
	         Dj_P += Di_P[s][is] * m_dot(pPoints[is], x); /** sum from eq. (18) **/
	         }
	      int row = GET3(pJo,s,isp,0,c1);
	      int col = GET3(pJo,s,isp,1,c1);
	      Di_P[row][col] = Dj_P;			 /** add new cofactors	**/

	      if (Dj_P > 0.00001)			 /** Condition 3 Theorem 2 **/
	         fail = TRUE;
         }
      if ((!fail) && (D_P[s] > 0.0))  /** Conditions 2 && 3 && 1 Theorem 2  **/
	      pass = TRUE;
      else
	      s++;
      }
   if (!pass)
      {
      StatusPrintf("*** using alternate procedure in sub_dist\n");
      s = subDistAlternate(pPoints, stopIndex, D_P, Di_P, pIs, c2);
      }
   else
      {
      AssertFatal( s < 15, "bad stuff in compSubDist" );
      }

   pNearPoint->set();
   for( int i = 0; i < pIs[s*c2]; i++ )
      {	 /** loop through all Is **/
      int is = pIs[s*c2+1+i];
      AssertFatal( is < nPoints, "bad index?" );
      pNearIndexes[i] = is;
      lambda[i] = Di_P[s][is] / D_P[s];		      /** eq. (17)  **/
      Point3D temp = pPoints[is];
      temp *= lambda[i];
      *pNearPoint += temp;                          /** eq. (17)  **/
      }

   return i;
}

/****************************************************************************
 *
 *   Function to compute the point in a polytope closest to the origin in
 *   3-space.  The polytope size m is restricted to 1 < m <= 4.
 *
 *   On Entry:
 *	   pPoints  - table of 3-element points containing polytope's vertices.
 *	   nPoints  - number of points in P.
 *  pNearPoint - an empty array of size 3.
 *  pNearIndexes - an empty array of size 4.
 *     lambda - an empty array of size 4.
 *
 *   On Exit:
 *   pNearPoint - the point in P closest to the origin.
 *  pNearIndexes - indices for a subset of P which is affinely independent.
 *		See eq. (14).
 *     lambda - the lambda as in eq. (14).
 *
 *   Function Return :
 *	the number of entries in pNearIndexes and lambda.
 *
 ****************************************************************************/

int subDist(Point3D *pPoints, int nPoints, Point3D *pNearPoint,
   int *pNearIndexes, RealD lambda[])
{
/*
 *
 *  Tables to index the Di_P cofactor table in comp_sub_dist.  The s,i
 *  entry indicates where to store the cofactors computed with Is_C.
 *
 */

   static int	   jo_2[2][2][2]  = { {{0,0}, {2,1}},
				      {{2,0}, {0,0}}};

   static int	   jo_3[6][3][2]  = { {{0,0}, {3,1}, {4,2}},
				      {{3,0}, {0,0}, {5,2}},
				      {{4,0}, {5,1}, {0,0}},
				      {{0,0}, {0,0}, {6,2}},
				      {{0,0}, {6,1}, {0,0}},
				      {{6,0}, {0,0}, {0,0}}};

   static int	  jo_4[14][4][2] = { { {0,0}, {4,1}, {5,2}, {6,3}},
				     { {4,0}, {0,0}, {7,2}, {8,3}},
				     { {5,0}, {7,1}, {0,0}, {9,3}},
				     { {6,0}, {8,1}, {9,2}, {0,0}},
				     { {0,0}, {0,0},{10,2},{11,3}},
				     { {0,0},{10,1}, {0,0},{12,3}},
				     { {0,0},{11,1},{12,2}, {0,0}},
				     {{10,0}, {0,0}, {0,0},{13,3}},
				     {{11,0}, {0,0},{13,2}, {0,0}},
				     {{12,0},{13,1}, {0,0}, {0,0}},
				     { {0,0}, {0,0}, {0,0},{14,3}},
				     { {0,0}, {0,0},{14,2}, {0,0}},
				     { {0,0},{14,1}, {0,0}, {0,0}},
				     {{14,0}, {0,0}, {0,0}, {0,0}}};


/*
 *  These tables represent each Is.  The first column of each row indicates
 *  the size of the set.
 *
 */
   static int	  Is_2[3][3] = { {1,0,0}, {1,1,0}, {2,0,1}};

   static int	  Is_3[7][4] = { {1,0,0,0}, {1,1,0,0}, {1,2,0,0}, {2,0,1,0},
				 {2,0,2,0}, {2,1,2,0}, {3,0,1,2}};

   static int	 Is_4[15][5] = { {1,0,0,0,0}, {1,1,0,0,0}, {1,2,0,0,0},
				 {1,3,0,0,0}, {2,0,1,0,0}, {2,0,2,0,0},
				 {2,0,3,0,0}, {2,1,2,0,0}, {2,1,3,0,0},
				 {2,2,3,0,0}, {3,0,1,2,0}, {3,0,1,3,0},
				 {3,0,2,3,0}, {3,1,2,3,0}, {4,0,1,2,3}};

/*
 *  These tables represent each Is complement. The first column of each row
 *  indicates the size of the set.
 *
 */
   static int	 IsC_2[3][2] = { {1,1}, {1,0}, {0,0}};

   static int	 IsC_3[7][3] = { {2,1,2}, {2,0,2}, {2,0,1}, {1,2,0}, {1,1,0},
				 {1,0,0}, {0,0,0}};

   static int	IsC_4[15][4] = { {3,1,2,3}, {3,0,2,3}, {3,0,1,3}, {3,0,1,2},
				 {2,2,3,0}, {2,1,3,0}, {2,1,2,0}, {2,0,3,0},
				 {2,0,2,0}, {2,0,1,0}, {1,3,0,0}, {1,2,0,0},
				 {1,1,0,0}, {1,0,0,0}, {0,0,0,0}};


   /** Call comp_sub_dist with appropriate tables according to size of P **/
   int size;
   switch (nPoints)
      {
      case 2:
	      size = compSubDist(pPoints, nPoints, (int*)jo_2, (int*)Is_2,
            (int*)IsC_2, pNearPoint, pNearIndexes, lambda);
         break;
      case 3:
	      size = compSubDist(pPoints, nPoints, (int*)jo_3, (int*)Is_3,
            (int*)IsC_3, pNearPoint, pNearIndexes, lambda);
         break;
      case 4:
	      size = compSubDist(pPoints, nPoints, (int*)jo_4, (int*)Is_4,
            (int*)IsC_4, pNearPoint, pNearIndexes, lambda);
         break;
      default:
         StatusPrintf("I:{%d,%d,%d,%d}\n",pNearIndexes[0],pNearIndexes[1],
            pNearIndexes[2],pNearIndexes[3]);
         AssertFatal(0,"call to subDist with invalid number of points");
         break;
      }

   return size;
}

/****************************************************************************
 *
 *   Function to evaluate the support and contact functions at A for the
 *   set difference of two polytopes. See equations (8) & (9).
 *
 *   On Entry:
 *	A        - vector at which to evaluate support and contact functions.
 *	pContact - an empty 3-element point.
 *	pIndex1  - a pointer to an int.
 *	pIndex2  - a pointer to an int.
 *
 *   On Exit:
 *	pContact - solution to equation 9.
 *	pIndex1  - index into P1 for solution to equation 9.
 *	pIndex2  - index into P2 for solution to equation 9.
 *
 *   Function Return :
 *	the result of the evaluation of eq. (8) for P1 and P2 at A.
 *
 ****************************************************************************/

RealD Couple::Hs( Point3D &A, Point3D *pContact, int *pIndex1, int *pIndex2 )
{
   RealD Hp_1 = ppolyhedron1->Hp( A, pIndex1);
   A.neg();
   RealD Hp_2 = ppolyhedron2->Hp( A, pIndex2 );

   *pContact = ppolyhedron1->GetVert(*pIndex1);
   *pContact -= ppolyhedron2->GetVert(*pIndex2);

   return (Hp_1 + Hp_2);
}

/*** RJR 05/26/93 ***********************************************************
 *
 *   Function to compute the minimum distance between two convex polytopes in
 *   3-space.
 *
 *   On Entry:
 *  vert_indx - a 4x2 matrix possibly containing indices of initialization
 *		points. The first column are indices into P1, and the second
 *		column are indices into P2.
 *	 n - indicates how many initial points
 *		to extract from vert_indx. If 0, vert_indx is ignored.
 *  lambda - empty
 *
 *   On Exit:
 *  vert_indx - updated indices into P1 and P2 which indicate the affinely
 *		independent point sets from each polytope which can be used
 *		to compute along with lambda the near points in P1 and P2
 *		as in eq. (12). These indices can be used to re-initialize
 *		dist3d in the next iteration.
 *  lambda - the lambda as in eqs. (11) & (12).
 *	 n - the updated number of indices for P1 and P2 in near_indx.
 *
 *   Function Return : the distance (length of Vk).
 *
 ****************************************************************************/

RealD Couple::dist3d()
{
   int         iters = 0;
   int		   I[4];
   Point3D     Pk[4], Pk_subset[4];

   n = 1;
   vert_indx[0][0] = 0;
   vert_indx[0][1] = 0;
   Pk[0] = ppolyhedron1->GetVert(0);
   Pk[0] -= ppolyhedron2->GetVert(0);	 /** first elementary polytope **/

   Bool pass = FALSE;
   Point3D Vk;
   while (!pass)
      {
      iters++;
      /** compute Vk **/
      if (n == 1)
         {
	      Vk = Pk[0];
	      I[0] = 0;
         }
      else
         {
         if( 0 && iters > 10 )
            {
            StatusPrintf( "Iters[%d]\n",iters );

            for(int i = 0; i < n; i++)
               StatusPrintf( "  I[%d]: P1: %d, P2: %d\n", 
                  i, vert_indx[I[i]][0], vert_indx[I[i]][1] );
            }
	      n = subDist(Pk, n, &Vk, I, lambda);
         /** keep track of indices for P1 and P2 **/
         for(int i = 0; i < n; i++)
            {
            AssertFatal(I[i]<=4, "Invalid point index from subDist" );
            if( 0 && iters > 10 )
               StatusPrintf( "  I[%d]: P1: %d, P2: %d\n", 
                  i, vert_indx[I[i]][0], vert_indx[I[i]][1] );
	         vert_indx[i][0] = vert_indx[I[i]][0];
   	      vert_indx[i][1] = vert_indx[I[i]][1];
            }
         }

      // Do we have a solution ? (eq. 13)
      Point3D     Cp;
      int		   P1_i, P2_i;
      Point3D negVk = Vk; negVk.neg();
      RealD Gp = m_dot(negVk, negVk) + Hs( negVk, &Cp, &P1_i, &P2_i );
      if( EqZ( Gp ) || n == 4 )
         {
	      pass = TRUE;
         if( !EqZ(Gp) )
            StatusPrintf("Gp = %lf, n = %d\n", Gp, n );
         }
      else
         {
         AssertFatal( n < 4, "Too many points in subset" );  // beware !

	      for (int i = 0; i < n; i++)
            Pk_subset[i] = Pk[I[i]];  /** extract affine subset of Pk **/

	      for (i = 0; i < n; i++)
	         Pk[i] = Pk_subset[i];  /** load into Pk+1 **/

         vert_indx[n][0] = P1_i;
         vert_indx[n][1] = P2_i;
         if( 0 && iters > 10 )
            StatusPrintf( "  I[%d]: P1: %d, P2: %d\n", 
               n, vert_indx[n][0], vert_indx[n][1] );
	      Pk[n] = Cp;		     /** Union of Pk+1 with Cp **/
	      n++;
         }
      }

   return Vk.len();
}

/****************************************************************************
 *
 *   Function to compute a proper separating plane between a pair of
 *   polytopes.	 The plane will be a support plane for polytope 1.
 *
 *   On Entry:
 *	couple - couple structure for a pair of polytopes.
 *
 *   On Exit:
 *	couple - containing new proper separating plane, if one was
 *		 found.
 *
 *   Function Return :
 *	result of whether a separating plane exists, or not.
 *
 ****************************************************************************/

Bool Couple::Collision()
{
   // determine tolerance value to use:
   fFuzz = 0.00005 * (ppolyhedron1->GetRadius() + ppolyhedron2->GetRadius());
   if( fFuzz < 0.0000001 )
      fFuzz = 0.0000001;

   /** solve eq. (1) for two polytopes **/
   Bool     planeExists = FALSE;

   if( !EQZ( dist3d() ) )
      {	       /** Does a separating plane exist **/
      planeExists = TRUE;
      pln_pnt1.set();
      pln_pnt2.set();
      for (int i = 0; i < n; i++)
         {
         Point3D temp = ppolyhedron1->GetVert(vert_indx[i][0]);
         temp *= lambda[i];
         pln_pnt1 += temp;

         temp = ppolyhedron2->GetVert(vert_indx[i][1]);
         temp *= lambda[i];
         pln_pnt2 += temp;
         }
      }

   return !planeExists;
}



