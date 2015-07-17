newClient
focusClient
newObject MainWindow SimGui::Canvas ConsoleExample 640 480 t 1 1\n
inputActivate

#
#
#
set ConsoleWorld::DefaultSearchPath ".;..\art"

# Load in default GFXFont
set Console::GFXFont console.pft

# Default MoveObject to move the camera around
# Only works with example plugin

set ActionTarget "camera"

newObject commonVol SimVolume common.vol
newObject darkVol SimVolume   darkstar.vol
newObject editVol SimVolume   editor.vol
newObject lushVol SimVolume   lush.vol
newObject entVol SimVolume   entities.vol
newObject LushTedVol SimVolume lush.ted

loadShow
setCursor MainWindow arrow.bmp
cursorOn MainWindow
alias winMouse "inputDeactivate mouse0; windowsMouseEnable MainWindow"
alias dirMouse "inputActivate mouse0; windowsMouseDisable MainWindow"
alias editGui "GuiInspect MainWindow;GuiToolbar MainWindow"

bind make ctrl-o "messageCanvasDevice MainWindow outline"
bind make  ctrl-p "messageCanvasDevice MainWindow outline"
bind break ctrl-p "messageCanvasDevice MainWindow outline"

winMouse
inputActivate keyboard0
inputActivate mouse0

newObject CSDelegate MainCSDelegate 0 false
echo

loadTagDictionary editor.strings.ttag
loadTagDictionary darkstar.strings.ttag
loadTagDictionary main.strings.ttag

alias tsView    "cursorOff MainWindow;GuiNewContentCtrl MainWindow SimGui::TSControl"
alias gotoStart "cursorOn MainWindow;disconnect;GuiLoadContentCtrl MainWindow Start.gui"

bind make Escape gotoStart

# Note, use your own AUTOEXEC.CS to set custom port:
set pref::Address1 IP:198.74.38.73:26002
set pref::Master1 IP:198.74.38.73:26002

if isFile autoexec.cs
	autoexec.cs
endif

gotoStart
move 100

