#같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같
#같   
#같 $Workfile:   makefile.mak  $
#같 $Version$
#같 $Revision:   1.10  $
#같   
#같 DESCRIPTION:
#같   
#같 (c) Copyright 1995, Dynamix Inc.   All rights reserved.
#같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같

PHOENIXMAKE ?= ..\makes
%include <$(PHOENIXMAKE)\builtins.mak>

#같같같같같같같같같같같같같같같같같같같같같같� LIBRARIES

CORE_OBJS =             $(BASE_OBJS)        \
                        $(FILEIO_OBJS)      \
                        $(PERSIST_OBJS)
                        
BASE_OBJS =             base.obj            \
                        bitset.obj          \
                        bitstream.obj       \
                        crc16.obj           \
                        fileName.obj        \
                        m_qsort.obj         \
                        registry.obj        \
                        rvecAlloc.obj       \
                        stringTable.obj     \
                        threadBase.obj      \
                        timer.obj           \
                        types.obj


FILEIO_OBJS =           blkstrm.obj         \
                        filrstrm.obj        \
                        filstrm.obj         \
                        filwstrm.obj        \
                        lzhrstrm.obj        \
                        lzhuff.obj          \
                        lzhwstrm.obj        \
                        memstrm.obj         \
                        resDictionary.obj   \
                        resManager.obj      \
                        rlerstrm.obj        \
                        rlewstrm.obj        \
                        streamio.obj        \
                        volstrm.obj

PERSIST_OBJS =          persist.obj     


#------------------------------------------------------------------------------
%ifdef CG32
CPPFLAGS += -vG
%endif

$(LIBdest)\$(DEBUG)$(COMPILER)Core.lib:   \
   $(CORE_OBJS)

#------------------------------------------------------------------------------
all:
   %foreach COMPILER in b m
        @:make -nologo -a COMPILER=$(COMPILER)
        @:make -nologo -a COMPILER=$(COMPILER) OPT=
   %endfor

#------------------------------------------------------------------------------
copy :
   %echo copying Core files ...
   @xcopy /D inc $(PHOENIXINC) /R /Q /I > NUL
   %if %defined(PHOENIXCODE)
      @xcopy /D code  $(PHOENIXCODE) /R /Q /I > NUL
   %endif
   @--xcopy /D lib\*.lib $(PHOENIXLIB) /R /Q /I > NUL
   %if "$(DEBUG)" && $(COMPILER) == "m"
      %echo copying Microsoft debug information ...
      @xcopy /D *.pdb $(PHOENIXLIB) /R /Q > NUL
   %endif

