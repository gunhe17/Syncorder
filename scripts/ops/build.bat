@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"

cl ^
  /std:c++17 ^
  /EHsc ^
  /W3 ^
  /O2 ^
  /D_CRT_SECURE_NO_WARNINGS ^
  /wd4819 ^
  /I . ^
  /I "C:\Users\user\Workspace\TobiiPro\64\include" ^
  /I "C:\Users\user\Workspace\evk_sdk\include" ^
  /I "C:\Users\user\Workspace\Intel RealSense SDK 2.0\include" ^
  /I "C:\Users\user\Workspace\Intel RealSense SDK 2.0\third-party" ^
  Syncorder\main.cpp ^
  Syncorder\gonfig\gonfig.cpp ^
  /Fe:bin\syncorder.exe ^
  /link ^
  /LIBPATH:"C:\Users\user\Workspace\TobiiPro\64\lib" ^
  /LIBPATH:"C:\Users\user\Workspace\evk_sdk\lib" ^
  /LIBPATH:"C:\Users\user\Workspace\Intel RealSense SDK 2.0\lib\x64" ^
  mf.lib ^
  mfplat.lib ^
  mfreadwrite.lib ^
  mfuuid.lib ^
  ole32.lib ^
  tobii_research.lib ^
  arducam_evk_cpp_sdk.lib ^
  realsense2.lib