##############################################################################
PHOENIXMAKE ?= ..\makes
%include <$(PHOENIXMAKE)\builtins.mak>

#%set PRECOMPILED_HEADER=

.PATH.exe = .


##############################################################################
#
LOBJ =   grdBlock.obj         \
         grdCollision.obj     \
         grdEdgeTable.obj     \
         grdFile.obj          \
         grdHeight.obj        \
         grdHRLM.obj          \
         grdMipMake.obj       \
         grdRender.obj        \
         grdTemplate.obj      \
         mipBlit.obj          \
         viewcone.obj

$(LIBdest)\$(BuildPrefix)grd.lib: $(LOBJ)

copy :
   %echo copying Terrain files...
   @xcopy /D inc\*.h $(PHOENIXINC) /R /Q  > NUL
   @--xcopy /D lib\*.lib $(PHOENIXLIB) /R /Q  > NUL
   %if %defined(PHOENIXCODE)
      @xcopy /D code $(PHOENIXCODE) /R /Q  > NUL
   %endif
   %if "$(DEBUG)" && $(COMPILER) == "m"
      %echo copying Microsoft debug information ...
      @xcopy /D *.pdb $(PHOENIXLIB) /R /Q > NUL
   %endif

