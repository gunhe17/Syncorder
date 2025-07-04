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
  Syncorder\main.cpp ^
  /Fe:bin\syncorder.exe ^
  /link ^
  /LIBPATH:"C:\Users\user\Workspace\TobiiPro\64\lib" ^
  mf.lib ^
  mfplat.lib ^
  mfreadwrite.lib ^
  mfuuid.lib ^
  ole32.lib ^
  tobii_research.lib