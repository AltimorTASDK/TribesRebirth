###############################################################################
# Aliases
###############################################################################
alias speed1        "set DirectionalVelocity  0.01; set PositiveRotation  0.2; set NegativeRotation  -0.2"
alias speed2        "set DirectionalVelocity  0.05; set PositiveRotation  0.5; set NegativeRotation  -0.5"
alias speed3        "set DirectionalVelocity  0.10; set PositiveRotation  1.0; set NegativeRotation  -1.0"
alias speed4        "set DirectionalVelocity  0.20; set PositiveRotation  2.0; set NegativeRotation  -2.0"
alias speed5        "set DirectionalVelocity  0.50; set PositiveRotation  3.0; set NegativeRotation  -3.0"
alias speed6        "set DirectionalVelocity  1.00; set PositiveRotation  4.0; set NegativeRotation  -4.0"
alias speed7        "set DirectionalVelocity  4.00; set PositiveRotation  6.0; set NegativeRotation  -6.0"
alias speed8        "set DirectionalVelocity 10.00; set PositiveRotation  8.0; set NegativeRotation  -8.0"
alias speed9        "set DirectionalVelocity 15.00; set PositiveRotation 15.0; set NegativeRotation -15.0"
alias speed0        "set DirectionalVelocity 20.00; set PositiveRotation 30.0; set NegativeRotation -30.0"
alias close         "echo"
alias action        "postAction '$ActionTarget'"
alias nomouse       "inputDeactivate mouse0"
alias setGlide      "setFullScreenDevice MainWindow Glide"
alias show          "set ConsoleWorld::Echo"
alias showGfx        show "'$ConsoleWorld::FrameRate' P:'$GFXMetrics::EmittedPolys','$GFXMetrics::RenderedPolys' C:'$GFXMetrics::RecachedPolys' E:'$GFXMetrics::UsedEdges' SE:'$GFXMetrics::SharedEdges' B:'$GFXMetrics::BackfacedPolys' S:'$GFXMetrics::UsedSpans' Pix: '$GFXMetrics::pixelsRendered'"
alias showTerrain    show "S:'$GridMetrics::VisibleSquares' V:'$GridMetrics::TransformedVertices' P:'$GridMetrics::RenderedPolys'"
alias setDT         "set SimTerrain::DetailTable"
alias setVD         "set SimTerrain::VisibleDistance"
alias setPD         "set SimTerrain::PerspectiveDistance"
alias setHD         "set SimTerrain::HazeDistance"
alias dtNear        "setDT 750 1500 3000 3500 6000 7000 10000 12000 18000 20000 30000 50000 50000 300000"
alias visNear       "dtNear ; setPD 1000 ; setVD 10000 ; setHD 5000"
alias visMid        "dtNear ; setPD 1000 ; setVD 25000 ; setHD 15000"
alias visFar        "dtNear ; setPD 1000 ; setVD 150000 ; setHD 60000"

###############################################################################
# Bindings
###############################################################################
bind make  o        postAction showCam MoveBackward '$DirectionalVelocity'
bind break o        postAction showCam MoveBackward 0
bind make  p        postAction showCam MoveForward '$DirectionalVelocity'
bind break p        postAction showCam MoveForward 0'
bind make  s        action MoveLeft '$DirectionalVelocity'
bind break s        action MoveLeft 0'
bind make  f        action MoveRight '$DirectionalVelocity'
bind break f        action MoveRight 0'
bind make  d        action MoveBackward '$DirectionalVelocity'
bind break d        action MoveBackward 0
bind make  e        action MoveForward '$DirectionalVelocity'
bind break e        action MoveForward 0'
bind make  r        action MoveUp '$DirectionalVelocity'
bind break r        action MoveUp 0'
bind make  v        action MoveDown '$DirectionalVelocity'
bind break v        action MoveDown 0'
bind make  j        action ViewYaw '$PositiveRotation'
bind break j        action ViewYaw 0'
bind make  l        action ViewYaw '$NegativeRotation'
bind break l        action ViewYaw 0'
bind make  i        action ViewPitch '$NegativeRotation'
bind break i        action ViewPitch 0'
bind make  k        action ViewPitch '$PositiveRotation'
bind break k        action ViewPitch 0'
bind make  y        action ViewRoll '$NegativeRotation'
bind break y        action ViewRoll 0'
bind make  u        action ViewRoll '$PositiveRotation'
bind break u        action ViewRoll 0'
bind make  tab      next
bind make  t        light
bind make  ctrl-q   quit
bind make  x        detach
bind make  1        speed1
bind make  2        speed2
bind make  3        speed3
bind make  4        speed4
bind make  5        speed5
bind make  6        speed6
bind make  7        speed7
bind make  8        speed8
bind make  9        speed9
bind make  0        speed0
bind make F9        setGlide
bind      F1        viewshape
bind      F2        shapeDialog 1
bind      F3        "hideToolWin terrainBar; showToolWin terrainBar"
bind      F4        loadpal
bind      F6        messageCanvasDevice MainWindow outline
bind      F5        perfchk
bind      F7        impseq1
bind      F8        camDialog
bind      F10       "deleteObject sky; not GFX::DrawOutline"
bind      F11       alias action postAction showCam
bind      F12       alias action postAction '$ActionTarget'

###############################################################################
# Macros
###############################################################################
define viewshape
    openFile Show::diskFile "DTS shape files" *.DTS
    if test $dlgResult != [cancel]
        view $Show::diskFile
    endif
enddef

define saveshapeas
    saveFileAs Show::diskFile "DTS shape files" *.DTS
    if test $dlgResult != [cancel]
        saveShape $Show::diskFile
    endif
enddef

define loadpal
    openFile Show::diskFile "Palette files" *.PPL
    if test $dlgResult != [cancel]
        newObject "" SimPalette $Show::diskFile
    endif
enddef

define imptrans
    openFile Show::diskFile "Shape files" *.DTS
    if test $dlgResult != [cancel]
        importTransitions $Show::diskFile
    endif
enddef

define impseq1
    openFile Show::diskFile "Shape files" *.DTS
    if test $dlgResult != [cancel]
        importSequence $Show::diskFile 0
    endif
enddef

define impseq
    openFile Show::diskFile "Shape files" *.DTS
    if test $dlgResult != [cancel]
        editBox "Import which sequence?" "Enter sequence #" "Show::seqNum"
        importSequence $Show::diskFile $Show::seqNum
    endif
enddef

define terr
    deleteObject Terrain
    cls

    setcat simVol     $1 .sim.vol
    setcat tedVol     $1 .ted.vol
    setcat terrainDtf $1 .dtf

    newObject terrainVol SimVolume $simVol
    newObject tedVolume  SimVolume $tedVol

    newObject Terrain SimTerrain Load $terrainDtf -24576 -24576 -250 0 0 0
    newObject sky SimSky
enddef

define perfchk
    editBox "Performance check" "Enter distance, count pairs:" "Perf::params"
    performanceCheck $Perf::params
enddef

define server
    newServer
    focusServer
    newCanvas ServerWindow "ServerWindow" 320 200 1
    newCamera ServerCamera ServerWindow -1
enddef

define lightoff
    deleteObject light1
    deleteObject light2
    deleteObject light3
    deleteObject light4
    deleteObject light5
    deleteObject light6
enddef

define lighton
    newObject "lite1" SimLight Directional 1 1 1 1 0 0
    newObject "lite2" SimLight Directional 1 1 1 -1 0 0
    newObject "lite3" SimLight Directional 1 1 1 0 1 0
    newObject "lite4" SimLight Directional 1 1 1 0 -1 0
    newObject "lite5" SimLight Directional 1 1 1 0 0 1
    newObject "lite6" SimLight Directional 1 1 1 0 0 -1
enddef

define line
    set Coll::timeCheck 1
    show '$Coll::collide' '$Coll::hit.x' '$Coll::hit.y' '$Coll::hit.z' '$Coll::time1' '$Coll::collisionList' '$Coll::name'
enddef

define tree
    if focusServer
	    focusClient
	    simTreeCreate ServerTree MainWindow server
	    simTreeAddSet ServerTree manager
    endif

    simTreeCreate ClientTree MainWindow
    simTreeAddSet ClientTree manager
enddef

###############################################################################
# Startup Instructions
###############################################################################

# Create the main window
newObject MainWindow MyCanvas "Show Tool" 640 480 t 1\n
GuiNewContentCtrl MainWindow SimGui::TSControl

setMainWindow MainWindow
loadMainMenu

# Set the default search path for finding loadable files
echo
set ConsoleWorld::DefaultSearchPath ".;.\art;..\art"

# Load in default GFXFont
set Console::GFXFont Console.PFT

activate 0
set AllowMultipleShapes FALSE
newObject defaultPalette SimPalette Show.PPL
newToolWindow terrainBar MainWindow
addToolGap terrainBar gap1
addToolButton terrainBar MarsTerrain button.bmp 
setButtonHelp terrainBar MarsTerrain "Mars Day"
setToolCommand terrainBar MarsTerrain "terr Mars"
addToolButton terrainBar IceTerrain button.bmp
setButtonHelp terrainBar IceTerrain "Ice Day"
setToolCommand terrainBar IceTerrain "terr Ice"
addToolGap terrainBar gap2
setToolWinPos terrainBar 600 600
set Perf::params "10 10"
speed3

