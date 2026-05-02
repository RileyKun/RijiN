@echo off

:: create temp folder
mkdir obj

set OUTPUT_FILE=fnv1a.exe
set MATH_EXTENSION=-mavx2
make make_objects -j%NUMBER_OF_PROCESSORS%
make make_output

:: delete temp folder
rmdir /S /Q obj

start fnv1a.exe