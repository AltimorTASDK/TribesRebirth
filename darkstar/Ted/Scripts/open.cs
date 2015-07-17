Ted::close $Ted::currFile
openFile Ted::diskName "Ted Volumes" *.ted
if test $dlgResult != [cancel]
	 newObject tedTerrVol SimVolume $Ted::diskName
    Ted::open $Ted::diskName
    Ted::focus
endif
