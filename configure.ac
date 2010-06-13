AC_PREREQ([2.60])
AC_INIT([jansson], [1.3], [petri@digip.org])

AM_INIT_AUTOMAKE([1.10 foreign])

AC_CONFIG_SRCDIR([src/value.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AC_PROG_LIBTOOL

# Checks for libraries.

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_INT32_T

AC_C_INLINE
case $ac_cv_c_inline in
    yes) json_inline=inline;;
    no) json_inline=;;
    *) json_inline=$ac_cv_c_inline;;
esac
AC_SUBST([json_inline])

# Checks for library functions.

AC_CONFIG_FILES([
        jansson.pc
        Makefile
        doc/Makefile
        src/Makefile
        src/jansson.h
        test/Makefile
        test/bin/Makefile
        test/suites/Makefile
        test/suites/api/Makefile
])
AC_OUTPUT
