//---------------------------------------------------------
// mud terrain file
//---------------------------------------------------------

// the terrain types for this world
function Terrain::Mud::setTypes()
{
   LS::flushTextures();
   
   LS::addTerrainType(S, " 1 Mud");
   LS::addTerrainType(D, " 2 Dirt");
   LS::addTerrainType(T, " 3 Rock");
   LS::addTerrainType(L, " 4 Lite grass");
   LS::addTerrainType(G, " 5 Mud and rock");
}

// default rules for this world type
function Terrain::Mud::setRules()
{
   Terrain::Mud::setTypes();
   LS::flushRules();

   LS::addRule(S, 50.0, 550.0, 150.0, 0.50, 0.50, 0, 0.00, 0.2,  1.5, 0.50, 0.50, 0);
   LS::addRule(D, 00.0, 550.0, 250.0, 0.50, 0.50, 0, 0.00, 1.0,  0.2, 0.50, 0.50, 0);
   LS::addRule(T, 00.0, 550.0, 25.0,  0.50, 0.50, 0, 0.00, 1.0,  0.1, 0.50, 0.50, 0);
   LS::addRule(G, 00.0, 550.0, 25.0,  0.50, 0.50, 0, 0.00, 1.0,  0.1, 0.50, 0.50, 0);
   LS::addRule(L, 00.0, 550.0, 25.0,  0.50, 0.50, 0, 0.00, 1.0,  0.1, 0.50, 0.50, 0);
}

// create the grid file and dml for this world
function Terrain::Mud::createGridFile()
{
   Terrain::Mud::setTypes();

   LS::addTerrainTexture("mdddd1.BMP", DDDD, 0xFF, 20, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("mdddd2.BMP", DDDD, 0xFF, 20, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("mdddd3.BMP", DDDD, 0xFF, 20, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("mdddd4.BMP", DDDD, 0xFF, 20, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("mdddd5.BMP", DDDD, 0xFF, 20, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("mtttt1.BMP", TTTT, 0xFF, 20, Stone, 0.5, 1.0);
   LS::addTerrainTexture("mtttt2.BMP", TTTT, 0xFF, 20, Stone, 0.5, 1.0);
   LS::addTerrainTexture("mtttt3.BMP", TTTT, 0xFF, 20, Stone, 0.5, 1.0);
   LS::addTerrainTexture("mtttt4.BMP", TTTT, 0xFF, 20, Stone, 0.5, 1.0);
   LS::addTerrainTexture("mtttt5.BMP", TTTT, 0xFF, 20, Stone, 0.5, 1.0);
   LS::addTerrainTexture("mssss1.BMP", SSSS, 0xFF, 11, Mud, 0.5, 1.0);
   LS::addTerrainTexture("mssss2.BMP", SSSS, 0xFF, 11, Mud, 0.5, 1.0);
   LS::addTerrainTexture("mssss3.BMP", SSSS, 0xFF, 11, Mud, 0.5, 1.0);
   LS::addTerrainTexture("mssss4.BMP", SSSS, 0xFF, 11, Mud, 0.5, 1.0);
   LS::addTerrainTexture("mssss5.BMP", SSSS, 0xFF, 11, Mud, 0.5, 1.0);
   LS::addTerrainTexture("mssss6.BMP", SSSS, 0xFF, 11, Mud, 0.5, 1.0);
   LS::addTerrainTexture("mssss7.BMP", SSSS, 0xFF, 11, Mud, 0.5, 1.0);
   LS::addTerrainTexture("mssss8.BMP", SSSS, 0xFF, 11, Mud, 0.5, 1.0);
   LS::addTerrainTexture("mssss9.BMP", SSSS, 0xFF, 12, Mud, 0.5, 1.0);
   LS::addTerrainTexture("mllll1.BMP", LLLL, 0xFF, 20, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("mllll2.BMP", LLLL, 0xFF, 20, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("mllll3.BMP", LLLL, 0xFF, 20, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("mllll4.BMP", LLLL, 0xFF, 20, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("mllll5.BMP", LLLL, 0xFF, 20, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("mddss1.bmp", DDSS, 0xFF, 50, Mud, 0.5, 1.0);
   LS::addTerrainTexture("mddtt1.bmp", DDTT, 0xFF, 50, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("mdsss1.bmp", DSSS, 0xFF, 50, Mud, 0.5, 1.0);
   LS::addTerrainTexture("mdttt1.bmp", DTTT, 0xFF, 50, Stone, 0.5, 1.0);
   LS::addTerrainTexture("msddd1.bmp", SDDD, 0xFF, 50, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("msstt1.bmp", SSTT, 0xFF, 50, Mud, 0.5, 1.0);
   LS::addTerrainTexture("msttt1.bmp", STTT, 0xFF, 50, Stone, 0.5, 1.0);
   LS::addTerrainTexture("mtddd1.bmp", TDDD, 0xFF, 50, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("mtddd.bmp", TDDD, 0xFF, 50, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("mdsss.bmp", DSSS, 0xFF, 50, Mud, 0.5, 1.0);
   LS::addTerrainTexture("mddss.bmp", DDSS, 0xFF, 50, Mud, 0.5, 1.0);
   LS::addTerrainTexture("mddtt.bmp", DDTT, 0xFF, 50, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("mdttt.bmp", DTTT, 0xFF, 50, Stone, 0.5, 1.0);
   LS::addTerrainTexture("msddd.bmp", SDDD, 0xFF, 50, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("msstt.bmp", SSTT, 0xFF, 50, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("msttt.bmp", STTT, 0xFF, 50, Stone, 0.5, 1.0);
   LS::addTerrainTexture("mtsss.bmp", TSSS, 0xFF, 50, Mud, 0.5, 1.0);
   LS::addTerrainTexture("mtsss1.bmp", TSSS, 0xFF, 50, Mud, 0.5, 1.0);
   LS::addTerrainTexture("mtlll1.bmp", TLLL, 0xFF, 50, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("mdtll1.bmp", DTLL, 0xFF, 50, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("mtldd1.bmp", TLDD, 0xFF, 50, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("mdlss1.bmp", DLSS, 0xFF, 50, Mud, 0.5, 1.0);
   LS::addTerrainTexture("mdsll1.bmp", DSLL, 0xFF, 50, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("msldd1.bmp", SLDD, 0xFF, 50, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("msltt1.bmp", SLTT, 0xFF, 50, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("mstll1.bmp", STLL, 0xFF, 50, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("mtlss1.bmp", TLSS, 0xFF, 50, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("mtlll.bmp", TLLL, 0xFF, 50, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("mttll.bmp", TTLL, 0xFF, 50, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("mlsss.bmp", LSSS, 0xFF, 50, Mud, 0.5, 1.0);
   LS::addTerrainTexture("mslll.bmp", SLLL, 0xFF, 50, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("mssll.bmp", SSLL, 0xFF, 50, Mud, 0.5, 1.0);
   LS::addTerrainTexture("mddll.bmp", DDLL, 0xFF, 50, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("mdlll.bmp", DLLL, 0xFF, 50, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("mdlll1.bmp", DLLL, 0xFF, 50, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("mlddd.bmp", LDDD, 0xFF, 50, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("msltt.bmp", SLTT, 0xFF, 50, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("mstll.bmp", STLL, 0xFF, 50, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("mtlss.bmp", TLSS, 0xFF, 50, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("mlttt.bmp", LTTT, 0xFF, 50, Stone, 0.5, 1.0);
   LS::addTerrainTexture("mddll1.bmp", DDLL, 0xFF, 50, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("mlddd1.bmp", LDDD, 0xFF, 50, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("mdltt.bmp", DLTT, 0xFF, 50, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("mdtll.bmp", DTLL, 0xFF, 50, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("mtldd.bmp", TLDD, 0xFF, 50, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("mdlss.bmp", DLSS, 0xFF, 50, Mud, 0.5, 1.0);
   LS::addTerrainTexture("mdsll.bmp", DSLL, 0xFF, 50, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("mdstt.bmp", DSTT, 0xFF, 50, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("mdstt1.bmp", DSTT, 0xFF, 50, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("mdtss.bmp", DTSS, 0xFF, 50, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("mdtss1.bmp", DTSS, 0xFF, 50, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("msldd.bmp", SLDD, 0xFF, 50, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("mstdd.bmp", STDD, 0xFF, 50, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("mstdd1.bmp", STDD, 0xFF, 50, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("mlttt1.bmp", LTTT, 0xFF, 50, Stone, 0.5, 1.0);
   LS::addTerrainTexture("mdltt1.bmp", DLTT, 0xFF, 50, Stone, 0.5, 1.0);
   LS::addTerrainTexture("mttll1.bmp", TTLL, 0xFF, 50, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("mlsss1.bmp", LSSS, 0xFF, 50, Mud, 0.5, 1.0);
   LS::addTerrainTexture("mslll1.bmp", SLLL, 0xFF, 50, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("mssll1.bmp", SSLL, 0xFF, 50, Mud, 0.5, 1.0);
   LS::addTerrainTexture("mgggg1.bmp", GGGG, 0xFF, 25, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("mgggg2.bmp", GGGG, 0xFF, 25, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("mgggg3.bmp", GGGG, 0xFF, 25, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("mgggg4.bmp", GGGG, 0xFF, 25, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("msggg.bmp", SGGG, 0xFF, 50, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("mgtss.bmp", GTSS, 0xFF, 50, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("mstgg.bmp", STGG, 0xFF, 50, Stone, 0.5, 1.0);
   LS::addTerrainTexture("mdgtt.bmp", DGTT, 0xFF, 50, Stone, 0.5, 1.0);
   LS::addTerrainTexture("mdtgg.bmp", DTGG, 0xFF, 50, Stone, 0.5, 1.0);
   LS::addTerrainTexture("mgtdd.bmp", GTDD, 0xFF, 50, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("mdgss.bmp", DGSS, 0xFF, 50, Mud, 0.5, 1.0);
   LS::addTerrainTexture("mdsgg.bmp", DSGG, 0xFF, 50, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("mgsdd.bmp", GSDD, 0xFF, 50, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("mggtt.bmp", GGTT, 0xFF, 50, Stone, 0.5, 1.0);
   LS::addTerrainTexture("mgttt.bmp", GTTT, 0xFF, 50, Stone, 0.5, 1.0);
   LS::addTerrainTexture("mtggg.bmp", TGGG, 0xFF, 50, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("mggss.bmp", GGSS, 0xFF, 50, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("mgsss.bmp", GSSS, 0xFF, 50, Mud, 0.5, 1.0);
   LS::addTerrainTexture("mgstt.bmp", GSTT, 0xFF, 50, Stone, 0.5, 1.0);
   LS::addTerrainTexture("mddgg.bmp", DDGG, 0xFF, 50, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("mdggg.bmp", DGGG, 0xFF, 50, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("mgddd.bmp", GDDD, 0xFF, 50, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("msggg1.bmp", SGGG, 0xFF, 50, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("mgtss1.bmp", GTSS, 0xFF, 50, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("mstgg1.bmp", STGG, 0xFF, 50, Stone, 0.5, 1.0);
   LS::addTerrainTexture("mdgtt1.bmp", DGTT, 0xFF, 50, Stone, 0.5, 1.0);
   LS::addTerrainTexture("mdtgg1.bmp", DTGG, 0xFF, 50, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("mgtdd1.bmp", GTDD, 0xFF, 50, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("mdgss1.bmp", DGSS, 0xFF, 50, Mud, 0.5, 1.0);
   LS::addTerrainTexture("mdsgg1.bmp", DSGG, 0xFF, 50, Mud, 0.5, 1.0);
   LS::addTerrainTexture("mgsdd1.bmp", GSDD, 0xFF, 50, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("mggtt1.bmp", GGTT, 0xFF, 50, Stone, 0.5, 1.0);
   LS::addTerrainTexture("mgttt1.bmp", GTTT, 0xFF, 50, Stone, 0.5, 1.0);
   LS::addTerrainTexture("mtggg1.bmp", TGGG, 0xFF, 50, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("mggss1.bmp", GGSS, 0xFF, 50, Mud, 0.5, 1.0);
   LS::addTerrainTexture("mgsss1.bmp", GSSS, 0xFF, 50, Mud, 0.5, 1.0);
   LS::addTerrainTexture("mgstt1.bmp", GSTT, 0xFF, 50, Stone, 0.5, 1.0);
   LS::addTerrainTexture("mddgg1.bmp", DDGG, 0xFF, 50, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("mdggg1.bmp", DGGG, 0xFF, 50, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("mgddd1.bmp", GDDD, 0xFF, 50, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("mdgll.bmp", DGLL, 0xFF, 50, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("mdgll1.bmp", DGLL, 0xFF, 50, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("mdlgg.bmp", DLGG, 0xFF, 50, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("mdlgg1.bmp", DLGG, 0xFF, 50, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("mggll.bmp", GGLL, 0xFF, 50, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("mggll1.bmp", GGLL, 0xFF, 50, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("mgldd.bmp", GLDD, 0xFF, 50, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("mgldd1.bmp", GLDD, 0xFF, 50, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("mglll.bmp", GLLL, 0xFF, 50, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("mglll1.bmp", GLLL, 0xFF, 50, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("mglss.bmp", GLSS, 0xFF, 50, Mud, 0.5, 1.0);
   LS::addTerrainTexture("mglss1.bmp", GLSS, 0xFF, 50, Mud, 0.5, 1.0);
   LS::addTerrainTexture("mgsll.bmp", GSLL, 0xFF, 50, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("mgsll1.bmp", GSLL, 0xFF, 50, SoftEarth, 0.5, 1.0);
   LS::addTerrainTexture("mlggg.bmp", LGGG, 0xFF, 50, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("mlggg1.bmp", LGGG, 0xFF, 50, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("mslgg.bmp", SLGG, 0xFF, 50, PackedEarth, 0.5, 1.0);
   LS::addTerrainTexture("mslgg1.bmp", SLGG, 0xFF, 50, PackedEarth, 0.5, 1.0);

   // create the dat and dml for this world
   LS::createGridFile("temp\\mud.grid.dat", "temp\\mud.dml");
}
