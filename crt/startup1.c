/**
 * @file crt/startup1.c
 *
 * OS/2 C based program initialization.  The real entry point must be in
 * assembly but complex tasks can be deferred to C.
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

PPSZ
CmdlineToArgcArgv(
    PSZ ProgramName,
    PSZ CmdLine,
    WORD MaxArgs,
    PWORD argc
    )
{
    WORD ArgCount = 0;
    WORD CharCount = 0;
    WORD SlashCount;
    PCH c;
    PPSZ ArgvArray;
    PSZ ReturnStrings;
    BOOL EndArg;
    BOOL QuoteOpen;

    //
    //  Consume all spaces.  After this, we're either at
    //  the end of string, or we have an arg, and it
    //  might start with a quote
    //

    c = CmdLine;
    while (*c == ' ') c++;

    QuoteOpen = FALSE;

    while (*c != '\0') {
        EndArg = FALSE;

        if (*c == '\\') {
            for (SlashCount = 1; c[SlashCount] == '\\'; SlashCount++);
            if (c[SlashCount] == '"') {

                //
                //  Because one char is left for regular processing, only
                //  adjust for one less pair.  Three slashes means consume
                //  two chars, output one; four means consume three, output
                //  one, etc.
                //

                if ((SlashCount % 2) == 0) {
                    SlashCount--;
                }
                CharCount += SlashCount / 2;
                c += SlashCount;
            }
        } else if (*c == '"') {
            QuoteOpen = !QuoteOpen;
            c++;
            if (ArgCount < MaxArgs && *c == '\0') {
                ArgCount++;
            }
            continue;
        } else if (!QuoteOpen && *c == ' ') {
            EndArg = TRUE;
        }

        if (ArgCount + 1 < MaxArgs && EndArg) {
            c++;
            while (*c == ' ') c++;
            ArgCount++;
        } else {
            CharCount++;

            //
            //  If we hit a break char, we count the argument then.
            //  If we hit end of string, count it here; note we're
            //  only counting it if we counted a character before it
            //  (ie., trailing whitespace is not an arg.)
            //

            c++;

            if (ArgCount < MaxArgs && *c == '\0') {
                ArgCount++;
            }
        }
    }

    //
    //  OS/2 clearly separates the program name from arguments, so there
    //  is always going to be one string for the program name.
    //

    ArgCount++;
    *argc = ArgCount;

    ArgvArray = malloc( (ArgCount * sizeof(PSZ)) + (CharCount + ArgCount) * sizeof(UCHAR));
    if (ArgvArray == NULL) {
        *argc = 0;
        return NULL;
    }

    ReturnStrings = (PSZ)(ArgvArray + ArgCount);

    ArgvArray[0] = ProgramName;
    if (ArgCount == 1) {
        return ArgvArray;
    }
    ArgCount = 1;
    ArgvArray[ArgCount] = ReturnStrings;

    //
    //  Consume all spaces.  After this, we're either at
    //  the end of string, or we have an arg, and it
    //  might start with a quote
    //

    c = CmdLine;
    while (*c == ' ') c++;
    QuoteOpen = FALSE;

    while (*c != '\0') {
        EndArg = FALSE;

        if (*c == '\\') {
            for (SlashCount = 1; c[SlashCount] == '\\'; SlashCount++);
            if (c[SlashCount] == '"') {

                //
                //  Always add one character in the regular path, below.  This
                //  code therefore needs to process each double-slash except
                //  the last one, and advance the c pointer to skip the first
                //  slash of the last pair.  After that can either be a slash
                //  or a double quote, which will be processed as a regular
                //  character below.
                //

                for (CharCount = 2; CharCount < SlashCount; CharCount += 2) {
                    *ReturnStrings = '\\';
                    ReturnStrings++;
                    c += 2;
                }
                c++;
            }
        } else if (*c == '"') {
            QuoteOpen = !QuoteOpen;
            c++;
            if (*c == '\0') {
                *ReturnStrings = '\0';
            }
            continue;
        } else if (!QuoteOpen && *c == ' ') {
            EndArg = TRUE;
        }

        if (ArgCount + 1 < MaxArgs && EndArg) {
            *ReturnStrings = '\0';
            ReturnStrings++;

            c++;
            while (*c == ' ') c++;
            if (*c != '\0') {
                ArgCount++;
                ArgvArray[ArgCount] = ReturnStrings;
            }
        } else {
            *ReturnStrings = *c;
            ReturnStrings++;

            //
            //  If we hit a break char, we count the argument then.
            //  If we hit end of string, count it here; note we're
            //  only counting it if we counted a character before it
            //  (ie., trailing whitespace is not an arg.)
            //

            c++;

            if (*c == '\0') {
                *ReturnStrings = '\0';
            }
        }
    }

    return ArgvArray;
}

VOID APIENTRY __startup()
{
    WORD exitCode;
    WORD argc;
    PPSZ argv;

    if (!malloc_init()) {
        DosExit(EXIT_PROCESS, 255);
    }

    {
        PSZ programName;
        WORD charIndex;

        //
        //  The command line buffer appears to consist of a NULL terminated
        //  program string followed by a NULL terminated argument string.
        //  Argv[0] can be taken from the first string.  The second string
        //  needs to be parsed for spaces and split into different components.
        //

        programName = GetCommandLine();
        charIndex = 0;

        for (;programName[charIndex] != '\0'; programName++);
        charIndex++;

        argv = CmdlineToArgcArgv(programName, &programName[charIndex], (WORD)-1, &argc);

        if (argv == NULL) {
            DosExit(EXIT_PROCESS, 255);
        }
    }

    exitCode = main(argc, argv);
    free(argv);
    malloc_cleanup();
    DosExit(EXIT_PROCESS, exitCode);
}
