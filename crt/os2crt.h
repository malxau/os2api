/**
 * @file crt/os2crt.h
 *
 * Declarations for a mini OS/2 C runtime library.
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

VOID APIENTRY
DebugBreak();

#define SelToFarPtr(selector) (PVOID)((DWORD)(selector) << 16)
#define FarPtrToSel(ptr) (SEL)((DWORD)(ptr) >> 16)

PVOID
malloc(
    WORD sizeInBytes
    );

VOID
free(
    PVOID ptr
    );

int CDECL
sprintf_s(
    PSZ szDest,
    WORD len,
    PSZ szFmt,
    ...
    );

int CDECL
sprintf(
    PSZ szDest,
    PSZ szFmt,
    ...
    );

int CDECL
sprintf_sz(
    PSZ szFmt,
    ...
    );

int CDECL
printf(
    PSZ szFmt,
    ...
    );

PVOID
memcpy(PVOID dest, PVOID src, WORD len);

PVOID
memmove(PVOID dest, PVOID src, WORD len);

PVOID
memset(PVOID dest, CHAR c, WORD len);

SHORT
memcmp(PVOID buf1, PVOID buf2, WORD len);

SHORT
atoi (PSZ str);

PSZ
strcat_s(PSZ dest, WORD len, const PSZ src);

PSZ
strncat(PSZ dest, const PSZ src, WORD len);

PSZ
strchr(PSZ str, UCHAR ch);

PSZ
strrchr(PSZ str, UCHAR ch);

WORD
strlen(PSZ str);

PSZ
strstr(const PSZ str, PSZ search);

UCHAR
toupper(UCHAR c);

UCHAR
tolower(UCHAR c);

PSZ
strupr(PSZ str);

PSZ
strlwr(PSZ str);

SHORT
strncmp(PSZ str1, PSZ str2, WORD count);

SHORT
strcmp(PSZ str1, PSZ str2);

SHORT
strnicmp(PSZ str1, PSZ str2, WORD count);

SHORT
stricmp(PSZ str1, PSZ str2);

PSZ
strtok_s(PSZ str, PSZ match, PSZ * context);

PSZ
strtok(PSZ str, PSZ match);

SHORT
strspn(PSZ str, PSZ chars);

SHORT
strcspn(PSZ str, PSZ match);

// vim:sw=4:ts=4:et:
