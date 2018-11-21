# Linux version
To enable i18n in the SteamCAD, the [gettext](https://www.gnu.org/software/gettext/) should be implemented.
However, all the strings in the source code are prepared for this using the _() notation.

# Windows version
For the windows version, it is sufficient to translate all the strings in the Win32/SteamCAD.rc file and
recompile the software with this new resource file.

# Manual
To translate the manual, you need to translate the LaTeX source files in the Manual folder (SteamCAD.tex)
and all the files in the Source subfolder. Then compile it following the instruction in CONTRIBUTING.md.
