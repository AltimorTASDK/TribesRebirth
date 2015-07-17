confirmBox "Pin Block" "This will pin all blocks touched by the selection. Continue?"
if test $dlgResult != [cancel]
    edit2Box "Pin Block" "Fill in the values" "initial Detail level" "Deviation" Ted::iDetail Ted::deviation

    if test $dlgResult != [cancel]
       Ted::pinBlock $Ted::iDetail $Ted::deviation
    endif
endif

