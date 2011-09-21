; dsc installer script for NSIS

; include modern UI
  !include "MUI2.nsh"

; define .NET Framework veersion
  !define MIN_FRA_MAJOR "2"
  !define MIN_FRA_MINOR "0"
  !define MIN_FRA_BUILD "*"

; set program name
  Name "dsc"
; set installer outfile name
  OutFile "dsc_setup.exe"
; enable XP style
  XPStyle on
; set installation directory
  InstallDir "C:\dsc"
; store installlation directory to registry
  InstallDirRegKey HKLM "Software\dsc" ""

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
  !define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\dsc" 
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
	SetOutPath "$INSTDIR"
	File "..\dsc.lock.template"
; store installation folder
	WriteRegStr HKLM "Software\dsc" "" $INSTDIR
; write the uninstall keys
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\dsc" "DisplayName" "dsc"
	WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\dsc" "UninstallString" '"$INSTDIR\Uninstall.exe"'
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\dsc" "NoModify" 1
	WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\dsc" "NoRepair" 1
; create uninstaller
   	WriteUninstaller "Uninstall.exe"
; create start menu shortcuts
	!insertmacro MUI_STARTMENU_WRITE_BEGIN Application

	CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
	CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
	CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Start.lnk" "$INSTDIR\dsc.start.bat"
	CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Stop.lnk" "$INSTDIR\dsc.stop.bat"

	!insertmacro MUI_STARTMENU_WRITE_END
  SectionEnd

; *** configuration files group ***
  SectionGroup /e "конфигурация" gr
; *** configuration files sections ***
	Section "40HLD01" c40HLD01
	SectionEnd	
  SectionGroupEnd

; *** service registr section ***
  Section "-hidden регистрация сервиса"
; determine selected configuration file	
	SectionGetFlags ${c40HLD01} $0
	${If} $0 == 1
		SectionGetText ${c40HLD01} $0
		goto reg
	${EndIf}
; registartion	
	reg:
; generate start and stop scripts
; dsc.start.bat
		DetailPrint "Создание скрипта для запуска dsc.start.bat"
		FileOpen $R0 "$INSTDIR\dsc.start.bat" w
		FileWrite $R0 "$INSTDIR\bin\dsc.exe /detach --directory $INSTDIR --config $INSTDIR\etc\$0.xml"
		FileClose $R0
; dsc.stop.bat
		DetailPrint "Создание скрипта для остановки dsc.stop.bat"
		FileOpen $R0 "$INSTDIR\dsc.stop.bat" w
		FileWrite $R0 "del /f $INSTDIR\dsc.lock"
		FileClose $R0
; register OPC-сервер
		DetailPrint "Регистрация OPC-сервера"
		StrCpy $0 "$INSTDIR\bin\dsc.exe /r"
		DetailPrint $0
		ExecCmd::exec $0 ""
		Pop $0
		${If} $0 != 0
			MessageBox MB_OK "Сервис не зарегистрирован.$\n$\nПопробуйте провести регистрацию самостоятельно"
		${EndIf}
; добавляем в автозагрузку
		DetailPrint "Добавляем в автозагрузку"
		WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Run" "dsc" "$INSTDIR\dsc.start.bat"

; create log directory
		CreateDirectory "$INSTDIR\log"

  SectionEnd

; *** ------------------------------------------------------------------------------------------ ***

; *** ------------------------------------------------------------------------------------------ ***
; set sections description

; define descriptions 
  LangString DESC_base ${LANG_RUSSIAN} "Основные файлы dsc"
  LangString DESC_gr ${LANG_RUSSIAN} "Конфигурационные файлы"

; set descriptions
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
	!insertmacro MUI_DESCRIPTION_TEXT ${base} $(DESC_base)
	!insertmacro MUI_DESCRIPTION_TEXT ${gr} $(DESC_gr)
  !insertmacro MUI_FUNCTION_DESCRIPTION_END

; *** ------------------------------------------------------------------------------------------ ***
; functions

; function call then install start (befor first gui)  
  Function .onInit
; check for .NET Framework
	Call AbortIfBadFramework
; check for already installed
	ReadRegStr $1 HKLM Software\dsc ""
	StrLen $1 $1
	${If} $1 != 0
		MessageBox MB_OK|MB_ICONSTOP "Приложение уже установлено.$\n$\nУдалите предыдущую установку."
		Abort
	${EndIf}
; set sections options
; configuration files group section
	SectionGetFlags ${gr} $0
	IntOp $0 $0 | ${SF_RO}
	SectionSetFlags ${gr} $0
; configuration files sections

; set selected configuration file by default
	StrCpy $RADIOBUTTON ${c40HLD01}
FunctionEnd

; function call then change selection on component page
Function .onSelChange

	!insertmacro StartRadioButtons $RADIOBUTTON

	!insertmacro RadioButton ${c40HLD01}

	!insertmacro EndRadioButtons

FunctionEnd
; *** ------------------------------------------------------------------------------------------ ***

; *** ------------------------------------------------------------------------------------------ ***
; uninstall section 

  Section "Uninstall"
; stop app
	DetailPrint "Останока программы"
	StrCpy $0 "$INSTDIR\dsc.stop.bat"
	DetailPrint $0
	ExecCmd::exec $0 ""
; delete registry information
	DetailPrint "Удаление регистрации OPC-сервера"
	StrCpy $0 "$INSTDIR\bin\dsc.exe /u"
	DetailPrint $0
	ExecCmd::exec $0 ""
; delete from autorun
	DetailPrint "Удаляем из автозагрузки"
	DeleteRegValue HKLM "Software\Microsoft\Windows\CurrentVersion\Run" "dsc"

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
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\dsc"
  DeleteRegKey HKLM "Software\dsc"
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
  MessageBox MB_OK|MB_ICONSTOP ".NET Framework не установлен.$\n$\nДля работы приложения требуется .NET Framework 2.0."
  abort
 
  wrong_framework:
  MessageBox MB_OK|MB_ICONSTOP ".NET Framework не установлен.$\n$\nДля работы приложения требуется .NET Framework 2.0."
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
  