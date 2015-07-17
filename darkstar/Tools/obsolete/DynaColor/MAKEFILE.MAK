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

all:                             \
   DynaColor.exe

#------------------------------------------------------------------------------
#CPPFLAGS += -vG
#   cg32.lib                      \


#	MainWin.res  \

DynaColor.exe:                   \
   $(BuildPrefix)core.lib        \
	avi.obj		                  \
	bmp.obj		                  \
	choose.obj	                  \
	dynacolr.obj                  \
	globs.obj	                  \
	tga.obj		                  \
   blend.obj
	%do %.exe CONSOLE=YES
   %if %exist($(PhoenixTool))
      @xcopy /Q $(.TARGET) $(PhoenixTool)
   %endif
