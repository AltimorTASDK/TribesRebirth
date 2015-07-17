editBox Config "Enter a thickness [~1000-10,000] for the selection lines" Ted::lthickness
if test $dlgResult != [cancel]
    Ted::setLineThickness $Ted::lthickness
endif
