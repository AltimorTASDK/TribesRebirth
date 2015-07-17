editBox Selection "Enter a name for the selection" Ted::selectionName
if test $dlgResult != [cancel]
    Ted::nameSelection $Ted::selectionName
endif
