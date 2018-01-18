# Microsoft Developer Studio Project File - Name="crank" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=crank - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "crank.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "crank.mak" CFG="crank - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "crank - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "crank - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "crank - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "..\bin"
# PROP BASE Intermediate_Dir "..\obj\crank\release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\bin"
# PROP Intermediate_Dir "..\obj\crank\release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /YX /FD /c
# ADD CPP /nologo /Ze /W3 /GX- /O2 /D "WIN32" /D "NDEBUG" /D "PARANOID" /D "USE_SERVICE" /D "USE_QUERY" /D "USE_LOGGING" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"../obj/crank/release/crank.bsc"
LINK32=link.exe
# ADD BASE LINK32 wsock32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 wsock32.lib advapi32.lib shell32.lib /nologo /subsystem:console /pdb:"../obj/crank/release/crank.pdb" /machine:I386
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "crank - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "..\bin"
# PROP BASE Intermediate_Dir "..\obj\crank\debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\bin"
# PROP Intermediate_Dir "..\obj\crank\debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "PARANOID" /D "USE_SERVICE" /D "USE_QUERY" /D "USE_LOGGING" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"../obj/crank/debug/crank-debug.bsc"
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 wsock32.lib advapi32.lib shell32.lib /nologo /subsystem:console /pdb:"../obj/crank/debug/crank-debug.pdb" /debug /machine:I386 /out:"../bin/crank-debug.exe" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "crank - Win32 Release"
# Name "crank - Win32 Debug"
# Begin Source File

SOURCE=.\crank.c
# End Source File
# Begin Source File

SOURCE=.\crank.h
# End Source File
# Begin Source File

SOURCE=.\generate.c
# End Source File
# Begin Source File

SOURCE=.\hash.c
# End Source File
# Begin Source File

SOURCE=.\hash.h
# End Source File
# Begin Source File

SOURCE=.\irc.c
# End Source File
# Begin Source File

SOURCE=.\MemLeakCheck.c
# End Source File
# Begin Source File

SOURCE=.\MemLeakCheck.h
# End Source File
# Begin Source File

SOURCE=.\objects.c
# End Source File
# Begin Source File

SOURCE=.\query.c
# End Source File
# Begin Source File

SOURCE=.\query.h
# End Source File
# Begin Source File

SOURCE=.\sockets.c
# End Source File
# Begin Source File

SOURCE=.\sockets.h
# End Source File
# Begin Source File

SOURCE=.\util.c
# End Source File
# Begin Source File

SOURCE=.\util.h
# End Source File
# End Target
# End Project
