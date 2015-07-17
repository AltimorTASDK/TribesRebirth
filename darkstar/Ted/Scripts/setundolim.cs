editBox "Undo Stack" "Enter a new value for the limit" Ted::undoLim
if test $dlgResult != [cancel]
  Ted::setUndoStackLimit $Ted::undoLim
endif



