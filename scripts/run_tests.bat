@echo off
setlocal
set TOOL=%~dp0..\out\x64\Release\pctool.exe
set SAMPLE=%~dp0..\samples\signed.exe
set OUTPUT=%~dp0..\out\x64\Release\signed_tagged.exe

if not exist "%TOOL%" (
  echo Build pctool.exe first in Release x64.
  exit /b 1
)
if not exist "%SAMPLE%" (
  echo Place a signed sample at %SAMPLE%
  exit /b 1
)

echo Insert metadata
"%TOOL%" --insert "%SAMPLE%" "%OUTPUT%" "{test_chan:123}"
if errorlevel 1 (
  echo Insert failed
  exit /b 1
)

echo Verify signature via WinVerifyTrust (internal)
"%TOOL%" --read "%OUTPUT%"
if errorlevel 1 (
  echo Read failed
  exit /b 1
)

echo Optionally verify with signtool (if installed)
where signtool.exe >nul 2>&1 && (
  signtool.exe verify /pa "%OUTPUT%"
)

echo Done
exit /b 0

