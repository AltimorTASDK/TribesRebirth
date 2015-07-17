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

Tag2Bin : tag2bin.exe

#==============================================================================

%ifdef CG32
CPPFLAGS += -vG
%endif

tag2bin.exe:                     	\
%ifdef CG32
   c:\bc5\lib\cg32.lib              \
%endif
   $(BuildPrefix)core.lib           \
   $(BuildPrefix)gfx.lib            \
   $(BuildPrefix)gfxio.lib          \
   $(BuildPrefix)ml.lib          \
   $(BuildPrefix)console.lib          \
   tag2bin.obj                 
   %do %.exe console=true
   %if %exist($(PhoenixTool))
      @xcopy /D /R /Q /I $(.TARGET) $(PhoenixTool)
   %endif

copy:
   %if %defined(PHOENIXTOOL)
      xcopy /D exe\tag2bin.exe $(PHOENIXTOOL) /R /Q /I
   %endif

