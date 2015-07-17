PHOENIXMAKE ?= ..\makes
%include <$(PHOENIXMAKE)\builtins.mak>

%if $(COMPILER) == "b"
    %ifdef CG32
        CPPFLAGS += -vG
    %endif
%endif


#------------------------------------------------------------------------------
copy :
   %echo copying DirectX headers...
   @xcopy /D inc\*.h $(PHOENIXINC) /R /Q  > NUL
