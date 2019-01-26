SUBDIRS = po Papers Images

AM_CPPFLAGS = $(GTK_CFLAGS) -DLOCALEDIR='"$(localedir)"' -DCONFDIR=\"$(sysconfdir)/@PACKAGE@\"
AM_LDFLAGS = -Wl,-rpath,.
SteamCAD_LDADD = $(GTK_LIBS)

bin_PROGRAMS = SteamCAD
SteamCAD_SOURCES = dxflib/dl_writer_ascii.cpp dxflib/dl_dxf.cpp \
    Source/DDataTypes.cpp Source/DMath.cpp Source/DTopo.cpp \
    Source/DDrawTypes.cpp Source/DPrimitive.cpp Source/DLine.cpp \
    Source/DCircle.cpp Source/DEllipse.cpp Source/DArcElps.cpp \
    Source/DHyper.cpp Source/DParabola.cpp Source/DSpline.cpp \
    Source/DEvolv.cpp Source/DExpCairo.cpp Source/DParser.cpp \
    Source/DExpDXF.cpp \
    Linux/SteamCAD.cpp Linux/DApplication.cpp Linux/DMenu.cpp \
    Linux/DFileMenu.cpp Linux/DModeMenu.cpp Linux/DEditMenu.cpp \
    Linux/DViewToolsMenu.cpp Linux/DFileSetupDlg.cpp \
    Linux/DLineStyleDlg.cpp Linux/DDimEditDlg.cpp Linux/DStatDlg.cpp \
    Linux/DSnapDlg.cpp Linux/DScaleDlg.cpp

