@echo off
setlocal enabledelayedexpansion
REM Pankti Build Helper for Windows (matches `make [command]` structure in Makefile)
set BIN=pankti
set CMAKE_BUILD_DIR=build
set ZIG_BUILD_DIR=zig-out
set FILE=.\a.pn
set PERFFILE=.\benchmarks\fib.pn
set TEST_BIN=pankti_tests
set RUNTIME_TEST_BIN=pankti_runtime_test
set RUNTIME_SAMPLES_DIR=tests\runtime\samples
set TEST_ARGS=
set RUNTIME_TEST_ARGS=
set BUILD_CONFIG=Debug

set CMAKE_OUTPUT=%CMAKE_BUILD_DIR%\%BIN%.exe
set ZIG_OUTPUT=%ZIG_BUILD_DIR%\bin\%BIN%.exe
set TEST_OUTPUT=%CMAKE_BUILD_DIR%\%TEST_BIN%.exe
set RUNTIME_TEST_OUTPUT=%CMAKE_BUILD_DIR%\%RUNTIME_TEST_BIN%.exe
set ZIG_TEST_OUTPUT=%ZIG_BUILD_DIR%\bin\%TEST_BIN%.exe

if "%1"=="" goto:run
if "%1"=="build" goto:build
if "%1"=="run" goto:run
if "%1"=="test" goto:test

if "%1"=="zbuild" goto:zbuild
if "%1"=="ztest" goto:ztest
if "%1"=="zrun" goto:zrun

if "%1"=="cmake_setup" goto:cmake_setup
if "%1"=="cmake_clang" goto:cmake_clang

if "%1"=="runtime_tests" goto:runtime_tests

if "%1"=="build_rls" goto:build_rls
if "%1"=="build_dbg" goto:build_dbg
if "%1"=="build_rld" goto:build_rld

echo Unknown command %1
echo Usage "winbuild.bat [COMMAND]"
exit /b 1




:run
call :build
if errorlevel 1 exit /b 1
echo Running file %FILE%
%CMAKE_OUTPUT% %FILE%
exit /b %errorlevel%

:build
echo Building With CMake
cmake --build %CMAKE_BUILD_DIR% --target %BIN%
if errorlevel 1 (
	echo Build failed!
	exit /b 1
)
echo Build successful!
exit /b 0

:test
call :build_rls
cmake --build build --target %TEST_BIN%
echo ==== Running Frontend Tests ====
%TEST_OUTPUT% %TEST_ARGS%
echo ==== Finished Frontend Tests ====
echo ==== Running Runtime Tests ====
set PANKTI_BIN=%CMAKE_OUTPUT%
set SAMPLES_DIR=%RUNTIME_SAMPLES_DIR%
%RUNTIME_TEST_OUTPUT% %RUNTIME_TEST_ARGS%
echo ==== Finished Runtime Tests ====
exit /b %errorlevel%

:zbuild
echo "Building with Zig"
zig build
if errorlevel 1 (
	echo Build failed!
	exit /b 1
)
echo Build successful!
exit /b 0

:zrun
call :zbuild
if errorlevel 1 exit /b 1
echo Running file %FILE%
%ZIG_OUTPUT% %FILE%
exit /b %errorlevel%

:ztest
zig build ntests -Doptimize=ReleaseFast
echo "==== Running Frontend Tests ===="
%ZIG_TEST_OUTPUT% %TEST_ARGS%
echo "==== Finished Frontend Tests ===="
echo "==== Running Runtime Tests ===="
zig build -Doptimize=ReleaseFast
set PANKTI_BIN=%ZIG_OUTPUT%
set SAMPLES_DIR=%RUNTIME_SAMPLES_DIR%
python -m unittest discover -s tests --verbose
echo "==== Finished Runtime Tests ===="
exit /b %errorlevel%

:runtime_tests
python -m unittest discover -s tests --verbose
exit /b %errorlevel%

:cmake_setup
cmake -S . -B %CMAKE_BUILD_DIR%
exit /b %errorlevel%

:cmake_clang
cmake -S . -B build -DCMAKE_C_COMPILER=clang
exit /b %errorlevel%

:build_rls
echo Building in Release Mode
if exist %CMAKE_BUILD_DIR% rmdir /s /q %CMAKE_BUILD_DIR%
cmake -S . -B %CMAKE_BUILD_DIR% -DCMAKE_BUILD_TYPE=Release
if errorlevel 1 exit /b 1
set BUILD_CONFIG=Release
cmake --build %CMAKE_BUILD_DIR% --target %BIN% --config %BUILD_CONFIG%
exit /b %errorlevel%

:build_dbg
echo Building in Debug Mode
if exist %CMAKE_BUILD_DIR% rmdir /s /q %CMAKE_BUILD_DIR%
cmake -S . -B %CMAKE_BUILD_DIR% -DCMAKE_BUILD_TYPE=Debug
if errorlevel 1 exit /b 1
set BUILD_CONFIG=Debug
cmake --build %CMAKE_BUILD_DIR% --target %BIN% --config %BUILD_CONFIG%
exit /b %errorlevel%

:build_rld
echo Building in Release Mode with Debug Symbols
if exist %CMAKE_BUILD_DIR% rmdir /s /q %CMAKE_BUILD_DIR%
cmake -S . -B %CMAKE_BUILD_DIR% -DCMAKE_BUILD_TYPE=RelWithDebInfo
if errorlevel 1 exit /b 1
set BUILD_CONFIG=Release
cmake --build %CMAKE_BUILD_DIR% --target %BIN% --config %BUILD_CONFIG%
exit /b %errorlevel%
