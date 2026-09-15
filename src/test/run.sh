#!/bin/bash

g++ -O1 -march=native -mbmi2 -fno-tree-vectorize $1 -DBENCHMARK_STATIC_DEFINE -I/usr/local/include -L/usr/local/lib -lbenchmark -lshlwapi -lpthread
