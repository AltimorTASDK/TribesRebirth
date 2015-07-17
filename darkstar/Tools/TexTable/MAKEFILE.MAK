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

TexTable : TexTable.exe

#==============================================================================

%ifdef CG32
CPPFLAGS += -vG
%endif

TexTable.exe:                    \
%ifdef CG32
   c:\bc5\lib\cg32.lib           \
%endif
   $(BuildPrefix)Core.lib        \
   $(BuildPrefix)gfxio.lib       \
   $(BuildPrefix)ts3.lib         \
   $(BuildPrefix)ml.lib         \
   \
   TexTable.obj                    
   %do %.exe CONSOLE=yes
   %if %exist($(PhoenixTool))
      @xcopy /Q $(.TARGET) $(PhoenixTool)
   %endif

#   %do %.exe PRECOMPILED_HEADER=esCore.cpp

#==============================================================================
#example

mars:
   @bcc32 -c -a4 -nobj code\mars.terrain.cpp
   @lstrip obj\mars.terrain.obj exe\mars.terrain.dat
   @exe\textable exe\mars.terrain.dat exe\Mars.Grid.dat exe\Mars.dml
