@echo off
if not "%WindowsSdkDir%" == "" goto build

echo Setup Env...

if exist "C:\Program Files\Microsoft Visual Studio\18\Professional\Common7\Tools\" (
    echo Found 2026 Professional
    call "C:\Program Files\Microsoft Visual Studio\18\Professional\Common7\Tools\VsDevCmd.bat"
    set BEIDCONNECT_VS_PROJECTPATH=VS_2026
    goto build
)
if exist "C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\Tools\" (
    echo Found 2022 Professional
    call "C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\Tools\VsDevCmd.bat"
    set BEIDCONNECT_VS_PROJECTPATH=VS_2022
    goto build
)
if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\" (
    echo Found 2022 Community
    call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"
    set BEIDCONNECT_VS_PROJECTPATH=VS_2022
    goto build
)
if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\Common7\Tools\" (
    echo Found 2019 Professional
    call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional\Common7\Tools\VsDevCmd.bat"
    set BEIDCONNECT_VS_PROJECTPATH=VS_2019
    goto build
)
if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\" (
    echo Found 2019 Community
    call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"
    set BEIDCONNECT_VS_PROJECTPATH=VS_2019
    goto build
)
if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\Common7\Tools\" (
    echo Found 2019 BuildTools
    call "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\Common7\Tools\VsDevCmd.bat"
    set BEIDCONNECT_VS_PROJECTPATH=VS_2019
    goto build
)

:build
echo Start Build...
nmake %1
