@echo off
:: esptool.py --port COM1 erase_flash
chgport /QUERY
:: C:\python27\scripts\platformio platform update
C:\python27\scripts\platformio run -d %1 --upload-port %2 -t upload

