@echo off

cd %CODEBASE_MAIN%\src\loader\loader_client
call compile.bat

cd %CODEBASE_MAIN%\src\loader\loader_stub
call compile.bat

cd %CODEBASE_MAIN%\src\other\vac_bypass
call compile.bat

cd %CODEBASE_MAIN%\src\other\dxgidmp
call compile.bat

cd %CODEBASE_MAIN%\src\cheats\left4dead2
call compile.bat

timeout 5