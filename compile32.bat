if "%1" == "a" mingw32-make -fmakefile.mingw BUILDDIR=Build32 clean
mingw32-make -fmakefile.mingw PREFIX=i686-w64-mingw32- TOOLPFX=i686-w64-mingw32- BUILDDIR=Build32 SUFFIX=32 libdxflib.a 2> libdxflib.log
mingw32-make -fmakefile.mingw PREFIX=i686-w64-mingw32- TOOLPFX=i686-w64-mingw32- BUILDDIR=Build32 SUFFIX=32 SteamCAD 2> SteamCAD.log
