#!/bin/bash

g++ -O2 -std=c++20 -march=native -mfpu=neon -fno-tree-vectorize $1 -DBENCHMARK_STATIC_DEFINE -I/usr/local/include -L/usr/local/lib -lbenchmark -lpthread
