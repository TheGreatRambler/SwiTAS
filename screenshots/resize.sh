for g in *.gif; do
    d=${g%.gif}; gifsicle --optimize < "$g" > "$d-resized.gif";
done