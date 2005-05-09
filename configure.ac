#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(ggtl-games, 1.2pre1, stig@brautaset.org)
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_MACRO_DIR([config])

AM_INIT_AUTOMAKE

AC_CONFIG_SRCDIR([src/moth.c])
AC_CONFIG_HEADER([config/config.h])

AC_ARG_WITH(glut,
  AS_HELP_STRING([--without-glut], [do not use glut (default is to use it if present)]),
  [ac_use_glut=$withval],
  [ac_use_glut=yes])
AM_CONDITIONAL(WITH_GLUT, test x$ac_use_glut = xyes)

# Checks for programs.
AC_PROG_CC

AC_CHECK_LIB(options, opt_val)
AC_CHECK_LIB(ggtl, ggtl_new)
AC_CHECK_LIB(glut, glutInit)

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T

AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT

