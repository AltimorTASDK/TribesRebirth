newToolWindow config toolbar.bmp 0
# add status bar to force window size
addStatusBar config 200

addToolButton config setcam 0
setButtonHelp config setcam "set camera moveObj name"
        setToolCommand config setcam setcam

addToolButton config settopview 0
setButtonHelp config settopview "set topView canvas name"
        setToolCommand config settopview settopview

addToolGap config g0

addToolButton config setlthick 0
setButtonHelp config setlthick "set line thickness"
        setToolCommand config setlthick setlthick

addToolButton config setselcol 0
setButtonHelp config setselcol "set selection draw color"
        setToolCommand config setselcol setselcol

addToolButton config setselng 0
setButtonHelp config setselng "set selecting draw color"
        setToolCommand config setselng setselngcol

addToolButton config setbrcol 0
setButtonHelp config setbrcol "set brush draw color"
        setToolCommand config setbrcol setbrcol

addToolButton config setpincol 0
setButtonHelp config setpincol "set pin grid color"
        setToolCommand config setpincol setpincol

addToolGap config gap1

addToolButton config setundolim 0 1
setButtonHelp config setundolim "set undo stack limit"
        setToolCommand config setundolim setundolim

addToolButton config setcurrfile 0
setButtonHelp config setcurrfile "set the current Ted file"
        setToolCommand config setcurrfile setcurrfile

addToolButton config setgscale 0 1
setButtonHelp config setgscale "set the default ground scale"
        setToolCommand config setgscale setgscale


# add buttons for defining Action names

addToolButton config closecon 0
setButtonHelp config closecon "close configuration window"
        setToolCommand config closecon "deleteObject config"


