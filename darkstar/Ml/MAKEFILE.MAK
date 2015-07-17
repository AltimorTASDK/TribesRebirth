#같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같
#같   
#같 $Workfile:   makefile.mak  $
#같 $Version$
#같 $Revision:   1.10  $
#같   
#같 DESCRIPTION:
#같   
#같 (c) Copyright 1995, Dynamix Inc.   All rights reserved.
#같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같


PHOENIXMAKE ?= ..\makes
%include <$(PHOENIXMAKE)\builtins.mak>

#같같같같같같같같같같같같같같같같같같같같같같� LIBRARIES

MATH_OBJS = m_atan.obj      \
            m_atrig.obj     \
            m_base.obj      \
            m_bscan.obj     \
            m_coll.obj      \
            m_collision.obj \
            m_dot.obj       \
            m_div.obj       \
            m_euler.obj     \
            m_fix.obj       \
            m_fpuCW.obj     \
            m_log.obj       \
            m_map.obj       \
            m_matf.obj      \
            m_mdiv.obj      \
            m_mul.obj       \
            m_mulf.obj      \
            m_plane.obj     \
            m_quat.obj      \
            m_quatf.obj     \
            m_random.obj    \
            m_reduce.obj    \
            m_sqrt.obj      \
            m_trig.obj      \
            template.obj


#------------------------------------------------------------------------------
%ifdef CG32
CPPFLAGS += -vG
%endif

$(LIBdest)\$(DEBUG)$(COMPILER)ml.lib:   \
   $(MATH_OBJS)


fpu_test.exe:  \
   $(LIBdest)\$(DEBUG)$(COMPILER)ml.lib   \
   $(DEBUG)$(COMPILER)gfx.lib   \
   fpu_test.obj
   %do %.exe CONSOLE=YES

quat_test.exe:  \
   $(LIBdest)\$(DEBUG)$(COMPILER)ml.lib   \
   $(DEBUG)$(COMPILER)gfx.lib   \
   quat_test.obj
   %do %.exe CONSOLE=YES



#------------------------------------------------------------------------------
all:
   %foreach COMPILER in b m
   @:make -nologo -a COMPILER=$(COMPILER)
   @:make -nologo -a COMPILER=$(COMPILER) OPT=
   %endfor

#------------------------------------------------------------------------------
copy :
   %echo copying Math files...
   @xcopy /D inc     $(PHOENIXINC) /R /Q /I  > NUL
   @--xcopy /D lib\*.lib    $(PHOENIXLIB) /R /Q /I > NUL
   %if %defined(PHOENIXCODE)
      @xcopy /D code      $(PHOENIXCODE) /R /Q /I > NUL
   %endif
   %if "$(DEBUG)" && $(COMPILER) == "m"
      %echo copying Microsoft debug information ...
      @xcopy /D *.pdb $(PHOENIXLIB) /R /Q > NUL
   %endif

