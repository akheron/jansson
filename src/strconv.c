#include "jansson_private.h"
#include "strbuffer.h"
#include <assert.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

/* need jansson_private_config.h to get the correct snprintf */
#ifdef HAVE_CONFIG_H
#include <jansson_private_config.h>
#endif

/*
  - This code assumes that the decimal separator is exactly one
    character.

  - If setlocale() is called by another thread between the call to
    get_decimal_point() and the call to sprintf() or strtod(), the
    result may be wrong. setlocale() is not thread-safe and should
    not be used this way. Multi-threaded programs should use
    uselocale() instead.
*/
static char get_decimal_point() {
    char buf[3];
    sprintf(buf, "%#.0f", 1.0); // "1." in the current locale
    return buf[1];
}

static void to_locale(strbuffer_t *strbuffer) {
    char point;
    char *pos;

    point = get_decimal_point();
    if (point == '.') {
        /* No conversion needed */
        return;
    }

    pos = strchr(strbuffer->value, '.');
    if (pos)
        *pos = point;
}

int jsonp_strtod(strbuffer_t *strbuffer, double *out) {
    double value;
    char *end;

    to_locale(strbuffer);

    errno = 0;
    value = strtod(strbuffer->value, &end);
    assert(end == strbuffer->value + strbuffer->length);

    if ((value == HUGE_VAL || value == -HUGE_VAL) && errno == ERANGE) {
        /* Overflow */
        return -1;
    }

    *out = value;
    return 0;
}

#if DTOA_ENABLED
/* see dtoa.c */
char *dtoa_r(double dd, int mode, int ndigits, int *decpt, int *sign, char **rve,
             char *buf, size_t blen);

int jsonp_dtostr(char *buffer, size_t size, double value, int precision) {
    /* adapted from `format_float_short()` in
     * https://github.com/python/cpython/blob/2cf18a44303b6d84faa8ecffaecc427b53ae121e/Python/pystrtod.c#L969
     */
    char digits[25];
    char *digits_end;
    int mode = precision == 0 ? 0 : 2;
    int decpt, sign, exp_len, exp = 0, use_exp = 0;
    int digits_len, vdigits_start, vdigits_end;
    char *p;

    if (dtoa_r(value, mode, precision, &decpt, &sign, &digits_end, digits, 25) == NULL) {
        // digits is too short => should not happen
        return -1;
    }

    digits_len = digits_end - digits;
    if (decpt <= -4 || decpt > 16) {
        use_exp = 1;
        exp = decpt - 1;
        decpt = 1;
    }

    vdigits_start = decpt <= 0 ? decpt - 1 : 0;
    vdigits_end = digits_len;
    if (!use_exp) {
        /* decpt + 1 to add ".0" if value is an integer */
        vdigits_end = vdigits_end > decpt ? vdigits_end : decpt + 1;
    } else {
        vdigits_end = vdigits_end > decpt ? vdigits_end : decpt;
    }

    if (
        /* sign, decimal point and trailing 0 byte */
        (size_t)(3 +

                 /* total digit count (including zero padding on both sides) */
                 (vdigits_end - vdigits_start) +

                 /* exponent "e+100", max 3 numerical digits */
                 (use_exp ? 5 : 0)) > size) {
        /* buffer is too short */
        return -1;
    }

    p = buffer;
    if (sign == 1) {
        *p++ = '-';
    }

    /* note that exactly one of the three 'if' conditions is true,
      so we include exactly one decimal point */
    /* Zero padding on left of digit string */
    if (decpt <= 0) {
        memset(p, '0', decpt - vdigits_start);
        p += decpt - vdigits_start;
        *p++ = '.';
        memset(p, '0', 0 - decpt);
        p += 0 - decpt;
    } else {
        memset(p, '0', 0 - vdigits_start);
        p += 0 - vdigits_start;
    }

    /* Digits, with included decimal point */
    if (0 < decpt && decpt <= digits_len) {
        strncpy(p, digits, decpt - 0);
        p += decpt - 0;
        *p++ = '.';
        strncpy(p, digits + decpt, digits_len - decpt);
        p += digits_len - decpt;
    } else {
        strncpy(p, digits, digits_len);
        p += digits_len;
    }

    /* And zeros on the right */
    if (digits_len < decpt) {
        memset(p, '0', decpt - digits_len);
        p += decpt - digits_len;
        *p++ = '.';
        memset(p, '0', vdigits_end - decpt);
        p += vdigits_end - decpt;
    } else {
        memset(p, '0', vdigits_end - digits_len);
        p += vdigits_end - digits_len;
    }

    if (p[-1] == '.')
        p--;

    if (use_exp) {
        *p++ = 'e';
        exp_len = sprintf(p, "%d", exp);
        p += exp_len;
    }
    *p = '\0';

    return (int)(p - buffer);
}
#else /* DTOA_ENABLED == 0 */
static void from_locale(char *buffer) {
    char point;
    char *pos;

    point = get_decimal_point();
    if (point == '.') {
        /* No conversion needed */
        return;
    }

    pos = strchr(buffer, point);
    if (pos)
        *pos = '.';
}

int jsonp_dtostr(char *buffer, size_t size, double value, int precision) {
    int ret;
    char *start, *end;
    size_t length;

    if (precision == 0)
        precision = 17;

    ret = snprintf(buffer, size, "%.*g", precision, value);
    if (ret < 0)
        return -1;

    length = (size_t)ret;
    if (length >= size)
        return -1;

    from_locale(buffer);

    /* Make sure there's a dot or 'e' in the output. Otherwise
       a real is converted to an integer when decoding */
    if (strchr(buffer, '.') == NULL && strchr(buffer, 'e') == NULL) {
        if (length + 3 >= size) {
            /* No space to append ".0" */
            return -1;
        }
        buffer[length] = '.';
        buffer[length + 1] = '0';
        buffer[length + 2] = '\0';
        length += 2;
    }

    /* Remove leading '+' from positive exponent. Also remove leading
       zeros from exponents (added by some printf() implementations) */
    start = strchr(buffer, 'e');
    if (start) {
        start++;
        end = start + 1;

        if (*start == '-')
            start++;

        while (*end == '0')
            end++;

        if (end != start) {
            memmove(start, end, length - (size_t)(end - buffer));
            length -= (size_t)(end - start);
        }
    }

    return (int)length;
}
#endif
