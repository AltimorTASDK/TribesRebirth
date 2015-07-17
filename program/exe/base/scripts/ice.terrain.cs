//---------------------------------------------------------
// ice terrain file
//---------------------------------------------------------

// the terrain types for this world
function Terrain::Ice::setTypes()
{
   LS::flushTextures();
   
   LS::addTerrainType(F, " 1 Snow type 1");
   LS::addTerrainType(D, " 2 Snow type 2");
   LS::addTerrainType(I, " 3 Cracked ice");
   LS::addTerrainType(Y, " 4 Rock");
   LS::addTerrainType(X, " 5 Snow and rock");
   LS::addTerrainType(P, " 6 Walking path");
}

// default rules for this world type
function Terrain::Ice::setRules()
{
   Terrain::Ice::setTypes();
   LS::flushRules();
   
   LS::addRule(I, 15.0, 200.0, 25.0,  0.50, 0.05, 0, 0.10, 0.5, 0.1, 0.50, 0.05, 0);
   LS::addRule(X, 60.0, 350.0, 150.0, 0.50, 0.50, 0, 0.00, 8.0, 0.9, 0.5,  0.50, 0);
   LS::addRule(X, 90.0, 500.0, 90.0,  0.50, 0.5,  0, 0.10, 0.5, 0.1, 0.50, 0.5,  0);
   LS::addRule(Y, 00.0, 350.0, 100.0, 0.50, 0.40, 0, 0.00, 8.0, 1.1, 0.5,  0.40, 0);
   LS::addRule(D, 00.0, 385.0, 200.0, 0.50, 0.50, 0, 0.00, 8.0, 0.7, 0.50, 0.5,  0);
   LS::addRule(F,  0.0, 40.0,  5.0,   0.50, 0.50, 0, 0.0,  4.0, 0.1, 0.50, 0.50, 0);
}

// create the grid file and dml for this world
function Terrain::Ice::createGridFile()
{
   Terrain::Ice::setTypes();

   LS::addTerrainTexture("idddd2.BMP", DDDD, 0xFF, 34, Snow, 0.5, 1.0);
   LS::addTerrainTexture("idddd1.BMP", DDDD, 0xFF, 33, Snow, 0.5, 1.0);
   LS::addTerrainTexture("idddd.BMP", DDDD, 0xFF, 33, Snow, 0.5, 1.0); 
   LS::addTerrainTexture("iffff4.BMP", FFFF, 0xFF, 25, Snow, 0.5, 1.0);
   LS::addTerrainTexture("iffff3.BMP", FFFF, 0xFF, 25, Snow, 0.5, 1.0);
   LS::addTerrainTexture("iffff2.BMP", FFFF, 0xFF, 25, Snow, 0.5, 1.0);
   LS::addTerrainTexture("iffff.BMP", FFFF, 0xFF, 25, Snow, 0.5, 1.0); 
   LS::addTerrainTexture("iddff.bmp", DDFF,  0xFF, 0, Snow, 0.5, 1.0); 
   LS::addTerrainTexture("idfff.bmp", DFFF,  0xFF, 0, Snow, 0.5, 1.0); 
   LS::addTerrainTexture("ifddd.bmp", FDDD,  0xFF, 0, Snow, 0.5, 1.0); 
   LS::addTerrainTexture("iyyyy7.bmp", YYYY, 0xFF, 3, Stone, 0.5, 1.0);
   LS::addTerrainTexture("iyyyy6.bmp", YYYY, 0xFF, 3, Stone, 0.5, 1.0);
   LS::addTerrainTexture("iyyyy5.bmp", YYYY, 0xFF, 3, Stone, 0.5, 1.0);
   LS::addTerrainTexture("iyyyy4.bmp", YYYY, 0xFF, 3, Stone, 0.5, 1.0);
   LS::addTerrainTexture("iyyyy3.bmp", YYYY, 0xFF, 22, Stone, 0.5, 1.0);
   LS::addTerrainTexture("iyyyy2.bmp", YYYY, 0xFF, 22, Stone, 0.5, 1.0);
   LS::addTerrainTexture("iyyyy1.bmp", YYYY, 0xFF, 22, Stone, 0.5, 1.0);
   LS::addTerrainTexture("iyyyy.bmp", YYYY, 0xFF, 22, Stone, 0.5, 1.0 ); 
   LS::addTerrainTexture("ixxxx3.bmp", XXXX, 0xFF, 25, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("ixxxx2.bmp", XXXX, 0xFF, 25, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("ixxxx1.bmp", XXXX, 0xFF, 25, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("ixxxx.bmp", XXXX, 0xFF, 25, PackedEarth, 0.5, 1.0); 
   LS::addTerrainTexture("ipath2.BMP", FFPP, 0xFF, 25, Snow, 0.5, 1.0);
   LS::addTerrainTexture("ipath2a.bmp", FFPP, 0xFF, 25, Snow, 0.5, 1.0);
   LS::addTerrainTexture("ipath2b.bmp", FFPP, 0xFF, 25, Snow, 0.5, 1.0);
   LS::addTerrainTexture("ipath2c.bmp", FFPP, 0xFF, 25, Snow, 0.5, 1.0);
   LS::addTerrainTexture("ipath1.bmp", FFFP, 0xFF, 50, Snow, 0.5, 1.0);
   LS::addTerrainTexture("ipath1a.bmp", FFFP, 0xFF, 50, Snow, 0.5, 1.0);
   LS::addTerrainTexture("ipath3.BMP", FFFP, 0, 0, Snow, 0.5, 1.0);
   LS::addTerrainTexture("ipath3a.bmp", FFFP, 0, 0, Snow, 0.5, 1.0);
   LS::addTerrainTexture("ipath4.bmp", PPFP, 0xFF, 25, Snow, 0.5, 1.0);
   LS::addTerrainTexture("ipath4a.bmp", PPFP, 0xFF, 25, Snow, 0.5, 1.0);
   LS::addTerrainTexture("ipath4b.bmp", PPFP, 0xFF, 25, Snow, 0.5, 1.0);
   LS::addTerrainTexture("ipath4c.bmp", PPFP, 0xFF, 25, Snow, 0.5, 1.0);
   LS::addTerrainTexture("ipathcurve1.BMP", FPFP, 0xFF, 33, Snow, 0.5, 1.0);
   LS::addTerrainTexture("ipathcurve1a.BMP", FPFP, 0xFF, 33, Snow, 0.5, 1.0);
   LS::addTerrainTexture("ipathcurve1b.BMP", FPFP, 0xFF, 34, Snow, 0.5, 1.0);
   LS::addTerrainTexture("idfxx.bmp", DFXX, 0xFF, 0, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("idxxx.bmp", DXXX, 0xFF, 0, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("ifxxx.bmp", FXXX, 0xFF, 0, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("ixddd.bmp", XDDD, 0xFF, 0, Snow, 0.5, 1.0);
   LS::addTerrainTexture("ixdff.bmp", XDFF, 0xFF, 0, Snow, 0.5, 1.0);
   LS::addTerrainTexture("ixfdd.bmp", XFDD, 0xFF, 0, Snow, 0.5, 1.0);
   LS::addTerrainTexture("ixfff.bmp", XFFF, 0xFF, 0, Snow, 0.5, 1.0);
   LS::addTerrainTexture("ixxdd.bmp", XXDD, 0xFF, 0, Snow, 0.5, 1.0);
   LS::addTerrainTexture("ixxff.bmp", XXFF, 0xFF, 0, Snow, 0.5, 1.0);
   LS::addTerrainTexture("iIIII3.BMP", IIII, 0xFF, 25, Ice, 0.5, 1.0);
   LS::addTerrainTexture("iIIII2.BMP", IIII, 0xFF, 25, Ice, 0.5, 1.0);
   LS::addTerrainTexture("iIIII1.BMP", IIII, 0xFF, 25, Ice, 0.5, 1.0);
   LS::addTerrainTexture("iIIII.BMP", IIII, 0xFF, 25, Ice, 0.5, 1.0);
   LS::addTerrainTexture("idfii.bmp", DFII, 0xFF, 0, Ice, 0.5, 1.0);
   LS::addTerrainTexture("idiii.bmp", DIII, 0xFF, 0, Ice, 0.5, 1.0);
   LS::addTerrainTexture("idxii.bmp", DXII, 0xFF, 0, Ice, 0.5, 1.0);
   LS::addTerrainTexture("ifiii.bmp", FIII, 0xFF, 0, Ice, 0.5, 1.0);
   LS::addTerrainTexture("ifxii.bmp", FXII, 0xFF, 0, Ice, 0.5, 1.0);
   LS::addTerrainTexture("iiddd.bmp", IDDD, 0xFF, 0, Snow, 0.5, 1.0);
   LS::addTerrainTexture("iidff.bmp", IDFF, 0xFF, 0, Snow, 0.5, 1.0);
   LS::addTerrainTexture("iidxx.bmp", IDXX, 0xFF, 0, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("iifdd.bmp", IFDD, 0xFF, 0, Snow, 0.5, 1.0);
   LS::addTerrainTexture("iifff.bmp", IFFF, 0xFF, 0, Snow, 0.5, 1.0);
   LS::addTerrainTexture("iifxx.bmp", IFXX, 0xFF, 0, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("iiidd.bmp", IIDD, 0xFF, 0, Snow, 0.5, 1.0);
   LS::addTerrainTexture("iiiff.bmp", IIFF, 0xFF, 0, Snow, 0.5, 1.0);
   LS::addTerrainTexture("iiixx.bmp", IIXX, 0xFF, 0, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("iixdd.bmp", IXDD, 0xFF, 0, Snow, 0.5, 1.0);
   LS::addTerrainTexture("iixff.bmp", IXFF, 0xFF, 0, Snow, 0.5, 1.0);
   LS::addTerrainTexture("iixxx.bmp", IXXX, 0xFF, 0, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("ixiii.bmp", XIII, 0xFF, 0, Ice, 0.5, 1.0);
   LS::addTerrainTexture("idfyy.bmp", DFYY, 0xFF, 0, Stone, 0.5, 1.0);
   LS::addTerrainTexture("idiyy.bmp", DIYY, 0xFF, 0, Stone, 0.5, 1.0);
   LS::addTerrainTexture("idxyy.bmp", DXYY, 0xFF, 0, Stone, 0.5, 1.0);
   LS::addTerrainTexture("idyyy.bmp", DYYY, 0xFF, 0, Stone, 0.5, 1.0);
   LS::addTerrainTexture("ifiyy.bmp", FIYY, 0xFF, 0, Stone, 0.5, 1.0);
   LS::addTerrainTexture("ifxyy.bmp", FXYY, 0xFF, 0, Stone, 0.5, 1.0);
   LS::addTerrainTexture("ifyyy.bmp", FYYY, 0xFF, 0, Stone, 0.5, 1.0);
   LS::addTerrainTexture("iixyy.bmp", IXYY, 0xFF, 0, Stone, 0.5, 1.0);
   LS::addTerrainTexture("iiyyy.bmp", IYYY, 0xFF, 0, Stone, 0.5, 1.0);
   LS::addTerrainTexture("ixyyy.bmp", XYYY, 0xFF, 0, Stone, 0.5, 1.0);
   LS::addTerrainTexture("iyddd.bmp", YDDD, 0xFF, 0, Snow, 0.5, 1.0);
   LS::addTerrainTexture("iydff.bmp", YDFF, 0xFF, 0, Snow, 0.5, 1.0);
   LS::addTerrainTexture("iydii.bmp", YDII, 0xFF, 0, Ice, 0.5, 1.0);
   LS::addTerrainTexture("iydxx.bmp", YDXX, 0xFF, 0, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("iyfdd.bmp", YFDD, 0xFF, 0, Snow, 0.5, 1.0);
   LS::addTerrainTexture("iyfff.bmp", YFFF, 0xFF, 0, Snow, 0.5, 1.0);
   LS::addTerrainTexture("iyfii.bmp", YFII, 0xFF, 0, Ice, 0.5, 1.0);
   LS::addTerrainTexture("iyfxx.bmp", YFXX, 0xFF, 0, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("iyidd.bmp", YIDD, 0xFF, 0, Snow, 0.5, 1.0);
   LS::addTerrainTexture("iyiff.bmp", YIFF, 0xFF, 0, Snow, 0.5, 1.0);
   LS::addTerrainTexture("iyiii.bmp", YIII, 0xFF, 0, Ice, 0.5, 1.0);
   LS::addTerrainTexture("iyixx.bmp", YIXX, 0xFF, 0, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("iyxdd.bmp", YXDD, 0xFF, 0, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("iyxff.bmp", YXFF, 0xFF, 0, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("iyxii.bmp", YXII, 0xFF, 0, Ice, 0.5, 1.0);
   LS::addTerrainTexture("iyxxx.bmp", YXXX, 0xFF, 0, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("iyydd.bmp", YYDD, 0xFF, 0, Snow, 0.5, 1.0);
   LS::addTerrainTexture("iyyff.bmp", YYFF, 0xFF, 0, Snow, 0.5, 1.0);
   LS::addTerrainTexture("iyyii.bmp", YYII, 0xFF, 0, Ice, 0.5, 1.0);
   LS::addTerrainTexture("iyyxx.bmp", YYXX, 0xFF, 0, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("iffff.bmp", FFFF, 0xFF, 25, Snow, 0.5, 1.0);
   LS::addTerrainTexture("iffff2.bmp", FFFF, 0xFF, 25, Snow, 0.5, 1.0);
   LS::addTerrainTexture("iffff3.bmp", FFFF, 0xFF, 25, Snow, 0.5, 1.0);
   LS::addTerrainTexture("iffff4.bmp", FFFF, 0xFF, 25, Snow, 0.5, 1.0);
   LS::addTerrainTexture("iffff.bmp", FFFF, 0xFF, 25, Snow, 0.5, 1.0);
   LS::addTerrainTexture("iffff2.bmp", FFFF, 0xFF, 25, Snow, 0.5, 1.0);
   LS::addTerrainTexture("iffff3.bmp", FFFF, 0xFF, 25, Snow, 0.5, 1.0);
   LS::addTerrainTexture("iffff4.bmp", FFFF, 0xFF, 25, Snow, 0.5, 1.0);
   LS::addTerrainTexture("iffff.bmp", FFFF, 0xFF, 25, Snow, 0.5, 1.0);
   LS::addTerrainTexture("iffff2.BMP", FFFF, 0xFF, 25, Snow, 0.5, 1.0);
   LS::addTerrainTexture("iffff3.BMP", FFFF, 0xFF, 25, Snow, 0.5, 1.0);
   LS::addTerrainTexture("iffff4.bmp", FFFF, 0xFF, 25, Snow, 0.5, 1.0);
   LS::addTerrainTexture("iffff.bmp", FFFF,  0xFF, 25, Snow, 0.5, 1.0);
   LS::addTerrainTexture("iffff2.bmp", FFFF,  0xFF, 25, Snow, 0.5, 1.0);
   LS::addTerrainTexture("iffff3.bmp", FFFF,  0xFF, 25, Snow, 0.5, 1.0);
   LS::addTerrainTexture("iffff4.BMP", FFFF, 0xFF, 25, Snow, 0.5, 1.0);
   LS::addTerrainTexture("iffff.BMP", FFFF, 0xFF, 25, Snow, 0.5, 1.0);
   LS::addTerrainTexture("iffff2.BMP", FFFF, 0xFF, 25, Snow, 0.5, 1.0);
   LS::addTerrainTexture("iffff3.BMP", FFFF, 0xFF, 25, Snow, 0.5, 1.0);
   LS::addTerrainTexture("iffff4.BMP", FFFF, 0xFF, 25, Snow, 0.5, 1.0);
   LS::addTerrainTexture("iffff.BMP", FFFF, 0xFF, 25, Snow, 0.5, 1.0);
   LS::addTerrainTexture("iffff2.BMP", FFFF, 0xFF, 25, Snow, 0.5, 1.0);
   LS::addTerrainTexture("iffff3.bmp", FFFF, 0xFF, 25, Snow, 0.5, 1.0);
   LS::addTerrainTexture("iffff4.bmp", FFFF, 0xFF, 25, Snow, 0.5, 1.0);
   LS::addTerrainTexture("iffff.bmp", FFFF, 0xFF, 25, Snow, 0.5, 1.0);
   LS::addTerrainTexture("iffff2.bmp", FFFF, 0xFF, 25, Snow, 0.5, 1.0);
   LS::addTerrainTexture("iffff3.BMP", FFFF, 0xFF, 25, Snow, 0.5, 1.0);
   LS::addTerrainTexture("iffff4.BMP", FFFF, 0xFF, 25, Snow, 0.5, 1.0);
   LS::addTerrainTexture("iffff.BMP", FFFF, 0xFF, 25, Snow, 0.5, 1.0);
   LS::addTerrainTexture("iffff2.BMP", FFFF, 0xFF, 25, Snow, 0.5, 1.0);
   LS::addTerrainTexture("ipath5.bmp", PPPP, 0xFF, 25, Snow, 0.5, 1.0);
   LS::addTerrainTexture("ipath5a.bmp", PPPP, 0xFF, 25, Snow, 0.5, 1.0);
   LS::addTerrainTexture("ipath5b.bmp", PPPP, 0xFF, 25, Snow, 0.5, 1.0);
   LS::addTerrainTexture("ipath5c.bmp", PPPP, 0xFF, 25, Snow, 0.5, 1.0);

   // create the dat and dml for this world
   LS::createGridFile("temp\\ice.grid.dat", "temp\\ice.dml");
}
