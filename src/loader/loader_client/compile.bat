@echo off

:: 1. Fetch old directory
:: 2. Fetch folder name and get build folder
:: 3. Generate current cmakelist if its build folder is empty
:: 4. Navigate to build folder and run make
set OLD_DIR=%cd%
for %%d in (".") do set BUILD_FOLDER=%CODEBASE_MAIN%/build/cmake/%%~nxd
if not exist %BUILD_FOLDER% ( cd %CODEBASE_MAIN% && call setup.bat )
cd %BUILD_FOLDER%

::cmake --build . --target clean
cmake --build . -j%NUMBER_OF_PROCESSORS%

:: patch to require admin priv
cd %OLD_DIR%
manifest_tool.exe -nologo -manifest data.manifest -hashupdate -outputresource:%CODEBASE_MAIN%\build\x64\loader.exe