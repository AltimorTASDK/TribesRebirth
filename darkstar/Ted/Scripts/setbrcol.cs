editBox Config "Enter a color (0-255) for the brush drawing" Ted::brushSelCol
if test $dlgResult != [cancel]
  Ted::setBrushSelColor $Ted::brushSelCol
endif
