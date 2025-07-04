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
  Syncorder\main.cpp ^
  /Fe:bin\syncorder.exe ^
  /link ^
  mf.lib ^
  mfplat.lib ^
  mfreadwrite.lib ^
  mfuuid.lib ^
  ole32.lib
