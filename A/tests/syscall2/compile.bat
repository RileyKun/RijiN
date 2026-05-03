@echo off

:: create temp folder
mkdir obj

set OUTPUT_FILE=time.exe
set MATH_EXTENSION=-mavx 
make make_objects -j%NUMBER_OF_PROCESSORS%
make make_output

:: delete temp folder
rmdir /S /Q obj

taskkill /IM time.exe /F
start time.exe