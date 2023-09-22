/**
 * @file crt/printf.c
 *
 * OS/2 printf implementation.
 *
 * Copyright (c) 2023 Malcolm J. Smith
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

#include <os2.h>
#include <os2crt.h>
#include <stdarg.h>

#include "printf.inc"

#define PRINTF_SIZEONLY 1
#include "printf.inc"

int
sprintf_s(
    PSZ szDest,
    WORD len,
    PSZ szFmt,
    ...
    )
{
    va_list marker;
    int out_len;

    va_start( marker, szFmt );
    out_len = vsprintf(szDest, len, szFmt, marker);
    va_end( marker );
    return out_len;
}

int
sprintf(
    PSZ szDest,
    PSZ szFmt,
    ...
    )
{
    va_list marker;
    int out_len;

    va_start( marker, szFmt );
    out_len = vsprintf(szDest, (WORD)-1, szFmt, marker);
    va_end( marker );
    return out_len;
}

int
sprintf_sz(
    PSZ szFmt,
    ...
    )
{
    va_list marker;
    int out_len;

    va_start( marker, szFmt );
    out_len = vsprintf_sz(szFmt, marker);
    va_end( marker );
    return out_len;
}

int
printf(
    PSZ szFmt,
    ...
    )
{
    va_list marker;
    CHAR buf[200];
    int out_len;

    va_start( marker, szFmt );
    out_len = vsprintf(buf, sizeof(buf), szFmt, marker);
    va_end( marker );

    if (out_len != -1 && out_len < sizeof(buf)) {
        WORD written;
        if (DosWrite(1, buf, out_len, &written) != NO_ERROR) {
            out_len = -1;
        }
    }
    return out_len;
}


// vim:sw=4:ts=4:et:
