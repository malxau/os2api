/**
 * @file crt/salloc.c
 *
 * OS/2 based small memory allocator.  This allocates memory within a single
 * 64Kb segment and is currently quite lame.
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

#define SALLOC_DBG 0

typedef struct _SMALL_POOL_HDR {
    WORD TotalBytes;
    WORD LargestFreeBlock;
    WORD FirstFreeHeader;
    WORD Reserved;
} SMALL_POOL_HDR;

typedef SMALL_POOL_HDR FAR* PSMALL_POOL_HDR;

typedef struct _ALLOC_HDR {
    WORD OffsetToPrev;
    WORD OffsetToNext;
} ALLOC_HDR;

typedef ALLOC_HDR FAR* PALLOC_HDR;

#define ALLOC_HDR_FLAG_ALLOCATED 1
#define ALLOC_HDR_OFFSET_MASK    0xfffc

#define AllocHdrIsAllocated(Hdr) (Hdr->OffsetToPrev & ALLOC_HDR_FLAG_ALLOCATED)
#define AllocHdrMarkAllocated(Hdr) Hdr->OffsetToPrev = Hdr->OffsetToPrev | ALLOC_HDR_FLAG_ALLOCATED
#define AllocHdrMarkFree(Hdr) Hdr->OffsetToPrev = Hdr->OffsetToPrev & ~(ALLOC_HDR_FLAG_ALLOCATED)
#define AllocHdrOffsetToPrev(Hdr) (Hdr->OffsetToPrev & ALLOC_HDR_OFFSET_MASK)
#define AllocHdrOffsetToNext(Hdr) (Hdr->OffsetToNext & ALLOC_HDR_OFFSET_MASK)
#define AllocHdrNextHdr(Hdr) (PALLOC_HDR)((PUCHAR)Hdr + AllocHdrOffsetToNext(Hdr))
#define AllocHdrPrevHdr(Hdr) (PALLOC_HDR)((PUCHAR)Hdr - AllocHdrOffsetToPrev(Hdr))
#define AllocHdrUsableBytes(Hdr) (Hdr->OffsetToNext - sizeof(ALLOC_HDR))
#define AllocHdrUserDataFromHdr(Hdr) ((PUCHAR)Hdr + sizeof(ALLOC_HDR))
#define AllocHdrHdrFromUserData(UserData) (PALLOC_HDR)((PUCHAR)UserData - sizeof(ALLOC_HDR))
#define AllocHdrOffsetFromStart(PoolHdr, Hdr) (WORD)((PUCHAR)Hdr - (PUCHAR)poolHdr)
#define AllocHdrFromOffset(PoolHdr, Offset) (PALLOC_HDR)((PUCHAR)PoolHdr + Offset)
#define AllocHdrBeyondPool(PoolHdr, Hdr) (AllocHdrOffsetFromStart(PoolHdr, Hdr) - sizeof(SMALL_POOL_HDR) >= poolHdr->TotalBytes)


PVOID
SmallPoolAllocNewPool(
    WORD requestedBytes
    )
{
    WORD sizeToRequest;
    SEL selector;
    WORD err;
    PSMALL_POOL_HDR poolHdr;
    PALLOC_HDR allocHdr;

    sizeToRequest = requestedBytes;
    if (sizeToRequest != 0) {
        if (sizeToRequest < sizeof(SMALL_POOL_HDR) + sizeof(ALLOC_HDR)) {
            return NULL;
        }
    }

    err = DosAllocSeg(sizeToRequest, &selector, 0);
    if (err != NO_ERROR) {
        return NULL;
    }

    poolHdr = (PSMALL_POOL_HDR)SelToFarPtr(selector);
    poolHdr->TotalBytes = sizeToRequest - sizeof(SMALL_POOL_HDR);
    poolHdr->LargestFreeBlock = poolHdr->TotalBytes - sizeof(ALLOC_HDR);
    poolHdr->FirstFreeHeader = sizeof(SMALL_POOL_HDR);

    allocHdr = (PALLOC_HDR)(poolHdr + 1);
    allocHdr->OffsetToNext = poolHdr->TotalBytes;
    allocHdr->OffsetToPrev = 0;

#if SALLOC_DBG
    printf("New pool sel %04x %p total %i largest %i allocHdr %p prev %04x next %04x\r\n",
           selector,
           poolHdr,
           poolHdr->TotalBytes,
           poolHdr->LargestFreeBlock,
           allocHdr,
           allocHdr->OffsetToPrev,
           allocHdr->OffsetToNext);
#endif

    return poolHdr;
}

BOOL
SmallPoolIsPoolEmpty(
    PVOID pool
    )
{
    PSMALL_POOL_HDR poolHdr;
    PALLOC_HDR allocHdr;

    poolHdr = (PSMALL_POOL_HDR)pool;
    allocHdr = (PALLOC_HDR)(poolHdr + 1);

    if (AllocHdrIsAllocated(allocHdr) ||
        !AllocHdrBeyondPool(poolHdr, AllocHdrNextHdr(allocHdr))) {

        return FALSE;
    }

    return TRUE;
}

VOID
SmallPoolFreePool(
    PVOID pool
    )
{
    SEL selector;

    if (!SmallPoolIsPoolEmpty(pool)) {
#if SALLOC_DBG
        printf("alloc: pool %p freed but not empty\r\n", pool);
        SmallPoolDump(pool);
#endif
        DebugBreak();
    }

    selector = FarPtrToSel(pool);
    DosFreeSeg(selector);
}

BOOL
SmallPoolFreePoolIfEmpty(
    PVOID pool
    )
{
    if (SmallPoolIsPoolEmpty(pool)) {
        SmallPoolFreePool(pool);
        return TRUE;
    }

    return FALSE;
}

VOID
SmallPoolDump(
    PVOID pool
    )
{
    PSMALL_POOL_HDR poolHdr;
    PALLOC_HDR allocHdr;

    poolHdr = (PSMALL_POOL_HDR)pool;
    printf("Pool %08p TotalBytes %04x LargestFree %04x FirstFree %04x\r\n",
        poolHdr,
        poolHdr->TotalBytes,
        poolHdr->LargestFreeBlock,
        poolHdr->FirstFreeHeader);

    allocHdr = (PALLOC_HDR)(poolHdr + 1);
    while (!AllocHdrBeyondPool(pool, allocHdr)) {
        printf("  Hdr %08p Prev %04x Next %04x\r\n", allocHdr, allocHdr->OffsetToPrev, allocHdr->OffsetToNext);
        if (allocHdr->OffsetToNext == 0) {
            break;
        }
        allocHdr = AllocHdrNextHdr(allocHdr);
    }
}

PALLOC_HDR
SmallPoolAllocStartOffset(
    PSMALL_POOL_HDR poolHdr,
    WORD sizeToAllocate,
    WORD startOffset
    )
{
    BOOL entireScan = FALSE;
    PALLOC_HDR allocHdr;
    PALLOC_HDR nextHdr;
    WORD largestUsable;
    WORD usableBytes;
    WORD hdrOffset;

    if (startOffset <= sizeof(SMALL_POOL_HDR)) {
        startOffset = sizeof(SMALL_POOL_HDR);
        entireScan = TRUE;
    }

#if SALLOC_DBG
    printf("alloc: searching %lp for %i bytes startOffset %04x\r\n",
        poolHdr,
        sizeToAllocate,
        startOffset);
#endif

    allocHdr = AllocHdrFromOffset(poolHdr, startOffset);
    usableBytes = 0;
    largestUsable = 0;

    while (TRUE) {

#if SALLOC_DBG
        printf("alloc:     hdr %lp prev %04x next %04x\r\n", allocHdr, allocHdr->OffsetToPrev, allocHdr->OffsetToNext);
#endif

        if (!AllocHdrIsAllocated(allocHdr)) {
            hdrOffset = AllocHdrOffsetFromStart(poolHdr, allocHdr);
            if (hdrOffset < poolHdr->FirstFreeHeader) {
                poolHdr->FirstFreeHeader = hdrOffset;
            }
            usableBytes = AllocHdrUsableBytes(allocHdr);
            if (usableBytes > largestUsable) {
                largestUsable = usableBytes;
            }

            if (usableBytes >= sizeToAllocate) {
#if SALLOC_DBG
                printf("alloc:     found usable hdr %lp usable bytes %i\r\n", allocHdr, usableBytes);
#endif
                break;
            }
        }

        nextHdr = AllocHdrNextHdr(allocHdr);
        if (AllocHdrBeyondPool(poolHdr, nextHdr)) {
#if SALLOC_DBG
            printf("alloc:     hdr %lp beyond pool end\r\n", nextHdr);
#endif
            allocHdr = NULL;
            break;
        }

        if (nextHdr == allocHdr) {
#if SALLOC_DBG
            printf("alloc: allocHdr %lp zero length, prev %04x next %04x\r\n",
                allocHdr,
                allocHdr->OffsetToPrev,
                allocHdr->OffsetToNext);
#endif
            DebugBreak();
            allocHdr = NULL;
            break;
        }

        if (AllocHdrOffsetToNext(allocHdr) != AllocHdrOffsetToPrev(nextHdr)) {
#if SALLOC_DBG
            printf("alloc: prev/next mismatch, hdr %lp next %04x nextHdr %lp prev %04x\r\n",
                allocHdr,
                AllocHdrOffsetToNext(allocHdr),
                nextHdr,
                AllocHdrOffsetToPrev(nextHdr));
#endif
            DebugBreak();
            allocHdr = NULL;
            break;
        }

        allocHdr = nextHdr;
    }

    if (allocHdr == NULL) {
        if (entireScan) {
#if SALLOC_DBG
            printf("alloc:     setting %08lp LargestFreeBlock to %i bytes\r\n",
                poolHdr,
                largestUsable);
#endif

            poolHdr->LargestFreeBlock = largestUsable;
        }
        return NULL;
    }

    return allocHdr;
}

PVOID
SmallPoolAlloc(
    PVOID pool,
    WORD sizeInBytes
    )
{
    PSMALL_POOL_HDR poolHdr;
    PALLOC_HDR allocHdr;
    PALLOC_HDR nextHdr;
    PVOID userData;
    WORD usableBytes;
    WORD sizeToAllocate;
    WORD hdrOffset;

    poolHdr = (PSMALL_POOL_HDR)pool;
    if (sizeInBytes > poolHdr->LargestFreeBlock) {
#if SALLOC_DBG
        printf("alloc: %lp has largest free block %i bytes, failing %i request\r\n",
            poolHdr,
            poolHdr->LargestFreeBlock,
            sizeInBytes);
#endif
        return NULL;
    }

    // Round up the size to allocate to be 32 bit aligned.
    sizeToAllocate = (sizeInBytes + sizeof(DWORD) - 1) & ~(sizeof(DWORD) - 1);
    allocHdr = NULL;

    //
    //  If there's a hint to the first free block, start the scan there
    //

    if (poolHdr->FirstFreeHeader > sizeof(SMALL_POOL_HDR)) {
        allocHdr = SmallPoolAllocStartOffset(poolHdr, sizeToAllocate, poolHdr->FirstFreeHeader);
    }

    //
    //  If nothing was found, start the search from the beginning.  The
    //  header already indicated that LargestFreeBlock can hold this, so
    //  this scan either succeeds or recalculates LargestFreeBlock to
    //  prevent this scan happening again.
    //

    if (allocHdr == NULL) {
        allocHdr = SmallPoolAllocStartOffset(poolHdr, sizeToAllocate, sizeof(SMALL_POOL_HDR));
    }

    if (allocHdr == NULL) {
        return NULL;
    }

#if SALLOC_DBG
    printf("alloc:     poolHdr %lp allocHdr %lp\r\n", poolHdr, allocHdr);
#endif
    hdrOffset = AllocHdrOffsetFromStart(poolHdr, allocHdr);
    usableBytes = AllocHdrUsableBytes(allocHdr);

    //
    //  If the block is large enough that it can be split, split it now.
    //  The test here is saying there must be additional bytes as well as
    //  the header to make the split worthwhile.
    //

    if (usableBytes > sizeToAllocate + sizeof(ALLOC_HDR)) {
        PALLOC_HDR nextNextHdr;

        nextHdr = (PALLOC_HDR)((PUCHAR)allocHdr + sizeof(ALLOC_HDR) + sizeToAllocate);
#if SALLOC_DBG
        printf("alloc:     set nextHdr %p usableBytes %i sizeToAllocate %i\r\n", nextHdr, usableBytes, sizeToAllocate);
#endif
        nextHdr->OffsetToPrev = sizeToAllocate + sizeof(ALLOC_HDR);
        nextHdr->OffsetToNext = allocHdr->OffsetToNext - nextHdr->OffsetToPrev;
#if SALLOC_DBG
        printf("alloc:     set nextHdr %p prev %04x next %04x\r\n", nextHdr, nextHdr->OffsetToPrev, nextHdr->OffsetToNext);
        printf("alloc:     FirstFreeHeader %04x hdrOffset %04x\r\n", poolHdr->FirstFreeHeader, hdrOffset);
#endif
        if (poolHdr->FirstFreeHeader == hdrOffset) {
#if SALLOC_DBG
            printf("alloc:     set FirstFreeHeader to %04x\r\n", hdrOffset);
#endif
            poolHdr->FirstFreeHeader = AllocHdrOffsetFromStart(poolHdr, nextHdr);
        }

        nextNextHdr = AllocHdrNextHdr(nextHdr);
        if (!AllocHdrBeyondPool(poolHdr, nextNextHdr)) {
            nextNextHdr->OffsetToPrev = nextHdr->OffsetToNext;
#if SALLOC_DBG
            printf("alloc:     set nextNextHdr %p prev %04x next %04x\r\n", nextNextHdr, nextNextHdr->OffsetToPrev, nextNextHdr->OffsetToNext);
#endif
        }
        allocHdr->OffsetToNext = nextHdr->OffsetToPrev;
#if SALLOC_DBG
        printf("alloc:     allocHdr %p prev %04x next %04x\r\n", allocHdr, allocHdr->OffsetToPrev, allocHdr->OffsetToNext);
#endif
    }

    AllocHdrMarkAllocated(allocHdr);
    userData = AllocHdrUserDataFromHdr(allocHdr);
#if SALLOC_DBG
    printf("alloc: returning %lp, hdr %lp prev %04x next %04x\r\n",
        userData,
        allocHdr,
        allocHdr->OffsetToPrev,
        allocHdr->OffsetToNext);
#endif
    return userData;
}

VOID
SmallPoolFree(
    PVOID ptr
    )
{
    SEL selector;
    PSMALL_POOL_HDR poolHdr;
    PALLOC_HDR allocHdr;
    PALLOC_HDR nextHdr;
    PALLOC_HDR prevHdr;
    WORD usableBytes;
    WORD hdrOffset;

    selector = FarPtrToSel(ptr);
    poolHdr = (PSMALL_POOL_HDR)SelToFarPtr(selector);

    allocHdr = AllocHdrHdrFromUserData(ptr);
    if (!AllocHdrIsAllocated(allocHdr)) {
#if SALLOC_DBG
        printf("free: allocHdr %p not allocated, prev %04i next %04i\r\n", allocHdr, allocHdr->OffsetToPrev, allocHdr->OffsetToNext);
#endif
        DebugBreak();
    }

    AllocHdrMarkFree(allocHdr);
    hdrOffset = AllocHdrOffsetFromStart(poolHdr, allocHdr);
    if (hdrOffset < poolHdr->FirstFreeHeader) {
        poolHdr->FirstFreeHeader = hdrOffset;
    }
#if SALLOC_DBG
    printf("free: allocHdr %p free, prev %04x next %04x\r\n", allocHdr, allocHdr->OffsetToPrev, allocHdr->OffsetToNext);
#endif

    //
    //  If nextHdr is in range and free, merge the current allocHdr with it.
    //  this means allocHdr points over nextHdr, and the one after nextHdr
    //  points back to allocHdr.  Note that nextNextHdr is either allocated
    //  or beyond range; the logic below preserves its allocation flag.
    //

    nextHdr = AllocHdrNextHdr(allocHdr);
    if (!AllocHdrBeyondPool(poolHdr, nextHdr) &&
        !AllocHdrIsAllocated(nextHdr)) {

        PALLOC_HDR nextNextHdr;
        nextNextHdr = AllocHdrNextHdr(nextHdr);

#if SALLOC_DBG
        printf("free: merging right allocHdr %p nextHdr %p nextNextHdr %p\r\n", allocHdr, nextHdr, nextNextHdr);
#endif

        allocHdr->OffsetToNext = allocHdr->OffsetToNext + nextHdr->OffsetToNext;
        if (!AllocHdrBeyondPool(poolHdr, nextNextHdr)) {
            nextNextHdr->OffsetToPrev = nextNextHdr->OffsetToPrev + nextHdr->OffsetToPrev;
        }
        usableBytes = AllocHdrUsableBytes(allocHdr);
        if (usableBytes > poolHdr->LargestFreeBlock) {
            poolHdr->LargestFreeBlock = usableBytes;
        }
    }

    prevHdr = AllocHdrPrevHdr(allocHdr);
    if (allocHdr != prevHdr &&
        !AllocHdrIsAllocated(prevHdr)) {

        nextHdr = AllocHdrNextHdr(allocHdr);

#if SALLOC_DBG
        printf("free: merging left allocHdr %p prevHdr %p nextHdr %p\r\n", allocHdr, prevHdr, nextHdr);
#endif

        if (!AllocHdrBeyondPool(poolHdr, nextHdr)) {
            nextHdr->OffsetToPrev = nextHdr->OffsetToPrev + allocHdr->OffsetToPrev;
        }
        prevHdr->OffsetToNext = prevHdr->OffsetToNext + allocHdr->OffsetToNext;

        usableBytes = AllocHdrUsableBytes(prevHdr);
        if (usableBytes > poolHdr->LargestFreeBlock) {
            poolHdr->LargestFreeBlock = usableBytes;
        }

        hdrOffset = AllocHdrOffsetFromStart(poolHdr, prevHdr);
        if (hdrOffset < poolHdr->FirstFreeHeader) {
            poolHdr->FirstFreeHeader = hdrOffset;
        }
    }
}


// vim:sw=4:ts=4:et:
