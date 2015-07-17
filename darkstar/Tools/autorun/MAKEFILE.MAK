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

%if !%exists($(PhoenixMake)\Builtins.Mak)
   %abort 1 Error: Environment var "PhoenixMake" not set. Set to Phoenix\Makes directory
   @-md $(var)
%endif
%include <$(PHOENIXMAKE)\builtins.mak>


#==============================================================================


autorun.exe:                        \
%ifdef CG32
   c:\bc5\lib\cg32.lib              \
%endif
   $(BuildPrefix)Core.lib           \
   $(BuildPrefix)GW.lib             \
   $(BuildPrefix)gfxio.lib          \
   $(BuildPrefix)console.lib        \
   autorun.res                      \
   autorun.obj                      \
   asWindows.obj                    
   %do %.exe STATIC=YES

