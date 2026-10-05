@echo off
setlocal enabledelayedexpansion
title FACTIONS BATTLES - Servidor local
cd /d "%~dp0"

echo ==============================================================
echo    FACTIONS BATTLES  -  arranque del servidor en tu PC
echo ==============================================================
echo.

rem ------------------------------------------------------------------
rem 1) Node.js instalado?
rem ------------------------------------------------------------------
where node >nul 2>nul
if errorlevel 1 (
    echo  [ERROR] No se encuentra "node" en el PATH.
    echo          Instala Node.js LTS desde https://nodejs.org y vuelve
    echo          a ejecutar este archivo.
    echo.
    pause
    exit /b 1
)
set "NODEV="
for /f "delims=" %%v in ('node -v') do set "NODEV=%%v"
echo  [OK] Node.js !NODEV!

rem ------------------------------------------------------------------
rem 2) Archivos del servidor
rem ------------------------------------------------------------------
set "SRV=%~dp0build\dist\server"
set "MM=C:\Users\Sas\Documents\Qoder\2026-09-30\2c5f85ea\mini-master"

if not exist "%SRV%\dist_back\skymp5-server.js" (
    echo  [ERROR] Falta el servidor compilado:
    echo          %SRV%\dist_back\skymp5-server.js
    echo          Compilalo con:  cmake --build .   [dentro de build]
    echo.
    pause
    exit /b 1
)
if not exist "%SRV%\server-settings.json" (
    echo  [ERROR] Falta server-settings.json en %SRV%
    echo.
    pause
    exit /b 1
)
if not exist "%SRV%\gamemode.js" (
    echo  [ERROR] Falta gamemode.js en %SRV%
    echo.
    pause
    exit /b 1
)
if not exist "%MM%\server.js" (
    echo  [AVISO] No encuentro mini-master en:
    echo          %MM%
    echo          El servidor de juego arrancara, pero el login de
    echo          Discord y los personajes NO funcionaran.
    echo.
)
echo  [OK] Archivos del servidor correctos
echo.

rem ------------------------------------------------------------------
rem 3) Puertos
rem ------------------------------------------------------------------
echo  Comprobando puertos...

rem --- UDP 7777: servidor de juego ---
set "BUSY7777="
for /f "tokens=4" %%P in ('netstat -ano -p UDP ^| findstr /C:":7777 "') do set "BUSY7777=%%P"
if defined BUSY7777 (
    echo  [AVISO] El puerto UDP 7777 ya esta en uso ^(PID !BUSY7777!^).
    echo          El servidor ya parece estar corriendo.
    echo          Si quieres reiniciar, ejecuta antes PARAR-SERVER.bat
    echo.
    pause
    exit /b 1
)

rem --- TCP 3000: UI del servidor SkyMP ---
set "BUSY3000="
for /f "tokens=5" %%P in ('netstat -ano -p TCP ^| findstr /C:":3000 " ^| findstr /C:"LISTENING"') do set "BUSY3000=%%P"
if defined BUSY3000 (
    set "NAME3000="
    for /f "tokens=1 delims=," %%N in ('tasklist /FI "PID eq !BUSY3000!" /FO CSV /NH 2^>nul') do set "NAME3000=%%N"
    set "NAME3000=!NAME3000:"=!"
    echo  [ERROR] El puerto 3000 esta ocupado por: !NAME3000! ^(PID !BUSY3000!^)
    echo.
    echo          SkyMP usa el 3000 para la UI y SI o SI necesita ese puerto.
    echo          - Si es tu web con "npm run dev": cierrala, o ya esta
    echo            configurada en el puerto 3002, asi que no deberia pasar.
    echo          - Si es otro programa, cambiale el puerto o cerralo.
    echo.
    pause
    exit /b 1
)
echo  [OK] Puerto 3000 libre ^(UI^)

rem --- TCP 3001: mini-master ---
set "BUSY3001="
for /f "tokens=5" %%P in ('netstat -ano -p TCP ^| findstr /C:":3001 " ^| findstr /C:"LISTENING"') do set "BUSY3001=%%P"

rem ------------------------------------------------------------------
rem 4) IP de esta PC (para que se conecten tus amigos)
rem ------------------------------------------------------------------
set "LANIP="
for /f %%I in ('powershell -NoProfile -Command "(Get-NetIPAddress -AddressFamily IPv4 -ErrorAction SilentlyContinue | Where-Object {$_.IPAddress -like '192.168.*'} | Select-Object -First 1).IPAddress"') do set "LANIP=%%I"

echo.
echo --------------------------------------------------------------
if defined LANIP (
    echo  Para jugar TU:            launcher -^> 127.0.0.1:7777
    echo  Para amigos en tu red:    launcher -^> !LANIP!:7777
) else (
    echo  Para jugar TU:            launcher -^> 127.0.0.1:7777
)
echo --------------------------------------------------------------
echo.

rem ------------------------------------------------------------------
rem 5) Arrancar mini-master (si no esta ya corriendo)
rem ------------------------------------------------------------------
if defined BUSY3001 (
    echo  [OK] mini-master ya esta corriendo en el 3001, no lo relanzo.
    echo.
    goto arrancar_server
)

if not exist "%MM%\start.bat" goto arrancar_server

echo  [1/2] Arrancando mini-master ^(login Discord, puerto 3001^)...
start "mini-master :3001" /MIN cmd /k ""%MM%\start.bat""

set /a T=0
:check3001
for /f "tokens=5" %%P in ('netstat -ano -p TCP ^| findstr /C:":3001 " ^| findstr /C:"LISTENING"') do set "BUSY3001=%%P"
if defined BUSY3001 goto ok3001
set /a T+=1
if !T! geq 15 goto aviso3001
ping -n 2 127.0.0.1 >nul
goto check3001

:aviso3001
echo  [AVISO] mini-master no ha respondido en 15 segundos.
echo          Mira su ventana por si ha dado algun error.
echo.
goto arrancar_server

:ok3001
echo  [OK] mini-master escuchando en el puerto 3001
echo.

rem ------------------------------------------------------------------
rem 6) Arrancar el servidor de juego (en esta ventana, para ver los logs)
rem ------------------------------------------------------------------
:arrancar_server
echo  [2/2] Arrancando servidor SkyMP ^(UDP 7777 + UI 3000^)...
echo.
echo ==============================================================
echo   Cierra el servidor con Ctrl+C en esta ventana.
echo   Para parar todo: PARAR-SERVER.bat
echo ==============================================================
echo.

cd /d "%SRV%"
node dist_back\skymp5-server.js
set "RC=!ERRORLEVEL!"

echo.
echo ==============================================================
if "!RC!"=="0" (
    echo   El servidor se ha detenido.
) else (
    echo   El servidor se ha detenido con error !RC!
)
echo   Relanzalo con INICIAR-SERVER.bat
echo ==============================================================
pause
exit /b 0
