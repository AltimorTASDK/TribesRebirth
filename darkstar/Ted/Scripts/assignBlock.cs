edit2Box "Assign Block" "Enter the coords for the block" "X" "Y" Ted::bX Ted::bY

if test $dlgResult != [cancel]
   Ted::listBlocks $Ted::currFile Ted::blockName

   if test $dlgResult != [cancel]
       Ted::assignBlock $Ted::currFile $Ted::blockName $Ted::bX $Ted::bY
   endif

endif

