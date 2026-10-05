@echo off
title FACTIONS BATTLES - Parar servidor
echo Parando los servicios del servidor...
echo.

rem El servidor de juego (UDP 7777) y el mini-master (TCP 3001) son
rem procesos propios, se paran por el puerto que tienen escuchado.
rem El puerto 3000 lo suelta solo cuando muere el proceso del 7777.

powershell -NoProfile -ExecutionPolicy Bypass -Command ^
 "$ids=@(); $ids+=(Get-NetUDPEndpoint -LocalPort 7777 -ErrorAction SilentlyContinue).OwningProcess; $ids+=(Get-NetTCPConnection -LocalPort 3001 -State Listen -ErrorAction SilentlyContinue).OwningProcess; $ids=@($ids | Where-Object { $_ } | Sort-Object -Unique); if(-not $ids){ Write-Host 'No hay servicios del servidor corriendo.' } else { foreach($i in $ids){ Stop-Process -Id $i -Force -ErrorAction SilentlyContinue; Write-Host ('Detenido PID ' + $i) } }"

echo.
echo Listo.
ping -n 4 127.0.0.1 >nul
