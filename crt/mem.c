/**
 * @file crt/mem.c
 *
 * OS/2 based memory manipulation routines.
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

PVOID
memcpy(PVOID dest, PVOID src, WORD len)
{
    WORD i;
    PUCHAR char_src = (PUCHAR)src;
    PUCHAR char_dest = (PUCHAR)dest;
    for (i = 0; i < len; i++) {
        char_dest[i] = char_src[i];
    }
    return dest;
}

PVOID
memmove(PVOID dest, PVOID src, WORD len)
{
    WORD i;
    PUCHAR char_src = (PUCHAR)src;
    PUCHAR char_dest = (PUCHAR)dest;
    if (char_dest > char_src) {
        if (len == 0) {
            return dest;
        }
        for (i = len - 1; ; i--) {
            char_dest[i] = char_src[i];
            if (i==0) break;
        }
    } else {
        for (i = 0; i < len; i++) {
            char_dest[i] = char_src[i];
        }
    }
    return dest;
}

PVOID
memset(PVOID dest, CHAR c, WORD len)
{
    WORD i;
    WORD fill;
    WORD chunks = len / sizeof(fill);
    PCHAR char_dest = (PCHAR)dest;
    PWORD word_dest = (PWORD)dest;

    //
    //  Note we go from the back to the front.  This is to 
    //  prevent newer compilers from noticing what we're doing
    //  and trying to invoke the built-in memset instead of us.
    //

    fill = (c<<8) + c;

    for (i = len; i > chunks * sizeof(fill); i--) {
        char_dest[i - 1] = c;
    }

    for (i = chunks; i > 0; i--) {
        word_dest[i - 1] = fill;
    }

    return dest;
}

SHORT
memcmp(PVOID buf1, PVOID buf2, WORD len)
{
    WORD i = 0;
    PUCHAR char_buf1 = (PUCHAR)buf1;
    PUCHAR char_buf2 = (PUCHAR)buf2;
    for (i = 0; i < len; i++) {
        if (char_buf1[i] < char_buf2[i]) {
            return -1;
        } else if (char_buf1[i] > char_buf2[i]) {
            return 1;
        }
    }
    return 0;
}

// vim:sw=4:ts=4:et:
