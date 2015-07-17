//--------------------------------------------------------------------------

//--------------------------------------------------------------------------


//--------------------------------------------------------------------------

StaticShapeData Example
{
   shapeFile = "radar";
   shadowDetailLevel = 0;
//   explosionId = 0;
   ambientSoundId = IDSFX_GENERATOR;
   maxDamage = 2.0;
};

function Example::onAdd(%this)
{
   //echo("Example added: ", %this);
}

function Example::onRemove(%this)
{
   //echo("Example removed: ", %this);
}

function Example::onEnabled(%this)
{
   //echo("Example enabled");
}

function Example::onDisabled(%this)
{
   //echo("Example disabled");
}

function Example::onDestroyed(%this)
{
   //echo("Example destroyed");
}

function Example::onPower(%this, %newState, %generator)
{
   //echo("Example power state: ", %newState);
}

function Example::onCollision(%this, %object)
{
   //echo("Example collision with ", %object);
}

function Example::onAttack(%this, %object)
{
   //echo("Example attacked ", %object);
}

