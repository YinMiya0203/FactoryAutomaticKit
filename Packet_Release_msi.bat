echo off
set DEVENV="E:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\devenv.com"
%DEVENV% FactoryAutoTest.sln /Rebuild "Release_console|x64" /project FactoryAutoTest
if not %errorlevel% == 0 ( goto ERROR_OUT )
%DEVENV% FactoryAutoTest.sln /Rebuild "Release|x64" /project FactoryAutoTest
if not %errorlevel% == 0 ( goto ERROR_OUT )
%DEVENV% FactoryAutoTest.sln /Rebuild "Release" /project SetupFactoryAutoTest
if not %errorlevel% == 0 ( goto ERROR_OUT )
goto END
:ERROR_OUT
echo make fail
goto END
:END
pause