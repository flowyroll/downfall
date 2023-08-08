key=$(hexdump -vn32 -e'8/4 "%08X" 1 "\n"' /dev/urandom)
echo Key: ${key:0:16}:${key:16:16}:${key:32:16}:${key:48:16}
echo "Downfall" > /tmp/downfall.txt
openssl aes-256-cbc -salt -e -in /tmp/downfall.txt -out /tmp/downfall.txt.encrypted -K $key -iv 11111111111111111111111111111111
timeout 20s taskset -c 1 openssl aes-256-cbc -salt -e -in /dev/urandom -out /dev/null -K $key -iv 11111111111111111111111111111111
