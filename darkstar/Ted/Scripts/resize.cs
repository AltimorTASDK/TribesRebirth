edit2Box "Resize" "Enter new dimensions (in # of blocks)" "Blocks wide" "Blocks high" Ted::bW Ted::bH
if test $dlgResult != [cancel]
   Ted::resizeFile $Ted::currFile $Ted::bW $Ted::bH
endif
