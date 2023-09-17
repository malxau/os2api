/**
 * @file dir/zdir.c
 *
 * OS/2 directory enumerator to demonstrate the rest of this code.
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

//
//  Files are sorted, and sorting requires a table of pointers to sort.
//  This limits the size of that table to some arbitrary value.  Note that
//  due to 64Kb segments, this program can only allocate around 63Kb of
//  contiguous buffer, and with 32 bits per pointer that implies a limit
//  of 16000 files due to memory layout.
//
#define ZDIR_MAX_FILES 4096

WORD
ZDirSortFilesInternal(
    PFILEFINDBUF * Files,
    WORD Count
    )
{
    WORD FirstOffset;
    WORD Index;
    FirstOffset = 0;
    while (TRUE) {
        WORD BreakPoint;
        WORD LastOffset;
        FILEFINDBUF MidPoint;
        PFILEFINDBUF TempForSwap;

        BreakPoint = Count / 2;
        memcpy(&MidPoint, Files[BreakPoint], sizeof(FILEFINDBUF));

        FirstOffset = 0;
        LastOffset = Count - 1;

        //
        //  Scan from the beginning looking for an item that should be sorted
        //  after the midpoint.
        //

        for (FirstOffset = 0; FirstOffset < LastOffset; FirstOffset++) {
            if (stricmp(Files[FirstOffset]->achName,MidPoint.achName) >= 0) {
                for (; LastOffset > FirstOffset; LastOffset--) {
                    if (stricmp(Files[LastOffset]->achName,MidPoint.achName) < 0) {
                        TempForSwap = Files[FirstOffset];
                        Files[FirstOffset] = Files[LastOffset];
                        Files[LastOffset] = TempForSwap;
                        LastOffset--;
                        break;
                    }
                }
                if (LastOffset <= FirstOffset) {
                    break;
                }
            }
        }

        FirstOffset = LastOffset;
        if (stricmp(Files[FirstOffset]->achName, MidPoint.achName) < 0) {
            FirstOffset++;
        }

        //
        //  If no copies occurred, check if everything in the list is
        //  sorted.
        //

        if (FirstOffset == 0 || FirstOffset == Count) {
            for (Index = 0; Index < Count - 1; Index++) {
                if (stricmp(Files[Index]->achName, Files[Index + 1]->achName) > 0) {
                    break;
                }
            }
            if (Index == Count - 1) {
                return 0;
            }

            //
            //  Nothing was copied and it's not because it's identical.  This
            //  implies a very bad choice of midpoint that belongs either at
            //  the beginning or the end (so nothing could move past it.)
            //  Move it around and repeat the process (which will get a new
            //  midpoint.)
            //

            if (FirstOffset == 0) {
                if (stricmp(Files[0]->achName, MidPoint.achName) > 0) {
                    TempForSwap = Files[0];
                    Files[0] = Files[BreakPoint];
                    Files[BreakPoint] = TempForSwap;
                }
            } else {
                TempForSwap = Files[Count - 1];
                Files[Count - 1] = Files[BreakPoint];
                Files[BreakPoint] = TempForSwap;
            }
        } else {
            break;
        }
    }

    return FirstOffset;
}

VOID
ZDirSortFiles(
    PFILEFINDBUF * Files,
    WORD Count,
    WORD Depth
    )
{
    WORD FirstOffset;
    WORD Index;

    if (Count <= 1) {
        return;
    }

    FirstOffset = ZDirSortFilesInternal(Files, Count);

    //
    //  If there are two sets (FirstOffset is not at either end), recurse.
    //

    if (FirstOffset && (Count - FirstOffset)) {
        if (FirstOffset > 0) {
            ZDirSortFiles(Files, FirstOffset, Depth + 1);
        }
        if ((Count - FirstOffset) > 0) {
            ZDirSortFiles(&Files[FirstOffset], Count - FirstOffset, Depth + 1);
        }
    }

    //
    //  Check that it's now sorted
    //
    for (Index = 0; Index < Count - 1; Index++) {
        if (stricmp(Files[Index]->achName, Files[Index + 1]->achName) > 0) {
            break;
        }
    }

    if (Index != Count - 1) {
        DebugBreak();
    }
}

typedef struct _ZDIR_FMTCHAR {
    UCHAR Char;
    UCHAR Attr;
} ZDIR_FMTCHAR;

typedef ZDIR_FMTCHAR FAR* PZDIR_FMTCHAR;

BOOL
ZDirWrite (
    PZDIR_FMTCHAR str,
    WORD count
    )
{
    WORD i, j;
    WORD CursorRow;
    WORD CursorColumn;
    UCHAR CacheAttr;
    UCHAR CharCache[64];
    HVIO hvio;
    WORD Err;

    hvio = 0;

    Err = VioGetCurPos(&CursorRow, &CursorColumn, hvio);
    if (Err != NO_ERROR) {
        return FALSE;
    }

    //
    //  We firstly load as much as we can into a stack buffer, then write it
    //  out.  Those syscalls are expensive - run in a slow VM to see.  We
    //  have to write out the buffer either when the next character has
    //  different color (so we can't use the same WriteConsole call) or when
    //  the stack buffer is full.
    //

    j = 0;
    for (i = 0; i < count; i++) {

        CacheAttr = str[i].Attr;
        CharCache[j] = str[i].Char;
        j++;

        if ((i + 1 < count && CacheAttr != str[i + 1].Attr) ||
            (j >= sizeof(CharCache)/sizeof(CharCache[0]))) {

            Err = VioWrtCharStrAtt(CharCache, j, CursorRow, CursorColumn, &CacheAttr, hvio);
            if (Err != NO_ERROR) {
                return FALSE;
            }
            CursorColumn = CursorColumn + j;
            j = 0;
        }
    }

    //
    //  If we have anything left, flush it now.
    //

    if (j > 0) {
        Err = VioWrtCharStrAtt(CharCache, j, CursorRow, CursorColumn, &CacheAttr, hvio);
        if (Err != NO_ERROR) {
            return FALSE;
        }
    }

    return TRUE;
}

BOOL
ZDirNewline(
    PWORD CursorLine,
    PVIOMODEINFO ModeInfo
)
{
    WORD Line;
    WORD BlankCell;
    HVIO hvio;
    
    hvio = 0;
    Line = *CursorLine;

    //
    //  VIO functions don't scroll the viewport.  Check if we're at the
    //  bottom and if so, scroll the viewport up.
    //

    Line++;
    if (Line >= ModeInfo->wRow) {
        BlankCell = 0x07 << 8 | ' ';
        VioScrollUp(1, 0, Line - 1, ModeInfo->wColumn - 1, 1, &BlankCell, hvio);
        Line--;
    }
    VioSetCurPos(Line, 0, hvio);
    *CursorLine = Line;

    return TRUE;
}

BOOL
ZDirPasteStrAndPad (
    PZDIR_FMTCHAR str,
    PUCHAR src,
    UCHAR attr,
    WORD count,
    WORD padsize
    )
{
    WORD i;

    for (i = 0; i < count && i < padsize; i++) {
        str[i].Char = src[i];
        str[i].Attr = attr;
    }

    for (; i < padsize; i++) {
        str[i].Char = ' ';
        str[i].Attr = attr;
    }

    return TRUE;
}


#define FIELD_OFFSET(type, field) ((WORD)&(((type NEAR *)0)->field))


WORD
ZDirCollectFiles(
    PSZ SearchCriteria,
    PVOID Pool,
    PFILEFINDBUF * SortTable,
    PWORD FilesFound
    )
{
    WORD DirHandle;
    WORD FileCount;
    WORD FindFirstContext;
    FILEFINDBUF StackFileBuf;
    PFILEFINDBUF HeapFileBuf;
    WORD Err;
    WORD Result;
    WORD SizeRequired;

    DirHandle = 1;
    FileCount = 0;
    FindFirstContext = 1;

    Err = DosFindFirst(SearchCriteria, &DirHandle, FILE_DIRECTORY, &StackFileBuf, sizeof(StackFileBuf), &FindFirstContext, 0);
    if (Err != NO_ERROR) {
        printf("Error %i\r\n", Err);
        *FilesFound = 0;
        return 1;
    }

    Result = 0;

    do {
        SizeRequired = FIELD_OFFSET(FILEFINDBUF, achName) + StackFileBuf.cchName + 1;
        HeapFileBuf = HugePoolAlloc(Pool, SizeRequired);
        if (HeapFileBuf == NULL) {
            printf("out of memory\r\n");
            Result = 1;
            break;
        }
        memcpy(HeapFileBuf, &StackFileBuf, SizeRequired);
        SortTable[FileCount] = HeapFileBuf;
        FileCount++;
        if (FileCount >= ZDIR_MAX_FILES) {
            printf("too many files\r\n");
            Result = 1;
            break;
        }
        Err = DosFindNext(DirHandle, &StackFileBuf, sizeof(StackFileBuf), &FindFirstContext);
    } while(Err == 0);

    DosFindClose(DirHandle);
    *FilesFound = FileCount;
    return Result;
}

BOOL
ZDirFileSizeToString(
    PUCHAR String,
    DWORD FileSize
    )
{
    UCHAR Suffixes[] = {'b', 'k', 'm'};
    WORD SuffixLevel = 0;
    WORD LengthInChars;
    DWORD Size;
    DWORD OldSize;

    Size = FileSize;
    OldSize = Size;

    while (Size > 9999) {
        SuffixLevel++;
        OldSize = Size;

        //
        //  Conceptually we want to divide by 1024.
        //

        Size = Size / 1024;
    }

    if (SuffixLevel >= sizeof(Suffixes)/sizeof(CHAR)) {
        SuffixLevel = sizeof(Suffixes)/sizeof(UCHAR) - 1;
    }

    if (Size < 100 && SuffixLevel > 0) {
        OldSize = (OldSize % 1024) * 10 / 1024;
        LengthInChars = sprintf(String, "%2i.%1i%c", (WORD)Size, (WORD)OldSize, Suffixes[SuffixLevel]);
    } else {
        LengthInChars = sprintf(String, "%4i%c", (WORD)Size, Suffixes[SuffixLevel]);
    }
    return TRUE;
}

UCHAR
ZDirAttributesForFile(
    PFILEFINDBUF fb
    )
{                   
    WORD Index;
    PSZ Ext;

    if (fb->attrFile & FILE_DIRECTORY) {
        return 0x0D;
    }

    for (Index = fb->cchName; Index > 0; Index--) {
        if (fb->achName[Index - 1] == '.') {
            break;
        }
    }

    if (Index == 0) {
        return 0x07;
    }

    Ext = &fb->achName[Index];
    if (stricmp(Ext, "exe") == 0 ||
        stricmp(Ext, "com") == 0) {

        return 0x0B;
    } else if (stricmp(Ext, "bat") == 0 ||
               stricmp(Ext, "cmd") == 0 ||
               stricmp(Ext, "ys1") == 0) {
        return 0x0C;
    }

    return 0x07;
}

WORD
ZDirGenerateSummary(
    PZDIR_FMTCHAR Line,
    PFILEFINDBUF * SortTable,
    WORD FileCount
    )
{
    WORD Index;
    WORD TotalFiles;
    WORD TotalDirectories;
    WORD LineIndex;
    WORD StrLen;
    UCHAR Str[16];
    DWORD TotalUsed;
    PFILEFINDBUF fb;
    FSALLOCATE FsSpace;

    TotalFiles = 0;
    TotalDirectories = 0;
    TotalUsed = 0;

    for (Index = 0; Index < FileCount; Index++) {
        fb = SortTable[Index];
        if (fb->attrFile & FILE_DIRECTORY) {
            if (strcmp(fb->achName, ".") != 0 &&
                strcmp(fb->achName, "..") != 0) {

                TotalDirectories++;
            }
        } else {
            TotalFiles++;
            TotalUsed = TotalUsed + fb->cbFile;
        }
    }

    LineIndex = 0;
    StrLen = sprintf(Str, " %i", TotalFiles);
    ZDirPasteStrAndPad(&Line[LineIndex], Str, 0xA, StrLen, StrLen);
    LineIndex = LineIndex + StrLen;
    StrLen = sizeof(" files,") - 1;
    ZDirPasteStrAndPad(&Line[LineIndex], " files,", 0x7, StrLen, StrLen);
    LineIndex = LineIndex + StrLen;
    StrLen = sprintf(Str, " %i", TotalDirectories);
    ZDirPasteStrAndPad(&Line[LineIndex], Str, 0xA, StrLen, StrLen);
    LineIndex = LineIndex + StrLen;
    StrLen = sizeof(" dirs, ") - 1;
    ZDirPasteStrAndPad(&Line[LineIndex], " dirs, ", 0x7, StrLen, StrLen);
    LineIndex = LineIndex + StrLen;
    ZDirFileSizeToString(Str, TotalUsed);
    StrLen = 5;
    ZDirPasteStrAndPad(&Line[LineIndex], Str, 0xE, StrLen, StrLen);
    LineIndex = LineIndex + StrLen;
    StrLen = sizeof(" used, ") - 1;
    ZDirPasteStrAndPad(&Line[LineIndex], " used, ", 0x7, StrLen, StrLen);
    LineIndex = LineIndex + StrLen;

    // MSFIX This might not be the right device
    DosQFSInfo(0, 1, &FsSpace, sizeof(FsSpace));

    TotalUsed = FsSpace.TotalAllocationUnits *
                FsSpace.SectorsPerAllocationUnit *
                FsSpace.BytesPerSector;

    ZDirFileSizeToString(Str, TotalUsed);
    StrLen = 5;
    ZDirPasteStrAndPad(&Line[LineIndex], Str, 0xE, StrLen, StrLen);
    LineIndex = LineIndex + StrLen;
    StrLen = sizeof(" vol size, ") - 1;
    ZDirPasteStrAndPad(&Line[LineIndex], " vol size, ", 0x7, StrLen, StrLen);
    LineIndex = LineIndex + StrLen;

    TotalUsed = FsSpace.AllocationUnitsAvailable *
                FsSpace.SectorsPerAllocationUnit *
                FsSpace.BytesPerSector;

    ZDirFileSizeToString(Str, TotalUsed);
    StrLen = 5;
    ZDirPasteStrAndPad(&Line[LineIndex], Str, 0xE, StrLen, StrLen);
    LineIndex = LineIndex + StrLen;
    StrLen = sizeof(" vol free") - 1;
    ZDirPasteStrAndPad(&Line[LineIndex], " vol free", 0x7, StrLen, StrLen);
    LineIndex = LineIndex + StrLen;

    return LineIndex;
}

#define ZDIR_MAX_WIDTH 160

#define ZDIR_LINE_ELEMENT_HORIZ      0
#define ZDIR_LINE_ELEMENT_TOP_T      1
#define ZDIR_LINE_ELEMENT_BOTTOM_T   2
#define ZDIR_LINE_ELEMENT_VERT       3

UCHAR ZDirLineElements[] = {196, 194, 193, 179};

#define ZDIR_GRID_COLOR 0x2

WORD
ZDirDisplayFiles(
    PFILEFINDBUF * SortTable,
    WORD FileCount
    )
{
    PFILEFINDBUF fb;
    WORD Index;
    WORD FileIndex;
    ZDIR_FMTCHAR Line[ZDIR_MAX_WIDTH];
    VIOMODEINFO ModeInfo;
    WORD CursorRow;
    WORD CursorColumn;
    WORD LongestDisplayName;
    WORD MetadataWidth;
    WORD BufferRows;
    WORD ColumnWidth;
    WORD Columns;
    WORD GridCellCount;
    WORD Err;
    WORD ActiveColumn;
    WORD CurrentChar;
    HVIO hvio;

    hvio = 0;

    ModeInfo.cb = sizeof(ModeInfo);
    Err = VioGetMode(&ModeInfo, hvio);
    if (Err != NO_ERROR) {
        printf("VioGetMode returned %i\r\n", Err);
        return Err;
    }

    Err = VioGetCurPos(&CursorRow, &CursorColumn, hvio);
    if (Err != NO_ERROR) {
        return Err;
    }

    LongestDisplayName = 0;
    for (Index = 0; Index < FileCount; Index++) {
        if (SortTable[Index]->cchName > LongestDisplayName) {
            LongestDisplayName = SortTable[Index]->cchName;
        }
    }

    ColumnWidth = ModeInfo.wColumn;
    if (ColumnWidth > ZDIR_MAX_WIDTH) {
        ColumnWidth = ZDIR_MAX_WIDTH;
    }

    //
    //  Five chars for size, plus space, plus seperator
    //  MSFIX This needs to get smarter
    //

    MetadataWidth = 7;

    Columns = ColumnWidth / (LongestDisplayName + MetadataWidth);
    if (Columns > 0) {
        ColumnWidth = ColumnWidth / Columns;
    } else {
        Columns = 1;
    }

    BufferRows = (FileCount + Columns - 1) / Columns;

    //
    //  Draw the top grid line.
    //

    for (Index = 0; Index < ColumnWidth * Columns; Index++) {
        if (Index % ColumnWidth == ColumnWidth - 1 && Index < (ColumnWidth*Columns-1)) {
            Line[Index].Char = ZDirLineElements[ZDIR_LINE_ELEMENT_TOP_T];
        } else {
            Line[Index].Char = ZDirLineElements[ZDIR_LINE_ELEMENT_HORIZ];
        }

        Line[Index].Attr = ZDIR_GRID_COLOR;
    }

    ZDirWrite(Line, Index);
    ZDirNewline(&CursorRow, &ModeInfo);

    GridCellCount = BufferRows * Columns;
    ActiveColumn = 0;
    CurrentChar = 0;

    for (Index = 0; Index < GridCellCount; Index++) {

        FileIndex = ActiveColumn * BufferRows + Index / Columns;
        if (FileIndex < FileCount) {
            fb = SortTable[FileIndex];
        } else {
            fb = NULL;
        }

        if (fb == NULL) {
            ZDirPasteStrAndPad(&Line[CurrentChar], NULL, 0x7, 0, ColumnWidth - 1);
            CurrentChar = CurrentChar + ColumnWidth - 1;
        } else {
            UCHAR FileSizeString[7];
            WORD MaxLength;
            WORD FileNameLength;
            UCHAR Attr;

            MaxLength = ColumnWidth - MetadataWidth;
            FileNameLength = MaxLength;
            if (fb->cchName < FileNameLength) {
                FileNameLength = fb->cchName;
            }

            Attr = ZDirAttributesForFile(fb);
            ZDirPasteStrAndPad(&Line[CurrentChar], fb->achName, Attr, FileNameLength, MaxLength);
            FileSizeString[0] = ' ';
            if (fb->attrFile & FILE_DIRECTORY) {
                sprintf(&FileSizeString[1], "<DIR>");
                Attr = 0xd;
            } else {
                FileSizeString[1] = '\0';
                ZDirFileSizeToString(&FileSizeString[1], fb->cbFile);
                Attr = 0xe;
            }
            ZDirPasteStrAndPad(&Line[CurrentChar + MaxLength], FileSizeString, Attr, 6, 6);

            CurrentChar = CurrentChar + ColumnWidth - 1;
        }

        ActiveColumn++;
        if (ActiveColumn % Columns == 0) {
            ZDirWrite(Line, CurrentChar);
            CurrentChar = 0;
            ActiveColumn = 0;
            ZDirNewline(&CursorRow, &ModeInfo);
        } else {
            Line[CurrentChar].Char = ZDirLineElements[ZDIR_LINE_ELEMENT_VERT];
            Line[CurrentChar].Attr = ZDIR_GRID_COLOR;
            CurrentChar++;
        }
    }

    //
    //  Draw the bottom grid line.
    //

    for (Index = 0; Index < ColumnWidth * Columns; Index++) {
        if (Index % ColumnWidth == ColumnWidth - 1 && Index < (ColumnWidth*Columns-1)) {
            Line[Index].Char = ZDirLineElements[ZDIR_LINE_ELEMENT_BOTTOM_T];
        } else {
            Line[Index].Char = ZDirLineElements[ZDIR_LINE_ELEMENT_HORIZ];
        }

        Line[Index].Attr = ZDIR_GRID_COLOR;
    }

    ZDirWrite(Line, Index);
    ZDirNewline(&CursorRow, &ModeInfo);

    Index = ZDirGenerateSummary(Line, SortTable, FileCount);
    if (Index > 0) {
        ZDirWrite(Line, Index);
        
        //
        //  Move the cursor to after the summary on the same line.  This
        //  hints to Yori to move to the next line; other shells do this
        //  unconditionally.
        //

        VioSetCurPos(CursorRow, Index, hvio);
        // ZDirNewline(&CursorRow, &ModeInfo);
    }

    return NO_ERROR;
}

VOID
ZDirFreeFiles(
    PVOID Pool,
    PFILEFINDBUF * SortTable,
    WORD FileCount
    )
{
    WORD Index;
    for (Index = 0; Index < FileCount; Index++) {
        HugePoolFree(SortTable[Index]);
        SortTable[Index] = NULL;
    }
}

WORD CDECL
main(
    WORD argc,
    PSZ argv[]
    )
{
    WORD FilesFound;
    PFILEFINDBUF * SortTable;
    PVOID Pool;
    WORD ArgIndex;
    PSZ SearchCriteria;
    WORD Result;

    Pool = HugePoolAllocNewPool(1ul * 1024 * 1024);
    if (Pool == NULL) {
        return 1;
    }

    SortTable = HugePoolAlloc(Pool, ZDIR_MAX_FILES * sizeof(PFILEFINDBUF));
    if (SortTable == NULL) {
        return 1;
    }

    Result = 0;

    ArgIndex = 1;
    do {
        if (ArgIndex < argc) {
            SearchCriteria = argv[ArgIndex];
        } else {
            SearchCriteria = "*.*";
        }
        Result = ZDirCollectFiles(SearchCriteria, Pool, SortTable, &FilesFound);
        if (Result != NO_ERROR) {
            break;
        }

        ZDirSortFiles(SortTable, FilesFound, 1);
        Result = ZDirDisplayFiles(SortTable, FilesFound);
        if (Result != NO_ERROR) {
            break;
        }

        ZDirFreeFiles(Pool, SortTable, FilesFound);
        FilesFound = 0;

        ArgIndex++;
    } while (ArgIndex < argc);

    if (FilesFound > 0) {
        ZDirFreeFiles(Pool, SortTable, FilesFound);
        FilesFound = 0;
    }

    HugePoolFree(SortTable);
    HugePoolFreePool(Pool);

    return Result;
}
