// check which version should be loaded..
if( $ME::Loaded == "true" )
{
   function ME::Move( %speed, %rot )
   {
      if( %speed == "" ) 
         %speed = 2;
      if( %rot == "" )
         %rot = 0.2;
      $ME::CameraMoveSpeed = %speed;
      $ME::CameraRotateSpeed = %rot;
   }

   newActionMap( "editor.sae" );
   bindAction( keyboard, make, a, TO, IDACTION_MOVELEFT, 1 );
   bindAction( keyboard, break, a, TO, IDACTION_MOVELEFT, 0 );
   bindAction( keyboard, make, d, TO, IDACTION_MOVERIGHT, 1 );
   bindAction( keyboard, break, d, TO, IDACTION_MOVERIGHT, 0 );
   bindAction( keyboard, make, s, TO, IDACTION_MOVEBACK, 1 );
   bindAction( keyboard, break, s, TO, IDACTION_MOVEBACK, 0 );
   bindAction( keyboard, make, w, TO, IDACTION_MOVEFORWARD, 1 );
   bindAction( keyboard, break, w, TO, IDACTION_MOVEFORWARD, 0 );
   bindAction( keyboard, make, e, TO, IDACTION_MOVEUP, 1 );
   bindAction( keyboard, break, e, TO, IDACTION_MOVEUP, 0 );
   bindAction( keyboard, make, c, TO, IDACTION_MOVEDOWN, 1 );
   bindAction( keyboard, break, c, TO, IDACTION_MOVEDOWN, 0 );

   bindCommand( keyboard, make, f1, to, "MEHide();");
   bindCommand( keyboard, make, f2, to, "MEShowInspector();");
   bindCommand( keyboard, make, f3, to, "MEShowCreator();");
   bindCommand( keyboard, make, f4, to, "MEShowTed();");
   bindCommand( keyboard, make, f5, to, "MEGameMode();");
   bindCommand( keyboard, make, f9, to, "METoggleHelp();" );
   bindCommand( keyboard, make, o, to, "METoggleOptions();");

   //
   bindCommand( keyboard, make, control, delete, to, "MEDeleteSelection();" );
   bindCommand( keyboard, make, control, c, to, "MECopySelection();" );
   bindCommand( keyboard, make, control, x, to, "MECutSelection();" );
   bindCommand( keyboard, make, control, v, to, "MEPasteSelection();" );
   bindCommand( keyboard, make, control, d, to, "MEDropSelection();");

   bindCommand( keyboard, make, control, n, to, "ME::CreateGroup();" );

   //
   bindCommand( keyboard, make, control, z, to, "MEUndo();" );
   bindCommand( keyboard, make, control, s, to, "MESave();" );

   // 
   bindAction( keyboard, make, control, TO, IDACTION_ME_MOD1, 1 );
   bindAction( keyboard, break, control, TO, IDACTION_ME_MOD1, 0 );
   bindAction( keyboard, make, shift, TO, IDACTION_ME_MOD2, 1 );
   bindAction( keyboard, break, shift, TO, IDACTION_ME_MOD2, 0 );
   bindAction( keyboard, make, alt, TO, IDACTION_ME_MOD3, 1 );
   bindAction( keyboard, break, alt, TO, IDACTION_ME_MOD3, 0 );

   //
   bindAction(mouse, xaxis, TO, IDACTION_YAW, scale, 0.002, flip);
   bindAction(mouse, yaxis, TO, IDACTION_PITCH, scale, 0.002, flip);

   // bookmark binds
   for( %i = 0; %i < 10; %i++ )
   {
      bindCommand( keyboard, make, control, %i, to, "ME::PlaceBookmark(" @ %i @ ");");
      bindCommand( keyboard, make, alt, %i, to, "ME::GotoBookmark(" @ %i @ ");");
   }

   // movement binds
   bindCommand( keyboard, make, 0, to, "ME::Move(1024);");
   for( %i = 1; %i < 10; %i++ )
      bindCommand( keyboard, make, %i, to, "ME::Move(" @ ( 1 << %i ) @ ");" );

   // toggle key for plane movement
   bindCommand( keyboard, make, capslock, to, "METogglePlaneMovement();" );
}
else
{
   // Set up a default movement action map file
   function move( %speed, %posRot )
   {
      if( %speed == "" ) {
         $MoveSpeed = 2;
      }
      else {
         $MoveSpeed = %speed;
      }

      if( %posRot == "" ) {
         $PosRotation = 0.2;
      }
      else {
         $PosRotation = %posRot;
      }

      %NegRotation = strcat( "-", $PosRotation );

      newActionMap("move.sae");
      bindAction( keyboard, make, a, TO, IDACTION_MOVELEFT, $MoveSpeed );
      bindAction( keyboard, break, a, TO, IDACTION_MOVELEFT, 0 );
      bindAction( keyboard, make, d, TO, IDACTION_MOVERIGHT, $MoveSpeed );
      bindAction( keyboard, break, d, TO, IDACTION_MOVERIGHT, 0 );
      bindAction( keyboard, make, s, TO, IDACTION_MOVEBACK, $MoveSpeed );
      bindAction( keyboard, break, s, TO, IDACTION_MOVEBACK, 0 );
      bindAction( keyboard, make, w, TO, IDACTION_MOVEFORWARD, $MoveSpeed );
      bindAction( keyboard, break, w, TO, IDACTION_MOVEFORWARD, 0 );
      bindAction( keyboard, make, e, TO, IDACTION_MOVEUP, $MoveSpeed );
      bindAction( keyboard, break, e, TO, IDACTION_MOVEUP, 0 );
      bindAction( keyboard, make, c, TO, IDACTION_MOVEDOWN, $MoveSpeed );
      bindAction( keyboard, break, c, TO, IDACTION_MOVEDOWN, 0 );

      //
      bindAction( keyboard, make, left, TO, IDACTION_YAW, $PosRotation );
      bindAction( keyboard, break, left, TO, IDACTION_YAW, 0 );
      bindAction( keyboard, make, right, TO, IDACTION_YAW, %NegRotation );
      bindAction( keyboard, break, right, TO, IDACTION_YAW, 0 );
      bindAction( keyboard, make, up, TO, IDACTION_PITCH, $PosRotation );
      bindAction( keyboard, break, up, TO, IDACTION_PITCH, 0 );
      bindAction( keyboard, make, down, TO, IDACTION_PITCH, %NegRotation );
      bindAction( keyboard, break, down, TO, IDACTION_PITCH, 0 );

      //
      bindCommand( keyboard, make, 1, to, "move(1);");
      bindCommand( keyboard, make, 2, to, "move(3);");
      bindCommand( keyboard, make, 3, to, "move(6);");
      bindCommand( keyboard, make, 4, to, "move(12);");
      bindCommand( keyboard, make, 5, to, "move(24);");
      bindCommand( keyboard, make, 6, to, "move(50);");
      bindCommand( keyboard, make, 7, to, "move(100);");
      bindCommand( keyboard, make, 8, to, "move(200);");
      bindCommand( keyboard, make, 9, to, "move(400);");
      bindCommand( keyboard, make, 0, to, "move(5);");
   }
}