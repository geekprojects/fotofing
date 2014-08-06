#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([Fotofing], [0.0.1], [fotofing@geekprojects.com])
AM_INIT_AUTOMAKE([-Wall -Werror])
AC_CONFIG_SRCDIR([src/ui/main.cpp])
AC_CONFIG_HEADERS([src/include/fotofing/config.h])

# Checks for programs.
AM_PROG_AR
AC_PROG_CXX
AC_PROG_CC
AC_PROG_LIBTOOL
AC_PROG_RANLIB

# Checks for libraries.
AC_CHECK_LIB([beyond], [_ZTS7Surface])
# FIXME: Replace `main' with a function in `-lexiv2':
AC_CHECK_LIB([exiv2], [_ZN5Exiv212ImageFactory4openERKSs])
# FIXME: Replace `main' with a function in `-lm':
AC_CHECK_LIB([m], [pow])
# FIXME: Replace `main' with a function in `-lsqlite3':
AC_CHECK_LIB([sqlite3], [sqlite3_open])

# Checks for header files.
AC_CHECK_HEADERS([limits.h stdint.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_INT64_T
AC_TYPE_SIZE_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK
AC_CHECK_FUNCS([floor gethostname localtime_r memset])

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 src/libfotofing/Makefile
                 src/tests/Makefile
                 src/tests/database/Makefile
                 src/tests/database/schema/Makefile
                 src/ui/Makefile])
AC_OUTPUT
