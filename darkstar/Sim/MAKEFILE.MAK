PHOENIXMAKE ?= ..\makes
%include <$(PHOENIXMAKE)\builtins.mak>

%if $(COMPILER) == "b"
    %ifdef CG32
        CPPFLAGS += -vG
    %endif
%endif

#   $(BuildPrefix)sim.pch     \

%ifdef NOSS
all:  $(LIBdest)\$(DEBUG)$(COMPILER)SimNet.lib $(LIBdest)\$(DEBUG)$(COMPILER)Sim.lib  
%else
all:  $(LIBdest)\$(DEBUG)$(COMPILER)SimNet.lib $(LIBdest)\$(DEBUG)$(COMPILER)Sim.lib  $(LIBdest)\$(DEBUG)$(COMPILER)SSSim.lib  
%endif

#------------------------------------------------------------------------------
$(LIBdest)\$(DEBUG)$(COMPILER)SimNet.lib: \
   netCSDelegate.obj             \
   netPacketStream.obj           \
   netEventManager.obj           \
   netGhostManager.obj           \
   simTelnetPlugin.obj           \
   netTemplates.obj

$(LIBdest)\$(DEBUG)$(COMPILER)SSSim.lib: \
   simDynamicDataPlugin.obj      \
   simCollideable.obj            \
   simPersman.obj                \
   simPing.obj                   \
   simPrefs.obj                  


$(LIBdest)\$(DEBUG)$(COMPILER)Sim.lib:      \
   simTerrain.obj                \
   simCollisionImages.obj		 \
   simTreeView.obj               \
   simToolWin.obj                \
   winTools.obj                  \
   simContainer.obj              \
   simCoordDb.obj                \
   simPalette.obj                \
   simBase.obj                   \
   simDictionary.obj             \
   simGuiConsolePlugin.obj       \
   simEv.obj                     \
   simLightGrp.obj               \
   simCollision.obj              \
   simMovement.obj               \
   simTimerGrp.obj               \
   simCanvas.obj                 \
   simVolume.obj                 \
   simResource.obj               \
   simAction.obj                 \
   simNetObject.obj              \
   simWorld.obj                  \
   simGame.obj                   \
   simInput.obj                  \
   simInputDevice.obj            \
   simTemplates.obj              \
   simTagDictionary.obj          \
   simSetIterator.obj            \
   simConsoleEvent.obj           \
   simConsolePlugin.obj          \
   inspect.obj                   \
   inspectWindow.obj             \
   simDynamicLight.obj           \
   simPreLoadManager.obj         
   %if $(COMPILER) == "b"
   %do %.lib LIBFLAGS="/C /P512"
   %else
   %do %.lib
   %endif

#------------------------------------------------------------------------------
$(BuildPrefix)sim.pch:      \
   pch.obj                  \


#------------------------------------------------------------------------------
copy :
   %echo copying Sim files...
   @xcopy /D inc\*.h $(PHOENIXINC) /R /Q  > NUL
   @--xcopy /D lib\*.lib $(PHOENIXLIB) /R /Q  > NUL
   %if %defined(PHOENIXCODE)
      @xcopy /D code\*.cpp $(PHOENIXCODE) /R /Q  > NUL
   %endif
   %if "$(DEBUG)" && $(COMPILER) == "m"
      %echo copying Microsoft debug information ...
      @xcopy /D *.pdb $(PHOENIXLIB) /R /Q > NUL
   %endif
