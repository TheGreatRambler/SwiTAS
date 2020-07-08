# Build for 32 bit and then 64 bit
# TODO

make clean
make -B BUILD=release ARCH=64

mkdir release
rm switas-windows-64bit.zip

mkdir release/bin
cp bin/* release/bin
cp mainSettings.json release
# https://unix.stackexchange.com/a/314823
find share -name '*.png' -o -name '*.jpg' | xargs cp --parents -t release

strip -s release/bin/switas.exe

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
#cp mainSettings.json release
## https://unix.stackexchange.com/a/314823
#find share -name '*.png' -o -name '*.jpg' | xargs cp --parents -t release
#
#strip -s release/bin/switas.exe
#
#zip -r switas-windows-32bit.zip release
#rm -r release