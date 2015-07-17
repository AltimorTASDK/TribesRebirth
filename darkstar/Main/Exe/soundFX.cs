

deleteObject sfxGroup
purgeResources
cls

newObject soundVol SimVolume sound.vol


newSfx
sfxRegister  0 0.0 ENGext.WAV   SFX_IS_3D SFX_IS_LOOPING
sfxRegister  1 0.0 FlybyM.wav
sfxRegister  2 0.0 explo1.wav
sfxRegister  3 0.0 fkd7-1.wav   SFX_IS_3D SFX_IS_LOOPING
sfxRegister  4 0.1 chopper1.wav SFX_IS_3D SFX_IS_LOOPING
sfxRegister  5 0.0 MLOOP.WAV
sfxRegister  6 0.0 biplane.wav  SFX_IS_3D SFX_IS_LOOPING
sfxRegister  7 0.0 brief.wav    SFX_IS_3D SFX_IS_LOOPING SFX_IS_STREAMING
sfxRegister  8 0.0 d_bang.wav
sfxRegister  9 0.0 hum.wav


bind break f1  sfxPlay  1  
bind break f2  sfxPlay  2
bind break f3  sfxPlay  3
bind break f4  sfxPlay  4
bind break f5  sfxPlay  5
bind break f6  sfxPlay  6
bind break f7  sfxPlay  7
bind break f8  sfxPlay  8
bind break f9  sfxPlay  9
bind break f10 sfxPlay  0


























