@echo off

:: 1. Fetch old directory
:: 2. Fetch folder name and get build folder
:: 3. Generate current cmakelist if its build folder is empty
:: 4. Navigate to build folder and run make
set OLD_DIR=%cd%
for %%d in (".") do set BUILD_FOLDER=%CODEBASE_MAIN%/build/cmake/%%~nxd
if not exist %BUILD_FOLDER% ( cd %CODEBASE_MAIN% && call setup.bat )
cd %BUILD_FOLDER%

cmake %OLD_DIR% -DBUILD_FLAG_USE_SIMD=avx && cmake --build . -j%NUMBER_OF_PROCESSORS%
cmake %OLD_DIR% -DBUILD_FLAG_USE_SIMD=sse2 && cmake --build . -j%NUMBER_OF_PROCESSORS%
cmake %OLD_DIR% -DBUILD_FLAG_USE_SIMD=avx512f && cmake --build . -j%NUMBER_OF_PROCESSORS%
::cmake %OLD_DIR% -DBUILD_FLAG_USE_SIMD=sse && cmake --build . -j%NUMBER_OF_PROCESSORS%
cmake %OLD_DIR% -DBUILD_FLAG_USE_SIMD=sse2 && cmake --build . -j%NUMBER_OF_PROCESSORS%
cmake %OLD_DIR% -DBUILD_FLAG_USE_SIMD=sse3 && cmake --build . -j%NUMBER_OF_PROCESSORS%
cmake %OLD_DIR% -DBUILD_FLAG_USE_SIMD=ssse3 && cmake --build . -j%NUMBER_OF_PROCESSORS%
cmake %OLD_DIR% -DBUILD_FLAG_USE_SIMD=sse4.1 && cmake --build . -j%NUMBER_OF_PROCESSORS%
cmake %OLD_DIR% -DBUILD_FLAG_USE_SIMD=sse4.2 && cmake --build . -j%NUMBER_OF_PROCESSORS%

:: reset to default
cmake %OLD_DIR% -DBUILD_FLAG_USE_SIMD=sse2