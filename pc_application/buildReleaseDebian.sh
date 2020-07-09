make clean
make -B BUILD=release ARCH=64 SYSTEM=debian

mkdir -p switas_debian/usr/bin
rm switas_debian/usr/bin/switas
cp bin/switas switas_debian/usr/bin
strip -s switas_debian/usr/bin/switas

cp switas_settings.json switas_debian/etc/switas
cp switas_recent.json switas_debian/etc/switas

rm -rf switas_debian/usr/share/switas
mkdir -p switas_debian/usr/share/switas
find share -name '*.png' -o -name '*.jpg' | xargs cp --parents -t switas_debian/usr/share/switas

dpkg-deb --build switas_debian switas-debian-64bit.deb