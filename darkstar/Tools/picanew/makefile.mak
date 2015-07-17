#==============================================================================
#      
#    $Workfile:   makefile.mak  $
#    $Version$
#    $Revision:   1.1  $
#      
#    DESCRIPTION:
#      
#    (c) Copyright 1995, Dynamix Inc.   All rights reserved.
#      
#==============================================================================

LIBdest    =.
OBJdest    =obj

%if !%exists($(PhoenixMake)\Builtins.Mak)
   %abort 1 Error: Environment var "PhoenixMake" not set. Set to Phoenix\Makes directory
   @-md $(var)
%endif
%include <$(PHOENIXMAKE)\builtins.mak>

%set PRECOMPILED_HEADER=

all: ZLib LPng PicaNew

PicaNew : picaNew.exe

#==============================================================================

LPng .MAKE .ALWAYS .MISER:
	@%chdir $(.TARGET)
	@make all copy $(MFLAGS)
	@%chdir $(MAKEDIR)

ZLib .MAKE .ALWAYS .MISER:
	@%chdir $(.TARGET)
	@make all copy $(MFLAGS)
	@%chdir $(MAKEDIR)

%ifdef CG32
CPPFLAGS += -vG
%endif

%ifdef OPT
CPPFLAGS += -x
%endif

CPPFLAGS += -Jg

PicaNew.exe:                    \
%ifdef CG32
   c:\bc5\lib\cg32.lib          \
%endif
   $(BuildPrefix)core.lib       \
   $(BuildPrefix)ml.lib         \
   $(BuildPrefix)gfxio.lib      \
   $(BuildPrefix)zlib.lib		\
   $(BuildPrefix)lpng.lib		\
   main.obj                     \
   pnOptions.obj                \
   pnState.obj                  \
   pnFileList.obj               \
   pnPseudoTrace.obj            \
   colorMatcher.obj				\
   pnPopTable.obj               \
   pnUnquantizedImage.obj       \
   pnBMPUQImage.obj             \
   pnPNGUQImage.obj             \
   pnUQImageFactory.obj         \
   pnQRenderer.obj              \
   luvconv.obj                  \
   vector.obj                   \
   svector.obj
   %do %.exe CONSOLE=YES
   %if %exist($(PhoenixTool))
      @xcopy /Q $(.TARGET) $(PhoenixTool)
   %endif
