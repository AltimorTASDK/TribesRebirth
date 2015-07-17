##############################################################################
%if !%exists($(PhoenixMake)\Builtins.Mak)
   %abort 1 Error: Environment var "PhoenixMake" not set. Set to Phoenix\Makes directory
   @-md $(var)
%endif
%include <$(PHOENIXMAKE)\builtins.mak>

#%set PRECOMPILED_HEADER=

.PATH.exe = exe

# when using code guard, use -vG
#CPPFLAGS += -Jg -vi-

all: $(BuildPrefix)show.exe


##############################################################################
#
# when using code guard, include this first:  c:\bc5\lib\CG32.LIB \

OBJS = 	\
   showMain.obj                  \
   tripod.obj                    \
   myGuy.obj                     \
   mover.obj                     \
   shapeDialog.obj               \
   transitionDialog.obj          \
   camDialog.obj                 \
   showPlugin.obj                \
   Show.res 


LIBS =	 \
   $(BuildPrefix)Core.lib        \
   $(BuildPrefix)GW.lib          \
   $(BuildPrefix)Sim.lib         \
   $(BuildPrefix)SimObjects.lib  \
   $(BuildPrefix)SSSimObjects.lib  \
   $(BuildPrefix)SimGui.lib      \
   $(BuildPrefix)grd.lib         \
   $(BuildPrefix)ls.lib          \
   $(BuildPrefix)itr.lib         \
   $(BuildPrefix)ts3.lib         \
   $(BuildPrefix)gfx.lib         \
   $(BuildPrefix)gfxio.lib       \
   $(BuildPrefix)ml.lib          \
   $(BuildPrefix)ted.lib         \
   $(BuildPrefix)dNet.lib        \
   $(BuildPrefix)Console.lib     \
   $(BuildPrefix)SimNet.lib 

$(BuildPrefix)show.exe: $(OBJS) $(LIBS) 
    %do %.exe CONSOLE=true 


