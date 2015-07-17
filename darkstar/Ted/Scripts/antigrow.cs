edit2Box "Antigrow" "Fill in the values" "iDetail" "Deviation" Ted::res Ted::res2
if test $dlgResult != [cancel]
   Ted::antiGrow $Ted::res $Ted::res2
endif
