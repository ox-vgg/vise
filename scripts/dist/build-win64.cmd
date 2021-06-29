::
:: Script to build Windows releases of VGG Image Search Engine (VISE)
:: Author: Abhishek Dutta <adutta@robots.ox.ac.uk>
:: 02 September 2020
::

@ECHO ON
cls

echo Running on %COMPUTERNAME%...
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"
SET CURRENT_PATH=%cd%

SET BUILD_BASE_FOLDER="C:\Users\tlm\build\vise\release"
SET CMAKE_EXEC="C:\Program Files\CMake\bin\cmake.exe"
SET CMAKE_GENERATOR_NAME="Visual Studio 16 2019"
SET VISE_SOURCE_PATH="C:\Users\tlm\code\vise\src"

::
:: Build release for Windows 64 bit
::
SET BUILD_BASE_FOLDER_WIN64=%BUILD_BASE_FOLDER%\Win64
IF EXIST %BUILD_BASE_FOLDER_WIN64% (
  cd /d %VISE_SOURCE_PATH%
  mkdir %BUILD_BASE_FOLDER_WIN64%
  cd /d %BUILD_BASE_FOLDER_WIN64%
  %CMAKE_EXEC% ^
   -G %CMAKE_GENERATOR_NAME% ^
   -DCMAKE_BUILD_TYPE=Release ^
   -DCMAKE_GENERATOR_PLATFORM=x64 ^
   -DCMAKE_TOOLCHAIN_FILE="C:\Users\tlm\dep\vise\vcpkg\scripts\buildsystems\vcpkg.cmake" ^
   %VISE_SOURCE_PATH%
   msbuild ALL_BUILD.vcxproj -maxcpucount:8 -p:PreferredToolArchitecture=x64 -nologo -consoleLoggerParameters:ErrorsOnly;Summary -verbosity:minimal -interactive:False -lowPriority:False /p:Configuration=Release
   msbuild PACKAGE.vcxproj /maxcpucount:8 -p:PreferredToolArchitecture=x64 -nologo -consoleLoggerParameters:ErrorsOnly;Summary -verbosity:minimal -interactive:False -lowPriority:False /p:Configuration=Release
)

cd %CURRENT_PATH%
