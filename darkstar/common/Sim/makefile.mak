PHOENIXMAKE ?= ..\..\makes
%include <$(PHOENIXMAKE)\builtins.mak>


#CPPFLAGS += -Jg
%ifdef CG32
CPPFLAGS += -vG
%endif


%ifdef NOSS
all:  $(LIBdest)\$(DEBUG)$(COMPILER)Common.lib
%else
all:  $(LIBdest)\$(DEBUG)$(COMPILER)Common.lib  $(LIBdest)\$(DEBUG)$(COMPILER)SSCommon.lib
%endif

$(LIBdest)\$(DEBUG)$(COMPILER)SSCommon.lib:      \
   SimDamageEv.obj               \
   simTrigger.obj                \
   #simTriggerPhysical.obj        \
   #simTriggerParser.obj          \
   esfTrail.obj                  \
   esfPlugin.obj                 \
   simMarker.obj                 \
   simDropPoint.obj              \
   simSoundSource.obj            \
	simMagnet.obj						\
   simShape.obj                  \
   simShapePlugin.obj            \
   simShapeGroup.obj             \
   simShapeGroupRep.obj          \
   simMovingShape.obj            \
   simInteriorShape.obj          \
   simTSShape.obj                \
   shadowRenderImage.obj         \
   esfObjectPersTags.obj         
   %do %.lib LIBFLAGS="/C /P512"

#------------------------------------------------------------------------------
$(LIBdest)\$(DEBUG)$(COMPILER)Common.lib:      \
   simVolumetric.obj			 \
   \
   baseShadowRenderImage.obj         \
   ircClient.obj                 \
   commonPersTags.obj            \
   ircPlugin.obj              

#------------------------------------------------------------------------------
copy :
   %echo copying CommonLib files...
   @xcopy /D inc\*.h $(PHOENIXINC) /R /Q   > NUL
   %if %defined(PHOENIXCODE)
      @xcopy /D code\*.cpp $(PHOENIXCODE) /R /Q > NUL
   %endif
   @--xcopy /D lib\*.lib $(PHOENIXLIB) /R /Q > NUL
