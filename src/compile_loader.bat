@echo off

cd %CODEBASE_MAIN%\src\loader\loader_client
call compile.bat

cd %CODEBASE_MAIN%\src\loader\loader_stub
call compile.bat

timeout 5