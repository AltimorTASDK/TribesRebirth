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

VT: VT.exe VTList.exe VInfo.exe Extract.exe VMerge.exe

#==============================================================================

%ifdef CG32
CPPFLAGS += -vG
%endif

VT.exe : \
%ifdef CG32
   c:\bc5\lib\cg32.lib          \
%endif
   $(DEBUG)$(COMPILER)core.lib  \
   $(DEBUG)$(COMPILER)gfxio.lib \
   $(DEBUG)$(COMPILER)ts3.lib   \
   $(DEBUG)$(COMPILER)ml.lib   \
   vt.obj
   @%set CC_FLAGS = -DM_BACK
   %do %.exe console=true
   %if %exist($(PhoenixTool))
      @xcopy /Q $(.TARGET) $(PhoenixTool)
   %endif

#==============================================================================

VInfo.exe : \
   $(DEBUG)$(COMPILER)core.lib \
   $(DEBUG)$(COMPILER)gfxio.lib \
   $(DEBUG)$(COMPILER)ml.lib   \
%ifdef CG32
   c:\bc5\lib\cg32.lib           \
%endif
   vinfo.obj
   @%set CC_FLAGS = -DM_BACK
   %do %.exe console=true
   %if %exist($(PhoenixTool))
      @xcopy /Q $(.TARGET) $(PhoenixTool)
   %endif

#==============================================================================

Extract.exe : \
   $(DEBUG)$(COMPILER)core.lib \
   $(DEBUG)$(COMPILER)gfxio.lib \
   $(DEBUG)$(COMPILER)ml.lib   \
%ifdef CG32
   c:\bc5\lib\cg32.lib           \
%endif
   xtrct.obj
   @%set CC_FLAGS = -DM_BACK
   %do %.exe console=true      
   %if %exist($(PhoenixTool))
      @xcopy /Q $(.TARGET) $(PhoenixTool)
   %endif

#==============================================================================

VTList.exe : \
   $(DEBUG)$(COMPILER)core.lib \
   $(DEBUG)$(COMPILER)gfxio.lib \
   $(DEBUG)$(COMPILER)ml.lib   \
%ifdef CG32
   c:\bc5\lib\cg32.lib           \
%endif
   VTList.obj
   @%set CC_FLAGS = -DM_BACK
   %do %.exe console=true
   %if %exist($(PhoenixTool))
      @xcopy /Q $(.TARGET) $(PhoenixTool)
   %endif

#==============================================================================

VMerge.exe : \
   $(DEBUG)$(COMPILER)core.lib \
   $(DEBUG)$(COMPILER)gfxio.lib \
   $(DEBUG)$(COMPILER)ml.lib   \
%ifdef CG32
   c:\bc5\lib\cg32.lib           \
%endif
   VMerge.obj
   @%set CC_FLAGS = -DM_BACK
   %do %.exe console=true
   %if %exist($(PhoenixTool))
      @xcopy /Q $(.TARGET) $(PhoenixTool)
   %endif
