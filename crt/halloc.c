/**
 * @file crt/halloc.c
 *
 * OS/2 huge memory allocator.  This is a multi-segment allocator that
 * dynamically allocated segments to satisfy memory allocations from an
 * arbitrary segment.
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

#define HALLOC_DBG 0

typedef struct _HUGE_POOL_HDR {
    WORD MaxSegments;
    WORD CurrentSegments;
    PVOID Segments[1];
} HUGE_POOL_HDR;

typedef HUGE_POOL_HDR FAR* PHUGE_POOL_HDR;

#define HUGE_POOL_SEGMENT_SHIFT (16)
#define HUGE_POOL_SEGMENT_SIZE (1ul << HUGE_POOL_SEGMENT_SHIFT)

PVOID
HugePoolAllocNewPool(
    DWORD requestedBytes
    )
{
    PHUGE_POOL_HDR poolHdr;
    WORD segmentCount;
    WORD allocSize;
    DWORD segmentSize;
    DWORD totalBytes;
    PVOID initialSmallPool;


    segmentSize = HUGE_POOL_SEGMENT_SIZE;
    totalBytes = requestedBytes + segmentSize - 1;
    segmentCount = (WORD)(totalBytes >> HUGE_POOL_SEGMENT_SHIFT);
#if HALLOC_DBG
    printf("halloc: requesting %08lx bytes %04x segments %04x bytes each\r\n", totalBytes, segmentCount, (WORD)segmentSize);
#endif
    initialSmallPool = SmallPoolAllocNewPool((WORD)segmentSize);
    if (initialSmallPool == NULL) {
        return NULL;
    }

    allocSize = sizeof(HUGE_POOL_HDR) + segmentCount * sizeof(PVOID);
    poolHdr = SmallPoolAlloc(initialSmallPool, allocSize);
    if (poolHdr == NULL) {
        printf("halloc: failed to allocate huge pool header, requested %i bytes from %i byte segment\r\n",
            allocSize,
            segmentSize);

        DebugBreak();
        SmallPoolFreePool(initialSmallPool);
        return NULL;
    }

    poolHdr->MaxSegments = segmentCount;
    poolHdr->CurrentSegments = 1;
    poolHdr->Segments[0] = initialSmallPool;

#if HALLOC_DBG
    printf("halloc: hdr %08p MaxSegments %04x CurrentSegments %04x\r\n", poolHdr, poolHdr->MaxSegments, poolHdr->CurrentSegments);
#endif

    return poolHdr;
}

VOID
HugePoolFreePool(
    PVOID pool
    )
{
    WORD segmentIndex;
    PHUGE_POOL_HDR poolHdr;
    PVOID finalSegment;

    poolHdr = (PHUGE_POOL_HDR)pool;

    finalSegment = SelToFarPtr(FarPtrToSel(pool));

    //
    //  Free all segments except the one hosting the huge pool header
    //

    for (segmentIndex = poolHdr->CurrentSegments; segmentIndex > 0; segmentIndex--) {
        if (poolHdr->Segments[segmentIndex - 1] != finalSegment) {
            SmallPoolFreePool(poolHdr->Segments[segmentIndex - 1]);
        }
    }

    SmallPoolFree(poolHdr);
    SmallPoolFreePool(finalSegment);
}

VOID
HugePoolDump(
    PVOID pool
    )
{
    PHUGE_POOL_HDR poolHdr;
    WORD segmentIndex;

    poolHdr = (PHUGE_POOL_HDR)pool;
    printf("Pool %08p CurrentSegments %04x MaxSegments %04x\r\n",
        poolHdr,
        poolHdr->CurrentSegments,
        poolHdr->MaxSegments);

    for (segmentIndex = 0; segmentIndex < poolHdr->CurrentSegments; segmentIndex++) {
        printf("Segment %04x\r\n", segmentIndex);
        SmallPoolDump(poolHdr->Segments[segmentIndex]);
    }
}

PVOID
HugePoolAlloc(
    PVOID pool,
    WORD sizeInBytes
    )
{
    WORD segmentIndex;
    PHUGE_POOL_HDR poolHdr;
    PVOID userData;
    DWORD segmentSize;

    segmentSize = HUGE_POOL_SEGMENT_SIZE;

    poolHdr = (PHUGE_POOL_HDR)pool;
    for (segmentIndex = 0; segmentIndex < poolHdr->CurrentSegments; segmentIndex++) {
        userData = SmallPoolAlloc(poolHdr->Segments[segmentIndex], sizeInBytes);
        if (userData != NULL) {
            return userData;
        }
    }

    if (segmentIndex == poolHdr->MaxSegments) {
        return NULL;
    }

    poolHdr->Segments[segmentIndex] = SmallPoolAllocNewPool((WORD)segmentSize);
    if (poolHdr->Segments[segmentIndex] == NULL) {
        return NULL;
    }

    poolHdr->CurrentSegments++;
    userData = SmallPoolAlloc(poolHdr->Segments[segmentIndex], sizeInBytes);
    if (userData == NULL) {
        DebugBreak();
    }
    
    return userData;
}

VOID
HugePoolFree(
    PVOID ptr
    )
{
    SmallPoolFree(ptr);
}


// vim:sw=4:ts=4:et:
