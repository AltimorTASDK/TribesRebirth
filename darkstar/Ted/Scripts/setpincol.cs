editBox Config "Enter a color (0-255) for the pin grid display" Ted::pinGridCol
if test $dlgResult != [cancel]
   Ted::setPinGridColor $Ted::pinGridCol
endif
