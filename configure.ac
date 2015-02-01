#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([Fotofing], [0.0.1], [fotofing@geekprojects.com])
AM_INIT_AUTOMAKE([-Wall -Werror])

AC_CONFIG_SRCDIR([src/ui/main.cpp])
AC_CONFIG_HEADERS([src/include/config.h])

PKG_PROG_PKG_CONFIG(0.16)

AC_LANG([C++])

# Checks for programs.
AM_PROG_AR
AC_PROG_CC
AC_PROG_CXX
AC_PROG_LIBTOOL
AC_PROG_RANLIB

# Checks for libraries.
AC_CHECK_LIB([m], [pow])
AC_CHECK_LIB([sqlite3], [sqlite3_open])
AC_CHECK_LIB([geek-gfx], _ZN4Geek3Gfx7Surface8loadJPEGESs)
AC_CHECK_LIB([xml2], [xmlReadFile])

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

######################
# Optional Libraries #
######################

# Check for gtkmm
AC_MSG_CHECKING([for gtkmm 3.0])
have_gtkmm=no
PKG_CHECK_EXISTS(
    gtkmm-3.0,
    have_gtkmm=yes
)
AC_MSG_RESULT([$have_gtkmm])

AM_CONDITIONAL(BUILD_GTK_UI, test "x$have_gtkmm" = xyes)

if test "x$have_gtkmm" = "xno"; then
    AC_MSG_WARN(*** GTK UI will not be built (gtkmm not found) ***)
fi

# OpenCV
have_opencv=no
AC_CHECK_LIB(
    [opencv_objdetect],
    cvHaarDetectObjects,
    AC_CHECK_HEADERS(
        [opencv/cv.h],
        have_opencv=yes,
        AC_MSG_WARN(*** Features tagger will not be built (OpenCV headers not found) ***)
    ),
    AC_MSG_WARN(*** Features tagger will not be built (OpenCV library not found) ***)
)

AM_CONDITIONAL(BUILD_TAGGER_FEATURES, test "x$have_taggers" = xyes)

# Check for exiv2
have_exiv2=no
AC_CHECK_LIB(
    [exiv2],
    [_ZN5Exiv212ImageFactory4openERKSs],
    AC_CHECK_HEADERS(
        [exiv2/exiv2.hpp],
        have_exiv2=yes,
        AC_MSG_WARN(*** Exif tagger will not be built (Exiv2 headers not found) ***)
    ),
    AC_MSG_WARN(*** Exif tagger will not be built (Exiv2 library not found) ***)
)

AM_CONDITIONAL(BUILD_TAGGER_EXIF, test "x$have_exiv2" = xyes)

# Check for Atlas
have_atlas=no
AC_CHECK_LIB(
    [atlas],
    ATL_buildinfo,
    AC_CHECK_HEADERS(
        [atlas/clapack.h],
        have_atlas=yes,
        AC_MSG_WARN(*** Refocus operation will not be built (Atlas headers not found) ***)
    ),
    AC_MSG_WARN(*** Refocus operation will not be built (Atlas library not found) ***)
)

AM_CONDITIONAL(BUILD_REFOCUS, test "x$have_atlas" = xyes)



AC_CONFIG_FILES([Makefile
                 src/Makefile
                 src/libfotofing/Makefile
                 src/libfotofing-workflow/Makefile
                 src/cli/Makefile
                 src/tests/Makefile
                 src/tests/database/Makefile
                 src/tests/database/schema/Makefile
                 src/tests/taggers/Makefile
                 src/tests/workflow/Makefile
                 src/ui/Makefile
                 src/ui/widgets/Makefile
                 src/ui/dialogs/Makefile
                 src/ui/library/Makefile
                 src/ui/edit/Makefile
                 src/plugins/Makefile
                 src/plugins/taggers/Makefile
                 src/plugins/taggers/histogram/Makefile
                 src/plugins/taggers/exif/Makefile
                 src/plugins/taggers/features/Makefile
                 src/plugins/operations/Makefile
                 src/plugins/operations/refocus/Makefile
                 src/plugins/operations/colourify/Makefile])
AC_OUTPUT

AC_MSG_RESULT([
Building Fotofing with prefix=$prefix, datarootdir=$datarootdir

UI:
  GTK UI:   $have_gtkmm

Tagger Plugins:
  Exif:     $have_exiv2
  Features: $have_opencv

Operation Plugins:
  Refocus:  $have_atlas
])

