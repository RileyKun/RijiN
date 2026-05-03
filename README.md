# RijiN/codebase_main

This project contains the entire codebase_main source code for RijiN, minus some additional projects previously discontinued / not finished.

* THIS PROJECT IS NOT GAY, IT DOES NOT USE MODERN C++ (STL/CRT) *
* THIS MEANS THIS PROJECT DOES NOT COMPILE TO A TPYICAL PE DLL, BUT INSTEAD A BLOCK OF CODE *
* READ src/loader/loader_server/src/gen.js AS IT IS VERY IMPORTANT TO UNDERSTAND HOW EVERYTHING WORKS *

project compiles its "binaries" into direct code, using the PE format for convenience but with no direct link to a typical DLL in the basic sense, with no CRT/STL, Which means the project relies entirely on own implementation for everything, this was done because DRM was a concern, and also from a pure control perspective, in relation to anti cheats and dynamic envrionments, we are able to compile code that can be directly injected into any part of the operating system, kernel etc. as a module (see loader_server/src/gen.js)

* IMPORTS ARE AUTOMATICALLY RESOLVED BY THE GENERATOR (SEE loader_server/src/gen.js) HOWEVER SOMETIMES A DIRECT REFERENCE TO THE IMPORT MAY BE REQUIRED FOR THE LOADER TO AUTOMAGICALLY LINK *

Do not try to inject the dll directly into the game, even though it is compiled as a dll, it is essentially a block of code (again see gen.js for more information)

Ofcourse there is a lot in this project, and a lot of tests take place in this project, theres a lot of silly code, and thats one of the trade offs with having a very large project, where breaking the software is not always possible, work arounds are required to allow development while not hindering the software itself.

To the right person this project is a gold mine of years of reserve, design and solutions.

I'd like to thank everyone who supported RijiN, and all the pay to cheat developers that had been our friends for years, these include
1. Developers of Aimware
2. Developers of Interwebz
3. Developers of Neverlose

# and thats all she wrote

# Credits
1. Senator - Creator of RijiN, designed the codebase, fundemental design of the loader(s), DRM (including gen), UI, cheats, security, auth, website, dashboard, etc
2. Styles - Convincing me to level up and work harder, changed me as a person, big respect bro
3. Nitro (interwebz) - Oldest cheat scene friend I have to date, helped on some very complex reversal issues
4. DurRud - Provided extensive help and creative work on tf2 and several parts of the codebase, RijiN would not be what it is today without Rud.
5. Soufiw - Helped extensively with DRM testing
6. Badster - Input into hypervisor / kernel cheats

## 1. Requirements
1. [NodeJS](https://nodejs.org/en/download/)
2. [MSYS2](http://repo.msys2.org/distrib/msys2-x86_64-latest.exe)

## 2. Setup MSYS2 & Install GCC
1. Download MYS2 ([Here](http://repo.msys2.org/distrib/msys2-x86_64-latest.exe)).
1. Install MYS2 To the directory: `C:\msys64`
1. Open MSYS2 And run the command `pacman -Syuu`
1. Keep running the Aforementioned command until it says `there is nothing to do`.
1. Open MSYS2 Again and run the following: `pacman -S --needed base-devel mingw-w64-i686-toolchain mingw-w64-x86_64-toolchain git subversion mercurial mingw-w64-i686-cmake mingw-w64-x86_64-cmake mingw-w64-x86_64-lua mingw-w64-i686-lua`

## 3. Add environment variables
1. Add `C:\msys64\mingw32\bin` to your environment paths.
1. Add `C:\msys64\mingw64\bin` to your environment paths.
1. Add `C:\msys64\usr\bin` to your environment paths.
1. Add a system environment variable called `CODEBASE_MAIN` with the value being the path of your codebase_main folder.

## 4. Setup Sublime Text Build System
1. Goto `Tools -> Build System -> New Build System`
2. Name it what ever you want and add the following:
```
{
  "shell_cmd": "node %CODEBASE_MAIN%\\src\\run_compile.js",
  "working_dir": "$file_path",
}
```
3. Save and set your current build system to the one you created.

## 5. Run setup file
1. Run setup.bat and wait for completion.
