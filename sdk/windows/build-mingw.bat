@echo off
setlocal
if "%CURL_ROOT%"=="" (
  echo Set CURL_ROOT to the matching Windows libcurl development package.
  exit /b 2
)
if "%CC%"=="" set CC=gcc
if not exist "%CURL_ROOT%\include\curl\curl.h" (
  echo Missing %CURL_ROOT%\include\curl\curl.h
  exit /b 2
)
if not exist "%CURL_ROOT%\lib\libcurl.dll.a" if not exist "%CURL_ROOT%\lib\libcurl.a" (
  echo Missing libcurl import library under %CURL_ROOT%\lib
  exit /b 2
)
if not exist bin mkdir bin
%CC% -std=c11 -O2 -shared -DQQBOT_SDK_BUILD -Isdk\include -I"%CURL_ROOT%\include" sdk\src\qqbot_sdk.c -L"%CURL_ROOT%\lib" -lcurl -Wl,--kill-at -o bin\qqbot_sdk.dll
if errorlevel 1 exit /b 1
echo Built bin\qqbot_sdk.dll. Ensure the matching libcurl runtime DLL is beside the plugin or on PATH.
echo The DLL architecture must match the 易语言 host process (x86 with x86, x64 with x64).
