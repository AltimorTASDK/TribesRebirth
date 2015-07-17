#------------------------------------------------------------------------------
# Description 
#    
# $Workfile$
# $Revision$
# $Author  $
# $Modtime $
#
#------------------------------------------------------------------------------

# Makes all the libraries in order
#
LIBS =            \
      directx     \
      volumeEdit  \
      volumeDark  \
      core        \
      ml          \
      console     \
      dgfx        \
      ts3         \
      window      \
      dnet        \
      terrain     \
	  terrain2    \
      interior    \
      sim         \
      simGui      \
      simObjects  \
      landscape   \
      ted         \
      common

#------------------------------------------------------------------------------
all: $(LIBS)

$(LIBS) .MAKE .ALWAYS .MISER:
   @%chdir $(.TARGET)
   @make $(MFLAGS)
   -4 @make $(MFLAGS) copy
   @%chdir $(MAKEDIR)

#==========================================================================

DEL_DIRECTORIES =       \
   $(PhoenixInc)        \
   $(PhoenixLib)        \
   $(PhoenixCode)

clean:
   @-del /q /s *.obj
   @-del /q /s *.map
   @-del /q /s *.lck
   @-del /q /s *.rsp
   @-del /q /s *.dat
   @-del /q /s *.exe
   @-del /q /s *.pch
   @-del /q /s *.tr2
   @-del /q /s *.log
   @-del /q /s *.cgl
   @-del /q /s *.res
   @-del /q /s *.lib
   @-del /q /s *.bak
   @-del /q /s *.err
   @-del /q /s *.ilc
   @-del /q /s *.ild
   @-del /q /s *.ilf
   @-del /q /s *.ils
   %foreach var in $(DEL_DIRECTORIES)
      %if %exists($(var))
          @-del /q $(var)
      %endif
   %endfor
