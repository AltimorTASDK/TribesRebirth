edit2Box "Pin Area" "File in the values" "initial Detail level" "Deviation" Ted::iDetail Ted::deviation
if test $dlgResult != [cancel]
    Ted::pinArea $Ted::iDetail $Ted::deviation
endif



