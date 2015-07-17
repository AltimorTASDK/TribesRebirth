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

OBJdest    =obj

%if !%exists($(PhoenixMake)\Builtins.Mak)
   %abort 1 Error: Environment var "PhoenixMake" not set. Set to Phoenix\Makes directory
   @-md $(var)
%endif
%include <$(PHOENIXMAKE)\builtins.mak>

%set PRECOMPILED_HEADER=

WSock32 : $(LIBdest)\wsock32.dll

#==============================================================================

CPPFLAGS += -Jgx- -Jg
LINK = tlink32.exe
LINKFLAGS   = -c -v -s -m -w-dup -V4.0 -o

%ifdef CG32
CPPFLAGS += -vG
%endif

$(LIBdest)\wsock32.dll:             \
%ifdef CG32
   c:\bc5\lib\cg32.lib              \
%endif
   $(BuildPrefix)Core.lib           \
   wsockPatch.obj wsock.obj         \
   wsock32.def
   %do %.dll
