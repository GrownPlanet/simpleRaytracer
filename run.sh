#!/bin/sh

mkdir -p bin
gcc main.c -lglfw -lGLEW -lGL -lm -o bin/main && ./bin/main
