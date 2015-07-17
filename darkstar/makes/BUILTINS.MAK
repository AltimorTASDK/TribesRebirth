#==============================================================================
#    
#  Description Builtins.mak
#    
#  (c) Copyright 1997, Dynamix Inc.   All rights reserved.
#
#==============================================================================

# Environment Variables Explained:
#  PhoenixInc   - where compilers look for Phoenix library header files
#  PhoenixLib   - where linkers look for Phoenix library files
#  PhoenixMake  - directory where Builtins.mak, Borland.mak, MSVC.mak are found
#  PhoenixCode  - if specified, library code will be copied here on Make copy
#  PhoenixObj   - base directory for .obj files.  Default is .\obj
#                 Setting environ variable OBJdest overrides placement of all .obj's
# Others:
#  PhoenixError - error file.  If set error outputs to this file.  Default is console
#  COMPILER     - set to b, m, or w, for Borland, MSVC++, or Watcom compilation
#  BORLAND      - set to base Borland directory.  (Default C:\BC5)
#  TASM         - set to tasm32.exe directory.  (i.e. C:\TASM\BIN)
#  MSVC         - set to base MSVC directory.  (Default C:\MSDEV)
#  MASM         - set to masm.exe directory.  (i.e. C:\MASM611)

   %set defPhoenixInc  = C:\Phoenix\Inc
   %set defPhoenixLib  = C:\Phoenix\Lib
   %set defPhoenixMake = C:\Phoenix\Makes
   %set defPhoenixCode = C:\Phoenix\Code
   %set defPhoenixData = C:\Phoenix\Data
   %set defPhoenixObj  = obj
   %set defPhoenixLibDest = lib

   %if "$(PhoenixObj)"
      %set PhoenixObj=$(PhoenixObj)\$(MAKEDIR, B)
   %endif

   %foreach var in PhoenixInc PhoenixLib PhoenixMake PhoenixCode PhoenixObj PhoenixData PhoenixLibDest
      %if !"$($(var))"
         %set $(var) = $(def$(var))
      %endif
   %endfor

#==============================================================================

#disable default targets
.NODEFAULT :

# Determine which compiler builtins to load
%set COMPILER_INCLUDE=
%if "$(COMPILER)" == "b"
   %set COMPILER_INCLUDE=borland.mak
%elseif "$(COMPILER)" == "m"
   %set COMPILER_INCLUDE=msvc.mak
%elseif "$(COMPILER)" == "w"
   %set COMPILER_INCLUDE=watcom.mak
%else
   %abort 1 Error: Environment var "COMPILER" not set.  Use b, m, or w - aborting.
%endif

%if %exist($(PHOENIXMAKE)\$(COMPILER_INCLUDE))
   %include $(PHOENIXMAKE)\$(COMPILER_INCLUDE)
%else
   %abort 1 Error: Unable find $(PHOENIXMAKE)\$(COMPILER_INCLUDE) - aborting.
%endif

#==============================================================================
# Patch up debug stuff  (DO NOT MODIFY INDENTING!!!  must exec on read pass.)

%if !"$(OPT)"
%set DEBUG=d
%if !"$(DEBUGNOOPT)"
%if "$(COMPILER)" == "m"
%foreach var CPPFLAGS CCFLAGS ASMFLAGS LIBFLAGS LINKFLAGS
   %set $(var) += $(dbopt$(var))
%endfor
%else
%foreach var CPPFLAGS CCFLAGS ASMFLAGS LIBFLAGS LINKFLAGS
   %set $(var) += $(db$(var))
   %set $(var) += $(opt$(var))
%endfor
%endif
%set MODIFIER = Debug Build
%else
%foreach var CPPFLAGS CCFLAGS ASMFLAGS LIBFLAGS LINKFLAGS
   %set $(var) += $(db$(var))
%endfor
%set MODIFIER = Debug Build (Opt off)
%endif
%else
%foreach var CPPFLAGS CCFLAGS ASMFLAGS LIBFLAGS LINKFLAGS
   %set $(var) += $(opt$(var))
%endfor
%set DEBUG=r
%set MODIFIER = Release Build
%endif

%if "$(FIRSTTARGET)" == "Copying"
%set MODIFIER = COPYING
%endif

BuildPrefix = $(DEBUG)$(COMPILER)

%if "$(PhoenixError)"
   %set ErrFile=>> $(PhoenixError)
%endif

#==============================================================================
# Source Directories

SRCdir      ?= code
INCdir      ?= inc;$(PhoenixInc)
LIBdir      ?= lib;$(PhoenixLib)
ARTdir      ?= Art
SFXdir      ?= SFX
DATdir      ?= data

#Destination Directories
OBJdest     ?= $(PhoenixObj)\$(BuildPrefix)obj
LIBdest     ?= $(PhoenixLibDest)
EXEdest     ?= exe
DLLdest     ?= $(EXEdest)
VOLdest     ?= $(EXEdest)
MAPdest		?= $(EXEdest)
DATdest     ?= $(OBJdest)

.PATH.c     = $(SRCdir)
.PATH.cpp   = $(SRCdir)
.PATH.asm   = $(SRCdir)
.PATH.h     = $(INCdir);$(LIBdest)
.PATH.inc   = $(INCdir);$(LIBdest)
.PATH.rc    = $(SRCdir)
.PATH.res   = $(OBJdest);$(LIBdir)
.PATH.def   = $(SRCdir)
.PATH.obj   = $(OBJdest)
.PATH.lib   = $(LIBdest);$(LIBdir)
.PATH.pch   = $(LIBdest)
.PATH.exe   = $(EXEdest)
.PATH.dll   = $(DLLdest)
.PATH.vol   = $(VOLdest)
.PATH.dat   = $(DATdest)
.PATH.ttag  = $(DATdir)
.PATH.btag  = $(DATdest)

#==============================================================================
# Things done before and after execution

.BEFORE:
   %foreach var in $(PhoenixInc) $(PhoenixLib) $(PhoenixCode) $(PhoenixObj) $(PhoenixData) $(OBJdest) $(LIBdest) $(EXEdest) $(DLLdest) $(MAPdest)
      %if !%exists($(var))
         %echo ---------------------------------------- Making Directory $(var)
         @-md $(var)
      %endif
   %endfor
   @%set makesource= 
   %if %exists($(PhoenixError))
      @-del $(PhoenixError)
   %endif

.AFTER:
   %if "$(Project)" == ""
      @%set Project = $(FIRSTTARGET)
   %endif
	%if $(MAKESTATUS) == 0
      %if ("$(Project)" != "copy" && "$(Project)" != "all")
         %echo ======================= $(MODIFIER), No Errors, Target: $(Project) 
      %endif
	%else
      %echo ======================= $(MODIFIER), Errors in $(PhoenixError), Target: $(Project) 
	%endif

#enable default targets
.DEFAULT :
