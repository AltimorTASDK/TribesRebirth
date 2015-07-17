//---------------------------------------------------------
// mars terrain file
//---------------------------------------------------------

// the terrain types for this world
function Terrain::Mars::setTypes()
{
   LS::flushTextures();
   
   LS::addTerrainType(G, " 1 Rough sand");
   LS::addTerrainType(S, " 2 Smooth sand");
   LS::addTerrainType(C, " 3 Rough sand and rock");
   LS::addTerrainType(R, " 4 Rock");
   LS::addTerrainType(D, " 5 Rough and smooth sand");
   LS::addTerrainType(L, " 6 Lined clay");
}

// default rules for this world type
function Terrain::Mars::setRules()
{
   Terrain::Mars::setTypes();
   LS::flushRules();

   LS::addRule(D, 0.0, 350.0, 150.0, 0.50, 0.50, 0, 0.00, 8.0, 1.5, 0.50, 0.50, 0);
   LS::addRule(C, 0.0, 350.0, 150.0, 0.50, 0.50, 0, 0.00, 8.0, 1.5, 0.50, 0.50, 0);
   LS::addRule(G, 0.0, 305.0, 25.0,  0.50, 0.50, 0, 0.10, 4.0, 0.5, 0.50, 0.50, 0);
   LS::addRule(S, 0.0, 450.0, 100.0, 0.50, 0.50, 0, 0.00, 2.0, 0.3, 0.50, 0.50, 0);
   LS::addRule(R, 0.0, 400.0, 150.0, 0.50, 0.40, 0, 0.00, 8.0, 0.5, 0.50, 0.30, 0);
   LS::addRule(L, 0.0, 185.0, 05.0,  0.10, 0.05, 0, 0.00, 1.0, 0.1, 0.60, 0.40, 0);
}

// create the grid file and dml for this world
function Terrain::Mars::createGridFile()
{
   Terrain::Mars::setTypes();

   LS::addTerrainTexture("xCCCC.BMP", CCCC, 0xFF, 20, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("xCCCC1.BMP", CCCC, 0xFF, 20, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("xCCCC2.BMP", CCCC, 0xFF, 20, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("xCCCC3.BMP", CCCC, 0xFF, 20, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("xCCCC4.BMP", CCCC, 0xFF, 20, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("xDDDD.BMP", DDDD, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xDDDD1.BMP", DDDD, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xDDDD2.BMP", DDDD, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xDDDD3.BMP", DDDD, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xDDDD4.BMP", DDDD, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xSSSS.BMP", SSSS, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xSSSS1.BMP", SSSS, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xSSSS2.BMP", SSSS, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xSSSS3.BMP", SSSS, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xSSSS4.BMP", SSSS, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xCCDD.BMP", CCDD, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xCCSS.BMP", CCSS, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xCDDD.BMP", CDDD, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xCDSS.BMP", CDSS, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xCSDD.BMP", CSDD, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xCSSS.BMP", CSSS, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xDCCC.BMP", DCCC, 0xFF, 20, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("xDDSS.BMP", DDSS, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xDSCC.BMP", DSCC, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xDSSS.BMP", DSSS, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xSCCC.BMP", SCCC, 0xFF, 20, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("xSDDD.BMP", SDDD, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xLLLL.BMP", LLLL, 0xFF, 20, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("xLLLL1.BMP", LLLL, 0xFF, 20, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("xLLLL2.BMP", LLLL, 0xFF, 20, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("xLLLL3.BMP", LLLL, 0xFF, 20, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("xLLLL4.BMP", LLLL, 0xFF, 20, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("xLLLL5.BMP", LLLL, 0xFF, 20, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("xGGGG.BMP", GGGG, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xGGGG1.BMP", GGGG, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xGGGG2.BMP", GGGG, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xGGGG3.BMP", GGGG, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xGGGG4.BMP", GGGG, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xRRRR.BMP", RRRR, 0xFF, 20, Stone, 0.5, 1.0);
   LS::addTerrainTexture("xRRRR1.BMP", RRRR, 0xFF, 20, Stone, 0.5, 1.0);
   LS::addTerrainTexture("xRRRR2.BMP", RRRR, 0xFF, 20, Stone, 0.5, 1.0);
   LS::addTerrainTexture("xRRRR3.BMP", RRRR, 0xFF, 20, Stone, 0.5, 1.0);
   LS::addTerrainTexture("xRRRR4.BMP", RRRR, 0xFF, 20, Stone, 0.5, 1.0);
   LS::addTerrainTexture("xCCGG.BMP", CCGG, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xCCLL.BMP", CCLL, 0xFF, 20, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("xCCRR.BMP", CCRR, 0xFF, 20, Stone, 0.5, 1.0);
   LS::addTerrainTexture("xCDGG.BMP", CDGG, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xCDLL.BMP", CDLL, 0xFF, 20, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("xCDRR.BMP", CDRR, 0xFF, 20, Stone, 0.5, 1.0);
   LS::addTerrainTexture("xCGDD.BMP", CGDD, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xCGGG.BMP", CGGG, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xCGLL.BMP", CGLL, 0xFF, 20, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("xCGRR.BMP", CGRR, 0xFF, 20, Stone, 0.5, 1.0);
   LS::addTerrainTexture("xCGSS.BMP", CGSS, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xCLGG.BMP", CLGG, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xCLLL.BMP", CLLL, 0xFF, 20, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("xCLRR.BMP", CLRR, 0xFF, 20, Stone, 0.5, 1.0);
   LS::addTerrainTexture("xCLSS.BMP", CLSS, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xCLGG.BMP", CLGG, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xCRDD.BMP", CRDD, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xCRGG.BMP", CRGG, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xCRLL.BMP", CRLL, 0xFF, 20, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("xCRRR.BMP", CRRR, 0xFF, 20, Stone, 0.5, 1.0);
   LS::addTerrainTexture("xCRSS.BMP", CRSS, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xCSGG.BMP", CSGG, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xCSLL.BMP", CSLL, 0xFF, 20, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("xCSRR.BMP", CSRR, 0xFF, 20, Stone, 0.5, 1.0);
   LS::addTerrainTexture("xDDGG.BMP", DDGG, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xDDLL.BMP", DDLL, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xDDRR.BMP", DDRR, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xDGCC.BMP", DGCC, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xDGGG.BMP", DGGG, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xDGLL.BMP", DGLL, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xDGRR.BMP", DGRR, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xDGSS.BMP", DGSS, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xDLCC.BMP", DLCC, 0xFF, 20, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("xDLGG.BMP", DLGG, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xDLLL.BMP", DLLL, 0xFF, 20, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("xDLRR.BMP", DLRR, 0xFF, 20, Stone, 0.5, 1.0);
   LS::addTerrainTexture("xDLSS.BMP", DLSS, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xDRCC.BMP", DRCC, 0xFF, 20, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("xDRGG.BMP", DRGG, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xDRLL.BMP", DRLL, 0xFF, 20, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("xDRRR.BMP", DRRR, 0xFF, 20, Stone, 0.5, 1.0);
   LS::addTerrainTexture("xDRSS.BMP", DRSS, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xDSGG.BMP", DSGG, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xDSLL.BMP", DSLL, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xDSRR.BMP", DSRR, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xGCCC.BMP", GCCC, 0xFF, 20, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("xGDDD.BMP", GDDD, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xGGLL.BMP", GGLL, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xGGRR.BMP", GGRR, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xGGSS.BMP", GGSS, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xGLCC.BMP", GLCC, 0xFF, 20, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("xGLDD.BMP", GLDD, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xGLLL.BMP", GLLL, 0xFF, 20, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("xGLRR.BMP", GLRR, 0xFF, 20, Stone, 0.5, 1.0);
   LS::addTerrainTexture("xGLSS.BMP", GLSS, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xGRCC.BMP", GRCC, 0xFF, 20, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("xGRDD.BMP", GRDD, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xGRLL.BMP", GRLL, 0xFF, 20, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("xGRRR.BMP", GRRR, 0xFF, 20, Stone, 0.5, 1.0);
   LS::addTerrainTexture("xGRSS.BMP", GRSS, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xGSCC.BMP", GSCC, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xGSDD.BMP", GSDD, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xGSLL.BMP", GSLL, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xGSRR.BMP", GSRR, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xGSSS.BMP", GSSS, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xLCCC.BMP", LCCC, 0xFF, 20, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("xLDDD.BMP", LDDD, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xLGGG.BMP", LGGG, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xLLRR.BMP", LLRR, 0xFF, 20, Stone, 0.5, 1.0);
   LS::addTerrainTexture("xLLSS.BMP", LLSS, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xLRCC.BMP", LRCC, 0xFF, 20, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("xLRDD.BMP", LRDD, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xLRGG.BMP", LRGG, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xLRRR.BMP", LRRR, 0xFF, 20, Stone, 0.5, 1.0);
   LS::addTerrainTexture("xLRSS.BMP", LRSS, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xLSCC.BMP", LSCC, 0xFF, 20, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("xLSDD.BMP", LSDD, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xLSGG.BMP", LSGG, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xLSRR.BMP", LSRR, 0xFF, 20, Stone, 0.5, 1.0);
   LS::addTerrainTexture("xLSSS.BMP", LSSS, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xRCCC.BMP", RCCC, 0xFF, 20, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("xRDDD.BMP", RDDD, 0xFF, 20, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("xRGGG.BMP", RGGG, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xRLLL.BMP", RLLL, 0xFF, 20, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("xRRSS.BMP", RRSS, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xRSCC.BMP", RSCC, 0xFF, 20, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("xRSDD.BMP", RSDD, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xRSGG.BMP", RSGG, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xRSLL.BMP", RSLL, 0xFF, 20, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("xRSSS.BMP", RSSS, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xSGGG.BMP", SGGG, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("xSLLL.BMP", SLLL, 0xFF, 20, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("xSRRR.BMP", SRRR, 0xFF, 20, Stone, 0.5, 1.0);

   // create the dat and dml for this world
   LS::createGridFile("temp\\mars.grid.dat", "temp\\mars.dml");
}
