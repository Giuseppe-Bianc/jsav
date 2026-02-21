#!/bin/bash
readonly JSAV_ROOT="$PWD"

readonly RUN_DIR="$JSAV_ROOT/build/src/jsav/"
clear
cmake -S . -B ./build -Wno-dev -GNinja -Djsav_WARNINGS_AS_ERRORS=ON -Djsav_ENABLE_CLANG_TIDY:BOOL=ON -Djsav_ENABLE_IPO:BOOL=OFF
cmake --build ./build -j 3
if [ -d "$RUN_DIR" ]; then
    cd  "$RUN_DIR"
    echo "Current working directory: $(pwd)"
    ./jsav
    echo "complete."
else
    echo "Directory $RUN_DIR does not exist."
fi