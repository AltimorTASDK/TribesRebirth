PHOENIXMAKE ?= ..\makes
%include <$(PHOENIXMAKE)\builtins.mak>

%set PRECOMPILED_HEADER=

%ifdef CG32
CPPFLAGS += -vG
%endif

$(LIBdest)\$(BuildPrefix)ls.lib:    \
   LS_Terrain.obj                   \
   LS_Cmplx.obj                     \
   LS_Exec.obj                      \
   LS_Fill.obj                      \
   LS_Fltr.obj                      \
   LS_IO.obj                        \
   LS_Math.obj                      \
   LS_Stack.obj                     \
   LS_Block.obj                     \
   LSMapper.obj                     \
   LSEditor.obj                     \
   LSPlugin.obj                     \
   LSDlg.res                        \

#   LSTerrainStamp.obj               \
#   LSStampInst.obj                  \
 
#==============================================================================

LSTest.exe :                        \
   $(LIBdest)\$(BuildPrefix)LS.lib  \
   $(BuildPrefix)Common.lib         \
   $(BuildPrefix)Core.lib           \
   $(BuildPrefix)dnet.lib           \
   $(BuildPrefix)gfx.lib            \
   $(BuildPrefix)gfxio.lib          \
   $(BuildPrefix)grd.lib            \
   $(BuildPrefix)GW.lib             \
   $(BuildPrefix)itr.lib            \
   $(BuildPrefix)ml.lib             \
   $(BuildPrefix)Sim.lib            \
   $(BuildPrefix)SimGui.lib         \
   $(BuildPrefix)SimNet.lib         \
   $(BuildPrefix)SimObjects.lib     \
   $(BuildPrefix)Ted.lib            \
   $(BuildPrefix)ts3.lib            \
   \
   LSDlg.res                        \
   LS_Test.obj                      \

#==============================================================================

copy :
   %echo copying Landscape files...
   @xcopy /D inc\*.h   $(PHOENIXINC) /R /Q  > NUL
   %if %defined(PHOENIXCODE)
      @xcopy /D code\*.cpp $(PHOENIXCODE) /R /Q  > NUL
   %endif
   @--xcopy /D lib\*.lib $(PHOENIXLIB) /R /Q  > NUL
   @--xcopy /D $(OBJdest)\*.res $(PHOENIXLIB) /R /Q  > NUL
   %if "$(DEBUG)" && $(COMPILER) == "m"
      %echo copying Microsoft debug information ...
      @xcopy /D *.pdb $(PHOENIXLIB) /R /Q > NUL
   %endif
