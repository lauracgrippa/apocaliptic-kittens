@echo off
REM Compila o projeto (main e apocaliptic_preview) com o g++ do MSYS2 MinGW64.
REM Uso:
REM   compilar.bat            compila em Debug
REM   compilar.bat release    compila em Release
REM   compilar.bat run        compila em Debug e executa o apocaliptic_preview
setlocal

set "MINGW_BIN=C:\msys64\mingw64\bin"
set "BUILD_TYPE=Debug"
set "RUN="
if /i "%~1"=="release" set "BUILD_TYPE=Release"
if /i "%~1"=="run" set "RUN=1"

if not exist "%MINGW_BIN%\g++.exe" (
  echo [ERRO] g++ nao encontrado em %MINGW_BIN%.
  echo Instale o MSYS2 e rode no terminal MINGW64: pacman -S mingw-w64-x86_64-gcc
  pause & exit /b 1
)
set "PATH=%MINGW_BIN%;%PATH%"

where cmake >nul 2>&1 || (
  echo [ERRO] CMake nao encontrado no PATH.
  pause & exit /b 1
)

cd /d "%~dp0"
set "BUILD_DIR=build-mingw-%BUILD_TYPE%"

cmake -S . -B "%BUILD_DIR%" -G "MinGW Makefiles" -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ -DCMAKE_BUILD_TYPE=%BUILD_TYPE% || (
  echo [ERRO] Falha ao configurar o CMake.
  pause & exit /b 1
)

cmake --build "%BUILD_DIR%" -j %NUMBER_OF_PROCESSORS% || (
  echo [ERRO] Falha na compilacao.
  pause & exit /b 1
)

echo.
echo Compilado com sucesso: bin\%BUILD_TYPE%\main.exe e bin\%BUILD_TYPE%\apocaliptic_preview.exe

if defined RUN (
  cd /d "%~dp0bin\%BUILD_TYPE%"
  apocaliptic_preview.exe
)

endlocal
