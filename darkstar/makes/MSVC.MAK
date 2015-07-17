#==============================================================================
#
#  Description MSVC.mak
#    
#  (c) Copyright 1997, Dynamix Inc.   All rights reserved.
#
#==============================================================================

#==============================================================================
#               Microsoft C/C++ Compiler Command-Line Options
# /O1 minimize space                       /Op[-] improve floating-pt consistency
# /O2 maximize speed                       /Os favor code space
# /Oa assume no aliasing                   /Ot favor code speed
# /Ob<n> inline expansion (default n=0)    /Ow assume cross-function aliasing
# /Od disable optimizations (default)      /Ox maximum opts. (/Ogityb1 /Gs)
# /Og enable global optimization           /Oy[-] enable frame pointer omission
# /Oi enable intrinsic functions
# /G3 optimize for 80386                   /Ge enable stack checking calls
# /G4 optimize for 80486                   /Gs[num] disable stack checking calls
# /G5 optimize for Pentium                 /Gh enable hook function call
# /G6 optimize for Pentium Pro             /GR[-] enable C++ RTTI
# /GB optimize for blended model (default) /GX[-] enable C++ EH (same as /EHsc)
# /Gd __cdecl calling convention           /Gi[-] enable incremental compilation
# /Gr __fastcall calling convention        /Gm[-] enable minimal rebuild
# /Gz __stdcall calling convention         /EHs enable synchronous C++ EH
# /GA optimize for Windows Application     /EHa enable asynchronous C++ EH
# /GD optimize for Windows DLL             /EHc extern "C" defaults to nothrow
# /Gf enable string pooling                /QIfdiv[-] enable Pentium FDIV fix 
# /GF enable read-only string pooling      /QI0f[-] enable Pentium 0x0f fix
# /Gy separate functions for linker
# /Fa[file] name assembly listing file     /Fo<file> name object file
# /FA[sc] configure assembly listing       /Fp<file> name precompiled header file
# /Fd[file] name .PDB file                 /Fr[file] name source browser file
# /Fe<file> name executable file           /FR[file] name extended .SBR file
# /Fm[file] name map file
# /C don't strip comments                  /FI<file> name forced include file
# /D<name>{=|#}<text> define macro         /U<name> remove predefined macro
# /E preprocess to stdout                  /u remove all predefined macros
# /EP preprocess to stdout, no #line       /I<dir> add to include search path
# /P preprocess to file                    /X ignore "standard places"
# /vd{0|1} disable/enable vtordisp         /Za disable extensions (implies /Op)
# /vm<x> type of pointers to members       /Ze enable extensions (default)
# /Zi generate debugging information       /Zg generate function prototypes
# /Z7 generate old-style debug info        /Zl omit default library name in .OBJ
# /Zd line number debugging info only      /Zs syntax check only
# /Zp[n] pack structs on n-byte boundary   /noBool disable "bool" keyword
# /?, /help print this help message        /V<string> set version string
# /c compile only, no link                 /w disable all warnings
# /H<num> max external name length         /W<n> set warning level (default n=1)
# /J default char type is unsigned         /WX treat warnings as errors
# /nologo suppress copyright message       /Yc[file] create .PCH file
# /Tc<source file> compile file as .c      /Yd put debug info in every .OBJ
# /Tp<source file> compile file as .cpp    /Yu[file] use .PCH file
# /TC compile all files as .c              /YX[file] automatic .PCH
# /TP compile all files as .cpp            /Zm<n> max memory alloc (% of default)
# /MD link with MSVCRT.LIB                 /MDd link with MSVCRTD.LIB debug lib
# /ML link with LIBC.LIB                   /MLd link with LIBCD.LIB debug lib
# /MT link with LIBCMT.LIB                 /MTd link with LIBCMTD.LIB debug lib
# /LD Create .DLL                          /F<num> set stack size
# /LDd Create .DLL debug libary            /link [linker options and libraries]
#==============================================================================
 
.AUTODEPEND
.AUTODEPEND_MODE : IG2SEC
 
MSVC ?= C:\MSDEV
MASM ?= C:\MASM611

CCdir       = $(MSVC)
ASMdir      = $(MASM)
CC          = cl.exe
CPP         = cl.exe
ASM         = ml.exe
RC          = rc.exe
LIB         = lib.exe
LINK        = link.exe

#==============================================================================
# a little error checking...

%if !%exist($(CCdir)\bin\$(CPP))
   %abort 1 Error: Environment var "MSVC" not set.  Set to directory where bin\$(CPP) is found.  (i.e. C:\MSDEV)
%endif
%if !%exist($(ASMdir)\bin\$(ASM))
   %abort 1 Error: Environment var "MASM" not set.  Set to directory where bin\$(ASM) is found. (i.e. C:\MASM611)
%endif

MSVCLIBdir = $(MCCdir)\lib

ASMFLAGS       = /nologo /Cp /DMSVC /coff
CPPFLAGS       = /nologo /c /Zp4 /GR /W3 /DMSVC /DWIN32 /DEXPORT= /DSTRICT /D_USERENTRY=__cdecl
CCFLAGS        = /nologo /c /Zp4 /GR /W3 /DMSVC /DWIN32 /DEXPORT= /DSTRICT /D_USERENTRY=__cdecl
LINKFLAGS      = /nologo
LIBFLAGS       = /nologo

dbASMFLAGS     = /DDEBUG
dbCPPFLAGS     = /Od /Gd /Zi /DDEBUG /D_DEBUG /MDd
dbCCFLAGS      = /Od /Gd /Zi /DDEBUG /D_DEBUG /MDd
dbLINKFLAGS    = /DEBUGTYPE:CV /DEBUG
dbLIBFLAGS     = /DEBUGTYPE:CV

optASMFLAGS    = 
optCPPFLAGS    = /Gr /Ow /G5 /Ob1 /Og /Oi /Ot /Gs /MD 
optCCFLAGS     = /Gr /Ow /G5 /Ob1 /Og /Oi /Ot /Gs /MD 
optLINKFLAGS   = 
optLIBFLAGS    = 

dboptASMFLAGS  = /DDEBUG
dboptCPPFLAGS  = /Gr /G5 /Od /Zi /DDEBUG /D_DEBUG /MDd
dboptCCFLAGS   = /Gr /G5 /Od /Zi /DDEBUG /D_DEBUG /MDd
dboptLINKFLAGS = /DEBUGTYPE:CV /DEBUG
dboptLIBFLAGS  = /DEBUGTYPE:CV

#==============================================================================

%.obj : %.cpp
   @%set makesource+= $(.TARGET) $(.SOURCE)

%.obj : %.c
   @%set makesource+= $(.TARGET) $(.SOURCE)

%.obj : %.asm
   @%set makesource+= $(.TARGET) $(.SOURCE)

%.res : %.rc
   @%set makesource+= $(.TARGET) $(.SOURCE)

%.def :
   @%set makesource+= $(.TARGET)

#==============================================================================

%buildSources:
   @%set PCH=
   %if "$(PRECOMPILED_HEADER)"
      @%set SRC = $(PRECOMPILED_HEADER, X)
      @%set TGT = $(PRECOMPILED_HEADER, <$(DEBUG)$(COMPILER), B, >.pch, X)
      %if %exists($(SRC))
         @%set PCH = $(TGT)
         %if "$(TGT)" && %time($(SRC)) > %time($(TGT))
            %echo ---------------------------------------- Building Pre-Compiled Header $(TGT)
            %if %exist($(TGT))
               @del $(TGT)
            %endif
            @+$(CCdir)\bin\$(CPP) @<<
/I$[s," /I",$[t,;," ",$(.PATH.h)]] $(CPPFLAGS) -Yc -Fp$(LIBdest)\$(TGT) /Fo$(OBJdest)\ $(SRC)
<< $(ErrFile)
         %endif
      %endif
   %endif

   @%set SRC = $(makesource, M"*.res", B, >.rc, X)
   @%set TGT = $(makesource, M"*.res")
   %if "$(TGT)" 
      %echo ---------------------------------------- Compiling resources
      @+rc.exe /x /fo$(TGT) /i$[s," /i",$[t,;," ",$(.Path.h)]] /i$(ARTdir) /i$(CCdir)\include;$(LIBdest) /i$(MSVC)\MFC\include $(SRC) $(ErrFile)
   %endif
   
   @%set SRC=$(makesource, M"*.asm")
   %if "$(SRC)" 
      %echo ---------------------------------------- Compiling asm's
      @%set masmINC=/I$[s," /I",$[t,;," ",$(.PATH.h)]]
      @:$(ASMdir)\bin\$(ASM) @<<
/c /D__MASM__ $(ASMFLAGS) $(masmINC) $(SRC) 
<<
      @copy *.obj $(OBJdest) > NUL
      
   %endif

   @%set SRC=$(makesource, M"*.cpp")
   %if "$(SRC)" 
      %echo ---------------------------------------- Compiling cpp's

      %if "$(DEBUG)"
          @%set PDB=/Fd$(.TARGET, B, >.PDB)
      %else
          @%set PDB=
      %endif

      @+$(CCdir)\bin\$(CPP) @<<
$(CPPFLAGS) /I$[s," /I",$[t,;," ",$(.PATH.h)]] /Fo$(OBJdest)\ $(SRC,W\n) $(PDB)
<< $(ErrFile)
   %endif

   @%set SRC = $(makesource, M"*.c")
   %if "$(SRC)" 
      %echo ---------------------------------------- Compiling c's

      %if "$(DEBUG)"
          @%set PDB=/Fd$(.TARGET, B, >.PDB)
      %else
          @%set PDB=
      %endif

      @+$(CCdir)\bin\$(CPP) @<<
$(CCFLAGS) /I$(.PATH.h)\ /Fo$(OBJdest)\ $(SRC)
<< $(ErrFile)
   %endif

#==============================================================================

%.exe :
   %echo ---------------------------------------- Building: $(.TARGET, B, >.EXE)
   %do %buildSources
   %ifdef CONSOLE
      @%set LINK_TYPE=/SUBSYSTEM:CONSOLE
   %else
      @%set LINK_TYPE=/SUBSYSTEM:WINDOWS
   %endif
   %echo ---------------------------------------- Linking: $(.TARGET, B, >.EXE)

   %if "$(DEBUG)"
       @%set PDB=/PDB:$(.TARGET, B, >.PDB)
   %else
       @%set PDB=
   %endif

   @+$(CCdir)\bin\link @<<
$(LINKFLAGS) $(LINK_TYPE) $(PDB)
/out:$(.target)
/opt:REF
/libpath:$(CCdir)\lib
$[s,"\n",$[m,*.obj,$(.SOURCES)]]
$[s,"\n",$[m,*.lib,$(.SOURCES)]]
$(.SOURCES, M"*.res")
kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib comctl32.lib shell32.lib advapi32.lib uuid.lib wsock32.lib rpcrt4.lib vfw32.lib winmm.lib ole32.lib dsound.lib
<<
   @%set makesource= 

#==============================================================================

%.dll :
   %echo ---------------------------------------- Building: $(.TARGET, B, >.DLL)
   %do %buildSources

   %if "$(DEBUG)"
       @%set PDB=/PDB:$(.TARGET, B, >.PDB)
   %else
       @%set PDB=
   %endif

   @+$(CCdir)\bin\link @<<
$(LINKFLAGS) 
/out:$(.target)
/dll
/opt:REF
/libpath:$(CCdir)\lib $(PDB)
$[s,"\n",$[m,*.obj,$(.SOURCES)]]
$(.SOURCES, M"*.lib") 
$(.SOURCES, M"*.def")
$(.SOURCES, M"*.res")
kernel32.lib user32.lib gdi32.lib 
<<
   @%set makesource= 

#==============================================================================

%.lib :
   %if !%null(.NEWSOURCES)
      %echo ---------------------------------------- Building: $(.TARGET, B, >.LIB)
      %do %buildSources
      @+$(CCdir)\bin\$(LIB) @<<
$(LIBFLAGS) /OUT:$(.TARGET)
$[s,"\n",$[m,*.obj,$(.SOURCES)]]
<< $(ErrFile)
   %endif
   @%set makesource= 
