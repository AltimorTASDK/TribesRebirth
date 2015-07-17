Ted::listFlags Ted::flags
if test $dlgResult != [cancel]
    Ted::clearFlags $Ted::flags
    flushTextureCache
endif
