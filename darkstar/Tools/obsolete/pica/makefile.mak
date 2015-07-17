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

Pica : Pica.exe

#==============================================================================

%ifdef CG32
CPPFLAGS += -vG
%endif

Pica.exe:                       \
%ifdef CG32
   c:\bc5\lib\cg32.lib          \
%endif
   $(BuildPrefix)core.lib       \
   bmpio.obj                    \
   palio.obj                    \
   main.obj                     \
   pcaparse.obj                 \
   luvconv.obj                  \
   makepal.obj                  \
   render.obj                   \
   pcaquan.obj                  \
   tgaio.obj                    \
   imagefile.obj                \
   vector.obj                   \
   svector.obj                  \
   parsfuncs.obj
   %do %.exe CONSOLE=YES
   %if %exist($(PhoenixTool))
      @xcopy /Q $(.TARGET) $(PhoenixTool)
   %endif
