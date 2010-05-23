# Microsoft Developer Studio Project File - Name="npbasic" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=npbasic - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "npbasic.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "npbasic.mak" CFG="npbasic - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "npbasic - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "npbasic - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "npbasic - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "NPBASIC_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "C:\gecko-sdk\include" /D "XPCOM_GLUE" /D "NDEBUG" /D "MOZILLA_STRICT_API" /D "XP_WIN" /D "XP_WIN32" /D "_X86_" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "NPBASIC_EXPORTS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib wsock32.lib shell32.lib wininet.lib c:\gecko-sdk\lib\xpcom.lib c:\gecko-sdk\lib\xpcomglue_s.lib /nologo /dll /map /machine:I386 /out:"Release/nppcman.dll"

!ELSEIF  "$(CFG)" == "npbasic - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "NPBASIC_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\..\include" /I "..\..\..\..\gecko-sdk\include" /I "..\..\..\..\gecko-sdk\plugin\include" /I "..\..\..\..\gecko-sdk\nspr\include" /I "..\..\..\..\gecko-sdk\java\include" /I "..\..\..\..\gecko-sdk\xpcom\include" /I "..\..\..\..\gecko-sdk\dom\include" /I "..\..\..\..\gecko-sdk" /D "_DEBUG" /D "MOZILLA_STRICT_API" /D "XP_WIN" /D "_X86_" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "NPBASIC_EXPORTS" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 wsock32.lib wininet.lib ..\..\..\..\gecko-sdk\lib\xpcom.lib ..\..\..\..\gecko-sdk\lib\xpcomglue_s.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /out:"D:\ºô¸ô\Mozilla Firefox\plugins\nppcman.dll" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "npbasic - Win32 Release"
# Name "npbasic - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\basic.def
# End Source File
# Begin Source File

SOURCE=.\caret.cpp
# End Source File
# Begin Source File

SOURCE=.\helperwnd.cpp
# End Source File
# Begin Source File

SOURCE=.\MouseGestures.cpp
# End Source File
# Begin Source File

SOURCE=..\np_entry.cpp
# End Source File
# Begin Source File

SOURCE=..\npn_gate.cpp
# End Source File
# Begin Source File

SOURCE=..\npp_gate.cpp
# End Source File
# Begin Source File

SOURCE=.\nsScriptablePeer.cpp
# End Source File
# Begin Source File

SOURCE=.\plugin.cpp
# End Source File
# Begin Source File

SOURCE=.\site.cpp
# End Source File
# Begin Source File

SOURCE=.\Sock.cpp
# End Source File
# Begin Source File

SOURCE=.\stringutil.cpp
# End Source File
# Begin Source File

SOURCE=.\telnetcon.cpp
# End Source File
# Begin Source File

SOURCE=.\telnetview.cpp
# End Source File
# Begin Source File

SOURCE=.\termdata.cpp
# End Source File
# Begin Source File

SOURCE=.\termview.cpp
# End Source File
# Begin Source File

SOURCE=.\Ucs2Conv.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\caret.h
# End Source File
# Begin Source File

SOURCE=.\helperwnd.h
# End Source File
# Begin Source File

SOURCE=.\MouseGestures.h
# End Source File
# Begin Source File

SOURCE=.\nsIPCMan.h
# End Source File
# Begin Source File

SOURCE=.\nsScriptablePeer.h
# End Source File
# Begin Source File

SOURCE=.\plugin.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\site.h
# End Source File
# Begin Source File

SOURCE=.\Sock.h
# End Source File
# Begin Source File

SOURCE=.\stringutil.h
# End Source File
# Begin Source File

SOURCE=.\telnetcon.h
# End Source File
# Begin Source File

SOURCE=.\telnetview.h
# End Source File
# Begin Source File

SOURCE=.\termdata.h
# End Source File
# Begin Source File

SOURCE=.\termview.h
# End Source File
# Begin Source File

SOURCE=.\Ucs2Conv.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\accept.ico
# End Source File
# Begin Source File

SOURCE=.\basic.rc
# End Source File
# Begin Source File

SOURCE=.\res\dragtext_cur.cur
# End Source File
# Begin Source File

SOURCE=.\res\exit.cur
# End Source File
# Begin Source File

SOURCE=.\res\eye.cur
# End Source File
# Begin Source File

SOURCE=.\res\pcmanx.ico
# End Source File
# Begin Source File

SOURCE=.\res\shell32_269.ico
# End Source File
# Begin Source File

SOURCE=.\res\upg.ico
# End Source File
# Begin Source File

SOURCE=.\res\urlmon_104.ico
# End Source File
# End Group
# End Target
# End Project
