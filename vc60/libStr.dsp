# Microsoft Developer Studio Project File - Name="libStr" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=libStr - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "libStr.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "libStr.mak" CFG="libStr - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "libStr - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "libStr - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "libStr - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "c:\tmp\_tools\libstr\Release"
# PROP Intermediate_Dir "c:\tmp\_tools\libstr\Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\..\include" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x413 /d "NDEBUG"
# ADD RSC /l 0x413 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"c:\tmp\_tools\libstr\Release\libStl.lib"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy  c:\tmp\_tools\libstr\Release\libStl.lib  ..\..\libs_win\Release\ 
# End Special Build Tool

!ELSEIF  "$(CFG)" == "libStr - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "c:\tmp\_tools\libstr\Debug"
# PROP Intermediate_Dir "c:\tmp\_tools\libstr\Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\..\include" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD BASE RSC /l 0x413 /d "_DEBUG"
# ADD RSC /l 0x413 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"c:\tmp\_tools\libstr\Debug\libStl.lib"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy  c:\tmp\_tools\libstr\Debug\libStl.lib  ..\..\libs_win\Debug\ 
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "libStr - Win32 Release"
# Name "libStr - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\stl_append.c
# End Source File
# Begin Source File

SOURCE=..\stl_case.c
# End Source File
# Begin Source File

SOURCE=..\stl_crc32.c
# End Source File
# Begin Source File

SOURCE=..\stl_crypt.c
# End Source File
# Begin Source File

SOURCE=..\stl_date.c
# End Source File
# Begin Source File

SOURCE=..\stl_delete.c
# End Source File
# Begin Source File

SOURCE=..\stl_file.c
# End Source File
# Begin Source File

SOURCE=..\stl_get.c
# End Source File
# Begin Source File

SOURCE=..\stl_getopt.c
# End Source File
# Begin Source File

SOURCE=..\stl_inifile.c
# End Source File
# Begin Source File

SOURCE=..\stl_integer.c
# End Source File
# Begin Source File

SOURCE=..\stl_locate.c
# End Source File
# Begin Source File

SOURCE=..\stl_md5.c
# End Source File
# Begin Source File

SOURCE=..\stl_printf.c
# End Source File
# Begin Source File

SOURCE=..\stl_printf_win.c
# End Source File
# Begin Source File

SOURCE=..\stl_random.c
# End Source File
# Begin Source File

SOURCE=..\stl_ras_win.c
# End Source File
# Begin Source File

SOURCE=..\stl_registry.c
# End Source File
# Begin Source File

SOURCE=..\stl_rs232.c
# End Source File
# Begin Source File

SOURCE=..\stl_safeexit.c
# End Source File
# Begin Source File

SOURCE=..\stl_sql.c
# End Source File
# Begin Source File

SOURCE=..\stl_store.c
# End Source File
# Begin Source File

SOURCE=..\stl_strip.c
# End Source File
# Begin Source File

SOURCE=..\stl_swap.c
# End Source File
# Begin Source File

SOURCE=..\stl_syslog.c
# End Source File
# Begin Source File

SOURCE=..\stl_tcpbase.c
# End Source File
# Begin Source File

SOURCE=..\stl_tcpdns.c
# End Source File
# Begin Source File

SOURCE=..\stl_tcpsrv.c
# End Source File
# Begin Source File

SOURCE=..\stl_thread.c
# End Source File
# Begin Source File

SOURCE=..\stl_timer.c
# End Source File
# Begin Source File

SOURCE=..\stl_win_service.c
# End Source File
# Begin Source File

SOURCE=..\stl_win_shmem.c
# End Source File
# Begin Source File

SOURCE=..\stl_xml.c
# End Source File
# Begin Source File

SOURCE=..\strtl_base.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\internal.h
# End Source File
# Begin Source File

SOURCE=..\stl_crypt.h
# End Source File
# Begin Source File

SOURCE=..\stl_ras.h
# End Source File
# Begin Source File

SOURCE=..\stl_rs232.h
# End Source File
# Begin Source File

SOURCE=..\stl_sql.h
# End Source File
# Begin Source File

SOURCE=..\stl_str.h
# End Source File
# Begin Source File

SOURCE=..\stl_strlnx.h
# End Source File
# Begin Source File

SOURCE=..\stl_strwin.h
# End Source File
# Begin Source File

SOURCE=..\stl_tcp.h
# End Source File
# Begin Source File

SOURCE=..\stl_thread.h
# End Source File
# Begin Source File

SOURCE=..\stl_xml.h
# End Source File
# End Group
# End Target
# End Project
