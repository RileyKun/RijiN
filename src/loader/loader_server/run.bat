@echo off

:: kill any instances of node
taskkill /f /im node.exe

set OLD_DIR=%cd%
if NOT EXIST "src/node_modules" (
  call install_npm.bat
  cd %OLD_DIR%
  cls
)

:: start server with dev param
node --expose-gc src/main.js -dev
::cd ..
start "" node --expose-gc loader_sync/main.js -dev
::
::start "" node --expose-gc "%CODEBASE_MAIN%\src\other\dev_watch\main.js" -dev

timeout 10
exit