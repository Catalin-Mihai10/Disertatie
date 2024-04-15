#!/bin/bash

clang -o3 -Wall -Wextra -pedantic -Wno-uninitialized -Wno-unused-parameter -Wno-deprecated-copy-with-user-provided-copy -Wno-gnu-zero-variadic-macro-arguments -Wno-writable-strings -Wno-missing-field-initializers -o network "source/network.c" "source/tensor.c" "utils/logger/NNLogger.c" "utils/parser/parser.c" -lm -lpcap
