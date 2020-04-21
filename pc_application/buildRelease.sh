make -B BUILD=release

mkdir release
rm nxtaspc.zip

mkdir release/bin
cp bin/* release/bin
cp mainSettings.json release
cp -r share release/share

strip -s release/bin/nxtas.exe

zip -r nxtaspc.zip release
rm -r release