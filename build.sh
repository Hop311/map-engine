#!/usr/bin/env bash

set +e +x

DIR="build"

RET=0
echo " === Updating submodules ==="
git submodule update --init --recursive
RET=$?
echo -e " ===========================\n"
if [ ${RET} -eq 0 ]; then
    echo " === CMake setup ==="
    cmake -S . -B "${DIR}"
    RET=$?
    echo -e " ===================\n"
fi
if [ ${RET} -eq 0 ]; then
    echo " === Building ==="
    cmake --build "${DIR}"
    RET=$?
    echo -e " ================\n"
fi
if [ ${RET} -eq 0 ]; then echo "*** BUILD SUCCESSFUL ***"
else echo "*** BUILD FAILED ***"; fi
read -n 1 -s -r -p $'\nPress any key to continue...\n'
exit ${RET}
