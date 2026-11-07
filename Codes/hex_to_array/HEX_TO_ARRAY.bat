@echo off

if "%~1"=="" (
    echo Usage: hextoarray.bat input.hex output.h
    exit /b 1
)

set input_file=%~1
set output_file=%~2

REM Count the number of valid records in the input file
set count=0
for /f "tokens=*" %%A in ('findstr "^:" "%input_file%"') do (
    set /a count+=1
)

REM Write the array declaration
(
    echo const char *arr[%count%] = {
) > "%output_file%"

REM Initialize line counter
set /a current_line=0

REM Write the array contents
for /f "tokens=*" %%A in ('findstr "^:" "%input_file%"') do (
    set "line=%%A"
    set /a current_line+=1
    setlocal enabledelayedexpansion
    set "line=!line!"
    if defined line (
        if !current_line! lss %count% (
            >> "%output_file%" echo     "!line!",
        ) else (
            >> "%output_file%" echo     "!line!"
        )
    )
    endlocal
)

REM Add closing brace
>> "%output_file%" (
    echo };
)

echo Header file saved to "%output_file%"
