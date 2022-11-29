#!/bin/bash
cmake -DCMAKE_INSTALL_PREFIX=/home/xpfan -DSGX_MODE=Debug -DCMAKE_DEBUG_POSTFIX=_debug -B ./build/debug && cmake --build ./build/debug -j && cmake --install ./build/debug

cmake -DCMAKE_INSTALL_PREFIX=/home/xpfan -DSGX_MODE=PreRelease -B ./build/release && cmake --build ./build/release -j && cmake --install ./build/release
