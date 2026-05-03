@echo off

color 4F
echo WARNING!
echo.
echo THIS WILL DELETE ALL GITIGNORED FILES INCLUDING CLOUD CONFIGS
echo CONTINUE?
echo.
pause

color 0f
git clean -xdf

echo [+] done
timeout 4