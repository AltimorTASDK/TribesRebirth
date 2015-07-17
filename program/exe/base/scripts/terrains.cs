// -------------------------------------------------
// declare the terrain types here
// -------------------------------------------------
$terrainTypes[0, type]           = "flat";
$terrainTypes[0, description]    = "Flat terrain";
$terrainTypes[0, visDistance]    = 900;
$terrainTypes[0, hazeDistance]   = 600;
$terrainTypes[0, screenSize]     = 90;

$terrainTypes[1, type]           = "rolplains";
$terrainTypes[1, description]    = "Rolling plains";
$terrainTypes[1, visDistance]    = 600;
$terrainTypes[1, hazeDistance]   = 400;
$terrainTypes[1, screenSize]     = 70;

$terrainTypes[2, type]           = "rolhills";
$terrainTypes[2, description]    = "Rolling hills";
$terrainTypes[2, visDistance]    = 500;
$terrainTypes[2, hazeDistance]   = 400;
$terrainTypes[2, screenSize]     = 70;

$terrainTypes[3, type]           = "modhills";
$terrainTypes[3, description]    = "Moderately hilly";
$terrainTypes[3, visDistance]    = 500;
$terrainTypes[3, hazeDistance]   = 400;
$terrainTypes[3, screenSize]     = 70;

$terrainTypes[4, type]           = "bighills";
$terrainTypes[4, description]    = "Large hills";
$terrainTypes[4, visDistance]    = 500;
$terrainTypes[4, hazeDistance]   = 400;
$terrainTypes[4, screenSize]     = 55;

$terrainTypes[5, type]           = "rughills";
$terrainTypes[5, description]    = "Rugged hills";
$terrainTypes[5, visDistance]    = 300;
$terrainTypes[5, hazeDistance]   = 200;
$terrainTypes[5, screenSize]     = 40;

// -------------------------------------------------
// the different terrain sizes
// -------------------------------------------------
$terrainSizes[0, size]            = 64;
$terrainSizes[0, name]            = "Small";
$terrainSizes[1, size]            = 128;
$terrainSizes[1, name]            = "Medium";
$terrainSizes[2, size]            = 256;
$terrainSizes[2, name]            = "Large";

// -------------------------------------------------
// pretty flat terrain
// -------------------------------------------------
function Terrain::flat::create(%size, %seed)
{
   if(%seed == "")
      %seed = 0;
   
   LS::addCommand("seed " @ %seed);
   LS::addCommand("terrain " @ %size @ " .9");
   LS::addCommand("normalize 0 15");
}

// -------------------------------------------------
// rolling plains
// -------------------------------------------------
function Terrain::rolplains::create(%size, %seed)
{
   if(%seed == "")
      %seed = 0;
   LS::addCommand("seed " @ %seed);
   LS::addCommand("terrain " @ %size @ " .95");
   
   if(%size == 64)
   {
      LS::addCommand("normalize 0 49");
   }
   else if(%size == 128)
   {
      LS::addCommand("normalize 0 59");
   }
   else if(%size == 256)
   {
      LS::addCommand("normalize 0 59");
      LS::addCommand("terrain 64 .96");
      LS::addCommand("tile");
      LS::addCommand("tile");
      LS::addCommand("normalize 0 24");
      LS::addCommand("add_stack 1");
   }
}

// -------------------------------------------------
// rolling hills
// -------------------------------------------------
function Terrain::rolhills::create(%size, %seed)
{
   if(%seed == "")
      %seed = 0;
 
   LS::addCommand("seed " @ %seed);
   LS::addCommand("terrain " @ %size @ " .95");
   
   if(%size == 64)
   {
      LS::addCommand("normalize 0 59");
      LS::addCommand("smooth .3 .4");
   }
   else if(%size == 128)
   {
      LS::addCommand("normalize 0 109");
      LS::addCommand("smooth .3 .4");
   }
   else if(%size == 256)
   {
      LS::addCommand("normalize 0 109");
      LS::addCommand("smooth .3 .4");
      LS::addCommand("terrain 64 .95");
      LS::addCommand("tile");
      LS::addCommand("tile");
      LS::addCommand("normalize 0 50");
      LS::addCommand("add_stack 1");
   }
}

// -------------------------------------------------
// moderately hilly
// -------------------------------------------------
function Terrain::modhills::create(%size, %seed)
{
   if(%seed == "")
      %seed = 0;
   LS::addCommand("seed " @ %seed);
   LS::addCommand("terrain " @ %size @ " .95");
   
   if(%size == 64)
   {
      LS::addCommand("normalize 0 109");
      LS::addCommand("smooth .3 .4");
   }
   else if(%size == 128)
   {
      LS::addCommand("normalize 0 169");
      LS::addCommand("smooth .3 .4");
   }
   else if(%size == 256)
   {
      LS::addCommand("normalize 0 239");
      LS::addCommand("smooth .3 .4");
      LS::addCommand("terrain 64 .9");
      LS::addCommand("tile");
      LS::addCommand("tile");
      LS::addCommand("normalize 0 37");
      LS::addCommand("add_stack 1");
   }
}

// -------------------------------------------------
// really hilly
// -------------------------------------------------
function Terrain::bighills::create(%size, %seed)
{
   if(%seed == "")
      %seed = 0;
   LS::addCommand("seed " @ %seed);
   LS::addCommand("terrain " @ %size @ " .95");
   
   if(%size == 64)
   {
      LS::addCommand("normalize 0 125");
      LS::addCommand("smooth .3 .4");
      LS::addCommand("terrain 16 .7");
      LS::addCommand("tile");
      LS::addCommand("tile");
      LS::addCommand("normalize 0 6");
      LS::addCommand("add_stack 1");
   }
   else if(%size == 128)
   {
      LS::addCommand("normalize 0 225");
      LS::addCommand("smooth .3 .4");
      LS::addCommand("terrain 32 .7");
      LS::addCommand("tile");
      LS::addCommand("tile");
      LS::addCommand("normalize 0 6");
      LS::addCommand("add_stack 1");
   }
   else if(%size == 256)
   {
      LS::addCommand("normalize 0 325");
      LS::addCommand("smooth .3 .4");
      LS::addCommand("terrain 32 .7");
      LS::addCommand("tile");
      LS::addCommand("tile");
      LS::addCommand("tile");
      LS::addCommand("normalize 0 12");
      LS::addCommand("add_stack 1");
   }
}

// -------------------------------------------------
// rugged hills
// -------------------------------------------------
function Terrain::rughills::create(%size, %seed)
{
   if(%seed == "")
      %seed = 0;
   LS::addCommand("terrain " @ %size @ " .95");

   if(%size == 64)
   {
      LS::addCommand("normalize 0 99");
      LS::addCommand("smooth .3 .4");
      LS::addCommand("terrain 32.7");
      LS::addCommand("tile");
      LS::addCommand("normalize  0 13");
      LS::addCommand("add_stack 1");
   }
   else if(%size == 128)
   {
      LS::addCommand("normalize 0 140");
      LS::addCommand("smooth .3 .4");
      LS::addCommand("terrain 32 .7");
      LS::addCommand("tile");
      LS::addCommand("tile");
      LS::addCommand("normalize  0 26");
      LS::addCommand("add_stack 1");
   }
   else if(%size == 256)
   {
      LS::addCommand("normalize 20 259");
      LS::addCommand("smooth .3 .4");
      LS::addCommand("terrain 32 .8");
      LS::addCommand("tile");
      LS::addCommand("tile");
      LS::addCommand("tile");
      LS::addCommand("normalize  0 43");
      LS::addCommand("add_stack 1");
   }
}
