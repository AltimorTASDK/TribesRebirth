PHOENIXMAKE ?= ..\makes
%include <$(PHOENIXMAKE)\builtins.mak>

%if $(COMPILER) == "b"
    %ifdef CG32
        CPPFLAGS += -vG
    %endif
%endif

all: Editor.vol data\editor.strings.h
ide: all copy

# if we do a dependency check on the .btag file instead of the .h, then the build
# process will be more efficient since the tag2bin is not run every time

# add *.dts to the following line to inclue shapes
Editor.vol .MAKE .MISER : $(StringTargets) data\*.bmp
   %echo ---------------------------------------- BUILDING VOLUME: $@ 
   @--:vt -sp $@ @<<
   $(.NEWSOURCES,M"*.bmp",W\n)
<<

data\editor.strings.h .MAKE .MISER : data\editor.strings.cs
	@tag2bin -idata -sdata editor.strings.cs

#------------------------------------------------------------------------------
copy :
   %echo copying VolumeEdit files...
	%if %defined(PHOENIXDATA)
		@xcopy /D exe\*.vol $(PHOENIXDATA) /R /Q  > NUL
		@xcopy /D data\*.cs $(PHOENIXDATA) /R /Q  > NUL
	%endif
	@xcopy /D data\*.h $(PHOENIXINC) /R /Q  > NUL
