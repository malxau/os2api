# OS/2 API

This repo is a package of tools for writing 16 bit OS/2 programs with Visual C++ 1.5.

## Why OS/2? Why 16 bit?

- OS/2 because I didn't have access to tools and documentation to learn it when it was current, but now I can :)
- Because the real target is DOS (ba-dum tss!)  DOS native API is assembly, and it's convenient to have a C based function API.  The OS/2 API is as good as any.  There is already an implementation of the OS/2 API on DOS, called the Family API or FAPI.

## What's included?

- A completely new OS/2 header file, `inc/os2api.h` which is a recreation of the API based on documentation, internet research, and assembly includes to be new and unencombered.
- A new import library, `lib/os2.lib`, generated here, also new and unencombered.
- A trivial C runtime library, heavily patterned on minicrt, so that it's possible to do common C operations.  This isn't as nice as a "real" one, but...well, it's unencombered.
- A demonstration program putting it all together.

## Building

This is intended to be compiled with Visual C++ 1.5.   The assembly parts were written with MASM 6.0.  It may be possible to use 6.1 for this, but that version lacks native OS/2 support.

## Contributing

I'm accepting changes, particularly if accompanied by a note indicating some interesting learning of the environment.

## License

This code is available under the MIT license.

## What's not included/what's next

- Reimplement the DOS based OS/2 API library so OS/2 APIs can be used on DOS without proprietary blobs.  One outcome is being able to compile a pure-DOS (not OS/2) binary using the OS/2 API.
- From there, it should be possible to compile once and link twice, first for DOS, second for OS/2 using the DOS program as a stub, generating a single hybrid executable.
- Extend the library beyond its current limits.  In particular, it currently doesn't include any of the mouse APIs, and doesn't support long file names on DOS, even though the APIs must be long file name clean on OS/2.
- Support memory models other than large.  I'm starting with it since it's the most familiar to a "modern" developer.
- Reimplement the bind.exe tool.  This is something I'm unlikely to ever do.  It requires writing an NE loader on DOS.
