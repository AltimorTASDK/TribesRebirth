editBox "Pin Grid" "Enter a starting detail level" Ted::iDetail
if test $dlgResult != [cancel]
  Ted::showPinGrid $Ted::iDetail
endif

