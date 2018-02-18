#!/bin/sh
#
# Builds and installs the latest clang-format.
#
# It needs to write to the current directory.
#
# Source: https://clang.llvm.org/get_started.html

git clone --depth 1 https://git.llvm.org/git/llvm.git &&
    cd llvm/tools &&
    git clone --depth 1 https://git.llvm.org/git/clang.git &&
    cd .. &&
    mkdir -p build &&
    cd build &&

    cmake -D CMAKE_BUILD_TYPE=Release .. &&
    make clang-format  &&
    sudo make install-clang-format
