# Linux Automated
The project should support the GNU automated build now. So running:
```
$ sudo apt-get install gcc libcairo2-dev gtk+-dev
$ git clone https://github.com/oskardolch/SteamCAD.git
$ cd SteamCAD
$ ./configure
$ make
$ sudo make install
```
should install the software, including the start menu launcher.

If the automated build fails for some reason, and the autotools are installed
on the build nachine, you can try to fix it by issuing the following command:
```
$ aclocal
$ autoconf
$ automake
```
Then repeate the ./configure steps.

# Ubuntu or non-MATE environment
The install prerequisities might differ on Ubuntu, or on any environment using neither
MATE nor a GTK+-2.0 based desktop system. In this case the needed libraries might be
`libgtk2.0-dev`, `libgtk+-2.0.dev` or so.

On Ubuntu it might also be good to install `libcanberra-gtk-dev`, although SteamCAD
does not depend on it, GTK might issue a worning if this library is not installed.

# Linux Manual
It still possible to compile the software without GNU build utilities.

Typical steps on a Debian based systems would be as follows:
```
$ sudo apt-get install gcc libcairo2-dev gtk+-dev
$ git clone https://github.com/oskardolch/SteamCAD.git
$ cd SteamCAD
$ mkdir Build
$ make -fmakefile.gcc libdxflib.a
$ make -fmakefile.gcc main
```
Also follow the more detailed instrcutions in the pdf manual.

# Windows
Both the provided MS Windows builds were compiled with MinGW64. The tricky part is, that the SteamCAD.exe
is linked to cairo.dll, which was build from Cairo source code. If you want to build SteamCAD using MinGW,
you can use the delivered libcairo.dll.a binary file to link it to the delivered cairo.dll library. In this
case, you first need to extract Cairo source into "..\Cairo\cairo-1.14.8" directory. If you have a different
version of Cairo, you must change the makefile.mingw and put the correct path to CAIROINC. Then you need
to copy the file "cairo-features.h" from the SteamCAD\cairo\src" to "..\Cairo\cairo-1.14.8\src". Finally
create folders Build32 and Build64 under the SteamCAD root folder, copy the files cairo.dll and libcairo.dll.a
to the corresponding BuildXX folder and issue the commands:
```
> compile32
```
or
```
> compile64
```
respectively.

If you want to use another compiler, such as the MS' one, you will need to compile the cairo.dll from the
source code, which involves compiling of other libraries, namely zlib, pixman and libpng. And you may also
need the cairo.def export definition file, which can be found in the cairo folder.

# Manual
The manual is written in LaTeX, so to build it is quite simple. Navigate to the Manual folder and issue
the following command:
```
$ latex SteamCAD
```
You may need to issue this command twice or even three times in order to get all indices and cross references
in sync. If you want the pdf version, follow the LaTeX build by the following command:
```
$ dvipdfm SteamCAD
```
