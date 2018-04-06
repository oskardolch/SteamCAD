if "%1" == "a" mingw32-make -fmakefile.mingw BUILDDIR=Build64 clean
mingw32-make -fmakefile.mingw PREFIX=x86_64-w64-mingw32- BUILDDIR=Build64 SUFFIX=64 SteamCAD 2> SteamCAD.log
