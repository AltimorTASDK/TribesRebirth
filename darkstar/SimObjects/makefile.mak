PHOENIXMAKE ?= ..\makes
%include <$(PHOENIXMAKE)\builtins.mak>

%if $(COMPILER) == "b"
    %ifdef CG32
        CPPFLAGS += -vG
        %echo CODEGUARD COMPILE
    %endif
%endif

%ifdef NOSS
all: $(LIBdest)\$(DEBUG)$(COMPILER)SimObjects.lib
%else
all: $(LIBdest)\$(DEBUG)$(COMPILER)SSSimObjects.lib $(LIBdest)\$(DEBUG)$(COMPILER)SimObjects.lib
%endif


$(LIBdest)\$(DEBUG)$(COMPILER)SSSimObjects.lib: \
   preferencePlugin.obj       \
   cdPlayerPlugin.obj         \
   cdPlayerDlg.obj            \
   simMoveobj.obj             \
   simExplosion.obj           \
   simExplosionTable.obj      \
   simExplosionCloud.obj      \
   simDebris.obj              \
   simPartDebris.obj          \
   simSmoke.obj               \
   simFire.obj                \
   \
   simSky.obj                 \
   simStarField.obj           \
   simPlanet.obj              \
   simInterior.obj            \
   simMovingInterior.obj      \
   skyPlugin.obj              \
   interiorPlugin.obj         \
   simInteriorGrouping.obj    \
   MissionPlugin.obj          \
   \
   MissionEditor.obj          \
   SSSimObjectPersTags.obj     
   %if $(COMPILER) == "b"
    %set LIBFLAGS = /C /P512
   %endif
   %do %.lib

$(LIBdest)\$(DEBUG)$(COMPILER)SimObjects.lib: \
   ia3d.obj                   \
   simWinConsolePlugin.obj    \
   shellOpenPlugin.obj        \
   gfxPlugin.obj              \
   movPlayPlugin.obj          \
   netPlugin.obj              \
   redbookPlugin.obj          \
   simInputPlugin.obj         \
   simTreePlugin.obj          \
   soundFXPlugin.obj          \
   toolPlugin.obj             \
   terrainPlugin.obj          \
   \
   redbook.obj                \
   movPlay.obj                \
   \
   move.obj                   \
   \
   fxRenderImage.obj          \
   boxRender.obj              \
   \
   simPathManager.obj         \
   \
   simPath.obj                \
   simLight.obj               \
   \
   MissionSelect.obj          \
   \
   soundFX.obj                \
   soundFXBuffer.obj          \
   simSoundSequence.obj       \
   \
#   simObjectTemplates.obj     \
   simObjectPersTags.obj      \
	simPlaceContainer.obj      \
   \
   toolPlugin.res
   %if $(COMPILER) == "b"
    %set LIBFLAGS = /C /P512
   %endif
   %do %.lib
   
#------------------------------------------------------------------------------
$(BuildPrefix)simObjects.pch:    \
   pch.obj                       \

copy :
   %echo copying SimObjects files...
   @xcopy /D inc\*.h $(PHOENIXINC) /R /Q  > NUL
   @--xcopy /D lib\*.lib $(PHOENIXLIB) /R /Q  > NUL
   @--xcopy /D $(OBJdest)\*.res $(PHOENIXLIB) /R /Q  > NUL
   %if %defined(PHOENIXCODE)
      @xcopy /D code\*.cpp $(PHOENIXCODE) /R /Q  > NUL
   %endif
   %if "$(DEBUG)" && $(COMPILER) == "m"
      %echo copying Microsoft debug information ...
      @xcopy /D *.pdb $(PHOENIXLIB) /R /Q > NUL
   %endif
