Ted::listBlocks $Ted::currFile Ted::blockName
if test $dlgResult != [cancel]
    Ted::deleteBlock $Ted::currFile $Ted::blockName
endif
