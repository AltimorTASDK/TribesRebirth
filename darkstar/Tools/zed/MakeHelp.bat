@echo off
REM -- First make map file from Microsoft Visual C++ generated resource.h
echo // MAKEHELP.BAT generated Help Map file.  Used by THRED.HPJ. >"hlp\THRED.hm"
echo. >>"hlp\THRED.hm"
echo // Commands (ID_* and IDM_*) >>"hlp\THRED.hm"
makehm ID_,HID_,0x10000 IDM_,HIDM_,0x10000 resource.h >>"hlp\THRED.hm"
echo. >>"hlp\THRED.hm"
echo // Prompts (IDP_*) >>"hlp\THRED.hm"
makehm IDP_,HIDP_,0x30000 resource.h >>"hlp\THRED.hm"
echo. >>"hlp\THRED.hm"
echo // Resources (IDR_*) >>"hlp\THRED.hm"
makehm IDR_,HIDR_,0x20000 resource.h >>"hlp\THRED.hm"
echo. >>"hlp\THRED.hm"
echo // Dialogs (IDD_*) >>"hlp\THRED.hm"
makehm IDD_,HIDD_,0x20000 resource.h >>"hlp\THRED.hm"
echo. >>"hlp\THRED.hm"
echo // Frame Controls (IDW_*) >>"hlp\THRED.hm"
makehm IDW_,HIDW_,0x50000 resource.h >>"hlp\THRED.hm"
REM -- Make help for Project THRED


echo Building Win32 Help files
start /wait hcrtf -x "hlp\THRED.hpj"
echo.
if exist Debug\nul copy "hlp\THRED.hlp" Debug
if exist Debug\nul copy "hlp\THRED.cnt" Debug
if exist Release\nul copy "hlp\THRED.hlp" Release
if exist Release\nul copy "hlp\THRED.cnt" Release
echo.


