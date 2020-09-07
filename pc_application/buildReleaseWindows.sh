make clean
make -B BUILD=release ARCH=64

rm -rf release

mkdir -p release/bin
rm release/bin/switas.exe
cp bin/* release/bin
strip -s release/bin/switas.exe

cp switas_settings.json release

rm -rf release/share
mkdir -p release/share
find share -name '*.png' -o -name '*.jpg' -o -name '*.ico' | xargs cp --parents -t release

rm switas-windows-64bit.zip
zip -r switas-windows-64bit.zip release
rm -r release