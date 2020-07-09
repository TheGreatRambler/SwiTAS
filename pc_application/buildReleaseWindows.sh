make clean
make -B BUILD=release ARCH=64

rm -rf release

mkdir -p release/bin
rm release/bin/switas.exe
cp bin/* release/bin
strip -s release/bin/switas.exe

cp switas_settings.json release
cp switas_recent.json release

rm -rf release/share
mkdir -p release/share
find share -name '*.png' -o -name '*.jpg' | xargs cp --parents -t release

rm switas-windows-64bit.zip
zip -r switas-windows-64bit.zip release
rm -r release

#make clean
#make -B BUILD=release ARCH=32
#
#mkdir release
#rm switas-windows-32bit.zip
#
#mkdir release/bin
#cp bin/* release/bin
#cp switas_settings.json release
#cp switas_recent.json release
## https://unix.stackexchange.com/a/314823
#find share -name '*.png' -o -name '*.jpg' | xargs cp --parents -t release
#
#strip -s release/bin/switas.exe
#
#zip -r switas-windows-32bit.zip release
#rm -r release