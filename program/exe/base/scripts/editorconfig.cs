//-------------------------------------------------------------------
// mission editor specific configuration file
//-------------------------------------------------------------------


//-------------------------------------------------------------------
// these functions act like hotkeys in ted for switching between
// mouse functions for a brief moment
//-------------------------------------------------------------------
function pushMouseAction(%action)
{
   $prevMouseAction = Ted::getActionName(Ted::getLButtonActionIndex());
   Ted::setLButtonAction(%action);
}

function popMouseAction()
{
   Ted::setLButtonAction($prevMouseAction);
}

bindCommand(keyboard, make, t, to, "pushMouseAction(select);");
bindCommand(keyboard, break, t, to, "popMouseAction();");

bindCommand(keyboard, make, g, to, "pushMouseAction(deselect);");
bindCommand(keyboard, break, g, to, "popMouseAction();");

bindCommand(keyboard, make, b, to, "pushMouseAction(smooth);");
bindCommand(keyboard, break, b, to, "popMouseAction();");
