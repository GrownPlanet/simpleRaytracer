#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include "renderer.h"

void move(const float *obj, const float *camera, float *dest, int i);
void rotate_movement(float *x, float *z, const float *camera);
void rotate(const float *obj, const float *camera, float *dest, int i);

#define CIRCLE_COUNT 7

/*
 * left handed coordinate system:
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
 */

#define X 0
#define Y 1
#define Z 2

#define YAW 3
#define PITCH 4

int main() {
    Renderer_t* renderer = renderer_init(800, 600, "simple shader");
    if (renderer == NULL) return 1;

    const int res = renderer_load_shaders(renderer, "./shader.vert", "./shader.frag");
    if (res == 1) return 1;

    GLint resolution_loc = renderer_locate_frag(renderer, "resolution");
    if (resolution_loc == -1) { printf("'resolution' not found\n"); return 1; }

    GLint focal_length_loc = renderer_locate_frag(renderer, "focal_length");
    if (focal_length_loc == -1) { printf("'focal_length' not found\n"); return 1; }

    GLint positions_loc = renderer_locate_frag(renderer, "positions");
    if (positions_loc == -1) { printf("'positions' not found\n"); return 1; }

    GLint radii_loc = renderer_locate_frag(renderer, "radii");
    if (radii_loc == -1) { printf("'radii' not found\n"); return 1; }

    GLint circle_count_loc = renderer_locate_frag(renderer, "circle_count");
    if (circle_count_loc == -1) { printf("'circle_count' not found\n"); return 1; }

    GLint colors_loc = renderer_locate_frag(renderer, "colors");
    if (colors_loc == -1) { printf("'colors' not found\n"); return 1; }

    const float fov = 50.0;
    const float focal_length = 0.5 / tan((fov * (M_PI / 180.0)) / 2.0);
    glUniform1f(focal_length_loc, focal_length); 
    float camera[6] = {
        0.0, 0.0, -5.0, // x, y, z
        0.0, 0.0,      // yaw, pitch
    };

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

    glUniform1fv(radii_loc, CIRCLE_COUNT, radii);
    glUniform1i(circle_count_loc, CIRCLE_COUNT);
    glUniform3fv(colors_loc, CIRCLE_COUNT, colors);

    double lastFrameTime = 0.0;
    double deltaTime = 0.0;

    const float speed = 10.0;
    const float turnSpeed = 5.0;

    while (!renderer_should_close(renderer)) {
        double now = glfwGetTime();
        deltaTime = now - lastFrameTime;
        lastFrameTime = now;

        glfwPollEvents();

        // movement
        float newX =
            (float)(renderer_key_down(renderer, GLFW_KEY_D) - renderer_key_down(renderer, GLFW_KEY_A)) * speed;
        float newZ =
            (float)(renderer_key_down(renderer, GLFW_KEY_W) - renderer_key_down(renderer, GLFW_KEY_S)) * speed;
        rotate_movement(&newX, &newZ, camera);
        camera[X] += newX * deltaTime;
        camera[Z] += newZ * deltaTime;

        if (renderer_key_down(renderer, GLFW_KEY_E))
            camera[Y] += speed * deltaTime;
        if (renderer_key_down(renderer, GLFW_KEY_Q))
            camera[Y] -= speed * deltaTime;

        if (renderer_key_down(renderer, GLFW_KEY_RIGHT))
            camera[YAW] += turnSpeed * deltaTime;
        if (renderer_key_down(renderer, GLFW_KEY_LEFT))
            camera[YAW] -= turnSpeed * deltaTime;

        if (camera[YAW] > 2 * M_PI) camera[YAW] = 0.0;
        if (camera[YAW] < 0) camera[YAW] = 2 * M_PI;

        if (renderer_key_down(renderer, GLFW_KEY_UP))
            camera[PITCH] += turnSpeed * deltaTime;
        if (renderer_key_down(renderer, GLFW_KEY_DOWN))
            camera[PITCH] -= turnSpeed * deltaTime;

        if (camera[PITCH] > 2 * M_PI) camera[PITCH] = 0.0;
        if (camera[PITCH] < 0) camera[PITCH] = 2 * M_PI;

        // update
        float updated[CIRCLE_COUNT * 3];
        for (int i = 0; i < CIRCLE_COUNT; i++) {
            move(positions, camera, updated, i);
            rotate(updated, camera, updated, i);
        }
        glUniform3fv(positions_loc, CIRCLE_COUNT, updated);

        // render
        renderer_draw(renderer, resolution_loc);
    }

    renderer_deinit(renderer);
}

void move(const float *obj, const float *camera, float *dest, int i) {
    dest[i*3 + X] = obj[i*3 + X] - camera[X];
    dest[i*3 + Y] = obj[i*3 + Y] - camera[Y];
    dest[i*3 + Z] = obj[i*3 + Z] - camera[Z];
}

void rotate_movement(float *x, float *z, const float *camera) {
    const float cosYaw = cos(camera[YAW]);
    const float sinYaw = sin(camera[YAW]);

    const float newZ = *z * cosYaw - *x * sinYaw;
    const float newX = *x * cosYaw + *z * sinYaw;

    *x = newX;
    *z = newZ;
}

void rotate(const float *obj, const float *camera, float *dest, int i) {
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
