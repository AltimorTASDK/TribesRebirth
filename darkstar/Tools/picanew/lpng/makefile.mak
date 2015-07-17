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

LPNG_OBJS = 		\
	png.obj 		\
	pngerror.obj 	\
	pngmem.obj 		\
	pngpread.obj 	\
	pngread.obj 	\
	pngset.obj 		\
	pngget.obj 		\
	pngrio.obj		\
	pngrtran.obj 	\
	pngrutil.obj 	\
	pngtrans.obj 	\
	pngwrite.obj 	\
	pngwtran.obj 	\
	pngwio.obj	 	\
	pngwutil.obj


#------------------------------------------------------------------------------
%ifdef CG32
CPPFLAGS += -vG
%endif

all: $(LIBdest)\$(DEBUG)$(COMPILER)LPng.lib

$(LIBdest)\$(DEBUG)$(COMPILER)LPng.lib:   \
   $(LPNG_OBJS)

#------------------------------------------------------------------------------
copy :
   %echo copying LPng files ...
   @xcopy /D inc $(PHOENIXINC) /R /Q /I > NUL
   @xcopy /D lib\*.lib $(PHOENIXLIB) /R /Q /I > NUL
   %if %defined(PHOENIXCODE)
      @xcopy /D code  $(PHOENIXCODE) /R /Q /I > NUL
   %endif
   %if "$(DEBUG)" && $(COMPILER) == "m"
      %echo copying Microsoft debug information ...
      @xcopy /D *.pdb $(PHOENIXLIB) /R /Q > NUL
   %endif
