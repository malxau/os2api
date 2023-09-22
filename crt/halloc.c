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
#include <crtpriv.h>

#define HALLOC_DBG 0

typedef struct _HUGE_POOL_HDR {
    WORD AllocatedSegments;
    WORD CurrentSegments;
    PPVOID Segments;
} HUGE_POOL_HDR;

typedef HUGE_POOL_HDR FAR* PHUGE_POOL_HDR;

#define HUGE_POOL_INITIAL_SEGMENT_COUNT (4)
#define HUGE_POOL_SEGMENT_COUNT_INCREMENT (4)
#define HUGE_POOL_SEGMENT_SHIFT (16)
#define HUGE_POOL_SEGMENT_SIZE (1ul << HUGE_POOL_SEGMENT_SHIFT)

PVOID
HugePoolAllocNewPool(VOID)
{
    PHUGE_POOL_HDR poolHdr;
    WORD segmentCount;
    WORD allocSize;
    DWORD segmentSize;
    PVOID initialSmallPool;


    segmentSize = HUGE_POOL_SEGMENT_SIZE;
    segmentCount = HUGE_POOL_INITIAL_SEGMENT_COUNT;
#if HALLOC_DBG
    printf("halloc: requesting %04x segments %04x bytes each\r\n", segmentCount, (WORD)segmentSize);
#endif
    initialSmallPool = SmallPoolAllocNewPool((WORD)segmentSize);
    if (initialSmallPool == NULL) {
        return NULL;
    }

    poolHdr = SmallPoolAlloc(initialSmallPool, sizeof(HUGE_POOL_HDR));
    if (poolHdr == NULL) {
#if HALLOC_DBG
        printf("halloc: failed to allocate huge pool header, requested %i bytes from %i byte segment\r\n",
            sizeof(HUGE_POOL_HDR),
            segmentSize);
#endif

        DebugBreak();
        SmallPoolFreePool(initialSmallPool);
        return NULL;
    }

    allocSize = segmentCount * sizeof(PVOID);
    poolHdr->Segments = SmallPoolAlloc(initialSmallPool, allocSize);
    if (poolHdr->Segments == NULL) {
#if HALLOC_DBG
        printf("halloc: failed to allocate huge pool segment array, requested %i bytes from %i byte segment\r\n",
            allocSize,
            segmentSize);
#endif

        DebugBreak();
        SmallPoolFree(poolHdr);
        SmallPoolFreePool(initialSmallPool);
        return NULL;
    }

    poolHdr->AllocatedSegments = segmentCount;
    poolHdr->CurrentSegments = 1;
    poolHdr->Segments[0] = initialSmallPool;

#if HALLOC_DBG
    printf("halloc: hdr %08lp AllocatedSegments %04x CurrentSegments %04x SegmentArray %08lp\r\n",
           poolHdr, poolHdr->AllocatedSegments, poolHdr->CurrentSegments, poolHdr->Segments);
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
    PVOID hdrSegment;
    PVOID arraySegment;
    PVOID segment;

    poolHdr = (PHUGE_POOL_HDR)pool;

#if HALLOC_DBG
    printf("halloc: hdr %08lp free pool AllocatedSegments %04x CurrentSegments %04x SegmentArray %08lp\r\n",
           poolHdr, poolHdr->AllocatedSegments, poolHdr->CurrentSegments, poolHdr->Segments);
#endif

    arraySegment = SelToFarPtr(FarPtrToSel(poolHdr->Segments));
    hdrSegment = SelToFarPtr(FarPtrToSel(pool));

    //
    //  Free all segments except the one hosting the huge pool header
    //

    for (segmentIndex = poolHdr->CurrentSegments; segmentIndex > 0; segmentIndex--) {
        segment = poolHdr->Segments[segmentIndex - 1];
        if (segment != hdrSegment && segment != arraySegment) {
            SmallPoolFreePool(segment);
        }
    }

    SmallPoolFree(poolHdr->Segments);
    if (arraySegment != hdrSegment) {
        SmallPoolFreePool(arraySegment);
    }

    SmallPoolFree(poolHdr);
    SmallPoolFreePool(hdrSegment);
}

VOID
HugePoolDump(
    PVOID pool
    )
{
    PHUGE_POOL_HDR poolHdr;
    WORD segmentIndex;

    poolHdr = (PHUGE_POOL_HDR)pool;
    printf("Pool %08lp CurrentSegments %04x AllocatedSegments %04x Segments %08lp\r\n",
        poolHdr,
        poolHdr->CurrentSegments,
        poolHdr->AllocatedSegments,
        poolHdr->Segments);

    for (segmentIndex = 0; segmentIndex < poolHdr->CurrentSegments; segmentIndex++) {
        printf("Segment %04x\r\n", segmentIndex);
        SmallPoolDump(poolHdr->Segments[segmentIndex]);
    }
}

PVOID
HugePoolAllocFromExistingSegments(
    PHUGE_POOL_HDR poolHdr,
    WORD sizeInBytes
    )
{
    WORD segmentIndex;
    PVOID userData;
    PVOID smallPool;

    for (segmentIndex = 0; segmentIndex < poolHdr->CurrentSegments; segmentIndex++) {
        smallPool = poolHdr->Segments[segmentIndex];
        if (smallPool != SelToFarPtr(FarPtrToSel(smallPool))) {
#if HALLOC_DBG
            printf("halloc: hdr %08lp searching small pool %08lp index %04x\r\n",
                   poolHdr, smallPool, segmentIndex);
#endif
            DebugBreak();
        }

        userData = SmallPoolAlloc(smallPool, sizeInBytes);
        if (userData != NULL) {
            return userData;
        }
    }

    return NULL;
}

PVOID
HugePoolAlloc(
    PVOID pool,
    WORD sizeInBytes
    )
{
    PHUGE_POOL_HDR poolHdr;
    PVOID newSmallPool;
    PVOID userData;
    DWORD segmentSize;
    WORD newSegmentCount;
    WORD newSegmentArrayBytes;
    WORD loopCount;

    segmentSize = HUGE_POOL_SEGMENT_SIZE;

    poolHdr = (PHUGE_POOL_HDR)pool;

    //
    //  Loop twice looking for an allocation.  The worst case for this
    //  allocator is to add one segment, need to allocate the segment
    //  array from it, then have no contiguous space for the user request.
    //  On a second pass, we're guaranteed to have one completely empty
    //  segment, since the array must be in the first new segment or the
    //  second new segment, but not both.
    //

    for (loopCount = 0; loopCount < 2; loopCount++) {
        userData = HugePoolAllocFromExistingSegments(poolHdr, sizeInBytes);
        if (userData != NULL) {
            return userData;
        }
#if HALLOC_DBG
        printf("halloc: hdr %08lp allocation failure %i bytes requested\r\n",
               poolHdr, sizeInBytes);
#endif

        //
        //  We need to allocate a new segment.  If we don't have space in the
        //  array, attempt to extend the array.  First look for an array
        //  allocation from the existing pool, to maximize the available space
        //  in the new segment.
        //
        if (poolHdr->CurrentSegments == poolHdr->AllocatedSegments) {
            newSegmentCount = poolHdr->AllocatedSegments + HUGE_POOL_SEGMENT_COUNT_INCREMENT;
            newSegmentArrayBytes = newSegmentCount * sizeof(PVOID);

            //
            //  The array needs to fit into a single segment.
            //
            if (newSegmentArrayBytes > segmentSize - 0x80) {
                return NULL;
            }

            if (newSegmentArrayBytes < sizeInBytes) {
                userData = HugePoolAllocFromExistingSegments(poolHdr, newSegmentArrayBytes);
                if (userData != NULL) {
#if HALLOC_DBG
                    printf("halloc: hdr %08lp extended segment array from existing space, AllocatedSegments %04x, SegmentArray %08lp\r\n",
                           poolHdr, newSegmentCount, userData);
#endif
                    memcpy(userData, poolHdr->Segments, poolHdr->CurrentSegments * sizeof(PVOID));
                    SmallPoolFree(poolHdr->Segments);
                    poolHdr->Segments = userData;
                    poolHdr->AllocatedSegments = newSegmentCount;
                }
            }
        }

        //
        //  Allocate a new segment.
        //

        newSmallPool = SmallPoolAllocNewPool((WORD)segmentSize);
        if (newSmallPool == NULL) {
#if HALLOC_DBG
            printf("halloc: hdr %08lp failed to allocate new segment\r\n",
                   poolHdr);
#endif
            return NULL;
        }
#if HALLOC_DBG
        printf("halloc: hdr %08lp allocated new segment %08lp\r\n",
               poolHdr, newSmallPool);
#endif

        //
        //  If the array is large enough, add the new segment.  If not, it
        //  means we couldn't extend the array above, and need to allocate
        //  the array from the new segment.
        //

        if (poolHdr->AllocatedSegments > poolHdr->CurrentSegments) {
            poolHdr->Segments[poolHdr->CurrentSegments] = newSmallPool;
            poolHdr->CurrentSegments++;

            //
            //  If the user request doesn't fit in a single segment, the
            //  request cannot be supported by this allocator.
            //

            userData = SmallPoolAlloc(newSmallPool, sizeInBytes);
            if (userData == NULL) {
#if HALLOC_DBG
                printf("halloc: hdr %08lp allocation for %i bytes doesn't fit in one segment\r\n",
                       poolHdr, sizeInBytes);
#endif
                DebugBreak();
            }
#if HALLOC_DBG
            printf("halloc: hdr %08lp returning user data %08lp after extension\r\n",
                   poolHdr, userData);
#endif
            return userData;
        } else {
            newSegmentCount = poolHdr->AllocatedSegments + HUGE_POOL_SEGMENT_COUNT_INCREMENT;
            newSegmentArrayBytes = newSegmentCount * sizeof(PVOID);
            userData = SmallPoolAlloc(newSmallPool, newSegmentArrayBytes);

            //
            //  If this fails, we can't fit the segment array in an empty
            //  segment, so the huge allocator has reached its limit.
            //
            if (userData == NULL) {
#if HALLOC_DBG
                printf("halloc: hdr %08lp cannot extend extended segment array, AllocatedSegments %04x\r\n",
                       poolHdr, poolHdr->AllocatedSegments);
#endif
                SmallPoolFreePool(newSmallPool);
                return NULL;
            }
#if HALLOC_DBG
            printf("halloc: hdr %08lp extended segment array from new space, AllocatedSegments %04x, loopCount %i, SegmentArray %08lp\r\n",
                   poolHdr, poolHdr->AllocatedSegments, loopCount, userData);
#endif

            memcpy(userData, poolHdr->Segments, poolHdr->CurrentSegments * sizeof(PVOID));
            SmallPoolFree(poolHdr->Segments);
            poolHdr->Segments = userData;
            poolHdr->AllocatedSegments = newSegmentCount;

            poolHdr->Segments[poolHdr->CurrentSegments] = newSmallPool;
            poolHdr->CurrentSegments++;

            //
            //  Loop again to retry the user's request.  If we've already
            //  looped twice, the user request didn't fit in a segment.
            //
            userData = NULL;
            if (loopCount == 1) {
#if HALLOC_DBG
                printf("halloc: hdr %08lp allocation for %i bytes doesn't fit in one segment\r\n",
                       poolHdr, sizeInBytes);
#endif
                DebugBreak();
            }
        }
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
