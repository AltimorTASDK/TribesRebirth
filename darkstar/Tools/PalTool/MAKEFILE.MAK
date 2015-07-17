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

PalTool : PalTool.exe   

#==============================================================================

%ifdef CG32
CPPFLAGS += -vG
%endif

PalTool.exe:                           \
%ifdef CG32
   c:\bc5\lib\cg32.lib           \
%endif
   $(BuildPrefix)Core.lib              \
   $(BuildPrefix)ml.lib                \
   $(BuildPrefix)gfxio.lib             \
   $(BuildPrefix)GW.lib                \
   $(BuildPrefix)gfx.lib               \
   \
   ptMain.obj                          \
   ptDialog.obj                        \
   ptControl.obj                       \
   ptShadeEdit.obj                     \
   ptBuild.obj                         \
   ptAbout.obj                         \
   luvConv.obj                         \
   paltool.res
   %do %.exe
   %if %exist($(PhoenixTool))
      @xcopy /Q $(.TARGET) $(PhoenixTool)
   %endif

##--------------------------------------
#PalMake.exe:                           \
#%ifdef CG32
#   c:\bc5\lib\cg32.lib                 \
#%endif
#   $(BuildPrefix)Core.lib              \
#   $(BuildPrefix)ml.lib                \
#   $(BuildPrefix)gfxio.lib             \
#   $(BuildPrefix)GW.lib                \
#   \
#   palMake.obj                         \
#   ptBuild.obj                         \
#   luvConv.obj                         
#   %do %.exe CONSOLE=YES
#   %if %exist($(PhoenixTool))
#      @xcopy /Q $(.TARGET) $(PhoenixTool)
#   %endif

#--------------------------------------
$(BuildPrefix)paltool.pch:             \
   ptCore.obj
