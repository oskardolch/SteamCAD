SUBDIRS = ../dxflib

AM_CPPFLAGS = $(GTK_CFLAGS)

libSteamCAD_core_a_LIBADD = ../dxflib/libdxflib.a

noinst_LIBRARIES = libSteamCAD_core.a
libSteamCAD_core_a_SOURCES = DDataTypes.cpp DDataTypes.hpp DMath.cpp DMath.hpp \
    DTopo.cpp DTopo.hpp DDrawTypes.cpp DDrawTypes.hpp \
    DPrimitive.cpp DPrimitive.hpp DLine.cpp DLine.hpp \
    DCircle.cpp DCircle.hpp DEllipse.cpp DEllipse.hpp \
    DArcElps.cpp DArcElps.hpp DHyper.cpp DHyper.hpp \
    DParabola.cpp DParabola.hpp DSpline.cpp DSpline.hpp \
    DEvolv.cpp DEvolv.hpp DExpCairo.cpp DExpCairo.hpp \
    DParser.cpp DParser.hpp DExpDXF.cpp DExpDXF.hpp

