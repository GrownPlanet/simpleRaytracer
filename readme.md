# Simple raytracer
A very simple raytracer written in c.

## How to run
You need to have `GLEW` and `glfw` installed on your system. Then you can run
the project with `run.sh` or
```
mkdir -p bin
gcc main.c renderer.c -lglfw -lGLEW -lGL -lm -o bin/main && ./bin/main
```

If you are on windows, may the lord have mercy on your soul.
