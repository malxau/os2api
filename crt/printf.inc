/**
 * @file crt/printf.inc
 *
 * OS/2 printf implementation.  Most logic is implemented here so it can be
 * instantiated twice, once to generate code to determine buffer sizes, and
 * once to perform the operation.
 *
 * Copyright (c) 2017-2023 Malcolm J. Smith
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifdef PRINTF_FN
#undef PRINTF_FN
#undef PRINTF_DESTLENGTH
#undef PRINTF_PUSHCHAR
#endif

#ifdef PRINTF_SIZEONLY

#define PRINTF_FN vsprintf_sz

#define PRINTF_DESTLENGTH() (1)
#define PRINTF_PUSHCHAR(x)  dest_offset++,x;

#else // PRINTF_SIZEONLY

#define PRINTF_FN vsprintf

#define PRINTF_DESTLENGTH()  (dest_offset < len - 1)
#define PRINTF_PUSHCHAR(x)   szDest[dest_offset++] = x;

#endif // PRINTF_SIZEONLY

int
PRINTF_FN(
#ifndef PRINTF_SIZEONLY
        PSZ szDest,
        WORD len,
#endif
        PSZ szFmt,
        va_list marker)
{
    WORD dest_offset = 0;
    WORD src_offset = 0;
    WORD i;

    BOOL leadingzero;
    BOOL leftalign;
    BOOL short_prefix;
    BOOL long_prefix;
    BOOL truncated_due_to_space;
    WORD element_len;

    truncated_due_to_space = FALSE;

    while (szFmt[src_offset] != '\0') {
        if (szFmt[src_offset] == '%') {
            src_offset++;
            leadingzero = FALSE;
            leftalign = FALSE;
            long_prefix = FALSE;
            short_prefix = FALSE;
            element_len = 0;

            if (szFmt[src_offset] == '-') {
                leftalign = TRUE;
                src_offset++;
            }
            if (szFmt[src_offset] == '0') {
                leadingzero = TRUE;
                src_offset++;
            }
            while (szFmt[src_offset] >= '0' && szFmt[src_offset] <= '9') {
                element_len = element_len * 10 + szFmt[src_offset] - '0';
                src_offset++;
            }
            if (szFmt[src_offset] == 'h') {
                short_prefix = TRUE;
                src_offset++;
            } else if (szFmt[src_offset] == 'l') {
                long_prefix = TRUE;
                src_offset++;
            }
            
            if (szFmt[src_offset] == 'p') {
                long_prefix = TRUE;
            }

            if (element_len == 0) {
                element_len = (WORD) -1;
            }

            switch(szFmt[src_offset]) {
                case '%':
                    if (PRINTF_DESTLENGTH()) {
                        PRINTF_PUSHCHAR('%');
                    } else {
                        truncated_due_to_space = TRUE;
                    }
                    break;

                case 'c':
                    {
                        //
                        //  The compiler always upconverts chars to ints when
                        //  creating variable arguments.  We have to mirror
                        //  that semantic here.  MSVC gets this "right" by
                        //  allowing these to be symmetrical and upconverting
                        //  both, but gcc gets it "wrong" and explodes by
                        //  upconverting one and not the other (then printing
                        //  a warning blaming this code.)
                        //

                        i = (UCHAR)va_arg(marker, int);
                        if (PRINTF_DESTLENGTH()) {
                            PRINTF_PUSHCHAR((UCHAR)i);
                        } else {
                            truncated_due_to_space = TRUE;
                        }
                    }
                    break;
                case 's':
                    {
                        PSZ str = va_arg(marker, PSZ);

                        if (str == NULL) {
                            str = "(null)";
                        }

                        if (element_len != (WORD)-1 && !leftalign) {
                            WORD str_len = 0;
                            while (str[str_len] != '\0') str_len++;

                            while (element_len > str_len) {
                                if (PRINTF_DESTLENGTH()) {
                                    PRINTF_PUSHCHAR(' ');
                                } else {
                                    truncated_due_to_space = TRUE;
                                }
                                element_len--;
                            }
                        }
                        while (*str != '\0' && element_len) {
                            if (PRINTF_DESTLENGTH()) {
                                PRINTF_PUSHCHAR(*str);
                            } else {
                                truncated_due_to_space = TRUE;
                            }
                            str++;
                            element_len--;
                        }

                        if (leftalign) {
                            while (element_len--) {
                                if (PRINTF_DESTLENGTH()) {
                                    PRINTF_PUSHCHAR(' ');
                                } else {
                                    truncated_due_to_space = TRUE;
                                }
                            }
                        }
                    }
                    break;
                case 'u':
                case 'd':
                case 'i':
                case 'x':
                case 'p':
                    if (!long_prefix) {
                        WORD num, tempnum;
                        WORD divisor, digits;
                        WORD radix = 10;

                        //
                        //  If we're %i we're base 10, if we're %x we're
                        //  base 16
                        //

                        if (szFmt[src_offset] == 'x' || szFmt[src_offset] == 'p') {
                            radix = 16;
                        }

                        num = va_arg(marker, int);

                        //
                        //  Count the number of digits we have in the user's
                        //  input.  Stop if we hit the format specifier.
                        //  Code below will preserve low order values.
                        //

                        divisor = 1;
                        digits = 1;
                        tempnum = num;
                        while (tempnum > radix - 1 && digits < element_len) {
                            divisor *= radix;
                            tempnum = num / divisor;
                            digits++;
                        }

                        //
                        //  If the field specifier is larger, pad it with
                        //  either a zero or space depending on the format
                        //  If the field specifier is too small, output more
                        //  characters than the field specifier specifies.
                        //

                        if (element_len != (WORD)-1 && digits < element_len) {
                            tempnum = element_len - digits;
                            while (tempnum > 0) {
                                if (!PRINTF_DESTLENGTH()) {
                                    truncated_due_to_space = TRUE;
                                    break;
                                }
                                if (leadingzero) {
                                    PRINTF_PUSHCHAR('0');
                                } else {
                                    PRINTF_PUSHCHAR(' ');
                                }
                                tempnum--;
                            }
                        }

                        do {
                            tempnum = (num / divisor) % radix;

                            if (!PRINTF_DESTLENGTH()) {
                                truncated_due_to_space = TRUE;
                                break;
                            }

                            if (tempnum > 9) {
                                PRINTF_PUSHCHAR((UCHAR)(tempnum + 'a' - 10));
                            } else {
                                PRINTF_PUSHCHAR((UCHAR)(tempnum + '0'));
                            }

                            divisor /= radix;
                            digits--;

                        } while(digits > 0);
                    } else {
                        DWORD num, tempnum;
                        WORD digits;
                        WORD digitcount;
                        WORD radix = 10;

                        //
                        //  If we're %i we're base 10, if we're %x we're
                        //  base 16
                        //

                        if (szFmt[src_offset] == 'x' || szFmt[src_offset] == 'p') {
                            radix = 16;
                        }

                        num = va_arg(marker, DWORD);

                        //
                        //  Count the number of digits we have in the user's
                        //  input.  Stop if we hit the format specifier.
                        //  Code below will preserve low order values.
                        //

                        digits = 1;
                        tempnum = num;
                        while (tempnum > radix - 1) {
                            tempnum = tempnum / radix;
                            digits++;
                        }

                        //
                        //  If the field specifier is larger, pad it with
                        //  either a zero or space depending on the format
                        //  If the field specifier is too small, output more
                        //  characters than the field specifier specifies.
                        //

                        if (element_len != (WORD)-1 && digits < element_len) {
                            tempnum = element_len - digits;
                            while (tempnum > 0) {
                                if (!PRINTF_DESTLENGTH()) {
                                    truncated_due_to_space = TRUE;
                                    break;
                                }
                                if (leadingzero) {
                                    PRINTF_PUSHCHAR('0');
                                } else {
                                    PRINTF_PUSHCHAR(' ');
                                }
                                tempnum--;
                            }
                        }

                        do {
                            tempnum = num;
                            for (digitcount = 1; digitcount < digits; digitcount++) {
                                tempnum = tempnum / radix;
                            }
                            tempnum = tempnum % radix;

                            if (!PRINTF_DESTLENGTH()) {
                                truncated_due_to_space = TRUE;
                                break;
                            }

                            if (tempnum > 9) {
                                PRINTF_PUSHCHAR((UCHAR)(tempnum + 'a' - 10));
                            } else {
                                PRINTF_PUSHCHAR((UCHAR)(tempnum + '0'));
                            }

                            digits--;
                        } while (digits > 0);
                    }
                    break;
                default:
                    {
                        PSZ szErr = "FMTERR";
                        i = 0;
                        while (szErr[i] != '\0') {
                            if (PRINTF_DESTLENGTH()) {
                                PRINTF_PUSHCHAR(szErr[i++]);
                            } else {
                                truncated_due_to_space = TRUE;
                            }
                        }
                    }
                    break;
            }

            src_offset++;

        } else {
            if (PRINTF_DESTLENGTH()) {
                PRINTF_PUSHCHAR(szFmt[src_offset++]);
            } else {
                truncated_due_to_space = TRUE;
            }
        }

        if (truncated_due_to_space) {
            break;
        }
    }

#ifndef PRINTF_SIZEONLY
    if (dest_offset >= len || szFmt[src_offset] != '\0') {
        szDest[0] = '\0';
        return -1;
    }
#endif
    PRINTF_PUSHCHAR('\0');

#ifndef PRINTF_SIZEONLY
    dest_offset--;
#endif

    return dest_offset;
}

// vim:sw=4:ts=4:et:
