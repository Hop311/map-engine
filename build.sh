#!/usr/bin/env bash

set -e

echo " === Updating submodules ==="
git submodule update --init --recursive
echo -e " ===========================\n"

echo " === CMake setup ==="
cmake -S . -B build
cd build
echo -e " ===================\n"

echo " === Building ==="
cmake --build .
echo -e " ================\n"

echo "Press any key to continue..."
read -n 1 -s -r
