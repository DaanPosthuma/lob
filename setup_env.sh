#!/bin/sh
source .venv/bin/activate
pip install -r requirements.txt
conan profile detect --force
conan config install ./.conan
conan install ./ -pr:h .conan2/profiles/clang/18/x64-libc++-release -pr:b .conan2/profiles/clang/18/x64-libc++-release --build missing -c tools.cmake.cmaketoolchain:generator="Ninja Multi-Config"
conan install ./ -pr:h .conan2/profiles/clang/18/x64-libc++-debug -pr:b .conan2/profiles/clang/18/x64-libc++-debug --build missing -c tools.cmake.cmaketoolchain:generator="Ninja Multi-Config"
conan install ./ -pr:h .conan2/profiles/clang/18/x64-libc++-rwdi -pr:b .conan2/profiles/clang/18/x64-libc++-rwdi --build missing -c tools.cmake.cmaketoolchain:generator="Ninja Multi-Config"
source ./build/generators/conanbuild.sh
cmake --preset conan-default

