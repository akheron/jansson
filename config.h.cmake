/* Reduced down to the defines that are actually used in the code */

/* Define to 1 if you have the <inttypes.h> header file. */
#cmakedefine HAVE_INTTYPES_H 1

/* Define to 1 if you have the <locale.h> header file. */
#cmakedefine HAVE_LOCALE_H 1

/* Define to 1 if you have the 'setlocale' function. */
#cmakedefine HAVE_SETLOCALE 1

/* Define to the type of a signed integer type of width exactly 32 bits if
   such a type exists and the standard includes do not define it. */
#cmakedefine HAVE_INT32_T 1

#ifndef HAVE_INT32_T
#  define int32_t @JSON_INT32@
#endif

#ifndef HAVE_SNPRINTF
#  define snprintf @JSON_SNPRINTF@
#endif
