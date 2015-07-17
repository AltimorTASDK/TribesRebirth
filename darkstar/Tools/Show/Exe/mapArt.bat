@echo off 
echo .
echo 	This tool remaps the 24 bit art contained within a subdirectory 
echo		called '24bit', and puts the remapped bitmaps and new palette
echo 		into a directory called 'DTS'.  Two script files are invoked 
echo		in this process:  artPass1.pca and artPass2.pca.  You need
echo		to have the tools mpMerger and picaNew for this art 
echo		conversion to work.  The .MAX shapes which use the 24bit art 
echo		should be exported to the DTS directory.  The new palette is
echo		called 'showTool.ppl'.  
echo .
echo     Executing conversion:
echo .
picaNew @artPass1.pca 
picaNew @artPass1B.pca 
mpMerger @artPass2.pca
