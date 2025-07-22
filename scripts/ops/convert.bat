@echo off
setlocal enabledelayedexpansion

REM ===================================================
REM Enhanced RealSense CSV Converter with Depth Visualization
REM ===================================================

set "CSV_FILE=%~1"
if "%CSV_FILE%"=="" (
    echo Usage: %~nx0 "path\to\realsense_data.csv"
    echo.
    echo This script converts RealSense raw files to PNG with proper depth visualization
    exit /b 1
)

if not exist "%CSV_FILE%" (
    echo Error: CSV file not found: %CSV_FILE%
    exit /b 1
)

for %%F in ("%CSV_FILE%") do set "SESSION_DIR=%%~dpF"
set "SESSION_DIR=%SESSION_DIR:~0,-1%"

set /a "CONVERTED=0, FAILED=0, COLOR_COUNT=0, DEPTH_COUNT=0"

echo Processing RealSense data from: %CSV_FILE%
echo Session directory: %SESSION_DIR%
echo.

REM Process CSV (skip header)
for /f "skip=1 usebackq tokens=1,13,14,15,16 delims=," %%A in ("%CSV_FILE%") do (
    if /i "%%B"=="YES" if not "%%D"=="" (
        call :convert_color "%%D" "%%A"
        set /a COLOR_COUNT+=1
    )
    if /i "%%C"=="YES" if not "%%E"=="" (
        call :convert_depth "%%E" "%%A"
        set /a DEPTH_COUNT+=1
    )
)

echo.
echo ===== CONVERSION SUMMARY =====
echo Color frames processed: %COLOR_COUNT%
echo Depth frames processed: %DEPTH_COUNT%
echo Total converted: %CONVERTED%
echo Total failed: %FAILED%
echo ==============================
exit /b 0

:convert_color
set "PNG_PATH=%~1"
set "FRAME=%~2"
set "RAW_PATH=%PNG_PATH:.png=.raw%"
set "FULL_RAW=%SESSION_DIR%\%RAW_PATH%"
set "FULL_PNG=%SESSION_DIR%\%PNG_PATH%"

if not exist "%FULL_RAW%" (
    echo Warning: Raw file not found: %FULL_RAW%
    set /a FAILED+=1
    goto :eof
)

REM Auto-detect resolution by file size for RGB24
for %%A in ("%FULL_RAW%") do set "SIZE=%%~zA"
set "RES=640x480"
if %SIZE%==2764800 set "RES=1280x720"
if %SIZE%==6220800 set "RES=1920x1080"
if %SIZE%==921600 set "RES=640x480"

for %%F in ("%FULL_PNG%") do if not exist "%%~dpF" mkdir "%%~dpF" >nul 2>&1

echo Converting color frame %FRAME%: %RES%
ffmpeg -y -f rawvideo -pixel_format rgb24 -video_size %RES% -i "%FULL_RAW%" -compression_level 1 "%FULL_PNG%" >nul 2>&1
if %errorlevel%==0 (
    set /a CONVERTED+=1
    echo   ✓ Success: %PNG_PATH%
) else (
    set /a FAILED+=1
    echo   ✗ Failed: %PNG_PATH%
)
goto :eof

:convert_depth
set "PNG_PATH=%~1"
set "FRAME=%~2"
set "RAW_PATH=%PNG_PATH:.png=.raw%"
set "FULL_RAW=%SESSION_DIR%\%RAW_PATH%"
set "FULL_PNG=%SESSION_DIR%\%PNG_PATH%"
set "TEMP_GRAY=%FULL_PNG:.png=_temp.png%"

if not exist "%FULL_RAW%" (
    echo Warning: Raw file not found: %FULL_RAW%
    set /a FAILED+=1
    goto :eof
)

REM Auto-detect resolution by file size for uint16 (2 bytes per pixel)
for %%A in ("%FULL_RAW%") do set "SIZE=%%~zA"
set "RES=640x480"
if %SIZE%==1228800 set "RES=848x480"
if %SIZE%==1843200 set "RES=1280x720"
if %SIZE%==614400 set "RES=640x480"

for %%F in ("%FULL_PNG%") do if not exist "%%~dpF" mkdir "%%~dpF" >nul 2>&1

echo Converting depth frame %FRAME%: %RES%

REM Step 1: Convert raw uint16 to grayscale PNG with normalization
ffmpeg -y -f rawvideo -pixel_format gray16le -video_size %RES% -i "%FULL_RAW%" ^
    -vf "format=gray16le,lutdepth=16:y=if(eq(val\,0)\,0\,clip((val-200)*255/(10000-200)\,0\,255))" ^
    -pix_fmt gray "%TEMP_GRAY%" >nul 2>&1

if %errorlevel% neq 0 (
    echo   ✗ Failed at grayscale conversion: %PNG_PATH%
    set /a FAILED+=1
    goto :cleanup_depth
)

REM Step 2: Apply colormap for better visualization
ffmpeg -y -i "%TEMP_GRAY%" ^
    -vf "pseudocolor=preset=turbo:opacity=1" ^
    -compression_level 1 "%FULL_PNG%" >nul 2>&1

if %errorlevel%==0 (
    set /a CONVERTED+=1
    echo   ✓ Success: %PNG_PATH%
) else (
    echo   ✗ Failed at colormap application: %PNG_PATH%
    set /a FAILED+=1
)

:cleanup_depth
if exist "%TEMP_GRAY%" del "%TEMP_GRAY%" >nul 2>&1
goto :eof