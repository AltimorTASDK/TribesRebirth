Ted::listSelections Ted::selectionName
if test $dlgResult != [cancel]
    Ted::deleteSelection $Ted::selectionName
endif
