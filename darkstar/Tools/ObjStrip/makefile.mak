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

ObjStrip : ObjStrip.exe

#==============================================================================

%ifdef CG32
CPPFLAGS += -vG
%endif

ObjStrip.exe :                   \
%ifdef CG32
   c:\bc5\lib\cg32.lib           \
%endif
   $(DEBUG)$(COMPILER)core.lib   \
   $(DEBUG)$(COMPILER)gfxio.lib  \
   $(DEBUG)$(COMPILER)GW.lib     \
   $(DEBUG)$(COMPILER)ml.lib     \
   ObjStrip.obj
   %do %.exe CONSOLE=yes
   %if %exist($(PhoenixTool))
      @xcopy /Q $(.TARGET) $(PhoenixTool)
   %endif
