#!/bin/bash

sh scripts/build.sh release 

rm -rf -d raycast

mkdir raycast

cp bin/release/raycast raycast
cp -r res raycast

echo "Deploy build created."
echo "cd raycast to find it"