#!/bin/sh
# This example script will sort using sort(1). YOU CANNOT USE THIS. obviously.
# This is just an example of how your dockerfile must work
echo sort -f -o $2 $1
/usr/bin/sort -f -o $2 $1
echo done