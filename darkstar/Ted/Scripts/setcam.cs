editBox Config "Enter the name of the camera moveObj" Ted::camName
if test $dlgResult != [cancel]
  Ted::setCamera $Ted::camName
endif
