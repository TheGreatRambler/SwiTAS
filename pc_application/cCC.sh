compiledb -n make
# Replace mingw paths with windows paths
sed -i 's|-I/|-IC:/msys2/|g' compile_commands.json
# Have to separate ALL -I from the rest of the text, vscode bug
# sed -i 's|-I|-I","|g' compile_commands.json
# Move the file
# mv compile_commands.json ../compile_commands.json