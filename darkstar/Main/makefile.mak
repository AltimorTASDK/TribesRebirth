##############################################################################
PHOENIXMAKE ?= ..\makes
%include <$(PHOENIXMAKE)\builtins.mak>

LIBDEST=.
DATdir=exe

#%set PRECOMPILED_HEADER=

#CPPFLAGS += -Jg -vi-

Test:  Strings copyPhoenixDat copyRenditionUC $(BuildPrefix)test.exe 

Strings:  main.strings.ttag            \
          main.strings.h

##############################################################################
#
LIBS =   \
      $(BuildPrefix)sim.lib            \
      $(BuildPrefix)SimObjects.lib     \
      $(BuildPrefix)SimGui.lib         \
      $(BuildPrefix)SimNet.lib         \
      $(BuildPrefix)itr.lib            \
      $(BuildPrefix)grd.lib            \
      $(BuildPrefix)gw.lib             \
      $(BuildPrefix)core.lib           \
      $(BuildPrefix)dnet.lib           \
      $(BuildPrefix)ls.lib             \
      $(BuildPrefix)ted.lib            \
      $(BuildPrefix)ts3.lib            \
      $(BuildPrefix)gfxio.lib          \
      $(BuildPrefix)gfx.lib            \
      $(BuildPrefix)ml.lib             \
      $(BuildPrefix)console.lib        \
      $(BuildPrefix)common.lib         
      


##############################################################################
# lsDlg.res needed for the lsDlgPlugin

MAIN_OBJ =        main.obj                \
                  generic.def             \
                  lsDlg.res

EXAMPLE_PLUGIN =  StartDelegate.obj       \
                  mainplayermanager.obj   \
                  maincsdelegate.obj      \
                  mainplayer.obj 

##############################################################################
#
ALL_OBJ =   $(EXAMPLE_PLUGIN) \
            $(MAIN_OBJ)

$(BuildPrefix)test.exe: $(ALL_OBJ) $(LIBS)

#------------------------------------------------------------------------------
main.strings.h  .MAKE .MISER : main.strings.ttag
   @tag2bin -oexe -iinc -sexe main.strings.ttag

#------------------------------------------------------------------------------
copyPhoenixDat:
   %if %defined(PHOENIXDATA)
      xcopy /D $(PHOENIXDATA)\*.cs   exe /R /Q
      xcopy /D $(PHOENIXDATA)\*.vol  exe /R /Q
   %endif
 
copyRenditionUC:
   %if %defined(PHOENIXDATA)
      xcopy /D $(PHOENIXDATA)\*.uc exe /R /Q
   %endif


