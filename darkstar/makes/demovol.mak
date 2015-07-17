Ü#==============================================================================
#    
#  MakeFile.mak for EarthSiege III Project
#    
#  (c) Copyright 1996, Dynamix Inc.   All rights reserved.
#
#==============================================================================

LIBdest    =.
VOLdest    =.
PhoenixObj =.
DATdest    =8Bit
DATsrc     =24Bit

Name       = Demo

HazeColor   = 114,78,131
HazePercent = 90

ShadeColor  = 0,0,0
ShadePercent= 90

HazeLevels  = 0
ShadeLevels = 0

DitherTolerance = 6

ChooseRange     = 10,245
RenderRange     = 10,245
MipRenderRange  = 10,245


BitmapList =                    \
        $(DATsrc)\*.bmp   		\


#PUT MIPPED DML FILES HERE!
MippedResources =               \
		.\lush.dml

NonMipResources =               \

OtherResources =                \
    $(Datdest)\$(Name).ppl      \

#==========================================================================
   
$(Name):                        \
   $(VOLdest)\$(Name).vol       \

   %echo ---------------------------------------- Done with $(Name)

$(VOLdest)\$(Name).vol :        \
   $(OtherResources)            \
   $(MippedResources)           \
   $(NonMipResources)           \

   %echo ---------------------------------------- BUILDING VOLUME: $@ 
   %echo DMLS: $(NonMipResources,M"*.dml")
   @-del /Q $@
   @--:vt -sp -r($(MipRenderRange)) $@ @<<
   $(MippedResources,M"*.dml",W\n,<-mip\#9\#$(DATdest)\$(Name).ppl -dml\#$(DATdest) )
   $(MippedResources,N"*.dml",W\n,<-mip\#9\#$(DATdest)\$(Name).ppl )
   $(NonMipResources,M"*.dml",W\n,<-dml\#$(DATdest) )
   $(NonMipResources,N"*.dml",W\n)
   $(OtherResources,N"*.dml",W\n)
<<

$(DATdest)\$(Name).ppl : $(BitmapList,(%)= )         
	%do Make8bit

#==========================================================================

Make8bit :
   %if !%exists($(DATdest))
      %echo ---------------------------------------- Making Directory $(DATdest)
      @-md $(DATdest)
   %endif
   %echo ---------------------------------------- Scanning 24-bit Artwork 

   # This first call quantizes only the terrain, in the range
   # specified by each world

   @pica @<<
      OUTPATH($(DATdest))
	  WEIGHTEDRGBDISTANCE
	  DITHERTOLERANCE($(DitherTolerance))
      NOZEROCOLOR
      PALOUTPUTNAME($(DATdest)\$(Name).pal)
      HAZELEVELS($(HazeLevels))
      SHADELEVELS($(ShadeLevels))
      CHOOSE($(ChooseRange))
	  RENDER($(RenderRange))
      SHADE($(ShadeColor), $(ShadePercent))
      HAZE($(HazeColor),$(HazePercent))
      $(BitmapList,W\n)
   <<

   %echo ---------------------------------------- Generating Palette $(DATdest)\$(Name).ppl
   @palmake $(DATdest)\$(Name).pal .\$(Name).pmp -o $(DATdest)\$(Name).ppl -haze ($(HazeColor))
#--------------------------------------------------------------------------

Complete:               \
   ReScan               \
   $(World)             \
