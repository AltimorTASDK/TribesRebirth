#==============================================================================
#
#    $Workfile:   makefile.mak  $
#    $Version$
#    $Revision:   1.2  $
#
#    DESCRIPTION:
#
#    (c) Copyright 1995, Dynamix Inc.   All rights reserved.
#
#==============================================================================

LIBdest     =.
OBJdest     =obj

%if !%exists($(PhoenixMake)\Builtins.Mak)
   %abort 1 Error: Environment var "PhoenixMake" not set. Set to Phoenix\Makes directory
   @-md $(var)
%endif
%include <$(PHOENIXMAKE)\builtins.mak>

%set PRECOMPILED_HEADER=

Alpha : alpha.exe

#==============================================================================

%ifdef CG32
CPPFLAGS += -vG
%endif

CC_FLAGS += -DM_BACK

Alpha.exe:                       \
%ifdef CG32
   c:\bc5\lib\cg32.lib           \
%endif
   $(DEBUG)$(COMPILER)core.lib   \
   $(DEBUG)$(COMPILER)gfxio.lib  \
   alpha.obj
   %do %.exe CONSOLE=yes
   %if %exist($(PhoenixTool))
      @xcopy /Q $(.TARGET) $(PhoenixTool)
   %endif

#==============================================================================

# code has become obsolete with newer version of Phoenix Libraries
#
#bitview.exe:                     \
#   $(DEBUG)$(COMPILER)core.lib   \
#   bitview.obj                   \
#   bitview.res
#   %if %exist($(PhoenixTool))
#      @xcopy /Q $(.TARGET) $(PhoenixTool)
#   %endif
      
