
deleteObject ( sfxGroup );
purgeResources ();
// cls

loadVolume ( soundVol, "sound.vol" );

newSfx();

sfxRegister( 0, 0.0, "ENGext.WAV", SFX_IS_3D, SFX_IS_LOOPING );
sfxRegister( 1, 0.0, "FlybyM.wav" );
sfxRegister( 2, 0.0, "explo1.wav" );
sfxRegister( 3, 0.0, "fkd7-1.wav", SFX_IS_3D, SFX_IS_LOOPING );
sfxRegister( 4, 0.1, "chopper1.wav", SFX_IS_3D, SFX_IS_LOOPING );
sfxRegister( 5, 0.0, "MLOOP.WAV" );
sfxRegister( 6, 0.0, "biplane.wav", SFX_IS_3D, SFX_IS_LOOPING );
sfxRegister( 7, 0.0, "brief.wav", SFX_IS_3D, SFX_IS_LOOPING, SFX_IS_STREAMING );
sfxRegister( 8, 0.0, "d_bang.wav" );
sfxRegister( 9, 0.0, "hum.wav" );

bind( keyboard, break, f1, to,  "sfxPlay( 1 );" );
bind( keyboard, break, f2, to,  "sfxPlay( 2 );" );
bind( keyboard, break, f3, to,  "sfxPlay( 3 );" );
bind( keyboard, break, f4, to,  "sfxPlay( 4 );" );
bind( keyboard, break, f5, to,  "sfxPlay( 5 );" );
bind( keyboard, break, f6, to,  "sfxPlay( 6 );" );
bind( keyboard, break, f7, to,  "sfxPlay( 7 );" );
bind( keyboard, break, f8, to,  "sfxPlay( 8 );" );
bind( keyboard, break, f9, to,  "sfxPlay( 9 );" );
bind( keyboard, break, f10, to, "sfxPlay( 0 );" );

