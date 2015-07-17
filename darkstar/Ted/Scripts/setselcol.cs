editBox Config "Enter a color (0-255) for the selection drawing" Ted::selCol
if test $dlgResult != [cancel]
  Ted::setSelColor $Ted::selCol
endif
