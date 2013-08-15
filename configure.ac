AC_PREREQ([2.60])
AC_INIT([jansson], [2.5-dev], [petri@digip.org])

AM_INIT_AUTOMAKE([1.10 foreign])

AC_CONFIG_SRCDIR([src/value.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_LIBTOOL
AM_CONDITIONAL([GCC], [test x$GCC = xyes])

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([locale.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_INT32_T
AC_TYPE_LONG_LONG_INT

AC_C_INLINE
case $ac_cv_c_inline in
    yes) json_inline=inline;;
    no) json_inline=;;
    *) json_inline=$ac_cv_c_inline;;
esac
AC_SUBST([json_inline])

# Checks for library functions.
AC_CHECK_FUNCS([strtoll localeconv])

case "$ac_cv_type_long_long_int$ac_cv_func_strtoll" in
     yesyes) json_have_long_long=1;;
     *) json_have_long_long=0;;
esac
AC_SUBST([json_have_long_long])

case "$ac_cv_header_locale_h$ac_cv_func_localeconv" in
     yesyes) json_have_localeconv=1;;
     *) json_have_localeconv=0;;
esac
AC_SUBST([json_have_localeconv])

# isinf/isnan
AC_MSG_CHECKING([whether isnan and isinf are defined and/or need -lm])
AC_LANG([C])

# First, test without -lm
isnan_isinf_found=no
AC_LINK_IFELSE(
    [AC_LANG_PROGRAM([#include <math.h>], [isnan(0.1); isinf(0.1);])],
    [AC_MSG_RESULT([defined, -lm not needed]); isnan_isinf_found=yes]
)

if test "x$isnan_isinf_found" = "xno"; then
    # Not found, test with -lm
    libs_before_test=$LIBS
    LIBS="$LIBS -lm"
    AC_LINK_IFELSE(
        [AC_LANG_PROGRAM([#include <math.h>], [isnan(0.1); isinf(0.1);])],
        [AC_MSG_RESULT([defined, -lm needed]); isnan_isinf_found=yes],
        [LIBS=$libs_before_test]
    )
fi

if test "x$isnan_isinf_found" = "xno"; then
    # Still not found
    AC_MSG_RESULT([not defined])
    json_have_isnan_isinf=0
else
    json_have_isnan_isinf=1
fi
AC_SUBST([json_have_isnan_isinf])

AC_CONFIG_FILES([
        jansson.pc
        Makefile
        doc/Makefile
        src/Makefile
        src/jansson_config.h
        test/Makefile
        test/bin/Makefile
        test/suites/Makefile
        test/suites/api/Makefile
])
AC_OUTPUT
