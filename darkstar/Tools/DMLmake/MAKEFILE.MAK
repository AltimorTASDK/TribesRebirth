#==============================================================================
#    
#  MakeFile.mak for EarthSiege III
#    
#  (c) Copyright 1996, Dynamix Inc.   All rights reserved.
#
#==============================================================================

%if !%exists($(PhoenixMake)\Builtins.Mak)
   %abort 1 Error: Environment var "PhoenixMake" not set. Set to Phoenix\Makes directory
   @-md $(var)
%endif
%include <$(PHOENIXMAKE)\builtins.mak>

DMLMake : DMLMake.exe

#------------------------------------------------------------------------------
#CPPFLAGS += -vG
#   cg32.lib                      \

dmlmake.exe:   \
   $(BuildPrefix)core.lib  \
   $(BuildPrefix)gfxio.lib \
   $(BuildPrefix)ts3.lib   \
   $(BuildPrefix)ml.lib    \
   DMLmake.obj 
   %do %.exe CONSOLE=YES
   %if %exist($(PhoenixTool))
      @xcopy /Q $(.TARGET) $(PhoenixTool)
   %endif
