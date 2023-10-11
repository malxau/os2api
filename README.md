# OS/2 API

This repo is a package of tools for writing 16 bit OS/2 programs with Visual C++ 1.5.

## Why OS/2? Why 16 bit?

- OS/2 because I didn't have access to tools and documentation to learn it when it was current, but now I can :)
- Because the real target is DOS (ba-dum tss!)  DOS native API is assembly, and it's convenient to have a C based function API.  The OS/2 API is as good as any.  There is already an implementation of the OS/2 API on DOS, called the Family API or FAPI.

## What's included?

- inc - A completely new OS/2 header file, `inc/os2.h` which is a recreation of the API based on documentation, internet research, and assembly includes to be new and unencombered.
- lib - A new import library, `lib/os2.lib`, generated here, also new and unencombered.
- crt - A trivial C runtime library, heavily patterned on minicrt, so that it's possible to do common C operations.  This isn't as nice as a "real" one, but...well, it's unencombered.
- dosbind - A library to extend the Family API on DOS.
- dosfapi - An alternative implementation to the Family API on DOS.  This is just enough to support dir below, and isn't a serious replacement.
- dir - A demonstration program putting it all together.

## Building

This is intended to be compiled with Visual C++ 1.5.   The assembly parts were written with MASM 6.0, and should work with higher versions.

## Contributing

I'm accepting changes, particularly if accompanied by a note indicating some interesting learning of the environment.

## License

This code is available under the MIT license.

