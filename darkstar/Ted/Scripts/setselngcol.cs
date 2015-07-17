editBox Config "Enter a color(0-255) for the selecting color" Ted::selectingCol
if test $dlgResult != [cancel]
   Ted::setSelectingColor $Ted::selectingCol
endif
