for i in in/00**.txt; do
    ./a.out < $i > out.txt
    echo $i
done