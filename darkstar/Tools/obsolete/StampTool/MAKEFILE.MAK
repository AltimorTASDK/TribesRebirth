#==============================================================================
#    
#  Description MakeFile.mak for EarthSiege III
#    
#  (c) Copyright 1996, Dynamix Inc.   All rights reserved.
#
#==============================================================================



%if !%exists($(PhoenixMake)\Builtins.Mak)
   %abort 1 Error: Environment var "PhoenixMake" not set. Set to Phoenix\Makes directory
   @-md $(var)
%endif
%include <$(PHOENIXMAKE)\builtins.mak>

%set PRECOMPILED_HEADER=

all:                                   \
   StampTool.exe

#------------------------------------------------------------------------------
#CPPFLAGS += -vG
#   cg32.lib                           \


StampTool.exe:                         \
   $(BuildPrefix)Core.lib              \
   $(BuildPrefix)gfxio.lib             \
   $(BuildPrefix)ts3.lib               \
   $(BuildPrefix)GW.lib                \
   $(BuildPrefix)LS.lib                \
   \
   stampTool.res  	   	    		   \
   inspectDlg.obj                      \
   gridWindow.obj                      \
   main.obj                            \


   %do %.exe
   %if %exist($(PhoenixTool))
      @xcopy /Q $(.TARGET) $(PhoenixTool)
      @xcopy /Q exe\StampTool.hlp $(PhoenixTool)
   %endif

