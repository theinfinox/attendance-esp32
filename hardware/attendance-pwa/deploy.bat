@echo off
C:\msys64\usr\bin\bash.exe -lc "rsync -acvz --delete '/c/Users/1sree/Documents/attendance-esp32/hardware/attendance-pwa/' theinfinox@192.168.29.134:/home/theinfinox/attendance-pwa/"
echo Exit Code: %ERRORLEVEL%
pause
