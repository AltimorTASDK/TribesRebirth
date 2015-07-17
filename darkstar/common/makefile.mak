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
LIBS =               \
      AI             \
      volumeEdit     \
      volumeCommon   \
      sim            

#------------------------------------------------------------------------------
all: $(LIBS)

$(LIBS) .MAKE .ALWAYS .MISER:
   %if %exist($(.TARGET))
   	@%chdir $(.TARGET)
   	@make $(MFLAGS)
   	@make $(MFLAGS) copy
   	@%chdir $(MAKEDIR)
   %endif
copy: