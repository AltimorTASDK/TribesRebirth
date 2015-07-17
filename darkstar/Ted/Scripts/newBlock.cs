editBox "New Block" "Enter a block name" Ted::blockName
if test $dlgResult != [cancel]
   Ted::newBLock $Ted::currFile $Ted::blockName
endif
