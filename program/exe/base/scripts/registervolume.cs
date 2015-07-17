// registervol volumename
function registervolume( %VolName )
{
	if( %VolName == "")
	{
		echo("registervolume volumename");
	}
	else
	{
		if( focusServer )
		{
			%Name = strcat("MissionGroup\\Volumes\\", %VolName);
			if( isObject( %Name) )
			{
			//		
			}
			else
			{
				%Volume = strcat(%VolName, ".vol");
				newObject( %VolName, SimVolume, %Volume );
				addToSet( "MissionGroup\\Volumes", %VolName);
			}
			focusClient();
		}
	}
}