/**
 * @file crt/string.c
 *
 * OS/2 based string manipulation routines.
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


SHORT 
atoi (PSZ str)
{
    SHORT ret = 0;
    while (*str >= '0' && *str <= '9') {
        ret *= 10;
        ret += *str - '0';
        str++;
    }
    return ret;
}

PSZ 
strcat_s(PSZ dest, WORD len, const PSZ src)
{
    WORD i,j;
    for (i = 0; dest[i] != '\0' && i < len; i++);
    for (j = 0; src[j] != '\0' && i < len - 1; ) {
        dest[i++] = src[j++];
    }
    dest[i++] = '\0';
    return dest;
}

PSZ 
strncat(PSZ dest, const PSZ src, WORD len)
{
    WORD i,j;
    for (i = 0; dest[i] != '\0'; i++);
    for (j = 0; src[j] != '\0' && j < len; ) {
        dest[i++] = src[j++];
    }
    dest[i++] = '\0';
    return dest;
}

PSZ 
strchr(PSZ str, UCHAR ch)
{
    const UCHAR FAR * ptr = str;
    while (*ptr != '\0' && *ptr != ch) ptr++;
    if (*ptr == ch) return (PSZ)ptr;
    return NULL;
}

PSZ 
strrchr(PSZ str, UCHAR ch)
{
    const UCHAR * ptr = str;
    while (*ptr != '\0') ptr++;
    while (*ptr != ch && ptr > str) ptr--;
    if (*ptr == ch) return (PSZ)ptr;
    return NULL;
}

WORD 
strlen(PSZ str)
{
    WORD i = 0;
    while (str[i] != '\0') {
        i++;
    }
    return i;
}

PSZ 
strstr(const PSZ str, PSZ search)
{
    PSZ ptr = str;
    int i;
    while (*ptr != '\0') {
        for (i=0;ptr[i]==search[i]&&search[i]!='\0'&&ptr[i]!='\0';i++);
        if (search[i]=='\0') return (PSZ)ptr;
        ptr++;
    }
    return NULL;
}

UCHAR 
toupper(UCHAR c)
{
    if (c >= 'a' && c <= 'z') {
        return c - 'a' + 'A';
    }
    return c;
}

UCHAR 
tolower(UCHAR c)
{
    if (c >= 'A' && c <= 'Z') {
        return c - 'A' + 'a';
    }
    return c;
}

PSZ 
strupr(PSZ str)
{
    PSZ ptr = str;
    while (*ptr != '\0') {
        *ptr = (UCHAR)toupper(*ptr);
        ptr++;
    }
    return str;
}

PSZ 
strlwr(PSZ str)
{
    PSZ ptr = str;
    while (*ptr != '\0') {
        *ptr = (UCHAR)tolower(*ptr);
        ptr++;
    }
    return str;
}

SHORT 
strncmp(PSZ str1, PSZ str2, WORD count)
{
    PSZ ptr1 = str1;
    PSZ ptr2 = str2;
    WORD remaining = count;

    while(remaining > 0) {
        if (*ptr1 < *ptr2) {
            return -1;
        } else if (*ptr1 > *ptr2) {
            return 1;
        } else if (*ptr1 == '\0') {
            return 0;
        }

        ptr1++;
        ptr2++;
        remaining--;
    }
    return 0;
}

SHORT 
strcmp(PSZ str1, PSZ str2)
{
    return strncmp(str1, str2, (WORD)-1);
}

SHORT 
strnicmp(PSZ str1, PSZ str2, WORD count)
{
    PSZ ptr1 = str1;
    PSZ ptr2 = str2;
    WORD remaining = count;

    while(remaining > 0) {
        if (toupper(*ptr1) < toupper(*ptr2)) {
            return -1;
        } else if (toupper(*ptr1) > toupper(*ptr2)) {
            return 1;
        } else if (*ptr1 == '\0') {
            return 0;
        }
        ptr1++;
        ptr2++;
        remaining--;
    }
    return 0;
}

SHORT 
stricmp(PSZ str1, PSZ str2)
{
    return strnicmp(str1, str2, (WORD)-1);
}

PSZ 
strtok_s(PSZ str, PSZ match, PSZ * context)
{
    PSZ next;
    if (str != NULL) {
        *context = str;
    }

    next = *context;
    if (next == NULL) {
        return NULL;
    }

    while (*next != match[0] && *next != '\0') next++;

    if (*next == match[0]) {
        PSZ ret = *context;
        *next = '\0';
        *context = ++next;
        return ret;
    } else {
        PSZ ret = *context;
        *context = NULL;
        return ret;
    }
}

PSZ strtok_ctx;

PSZ 
strtok(PSZ str, PSZ match)
{
    return strtok_s(str, match, &strtok_ctx);
}

SHORT 
strspn(PSZ str, PSZ chars)
{
    WORD len = 0;
    WORD i;

    for (len = 0; str[len] != '\0'; len++) {
        for (i = 0; chars[i] != '\0'; i++) {
            if (str[len] == chars[i]) {
                break;
            }
        }
        if (chars[i] == '\0') {
            return len;
        }
    }

    return len;
}

SHORT
strcspn(PSZ str, PSZ match)
{
    WORD len = 0;
    WORD i;

    for (len = 0; str[len] != '\0'; len++) {
        for (i = 0; match[i] != '\0'; i++) {
            if (str[len] == match[i]) {
                return len;
            }
        }
    }

    return len;
}

// vim:sw=4:ts=4:et:
