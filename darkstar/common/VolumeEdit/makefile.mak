PHOENIXMAKE ?= ..\..\makes
%include <$(PHOENIXMAKE)\builtins.mak>

#CPPFLAGS += -Jg
%ifdef CG32
CPPFLAGS += -vG
%endif

   data\commonEditor.strings.btag

ShapeSources =    \
   arrow25.dts    \
   arrow5_r.dts   \
   arrow5_g.dts   \
   arrow5_b.dts   \
   arrow5_y.dts   \
   ARROW50.dts    \
   cube2.dts      \
   cube4.dts      \
   cube8.dts      \
   pyrm2.dts      \
   pyrm4.dts      \
   pyrm8.dts

all: $(PHOENIXDATA)\Editor.vol inc\commonEditor.strings.h

$(PHOENIXDATA)\Editor.vol .MISER .ALWAYS : $(ShapeSources)
   %echo ---------------------------------------- BUILDING VOLUME: $@ 
   @--:vt -sp $@ @<<
$(ShapeSources,W\n)
<<

inc\commonEditor.strings.h .MAKE .MISER : data\commonEditor.strings.cs
	@tag2bin -iinc -sdata commonEditor.strings.cs

#------------------------------------------------------------------------------
copy :
   %echo copying Common Edit files...
   @xcopy /D data\*.cs $(PHOENIXDATA) /R /Q  > NUL
   @copy inc\*.h $(PHOENIXINC) > NUL
   
