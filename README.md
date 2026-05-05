# RijiN/codebase_main

The full source code for RijiN. Originally posted by senator. This fork modifies a handful of files to allow the cheat to compile and run out of the box with no issues.

## How to build?
1. Download [NodeJS](https://nodejs.org/en/download/) and [MSYS2](http://repo.msys2.org/distrib/msys2-x86_64-latest.exe)
2. Open the MSYS2 MSYS terminal from your start menu.
3. Run `pacman -Syuu` until `there is nothing to do.` is displayed.
4. Run `pacman -S --needed base-devel mingw-w64-i686-toolchain mingw-w64-x86_64-toolchain git subversion mercurial mingw-w64-i686-cmake mingw-w64-x86_64-cmake mingw-w64-x86_64-lua mingw-w64-i686-lua` afterwards. You can run `pacman -Syuu` again if you wish to be safe.
5. Add the following entries to your **PATH**:
```
C:\msys64\mingw32\bin
C:\msys64\mingw64\bin
C:\msys64\usr\bin
```
6. Add a **var** named `CODEBASE_MAIN` to your **environment variables**. It must be the directory that **contains this repository**, not the src folder.
* Correct:
`C:\RijiN\codebase_main`
* Incorrect:
`C:\RijiN\codebase_main\src`
7. Run `setup.bat`.
8. Run `compile_tf2_win64_required.bat` in `src`. You have now built the source.

## How to run?
After you have built the source...
1. Run `run.bat` in `src/loader/loader_server`.
2. Run the loader executable in `build`.