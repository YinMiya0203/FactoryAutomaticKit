set DEVENV="E:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\devenv.com"
%DEVENV% FactoryAutoTest.sln /Rebuild "Release_console|x64" /project FactoryAutoTest
%DEVENV% FactoryAutoTest.sln /Rebuild "Release|x64" /project FactoryAutoTest
%DEVENV% FactoryAutoTest.sln /Rebuild "Release" /project SetupFactoryAutoTest
pause