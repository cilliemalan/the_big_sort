#!/bin/bash
# This script runs the sort and records the times. Run like so:
#
#   ./run-sort.sh  <your image name>
#
# Expectations: This script expects /src and /dst to exist, docker to be installed
# and the executables in this repo to be installed.

IMAGE=$1
GIGABYTES=$2
MEMORYLIMIT=${MEMORYLIMIT:-50g}

if [[ "$GIGABYTES" -gt "150" ]]; then MEMORYLIMIT="150g"; fi
if [[ "$IMAGE" = "" ]]; then echo "No image specified"; exit 0; fi
if [[ "$GIGABYTES" = "" ]]; then echo "No gigabytes specified"; exit 0; fi

TIMEFORMAT=%E

# pull the image
docker pull $IMAGE

# remove the sorted file
rm -f /dst/file.dat

# generate the file to be sorter
rm -f /src/file.dat /dst/file.dat
echo "Generating $GIGABYTES GB file"
generate $GIGABYTES > /src/file.dat

# purge
sync && echo 3 > /proc/sys/vm/drop_caches

# the command that will be run
torun="docker run
    --network none
    --rm -it
    -m $MEMORYLIMIT
    -v /src:/src:ro
    -v /dst:/dst
    $IMAGE
    /src/file.dat
    /dst/file.dat"

echo
echo "==SORTING=="
echo
echo $torun
exectime=$({ time $torun 2>&1; } 3>&2 2>&1 1>&3)
echo
echo "===SORT DONE in $exectime seconds==="
echo

if [[ "$?" -eq "0" ]]
then
    echo "CHECKING IF THE FILE IS SORTED"
    check-sorted /dst/file.dat /src/file.dat

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

