#browseBox "New" "Select a material list for the new file" Ted::matListName "Material lists" *.dml
editBox "New" "Enter a material list name for the new file" Ted::matListName
if test $dlgResult == [cancel]
	return
endif

editBox "New" "Enter a name for the file" Ted::currFile
if test $dlgResult != [cancel]
    edit2Box "New" "Enter the file dimensions (in blocks)" "blocks Wide" "blocks High" Ted::bW Ted::bH

    if test $dlgResult != [cancel]
       edit2Box "New" "Enter a block dimension and groundscale" "Block dim" "Ground scale" Ted::bDim Ted::groundScale

       if test $dlgResult != [cancel]
          Ted::newTedFile $Ted::currFile $Ted::groundScale $Ted::bW $Ted::bH $Ted::bDim $Ted::matListName

			 editBox "New" "You may load a palette for this file" Ted::paletteFile
          if test $dlgResult == [ok]
				newObject tedpal SimPalette $Ted::paletteFile
          endif

          Ted::focus
          setDetail $Ted::terrainName 3
       endif
    endif
endif
