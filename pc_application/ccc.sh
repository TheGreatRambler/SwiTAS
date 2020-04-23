compiledb -n make
if [[ "$OSTYPE" == "msys" ]]; then
    # Replace mingw paths with windows paths
    sed -i 's|-I/|-IC:/msys2/|g' compile_commands.json
fi