PHOENIXMAKE ?= ..\makes
%include <$(PHOENIXMAKE)\builtins.mak>

$(LIBdest)\$(DEBUG)$(COMPILER)console.lib:   \
   tagDictionary.obj \
   gram.obj    \
   console.obj \
   consoleInternal.obj \
   eval.obj    \
   scan.obj    

all:     \
   parser   \
   console.lib \
   test.exe

parser:
   lex -a -p CMD -o code\scan.cpp code\scan.l
   yacc -p CMD -D inc\gram.h -o code\gram.cpp code\gram.y



test.exe: \
   dbgfxio.lib \
   dbcore.lib \
   dbml.lib \
   $(LIBdest)\$(DEBUG)$(COMPILER)console.lib \
   test.obj
   %do %.exe CONSOLE=true

#------------------------------------------------------------------------------
copy :
   %echo copying Console files...
   @xcopy /D inc\*.h $(PHOENIXINC) /R /Q  > NUL
   %if %defined(PHOENIXCODE)
      @xcopy /D code\*.cpp $(PHOENIXCODE) /R /Q  > NUL
   %endif
   @--xcopy /D lib\*.lib $(PHOENIXLIB) /R /Q  > NUL
   %if "$(DEBUG)" && $(COMPILER) == "m"
      %echo copying Microsoft debug information ...
      @xcopy /D *.pdb $(PHOENIXLIB) /R /Q > NUL
   %endif
