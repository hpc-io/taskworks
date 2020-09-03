# /bin/bash

pexec() { echo "\$ $*" ; "$@" ; }

echo "Running $2 for $1 times"

for i in $(seq 1 $1);
do
    echo "========================== Iteration $i =========================="
    echo "$2"
    $2
    if [ $? -ne 0 ]
    then
        echo "Stop on error"
        break
    fi
done
