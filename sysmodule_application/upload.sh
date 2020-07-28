# scp -r 0100000000000BAC 192.168.1.26:5000/atmosphere/contents/0100000000000BAC

lftp -c "open 192.168.1.165:5000; mirror -Rn switch /"