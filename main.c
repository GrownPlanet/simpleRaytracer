#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

char *fragmentShaderSrc;
const char *vertexShaderSrc =
"#version 330\n"
"layout(location=0) in vec2 position;"
"void main() { gl_Position = vec4(position, 0.0, 1.0); }";

static GLFWwindow *initWindow(int windowWidth, int windowHeight, const char *name);
static GLuint compileShader(GLenum type, const char *src);
static GLuint createProgram();
static int readFragmentShader(const char *filename);
static void deinit(GLFWwindow *win);
static void loadShader();

static void move(const float *obj, const float *camera, float *dest, int i);
static void rotate(const float *obj, const float *camera, float *dest, int i);
static void rotateMovement(float *x, float *z, const float *camera);

#define isKeyDown(win, scancode) (int)(glfwGetKey(win, scancode) == GLFW_PRESS)

#define CIRCLE_COUNT 7

/*
 * 
 * y
 * ^
 * |     _> z
 * |   _/
 * | _/
 * |/
 * +------------> x
 *
 * yaw: rotation around y
 * pitch: rotation around z
 *
 */
#define X 0
#define Y 1
#define Z 2

#define YAW 3
#define PITCH 4

int main() {
    GLFWwindow *win = initWindow(800, 600, "Simple shader");
    if (win == NULL) return 1;

    GLuint prog = createProgram();
    glUseProgram(prog);

    loadShader();

    GLint resolutionLoc = glGetUniformLocation(prog, "resolution");
    if (resolutionLoc == -1) { printf("'resolution' not found\n"); return 1; }
    GLint focalLengthLoc = glGetUniformLocation(prog, "focalLength");
    if (focalLengthLoc == -1) { printf("'focalLength' not found\n"); return 1; }

    GLint positionsLoc = glGetUniformLocation(prog, "positions");
    if (positionsLoc == -1) { printf("'positions' not found\n"); return 1; }
    GLint radiiLoc = glGetUniformLocation(prog, "radii");
    if (radiiLoc == -1) { printf("'radii' not found\n"); return 1; }
    GLint circleCountLoc = glGetUniformLocation(prog, "circleCount");
    if (circleCountLoc == -1) { printf("'circleCount' not found\n"); return 1; }
    GLint colorsLoc = glGetUniformLocation(prog, "colors");
    if (colorsLoc == -1) { printf("'colors' not found\n"); return 1; }

    const float fov = 50.0;
    const float focalLength = 0.5 / tan((fov * (M_PI / 180.0)) / 2.0);
    glUniform1f(focalLengthLoc, focalLength);

    float camera[6] = {
        0.0, 0.0, -5.0, // x, y, z
        0.0, 0.0,      // yaw, pitch
    };

    // const float positions[CIRCLE_COUNT * 3] = {
    //     0.0, 0.0, 2.0,
    //     2.0, 1.0, -1.0,
    // };
    // const float colors[CIRCLE_COUNT * 3] = {
    //     0.0, 0.5, 1.0,
    //     1.0, 0.0, 0.0,
    // };
    // const float radii[CIRCLE_COUNT] = { 0.5, 0.2 };

    const float positions[CIRCLE_COUNT * 3] = {
        0.0, -50.0, 0.0, 
        -1.5, -0.5,  2.0,
        1.8,  1.2, -1.5,
        0.0,  0.0,  0.0,
        -2.5,  2.0, -0.8,
        2.2, -1.5,  1.0,
        -0.5,  1.0,  1.8
    };

    const float colors[CIRCLE_COUNT * 3] = {
        0.1, 0.1, 0.1,
        1.0, 0.2, 0.2,
        0.2, 1.0, 0.4,
        0.2, 0.5, 1.0,
        1.0, 0.8, 0.2,
        0.8, 0.2, 1.0,
        0.5, 0.5, 0.5
    };

    const float radii[CIRCLE_COUNT] = {
        45.0, 0.5, 0.3, 0.8, 0.4, 0.6, 0.25
    };


    glUniform1fv(radiiLoc, CIRCLE_COUNT, radii);
    glUniform1i(circleCountLoc, CIRCLE_COUNT);
    glUniform3fv(colorsLoc, CIRCLE_COUNT, colors);

    double lastFrameTime = 0.0;
    double deltaTime = 0.0;

    const float speed = 10.0;
    const float turnSpeed = 5.0;

    while (!glfwWindowShouldClose(win)) {
        double now = glfwGetTime();
        deltaTime = now - lastFrameTime;
        lastFrameTime = now;

        glfwPollEvents();

        // movement
        float newX = (float)(isKeyDown(win, GLFW_KEY_D) - isKeyDown(win, GLFW_KEY_A)) * speed;
        float newZ = (float)(isKeyDown(win, GLFW_KEY_W) - isKeyDown(win, GLFW_KEY_S)) * speed;
        rotateMovement(&newX, &newZ, camera);
        camera[X] += newX * deltaTime;
        camera[Z] += newZ * deltaTime;

        if (isKeyDown(win, GLFW_KEY_E)) camera[Y] += speed * deltaTime;
        if (isKeyDown(win, GLFW_KEY_Q)) camera[Y] -= speed * deltaTime;

        if (isKeyDown(win, GLFW_KEY_RIGHT)) camera[YAW] += turnSpeed * deltaTime;
        if (isKeyDown(win, GLFW_KEY_LEFT))  camera[YAW] -= turnSpeed * deltaTime;
        if (camera[YAW] > 2 * M_PI) camera[YAW] = 0.0;
        if (camera[YAW] < 0) camera[YAW] = 2 * M_PI;

        if (isKeyDown(win, GLFW_KEY_UP))   camera[PITCH] += turnSpeed * deltaTime;
        if (isKeyDown(win, GLFW_KEY_DOWN)) camera[PITCH] -= turnSpeed * deltaTime;
        if (camera[PITCH] > 2 * M_PI) camera[PITCH] = 0.0;
        if (camera[PITCH] < 0) camera[PITCH] = 2 * M_PI;

        // update
        float updated[CIRCLE_COUNT * 3];
        for (int i = 0; i < CIRCLE_COUNT; i++) {
            move(positions, camera, updated, i);
            rotate(updated, camera, updated, i);
        }
        glUniform3fv(positionsLoc, CIRCLE_COUNT, updated);

        int w, h;
        glfwGetFramebufferSize(win, &w, &h);
        glViewport(0, 0, w, h);
        glUniform2f(resolutionLoc, (float)w, (float)h);

        // render
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glfwSwapBuffers(win);
    }

    deinit(win);
}

static void move(const float *obj, const float *camera, float *dest, int i) {
    dest[i*3 + X] = obj[i*3 + X] - camera[X];
    dest[i*3 + Y] = obj[i*3 + Y] - camera[Y];
    dest[i*3 + Z] = obj[i*3 + Z] - camera[Z];
}

static void rotateMovement(float *x, float *z, const float *camera) {
    const float cosYaw = cos(camera[YAW]);
    const float sinYaw = sin(camera[YAW]);

    const float newZ = *z * cosYaw - *x * sinYaw;
    const float newX = *x * cosYaw + *z * sinYaw;

    *x = newX;
    *z = newZ;
}

static void rotate(const float *obj, const float *camera, float *dest, int i) {
    float x = obj[i*3 + X];
    float y = obj[i*3 + Y];
    float z = obj[i*3 + Z];

    const float cosYaw = cos(camera[YAW]);
    const float sinYaw = sin(camera[YAW]);

    dest[i*3 + X] = x * cosYaw - z * sinYaw;
    dest[i*3 + Z] = z * cosYaw + x * sinYaw;

    z = obj[i*3 + Z];

    const float cosPitch = cos(camera[PITCH]);
    const float sinPitch = sin(camera[PITCH]);

    dest[i*3 + Y] = y * cosPitch - z * sinPitch;
    dest[i*3 + Z] = z * cosPitch + y * sinPitch;
}

static int readFragmentShader(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("failed to open `%s`!\n", filename);
        return 1;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    rewind(file);

    fragmentShaderSrc = (char*)malloc(length + 1);
    if (fragmentShaderSrc == NULL) {
        printf("malloc failed");
        return 1;
    }

    long result = fread(fragmentShaderSrc, 1, length, file);
    if (result != length) {
        printf("error while reading file\n");
        return 1;
    }

    fragmentShaderSrc[length] = '\0';

    fclose(file);

    return 0;
}

static GLuint compileShader(GLenum type, const char *src) {
    if (readFragmentShader("./shader.frag") == 1) return 1;

    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);
    GLint ok;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetShaderInfoLog(shader, 512, NULL, log);
        fprintf(stderr, "Shader error: %s\n", log);
        exit(1);
    }
    return shader;
}

static GLuint createProgram() {
    GLuint vs = compileShader(GL_VERTEX_SHADER, vertexShaderSrc);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSrc);
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);
    GLint ok;
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetProgramInfoLog(prog, 512, NULL, log);
        fprintf(stderr, "Link error: %s\n", log);
        exit(1);
    }
    glDeleteShader(vs);
    glDeleteShader(fs);
    return prog;
}

static GLFWwindow *initWindow(int windowWidth, int windowHeight, const char* name) {
    if (!glfwInit()) return NULL;
    GLFWwindow *win = glfwCreateWindow(windowWidth, windowHeight, name, NULL, NULL);
    if (!win) { glfwTerminate(); return NULL; }
    glfwMakeContextCurrent(win);
    glewInit();

    return win;
}

static void loadShader() {
    // fullscreen quad
    //   -1     1
    // -1 +----+
    //    | \ b|
    //    |a \ |
    //  1 +----+
    float verts[] = {
        -1, -1,  1, -1,  -1, 1, // (upper) triangle a
         1, -1,  1,  1,  -1, 1  // triangle b
    };
    // https://www.youtube.com/watch?v=Rin5Cp-Hhj8
    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
}

static void deinit(GLFWwindow *win) {
    free(fragmentShaderSrc);
    glfwDestroyWindow(win);
    glfwTerminate();
}
