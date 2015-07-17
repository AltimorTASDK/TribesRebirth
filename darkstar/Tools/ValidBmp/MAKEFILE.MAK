##############################################################################

PHOENIXMAKE ?= ..\makes
%include <$(PHOENIXMAKE)\builtins.mak>

#
LIBS = $(BuildPrefix)core.lib   \
        $(BuildPrefix)gfxio.lib  \
        $(BuildPrefix)ml.lib

OBJS = ValidBmp.obj             \

%ifdef CG32
        cg32.lib                 \
%endif

ValidBmp.exe: $(OBJS) $(LIBS)
   %do %.exe CONSOLE=true
   %if %exist($(PhoenixTool))
      @xcopy /Q $(.TARGET) $(PhoenixTool)
   %endif

