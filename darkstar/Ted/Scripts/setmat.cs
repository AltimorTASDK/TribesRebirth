Ted::listMaterials $Ted::currFile Ted::matIndex
if test $dlgResult != [cancel]
   Ted::setMaterials $Ted::matIndex
	flushTextureCache
endif

