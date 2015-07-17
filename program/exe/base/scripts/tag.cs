
function onPlayerKilled(%killerId, %playerId, %msgTag)
{
   if(!%killerId)
   {
      messageAll(0, strcat(getPlayerName(%playerId), " died."));
   }
   else if(%playerId == %killerId)
   {
      %message1 = strcat(getPlayerName(%playerId), " killed himself.");
      %message2 = "You killed yourself.";
      if(%playerId == $ITplayer)
      {
         %message1 = strcat(%message1, "  IT is up for grabs!");
         %message2 = strcat(%message2, "  You are no longer it.");
      }   
      messageAllExcept(%playerId, 0, %message1);
      messageToPlayer(%playerId, 0, %message2);
      $ITplayer = 0;
   }
   else
   {
      %message1 = strcat(getPlayerName(%killerId), " killed player ",
          getPlayerName(%playerId), ".");
      %message2 = strcat("You killed ", getPlayerName(%playerId), ".");

      if(!$ITplayer)
      {
         %message1 = strcat(%message1, "  ", getPlayerName(%killerId), " is now IT!");
         %message2 = strcat(%message2, "  You are now IT!");
         $ITplayer = %killerId;
      }
      else if($ITplayer == %killerId)
      {
         %message1 = strcat(%message1, "  ", getPlayerName(%killerId), " scores a point because he is IT!");
         %message2 = strcat(%message2, "  You score!");
         playerScored(%killerId, 1);
      }
      else if($ITplayer == %playerId)
      {
         %message1 = strcat(%message1, "  ", getPlayerName(%killerId), " is now IT!");
         %message2 = strcat(%message2, "  You are now IT!");
         $ITplayer = %killerId;
      }
      messageAllExcept(%killerId, 0, %message1);
      messageToPlayer(%killerId, 0, %message2);
   }
}

function onPlayerDisconnect(%playerId)
{
   
}