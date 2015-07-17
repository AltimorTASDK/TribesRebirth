#openFile Ted::diskFile "Material Lists" *.dml
editBox "MaterialList" "Enter material list name" Ted::diskFile
if test $dlgResult != [cancel]
    Ted::assignMatList $Ted::currFile $Ted::diskFile
endif
