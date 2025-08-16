#!/bin/sh

mkdir -p bin
gcc main.c renderer.c -lglfw -lGLEW -lGL -lm -o bin/main && ./bin/main
