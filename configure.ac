#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.62])
AC_INIT([SteamCAD], [1.1], [pavel@pamsoft.cz])
AC_CONFIG_HEADERS([Linux/config.h])

AM_INIT_AUTOMAKE([subdir-objects])
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.18.1])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_RANLIB

# Checks for libraries.

PKG_CHECK_MODULES([GTK], [gtk+-2.0 gthread-2.0 gio-2.0 cairo])

# Checks for header files.
AC_CHECK_HEADERS([libintl.h locale.h malloc.h stddef.h stdlib.h string.h strings.h wchar.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([floor memmove setlocale sqrt strcasecmp strchr strrchr strtol])

AC_CONFIG_FILES([Makefile po/Makefile Papers/Makefile Images/steamcad.desktop Images/Makefile])
AC_OUTPUT
