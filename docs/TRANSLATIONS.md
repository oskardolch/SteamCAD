# Linux version
To translate the Linux source code invoke the following command from the Linux folder. So assuming
you are in the SteamCAD root folder, do
```
$ cd Linux
$ msginit --input=po/SteamCAD.pot --locale=cs --output=po/cs/SteamCAD.po
```
where you replace oll occurances of `cs` by your language code. Then translate all the strings inside
the newly generated SteamCAD.po file. See the `po/cs/SteamCAD.po` for reference. When traslation
is done, create the `mo` file by:
```
$ msgfmt --output-file=po/cs/LC_MESSAGES/steamcad.mo po/cs/SteamCAD.po
```
Note the lowercase in the `mo` file name.

Finally you need to copy the `mo` file into `SteamCAD/po/cs/LC_MESSAGES` folder, modify the `po/Makefile.in`
by adding the two lines for your language code, and run the sequence:
```
$ aclocal
$ autoconf
$ automake
```

You finish the installation by standard `./configure make install` sequence.

You are also welcome to contibute the translated `po` file.

# Windows version
For the windows version, it is sufficient to translate all the strings in the Win32/SteamCAD.rc file and
recompile the software with this new resource file. You are welcome to contribute witht he translated
resource file.

# Manual
To translate the manual, you need to translate the LaTeX source files in the Manual folder (SteamCAD.tex)
and all the files in the Source subfolder. Then compile it following the instruction in BUILD_INSTRUCTIONS.md.
You are welcome to contribute with the translated TeX source files.
