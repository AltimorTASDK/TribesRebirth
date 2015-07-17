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

#==============================================================================

%ifdef CG32
CPPFLAGS += -vG
%endif

CPPFLAGS += -Jg

mpMerger.exe:					\
%ifdef CG32
   c:\bc5\lib\cg32.lib\
%endif
   $(DEBUG)$(COMPILER)core.lib       \
   $(DEBUG)$(COMPILER)gfxio.lib      \
   $(DEBUG)$(COMPILER)ml.lib         \
   $(DEBUG)$(COMPILER)gfxio.lib  \
   $(DEBUG)$(COMPILER)GW.lib     \
   $(DEBUG)$(COMPILER)ml.lib     \
#   mpmTemplates.obj				\
   main.obj						\
   mpmState.obj					\
   mpmPseudoTrace.obj			\
   mpmFileList.obj				\
   mpmOptions.obj				\
   mpmPopTable.obj				\
   mpmMerger.obj				\
   sVector.obj					\
   colorMatcher.obj				\
   vector.obj
   %do %.exe CONSOLE=YES
   %if %exist($(PhoenixTool))
      @xcopy /Q $(.TARGET) $(PhoenixTool)
   %endif
