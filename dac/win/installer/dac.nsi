; dac installer script for NSIS

; include modern UI
  !include "MUI2.nsh"

; define .NET Framework veersion
  !define MIN_FRA_MAJOR "2"
  !define MIN_FRA_MINOR "0"
  !define MIN_FRA_BUILD "*"

; set program name
  Name "dac"
; set installer outfile name
  OutFile "dac_setup.exe"
; enable XP style
  XPStyle on
; set installation directory
  InstallDir "C:\dac"
; store installlation directory to registry
  InstallDirRegKey HKLM "Software\dac" ""

; declare global variables
  var StartMenuFolder
  var RADIOBUTTON

; interface settings
  !define MUI_ABORTWARNING

; declare pages
  !insertmacro MUI_PAGE_WELCOME
  !insertmacro MUI_PAGE_DIRECTORY 
  !insertmacro MUI_PAGE_COMPONENTS

  !define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKLM" 
  !define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\dac" 
  !define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"

  !insertmacro MUI_PAGE_STARTMENU Application $StartMenuFolder

  !insertmacro MUI_PAGE_INSTFILES
     
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES

; set language
  !insertmacro MUI_LANGUAGE "Russian"

; ***  ------------------------------------------------------------------------------------------ ***
; install sections

; *** base files section ***
  Section "основные файлы" base
; set section as required
	SectionIn RO
; set base files
	SetOutPath "$INSTDIR\bin"
	File "..\bin\*.*"
	SetOutPath "$INSTDIR\etc"
	File "..\etc\*.*"
; store installation folder
	WriteRegStr HKLM "Software\dac" "" $INSTDIR
; write the uninstall keys
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\dac" "DisplayName" "dac"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\dac" "UninstallString" '"$INSTDIR\Uninstall.exe"'
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\dac" "NoModify" 1
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\dac" "NoRepair" 1
; create uninstaller
   	WriteUninstaller "Uninstall.exe"
; create start menu shortcuts
	!insertmacro MUI_STARTMENU_WRITE_BEGIN Application

	CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
	CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
	CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Start.lnk" "$INSTDIR\bin\dac.exe"
	CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Stop.lnk" "$INSTDIR\bin\dacstop.bat"

	!insertmacro MUI_STARTMENU_WRITE_END
  SectionEnd

; *** ntp snmp plugin section ***
  Section /o "ntp snmp плагин" ntp_snmp
; write registry keys
	DetailPrint "Регистрация ntp snmp плагина"
	WriteRegStr HKLM "SOFTWARE\ntpext\CurrentVersion" "Pathname" "$INSTDIR\bin\ntpext.dll"
	WriteRegStr HKLM "SYSTEM\CurrentControlSet\Services\SNMP\Parameters\ExtensionAgents" "ntpext" "SOFTWARE\ntpext\CurrentVersion"
; restart snmp service
	DetailPrint "Перезапуск службы snmp"
	services::SendServiceCommand 'stop' 'snmp'
	Sleep 5000
	services::SendServiceCommand 'start' 'snmp'
  SectionEnd

; *** configuration files group ***
  SectionGroup /e "конфигурация" gr
; *** configuration files sections ***
	Section "40HLB01" c40HLB01
	SectionEnd	
	Section /o "40HLB02" c40HLB02
	SectionEnd	
	Section /o "40HLB03_01" c40HLB03_01
	SectionEnd	
	Section /o "40HLB03_02" c40HLB03_02
	SectionEnd	
	Section /o "40HLB04_01" c40HLB04_01
	SectionEnd	
	Section /o "40HLB04_02" c40HLB04_02
	SectionEnd	
	Section /o "40HLB05_01" c40HLB05_01
	SectionEnd	
	Section /o "40HLB05_02" c40HLB05_02
	SectionEnd	
	Section /o "40HLB06_01" c40HLB06_01
	SectionEnd	
	Section /o "40HLB06_02" c40HLB06_02
	SectionEnd	
	Section /o "40HLD01_01" c40HLD01_01
	SectionEnd	
  SectionGroupEnd

; *** service registr section ***
  Section "-hidden регистрация сервиса"
; determine selected configuration file	
	SectionGetFlags ${c40HLB01} $0
	${If} $0 == 1
		SectionGetText ${c40HLB01} $0
		goto reg
	${EndIf}
	SectionGetFlags ${c40HLB02} $0
	${If} $0 == 1
		SectionGetText ${c40HLB02} $0
		goto reg
	${EndIf}
	SectionGetFlags ${c40HLB03_01} $0
	${If} $0 == 1
		SectionGetText ${c40HLB03_01} $0
		goto reg
	${EndIf}
	SectionGetFlags ${c40HLB03_02} $0
	${If} $0 == 1
		SectionGetText ${c40HLB03_02} $0
		goto reg
	${EndIf}
	SectionGetFlags ${c40HLB04_01} $0
	${If} $0 == 1
		SectionGetText ${c40HLB04_01} $0
		goto reg
	${EndIf}
	SectionGetFlags ${c40HLB04_02} $0
	${If} $0 == 1
		SectionGetText ${c40HLB04_02} $0
		goto reg
	${EndIf}
	SectionGetFlags ${c40HLB05_01} $0
	${If} $0 == 1
		SectionGetText ${c40HLB05_01} $0
		goto reg
	${EndIf}
	SectionGetFlags ${c40HLB05_02} $0
	${If} $0 == 1
		SectionGetText ${c40HLB05_02} $0
		goto reg
	${EndIf}
	SectionGetFlags ${c40HLB06_01} $0
	${If} $0 == 1
		SectionGetText ${c40HLB06_01} $0
		goto reg
	${EndIf}
	SectionGetFlags ${c40HLB06_02} $0
	${If} $0 == 1
		SectionGetText ${c40HLB06_02} $0
		goto reg
	${EndIf}
	SectionGetFlags ${c40HLD01_01} $0
	${If} $0 == 1
		SectionGetText ${c40HLD01_01} $0
		goto reg
	${EndIf}
; registartion	
	reg:
; registr service
		DetailPrint "Регистрация сервиса"
		StrCpy $0 "$INSTDIR\bin\dac.exe -i $INSTDIR $0.xml"
		DetailPrint $0
		ExecCmd::exec $0 ""
 		Pop $0
		${If} $0 != 0
			MessageBox MB_OK "Сервис не зарегистрирован.$\n$\nПопробуйте провести регистрацию самостоятельно"
		${EndIf}
; create log directory
		CreateDirectory "$INSTDIR\log"
; ask user for service start
		${If} $0 == 0
			MessageBox MB_YESNO "Запустить сервис dac ?" IDYES true IDNO false
			true:
; start service
				DetailPrint "Запуск сервиса dac"
				services::SendServiceCommand 'start' 'dac'
				Goto end	
			false:
			end:
		${EndIf}
  SectionEnd

; *** ------------------------------------------------------------------------------------------ ***

; *** ------------------------------------------------------------------------------------------ ***
; set sections description

; define descriptions 
  LangString DESC_base ${LANG_RUSSIAN} "Основные файлы dac"
  LangString DESC_gr ${LANG_RUSSIAN} "Конфигурационные файлы"
  LangString DESC_ntp_snmp ${LANG_RUSSIAN} "Дополнение к сервису snmp для диагностики службы ведения времени"

; set descriptions
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
	!insertmacro MUI_DESCRIPTION_TEXT ${base} $(DESC_base)
	!insertmacro MUI_DESCRIPTION_TEXT ${gr} $(DESC_gr)
	!insertmacro MUI_DESCRIPTION_TEXT ${ntp_snmp} $(DESC_ntp_snmp)
  !insertmacro MUI_FUNCTION_DESCRIPTION_END

; *** ------------------------------------------------------------------------------------------ ***
; functions

; function call then install start (befor first gui)  
  Function .onInit
; check for .NET Framework
	Call AbortIfBadFramework
; check for already installed
	ReadRegStr $1 HKLM Software\dac ""
	StrLen $1 $1
	${If} $1 != 0
		MessageBox MB_OK|MB_ICONSTOP "Приложение уже установлено.$\n$\nУдалите предыдущую установку."
		Abort
	${EndIf}
; check for snmp service
	services::IsServiceInstalled 'snmp'
	Pop $0
	StrCmp $0 "No" 0 NoAbort2
		MessageBox MB_OK|MB_ICONSTOP "Служба SNMP не установлена.$\n$\nУстановка не возможна."
		Abort
	NoAbort2:
; set sections options
; ntp snmp section
	SectionGetFlags ${ntp_snmp} $0
	IntOp $0 $0 ^ ${SF_SELECTED}
	SectionSetFlags ${ntp_snmp} $0
; configuration files group section
	SectionGetFlags ${gr} $0
	IntOp $0 $0 | ${SF_RO}
	SectionSetFlags ${gr} $0
; configuration files sections
	SectionGetFlags ${c40HLB02} $0
	IntOp $0 $0 ^ ${SF_SELECTED}
	SectionSetFlags ${c40HLB02} $0

	SectionGetFlags ${c40HLB03_01} $0
	IntOp $0 $0 ^ ${SF_SELECTED}
	SectionSetFlags ${c40HLB03_01} $0

	SectionGetFlags ${c40HLB03_02} $0
	IntOp $0 $0 ^ ${SF_SELECTED}
	SectionSetFlags ${c40HLB03_02} $0

	SectionGetFlags ${c40HLB04_01} $0
	IntOp $0 $0 ^ ${SF_SELECTED}
	SectionSetFlags ${c40HLB04_01} $0

	SectionGetFlags ${c40HLB04_02} $0
	IntOp $0 $0 ^ ${SF_SELECTED}
	SectionSetFlags ${c40HLB04_02} $0

	SectionGetFlags ${c40HLB05_01} $0
	IntOp $0 $0 ^ ${SF_SELECTED}
	SectionSetFlags ${c40HLB05_01} $0

	SectionGetFlags ${c40HLB05_02} $0
	IntOp $0 $0 ^ ${SF_SELECTED}
	SectionSetFlags ${c40HLB05_02} $0

	SectionGetFlags ${c40HLB06_01} $0
	IntOp $0 $0 ^ ${SF_SELECTED}
	SectionSetFlags ${c40HLB06_01} $0

	SectionGetFlags ${c40HLB06_02} $0
	IntOp $0 $0 ^ ${SF_SELECTED}
	SectionSetFlags ${c40HLB06_02} $0

	SectionGetFlags ${c40HLD01_01} $0
	IntOp $0 $0 ^ ${SF_SELECTED}
	SectionSetFlags ${c40HLD01_01} $0

; set selected configuration file by default
	StrCpy $RADIOBUTTON ${c40HLB01}
FunctionEnd

; function call then change selection on component page
Function .onSelChange

	!insertmacro StartRadioButtons $RADIOBUTTON

	!insertmacro RadioButton ${c40HLB01}
	!insertmacro RadioButton ${c40HLB02}
	!insertmacro RadioButton ${c40HLB03_01}
	!insertmacro RadioButton ${c40HLB03_02}
	!insertmacro RadioButton ${c40HLB04_01}
	!insertmacro RadioButton ${c40HLB04_02}
	!insertmacro RadioButton ${c40HLB05_01}
	!insertmacro RadioButton ${c40HLB05_02}
	!insertmacro RadioButton ${c40HLB06_01}
	!insertmacro RadioButton ${c40HLB06_02}
	!insertmacro RadioButton ${c40HLD01_01}

	!insertmacro EndRadioButtons

FunctionEnd
; *** ------------------------------------------------------------------------------------------ ***

; *** ------------------------------------------------------------------------------------------ ***
; uninstall section 

  Section "Uninstall"
; stop service (if running)
	DetailPrint "Остановка сервиса"
	services::IsServiceRunning 'dac'
	Pop $0
	StrCmp $0 "Yes" 0 continue1
		services::SendServiceCommand 'stop' 'dac'	
	continue1:
; delete service registry information
	DetailPrint "Удаление регистрации сервиса"
	services::IsServiceInstalled 'dac'
	Pop $0
	StrCmp $0 "Yes" 0 continue2
		StrCpy $0 "$INSTDIR\bin\dac.exe -u"
		DetailPrint $0
		ExecCmd::exec $0 ""
	 	Pop $0
		${If} $0 != 0
			MessageBox MB_OK "Удаление регистрации сервиса не произведено.$\n$\nПопробуйте провести удаление регистрации самостоятельно."
		${EndIf}
	continue2:
; delete ntp snmp plugin (if installed)
; determine for installation
	ReadRegStr $1 HKLM Software\ntpext\CurrentVersion "Pathname"
	StrLen $1 $1
	${If} $1 != 0
; delete ntp snmp plugin registry information 	
		DetailPrint "Удаление ntp snmp плагина"
		DeleteRegValue HKLM "SYSTEM\CurrentControlSet\Services\SNMP\ExtensionAgents" "ntpext"
		DeleteRegKey HKLM "Software\ntpext"
; restart snmp service
		DetailPrint "Перезапуск службы snmp"
		services::SendServiceCommand 'stop' 'snmp'
		Sleep 5000
		services::SendServiceCommand 'start' 'snmp'
	${EndIf}
; delete base files
	Delete "$INSTDIR\*.*"
	Delete "$INSTDIR\bin\*.*"
	Delete "$INSTDIR\etc\*.*"
	Delete "$INSTDIR\log\*.*"
; delete uninstall file
	Delete "$INSTDIR\Uninstall.exe"
; delete installation directory
	RMDir "$INSTDIR\bin"
	RMDir "$INSTDIR\etc"
	RMDir "$INSTDIR\log"
	RMDir "$INSTDIR"
; delete start menu shortcuts
	!insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuFolder
    
	Delete "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk"
	Delete "$SMPROGRAMS\$StartMenuFolder\Start.lnk"
	Delete "$SMPROGRAMS\$StartMenuFolder\Stop.lnk"  
	Delete "$SMPROGRAMS\$StartMenuFolder\*.*"
	RMDir "$SMPROGRAMS\$StartMenuFolder"
; delete registry information about profgram
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\dac"
  DeleteRegKey HKLM "Software\dac"
; end
  SectionEnd
; *** ------------------------------------------------------------------------------------------ ***

; *** ------------------------------------------------------------------------------------------ ***
;; No pops. It just aborts inside the function, or returns if all is well.
;; Change this if you like.
Function AbortIfBadFramework
 
  ;Save the variables in case something else is using them
  Push $0
  Push $1
  Push $2
  Push $3
  Push $4
  Push $R1
  Push $R2
  Push $R3
  Push $R4
  Push $R5
  Push $R6
  Push $R7
  Push $R8
 
  StrCpy $R5 "0"
  StrCpy $R6 "0"
  StrCpy $R7 "0"
  StrCpy $R8 "0.0.0"
  StrCpy $0 0
 
  loop:
 
  ;Get each sub key under "SOFTWARE\Microsoft\NET Framework Setup\NDP"
  EnumRegKey $1 HKLM "SOFTWARE\Microsoft\NET Framework Setup\NDP" $0
  StrCmp $1 "" done ;jump to end if no more registry keys
  IntOp $0 $0 + 1
  StrCpy $2 $1 1 ;Cut off the first character
  StrCpy $3 $1 "" 1 ;Remainder of string
 
  ;Loop if first character is not a 'v'
  StrCmpS $2 "v" start_parse loop
 
  ;Parse the string
  start_parse:
  StrCpy $R1 ""
  StrCpy $R2 ""
  StrCpy $R3 ""
  StrCpy $R4 $3
 
  StrCpy $4 1
 
  parse:
  StrCmp $3 "" parse_done ;If string is empty, we are finished
  StrCpy $2 $3 1 ;Cut off the first character
  StrCpy $3 $3 "" 1 ;Remainder of string
  StrCmp $2 "." is_dot not_dot ;Move to next part if it's a dot
 
  is_dot:
  IntOp $4 $4 + 1 ; Move to the next section
  goto parse ;Carry on parsing
 
  not_dot:
  IntCmp $4 1 major_ver
  IntCmp $4 2 minor_ver
  IntCmp $4 3 build_ver
  IntCmp $4 4 parse_done
 
  major_ver:
  StrCpy $R1 $R1$2
  goto parse ;Carry on parsing
 
  minor_ver:
  StrCpy $R2 $R2$2
  goto parse ;Carry on parsing
 
  build_ver:
  StrCpy $R3 $R3$2
  goto parse ;Carry on parsing
 
  parse_done:
 
  IntCmp $R1 $R5 this_major_same loop this_major_more
  this_major_more:
  StrCpy $R5 $R1
  StrCpy $R6 $R2
  StrCpy $R7 $R3
  StrCpy $R8 $R4
 
  goto loop
 
  this_major_same:
  IntCmp $R2 $R6 this_minor_same loop this_minor_more
  this_minor_more:
  StrCpy $R6 $R2
  StrCpy $R7 R3
  StrCpy $R8 $R4
  goto loop
 
  this_minor_same:
  IntCmp R3 $R7 loop loop this_build_more
  this_build_more:
  StrCpy $R7 $R3
  StrCpy $R8 $R4
  goto loop
 
  done:
 
  ;Have we got the framework we need?
  IntCmp $R5 ${MIN_FRA_MAJOR} max_major_same fail end
  max_major_same:
  IntCmp $R6 ${MIN_FRA_MINOR} max_minor_same fail end
  max_minor_same:
  IntCmp $R7 ${MIN_FRA_BUILD} end fail end
 
  fail:
  StrCmp $R8 "0.0.0" no_framework
  goto wrong_framework
 
  no_framework:
  MessageBox MB_OK|MB_ICONSTOP ".NET Framework не установлен.$\n$\n Для работы приложения требуется .NET Framework 2.0."
  abort
 
  wrong_framework:
  MessageBox MB_OK|MB_ICONSTOP ".NET Framework не установлен.$\n$\n Для работы приложения требуется .NET Framework 2.0."
  abort
 
  end:
 
  ;Pop the variables we pushed earlier
  Pop $R8
  Pop $R7
  Pop $R6
  Pop $R5
  Pop $R4
  Pop $R3
  Pop $R2
  Pop $R1
  Pop $4
  Pop $3
  Pop $2
  Pop $1
  Pop $0
 
FunctionEnd
; *** ------------------------------------------------------------------------------------------ ***
  