#------------------------------------------------------------------------
#  Copyright 2009 (c) Jeff Brown <spadix@users.sourceforge.net>
#
#  This file is part of the ZBar Bar Code Reader.
#
#  The ZBar Bar Code Reader is free software; you can redistribute it
#  and/or modify it under the terms of the GNU Lesser Public License as
#  published by the Free Software Foundation; either version 2.1 of
#  the License, or (at your option) any later version.
#
#  The ZBar Bar Code Reader is distributed in the hope that it will be
#  useful, but WITHOUT ANY WARRANTY; without even the implied warranty
#  of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU Lesser Public License for more details.
#
#  You should have received a copy of the GNU Lesser Public License
#  along with the ZBar Bar Code Reader; if not, write to the Free
#  Software Foundation, Inc., 51 Franklin St, Fifth Floor,
#  Boston, MA  02110-1301  USA
#
#  http://sourceforge.net/projects/zbar
#------------------------------------------------------------------------

!ifndef VERSION
  !define VERSION "test"
!endif

!define ZBAR_KEY "Software\ZBar"
!define UNINSTALL_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\ZBar"

OutFile zbar-${VERSION}-setup.exe

SetCompressor /SOLID bzip2

InstType "Typical"
InstType "Full"

InstallDir $PROGRAMFILES\ZBar
InstallDirRegKey HKLM ${ZBAR_KEY} "InstallDir"

!define SMPROG_ZBAR "$SMPROGRAMS\ZBar Bar Code Reader"

# do we need admin to install DLL and uninstall info?
RequestExecutionLevel admin


!include "MUI2.nsh"
!include "Memento.nsh"


Name "ZBar"
Caption "ZBar ${VERSION} Setup"


# Check that ImageMagick is installed
Function .onInit
    SearchPath $0 "CORE_RL_wand_.dll"
    IfErrors 0 goinstall
        MessageBox MB_OKCANCEL|MB_ICONSTOP \
                   "ImageMagick was not found in your PATH!$\n\
                   $\n\
                   The zbarimg program will not be able to scan image files \
                   unless you install ImageMagick$\n\
                   $\n\
                   See the README for more details$\n\
                   $\n\
                   Press OK to continue anyway...$\n\
                   Press Cancel to abort the installation..." \
                   /SD IDCANCEL \
                   IDOK goinstall
        Abort
goinstall:
FunctionEnd


!define MEMENTO_REGISTRY_ROOT HKLM
!define MEMENTO_REGISTRY_KEY ${UNINSTALL_KEY}

!define MUI_ABORTWARNING
!define MUI_FINISHPAGE_NOAUTOCLOSE
!define MUI_UNFINISHPAGE_NOAUTOCLOSE

!define MUI_ICON ${NSISDIR}\Contrib\Graphics\Icons\orange-install.ico
!define MUI_UNICON ${NSISDIR}\Contrib\Graphics\Icons\orange-uninstall.ico

!define MUI_WELCOMEFINISHPAGE_BITMAP ${NSISDIR}\Contrib\Graphics\Wizard\orange.bmp
!define MUI_UNWELCOMEFINISHPAGE_BITMAP ${NSISDIR}\Contrib\Graphics\Wizard\orange-uninstall.bmp

!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_BITMAP ${NSISDIR}\Contrib\Graphics\Header\orange.bmp
!define MUI_HEADERIMAGE_UNBITMAP ${NSISDIR}\Contrib\Graphics\Header\orange-uninstall.bmp

!define MUI_WELCOMEPAGE_TITLE "Welcome to the ZBar ${VERSION} Setup Wizard"
!define MUI_WELCOMEPAGE_TEXT \
    "This wizard will guide you through the installation of the \
    ZBar Bar Code Reader version ${VERSION}."

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "share\doc\zbar\COPYING.LIB"

!define MUI_COMPONENTSPAGE_SMALLDESC
!define MUI_COMPONENTSPAGE_CHECKBITMAP ${NSISDIR}\Contrib\Graphics\Checks\simple-round2.bmp

!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES

Function ShowREADME
     Exec '"notepad.exe" "$INSTDIR\README.windows"'
FunctionEnd

!define MUI_FINISHPAGE_NOREBOOTSUPPORT
!define MUI_FINISHPAGE_SHOWREADME
!define MUI_FINISHPAGE_SHOWREADME_FUNCTION ShowREADME
!define MUI_FINISHPAGE_LINK \
        "Visit the ZBar website for the latest news, FAQs and support"
!define MUI_FINISHPAGE_LINK_LOCATION "http://zbar.sourceforge.net/"

!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"

Section "ZBar Core Files (required)" SecCore
    DetailPrint "Installing ZBar Program and Library..."
    SectionIn 1 2 RO

    SetOutPath $INSTDIR
    File share\doc\zbar\README.windows
    File share\doc\zbar\NEWS
    File share\doc\zbar\TODO
    File share\doc\zbar\COPYING.LIB

    # emit a batch file to add the install directory to the path
    FileOpen $0 zbarvars.bat w
    FileWrite $0 "@rem  Add the ZBar installation directory to the path$\n"
    FileWrite $0 "@rem  so programs may be run from the command prompt$\n"
    FileWrite $0 "@set PATH=%PATH%;$INSTDIR\bin$\n"
    FileWrite $0 "@echo For basic command instructions type:$\n"
    FileWrite $0 "@echo     zbarimg --help$\n"
    FileClose $0

    SetOutPath $INSTDIR\bin
    File bin\libzbar-0.dll
    File bin\zbarimg.exe

    SetOutPath $INSTDIR\doc
    File share\doc\zbar\html\*

    SetOutPath $INSTDIR\examples
    File share\zbar\barcode.png
SectionEnd

#SectionGroup "Start Menu and Desktop Shortcuts" SecShortcuts
    Section "Start Menu Shortcut" SecShortcutsStartMenu
        DetailPrint "Creating Start Menu Shortcut..."
        SectionIn 1 2
        SetOutPath $INSTDIR
        #CreateShortCut "${SMPROG_ZBAR}\ZBar.lnk" "$INSTDIR\ZBar.exe"
        CreateDirectory "${SMPROG_ZBAR}"
        ExpandEnvStrings $0 '%comspec%'
        CreateShortCut "${SMPROG_ZBAR}\Start ZBar Command Prompt.lnk" \
                       $0 "/k $\"$\"$INSTDIR\zbarvars.bat$\"$\"" $0
        CreateShortCut "${SMPROG_ZBAR}\Command Reference.lnk" \
                       "$\"$INSTDIR\doc\zbarimg.html$\""
    SectionEnd

#    Section "Desktop Shortcut" SecShortcutsDesktop
#        DetailPrint "Creating Desktop Shortcut..."
#        SectionIn 1 2
#        SetOutPath $INSTDIR
#        #CreateShortCut "$DESKTOP\ZBar.lnk" "$INSTDIR\ZBar.exe"
#    SectionEnd
#SectionGroupEnd

Section "Development Headers and Libraries" SecDevel
    DetailPrint "Installing ZBar Development Files..."
    SectionIn 2

    SetOutPath $INSTDIR\include
    File include\zbar.h

    SetOutPath $INSTDIR\include\zbar
    File include\zbar\Video.h
    File include\zbar\Exception.h
    File include\zbar\Symbol.h
    File include\zbar\Image.h
    File include\zbar\ImageScanner.h
    File include\zbar\Window.h
    File include\zbar\Processor.h
    File include\zbar\Decoder.h
    File include\zbar\Scanner.h

    SetOutPath $INSTDIR\lib
    File lib\libzbar-0.def
    File lib\libzbar-0.lib

    SetOutPath $INSTDIR\examples
    File share\zbar\scan_image.cpp
    File share\zbar\scan_image.vcproj
SectionEnd

Section -post
    DetailPrint "Creating Registry Keys..."
    SetOutPath $INSTDIR
    WriteRegStr HKLM ${ZBAR_KEY} "InstallDir" $INSTDIR

    # register uninstaller
    WriteRegStr HKLM ${UNINSTALL_KEY} "UninstallString" \
                "$\"$INSTDIR\uninstall.exe$\""
    WriteRegStr HKLM ${UNINSTALL_KEY} "QuietUninstallString" \
                "$\"$INSTDIR\uninstall.exe$\" /S"
    WriteRegStr HKLM ${UNINSTALL_KEY} "InstallLocation" "$\"$INSTDIR$\""

    WriteRegStr HKLM ${UNINSTALL_KEY} "DisplayName" "ZBar Bar Code Reader"
    WriteRegStr HKLM ${UNINSTALL_KEY} "DisplayIcon" "$INSTDIR\zbarimg.exe,0"
    WriteRegStr HKLM ${UNINSTALL_KEY} "DisplayVersion" "${VERSION}"

    WriteRegStr HKLM ${UNINSTALL_KEY} "URLInfoAbout" "http://zbar.sf.net/"
    WriteRegStr HKLM ${UNINSTALL_KEY} "HelpLink" "http://zbar.sf.net/"
    WriteRegDWORD HKLM ${UNINSTALL_KEY} "NoModify" "1"
    WriteRegDWORD HKLM ${UNINSTALL_KEY} "NoRepair" "1"

    DetailPrint "Generating Uninstaller..."
    WriteUninstaller $INSTDIR\uninstall.exe
SectionEnd


Section Uninstall
    DetailPrint "Uninstalling ZBar Bar Code Reader.."

    DetailPrint "Deleting Files..."
    RMDir /r $INSTDIR\examples
    RMDir /r $INSTDIR\include
    RMDir /r $INSTDIR\doc
    RMDir /r $INSTDIR\lib
    RMDir /r $INSTDIR\bin
    Delete $INSTDIR\README.windows
    Delete $INSTDIR\NEWS
    Delete $INSTDIR\TODO
    Delete $INSTDIR\COPYING.LIB
    Delete $INSTDIR\zbarvars.bat
    Delete $INSTDIR\uninstall.exe
    RMDir $INSTDIR

    DetailPrint "Removing Shortcuts..."
    RMDir /r "${SMPROG_ZBAR}"

    DetailPrint "Deleting Registry Keys..."
    DeleteRegKey HKLM ${ZBAR_KEY}
    DeleteRegKey HKLM ${UNINSTALL_KEY}
SectionEnd


!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecCore} \
        "The core files required to use the bar code reader"
#    !insertmacro MUI_DESCRIPTION_TEXT ${SecShortcuts} \
#        "Adds icons to your start menu and/or your desktop for easy access"
    !insertmacro MUI_DESCRIPTION_TEXT ${SecShortcutsStartMenu} \
        "Adds shortcuts to your start menu"
#    !insertmacro MUI_DESCRIPTION_TEXT ${SecShortcutsDesktop} \
#        "Adds an icon on your desktop"
    !insertmacro MUI_DESCRIPTION_TEXT ${SecDevel} \
        "Optional files used to develop other applications using ZBar"
!insertmacro MUI_FUNCTION_DESCRIPTION_END
