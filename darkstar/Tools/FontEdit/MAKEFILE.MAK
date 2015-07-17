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

FontEd: fonted.exe

#==============================================================================

%ifdef CG32
CPPFLAGS += -vG
%endif

fonted.exe:                      \
%ifdef CG32
   c:\bc5\lib\cg32.lib           \
%endif
   $(DEBUG)$(COMPILER)core.lib   \
   $(DEBUG)$(COMPILER)gfx.lib    \
   $(DEBUG)$(COMPILER)gfxio.lib  \
   $(DEBUG)$(COMPILER)GW.lib     \
   $(DEBUG)$(COMPILER)ml.lib     \
   $(DEBUG)$(COMPILER)console.lib     \
   g_oldfont.obj                 \
   wintools.obj                  \
   fewindow.obj                  \
   viewWin.obj                   \
   editWin.obj                   \
   cInfoWin.obj                  \
   fInfoWin.obj                  \
   typeWin.obj                   \
   g_fontedit.obj                \
   fetool.res                    \
   

