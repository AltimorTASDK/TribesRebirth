#
# Set up a default movement action map file
#
if test "$1" != ""
	set MoveSpeed $1
else
	set MoveSpeed 20
endif
if test "$2" != ""
	set PosRotation $2
else
	set PosRotation 1
endif
setcat NegRotation - $PosRotation

#
newActionMap move.sae
bindAction keyboard make a	TO IDACTION_MOVELEFT $MoveSpeed
bindAction keyboard break a TO IDACTION_MOVELEFT 0
bindAction keyboard make f	TO IDACTION_MOVERIGHT $MoveSpeed
bindAction keyboard break f TO IDACTION_MOVERIGHT 0
bindAction keyboard make s	TO IDACTION_MOVEBACK $MoveSpeed
bindAction keyboard break s TO IDACTION_MOVEBACK 0
bindAction keyboard make d	TO IDACTION_MOVEFORWARD $MoveSpeed
bindAction keyboard break d TO IDACTION_MOVEFORWARD 0
bindAction keyboard make e	TO IDACTION_MOVEUP $MoveSpeed
bindAction keyboard break e TO IDACTION_MOVEUP   0
bindAction keyboard make c	TO IDACTION_MOVEDOWN $MoveSpeed
bindAction keyboard break c TO IDACTION_MOVEDOWN 0

bindAction	mouse	make	button1		TO	IDACTION_MOVEFORWARD $MoveSpeed
bindAction	mouse	break	button1		TO	IDACTION_MOVEFORWARD 0

# bindAction	mouse	xaxis	TO	IDACTION_YAW	 -.01	Value	Scale
# bindAction	mouse	yaxis	TO	IDACTION_PITCH -.01	Value	Scale
bindAction	mouse	xaxis	TO	IDACTION_YAW	 scale .02	flip 
bindAction	mouse	yaxis	TO	IDACTION_PITCH 	 scale .02	flip 

bindAction keyboard make left	TO IDACTION_YAW $PosRotation
bindAction keyboard break left	TO IDACTION_YAW 0
bindAction keyboard make right	TO IDACTION_YAW $NegRotation
bindAction keyboard break right	TO IDACTION_YAW 0
bindAction keyboard make up	   	TO IDACTION_PITCH $PosRotation
bindAction keyboard break up	TO IDACTION_PITCH 0
bindAction keyboard make down	TO IDACTION_PITCH $NegRotation
bindAction keyboard break down	TO IDACTION_PITCH 0

# Clear out temps
set MoveSpeed
set PosRotation
set NegRotation

