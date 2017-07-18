@taskkill /f /im ServiceApp.exe
@sc stop ServiceApp
@ping 127.0.0.1 -n 3 >nul
@sc delete ServiceApp

