#
#
#
set ConsoleWorld::DefaultSearchPath ".;..\art;"

# Load in default GFXFont
set Console::GFXFont console.pft

# Default MoveObject to move the camera around
# Only works with example plugin
set ActionTarget "camera"
newMoveObj $ActionTarget

set DirectionalVelocity 100
set PositiveRotation 1.0
set NegativeRotation -1.0
alias action postAction '$ActionTarget'

#bind make  a action MoveLeft '$DirectionalVelocity'
#bind break a action MoveLeft 0'
#bind make  d action MoveRight '$DirectionalVelocity'
#bind break d action MoveRight 0'
#bind make  s action MoveBackward '$DirectionalVelocity'
#bind break s action MoveBackward 0
#bind make  w action MoveForward '$DirectionalVelocity'
#bind break w action MoveForward 0'
#bind make  e action MoveUp '$DirectionalVelocity'
#bind break e action MoveUp 0'
#bind make  c action MoveDown '$DirectionalVelocity'
#bind break c action MoveDown 0'

#bind make  Left  action MoveYaw '$PositiveRotation'
#bind break Left  action MoveYaw 0'
#bind make  Right action MoveYaw '$NegativeRotation'
#bind break Right action MoveYaw 0'
#bind make  Up    action MovePitch '$PositiveRotation'
#bind break Up    action MovePitch 0'
#bind make  Down  action MovePitch '$NegativeRotation'
#bind break Down  action MovePitch 0'
#bind make  r  action MoveRoll '$PositiveRotation'
#bind break r  action MoveRoll 0'
#bind make  R  action MoveRoll '$NegativeRotation'
#bind break R  action MoveRoll 0'

detail
loadShow
#showGfx

activate
alias close "echo"


#ted
loadVolume mv ..\art\mars.sim.vol
loadVolume tv tedsc.vol
ted
