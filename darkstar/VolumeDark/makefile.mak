PHOENIXMAKE ?= ..\makes
%include <$(PHOENIXMAKE)\builtins.mak>

%if $(COMPILER) == "b"
    %ifdef CG32
        CPPFLAGS += -vG
    %endif
%endif

all: Darkstar.vol data\darkstar.strings.h
ide: all copy

# if we do a dependency check on the .btag file instead of the .h, then the build
# process will be more efficient since the tag2bin is not run every time

# add "*.dts" to the dependencies on the next line to wrap up shapes, as well
Darkstar.vol .MAKE .MISER : $(StringTargets)
   %echo ---------------------------------------- BUILDING VOLUME: $@ 
   @--:vt -sp $@ @<<
data\darkscroll.pba
data\darkbox.pba
data\METreeView.pba
data\METreeViewSel.pba
data\MEFont.pft
data\MEFontHL.pft
data\darkfont.pft
data\darkpal.pal
data\darkbmp.bmp
<<

data\darkstar.strings.h .MAKE .MISER : data\darkstar.strings.cs
	@tag2bin -idata -sdata darkstar.strings.cs

#------------------------------------------------------------------------------
copy :
   %echo copying VolumeDark files...
	%if %defined(PHOENIXDATA)
	   @xcopy /D exe\*.vol $(PHOENIXDATA) /R /Q  > NUL
	   @xcopy /D data\*.cs $(PHOENIXDATA) /R /Q  > NUL
	%endif
	@xcopy /D data\*.h $(PHOENIXINC) /R /Q  > NUL
