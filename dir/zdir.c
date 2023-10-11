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
        WORD SizeRequired;
        FILEFINDBUF MidPoint;
        PFILEFINDBUF TempForSwap;

        BreakPoint = Count / 2;
        SizeRequired = FIELD_OFFSET(FILEFINDBUF, achName) + Files[BreakPoint]->cchName + 1;
        memcpy(&MidPoint, Files[BreakPoint], SizeRequired);

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
}

typedef struct _ZDIR_FMTCHAR {
    UCHAR Char;
    UCHAR Attr;
} ZDIR_FMTCHAR;

typedef ZDIR_FMTCHAR FAR* PZDIR_FMTCHAR;

BOOL
ZDirWrite (
    PZDIR_FMTCHAR str,
    WORD count,
    WORD cursorRow
    )
{
    WORD i, j;
    UCHAR CacheAttr;
    UCHAR CharCache[64];
    HVIO hvio;
    WORD Err;
    WORD cursorColumn;

    hvio = 0;
    cursorColumn = 0;

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

            Err = VioWrtCharStrAtt(CharCache, j, cursorRow, cursorColumn, &CacheAttr, hvio);
            if (Err != NO_ERROR) {
                return FALSE;
            }
            cursorColumn = cursorColumn + j;
            j = 0;
        }
    }

    //
    //  If we have anything left, flush it now.
    //

    if (j > 0) {
        Err = VioWrtCharStrAtt(CharCache, j, cursorRow, cursorColumn, &CacheAttr, hvio);
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
    HVIO hvio;

    hvio = 0;
    Line = *CursorLine;
    Line++;
    VioSetCurPos(Line, 0, hvio);
    *CursorLine = Line;

    return TRUE;
}

BOOL
ZDirScrollForMoreRows(
    PVIOMODEINFO ModeInfo,
    PWORD CursorRow,
    WORD RowsToDisplay
)
{
    WORD RowsAvailableOnScreen;
    WORD RowsToScroll;
    WORD Line;
    WORD BlankCell;
    HVIO hvio;

    hvio = 0;
    Line = *CursorRow;
    RowsAvailableOnScreen = ModeInfo->wRow - Line;
    RowsToScroll = 0;

    if (RowsToDisplay > RowsAvailableOnScreen) {
        if (RowsToDisplay >= Line) {
            RowsToScroll = Line;
        } else {
            RowsToScroll = RowsToDisplay;
        }
        BlankCell = 0x07 << 8 | ' ';
        VioScrollUp(0, 0, Line - 1, ModeInfo->wColumn - 1, RowsToScroll, &BlankCell, hvio);
        Line = Line - RowsToScroll;
        VioSetCurPos(Line, 0, hvio);
        *CursorRow = Line;
    }
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

WORD
ZDirCollectFiles(
    PSZ UserSearchCriteria,
    PFILEFINDBUF * SortTable,
    PUCHAR DriveNumber,
    PWORD FilesFound
    )
{
    WORD DirHandle;
    WORD FileCount;
    WORD FindFirstContext;
    FILEFINDBUF StackFileBuf;
    PFILEFINDBUF HeapFileBuf;
    PSZ SearchCriteria;
    WORD Index;
    WORD Err;
    WORD Result;
    WORD SizeRequired;
    BOOL WildFound;

    FileCount = 0;
    SearchCriteria = UserSearchCriteria;
    Result = 0;

    *DriveNumber = 0;
    if ((SearchCriteria[0] >= 'A' && SearchCriteria[0] <= 'Z') &&
        SearchCriteria[1] == ':') {

        *DriveNumber = SearchCriteria[0] - 'A' + 1;

    } else if ((SearchCriteria[0] >= 'a' && SearchCriteria[0] <= 'z') &&
        SearchCriteria[1] == ':') {

        *DriveNumber = SearchCriteria[0] - 'a' + 1;
    }

    while(TRUE) {
        DirHandle = HDIR_SYSTEM;
        FindFirstContext = 1;
        Err = DosFindFirst(SearchCriteria, &DirHandle, FILE_HIDDEN | FILE_SYSTEM | FILE_DIRECTORY, &StackFileBuf, sizeof(StackFileBuf), &FindFirstContext, 0);
        if (Err != NO_ERROR &&
            Err != ERROR_NO_MORE_FILES &&
            Err != ERROR_PATH_NOT_FOUND) {

            printf("Error %i: %s\r\n", Err, SearchCriteria);
            Result = 1;
            break;
        }

        //
        //  If the string doesn't contain a wildcard, and it matches a
        //  directory (should only match one), append a \*.* and look again.
        //

        WildFound = FALSE;
        for (Index = 0; SearchCriteria[Index] != '\0'; Index++) {
            if (SearchCriteria[Index] == '?' || SearchCriteria[Index] == '*') {
                WildFound = TRUE;
            }
        }

        //
        //  ERROR_NO_MORE_FILES can be returned for a root directory, where
        //  unlike other directories, there's no way to return the directory
        //  itself.  However it also might be a specific object that doesn't
        //  exist.  DOS uses ERROR_PATH_NOT_FOUND for this condition.
        //

        if (Err == ERROR_NO_MORE_FILES || Err == ERROR_PATH_NOT_FOUND) {

            //
            //  Truncate trailing slashes.  If this results in an empty
            //  string, the criteria had to be for the root (and it can be
            //  added back as start of \*.* below), or if it results in a
            //  drive letter this is a root for some other drive (also
            //  handled the same way.)
            //

            while (Index > 0 &&
                   (SearchCriteria[Index - 1] == '\\' || SearchCriteria[Index - 1] == '/')) {
               Index--;
            }
            SearchCriteria[Index] = '\0';

            if (Index > 2 || (Index == 2 && SearchCriteria[1] != ':')) {
                printf("No matching files found.\r\n");
                Result = 1;
                break;
            }
        } else if (WildFound || ((StackFileBuf.attrFile & FILE_DIRECTORY) == 0)) {
            break;
        }

        SearchCriteria = malloc(Index + sizeof("\\*.*"));
        if (SearchCriteria == NULL) {
            printf("out of memory\r\n");
            Result = 1;
            break;
        }

        sprintf(SearchCriteria, "%s\\*.*", UserSearchCriteria);
    }

    if (SearchCriteria != UserSearchCriteria) {
        free(SearchCriteria);
    }

    if (Result != 0) {
        *FilesFound = 0;
        return Result;
    }

    do {
        SizeRequired = FIELD_OFFSET(FILEFINDBUF, achName) + StackFileBuf.cchName + 1;
        HeapFileBuf = malloc(SizeRequired);
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
    DWORD FileSize,
    UCHAR SuffixShift
    )
{
    UCHAR Suffixes[] = {'b', 'k', 'm', 'g', 't', '?'};
    WORD SuffixLevel = 0;
    WORD LengthInChars;
    DWORD Size;
    DWORD OldSize;

    Size = FileSize;
    SuffixLevel = SuffixShift;
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


BOOL
ZDirIsPharLap(VOID)
{
    HMODULE hPhapi;
    WORD Err;

    Err = DosGetModHandle("PHAPI", &hPhapi);
    if (Err != NO_ERROR) {
        return FALSE;
    }

    return TRUE;
}


WORD
ZDirGenerateSummary(
    PZDIR_FMTCHAR Line,
    UCHAR DriveNumber,
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
    DWORD BytesPerAllocationUnit;
    UCHAR SuffixShift;
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

    //
    //  This space is so that command.com inherits this color when it
    //  scrolls after the program exits
    //
    StrLen = sizeof(" ") - 1;
    ZDirPasteStrAndPad(&Line[LineIndex], " ", 0x7, StrLen, StrLen);
    LineIndex = LineIndex + StrLen;
    StrLen = sprintf(Str, "%i", TotalFiles);
    ZDirPasteStrAndPad(&Line[LineIndex], Str, 0xA, StrLen, StrLen);
    LineIndex = LineIndex + StrLen;
    StrLen = sizeof(" files,") - 1;
    ZDirPasteStrAndPad(&Line[LineIndex], " files,", 0x7, StrLen, StrLen);
    LineIndex = LineIndex + StrLen;
    StrLen = sprintf(Str, " %i", TotalDirectories);
    ZDirPasteStrAndPad(&Line[LineIndex], Str, 0xA, StrLen, StrLen);
    LineIndex = LineIndex + StrLen;
    StrLen = sizeof(" dirs") - 1;
    ZDirPasteStrAndPad(&Line[LineIndex], " dirs", 0x7, StrLen, StrLen);
    LineIndex = LineIndex + StrLen;

    if (ZDirIsPharLap()) {
        return LineIndex;
    }

    StrLen = sizeof(", ") - 1;
    ZDirPasteStrAndPad(&Line[LineIndex], ", ", 0x7, StrLen, StrLen);
    LineIndex = LineIndex + StrLen;
    ZDirFileSizeToString(Str, TotalUsed, 0);
    StrLen = 5;
    ZDirPasteStrAndPad(&Line[LineIndex], Str, 0xE, StrLen, StrLen);
    LineIndex = LineIndex + StrLen;
    StrLen = sizeof(" used, ") - 1;
    ZDirPasteStrAndPad(&Line[LineIndex], " used, ", 0x7, StrLen, StrLen);
    LineIndex = LineIndex + StrLen;

    DosQFSInfo(DriveNumber, 1, &FsSpace, sizeof(FsSpace));

    //
    //  Free space is returned with two 32 bit numbers and a 16 bit number.
    //  It looks like the system intended to support free space larger than
    //  32 bits.  Calculate a 32 bit allocation unit size, and see if it
    //  is a multiple of kilobytes.  If so, we can reclaim 10 bits here and
    //  use them to describe larger amounts of disk space.
    //

    SuffixShift = 0;
    BytesPerAllocationUnit = FsSpace.SectorsPerAllocationUnit * FsSpace.BytesPerSector;
    while (BytesPerAllocationUnit >= 1024) {
        BytesPerAllocationUnit = BytesPerAllocationUnit / 1024;
        SuffixShift++;
    }

    TotalUsed = FsSpace.TotalAllocationUnits * BytesPerAllocationUnit;

    ZDirFileSizeToString(Str, TotalUsed, SuffixShift);
    StrLen = 5;
    ZDirPasteStrAndPad(&Line[LineIndex], Str, 0xE, StrLen, StrLen);
    LineIndex = LineIndex + StrLen;
    StrLen = sizeof(" vol size, ") - 1;
    ZDirPasteStrAndPad(&Line[LineIndex], " vol size, ", 0x7, StrLen, StrLen);
    LineIndex = LineIndex + StrLen;

    TotalUsed = FsSpace.AllocationUnitsAvailable * BytesPerAllocationUnit;

    ZDirFileSizeToString(Str, TotalUsed, SuffixShift);
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

const UCHAR ZDirLineElements[] = {196, 194, 193, 179};

#define ZDIR_GRID_COLOR 0x2

WORD
ZDirDisplayFiles(
    PFILEFINDBUF * SortTable,
    WORD FileCount,
    UCHAR DriveNumber,
    BOOL PauseEnabled
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
    WORD LinesThisPage;
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

    // printf("\r\n");
    Err = VioGetCurPos(&CursorRow, &CursorColumn, hvio);
    if (Err != NO_ERROR) {
        return Err;
    }
    // VioSetCurPos(CursorRow - 1, CursorColumn, hvio);
    // printf("Display %ix%i, Cursor %i, %i\r\n", ModeInfo.wColumn, ModeInfo.wRow, CursorColumn, CursorRow);

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
    //  In addition to rows containing files, we have two grid lines and
    //  a summary line.
    //

    ZDirScrollForMoreRows(&ModeInfo, &CursorRow, BufferRows + 3);

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

    ZDirWrite(Line, Index, CursorRow);
    ZDirNewline(&CursorRow, &ModeInfo);

    GridCellCount = BufferRows * Columns;
    ActiveColumn = 0;
    CurrentChar = 0;
    LinesThisPage = 1;

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
                ZDirFileSizeToString(&FileSizeString[1], fb->cbFile, 0);
                Attr = 0xe;
            }
            ZDirPasteStrAndPad(&Line[CurrentChar + MaxLength], FileSizeString, Attr, 6, 6);

            CurrentChar = CurrentChar + ColumnWidth - 1;
        }

        ActiveColumn++;
        if (ActiveColumn % Columns == 0) {
            WORD LinesRemaining;
            LinesRemaining = BufferRows - (Index / Columns) + 2;
            ZDirWrite(Line, CurrentChar, CursorRow);
            CurrentChar = 0;
            ActiveColumn = 0;
            ZDirNewline(&CursorRow, &ModeInfo);
            LinesThisPage++;
            if (LinesThisPage == ModeInfo.wRow - 1) {
                if (PauseEnabled) {
                    WORD StrLen;
                    KBDKEYINFO CharData;
                    StrLen = sizeof("Press any key to continue...") - 1;
                    ZDirPasteStrAndPad(Line, "Press any key to continue...", 0xD, StrLen, StrLen);
                    ZDirWrite(Line, StrLen, CursorRow);
                    VioSetCurPos(CursorRow, StrLen, hvio);
                    KbdCharIn(&CharData, 0, 0);
                    if (CharData.cChar == 'q') {
                        return NO_ERROR;
                    }

                    //
                    //  Pretend the cursor is on the next line so this
                    //  message is included in the text that gets scrolled
                    //
                    CursorRow = CursorRow + 1;
                }
                LinesThisPage = 0;
                ZDirScrollForMoreRows(&ModeInfo, &CursorRow, LinesRemaining);
            }
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

    ZDirWrite(Line, Index, CursorRow);
    ZDirNewline(&CursorRow, &ModeInfo);

    Index = ZDirGenerateSummary(Line, DriveNumber, SortTable, FileCount);
    if (Index > 0) {
        ZDirWrite(Line, Index, CursorRow);

        //
        //  Move the cursor to after the summary on the same line.  This
        //  hints to Yori to move to the next line; other shells do this
        //  unconditionally.
        //

        VioSetCurPos(CursorRow, Index, hvio);
    }

    return NO_ERROR;
}

VOID
ZDirFreeFiles(
    PFILEFINDBUF * SortTable,
    WORD FileCount
    )
{
    WORD Index;
    for (Index = 0; Index < FileCount; Index++) {
        free(SortTable[Index]);
        SortTable[Index] = NULL;
    }
}

BOOL
ZDirIsCommandLineOptionChar(
    UCHAR Char
    )
{
    if (Char == '/' || Char == '-') {
        return TRUE;
    }
    return FALSE;
}

VOID
ZDirHelp(VOID)
{
    printf("ZDir [-?] [-license] [-p|-pn] [FileSpec]\r\n\r\n");
    printf("  -p    Pause after each screen\r\n");
    printf("  -pn   Don't pause after each screen\r\n");
}

const
CHAR strMitLicenseText[] =
     "Copyright (c) 2023 Malcolm J. Smith\r\n"
     "\r\n"
     "Permission is hereby granted, free of charge, to any person obtaining a copy\r\n"
     "of this software and associated documentation files (the \"Software\"), to deal\r\n"
     "in the Software without restriction, including without limitation the rights\r\n"
     "to use, copy, modify, merge, publish, distribute, sublicense, and/or sell\r\n"
     "copies of the Software, and to permit persons to whom the Software is\r\n"
     "furnished to do so, subject to the following conditions:\r\n"
     "\n"
     "The above copyright notice and this permission notice shall be included in\r\n"
     "all copies or substantial portions of the Software.\r\n"
     "\n"
     "THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR\r\n"
     "IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,\r\n"
     "FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE\r\n"
     "AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER\r\n"
     "LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,\r\n"
     "OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN\r\n"
     "THE SOFTWARE.\r\n";

VOID
ZDirLicense(VOID)
{
    WORD BytesWritten;
    DosWrite(1, (PSZ)strMitLicenseText, sizeof(strMitLicenseText) - 1, &BytesWritten);
}


WORD CDECL
main(
    WORD argc,
    PSZ argv[]
    )
{
    WORD FilesFound;
    PFILEFINDBUF * SortTable;
    WORD ArgIndex;
    PSZ Arg;
    WORD Result;
    WORD StartArg;
    UCHAR DriveNumber;
    BOOL OutputGenerated;
    BOOL PauseEnabled;

    PauseEnabled = TRUE;

    StartArg = 0;
    for (ArgIndex = 1; ArgIndex < argc; ArgIndex++) {
        Arg = argv[ArgIndex];
        if (ZDirIsCommandLineOptionChar(Arg[0])) {
            if (stricmp(&Arg[1], "?") == 0) {
                ZDirHelp();
                return Result;
            } else if (stricmp(&Arg[1], "license") == 0) {
                ZDirLicense();
                return Result;
            } else if (stricmp(&Arg[1], "p") == 0) {
                PauseEnabled = TRUE;
            } else if (stricmp(&Arg[1], "pn") == 0) {
                PauseEnabled = FALSE;
            }
        } else {
            StartArg = ArgIndex;
        }
    }

    SortTable = malloc(ZDIR_MAX_FILES * sizeof(PFILEFINDBUF));
    if (SortTable == NULL) {
        return 1;
    }

    //
    //  If no usable argument was found, start the loop beyond all of the
    //  arguments, which causes it to default to *.*
    //

    if (StartArg == 0) {
        StartArg = argc;
    }

    Result = 0;
    ArgIndex = StartArg;
    OutputGenerated = FALSE;

    do {
        if (ArgIndex < argc) {
            Arg = argv[ArgIndex];
            if (ZDirIsCommandLineOptionChar(Arg[0])) {
                ArgIndex++;
                continue;
            }
        } else if (!OutputGenerated) {
            Arg = "*.*";
        } else {
            break;
        }

        OutputGenerated = TRUE;

        Result = ZDirCollectFiles(Arg, SortTable, &DriveNumber, &FilesFound);
        if (Result != NO_ERROR) {
            break;
        }

        ZDirSortFiles(SortTable, FilesFound, 1);
        Result = ZDirDisplayFiles(SortTable, FilesFound, DriveNumber, PauseEnabled);
        if (Result != NO_ERROR) {
            break;
        }

        ZDirFreeFiles(SortTable, FilesFound);
        FilesFound = 0;

        ArgIndex++;
    } while (ArgIndex < argc);

    if (FilesFound > 0) {
        ZDirFreeFiles(SortTable, FilesFound);
        FilesFound = 0;
    }

    free(SortTable);

    return Result;
}
