!#/bin/bash

clang -o3 -Wall -Wextra -pedantic -Wno-unused-parameter -o network "source/network.cpp" "source/tensor.cpp" "utils/logger/NNLogger.cpp" -lm
