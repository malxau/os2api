/**
 * @file crt/malloc.c
 *
 * OS/2 based memory allocator wrapper.
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
#include <crtpriv.h>

PVOID hugeAllocator;

BOOL
malloc_init(VOID)
{
    hugeAllocator = HugePoolAllocNewPool();
    if (hugeAllocator == NULL) {
        return FALSE;
    }
    
    return TRUE;
}

VOID
malloc_cleanup(VOID)
{
    if (hugeAllocator != NULL) {
        HugePoolFreePool(hugeAllocator);
    }
}

PVOID
malloc(WORD sizeInBytes)
{
    if (hugeAllocator == NULL) {
        hugeAllocator = HugePoolAllocNewPool();
        if (hugeAllocator == NULL) {
            return NULL;
        }
    }

    return HugePoolAlloc(hugeAllocator, sizeInBytes);
}

VOID
free(PVOID ptr)
{
    HugePoolFree(ptr);
}

// vim:sw=4:ts=4:et:
