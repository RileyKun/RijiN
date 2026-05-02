@echo off

:: remove cmake directory entirely
rmdir /s /q %CODEBASE_MAIN%\build >nul 2>&1
rm -rf %CODEBASE_MAIN%/src/loader/loader_server/bin/*.dll

:: create build folders (if empty)
if NOT EXIST "build/x86" mkdir "build/x86"
if NOT EXIST "build/x64" mkdir "build/x64"

:: find all CMakeLists in entire project and store
:: Directory into %%~dpi
for /f "tokens=*" %%i in ('dir CMakeLists.txt /b /s') do (
  :: Extract folder name and store into %%~nxD
  for %%d in ("%%~dpi\.") do (
    start /MIN cmake -Wno-dev -G "MinGW Makefiles" -S %%~dpi -B %CODEBASE_MAIN%\build\cmake\%%~nxd > nul
	echo [!] building makefile for "%%~dpi"
  )
)

:LOOP
tasklist | find /i "cmake" >nul 2>&1
if errorlevel 1 (
  GOTO CONTINUE
) else (
  echo [!] waiting for cmake jobs...
  Timeout /T 1 /Nobreak > nul
  goto LOOP
)
:CONTINUE

echo [+] DONE!

echo.
echo *************************
echo [+] Generated Build system(s)


:: Generate post-checkout hook to clean cmake folder on branch swap
if EXIST ".git" (
  for /f "tokens=*" %%i in ('where sh') do (
    echo #!%%i                							>   .git/hooks/post-checkout
    echo rm -rf build   								>>  .git/hooks/post-checkout
    echo rm -rf src/loader/loader_server/bin/*.dll 		>>  .git/hooks/post-checkout

    echo [+] Generated git hook "post-checkout"
  )
)

echo.

:: only pause when directly called and not from a compile script
if not defined BUILD_FOLDER timeout 4