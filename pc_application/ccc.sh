# Match the network code in this source tree to do intellisense
rm -r -f ./source/sharedNetworkCode
cp -p -r ../sharedNetworkCode ./source/sharedNetworkCode

compiledb -n make
if [[ "$OSTYPE" == "msys" ]]; then
    # Replace mingw paths with windows paths
    sed -i 's|-I/|-IC:/msys2/|g' compile_commands.json
fi