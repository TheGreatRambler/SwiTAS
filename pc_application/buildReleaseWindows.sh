# Build for 32 bit and then 64 bit
# TODO

make -B BUILD=release ARCH=64

mkdir release
rm nxtaspc.zip

mkdir release/bin
cp bin/* release/bin
cp mainSettings.json release
# https://unix.stackexchange.com/a/314823
find share -name '*.png' -o -name '*.jpg' | xargs cp --parents -t release

strip -s release/bin/nxtas.exe

zip -r nxtaspc-windows-64bit.zip release
rm -r release

make -B BUILD=release ARCH=32

mkdir release
rm nxtaspc.zip

mkdir release/bin
cp bin/* release/bin
cp mainSettings.json release
# https://unix.stackexchange.com/a/314823
find share -name '*.png' -o -name '*.jpg' | xargs cp --parents -t release

strip -s release/bin/nxtas.exe

zip -r nxtaspc-windows-64bit.zip release
rm -r release