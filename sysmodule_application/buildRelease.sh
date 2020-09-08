# Build for 32 bit and then 64 bit
# DLL build
# TODO

#make clean
#make yuzu -B BUILD=release ARCH=64
#
#mkdir release-dll
#rm switas_dll-64bit.zip
#
#cp build_yuzu/* release-dll
#
#strip -s release-dll/libswitas.dll
#
#zip -r switas_dll-64bit.zip release-dll
#rm -r release-dll

# Sysmodule build

make sysmodule -B BUILD=release

cd switch
zip -r ../switas_sysmodule.zip *
cd ../

rm -r switch