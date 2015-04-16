#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([fgddem2pgsql], [0.2.1], [yellow73.github@gmail.com])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/fgddem2pgsql.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
AC_CHECK_LIB([m], [round])

# Checks for header files.
AC_CHECK_HEADERS([math.h stdint.h stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T

# Checks for library functions.
AC_FUNC_MALLOC

# expat
EXPAT_LIB=""
AC_ARG_WITH(
  expat,
  [  --with-expat=<prfix>],
  [
    expat=$withval
    CPPFLAGS="$CPPFLAGS -I$withval/include"
    LDFLAGS="$LDFLAGS -L$withval/lib"
  ]
)
AC_CHECK_HEADERS(
  expat.h,
  [AC_DEFINE(HAVE_EXPAT_H)],
  [AC_MSG_ERROR([expat.h not found.])]
)
AC_CHECK_LIB(
  expat,
  XML_ParserCreate,
  [EXPAT_LIBS="-lexpat"],
  [AC_MSG_ERROR([libexpat not found.])]
)
AC_SUBST(EXPAT_LIBS)

AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT
