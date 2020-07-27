::
:: Script to build Windows releases
:: Author: Abhishek Dutta <adutta@robots.ox.ac.uk>
:: March 01, 2018
::

@ECHO ON
cls

SET CURRENT_PATH=%cd%

SET BUILD_BASE_FOLDER="C:\Users\tlm\build\vise\release"
SET CMAKE_EXEC="C:\Program Files\CMake\bin\cmake.exe"
SET CMAKE_GENERATOR_NAME="Visual Studio 16 2019"
SET IMCOMP_SOURCE_PATH="C:\Users\tlm\code\vise2\src"

::
:: Build release for Windows 64 bit
::
SET BUILD_BASE_FOLDER_WIN64=%BUILD_BASE_FOLDER%\Win64
IF EXIST %BUILD_BASE_FOLDER_WIN64% (
  mkdir %BUILD_BASE_FOLDER_WIN64%
  cd /d %BUILD_BASE_FOLDER_WIN64%
  %CMAKE_EXEC% ^
   -G %CMAKE_GENERATOR_NAME% ^
   -DCMAKE_BUILD_TYPE=Release ^
   -DCMAKE_GENERATOR_PLATFORM=x64 ^
   -DCMAKE_TOOLCHAIN_FILE="C:\Users\tlm\dep\vise\vcpkg\scripts\buildsystems\vcpkg.cmake" ^
   %IMCOMP_SOURCE_PATH%
  msbuild PACKAGE.vcxproj /maxcpucount:8 -p:PreferredToolArchitecture=x64 /nologo /p:configuration=Release
)

cd %CURRENT_PATH%
