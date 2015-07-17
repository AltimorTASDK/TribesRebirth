PHOENIXMAKE ?= ..\..\makes
%include <$(PHOENIXMAKE)\builtins.mak>

#CPPFLAGS += -Jg
%ifdef CG32
CPPFLAGS += -vG
%endif

inc\esf.strings.h .MAKE .MISER : data\esf.strings.cs
	@tag2bin -iinc -sdata esf.strings.cs

#------------------------------------------------------------------------------
copy :
   %echo copying Common Volume files...
   @xcopy /D data\*.cs $(PHOENIXDATA) /R /Q  > NUL
   @copy inc\esf.strings.h $(PHOENIXINC)
