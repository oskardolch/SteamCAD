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
case it is just sufficient to create folder Build32 and Build64 under the SteamCAD root folder, copy the
files cairo.dll and libcairo.dll.a to the corresponding BuildXX folder and issue the commands:
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
