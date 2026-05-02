# codebase_main

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
