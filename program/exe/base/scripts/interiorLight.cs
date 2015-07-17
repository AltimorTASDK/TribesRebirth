//-------------------------------------- Interior light scripting functions...
//                                        based on the tsshape animation functions
//
function Interior::lightOn(%int, %light)
{
   Interior::setLightTime(%int, %light, 0.5);
}

function Interior::lightOff(%int, %light)
{
   Interior::setLightTime(%int, %light, 0);
}

