#!/bin/bash

LOCALE=C /usr/bin/sort -c -f --stable $1

if [ "$?" -eq "0" ]
then
    echo "SORTED"
    exit 0
else
    echo "NOT SORTED"
    exit 1
fi
