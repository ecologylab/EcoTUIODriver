;Author - Rajat Goyal
;The script bleow makes windows installer for WinTUIODriver Project ,
;which is hosted at https://github.com/ecologylab/WinTUIODriver

!include MUI2.nsh
!include "Sections.nsh"
!include "logiclib.nsh"

OutFile "WinTuioDriver.exe"
RequestExecutionLevel admin

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
!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_COMPONENTS
Page Custom pre post
!insertmacro MUI_PAGE_INSTFILES

;;Finish Page
;!define MUI_PAGE_CUSTOMFUNCTION_PRE Done
;!insertmacro MUI_PAGE_FINISH

;install directory
InstallDir  "C:\Program Files\WinTuioDriver"

;un-installatin pages
UninstPage uninstConfirm
UninstPage instfiles

var dialog
var hwnd
var driverversion_32

Function .onInit
UserInfo::GetAccountType
pop $0
${If} $0 != "admin" ;Require admin rights on NT4+
    MessageBox mb_iconstop "Administrator rights required!"
    SetErrorLevel 740 ;ERROR_ELEVATION_REQUIRED
    Quit
${EndIf}
FunctionEnd

Function pre
	nsDialogs::Create 1018
		Pop $dialog

	${NSD_CreateRadioButton} 0 0 40% 6% "x64"
		Pop $hwnd
		${NSD_AddStyle} $hwnd ${WS_GROUP}
		${NSD_SetUserData} $hwnd "false"
		${NSD_OnClick} $hwnd RadioClick
	${NSD_CreateRadioButton} 0 12% 40% 6% "x86"
		Pop $hwnd
		${NSD_SetUserData} $hwnd "true"
		${NSD_OnClick} $hwnd RadioClick

	nsDialogs::Show
FunctionEnd

Function RadioClick
	Pop $hwnd
	${NSD_GetUserData} $hwnd $driverversion_32
FunctionEnd

Function post
	${If} $driverversion_32 == ""
	    MessageBox MB_OK "Please select your machine type"
	    Abort
	${ElseIf} $driverversion_32 == "true"
            MessageBox MB_OK "Your computer might freez sevral times during the installation , don't do anything | The Installer installs 5 virtual TUIO touch devices . It's important that you provide permission each time."
        ${Else}
	   MessageBox MB_OK "Your computer might freez sevral times during the installation , don't do anything | The Installer installs 5 virtual TUIO touch devices . It's important that you provide permission each time."
     	${EndIf}
FunctionEnd

Section "Driver and Configuration Utility"
  SetOutPath $INSTDIR
        File /r "Drivers"
        File /r "Executables"
        
  WriteUninstaller "$INSTDIR\Uninstall-WinTuioDriver.exe"
  SetOutPath "$INSTDIR\Executables"
  
  ;CreateShortCut "$SMPROGRAMS\Startup\WinTUIODriver.lnk" "$INSTDIR\Executables\Configuration_Utility.exe"
  CreateShortCut "$SMPROGRAMS\WinTUIODriver.lnk" "$INSTDIR\Executables\Configuration_Utility.exe"

  SetOutPath $INSTDIR
  CreateShortCut "$SMPROGRAMS\Uninstall-WinTuioDriver.lnk" "$INSTDIR\Uninstall-WinTuioDriver.exe"
  
  SetOutPath "C:\Users\AppData\TUIO-To-Vmulti\Data"
  
  File /r "Data"

  SetOutPath $INSTDIR


;cmd related to 64bit or 32bit driver .
  ${If} $driverversion_32 == "true"
        SetOutPath "$INSTDIR\Drivers\x86-Driver-Installers"
	   Execwait "$INSTDIR\Drivers\x86-Driver-Installers\Installvmulti1.cmd"
            Execwait "$INSTDIR\Drivers\x86-Driver-Installers\Installvmulti2.cmd"
             Execwait "$INSTDIR\Drivers\x86-Driver-Installers\Installvmulti3.cmd"
              Execwait "$INSTDIR\Drivers\x86-Driver-Installers\Installvmulti4.cmd"
               Execwait "$INSTDIR\Drivers\x86-Driver-Installers\Installvmulti5.cmd"
        SetOutPath $INSTDIR
  ${Else}
   SetOutPath "$INSTDIR\Drivers\x64-Driver-Installers"
            Execwait "$INSTDIR\Drivers\x64-Driver-Installers\Installvmulti1.cmd"
            Execwait "$INSTDIR\Drivers\x64-Driver-Installers\Installvmulti2.cmd"
             Execwait "$INSTDIR\Drivers\x64-Driver-Installers\Installvmulti3.cmd"
              Execwait "$INSTDIR\Drivers\x64-Driver-Installers\Installvmulti4.cmd"
               Execwait "$INSTDIR\Drivers\x64-Driver-Installers\Installvmulti5.cmd"
  SetOutPath $INSTDIR
  ${EndIf}

SectionEnd

Section "Source"
SetOutPath $INSTDIR
        File /r "Configuration_Utility"
        File /r "Services"

SectionEnd


Section "uninstall"
 RmDir /r $INSTDIR
SectionEnd

