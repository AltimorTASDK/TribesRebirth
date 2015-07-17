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

ZLIB_OBJS = 		\
	adler32.obj		\
	compress.obj	\
	crc32.obj		\
	deflate.obj		\
	gzio.obj		\
	infblock.obj	\
	infcodes.obj	\
	inffast.obj		\
	inflate.obj		\
	inftrees.obj	\
	infutil.obj		\
	uncompr.obj		\
	zutil.obj


#------------------------------------------------------------------------------
%ifdef CG32
CPPFLAGS += -vG
%endif

all: $(LIBdest)\$(DEBUG)$(COMPILER)ZLib.lib

$(LIBdest)\$(DEBUG)$(COMPILER)ZLib.lib:   \
   $(ZLIB_OBJS)

#------------------------------------------------------------------------------
copy :
   %echo copying ZLib files ...
   @xcopy /D inc $(PHOENIXINC) /R /Q /I > NUL
   @xcopy /D lib\*.lib $(PHOENIXLIB) /R /Q /I > NUL
   %if %defined(PHOENIXCODE)
      @xcopy /D code  $(PHOENIXCODE) /R /Q /I > NUL
   %endif
   %if "$(DEBUG)" && $(COMPILER) == "m"
      %echo copying Microsoft debug information ...
      @xcopy /D *.pdb $(PHOENIXLIB) /R /Q > NUL
   %endif
