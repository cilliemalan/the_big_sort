#!/bin/bash
# This script runs the sort and records the times. Run like so:
#
#   ./run-sort.sh  <your image name>

IMAGE=$1
GIGABYTES=$2

if [[ "$IMAGE" = "" ]]; then echo "No image specified"; exit 0; fi
if [[ "$GIGABYTES" = "" ]]; then echo "No gigabytes specified"; exit 0; fi

TIMEFORMAT=%E

# pull the image
docker pull $IMAGE

# remove the sorted file
rm -f /dst/file.dat


torun="docker run
    --network none
    --rm -it
    -v /src:/src:ro
    -v /dst:/dst
    $IMAGE
    /src/file.dat
    /dst/file.dat"

rm -f /src/file.dat /dst/file.dat
echo "Generating $GIGABYTES GB file"
generate $GIGABYTES > /src/file.dat

echo
echo "==SORTING=="
echo
echo $torun
exectime=$({ time $torun 2>&1; } 3>&2 2>&1 1>&3)
echo
echo "===SORT DONE==="
echo

if [[ "$?" -eq "0" ]]
then
    echo "CHECKING IF THE FILE IS SORTED"
    ./check-sorted.sh /dst/file.dat

    if [[ "$?" -eq "0" ]]
    then
        echo "THE FILE IS SORTED"
        echo
        echo "RESULT for $IMAGE -> $exectime to sort $GIGABYTES GB" | tee -a /results.txt
        echo
        exit 0
    else
        echo "!!!!THE FILE IS NOT SORTED!!!!"
        exit 1
    fi

else
    echo "!!!!SORT FAILED!!!!"
fi
