@if not "%WindowsSdkDir%" == ""  goto build

@call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\Common7\Tools\vsvars32.bat"

@set PATH=%WindowsSdkDir%bin\%WindowsSDKVersion%\x86;%PATH%

:build
nmake %1
