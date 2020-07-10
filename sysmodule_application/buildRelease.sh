# Build for 32 bit and then 64 bit
# DLL build
# TODO

#make clean
#make yuzu -B BUILD=release ARCH=64
#
#mkdir release-dll
#rm switas_dll-64bit.zip
#
#cp buildYuzu/* release-dll
#
#strip -s release-dll/libswitas.dll
#
#zip -r switas_dll-64bit.zip release-dll
#rm -r release-dll

# Sysmodule build

make sysmodule -B BUILD=release

mkdir -p atmosphere/contents
rm switas_sysmodule.zip

cp -R 4200000000000BAC atmosphere/contents

zip -r switas_sysmodule.zip atmosphere
rm -r atmosphere

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