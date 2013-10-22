#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.61])
AC_INIT([libbcon], [1.0], [jason.carey@mongodb.com])
AC_CONFIG_HEADERS([config.h])

LT_PREREQ([2.2])
LT_INIT

AM_INIT_AUTOMAKE([1.10])

# Checks for programs.
AC_PROG_CC_C99
AC_CHECK_PROG(PERL, perl, perl)

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([unistd.h sys/types.h error.h])

# Checks for library functions.
PKG_CHECK_MODULES(BSON, libbson-1.0 > 0.2.3)

AC_CONFIG_FILES([
    Makefile
])

AC_OUTPUT
