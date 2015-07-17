INCdir = inc;$(PhoenixInc)

PHOENIXMAKE ?= ..\makes
%include <$(PHOENIXMAKE)\builtins.mak>

%ifdef CG32
CPPFLAGS += -vG
%endif

%if $(COMPILER) == "b"
CPPFLAGS += -Jgx- -Jg
%endif

# the windows include file tapi.h requires WIN32 to be defined to true, rather
# than NULL
CPPFlags += -UWIN32 -DWIN32="1"

.PATH.exe = .

#session.obj \
#transport.obj \
#vc.obj \
DNET_OBJS = DNet.obj       \
            UDPNet.obj     \
            VCProtocol.obj \
            LOOPNet.obj \
            ipxnet.obj     \
            logfile.obj    \

#
#            NullNet.obj    \
#            TAPINet.obj    \
#            commcode.obj   \
#            commthrd.obj   \
#            crcmodel.obj   \
#            crctable.obj   \
#            crctest.obj    \
#            servcmd.obj    \
#            tapicode.obj   \


$(LIBdest)\$(DEBUG)$(COMPILER)dnet.lib: \
   %ifdef CG32
   $(BORLAND)\lib\cg32.lib                \
   %endif
   $(DNET_OBJS)

all: \
   $(LIBdest)\$(DEBUG)$(COMPILER)dnet.lib \
   tserver.exe \
   tclient.exe \

tserver.exe: \
   tserver.obj \
   $(BuildPrefix)core.lib \
   dbdnet.lib
   %do %.exe CONSOLE=true

tclient.exe: \
   dbdnet.lib \
   $(BuildPrefix)core.lib \
   tclient.obj 
   %do %.exe CONSOLE=true


copy :
   %echo copying DNet files...
   @-xcopy /D inc\*.h       $(PHOENIXINC) /R /Q  > NUL
   @--xcopy /D lib\*.dll     $(PHOENIXLIB) /R /Q  > NUL
   %if %defined(PHOENIXCODE)
      @xcopy /D code $(PHOENIXCODE) /R /Q  > NUL
   %endif
   @--xcopy /D lib\*.lib     $(PHOENIXLIB) /R /Q  > NUL
   @--xcopy /D lib\*.tds     $(PHOENIXLIB) /R /Q  > NUL
   %if "$(DEBUG)" && $(COMPILER) == "m"
      %echo copying Microsoft debug information ...
      @xcopy /D *.pdb $(PHOENIXLIB) /R /Q > NUL
   %endif
