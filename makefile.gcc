
CC = g++

BUILDDIR = Build/

CFLAGS = -Wall \
	-DG_DISABLE_DEPRECATED 	 	\
	-DGDK_PIXBUF_DISABLE_DEPRECATED \
    -DGDK_DISABLE_DEPRECATED

ARFLAGS=rcs

CARGS = `pkg-config --cflags gtk+-2.0 gthread-2.0 gio-2.0`
GCARGS = `pkg-config --cflags --libs gtk+-2.0 gthread-2.0 gio-2.0`

# dxflib

dxflibobj = dl_writer_ascii.o dl_dxf.o
dxflibmod = $(addprefix $(BUILDDIR), $(dxflibobj))

#cairo.dll: $(OBJECTS)
#	@$(CP) -s -pie -o$(BUILDDIR)/$@ win32/cairo.def -L./$(BUILDDIR) $(MODS) $(CAIRO_LIBS) \
#	-static-libgcc -static-libstdc++ -mdll

libdxflib.a: $(dxflibobj)
	@$(AR) $(ARFLAGS) $(BUILDDIR)$@ $(dxflibmod)

#libcairo.dll.a : cairo.h win32/cairo.def
#	$(DL) -d win32/cairo.def -D cairo.dll -k -A -l $(BUILDDIR)/libcairo.dll.a $(BUILDDIR)/cairo.o


# Template command for compiling dxflib
%.o : dxflib/%.cpp
	$(CC) -c $< -o $(BUILDDIR)$@

# dxflib sources
dl_writer_ascii.o : dxflib/dl_writer_ascii.cpp dxflib/dl_writer_ascii.h
dl_dxf.o : dxflib/dl_dxf.cpp dxflib/dl_dxf.h


DGuiobj = SteamCAD.o DMenu.o DApplication.o DDataTypes.o DMath.o DTopo.o \
    DDrawTypes.o DPrimitive.o DLine.o DCircle.o DEllipse.o DArcElps.o \
    DHyper.o DParabola.o DSpline.o DEvolv.o DExpCairo.o DParser.o \
    DFileMenu.o DModeMenu.o DEditMenu.o DViewToolsMenu.o DFileSetupDlg.o \
    DLineStyleDlg.o DDimEditDlg.o DStatDlg.o DSnapDlg.o DScaleDlg.o DExpDXF.o
DGuimod = $(addprefix $(BUILDDIR), $(DGuiobj)) \
	-L./Build -Wl,-rpath,/home/pavel/Programs/SteamCAD/Build

%.o : Linux/%.cpp
	$(CC) $(CFLAGS) -c $< -o $(BUILDDIR)$@ $(CARGS)

SteamCAD.o : Linux/SteamCAD.cpp Linux/DApplication.hpp
DApplication.o : Linux/DApplication.cpp Linux/DApplication.hpp Linux/DLxBaseDefs.h \
    Linux/DMenu.hpp Source/DDrawTypes.hpp Source/DMath.hpp Source/DExpCairo.hpp \
    Linux/DFileSetupDlg.hpp Linux/DLineStyleDlg.hpp Linux/DStatDlg.hpp \
    Linux/DSnapDlg.hpp Linux/DScaleDlg.hpp Source/DExpDXF.hpp
DMenu.o : Linux/DMenu.cpp Linux/DMenu.hpp Linux/DApplication.hpp \
    Linux/DFileMenu.hpp Linux/DModeMenu.hpp Linux/DEditMenu.hpp \
    Linux/DViewToolsMenu.hpp
DFileMenu.o : Linux/DFileMenu.cpp Linux/DFileMenu.hpp Linux/DLxBaseDefs.h
DModeMenu.o : Linux/DModeMenu.cpp Linux/DModeMenu.hpp Linux/DLxBaseDefs.h
DEditMenu.o : Linux/DEditMenu.cpp Linux/DEditMenu.hpp Linux/DLxBaseDefs.h
DViewToolsMenu.o : Linux/DViewToolsMenu.cpp Linux/DViewToolsMenu.hpp Linux/DLxBaseDefs.h
DFileSetupDlg.o : Linux/DFileSetupDlg.cpp Linux/DFileSetupDlg.hpp Source/DParser.hpp \
    Linux/DLxBaseDefs.h
DLineStyleDlg.o : Linux/DLineStyleDlg.cpp Linux/DLineStyleDlg.hpp Source/DDataTypes.hpp \
    Linux/DLxBaseDefs.h Linux/DFileSetupDlg.hpp
DDimEditDlg.o : Linux/DDimEditDlg.cpp Linux/DDimEditDlg.hpp Source/DDataTypes.hpp \
    Linux/DLxBaseDefs.h Linux/DFileSetupDlg.hpp
DStatDlg.o : Linux/DStatDlg.cpp Linux/DStatDlg.hpp Linux/DLxBaseDefs.h
DSnapDlg.o : Linux/DSnapDlg.cpp Linux/DSnapDlg.hpp Linux/DLxBaseDefs.h
DScaleDlg.o : Linux/DScaleDlg.cpp Linux/DScaleDlg.hpp Source/DDataTypes.hpp \
    Linux/DLxBaseDefs.h Linux/DFileSetupDlg.hpp

%.o : Source/%.cpp
	$(CC) $(CFLAGS) -c $< -o $(BUILDDIR)$@ $(CARGS)

DDataTypes.o : Source/DDataTypes.cpp Source/DDataTypes.hpp Source/DTopo.hpp \
	Source/DMath.hpp
DMath.o : Source/DMath.cpp Source/DMath.hpp
DTopo.o : Source/DTopo.cpp Source/DTopo.hpp Source/DMath.hpp
DDrawTypes.o : Source/DDrawTypes.cpp Source/DDrawTypes.hpp Source/DDataTypes.hpp \
	Source/DLine.hpp Source/DCircle.hpp Source/DEllipse.hpp Source/DPrimitive.hpp \
	Source/DArcElps.hpp Source/DHyper.hpp Source/DParabola.hpp Source/DSpline.hpp \
	Source/DEvolv.hpp Source/DParser.hpp
DPrimitive.o : Source/DPrimitive.cpp Source/DPrimitive.hpp Source/DDataTypes.hpp \
	Source/DMath.hpp
DLine.o : Source/DLine.cpp Source/DLine.hpp Source/DDataTypes.hpp \
	Source/DPrimitive.hpp
DCircle.o : Source/DCircle.cpp Source/DCircle.hpp Source/DDataTypes.hpp \
	Source/DPrimitive.hpp Source/DMath.hpp
DEllipse.o : Source/DEllipse.cpp Source/DEllipse.hpp Source/DDataTypes.hpp \
	Source/DPrimitive.hpp Source/DMath.hpp
DArcElps.o : Source/DArcElps.cpp Source/DArcElps.hpp Source/DDataTypes.hpp \
	Source/DPrimitive.hpp Source/DMath.hpp
DHyper.o : Source/DHyper.cpp Source/DHyper.hpp Source/DDataTypes.hpp \
	Source/DPrimitive.hpp Source/DMath.hpp
DParabola.o : Source/DParabola.cpp Source/DParabola.hpp Source/DDataTypes.hpp \
	Source/DPrimitive.hpp Source/DMath.hpp
DSpline.o : Source/DSpline.cpp Source/DSpline.hpp Source/DDataTypes.hpp \
	Source/DPrimitive.hpp Source/DMath.hpp
DEvolv.o : Source/DEvolv.cpp Source/DEvolv.hpp Source/DDataTypes.hpp \
	Source/DPrimitive.hpp Source/DMath.hpp
DExpCairo.o : Source/DExpCairo.cpp Source/DExpCairo.hpp Source/DDataTypes.hpp \
	Source/DDrawTypes.hpp Source/DPrimitive.hpp Source/DParser.hpp
DParser.o : Source/DParser.cpp Source/DParser.hpp Source/DMath.hpp Source/DDataTypes.hpp
DExpDXF.o : Source/DExpDXF.cpp Source/DExpDXF.hpp Source/DDataTypes.hpp \
	Source/DDrawTypes.hpp Source/DPrimitive.hpp Source/DParser.hpp


main: $(DGuiobj)
	$(CC) -o $(BUILDDIR)SteamCAD $(DGuimod) $(GCARGS) -ldxflib


clean: 
	rm -f $(BUILDDIR)*.o

