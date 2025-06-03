@echo off
echo 🔄 Starten van alle services...

start "" "heartbeat_service.exe"
start "" "logging_service.exe"
start "" "stats_service.exe"
start "" "weer_time_writer.exe"


start "" "custom_dice_service.exe"
start "" "paardenrace_service.exe"

timeout /t 2 >nul
echo 🟢 Services gestart...

start "" "CasinoGUI.exe"

echo ✅ Alles gestart. Sluit dit venster niet als je de processen wil laten lopen.
pause