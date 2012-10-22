;Author - Rajat Goyal
;The script bleow makes windows installer for EcoTUIODriver Project ,
;which is hosted at https://github.com/ecologylab/WinTUIODriver

!include MUI2.nsh
!include "Sections.nsh"
!include "logiclib.nsh"

OutFile "EcoTuioDriver.exe"
RequestExecutionLevel admin

name "Eco TUIO Driver"

!define MUI_ICON "Configuration_Utility\Configuration_Utility\Configuration_Utility.ico"
!define MUI_UNICON "${NSISDIR}\Contrib\Graphics\Icons\modern-uninstall.ico"
!define MUI_WELCOMEFINISHPAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Wizard\win.bmp"
!define MUI_UNWELCOMEFINISHPAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Wizard\orange-uninstall.bmp"

ShowInstDetails show
ShowUnInstDetails show

;Defination of the macros for user data
!macro NSD_SetUserData hwnd data
	nsDialogs::SetUserData ${hwnd} ${data}
!macroend
!define NSD_SetUserData `!insertmacro NSD_SetUserData`

!macro NSD_GetUserData hwnd outvar
	nsDialogs::GetUserData ${hwnd}
	Pop ${outvar}
!macroend
!define NSD_GetUserData `!insertmacro NSD_GetUserData`

;--------------------------------
;Interface Configuration
;Header Image for the installer
!define MUI_HEADERIMAGE
;!define MUI_HEADERIMAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Header\nsis.bmp" ; optional
!define MUI_ABORTWARNING
;Pages in the installation
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "LISCENSE.txt"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_COMPONENTS
;Page Custom pre post
!insertmacro MUI_PAGE_INSTFILES

;;Finish Page
;!define MUI_PAGE_CUSTOMFUNCTION_PRE Done
!define MUI_FINISHPAGE_TITLE "Eco TUIO Driver Installed Successfuly"
;!define MUI_WELCOMEFINISHPAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Wizard\win.bmp"
!define MUI_FINISHPAGE_BUTTON "Ok"
!define MUI_FINISHPAGE_TEXT_REBOOT "To complete the Eco TUIO Driver installation the system needs to reboot"
!define MUI_FINISHPAGE_TEXT_REBOOTNOW "Restart now"
!define MUI_FINISHPAGE_TEXT_REBOOTLATER "Restart later"
!insertmacro MUI_PAGE_FINISH

;install directory
InstallDir  "C:\Program Files\EcoTuioDriver"

;un-installatin pages
;UninstPage uninstConfirm
;UninstPage instfiles
!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!define MUI_FINISHPAGE_TITLE "Eco TUIO Driver Removed Successfuly"
;!define MUI_UNWELCOMEFINISHPAGE_BITMAP "${NSISDIR}\Contrib\Graphics\Wizard\win.bmp"
!define MUI_FINISHPAGE_BUTTON "Ok"
!define MUI_FINISHPAGE_TEXT_REBOOT "To complete the Eco TUIO Driver removal the system needs to reboot"
!define MUI_FINISHPAGE_TEXT_REBOOTNOW "Restart now"
!define MUI_FINISHPAGE_TEXT_REBOOTLATER "Restart later"
!insertmacro MUI_UNPAGE_FINISH

!insertmacro MUI_LANGUAGE "English"

Function .onInit
UserInfo::GetAccountType
pop $0
${If} $0 != "admin" ;Require admin rights on NT4+
    MessageBox mb_iconstop "Administrator rights required!"
    SetErrorLevel 740 ;ERROR_ELEVATION_REQUIRED
    Quit
${EndIf}
FunctionEnd

Section "Driver and Configuration Utility"
  
  MessageBox MB_OK "Your computer might freeze several times during the installation , don't do anything | The Installer installs 5 virtual TUIO touch devices . It's important that you provide permission each time."

  SetOutPath $INSTDIR
        File /r "Drivers"
        File /r "Executables"
        
  WriteUninstaller "$INSTDIR\Uninstall-EcoTuioDriver.exe"
  SetOutPath "$INSTDIR\Executables"
  
  ;CreateShortCut "$SMPROGRAMS\Startup\WinTUIODriver.lnk" "$INSTDIR\Executables\Configuration_Utility.exe"
  CreateShortCut "$DESKTOP\EcoTUIODriver.lnk" "$INSTDIR\Executables\Configuration_Utility.exe"
  
  CreateDirectory "$SMPROGRAMS\Eco TUIO Driver"
  CreateShortCut "$SMPROGRAMS\Eco TUIO Driver\EcoTUIODriver.lnk" "$INSTDIR\Executables\Configuration_Utility.exe"

  SetOutPath $INSTDIR
  CreateShortCut "$SMPROGRAMS\Eco TUIO Driver\Uninstall-EcoTuioDriver.lnk" "$INSTDIR\Uninstall-EcoTuioDriver.exe"
  
  
  
  SetOutPath "C:\Users\AppData\TUIO-To-Vmulti"
  
  File /r "Data"

  SetOutPath $INSTDIR


;cmd related to 64bit or 32bit driver .
  IfFileExists $WINDIR\SYSWOW64\*.* Is64bit Is32bit
  Is32bit:
    SetOutPath "$INSTDIR\Drivers\x86-Driver-Installers"
    Execwait "$INSTDIR\Drivers\x86-Driver-Installers\Installvmulti1.cmd"
    Execwait "$INSTDIR\Drivers\x86-Driver-Installers\Installvmulti2.cmd"
    Execwait "$INSTDIR\Drivers\x86-Driver-Installers\Installvmulti3.cmd"
    Execwait "$INSTDIR\Drivers\x86-Driver-Installers\Installvmulti4.cmd"
    Execwait "$INSTDIR\Drivers\x86-Driver-Installers\Installvmulti5.cmd"
    SetOutPath $INSTDIR
    GOTO End32Bitvs64BitCheck
  Is64Bit:
    SetOutPath "$INSTDIR\Drivers\x64-Driver-Installers"
    Execwait "$INSTDIR\Drivers\x64-Driver-Installers\Installvmulti1.cmd"
    Execwait "$INSTDIR\Drivers\x64-Driver-Installers\Installvmulti2.cmd"
    Execwait "$INSTDIR\Drivers\x64-Driver-Installers\Installvmulti3.cmd"
    Execwait "$INSTDIR\Drivers\x64-Driver-Installers\Installvmulti4.cmd"
    Execwait "$INSTDIR\Drivers\x64-Driver-Installers\Installvmulti5.cmd"
    SetOutPath $INSTDIR
    
  End32Bitvs64BitCheck:

SectionEnd

Section "Source"
  SetOutPath $INSTDIR
        File /r "Configuration_Utility"
        File /r "Services"

  SetRebootFlag true
SectionEnd


Section "uninstall"
 Delete "$SMPROGRAMS\Eco TUIO Driver\EcoTUIODriver.lnk"
 Delete "$SMPROGRAMS\Eco TUIO Driver\Uninstall-EcoTuioDriver.lnk"
 RmDir  "$SMPROGRAMS\Eco TUIO Driver"
 
 Delete "$DESKTOP\EcoTUIODriver.lnk"
 
 Execwait "$INSTDIR\Executables\Tuio-to-Vmulti-Service-1.exe remove 3"
 Execwait "$INSTDIR\Executables\Tuio-to-Vmulti-Service-2.exe remove 3"
 Execwait "$INSTDIR\Executables\Tuio-to-Vmulti-Service-3.exe remove 3"
 Execwait "$INSTDIR\Executables\Tuio-to-Vmulti-Service-4.exe remove 3"
 Execwait "$INSTDIR\Executables\Tuio-to-Vmulti-Service-5.exe remove 3"
 
 ;remove drivers
 IfFileExists $WINDIR\SYSWOW64\*.* Is64bit2 Is32bit2
  Is32bit2:
    SetOutPath "$INSTDIR\Drivers\x86-Driver-Installers"
    Execwait "$INSTDIR\Drivers\x86-Driver-Installers\Uninstallvmulti.cmd"
    GOTO End32Bitvs64BitCheck2
  Is64Bit2:
    SetOutPath "$INSTDIR\Drivers\x64-Driver-Installers"
    Execwait "$INSTDIR\Drivers\x64-Driver-Installers\Uninstallvmulti.cmd"
  End32Bitvs64BitCheck2:
 
 SetOutPath $INSTDIR
 RmDir /r "C:\Users\AppData\TUIO-To-Vmulti"
 RmDir /r $INSTDIR
 
 SetRebootFlag true
 
SectionEnd

