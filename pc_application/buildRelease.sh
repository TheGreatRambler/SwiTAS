make -B BUILD=release

mkdir release
rm nxtaspc.zip

mkdir release/bin
cp bin/* release/bin
cp mainSettings.json release
# https://unix.stackexchange.com/a/314823
find share -name '*.png' -o -name '*.jpg' | xargs cp --parents -t release

strip -s release/bin/nxtas.exe

zip -r nxtaspc.zip release
rm -r release