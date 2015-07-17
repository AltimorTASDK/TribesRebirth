//---------------------------------------------------------
// lush terrain file
//---------------------------------------------------------

// the terrain types for this world
function Terrain::Lush::setTypes()
{
   LS::flushTextures();
   
   LS::addTerrainType(F, " 1 Dirt");
   LS::addTerrainType(N, " 2 Dirt medium grass");
   LS::addTerrainType(C, " 3 Dirt much grass");
   LS::addTerrainType(R, " 4 Rock");
   LS::addTerrainType(H, " 5 Cracked Earth");
   LS::addTerrainType(P, " 6 Path");
   LS::addTerrainType(G, " 7 Concrete pad");
   LS::addTerrainType(X, " 8 Concrete road pad");
   LS::addTerrainType(S, " 9 Half rock - half sand");
}

// default rules for this world type
function Terrain::Lush::setRules()
{
   Terrain::Lush::setTypes();
   LS::flushRules();

   LS::addRule(F, 50.0,  350.0, 150.0, 0.50, 0.50, 0, 0.00, 8.0, 1.5, 0.50, 0.50, 0);
   LS::addRule(N, 15.0,  305.0, 25.0,  0.50, 0.50, 0, 0.10, 4.0, 0.5, 0.50, 0.50, 0);
   LS::addRule(C, 0.0,   450.0, 100.0, 0.50, 0.50, 0, 0.00, 2.0, 0.3, 0.50, 0.50, 0);
   LS::addRule(R, 150.0, 400.0, 150.0, 0.50, 0.40, 0, 0.00, 8.0, 0.5, 0.50, 0.30, 0);
   LS::addRule(H, 0.0,   185.0, 05.0,  0.10, 0.05, 0, 0.00, 1.0, 0.1, 0.60, 0.40, 0);
}

// create the grid file and dml for this world
function Terrain::Lush::createGridFile()
{
   Terrain::Lush::setTypes();

   LS::addTerrainTexture("lCCCC.BMP", CCCC, 0xFF, 20, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("lCCCC1.BMP", CCCC, 0xFF, 20, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("lCCCC2.BMP", CCCC, 0xFF, 20, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("lCCCC3.BMP", CCCC, 0xFF, 20, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("lCCCC4.BMP", CCCC, 0xFF, 20, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("lCCFF.BMP", CCFF, 0xFF, 0, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("lCCNN.BMP", CCNN, 0xFF, 0, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("lCCRR.BMP", CCRR, 0xFF, 20, Stone, 0.5, 1.0);
   LS::addTerrainTexture("lCFFF.BMP", CFFF, 0xFF, 0, Sand, 0.5, 1.0);
   LS::addTerrainTexture("lCFHH.BMP", CFHH, 0xFF, 0, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("lCFNN.BMP", CFNN, 0xFF, 0, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("lCFRR.BMP", CFRR, 0xFF, 0, Stone, 0.5, 1.0);
   LS::addTerrainTexture("lCHHH.BMP", CHHH, 0xFF, 0, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("lCNFF.BMP", CNFF, 0xFF, 0, Sand, 0.5, 1.0);
   LS::addTerrainTexture("lCNHH.BMP", CNHH, 0xFF, 0, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("lCNNN.BMP", CNNN, 0xFF, 0, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("lCNRR.BMP", CNRR, 0xFF, 0, Stone, 0.5, 1.0);
   LS::addTerrainTexture("lCRFF.BMP", CRFF, 0xFF, 0, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("lCRNN.BMP", CRNN, 0xFF, 0, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("lCRRR.BMP", CRRR, 0xFF, 20, Stone, 0.5, 1.0);
   LS::addTerrainTexture("lFCCC.BMP", FCCC, 0xFF, 0, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("lFFFF.BMP", FFFF, 0xFF, 33, Sand, 0.5, 1.0);
   LS::addTerrainTexture("lFFFF2.BMP", FFFF, 0xFF, 33, Sand, 0.5, 1.0);
   LS::addTerrainTexture("lFFFF3.BMP", FFFF, 0xFF, 34, Sand, 0.5, 1.0);
   LS::addTerrainTexture("lFFRR.BMP", FFRR, 0xFF, 0, Stone, 0.5, 1.0);
   LS::addTerrainTexture("lFHHH.BMP", FHHH, 0xFF, 0, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("lFNCC.BMP", FNCC, 0xFF, 0, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("lFNHH.BMP", FNHH, 0xFF, 0, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("lFNNN.BMP", FNNN, 0xFF, 0, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("lFNRR.BMP", FNRR, 0xFF, 0, Stone, 0.5, 1.0);
   LS::addTerrainTexture("lFRNN.BMP", FRNN, 0xFF, 0, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("lFRRR.BMP", FRRR, 0xFF, 0, Stone, 0.5, 1.0);
   LS::addTerrainTexture("lHCCC.BMP", HCCC, 0xFF, 0, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("lHCFF.BMP", HCFF, 0xFF, 0, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("lHCNN.BMP", HCNN, 0xFF, 0, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("lHFCC.BMP", HFCC, 0xFF, 0, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("lHFFF.BMP", HFFF, 0xFF, 0, Sand, 0.5, 1.0);
   LS::addTerrainTexture("lHFNN.BMP", HFNN, 0xFF, 0, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("lHHCC.BMP", HHCC, 0xFF, 0, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("lHHFF.BMP", HHFF, 0xFF, 0, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("lHHHH.BMP", HHHH, 0xFF, 20, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("lHHHH1.BMP", HHHH, 0xFF, 20, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("lHHHH2.BMP", HHHH, 0xFF, 20, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("lHHHH3.BMP", HHHH, 0xFF, 20, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("lHHHH4.BMP", HHHH, 0xFF, 20, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("lHHNN.BMP", HHNN, 0xFF, 0, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("lHNCC.BMP", HNCC, 0xFF, 0, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("lHNFF.BMP", HNFF, 0xFF, 0, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("lHNNN.BMP", HNNN, 0xFF, 0, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("lNCCC.BMP", NCCC, 0xFF, 0, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("lNFFF.BMP", NFFF, 0xFF, 0, Sand, 0.5, 1.0);
   LS::addTerrainTexture("lNHHH.BMP", NHHH, 0xFF, 0, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("lNNFF.BMP", NNFF, 0xFF, 0, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("lNNNN.BMP", NNNN, 0xFF, 20, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("lNNNN1.BMP", NNNN, 0xFF, 20, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("lNNNN2.BMP", NNNN, 0xFF, 20, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("lNNNN3.BMP", NNNN, 0xFF, 20, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("lNNNN4.BMP", NNNN, 0xFF, 20, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("lNNRR.BMP", NNRR, 0xFF, 0, Stone, 0.5, 1.0);
   LS::addTerrainTexture("lNRRR.BMP", NRRR, 0xFF, 0, Stone, 0.5, 1.0);
   LS::addTerrainTexture("lRCCC.BMP", RCCC, 0xFF, 0, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("lRFCC.BMP", RFCC, 0xFF, 0, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("lRFFF.BMP", RFFF, 0xFF, 0, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("lRNCC.BMP", RNCC, 0xFF, 0, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("lRNFF.BMP", RNFF, 0xFF, 0, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("lRNNN.BMP", RNNN, 0xFF, 0, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("lRRRR.BMP", RRRR, 0xFF, 12, Stone, 0.5, 1.0);
   LS::addTerrainTexture("lRRRR1.BMP", RRRR, 0xFF, 12, Stone, 0.5, 1.0);
   LS::addTerrainTexture("lRRRR2.BMP", RRRR, 0xFF, 12, Stone, 0.5, 1.0);
   LS::addTerrainTexture("lRRRR3.BMP", RRRR, 0xFF, 12, Stone, 0.5, 1.0);
   LS::addTerrainTexture("lGGCC.bmp" , GGCC, 0xFF, 50, Concrete, 0.5, 1.0);
   LS::addTerrainTexture("lGGCC1.bmp", GGCC, 0xFF, 50, Concrete, 0.5, 1.0);
   LS::addTerrainTexture("lCCGG.bmp" , GGCC, 0, 0, Concrete, 0.5, 1.0);
   LS::addTerrainTexture("lCCGG1.bmp", GGCC, 0, 0, Concrete, 0.5, 1.0);
   LS::addTerrainTexture("lCGGG.bmp" , CGGG, 0xFF, 50, Concrete, 0.5, 1.0);
   LS::addTerrainTexture("lCGGG1.bmp", CGGG, 0xFF, 50, Concrete, 0.5, 1.0);
   LS::addTerrainTexture("lGCCC.bmp" , GCCC, 0xFF, 50, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("lGCCC1.bmp", GCCC, 0xFF, 50, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("lGGGG.BMP" , GGGG, 0xFF, 100, Concrete, 0.5, 1.0);
   LS::addTerrainTexture("lLOGO1.bmp", GGGG, 0xFF, 0, Concrete, 0.5, 1.0);
   LS::addTerrainTexture("lLOGO2.bmp", GGGG, 0xFF, 0, Concrete, 0.5, 1.0);
   LS::addTerrainTexture("lLOGO3.bmp", GGGG, 0xFF, 0, Concrete, 0.5, 1.0);
   LS::addTerrainTexture("lLOGO4.bmp", GGGG, 0xFF, 0, Concrete, 0.5, 1.0);
   LS::addTerrainTexture("lLOGO5.bmp", GGGG, 0xFF, 0, Concrete, 0.5, 1.0);
   LS::addTerrainTexture("lLOGO6.bmp", GGGG, 0xFF, 0, Concrete, 0.5, 1.0);
   LS::addTerrainTexture("lLOGO7.bmp", GGGG, 0xFF, 0, Concrete, 0.5, 1.0);
   LS::addTerrainTexture("lLOGO8.bmp", GGGG, 0xFF, 0, Concrete, 0.5, 1.0);
   LS::addTerrainTexture("lLOGO9.bmp", GGGG, 0xFF, 0, Concrete, 0.5, 1.0);
   LS::addTerrainTexture("lLOGO10.bmp", GGGG, 0xFF, 0, Concrete, 0.5, 1.0);
   LS::addTerrainTexture("lPath2.bmp", CCPP, 0xFF, 25, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("lPath2a.bmp", CCPP, 0xFF, 25, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("lPath2b.bmp", CCPP, 0xFF, 25, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("lPath2c.bmp", CCPP, 0xFF, 25, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("lPath1.bmp", CCCP, 0xFF, 100, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("lPath3.bmp", CCCP, 0, 0, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("lPath3a.bmp", CCCP, 0, 0, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("lPath4.bmp", PPCP, 0xFF, 50, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("lPath4a.bmp", PPCP, 0xFF, 50, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("lPath5.bmp", PPPP, 0xFF, 33, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("lPath5a.bmp", PPPP, 0xFF, 33, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("lPath5b.bmp", PPPP, 0xFF, 34, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("lPathCURVE.bmp", CPCP, 0xFF, 50, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("lPathCURVE2.bmp", CPCP, 0xFF, 50, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("lnnnn.bmp", NNNN, 0xFF, 20, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("lnnnn1.bmp", NNNN, 0xFF, 20, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("lnnnn2.bmp", NNNN, 0xFF, 20, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("lnnnn3.bmp", NNNN, 0xFF, 20, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("lnnnn4.bmp", NNNN, 0xFF, 20, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("lnnnn.bmp", NNNN, 0xFF, 20, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("lnnnn1.bmp", NNNN, 0xFF, 20, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("lnnnn2.bmp", NNNN, 0xFF, 20, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("lnnnn3.bmp", NNNN, 0xFF, 20, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("lnnnn4.BMP", NNNN, 0xFF, 20, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("lnnnn.BMP", NNNN, 0xFF, 20, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("lnnnn1.bmp", NNNN, 0xFF, 20, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("lnnnn2.bmp", NNNN, 0xFF, 20, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("lpath5.bmp", PPPP, 0xFF, 33, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("lpath5a.bmp", PPPP, 0xFF, 33, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("lpath5b.bmp", PPPP, 0xFF, 34, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("lpath5.bmp", PPPP, 0xFF, 33, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("lpath5a.BMP", PPPP, 0xFF, 33, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("lpath5b.BMP", PPPP, 0xFF, 34, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("lpath5.BMP", PPPP, 0xFF, 33, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("lpath5a.BMP", PPPP, 0xFF, 33, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("lpath5b.bmp", PPPP, 0xFF, 34, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("lpath5.bmp", PPPP, 0xFF, 33, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("lpath5a.bmp", PPPP, 0xFF, 33, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("lpath5b.BMP", PPPP, 0xFF, 34, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("lpath5.bmp", PPPP, 0xFF, 33, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("lpath5a.bmp", PPPP, 0xFF, 33, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("lpath5b.bmp", PPPP, 0xFF, 34, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("lpath5.bmp", PPPP, 0xFF, 33, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("lpath5a.bmp", PPPP, 0xFF, 33, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("lpath5b.bmp", PPPP, 0xFF, 34, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("lpath5.bmp", PPPP, 0xFF, 33, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("lpath5a.bmp", PPPP, 0xFF, 33, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("lnnnn.BMP", NNNN, 0xFF, 20, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("lnnnn1.BMP", NNNN, 0xFF, 20, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("lnnnn2.BMP", NNNN, 0xFF, 20, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("lnnnn3.BMP", NNNN, 0xFF, 20, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("lRCNN.BMP", RCNN, 0xFF, 0, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("lCHRR.BMP", CHRR, 0xFF, 0, Stone, 0.5, 1.0);
   LS::addTerrainTexture("lHRRR.BMP", HRRR, 0xFF, 0, Stone, 0.5, 1.0);
   LS::addTerrainTexture("lRCHH.BMP", RCHH, 0xFF, 0, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("lRHCC.BMP", RHCC, 0xFF, 0, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("lRHHH.BMP", RHHH, 0xFF, 0, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("lHNRR.BMP", HNRR, 0xFF, 0, Stone, 0.5, 1.0);
   LS::addTerrainTexture("lRHNN.BMP", RHNN, 0xFF, 0, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("lRNHH.BMP", RNHH, 0xFF, 0, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("lFHRR.BMP", FHRR, 0xFF, 0, Stone, 0.5, 1.0);
   LS::addTerrainTexture("lRFHH.BMP", RFHH, 0xFF, 0, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("lRHFF.BMP", RHFF, 0xFF, 0, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("lRRRR4.BMP", RRRR, 0xFF, 13, Stone, 0.5, 1.0);
   LS::addTerrainTexture("lRRRR5.BMP", RRRR, 0xFF, 13, Stone, 0.5, 1.0);
   LS::addTerrainTexture("lRRRR6.BMP", RRRR, 0xFF, 13, Stone, 0.5, 1.0);
   LS::addTerrainTexture("lRRRR7.BMP", RRRR, 0xFF, 13, Stone, 0.5, 1.0);
   LS::addTerrainTexture("lCRRR1.BMP", CRRR, 0xFF, 20, Stone, 0.5, 1.0);
   LS::addTerrainTexture("lCRRR2.BMP", CRRR, 0xFF, 20, Stone, 0.5, 1.0);
   LS::addTerrainTexture("lCRRR3.BMP", CRRR, 0xFF, 20, Stone, 0.5, 1.0);
   LS::addTerrainTexture("lCRRR4.BMP", CRRR, 0xFF, 20, Stone, 0.5, 1.0);
   LS::addTerrainTexture("lCCRR1.BMP", CCRR, 0xFF, 20, Stone, 0.5, 1.0);
   LS::addTerrainTexture("lCCRR2.BMP", CCRR, 0xFF, 20, Stone, 0.5, 1.0);
   LS::addTerrainTexture("lCCRR3.BMP", CCRR, 0xFF, 20, Stone, 0.5, 1.0);
   LS::addTerrainTexture("lCCRR4.BMP", CCRR, 0xFF, 20, Stone, 0.5, 1.0);
   LS::addTerrainTexture("lPathCURVE1.bmp", CPCP, 0xFF, 34, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("lPath4B.bmp", PPCP, 0xFF, 34, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("lRRSS.bmp", RRSS, 0xFF, 33, Stone, 0.5, 1.0);
   LS::addTerrainTexture("lRRSS1.bmp", RRSS, 0xFF, 33, Stone, 0.5, 1.0);
   LS::addTerrainTexture("lRSSS.bmp", RSSS, 0xFF, 0, Stone, 0.5, 1.0);
   LS::addTerrainTexture("lSRRR.bmp", SRRR, 0xFF, 0, Stone, 0.5, 1.0);
   LS::addTerrainTexture("lssss.BMP", SSSS, 0xFF, 25, Stone, 0.5, 1.0);
   LS::addTerrainTexture("lssss1.BMP", SSSS, 0xFF, 25, Stone, 0.5, 1.0);
   LS::addTerrainTexture("lssss2.BMP", SSSS, 0xFF, 25, Stone, 0.5, 1.0);
   LS::addTerrainTexture("lssss3.BMP", SSSS, 0xFF, 25, Stone, 0.5, 1.0);
   LS::addTerrainTexture("lHHRR.BMP", HHRR, 0xFF, 0, Stone, 0.5, 1.0);
   LS::addTerrainTexture("lffss.BMP", FFSS, 0xFF, 0, Stone, 0.5, 1.0);
   LS::addTerrainTexture("lfSss.BMP", FSSS, 0xFF, 0, Stone, 0.5, 1.0);
   LS::addTerrainTexture("lSfFF.BMP", SFFF, 0xFF, 0, Stone, 0.5, 1.0);
   LS::addTerrainTexture("lfRss.BMP", FRSS, 0xFF, 0, Stone, 0.5, 1.0);
   LS::addTerrainTexture("lfSRR.BMP", FSRR, 0xFF, 0, Stone, 0.5, 1.0);
   LS::addTerrainTexture("lRSFF.BMP", RSFF, 0xFF, 0, Stone, 0.5, 1.0);
   LS::addTerrainTexture("lsrrr1.BMP", SRRR, 0xFF, 50, Stone, 0.5, 1.0);
   LS::addTerrainTexture("lRSSS.BMP", RSSS, 0xFF, 50, Stone, 0.5, 1.0);
   LS::addTerrainTexture("lRRSS2.BMP", RRSS, 0xFF, 34, Stone, 0.5, 1.0);

   // create the dat and dml for this world
   LS::createGridFile("temp\\lush.grid.dat", "temp\\lush.dml");
}
