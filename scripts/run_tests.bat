@echo off
setlocal
set TOOL=%~dp0..\out\x64\Release\petag2.exe
set DLL=%~dp0..\out\x64\Release\petag.dll
set SAMPLE=%~dp0..\samples\signed.exe
set OUTPUT=%~dp0..\out\x64\Release\signed_tagged.exe

if not exist "%TOOL%" (
  echo Build petag2.exe first in Release x64.
  exit /b 1
)
if not exist "%DLL%" (
  echo Build petag.dll first in Release x64 and place next to petag2.exe.
  exit /b 1
)
if not exist "%SAMPLE%" (
  echo Place a signed sample at %SAMPLE%
  exit /b 1
)

echo Insert metadata via DLL
"%TOOL%" --insert "%SAMPLE%" "%OUTPUT%" "{test_chan:127}"
if errorlevel 1 (
  echo Insert failed
  exit /b 1
)

echo Read metadata via DLL
"%TOOL%" --read "%OUTPUT%"
if errorlevel 1 (
  echo Read failed
  exit /b 1
)

set SIGNSRC=
where signtool.exe >nul 2>&1
if %ERRORLEVEL% EQU 0 (
  set SIGNSRC=signtool.exe
) else (
  if exist "C:\Program Files (x86)\Windows Kits\10\App Certification Kit\signtool.exe" (
    set SIGNSRC="C:\Program Files (x86)\Windows Kits\10\App Certification Kit\signtool.exe"
  )
)

if defined SIGNSRC (
  echo Verify signature with signtool
  %SIGNSRC% verify /pa "%OUTPUT%"
  if errorlevel 1 (
    echo signtool verify failed
    exit /b 1
  )
) else (
  echo signtool.exe not found, skipping external verification
)

echo Done
exit /b 0
