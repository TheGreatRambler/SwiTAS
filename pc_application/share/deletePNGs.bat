:: https://superuser.com/a/885150
:: Only delete png if there is a svg present
For /F "tokens=*" %%G in ('dir /b /s  *.png') do (
   if exist "%%~dpnG.svg" (
        del "%%~dpnxG"
   ) else (
        retain %%~dpnxG
   )
)