PHOENIXMAKE ?= ..\makes
%include <$(PHOENIXMAKE)\builtins.mak>

%if $(COMPILER) == "b"
CPPFLAGS += -Jg
%endif

$(LIBdest)\$(DEBUG)$(COMPILER)Ted.lib:    \
         simTed.obj                       \
         ted.res                          \
         tedPlugin.obj                    \
         tedwindow.obj

ted.res: res\ted.rc
	@cd res
	@brc32 -r $(.SOURCE)
	@cd ..

copy :
   %echo copying Ted files...
   @xcopy /D inc\*.h $(PHOENIXINC) /R /Q  > NUL
   @--xcopy /D lib\*.lib $(PHOENIXLIB) /R /Q  > NUL
   @xcopy /D res\*.res $(PHOENIXLIB) /R /Q  > NUL
   %if %defined(PHOENIXCODE)
      @xcopy /D code\*.cpp $(PHOENIXCODE) /R /Q  > NUL
   %endif
   %if %defined(PHOENIXDATA)
      @xcopy /D data\*.*    $(PHOENIXDATA) /R /Q  > NUL
   %endif
   %if "$(DEBUG)" && $(COMPILER) == "m"
      %echo copying Microsoft debug information ...
      @xcopy /D *.pdb $(PHOENIXLIB) /R /Q > NUL
   %endif
