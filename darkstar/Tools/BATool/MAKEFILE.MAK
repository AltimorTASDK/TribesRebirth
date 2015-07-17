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

BaTools: BaTool.exe BaBuild.exe

#==============================================================================

%ifdef CG32
CPPFLAGS += -vG
%endif

BaTool.exe:                      \
%ifdef CG32
   c:\bc5\lib\cg32.lib           \
%endif
   $(DEBUG)$(COMPILER)core.lib   \
   $(DEBUG)$(COMPILER)gfxio.lib  \
   $(DEBUG)$(COMPILER)GW.lib     \
   $(DEBUG)$(COMPILER)ml.lib     \
   wintools.obj                  \
   bmlist.obj                    \
   bacanvas.obj                  \
   bawindow.obj                  \
   batool.res
   %do %.exe
   %if %exist($(PhoenixTool))
      @xcopy /Q $(.TARGET) $(PhoenixTool)
   %endif

BaBuild.exe:                     \
%ifdef CG32
   c:\bc5\lib\cg32.lib           \
%endif
   $(DEBUG)$(COMPILER)core.lib   \
   $(DEBUG)$(COMPILER)gfxio.lib  \
   $(DEBUG)$(COMPILER)ml.lib     \
   babuild.obj                   
   %do %.exe console=true
   %if %exist($(PhoenixTool))
      @xcopy /Q $(.TARGET) $(PhoenixTool)
   %endif

