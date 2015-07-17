//============================================================================================
// ES3 and Fear common strings
// Reserved: 1.05 million to 1.1 million

IDRGN_BEG_ES3FEAR                   = 01050000;  

// SimShape: May be moved...
IDSIMOBJ_SSP_DAMAGEDIST_BEGIN       = 01050010; // "";
IDSIMOBJ_SSP_DAMAGETONONE           = 01050011, "Forward Damage to None";
IDSIMOBJ_SSP_DAMAGETOPARENT         = 01050012, "Forward Full Damage to Parent";
IDSIMOBJ_SSP_DAMAGETOPARENTATTEN    = 01050013, "Forward Attenuated Damage to Parent";
IDSIMOBJ_SSP_DAMAGEDIST_END         = 01050019; // "";

IDSIMOBJ_SSP_DESTRUCTEV_BEGIN       = 01050020; // "";
IDSIMOBJ_SSP_DESTDONOTHING          = 01050021, "Do Nothing";
IDSIMOBJ_SSP_DESTAPPLYMAXDAMAGE     = 01050022, "Apply Max Damage";
IDSIMOBJ_SSP_DESTAPPLYDAMAGEPOINTS  = 01050023, "Apply Specified Destruction Points";
IDSIMOBJ_SSP_DESTRUCTEV_END         = 01050029; // "";



// Font <f0> = IDFNT_FONTIS_3      normal green   
// Font <f1> = IDFNT_FONTIS_2      pale yellow
// Font <f2> = IDFNT_FONTIS_1      bright yellow
// Font <f3> = IDFNT_FONTIS_4      bright green
// Font <f4> = IDFNT_FONTIS_5      gray
// Font <f5> = IDFNT_FONTIS_6      red
// Font <f6> = IDFNT_FONTIS_3I     normal italic   

//-------------------------------------- IRC CHAT STRINGS
IDIRC_JOIN                          = 01060000, "<f4>%s (%s) has joined the conversation.";           
IDIRC_QUIT                          = 01060001, "<f4>%s (%s) has quit the conversation.";           
IDIRC_PART                          = 01060002, "<f4>%s (%s) has left the conversation.";           
IDIRC_NICK                          = 01060003, "<f4>%s is now known as %s.";           
IDIRC_SAYS                          = 01060004, "<f1>%s says:<f4> %s";           
IDIRC_SAYS_ME                       = 01060005, "<f2>%s says:<f4> %s";           
IDIRC_ACTION                        = 01060006, "<f6>%s %s";           
IDIRC_NOTICE                        = 01060007, "<f4>NOTICE FROM %s: %s";           
IDIRC_KICK                          = 01060008, "<f5>Host %s kicks %s out of the chat room, saying \"%s\".";           
IDIRC_BAN                           = 01060009, "<f5>Host %s bans %s out of the chat room, saying \"%s\".";           
IDIRC_IGNORE                        = 01060010, "<f4>You are now ignoring %s.";
IDIRC_NO_IGNORE                     = 01060011, "<f4>You are no longer ignoring %s.";
IDIRC_OPERATOR                      = 01060012, "<f4>%s made you an operator.";
IDIRC_SPEAKER                       = 01060013, "<f4>%s made you a speaker.";
IDIRC_SPECTATOR                     = 01060014, "<f4>%s made you a spectator.";
IDIRC_WHO_REPLY                     = 01060015, "<f4>WHO %s: %s@%s (%s) connected to server %s.";
IDIRC_CHANNEL                       = 01060016, "<f4>%03d %s -- %s";
IDIRC_AWAY                          = 01060017, "<f4>%s is away: %s.";
IDIRC_RETURNED                      = 01060018, "<f4>%s has returned.";
IDIRC_MOTD                          = 01060019, "<f4>%s";
IDIRC_ERROR                         = 01060020, "<f5>%s";
IDIRC_PING                          = 01060021, "<f4>%s roundtrip delay: %d seconds.";
IDIRC_WHOIS_REPLY                   = 01060022, "<f4>%s.";
IDIRC_WHOIS_IDLE_M                  = 01060023, "<f4>%s has been idle for %d minute(s) and %d second(s).";
IDIRC_WHOIS_IDLE_S                  = 01060024, "<f4>%s has been idle for %d second(s).";
IDIRC_VERSION                       = 01060025, "<f4>%s's version: %s.";
IDIRC_JOIN_IP_MESSAGE               = 01060026, "left to play %s on server %s [%s:%d%d]";

//------------ IRC Errors
IDIRC_CONNECTING_SOCKET             = 01060200, "<f4>Connecting to %s:%d ...";
IDIRC_CONNECTING_IRC                = 01060201, "<f4>Logging into chat server.";
IDIRC_CONNECTING_WAITING            = 01060202, "";
IDIRC_CONNECTING_DONE               = 01060203, "<f4>Successfully connected to %s:%d.";
IDIRC_CONNECTED                     = 01060204, "<f4>Connected to %s.";
IDIRC_RECONNECT                     = 01060205, "<f4>Try %d.  Attempting to reconnect to %s.";
IDIRC_DISCONNECTED                  = 01060206, "<f4>Disconnect successful.";
IDIRC_ERR_DROPPED                   = 01060207, "<f5>NOTICE: You have been disconnected from %s.";
IDIRC_ERR_CONNECT                   = 01060208, "<f5>NOTICE: Could not connect to %s.";
IDIRC_ERR_HOSTNAME                  = 01060209, "<f5>NOTICE: Could not resolve address %s.";
IDIRC_ERR_SET_NICK                  = 01060210, "<f5>NOTICE: Your Nick and Full Name must be set before connecting.";
IDIRC_ERR_NICK                      = 01060211, "<f5>NOTICE: Your Nick (%s) is in use.";
IDIRC_ERR_RECONNECT                 = 01060212, "<f5>NOTICE: Unable to reconnect to server %s.";
IDIRC_CHANNEL_LIST                  = 01060213, "";
IDIRC_ERR_NICK_IN_USE               = 01060214, "<f5>NOTICE: Nickname (%s) is already in use.";
IDIRC_ERR_TIMEOUT                   = 01060215, "<f5>NOTICE: Connection failed. The server did not respond.";
IDIRC_BANNED_CH                     = 01060216, "<f4>Cannot join %s: you have been banned.";
IDIRC_BANNED_SERVER                 = 01060217, "<f4>You have been banned from this server.";
IDIRC_INVITE_ONLY                   = 01060218, "<f4>Cannot join %s: room is invite only.";
IDIRC_CHANNEL_FULL                  = 01060219, "<f4>Cannot join %s: room is full.";
IDIRC_CHANNEL_FLAGS                 = 01060220, "";
IDIRC_INVITED                       = 01060221, "";
IDIRC_UNAWAY_REPLY                  = 01060222, "<F4>You are no longer marked as being away.";
IDIRC_NOWAWAY_REPLY                 = 01060223, "<F4>You have been marked as being away.";
IDIRC_NO_TOPIC                      = 01060224, "<F4>%s: No topic is set.";
IDIRC_INVITE_REPLY                  = 01060225, "<F4>You have invited %s to room %s.";
IDIRC_BAN_LIST                      = 01060226, "";
IDIRC_END_OF_WHO                    = 01060227, "<F4>End of WHO list.";
IDIRC_CHANNEL_HAS_KEY               = 01060228, "<F4>Cannot join %s: invalid key.";
IDIRC_PART_PRIVATE                  = 01020229, "<F4>%s (%s) has disconnected from IRC.";

IDRGN_END_ES3FEAR                  = 01099999;
