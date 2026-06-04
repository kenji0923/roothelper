@echo off
setlocal
set ROOT_FILE_PATH=%~1
set WSLENV=ROOT_FILE_PATH/p
wsl bash -lc "root -l \"$ROOT_FILE_PATH\" -e \"new TBrowser\""
endlocal
