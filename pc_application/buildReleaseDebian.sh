# Build for 32 bit and then 64 bit
# TODO

make clean
# For both debian and ubuntu
make -B BUILD=release ARCH=64 SYSTEM=debian

strip -s bin/switas

rm switas_debian/usr/bin/switas
mkdir -p switas_debian/usr/bin
cp bin/switas switas_debian/usr/bin

rm -rf switas_debian/usr/share/switas
mkdir -p switas_debian/usr/share/switas
find share -name '*.png' -o -name '*.jpg' | xargs cp --parents -t switas_debian/usr/share/switas

rm -rf switas_debian/etc/switas
mkdir -p switas_debian/etc/switas
cp switas_settings.json switas_debian/etc/switas
cp switas_recent.json switas_debian/etc/switas

dpkg-deb --build switas_debian
mv switas_debian.deb switas-debian-64bit.deb