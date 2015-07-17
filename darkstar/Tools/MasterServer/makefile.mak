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

all: mstrsvr testgame testspy parse

mstrsvr : mstrsvr.exe
testgame : testgame.exe
testspy : testspy.exe
parse : parse.exe

#==============================================================================

%ifdef CG32
CPPFLAGS += -vG
%endif

%ifdef OPT
CPPFLAGS += -x
%endif

CPPFLAGS += -Jg

mstrsvr.exe:                    \
%ifdef CG32
   c:\bc5\lib\cg32.lib          \
%endif
   mstrsvr.obj \
   servcmd.obj \
   servlist.obj \
   servlog.obj 
   %do %.exe CONSOLE=YES
   %if %exist($(PhoenixTool))
      @xcopy /Q $(.TARGET) $(PhoenixTool)
   %endif

testgame.exe:                    \
%ifdef CG32
   c:\bc5\lib\cg32.lib          \
%endif
   testgame.obj \
   servinfo.obj \
   servcmd.obj \
   servlist.obj \
   servlog.obj 
   %do %.exe CONSOLE=YES
   %if %exist($(PhoenixTool))
      @xcopy /Q $(.TARGET) $(PhoenixTool)
   %endif

testspy.exe:                    \
%ifdef CG32
   c:\bc5\lib\cg32.lib          \
%endif
   testspy.obj \
   servinfo.obj \
   servcmd.obj \
   servlist.obj \
   servlog.obj 
   %do %.exe CONSOLE=YES
   %if %exist($(PhoenixTool))
      @xcopy /Q $(.TARGET) $(PhoenixTool)
   %endif

parse.exe:                    \
%ifdef CG32
   c:\bc5\lib\cg32.lib          \
%endif
   parse.obj
   %do %.exe CONSOLE=YES
   %if %exist($(PhoenixTool))
      @xcopy /Q $(.TARGET) $(PhoenixTool)
   %endif
