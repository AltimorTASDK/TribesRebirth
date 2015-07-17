//---------------------------------------------------------
// desert terrain file
//---------------------------------------------------------

// the terrain types for this world
function Terrain::Desert::setTypes()
{
   LS::flushTextures();
   
   LS::addTerrainType(B, " 1 Sand 2");
   LS::addTerrainType(G, " 2 Sand 3");
   LS::addTerrainType(R, " 3 Rock");
   LS::addTerrainType(A, " 4 Orange sand");
   LS::addTerrainType(S, " 5 Sand and rock");
   LS::addTerrainType(M, " 6 Man made road");
   LS::addTerrainType(C, " 7 Pad");
   LS::addTerrainType(Y, " 8 Concrete pad");
}

// default rules for this world type
function Terrain::Desert::setRules()
{
   Terrain::Desert::setTypes();
   LS::flushRules();
   
   LS::addRule(B,  50.0, 300.0, 150.0, 0.50, 0.50, 0, 0.00, 8.0, 1.5, 0.50,  0.50, 0);
   LS::addRule(G,  15.0, 300.0, 180.0, 0.50, 0.50, 0, 0.10, 4.0, 0.5, 0.50,  0.50, 0);
   LS::addRule(S,  25.0, 300.0, 160.0, 0.50, 0.30, 0, 0.50, 2.0, 0.75, 0.50, 0.70, 0);
   LS::addRule(A, 160.0, 300.0, 200.0, 0.50, 0.9,  0, 0.00, 1.0, 0.8, 0.60,  0.10, 0);
   LS::addRule(M,   0.0, 200.0, 160.0, 0.50, 0.30, 0, 0.00, 8.0, 1.1, 0.50,  0.70, 0);
   LS::addRule(R, 200.0, 300.0, 250.0, 0.50, 0.80, 0, 0.00, 8.0, 1.3, 0.50,  0.20, 0);
}

// create the grid file and dml for this world
function Terrain::Desert::createGridFile()
{
   Terrain::Desert::setTypes();

   LS::addTerrainTexture("dbbbb.bmp", BBBB, 0xFF, 25, TS3_SandType, 0.5, 1.0);
   LS::addTerrainTexture("dbbbb1.bmp", BBBB, 0xFF, 25, TS3_SandType, 0.5, 1.0);
   LS::addTerrainTexture("dbbbb2.bmp", BBBB, 0xFF, 25, TS3_SandType, 0.5, 1.0);
   LS::addTerrainTexture("dbbbb3.bmp", BBBB, 0xFF, 25, TS3_SandType, 0.5, 1.0);
   LS::addTerrainTexture("dbbgg.bmp", BBGG, 0xFF, 0, TS3_SandType, 0.5, 1.0);
   LS::addTerrainTexture("dbggg.bmp", BGGG, 0xFF, 0, TS3_SandType, 0.5, 1.0);
   LS::addTerrainTexture("dgbbb.bmp", GBBB, 0xFF, 0, TS3_SandType, 0.5, 1.0);
   LS::addTerrainTexture("dgggg.BMP", GGGG, 0xFF, 25, TS3_SandType, 0.5, 1.0);
   LS::addTerrainTexture("dgggg1.bmp", GGGG, 0xFF, 25, TS3_SandType, 0.5, 1.0);
   LS::addTerrainTexture("dgggg2.BMP", GGGG, 0xFF, 25, TS3_SandType, 0.5, 1.0);
   LS::addTerrainTexture("dgggg3.bmp", GGGG, 0xFF,25, TS3_SandType, 0.5, 1.0);
   LS::addTerrainTexture("drrrr.BMP", RRRR,  0xFF, 8, TS3_StoneType, 0.5, 1.0);
   LS::addTerrainTexture("drrrr1.BMP", RRRR, 0xFF, 14, TS3_StoneType, 0.5, 1.0);
   LS::addTerrainTexture("drrrr2.BMP", RRRR, 0xFF, 15, TS3_StoneType, 0.5, 1.0);
   LS::addTerrainTexture("drrrr3.BMP", RRRR, 0xFF, 15, TS3_StoneType, 0.5, 1.0);
   LS::addTerrainTexture("drrrr4.BMP", RRRR, 0xFF, 15, TS3_StoneType, 0.5, 1.0);
   LS::addTerrainTexture("drrrr5.BMP", RRRR, 0xFF, 16, TS3_StoneType, 0.5, 1.0);
   LS::addTerrainTexture("drrrr6.BMP", RRRR, 0xFF, 16, TS3_StoneType, 0.5, 1.0);
   LS::addTerrainTexture("dbbrr.bmp", BBRR, 0xFF, 50, TS3_PackedEarthType, 0.5, 1.0);
   LS::addTerrainTexture("dbbrr1.bmp", BBRR, 0xFF, 50, TS3_PackedEarthType, 0.5, 1.0);
   LS::addTerrainTexture("dbrrr.bmp", BRRR, 0xFF, 50, TS3_StoneType, 0.5, 1.0);
   LS::addTerrainTexture("dbrrr1.bmp", BRRR, 0xFF, 50, TS3_StoneType, 0.5, 1.0);
   LS::addTerrainTexture("dgbrr.bmp", GBRR, 0xFF, 50, TS3_PackedEarthType, 0.5, 1.0);
   LS::addTerrainTexture("dgbrr1.bmp", GBRR, 0xFF, 50, TS3_PackedEarthType, 0.5, 1.0);
   LS::addTerrainTexture("dggrr.bmp", GGRR, 0xFF, 50, TS3_PackedEarthType, 0.5, 1.0);
   LS::addTerrainTexture("dggrr1.bmp", GGRR, 0xFF, 50, TS3_PackedEarthType, 0.5, 1.0);
   LS::addTerrainTexture("dgrbb.bmp", GRBB, 0xFF, 50, TS3_SandType, 0.5, 1.0);
   LS::addTerrainTexture("dgrbb1.bmp", GRBB, 0xFF, 50, TS3_SandType, 0.5, 1.0);
   LS::addTerrainTexture("dgrrr.bmp", GRRR, 0xFF, 50, TS3_StoneType, 0.5, 1.0);
   LS::addTerrainTexture("dgrrr1.bmp", GRRR, 0xFF, 50, TS3_StoneType, 0.5, 1.0);
   LS::addTerrainTexture("drbbb.bmp", RBBB, 0xFF, 50, TS3_SandType, 0.5, 1.0);
   LS::addTerrainTexture("drbbb1.bmp", RBBB, 0xFF, 50, TS3_SandType, 0.5, 1.0);
   LS::addTerrainTexture("drbgg.bmp", RBGG, 0xFF, 50, TS3_SandType, 0.5, 1.0);
   LS::addTerrainTexture("drbgg1.bmp", RBGG, 0xFF, 50, TS3_SandType, 0.5, 1.0);
   LS::addTerrainTexture("drggg.bmp", RGGG, 0xFF, 50, TS3_SandType, 0.5, 1.0);
   LS::addTerrainTexture("drggg1.bmp", RGGG, 0xFF, 50, TS3_SandType, 0.5, 1.0);
   LS::addTerrainTexture("daaaa.bmp", AAAA, 0xFF, 25, TS3_SandType, 0.5, 1.0);
   LS::addTerrainTexture("daaaa1.bmp", AAAA, 0xFF, 25, TS3_SandType, 0.5, 1.0);
   LS::addTerrainTexture("daaaa2.bmp", AAAA, 0xFF, 25, TS3_SandType, 0.5, 1.0);
   LS::addTerrainTexture("daaaa3.bmp", AAAA, 0xFF, 25, TS3_SandType, 0.5, 1.0);
   LS::addTerrainTexture("dabbb.bmp", ABBB, 0xFF, 0, TS3_SandType, 0.5, 1.0);
   LS::addTerrainTexture("daggg.bmp", AGGG, 0xFF, 0, TS3_SandType, 0.5, 1.0);
   LS::addTerrainTexture("darrr.bmp", ARRR, 0xFF, 50, TS3_StoneType, 0.5, 1.0);
   LS::addTerrainTexture("darrr1.bmp", ARRR, 0xFF, 50, TS3_StoneType, 0.5, 1.0);
   LS::addTerrainTexture("dbaaa.bmp", BAAA, 0xFF, 0, TS3_SandType, 0.5, 1.0);
   LS::addTerrainTexture("dbagg.bmp", BAGG, 0xFF, 0, TS3_SandType, 0.5, 1.0);
   LS::addTerrainTexture("dbarr.bmp", BARR, 0xFF, 50, TS3_PackedEarthType, 0.5, 1.0);
   LS::addTerrainTexture("dbarr1.bmp", BARR, 0xFF, 50, TS3_PackedEarthType, 0.5, 1.0);
   LS::addTerrainTexture("dbbaa.bmp", BBAA, 0xFF, 0, TS3_SandType, 0.5, 1.0);
   LS::addTerrainTexture("dbgaa.bmp", BGAA, 0xFF, 0, TS3_SandType, 0.5, 1.0);
   LS::addTerrainTexture("dbraa.bmp", BRAA, 0xFF, 50, TS3_SandType, 0.5, 1.0);
   LS::addTerrainTexture("dbraa1.bmp", BRAA, 0xFF, 50, TS3_SandType, 0.5, 1.0);
   LS::addTerrainTexture("dgaaa.bmp", GAAA, 0xFF, 0, TS3_SandType, 0.5, 1.0);
   LS::addTerrainTexture("dgabb.bmp", GABB, 0xFF, 0, TS3_SandType, 0.5, 1.0);
   LS::addTerrainTexture("dgarr.bmp", GARR, 0xFF, 50, TS3_PackedEarthType, 0.5, 1.0);
   LS::addTerrainTexture("dgarr1.bmp", GARR, 0xFF, 50, TS3_PackedEarthType, 0.5, 1.0);
   LS::addTerrainTexture("dggaa.bmp", GGAA, 0xFF, 0, TS3_SandType, 0.5, 1.0);
   LS::addTerrainTexture("dgraa.bmp", GRAA, 0xFF, 50, TS3_SandType, 0.5, 1.0);
   LS::addTerrainTexture("dgraa1.bmp", GRAA, 0xFF, 50, TS3_SandType, 0.5, 1.0);
   LS::addTerrainTexture("draaa.bmp", RAAA, 0xFF, 50, TS3_SandType, 0.5, 1.0);
   LS::addTerrainTexture("draaa1.bmp", RAAA, 0xFF, 50, TS3_SandType, 0.5, 1.0);
   LS::addTerrainTexture("drabb.bmp", RABB, 0xFF, 50, TS3_SandType, 0.5, 1.0);
   LS::addTerrainTexture("drabb1.bmp", RABB, 0xFF, 50, TS3_SandType, 0.5, 1.0);
   LS::addTerrainTexture("dragg.bmp", RAGG, 0xFF, 50, TS3_SandType, 0.5, 1.0);
   LS::addTerrainTexture("dragg1.bmp", RAGG, 0xFF, 50, TS3_SandType, 0.5, 1.0);
   LS::addTerrainTexture("drraa.bmp", RRAA, 0xFF, 50, TS3_PackedEarthType, 0.5, 1.0);
   LS::addTerrainTexture("drraa1.bmp", RRAA, 0xFF, 50, TS3_PackedEarthType, 0.5, 1.0);
   LS::addTerrainTexture("dyyyy1.bmp", YYBB, 0xFF, 9, TS3_ConcreteType, 0.5, 1.0);
   LS::addTerrainTexture("dyyyy1A.bmp", YYBB, 0xFF, 13, TS3_ConcreteType, 0.5, 1.0);
   LS::addTerrainTexture("dyyyy1B.bmp", YYBB, 0xFF, 13, TS3_ConcreteType, 0.5, 1.0);
   LS::addTerrainTexture("dyyyy1C.bmp", YYBB, 0xFF, 13, TS3_ConcreteType, 0.5, 1.0);
   LS::addTerrainTexture("dyyyy1D.bmp", YYBB, 0xFF, 13, TS3_ConcreteType, 0.5, 1.0);
   LS::addTerrainTexture("dyyyy1E.bmp", YYBB, 0xFF, 13, TS3_ConcreteType, 0.5, 1.0);
   LS::addTerrainTexture("dyyyy1F.bmp", YYBB, 0xFF, 13, TS3_ConcreteType, 0.5, 1.0);
   LS::addTerrainTexture("dyyyy1G.bmp", YYBB, 0xFF, 13, TS3_ConcreteType, 0.5, 1.0);
   LS::addTerrainTexture("dyyyy2.bmp", BYBB, 0xFF, 25, TS3_ConcreteType, 0.5, 1.0);
   LS::addTerrainTexture("dyyyy2A.bmp", BYBB, 0xFF, 25, TS3_ConcreteType, 0.5, 1.0);
   LS::addTerrainTexture("dyyyy2B.bmp", BYBB, 0xFF, 25, TS3_ConcreteType, 0.5, 1.0);
   LS::addTerrainTexture("dyyyy2C.bmp", BYBB, 0xFF, 25, TS3_ConcreteType, 0.5, 1.0);
   LS::addTerrainTexture("dyyyy3.bmp", YYYY, 0xFF, 25, TS3_ConcreteType, 0.5, 1.0);
   LS::addTerrainTexture("dyyyy3A.bmp", YYYY, 0xFF, 25, TS3_ConcreteType, 0.5, 1.0);
   LS::addTerrainTexture("dyyyy3B.bmp", YYYY, 0xFF, 25, TS3_ConcreteType, 0.5, 1.0);
   LS::addTerrainTexture("dyyyy3C.bmp", YYYY, 0xFF, 25, TS3_ConcreteType, 0.5, 1.0);
   LS::addTerrainTexture("dssss.bmp", SSSS, 0xFF, 25, TS3_PackedEarthType, 0.5, 1.0);
   LS::addTerrainTexture("dssss1.bmp", SSSS, 0xFF, 25, TS3_PackedEarthType, 0.5, 1.0);
   LS::addTerrainTexture("dssss2.bmp", SSSS, 0xFF, 25, TS3_PackedEarthType, 0.5, 1.0);
   LS::addTerrainTexture("dssss3.bmp", SSSS, 0xFF, 25, TS3_PackedEarthType, 0.5, 1.0);
   LS::addTerrainTexture("dabss.bmp", ABSS, 0xFF, 0, TS3_PackedEarthType, 0.5, 1.0);
   LS::addTerrainTexture("darss.bmp", ARSS, 0xFF, 0, TS3_PackedEarthType, 0.5, 1.0);
   LS::addTerrainTexture("dasss.bmp", ASSS, 0xFF, 0, TS3_PackedEarthType, 0.5, 1.0);
   LS::addTerrainTexture("dbsss.bmp", BSSS, 0xFF, 0, TS3_PackedEarthType, 0.5, 1.0);
   LS::addTerrainTexture("dgass.bmp", GASS, 0xFF, 0, TS3_PackedEarthType, 0.5, 1.0);
   LS::addTerrainTexture("dgbss.bmp", GBSS, 0xFF, 0, TS3_PackedEarthType, 0.5, 1.0);
   LS::addTerrainTexture("dgrss.bmp", GRSS, 0xFF, 0, TS3_PackedEarthType, 0.5, 1.0);
   LS::addTerrainTexture("dgsss.bmp", GSSS, 0xFF, 0, TS3_PackedEarthType, 0.5, 1.0);
   LS::addTerrainTexture("drbss.bmp", RBSS, 0xFF, 0, TS3_PackedEarthType, 0.5, 1.0);
   LS::addTerrainTexture("drsss.bmp", RSSS, 0xFF, 0, TS3_PackedEarthType, 0.5, 1.0);
   LS::addTerrainTexture("dsaaa.bmp", SAAA, 0xFF, 0, TS3_SandType, 0.5, 1.0);
   LS::addTerrainTexture("dsabb.bmp", SABB, 0xFF, 0, TS3_SandType, 0.5, 1.0);
   LS::addTerrainTexture("dsagg.bmp", SAGG, 0xFF, 0, TS3_SandType, 0.5, 1.0);
   LS::addTerrainTexture("dsarr.bmp", SARR, 0xFF, 0, TS3_StoneType, 0.5, 1.0);
   LS::addTerrainTexture("dsbaa.bmp", SBAA, 0xFF, 0, TS3_SandType, 0.5, 1.0);
   LS::addTerrainTexture("dsbbb.bmp", SBBB, 0xFF, 0, TS3_SandType, 0.5, 1.0);
   LS::addTerrainTexture("dsbgg.bmp", SBGG, 0xFF, 0, TS3_SandType, 0.5, 1.0);
   LS::addTerrainTexture("dsbrr.bmp", SBRR, 0xFF, 0, TS3_StoneType, 0.5, 1.0);
   LS::addTerrainTexture("dsgaa.bmp", SGAA, 0xFF, 0, TS3_SandType, 0.5, 1.0);
   LS::addTerrainTexture("dsgbb.bmp", SGBB, 0xFF, 0, TS3_SandType, 0.5, 1.0);
   LS::addTerrainTexture("dsggg.bmp", SGGG, 0xFF, 0, TS3_SandType, 0.5, 1.0);
   LS::addTerrainTexture("dsgrr.bmp", SGRR, 0xFF, 0, TS3_StoneType, 0.5, 1.0);
   LS::addTerrainTexture("dsraa.bmp", SRAA, 0xFF, 0, TS3_PackedEarthType, 0.5, 1.0);
   LS::addTerrainTexture("dsrbb.bmp", SRBB, 0xFF, 0, TS3_SandType, 0.5, 1.0);
   LS::addTerrainTexture("dsrgg.bmp", SRGG, 0xFF, 0, TS3_PackedEarthType, 0.5, 1.0);
   LS::addTerrainTexture("dsrrr.bmp", SRRR, 0xFF, 0, TS3_StoneType, 0.5, 1.0);
   LS::addTerrainTexture("dssaa.bmp", SSAA, 0xFF, 0, TS3_PackedEarthType, 0.5, 1.0);
   LS::addTerrainTexture("dssbb.bmp", SSBB, 0xFF, 0, TS3_PackedEarthType, 0.5, 1.0);
   LS::addTerrainTexture("dssgg.bmp", SSGG, 0xFF, 0, TS3_PackedEarthType, 0.5, 1.0);
   LS::addTerrainTexture("dssrr.bmp", SSRR, 0xFF, 0, TS3_StoneType, 0.5, 1.0);
   LS::addTerrainTexture("dgggg.bmp", GGGG, 0xFF, 25, TS3_SandType, 0.5, 1.0);
   LS::addTerrainTexture("dgggg1.bmp", GGGG, 0xFF, 25, TS3_SandType, 0.5, 1.0);
   LS::addTerrainTexture("dgggg2.bmp", GGGG, 0xFF, 25, TS3_SandType, 0.5, 1.0);
   LS::addTerrainTexture("dgggg3.bmp", GGGG, 0xFF, 25, TS3_SandType, 0.5, 1.0);
   LS::addTerrainTexture("dgggg.bmp", GGGG, 0xFF, 25, TS3_SandType, 0.5, 1.0);
   LS::addTerrainTexture("dgggg1.bmp", GGGG, 0xFF, 25, TS3_SandType, 0.5, 1.0);
   LS::addTerrainTexture("dgggg2.bmp", GGGG, 0xFF, 25, TS3_SandType, 0.5, 1.0);
   LS::addTerrainTexture("dgggg3.bmp", GGGG, 0xFF, 25, TS3_SandType, 0.5, 1.0);
   LS::addTerrainTexture("dgggg.bmp", GGGG, 0xFF, 25, TS3_SandType, 0.5, 1.0);
   LS::addTerrainTexture("dgggg1.bmp", GGGG, 0xFF, 25, TS3_SandType, 0.5, 1.0);
   LS::addTerrainTexture("dgggg2.bmp", GGGG, 0xFF, 25, TS3_SandType, 0.5, 1.0);
   LS::addTerrainTexture("dgggg3.bmp", GGGG, 0xFF, 25, TS3_SandType, 0.5, 1.0);
   LS::addTerrainTexture("dgggg.bmp", GGGG, 0xFF, 25, TS3_SandType, 0.5, 1.0);
   LS::addTerrainTexture("dgggg1.bmp", GGGG, 0xFF, 25, TS3_SandType, 0.5, 1.0);
   LS::addTerrainTexture("dgggg2.bmp", GGGG, 0xFF, 25, TS3_SandType, 0.5, 1.0);
   LS::addTerrainTexture("dgggg3.bmp", GGGG, 0xFF, 25, TS3_SandType, 0.5, 1.0);
   LS::addTerrainTexture("dgggg.bmp", GGGG, 0xFF, 25, TS3_SandType, 0.5, 1.0);
   LS::addTerrainTexture("dgggg1.bmp", GGGG, 0xFF, 25, TS3_SandType, 0.5, 1.0);
   LS::addTerrainTexture("dgggg2.bmp", GGGG, 0xFF, 25, TS3_SandType, 0.5, 1.0);
   LS::addTerrainTexture("droad1.bmp", CMMC, 0xFF, 15, TS3_ConcreteType, 0.5, 1.0);
   LS::addTerrainTexture("droad1a.bmp", CMMC, 0xFF, 15, TS3_ConcreteType, 0.5, 1.0);
   LS::addTerrainTexture("droad1b.bmp", CMMC, 0xFF, 16, TS3_ConcreteType, 0.5, 1.0);
   LS::addTerrainTexture("droad1c.bmp", CMMC, 0xFF, 18, TS3_ConcreteType, 0.5, 1.0);
   LS::addTerrainTexture("droad1d.bmp", CMMC, 0xFF, 18, TS3_ConcreteType, 0.5, 1.0);
   LS::addTerrainTexture("droad1e.bmp", CMMC, 0xFF, 18, TS3_ConcreteType, 0.5, 1.0);
   LS::addTerrainTexture("droad2.bmp", CCMC, 0xFF, 33, TS3_ConcreteType, 0.5, 1.0);
   LS::addTerrainTexture("droad2a.bmp", CCMC, 0xFF, 33, TS3_ConcreteType, 0.5, 1.0);
   LS::addTerrainTexture("droad2b.bmp", CCMC, 0xFF, 34, TS3_ConcreteType, 0.5, 1.0);
   LS::addTerrainTexture("droad3.BMP", CMMM, 0xFF, 50, TS3_ConcreteType, 0.5, 1.0);
   LS::addTerrainTexture("droad3a.BMP", CMMM, 0xFF, 50, TS3_ConcreteType, 0.5, 1.0);
   LS::addTerrainTexture("droad4.bmp", CMMM, 0, 0, TS3_ConcreteType, 0.5, 1.0);
   LS::addTerrainTexture("droad4a.bmp", CMMM, 0, 0, TS3_ConcreteType, 0.5, 1.0);
   LS::addTerrainTexture("droad5.bmp", GGMG, 0xFF, 25, TS3_ConcreteType, 0.5, 1.0);
   LS::addTerrainTexture("droad5a.bmp", GGMG, 0xFF, 25, TS3_ConcreteType, 0.5, 1.0);
   LS::addTerrainTexture("droad5b.bmp", GGMG, 0xFF, 25, TS3_ConcreteType, 0.5, 1.0);
   LS::addTerrainTexture("droad5c.bmp", GGMG, 0xFF, 25, TS3_ConcreteType, 0.5, 1.0);
   LS::addTerrainTexture("droad6.BMP", GMMM, 0xFF, 25, TS3_ConcreteType, 0.5, 1.0);
   LS::addTerrainTexture("droad6a.BMP", GMMM, 0xFF, 25, TS3_ConcreteType, 0.5, 1.0);
   LS::addTerrainTexture("droad6b.BMP", GMMM, 0xFF, 25, TS3_ConcreteType, 0.5, 1.0);
   LS::addTerrainTexture("droad6c.BMP", GMMM, 0xFF, 25, TS3_ConcreteType, 0.5, 1.0);
   LS::addTerrainTexture("droad7.bmp", GMMM, 0, 0, TS3_ConcreteType, 0.5, 1.0);
   LS::addTerrainTexture("droad7a.bmp", GMMM, 0, 0, TS3_ConcreteType, 0.5, 1.0);
   LS::addTerrainTexture("droad7b.bmp", GMMM, 0, 0, TS3_ConcreteType, 0.5, 1.0);
   LS::addTerrainTexture("droad7c.bmp", GMMM, 0, 0, TS3_ConcreteType, 0.5, 1.0);
   LS::addTerrainTexture("droad8.BMP", GMMG, 0xFF, 15, TS3_ConcreteType, 0.5, 1.0);
   LS::addTerrainTexture("droad8a.BMP", GMMG, 0xFF, 15, TS3_ConcreteType, 0.5, 1.0);
   LS::addTerrainTexture("droad8b.bmp", GMMG, 0xFF, 16, TS3_ConcreteType, 0.5, 1.0);
   LS::addTerrainTexture("droad8c.bmp", GMMG, 0xFF, 18, TS3_ConcreteType, 0.5, 1.0);
   LS::addTerrainTexture("droad8d.bmp", GMMG, 0xFF, 18, TS3_ConcreteType, 0.5, 1.0);
   LS::addTerrainTexture("droad8e.bmp", GMMG, 0xFF, 18, TS3_ConcreteType, 0.5, 1.0);
   LS::addTerrainTexture("droad9.BMP", CCCC, 0xFF, 25, TS3_ConcreteType, 0.5, 1.0);
   LS::addTerrainTexture("droad9a.BMP", CCCC, 0xFF, 25, TS3_ConcreteType, 0.5, 1.0);
   LS::addTerrainTexture("droad9b.BMP", CCCC, 0xFF, 25, TS3_ConcreteType, 0.5, 1.0);
   LS::addTerrainTexture("droad9c.BMP", CCCC, 0xFF, 25, TS3_ConcreteType, 0.5, 1.0);

   // create the dat and dml for this world
   LS::createGridFile("temp\\desert.grid.dat", "temp\\desert.dml");
}
