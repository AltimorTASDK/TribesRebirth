PHOENIXMAKE ?= ..\makes
%include <$(PHOENIXMAKE)\builtins.mak>

%if $(COMPILER) == "b"
    %ifdef CG32
        CPPFLAGS += -vG
    %endif
%endif

%set PRECOMPILED_HEADER=

$(LIBdest)\$(DEBUG)$(COMPILER)GW.lib:           \
            gwBase.obj                          \
            gwButtonProxy.obj                   \
            gwCanvas.obj                        \
            gwComboBox.obj                      \
            gwControlProxy.obj                  \
            gwDeviceManager.obj                 \
            gwDialog.obj                        \
            gwEditBox.obj                       \
            gwEditProxy.obj                     \
            gwListBoxProxy.obj                  \
            gwLog.obj                           \
            gwMain.obj                          \
            gwMenu.obj                          \
            gwMsg.obj                           \
            gwTool.obj                          \
            gwTreeView.obj                      \

example.exe:                              \
   $(DEBUG)$(COMPILER)core.lib            \
   $(LIBdest)\$(DEBUG)$(COMPILER)GW.lib   \
   example.res                            \
   example.obj                   

whello.exe:                               \
   $(DEBUG)$(COMPILER)core.lib            \
   $(LIBdest)\$(DEBUG)$(COMPILER)GW.lib   \
   whello.obj

copy :
   %echo copying Window files...
   @xcopy /D inc\*.h $(PHOENIXINC) /R /Q  > NUL
   @--xcopy /D lib\*.lib $(PHOENIXLIB) /R /Q  > NUL
   %if %defined(PHOENIXCODE)
      @xcopy /D code\*.cpp $(PHOENIXCODE) /R /Q  > NUL
   %endif
   %if "$(DEBUG)" && $(COMPILER) == "m"
      %echo copying Microsoft debug information ...
      @xcopy /D *.pdb $(PHOENIXLIB) /R /Q > NUL
   %endif
