@echo off

set OLD_DIR=%cd%
if NOT EXIST "node_modules" (
  npm install simple-git
  cd %OLD_DIR%
  cls
)

:: start server with dev param
call node --expose-gc main.js -dev
timeout 10
exit