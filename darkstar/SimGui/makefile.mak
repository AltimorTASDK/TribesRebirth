PHOENIXMAKE ?= ..\makes
%include <$(PHOENIXMAKE)\builtins.mak>

%if $(COMPILER) == "b"
    %ifdef CG32
        CPPFLAGS += -vG -Jgx
    %endif
%endif

SimGui:                \
   art\guiToolbar.vol      \
   $(LIBdest)\$(DEBUG)$(COMPILER)SimGui.lib

$(LIBdest)\$(DEBUG)$(COMPILER)SimGui.lib: \
   simGuiAnimateBMA.obj    \
   simGuiCanvas.obj        \
   simGuiTextFormat.obj    \
   simGuiBase.obj          \
   simGuiCtrl.obj          \
   simGuiHelpCtrl.obj      \
   simGuiEditCtrl.obj      \
   simGuiActiveCtrl.obj    \
   simGuiDelegate.obj      \
   simGuiArrayCtrl.obj     \
   simGuiTestCtrl.obj      \
   simGuiTestButton.obj    \
   simGuiTestRadial.obj    \
   simGuiTextEdit.obj      \
   simGuiBitmapCtrl.obj    \
   simGuiPaletteCtrl.obj   \
   simGuiSimpleText.obj    \
   simGuiPlugin.obj        \
   simGuiPersTags.obj      \
   simGuiProgressCtrl.obj  \
   simGuiTSCtrl.obj        \
   simGuiTimerCtrl.obj     \
   simGuiBitmapBox.obj     \
   simGuiScrollCtrl.obj    \
   simGuiMatrixCtrl.obj    \
   simGuiTextList.obj      \
   simGuiTextWrap.obj      \
   simGuiSlider.obj        \
   simGuiComboBox.obj      \
   simGuiTestCheck.obj     \
   chunkedBitmap.obj
   %if $(COMPILER) == "b"
    %set LIBFLAGS = /C /P256
   %endif
   %do %.lib
   
#------------------------------------------------------------------------------
art\guiToolbar.vol:              \
   art\guiTBSetup.cs             \
   art\tb_*.bmp
   %echo ---------------------------------------- BUILDING VOLUME: $@ 
   @-del $@
   @--:vt -sp $@ @<<
   $(.SOURCES,W\n)
<<
          
#------------------------------------------------------------------------------
                          
copy :
   %echo copying SimGui files...
   @xcopy /D inc\*.h $(PHOENIXINC) /R /Q  > NUL
   @--xcopy /D lib\*.lib $(PHOENIXLIB) /R /Q  > NUL
   @xcopy /D art\*.vol $(PHOENIXDATA) /R /Q  > NUL
   %if %defined(PHOENIXCODE)
      @xcopy /D code\*.cpp $(PHOENIXCODE) /R /Q  > NUL
   %endif
   %if "$(DEBUG)" && $(COMPILER) == "m"
      %echo copying Microsoft debug information ...
      @xcopy /D *.pdb $(PHOENIXLIB) /R /Q > NUL
   %endif
