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
  test/test_syncorder/test_syncorder.cpp ^
  /Fe:test/test_syncorder/test_syncorder.exe ^
  /link