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

#define MAX_ARGS 16

VOID APIENTRY __startup()
{
    WORD exitCode;
    WORD argc;
    PSZ argv[MAX_ARGS];

    argc = 0;
    {
        WORD charIndex;
        PSZ cmdLine;
        
        //
        //  The command line buffer appears to consist of a NULL terminated
        //  program string followed by a NULL terminated argument string.
        //  Argv[0] can be taken from the first string.  The second string
        //  needs to be parsed for spaces and split into different components.
        //

        cmdLine = GetCommandLine();
        charIndex = 0;
        argv[0] = cmdLine;

        for (;cmdLine[charIndex] != '\0'; charIndex++);
        charIndex++;
        argc++;

        while(TRUE) {

            for (;cmdLine[charIndex] == ' ';charIndex++);

            if (cmdLine[charIndex] == '\0') {
                break;
            }

            argv[argc] = &cmdLine[charIndex];
            for (;cmdLine[charIndex] != ' ' && cmdLine[charIndex] != '\0'; charIndex++);

            argc++;

            cmdLine[charIndex] = '\0';
            charIndex++;

            if (argc == MAX_ARGS) {
                break;
            }
        }
    }

    exitCode = main(argc, argv);
	DosExit(EXIT_PROCESS, exitCode);
}
