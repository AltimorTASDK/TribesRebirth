#include "stdafx.h"
#include "detail.h"

Part::Part()
{
   for( int i = 0; i < MAX_CELLS; i++ )
      faParts[i] = 0;
   Reset();
}

Part::~Part()
{
   Reset();
}

void Part::Reset()
{
   fnParts = 0;
   fnCells = 0;
   fSeq = -1;
   for( int i = 0; i < MAX_CELLS; i++ )
      {
      delete faParts[i];
      faParts[i] = 0;
      faCells[i] = -1;
      faPartLabels[i] = -1;
      }
   fPolyhedron.Reset();
}

void Part::AddCell( Mesh *pMesh )
{
   if( !fnParts )
      {
      strcpy( fName, pMesh->fName );
      fSeq = pMesh->fSeq;
      }
   else
      {
      if( fSeq != pMesh->fSeq )
         {
         char s[200];
         sprintf( s, "Part uses two sequences ('%c' & '%c') : %s",
            fSeq, pMesh->fSeq, fName );
         throw GeneralError( s );
         }
      }
   if( pMesh->fPart == -1 )
      {
      if( faPartLabels[pMesh->fCell] != -1 )
         {
         char s[200];
         sprintf( s, "Duplicate cell '%c' for mesh %s", pMesh->fCell + 'a',
            pMesh->fName );
         throw GeneralError( s );
         }
      faCells[pMesh->fCell] = pMesh->fCell;
      faPartLabels[pMesh->fCell] = fnParts;
      faParts[fnParts++] = pMesh;
      }
   else
      {
      faCells[pMesh->fCell] = pMesh->fPart;
      delete pMesh;
      }
   fnCells++;
   fPolyhedron.AddVerts( &pMesh->fPolyhedron );
}

void Part::ValidateCells()
{
   // make sure there are no missing cells:
   for( int i = 0; i < fnCells; i++ )
      {
      if( faCells[i] == -1 )
         {
         char s[200];
         sprintf( s, "Missing cell '%c' for mesh %s", i + 'a', fName );
         throw GeneralError( s );
         }
      if( faCells[i] >= fnCells )
         {
         char s[200];
         sprintf( s, "Undefined reference to cell '%c' for mesh %s", 
            faCells[i] + 'a', fName );
         throw GeneralError( s );
         }
      faCells[i] = faPartLabels[faCells[i]];
      if( faCells[i] < 0 || faCells[i] >= fnParts )
         throw GeneralError( "Bad human!" );
      }
}

TSPartBase* Part::MakeTSCellAnimPart()
{
   // if just one cell, just return it:
   if( fnParts == 1 )
      return faParts[0]->MakeTSMesh();

   // otherwise, make a cell anim part:
   TSCellAnimPart * pCellPart = new TSCellAnimPart();
   pCellPart->setAnimSequence( fSeq );

	SphereF bs;

   for( int i = 0; i < fnParts; i++ )
		{
      pCellPart->addPart( faParts[i]->MakeTSMesh() );
      if( !i )
         {
			bs.center = faParts[i]->fCenter;
			bs.radius = faParts[i]->fRadius;
         }
      else
			{
			SphereF sp( faParts[i]->fCenter, faParts[i]->fRadius ); 
			bs.adjustToEnclose( sp );
			}
		}

	pCellPart->setCenter( bs.center );
	pCellPart->setRadius( bs.radius );
   pCellPart->setCells( fnCells, faCells );

   return pCellPart;
}

//---------------------------------------------------------------------------

void Part::GetSequenceLengths( Int32 * pLengths )
{
   if( fnCells > 1 && pLengths[fSeq] < fnCells )
         pLengths[fSeq] = fnCells;
}

//---------------------------------------------------------------------------
