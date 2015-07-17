###############################################################################
#
#
###############################################################################
#
#
#SCRIPTS = \
#		server.cs \
#		netConnect.cs \
#      default.client.cs \
#      default.server.cs \
#      sae.cs \
#      GUI.cs \
#
#
#EDITOR = \
#      EditMission.cs \
#      newFearMission.cs \
#      restart.cs \
#      trees.cs \
#		tree.cs \
#		building.cs \
#		item.cs \
#		registerobjects.cs \
#		loadShow.cs \
#		camview.cs \
#		move.cs \
#      ted.cs \
#		med.cs
# 
#.PATH.vol: ..
#
#
##############################################################################
#scripts.vol: $(SCRIPTS) $(EDITOR) $(BMPS)
#	del /F $(.TARGET)
#	%foreach SFILE in $(.SOURCES)
#		--vt $(.TARGET) $(SFILE)
#	%endfor
##############################################################################
.PATH.vol: ..
SCRIPTS = *.cs

scripts.vol: $(SCRIPTS)
	del /F $(.TARGET)
	%foreach SFILE in $(.SOURCES)
		@--vt -sp $(.TARGET) $(SFILE)
	%endfor
