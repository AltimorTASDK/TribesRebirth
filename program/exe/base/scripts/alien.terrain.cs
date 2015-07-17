//---------------------------------------------------------
// alien terrain file
//---------------------------------------------------------

// the terrain types for this world
function Terrain::Alien::setTypes()
{
   LS::flushTextures();
   
   LS::addTerrainType(C, " 1 mottled sands");
   LS::addTerrainType(R, " 2 light sand");
   LS::addTerrainType(D, " 3 cracked earth");
   LS::addTerrainType(G, " 4 moss");
   LS::addTerrainType(P, " 5 path in moss");
   LS::addTerrainType(S, " 6 path in sand");
}

// default rules for this world type
function Terrain::Alien::setRules()
{
   Terrain::Alien::setTypes();
   LS::flushRules();
   
   LS::addRule(C, 50.0, 550.0, 150.0, 0.50, 0.30, 0, 0.00, 8.0, 1.5, 0.50, 0.70, 0);
   LS::addRule(R, 00.0, 550.0, 250.0, 0.50, 0.50, 0, 0.00, 1.0, 0.2, 0.50, 0.50, 0);
   LS::addRule(D, 00.0, 550.0, 25.0,  0.70, 0.50, 0, 0.00, 1.0, 0.1, 0.50, 0.30, 0); 
   LS::addRule(G, 00.0, 550.0, 25.0,  0.10, 0.50, 0, 0.00, 1.0, 0.1, 0.50, 0.90, 0);
}

// create the grid file and dml for this world
function Terrain::Alien::createGridFile()
{
   Terrain::Alien::setTypes();
   
   LS::addTerrainTexture("ACCCC1.BMP", CCCC, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("ACCCC2.BMP", CCCC, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("ACCCC3.BMP", CCCC, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("ACCCC4.BMP", CCCC, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("ACCCC5.BMP", CCCC, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("accdd.bmp", CCDD, 0xFF, 0, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("accgg.bmp", CCGG, 0xFF, 0, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("accrr.bmp", CCRR, 0xFF, 0, Sand, 0.5, 1.0);
   LS::addTerrainTexture("acddd.bmp", CDDD, 0xFF, 0, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("acdgg.bmp", CDGG, 0xFF, 0, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("acdrr.bmp", CDRR, 0xFF, 0, Sand, 0.5, 1.0);
   LS::addTerrainTexture("acgdd.bmp", CGDD, 0xFF, 0, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("acggg.bmp", CGGG, 0xFF, 0, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("acgrr.bmp", CGRR, 0xFF, 0, Sand, 0.5, 1.0);
   LS::addTerrainTexture("acrdd.bmp", CRDD, 0xFF, 0, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("acrgg.bmp", CRGG, 0xFF, 0, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("acrrr.bmp", CRRR, 0xFF, 0, Sand, 0.5, 1.0);
   LS::addTerrainTexture("adccc.bmp", DCCC, 0xFF, 0, Sand, 0.5, 1.0);
   LS::addTerrainTexture("adddd.BMP", DDDD, 0xFF, 20, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("adddd1.BMP", DDDD, 0xFF, 20, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("adddd2.BMP", DDDD, 0xFF, 20, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("adddd3.BMP", DDDD, 0xFF, 20, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("adddd4.BMP", DDDD, 0xFF, 20, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("addgg.bmp", DDGG, 0xFF, 0, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("addrr.bmp", DDRR, 0xFF, 0, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("adgcc.bmp", DGCC, 0xFF, 0, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("adggg.bmp", DGGG, 0xFF, 0, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("adgrr.bmp", DGRR, 0xFF, 0, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("adrcc.bmp", DRCC, 0xFF, 0, Sand, 0.5, 1.0);
   LS::addTerrainTexture("adrgg.bmp", DRGG, 0xFF, 0, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("adrrr.bmp", DRRR, 0xFF, 0, Sand, 0.5, 1.0);
   LS::addTerrainTexture("agccc.bmp", GCCC, 0xFF, 0, Sand, 0.5, 1.0);
   LS::addTerrainTexture("agddd.bmp", GDDD, 0xFF, 0, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("agggg.BMP", GGGG, 0xFF, 16, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("agggg1.BMP", GGGG, 0xFF, 16, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("agggg2.BMP", GGGG, 0xFF, 17, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("agggg3.BMP", GGGG, 0xFF, 17, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("agggg4.BMP", GGGG, 0xFF, 17, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("agggg5.BMP", GGGG, 0xFF, 17, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("agrrr.bmp", GRRR, 0xFF, 0, Sand, 0.5, 1.0);
   LS::addTerrainTexture("arccc.bmp", RCCC, 0xFF, 0, Sand, 0.5, 1.0);
   LS::addTerrainTexture("arddd.bmp", RDDD, 0xFF, 0, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("argcc.bmp", RGCC, 0xFF, 0, Sand, 0.5, 1.0);
   LS::addTerrainTexture("argdd.bmp", RGDD, 0xFF, 0, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("arggg.bmp", RGGG, 0xFF, 0, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("arrgg.bmp", RRGG, 0xFF, 0, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("arrrr.bmp", RRRR, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("arrrr1.bmp", RRRR, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("arrrr2.bmp", RRRR, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("arrrr3.bmp", RRRR, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("arrrr4.bmp", RRRR, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("APath2a.bmp", GGPP, 0xFF, 12, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("APath2b.bmp", GGPP, 0xFF, 12, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("APath2c.bmp", GGPP, 0xFF, 12, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("APath2D.bmp", GGPP, 0xFF, 12, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("APath2E.bmp", GGPP, 0xFF, 12, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("APath2F.bmp", GGPP, 0xFF, 13, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("APath2G.bmp", GGPP, 0xFF, 13, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("APath2H.bmp", GGPP, 0xFF, 14, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("APath1A.bmp", GGGP, 0xFF, 33, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("APath1B.bmp", GGGP, 0xFF, 33, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("APath1C.bmp", GGGP, 0xFF, 34, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("APath3A.bmp", GGGP, 0, 0, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("APath3B.bmp", GGGP, 0, 0, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("APath3C.bmp", GGGP, 0, 0, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("APath4A.bmp", PPGP, 0xFF, 25, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("APath4B.bmp", PPGP, 0xFF, 25, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("APath4C.bmp", PPGP, 0xFF, 25, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("APath4D.bmp", PPGP, 0xFF, 25, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("APath5A.bmp", PPPP, 0xFF, 33, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("APath5B.bmp", PPPP, 0xFF, 33, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("APath5C.bmp", PPPP, 0xFF, 34, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("APathCURVE1A.bmp", GPGP, 0xFF, 25, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("APathCURVE1B.bmp", GPGP, 0xFF, 25, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("APathCURVE1C.bmp", GPGP, 0xFF, 25, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("APathCURVE1D.bmp", GPGP, 0xFF, 25, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("AsPath2a.bmp", RRSS, 0xFF, 16, Sand, 0.5, 1.0);
   LS::addTerrainTexture("AsPath2b.bmp", RRSS, 0xFF, 16, Sand, 0.5, 1.0);
   LS::addTerrainTexture("AsPath2c.bmp", RRSS, 0xFF, 16, Sand, 0.5, 1.0);
   LS::addTerrainTexture("AsPath2D.bmp", RRSS, 0xFF, 16, Sand, 0.5, 1.0);
   LS::addTerrainTexture("AsPath2E.bmp", RRSS, 0xFF, 18, Sand, 0.5, 1.0);
   LS::addTerrainTexture("AsPath2F.bmp", RRSS, 0xFF, 18, Sand, 0.5, 1.0);
   LS::addTerrainTexture("AsPath1A.bmp", RRRS, 0xFF, 50, Sand, 0.5, 1.0);
   LS::addTerrainTexture("AsPath1B.bmp", RRRS, 0xFF, 50, Sand, 0.5, 1.0);
   LS::addTerrainTexture("AsPath3A.bmp", RRRS, 0, 0, Sand, 0.5, 1.0);
   LS::addTerrainTexture("AsPath3B.bmp", RRRS, 0, 0, Sand, 0.5, 1.0);
   LS::addTerrainTexture("AsPath3C.bmp", RRRS, 0, 0, Sand, 0.5, 1.0);
   LS::addTerrainTexture("AsPath4A.bmp", SSRS, 0xFF, 33, Sand, 0.5, 1.0);
   LS::addTerrainTexture("AsPath4B.bmp", SSRS, 0xFF, 33, Sand, 0.5, 1.0);
   LS::addTerrainTexture("AsPath4C.bmp", SSRS, 0xFF, 34, Sand, 0.5, 1.0);
   LS::addTerrainTexture("AsPath5A.bmp", SSSS, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("AsPath5B.bmp", SSSS, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("AsPath5C.bmp", SSSS, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("AsPath5D.bmp", SSSS, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("AsPath5E.bmp", SSSS, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("AsPathCURVE1A.bmp", RSRS, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("AsPathCURVE1B.bmp", RSRS, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("AsPathCURVE1C.bmp", RSRS, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("AsPathCURVE1D.bmp", RSRS, 0xFF, 20, Sand, 0.5, 1.0);
   LS::addTerrainTexture("AsPathCURVE1e.bmp", RSRS, 0xFF, 20, Sand, 0.5, 1.0);

   // create the dat and dml for this world
   LS::createGridFile("temp\\alien.grid.dat", "temp\\alien.dml");
}