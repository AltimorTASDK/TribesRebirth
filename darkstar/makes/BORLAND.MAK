#==============================================================================
#
#  Description Borland.mak
#    
#  (c) Copyright 1997, Dynamix Inc.   All rights reserved.
#
#==============================================================================

#==============================================================================
# BORLAND 5.01 compiler flags
#
#  -3             Generate 80386 protected-mode compatible instructions (Default for 32-bit)
#  -4             Generate 80386/80486 protected-mode compatible instructions
#  -5             Generate Pentium instructions
#  -6             Generate Pentium Pro instructions
#  -A             Use only ANSI keywords
#  -a             Align byte (Default: -a- use byte-aligning)
#  -AK            Use only Kernighan and Ritchie keywords
#  -an            Align to "n" where 1=byte, 2=word (16-bit = 2 bytes), 4=Double word (32-bit only, 4 bytes), 8=Quad word (32-bit only, 8 bytes)
#
#  -AT            Use Borland C++ keywords (also -A-)
#  -AU            Use only UNIX V keywords
#  -B             Compile to .ASM (-S), the assemble to .OBJ (command-line compiler only)
#  -b             Make enums always integer-sized (Default: -b- make enums byte-sized when possible)
#  -C             Turn nested comments on (Default: -C- turn nested comments off)
#  -c             Compile to .OBJ, no link (command-line compiler only)
#  -Dname         Define "name" to the null string
#
#  -Dname=string  Define "name" to "string"
#  -d             Merge duplicate strings (Default)
#  -Efilename     Specify assembler
#  -efilename     Specify executable file name
#  -f             Emulate floating point
#  -f-            No floating point
#  -f87           Generate 8087 floating-point code (command-line compiler only)
#
#  -Ff            Create far variables automatically
#  -ff            Fast floating point
#  -fp            Correct Pentium FDIV flaw
#  -gn            Warnings: stop after "n" messages (Default: 255)
#  -H             Generate and use precompiled headers (Default)
#
#  -H=filename    Set the name of the file for precompiled headers
#  -H”xxx”        Stop precompiling after header file xxxx
#  -h             Uses fast huge pointers
#  -Hc            Cache precompiled header
#  -Hu            Use but do not generate precompiled headers
#  -in            Make significant identifier length to be "n" (Default)
#  -Jg            Generate definitions for all template instances and merge duplicates (Default)
#
#  -Jgd           Generate public definitions for all template instances; duplicates result in redefinition errors
#  -Jgx           Generate external references for all template instances
#  -jn            Errors: stop after "n" messages (Default)
#  -K             Default character type unsigned (Default: -K- default character type signed)
#  -k             Turn on standard stack frame (Default)
#  -K2            Allow only two character types (signed and unsigned). Char is treated as signed. Compatibility with Borland C++ 3.1 and earlier.
#
#  -lx            Pass option “x” to linker (command-line compiler only)
#  -M             Create a Map file (command-line compiler only)
#  -mh            Compile using huge memory model
#
#  -N             Check for stack overflow
#  -O             Optimize jumps
#  -ofilename     Compile .OBJ to “filename” (command-line compiler only)
#  -O1            Generate smallest possible code
#
#  -O2            Generate fastest possible code
#  -Oa            Optimize assuming pointer expressions are not aliased on common subexpression evaluation
#  -Ob            Eliminate dead code
#  -Oc            Eliminate duplicate expressions within basic blocks
#  -Od            Disable all optimizations
#  -Oe            Allocate global registers and analyze variable live ranges
#  -Og            Eliminate duplicate expressions within functions
#  -OI            Optimize across function boundaries (Intel compiler only)
#
#  -Oi            Expand common intrinsic functions
#  -Ol            Compact loops
#  -OM            Cache hit optimizations (Intel compiler only)
#  -Om            Move invariant code out of loops
#  -Op            Propagate copies
#  -OS            Pentium instruction scheduling
#  -Ov            Enable loop induction variable and strength reduction
#  -P             Force C++ compile (command-line compiler only)
#
#  -p             Use Pascal calling convention
#  -pc            Use C calling convention (Default: -pc, -p-)
#  -po            Use fastthis calling convention for passing this parameter in registers
#  -pr            Use fastcall calling convention for passing parameters in registers
#  -ps            Use stdcall calling convention (32-bit compiler only)
#  -R             Include browser information in generated .OBJ files
#  -r             Use register variables (Default)
#
#  -rd            Allow only declared register variables to be kept in registers
#  -RT            Enable runtime type information (Default)
#  -S             Compile to assembler (command-line compiler only)
#  -Tx            Specify assembler option “x” (command-line compiler only)
#  -tD            Compile to a DOS .EXE file (same as -tDe) (command-line compiler only)
#  -tDc           Compile to a DOS .COM file (command-line compiler only)
#  -tW            Make the target a Windows .EXE with all functions exportable (Default)
#
#  -tWD           Make the target a Windows .DLL with all functions exportable
#  -tWDE          Make the target a Windows .DLL with explicit functions exportable
#  -tWE           Make the target a Windows .EXE with explicit functions exportable
#  -tWM           Generate a 32-bit multi-threaded target (command-line compiler only)
#  -tWS           Make the target a Windows .EXE that uses smart callbacks (16-bit compiler only)
#  -tWSE          Make the target a Windows .EXE that uses smart callbacks, with explicit functions exportable (16-bit compiler only)
#
#  -Uname         Undefine any previous definitions of "name" (command-line compiler only)
#  -u             Generate underscores (Default)
#  -V             Use smart C++ virtual tables (Default)
#  -v             Turn on source debugging
#  -V0            External C++ virtual tables
#  -V1            Public C++ virtual tables
#
#  -VC            Calling convention mangling compatibility
#  -Vd            for loop variable scoping
#  -Ve            Zero-length empty base classes
#  -VF            MFC compatibility
#  -Vh            Treat “far” classes as “huge”
#  -vi            Control expansion of inline functions
#
#  -Vmd           Use the smallest representation for member pointers
#  -Vmm           Member pointers support multiple inheritance
#  -Vmp           Honor the declared precision for all member pointer types
#  -Vms           Member pointers support single inheritance
#  -Vmv           Member pointers have no restrictions (most general representation) (Default)
#  -Vo            Enable backward compatibility options (command-line compiler only)
#  -Vp            Pass the 'this' parameter to 'pascal' member functions as the first
#
#  -Vs            Local C++ virtual tables
#  -Vv            ‘deep’ virtual basses
#  -WX            Compile to a DOS DPMI .EXE file (command-line compiler only)
#  -w             Display warnings on
#  -w"xxx"        Enable "xxx" warning message (Default)
#  -wamb          Ambiguous operators need parentheses
#
#  -wamp          Superfluous & with function
#  -wasm          Unknown assembler instruction
#  -waus          'identifier' is assigned a value that is never used (Default)
#  -wbbf          Bit fields must be signed or unsigned int
#  -wbei          Initializing 'identifier' with 'identifier' (Default)
#
#  -wbig          Hexadecimal value contains more than three digits (Default)
#  -wccc          Condition is always true OR Condition is always false (Default)
#  -wcln          Constant is long
#  -wcpt          Nonportable pointer comparison (Default)
#  -wdef          Possible use of 'identifier' before definition
#
#  -wdpu          Declare type 'type' prior to use in prototype (Default)
#  -wdup          Redefinition of 'macro' is not identical (Default)
#  -wdsz          Array size for 'delete' ignored (Default)
#  -weas          Assigning ‘type’ to ‘enum’
#  -weff          Code has no effect (Default)
#
#  -wias          Array variable 'identifier' is near (Default)
#  -wext          'identifier' is declared as both external and static (Default)
#  -whch          Handler for '<type1>' Hidden by Previous Handler for '<type2>'
#  -whid          'function1' hides virtual function 'function2' (Default)
#
#  -wibc          Base class 'base1' is inaccessible because also in 'base2' (Default)
#  -will          Ill-formed pragma (Default)
#  -winl          Functions containing reserved words are not expanded inline (Default)
#  -wlin          Temporary used to initialize 'identifier' (Default)
#
#  -wlvc          Temporary used for parameter 'parameter' in call to 'function' (Default)
#  -wmsg          User-defined warnings
#  -wmpc          Conversion to type fails for members of virtual base class base (Default)
#  -wmpd          Maximum precision used for member pointer type <type> (Default)
#  -wnak          Non-ANSI Keyword Used: '<keyword>' (Note: Use of this option is a requirement for ANSI conformance)
#
#  -wnci          The constant member 'identifier' is not initialized (Default)
#  -wnfc          Non-constant function ‘ident’ called for const object
#  -wnod          No declaration for function 'function'
#  -wnst          Use qualified name to access nested type 'type' (Default)
#
#  -wntd          Use '> >' for nested templates instead of '>>' (Default)
#  -wnvf          Non-volatile function <function> called for volatile object (Default)
#  -wobi          Base initialization without a class name is now obsolete (Default)
#  -wobs          'ident' is obsolete
#
#  -wofp          Style of function definition is now obsolete (Default)
#  -wovl          Overload is now unnecessary and obsolete (Default)
#  -wpar          Parameter 'parameter' is never used (Default)
#  -wpch          Cannot create precompiled header: header (Default)
#
#  -wpia          Possibly incorrect assignment (Default)
#  -wpin          Initialization is only partially bracketed
#  -wpre          Overloaded prefix operator 'operator' used as a postfix operator
#  -wpro          Call to function with no prototype (Default)
#
#  -wrch          Unreachable code (Default)
#  -wret          Both return and return of a value used (Default)
#  -wrng          Constant out of range in comparison (Default)
#  -wrpt          Nonportable pointer conversion (Default)
#  -wrvl          Function should return a value (Default)
#
#  -wsig          Conversion may lose significant digits
#  -wstu          Undefined structure 'structure'
#  -wstv          Structure passed by value
#  -wsus          Suspicious pointer conversion (Default)
#  -wucp          Mixing pointers to different 'char' types
#
#  -wuse          'identifier' declared but never used
#  -wvoi          Void functions may not return a value (Default)
#  -wzdi          Division by zero (Default)
#  -X             Disable compiler autodependency output (Default: -X- use compiler autodependency output)
#  -x             Enable exception handling (Default)
#
#  -xc            Enable compatible exception handling
#  -xd            Enable destructor cleanup (Default)
#  -xf            Enable fast exception prologs
#  -xp            Enable exception location information
#  -Y             Generate DOS overlay code (command-line compiler only)
#  -y             Line numbers on
#  -Yo            Overlay the compiled files (command-line compiler only)
#  -Z             Enable register load suppression optimization
#==============================================================================

.AUTODEPEND
.AUTODEPEND_MODE : IG2SEC

BORLAND ?= C:\BC5
TASM    ?= C:\TASM

CCdir       = $(BORLAND)
CC          = bcc32.exe
CPP         = bcc32.exe
ASM         = tasm32.exe
ASMdir      = $(TASM)
RC          = brcc32.exe
LIB         = tlib.exe
LINK        = ilink32.exe

#==============================================================================
# a little error checking...

%if !%exist($(CCdir)\bin\$(CPP))
   %abort 1 Error: Environment var "BORLAND" not set.  Set to directory where bin\$(CPP) is found.  (i.e. C:\BC5)
%endif
%if !%exist($(ASMdir)\$(ASM))
   %abort 1 Error: Environment var "TASM" not set.  Set to directory where $(ASM) is found. (i.e. C:\TASM\BIN)
%endif

ASMFLAGS    = /t /m /ml /z /zi /w-MCP /DBORLAND
%ifdef DYNAMIC
CPPFLAGS    = -5 -O2 -a4 -f- -ff -k- -x- -pr -Jg -v -V -P -tWE -DBORLAND -DSTRICT -DEXPORT= -DWIN32 -D_RTLDLL
%else
CPPFLAGS    = -5 -O2 -a4 -f- -ff -k- -x- -pr -Jg -v -V -P -tWE -DBORLAND -DSTRICT -DEXPORT= -DWIN32
%endif
CCFLAGS     = -5 -a4 -f- -x- -pr -v-         -tWE -DBORLAND -DSTRICT -DEXPORT= -DWIN32
LIBFLAGS    = /C /P128
LINKFLAGS   = -c -w-dup -V4.0 -s -m -v
#LINKFLAGS   = -c -w-dup -V4.0 -v -m -s

dbASMFLAGS  = -DDEBUG
dbCPPFLAGS  = -v -vi- -pc -DDEBUG -x -Od -k
dbCCFLAGS   = -v -vi- -pc -DDEBUG -x
dbLIBFLAGS  = 
dbLINKFLAGS = -v

optASMFLAGS  = 
optCPPFLAGS  = -O2 -vi
optCCFLAGS   = -O2
optLIBFLAGS  = 
optLINKFLAGS =
#optLINKFLAGS = -P -r

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
   @%set PCH = $(.SOURCES, M"*.pch", X)

   @%set SRC = $(makesource, M"*.res", B, >.rc, X)
   @%set TGT = $(makesource, M"*.res")
   %if "$(TGT)" 
      %echo ---------------------------------------- Compiling resources
      %foreach RESFILE in $(SRC)
         @+$(CCdir)\bin\$(RC) @<<
         $(RCCFLAGS) -I$(.PATH.h);$(ARTdir);$(CCdir)\include;$(LIBdir)
         -fo$(RESFILE,B,>.res, X)
         $(RESFILE)
         << $(ErrFile)
      %endfor
   %endif

   @%set SRC=$(makesource, M"*.asm")
   %if "$(SRC)" 
      %echo ---------------------------------------- Compiling asm's
      @%set tasmINC=/i$[s," /i",$[t,;," ",$(INCdir)]]
      %foreach ASMFILE in $(SRC)
         %echo $(ASMFILE)
         -:$(ASMdir)\$(ASM) @<<
$(tasmINC) $(ASMFLAGS) $(ASMFILE), $(.PATH.obj)\$[r,$(ASMFILE)].obj
<< 
      %endfor
   %endif
   
   @%set SRC=$(makesource, M"*.cpp")
   %if "$(SRC)" 
      %echo ---------------------------------------- Compiling cpp's
      @+$(CCdir)\bin\$(CPP) @<<
$(CPPFLAGS) -I$(.PATH.h) -Ti$(.PATH.h) -n$(OBJdest) -c -H=$(PCH) -Hu $(SRC)
<< $(ErrFile)
   %endif

   @%set SRC = $(makesource, M"*.c")
   %if "$(SRC)" 
      %echo ---------------------------------------- Compiling c's
      @+$(CCdir)\bin\$(CPP) @<<
$(CFLAGS) -I$(.PATH.h) -Ti$(.PATH.h) -n$(OBJdest) -c -H=$(PCH) -Hu $(SRC)
<< $(ErrFile)
   %endif

#==============================================================================

%.exe :
   %echo ---------------------------------------- Building: $(.TARGET, B, >.EXE)
   %do %buildSources
   %ifdef CONSOLE
      @%set LINK_TYPE=-Tpe -ap c0x32.obj
   %else
      @%set LINK_TYPE=-Tpe -aa c0w32.obj
   %endif

   %ifdef DYNAMIC
      @%set LIB_TYPE=cw32mti.lib
   %else
      @%set LIB_TYPE=cw32mt.lib
   %endif

   %echo ---------------------------------------- Linking: $(.TARGET, B, >.EXE)
   +$(CCdir)\bin\$(LINK) @<<
$(LINKFLAGS) $(LINK_TYPE)+
$(.SOURCES, M"*.obj",W+\n)  
$(.target)
$(.target, B, <$(MAPdest)\\, >.map)
$(.SOURCES, M"*.lib") import32.lib $(LIB_TYPE) 
$(.SOURCES, M"*.def")
$(.SOURCES, M"*.res")
<< $(ErrFile)
   @%set makesource= 

#==============================================================================

%.dll :
   %echo ---------------------------------------- Building: $(.TARGET, B, >.DLL)
   @%set $(CPPFLAGS) += -tWDE
   %do %buildSources
   @+$(CCdir)\bin\$(LINK) @<<
-Tpd $(LINKFLAGS) -aa c0d32.obj+
$(.SOURCES, M"*.obj", W+\n)
$(.target)
$(.target, B, <$(MAPdest)\\, >.map)
$(.SOURCES, M"*.lib") import32.lib cw32mti.lib
$(.SOURCES, M"*.def")
$(.SOURCES, M"*.res")
<< $(ErrFile)
   @%set makesource= 

#==============================================================================

%.lib :
   %if !%null(.NEWSOURCES)
      %echo ---------------------------------------- Building: $(.TARGET, B, >.LIB)
      %do %buildSources
      @+$(CCdir)\bin\$(LIB) @<<
$(LIBFLAGS) $(.TARGET) &
-+$(.NEWSOURCES, M"*.obj", W &\n-+)
<< $(ErrFile)
   %endif
   @%set makesource= 


#==============================================================================

%.pch :
   @%set SRC=$(.SOURCES, M"*.obj", B, >.cpp, X)
   %if "$(SRC)" 
      %echo ---------------------------------------- Building: $(.TARGET, B, >.PCH)
      %if %exists($(.TARGET))
         @del $(.TARGET)
      %endif
      @+$(CCdir)\bin\$(CPP) @<<
$(CPPFLAGS) -P -I$(.PATH.h) -Ti$(.PATH.h) -n$(OBJdest) -c -H=$(.TARGET) -H $(SRC)
<< $(ErrFile)
   %endif
   @%set makesource= 


