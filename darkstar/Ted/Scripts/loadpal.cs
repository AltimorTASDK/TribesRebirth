openFile Ted::diskFile "Palette files" *.ppl
if test $dlgResult != [cancel]
    newObject tedpal SimPalette $Ted::diskFile
endif
