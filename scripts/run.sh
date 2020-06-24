#!/bin/bash

if [ "$1" = "release" ]
then
    ./bin/release/raycast 
else
    ./bin/debug/raycast 
fi