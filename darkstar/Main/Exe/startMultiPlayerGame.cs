newServer
focusServer

# create the server delegate...

newObject CSDelegate MainCSDelegate 26000 true

newObject world SimSet 1
newObject simPalette SimPalette Lush.ppl
# Create the terrain
#

newObject Terrain SimTerrain Load tim1.dtf
newObject simSky SimSky 0.5 0.5 0.5

# Create the sun
newObject sunLight SimLight Directional 1.0 1.0 1.0  .5 .5 -1

# Add the objects to the world...
addToSet world sunLight simPalette

# Force the terrain to relight...
postAction Lush MoveUp 0

focusClient
