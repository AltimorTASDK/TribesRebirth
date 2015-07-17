#
# Set the terrain visibility to some reasonable values
# based on ground scale
# Usage:  setDetail [terrainObj=Terrain] [groundScale=3]
#
if test "$1" == ""
   set 1 Terrain
   echo Default terrain: $1
endif
if test "$2" == ""
   set 2 3
   echo Default scale: $2
endif
#
if test "$2" == 3
   setTerrainVisibility $1 1500 600
   setTerrainDetail $1 200 25 50 100 150  200 250  300 400  500 900  1000 3000
endif
if test "$2" == 4
   setTerrainVisibility $1 1500 600
   setTerrainDetail $1 200 50 100  200 300  400 500 600 800 1800 2000 3000 5000 5000 30000
endif
if test "$2" == 6
   setTerrainVisibility $1 5000 4000
   setTerrainDetail $1 800 300 500  700 1000  1500 2500  3000 4000  9000 10000  30000 50000
endif
if test "$2" == 8
   setTerrainVisibility $1 10000 5000
   setTerrainDetail $1 1000 750 1500 3000 3500 6000 7000 10000 12000 18000 20000 30000 50000 50000 300000
endif
