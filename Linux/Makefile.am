AUTOMAKE_OPTIONS = foreign

CFLAGS = -O2

SUBDIRS = ../Source

CPPFLAGS = $(GTK_CFLAGS)
LDFLAGS = $(GTK_LIBS)
SteamCAD_LDADD = ../Source/libSteamCAD_core.a ../dxflib/libdxflib.a

AM_CPPFLAGS = -DLOCALEDIR='"$(localedir)"' -DCONFDIR=\"$(sysconfdir)/@PACKAGE@\"

bin_PROGRAMS = SteamCAD
SteamCAD_SOURCES = SteamCAD.cpp DApplication.cpp DApplication.hpp DLxBaseDefs.h \
    DSnapDlg.hpp DScaleDlg.hpp DMenu.cpp DMenu.hpp \
    DFileMenu.cpp DFileMenu.hpp DModeMenu.cpp DModeMenu.hpp \
    DEditMenu.cpp DEditMenu.hpp DViewToolsMenu.cpp DViewToolsMenu.hpp \
    DFileSetupDlg.cpp DFileSetupDlg.hpp DLineStyleDlg.cpp DLineStyleDlg.hpp \
    DDimEditDlg.cpp DDimEditDlg.hpp DStatDlg.cpp DStatDlg.hpp \
    DSnapDlg.cpp DSnapDlg.hpp DScaleDlg.cpp DScaleDlg.hpp

