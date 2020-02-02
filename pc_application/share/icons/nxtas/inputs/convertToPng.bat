@echo off
:: Have to use ugly purple for background
:: Full solid color for background
:: Just the height
for %%f in (*) do if not "%%~f" == inputicons.svg (
    echo %%~f
    "C:\Program Files\Inkscape\inkscape.exe" ^
      -z ^
	  --export-background="rgb(48,0,77)" ^
      --export-background-opacity=255 ^
      --export-height=100 ^
      --export-png="%%~dpnf.png" ^
      --file="%%~f"
)