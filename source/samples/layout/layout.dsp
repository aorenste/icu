# Microsoft Developer Studio Project File - Name="layout" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=layout - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "layout.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "layout.mak" CFG="layout - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "layout - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "layout - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "layout - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /G6 /MD /W3 /GX /O2 /I "..\..\..\include" /I "..\..\..\include\layout" /I "..\..\extra\scrptrun" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "UNICODE" /D _WIN32_WINNT=0X500 /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 ..\..\..\lib\icule.lib ..\..\..\lib\icuuc.lib ..\..\..\lib\icuin.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "layout - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /G6 /MDd /W3 /Gm /GX /ZI /Od /I "..\..\..\include" /I "..\..\..\include\layout" /I "..\..\extra\scrptrun" /I "..\..\common" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "UNICODE" /D _WIN32_WINNT=0X500 /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 ..\..\..\lib\iculed.lib ..\..\..\lib\icuucd.lib ..\..\..\lib\icuind.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "layout - Win32 Release"
# Name "layout - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\cmaps.cpp
# End Source File
# Begin Source File

SOURCE=.\FontMap.cpp
# End Source File
# Begin Source File

SOURCE=.\GDIFontInstance.cpp
# End Source File
# Begin Source File

SOURCE=.\GDIFontMap.cpp
# End Source File
# Begin Source File

SOURCE=.\GDIGUISupport.cpp
# End Source File
# Begin Source File

SOURCE=.\layout.cpp
# End Source File
# Begin Source File

SOURCE=.\LayoutSample.rc
# End Source File
# Begin Source File

SOURCE=.\paragraph.cpp
# End Source File
# Begin Source File

SOURCE=.\ParagraphLayout.cpp
# End Source File
# Begin Source File

SOURCE=.\RenderingFontInstance.cpp
# End Source File
# Begin Source File

SOURCE=..\..\extra\scrptrun\scrptrun.cpp
# End Source File
# Begin Source File

SOURCE=.\UnicodeReader.cpp
# End Source File
# Begin Source File

SOURCE=.\Utilities.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\cmaps.h
# End Source File
# Begin Source File

SOURCE=.\FontMap.h
# End Source File
# Begin Source File

SOURCE=.\GDIFontInstance.h
# End Source File
# Begin Source File

SOURCE=.\GDIFontMap.h
# End Source File
# Begin Source File

SOURCE=.\GDIGUISupport.h
# End Source File
# Begin Source File

SOURCE=.\GUISupport.h
# End Source File
# Begin Source File

SOURCE=.\paragraph.h
# End Source File
# Begin Source File

SOURCE=.\ParagraphLayout.h
# End Source File
# Begin Source File

SOURCE=.\RenderingFontInstance.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=..\..\extra\scrptrun\scrptrun.h
# End Source File
# Begin Source File

SOURCE=.\sfnt.h
# End Source File
# Begin Source File

SOURCE=.\UnicodeReader.h
# End Source File
# Begin Source File

SOURCE=.\Utilities.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
