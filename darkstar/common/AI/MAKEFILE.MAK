#==============================================================================
#    
#  MakeFile.mak for Bayesian test
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

Bayesian :                             \
   $(LIBdest)\$(BuildPrefix)Bayes.lib

#   BayWatch.exe                        \

#==========================================================================

%ifdef CG32
CPPFLAGS += -vG
%endif

%if "$(COMPILER)" == "M"
CPPFLAGS += /GX
%endif
#------------------------------------------------------------------------------

$(LIBdest)\$(BuildPrefix)Bayes.lib:    \
   BayWatch.res                        \
   BayesNode.obj                       \
   BayesNet.obj                        \
   BayesChance.obj                     \
	BayWatch.obj                        \
   BWEditNode.obj                      \
   BWEditNet.obj                       \
   BWNEdit.obj                         \

BayWatch.exe :                         \
%ifdef CG32
   c:\bc5\lib\cg32.lib                 \
%endif
   $(BuildPrefix)Bayes.lib             \
   $(BuildPrefix)Common.lib            \
   $(BuildPrefix)console.lib           \
   $(BuildPrefix)Core.lib              \
   $(BuildPrefix)dnet.lib              \
   $(BuildPrefix)gfx.lib               \
   $(BuildPrefix)gfxio.lib             \
   $(BuildPrefix)grd.lib               \
   $(BuildPrefix)GW.lib                \
   $(BuildPrefix)itr.lib               \
   $(BuildPrefix)ls.lib                \
   $(BuildPrefix)ml.lib                \
   $(BuildPrefix)Sim.lib               \
   $(BuildPrefix)SimGui.lib            \
   $(BuildPrefix)SimNet.lib            \
   $(BuildPrefix)SimObjects.lib        \
   $(BuildPrefix)Ted.lib               \
   $(BuildPrefix)ts3.lib               \
   $(BuildPrefix)zeditr.lib            \
   \
   $(LIBdest)\$(BuildPrefix)Bayes.lib  \
   BayWatch.res                        \
   BW.obj
   %do %.exe
   %if %defined(PHOENIXDATA)
      @xcopy /D $(PHOENIXDATA)\Editor.vol exe /R /Q > NUL
   %else
      @echo ---------------------------- !Manually copy VolumeEdit.vol to EXE directory
   %endif

#------------------------------------------------------------------------------

copy :
   %echo copying CommonLib files...
   @-xcopy /D inc\*.h $(PHOENIXINC) /R /Q   > NUL
   @-xcopy /D lib\*.lib $(PHOENIXLIB) /R /Q > NUL
   @-xcopy /D $(OBJdest)\*.res $(PHOENIXLIB) /R /Q  > NUL
   %if %defined(PHOENIXCODE)
      @xcopy /D code\*.cpp $(PHOENIXCODE) /R /Q > NUL
   %endif
