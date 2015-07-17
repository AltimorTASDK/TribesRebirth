editBox "Ground Scale" "Enter a new value for the default ground scale" Ted::GroundScale
if test $dlgResult != [cancel]
   set SimTerrain::GroundScale $Ted::GroundScale
endif
