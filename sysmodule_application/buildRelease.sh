# Build for 32 bit and then 64 bit
# TODO

make clean
make yuzu -B BUILD=release ARCH=64

mkdir release-dll
rm switasdll-64bit.zip

cp buildYuzu/* release-dll

strip -s release-dll/libswitas.dll

zip -r switasdll-64bit.zip release-dll
rm -r release-dll

# make clean
# make yuzu -B BUILD=release ARCH=32
#
# mkdir release-dll
# rm switasdll-32bit.zip
#
# cp buildYuzu/* release-dll
#
# strip -s release-dll/libswitas.dll
#
# zip -r switasdll-32bit.zip release-dll
# rm -r release-dll