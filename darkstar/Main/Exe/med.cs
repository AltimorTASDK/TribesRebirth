#
#
#
# Names of objects

set _Me ME

# Names built by camview script
setcat _MeCanvas $_Me Canvas
setcat _MeCamera  $_Me MoveObj

#
#
move
newObject MEMoveObj SimMoveObject move.sae 1007 1245 115

MissionEditor MainWindow
set MED::camera MEMoveObj

winMouse
# Clear
set _Me
set _MeCanvas
set _MeCamera

