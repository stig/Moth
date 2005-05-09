#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT(moth, 1.2.0, stig@brautaset.org)
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_MACRO_DIR([config])

AM_INIT_AUTOMAKE

AC_CONFIG_SRCDIR([gmoth.c])
AC_CONFIG_HEADER([config/config.h])

# Checks for programs.
AC_PROG_CC

# Check for libraries.
AC_CHECK_LIB(options, opt_val, [], [ AC_MSG_ERROR([
  Fatal error -- the options library is missing.
  options is available at http://brautaset.org/projects/options/
  ])
])
AC_CHECK_LIB(ggtl, ggtl_new, [], [ AC_MSG_ERROR([
  Fatal error -- the GGTL library is missing.
  GGTL is available at http://brautaset.org/projects/ggtl/
  ])
])
AC_CHECK_LIB(glut, glutInit)

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h string.h ggtl/core.h options/opt.h sl/sl.h])


# Checks for typedefs, structures, and compiler characteristics.
AC_CONFIG_FILES([Makefile])
AC_OUTPUT

