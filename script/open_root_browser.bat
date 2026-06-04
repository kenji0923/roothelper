@echo off
:: Launch ROOT in WSL login shell, convert Windows path to WSL path, and open TBrowser
wsl bash -lc "root -l \"$(wslpath -u \"%~1\")\" -e \"new TBrowser\""
