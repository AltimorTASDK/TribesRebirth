Ted::listFlags Ted::flags
if test $dlgResult != [cancel]
    Ted::setFlags $Ted::flags
    flushTextureCache
endif
