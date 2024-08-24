#!/bin/bash

g++ main.cpp field.cpp game.cpp -lncurses -pedantic -fsanitize=address

if [ $? -eq 0 ]; then
    mv a.out sea_battle
    chmod +x sea_battle
    sudo mv sea_battle /usr/local/bin/
    sea_battle
else
    echo "Compilation failed."
    exit 1
fi

