##############################################################################

PHOENIXMAKE ?= ..\makes
%include <$(PHOENIXMAKE)\builtins.mak>

#%set PRECOMPILED_HEADER=

.PATH.exe = .

%ifdef CG32
CPPFLAGS += -vG
%endif

all: $(LIBdest)\$(DEBUG)$(COMPILER)itr.lib $(LIBdest)\$(DEBUG)$(COMPILER)zeditr.lib
tools: zedBuild.exe zedLight.exe zedShape.exe

##############################################################################
# The library gets the runtime modules
#
LOBJ =  itrgeometry.obj          \
        itrinstance.obj          \
        itrlighting.obj          \
        itrrender.obj            \
        itrcollision.obj         \
        itrmetrics.obj           \
        itrbit.obj               \
        itrshape.obj             \
		polyBSPClip.obj          \
        tplane.obj 

$(LIBdest)\$(DEBUG)$(COMPILER)itr.lib: $(LOBJ)

##############################################################################
#  extra library that zed requires
#	
ZEDOBJ = itrshapeedit.obj		\
		 itrbasiclighting.obj	\
		 zedperslight.obj		\
		 itrmatrix.obj

$(LIBdest)\$(DEBUG)$(COMPILER)zeditr.lib: $(ZEDOBJ)

##############################################################################
#
#
BLIBS = $(BuildPrefix)core.lib   \
        $(BuildPrefix)gfxio.lib  \
        $(BuildPrefix)ml.lib

BOBJS = itrbuild.obj             \
        itr3dmimport.obj         \
        itrbsp.obj               \
        itrportal.obj            \
        itrgeometry.obj          \
        itrshape.obj             \
        itrshapeedit.obj         \
        tplane.obj               \
        itrbit.obj               \
%ifdef CG32
        cg32.lib                 \
%endif
        generic.def

zedBuild.exe: $(BOBJS) $(BLIBS)
   %do %.exe CONSOLE=true
   %if %exist($(PhoenixTool))
      @-xcopy /Q $(.TARGET) $(PhoenixTool)
   %endif
   

##############################################################################
#
#
LLIBS = $(BuildPrefix)core.lib    \
        $(BuildPrefix)console.lib \
        $(BuildPrefix)gfxio.lib   \
        $(BuildPrefix)ml.lib      \
        $(BuildPrefix)ts3.lib

LOBJS = itrlight.obj             \
        itrgeometry.obj          \
        itrlighting.obj          \
        itrbasiclighting.obj     \
        itrcollision.obj         \
        itrmatrix.obj            \
        itrbit.obj               \
        itrshape.obj             \
        itrshapeedit.obj         \
        tplane.obj               \
        zedPersLight.obj         \
        polyBSPClip.obj         \
%ifdef CG32
        cg32.lib                 \
%endif
        generic.def

zedLight.exe: $(LOBJS) $(LLIBS)
    %do %.exe CONSOLE=true
    %if %exist($(PhoenixTool))
       @-xcopy /Q $(.TARGET) $(PhoenixTool)
    %endif


##############################################################################
#

SLIBS = $(BuildPrefix)core.lib   \
        $(BuildPrefix)gfxio.lib  \
        $(BuildPrefix)ml.lib     \
        $(BuildPrefix)ts3.lib

SOBJS = zedSParse.obj            \
        itrshape.obj             \
        itrshapeedit.obj         \
%ifdef CG32
        cg32.lib
%endif


zedShape.exe: $(SOBJS) $(SLIBS)
   %do %.exe CONSOLE=true
   %if %exist($(PhoenixTool))
      @-xcopy /Q $(.TARGET) $(PhoenixTool)
   %endif

##############################################################################
#
#
copy :
   %echo copying Interior files...
   @xcopy /D inc\*.h $(PHOENIXINC)   /R /Q  > NUL
   %if %defined(PHOENIXCODE)
      @xcopy /D code $(PHOENIXCODE)  /R /Q  > NUL
   %endif
   @--xcopy /D lib\*.lib $(PHOENIXLIB) /R /Q  > NUL
   %if "$(DEBUG)" && $(COMPILER) == "m"
      %echo copying Microsoft debug information ...
      @xcopy /D *.pdb $(PHOENIXLIB) /R /Q > NUL
   %endif

