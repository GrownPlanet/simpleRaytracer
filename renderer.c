#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>

#include "renderer.h"

static GLuint compile_shader(GLenum type, const char *src);
static char *read_file(const char *filename);

Renderer_t *renderer_init(int window_width, int window_height, const char* name) {
    if (!glfwInit()) return NULL;
    GLFWwindow *window = glfwCreateWindow(window_width, window_height, name, NULL, NULL);
    if (!window) { glfwTerminate(); return NULL; }
    glfwMakeContextCurrent(window);
    glewInit();

    Renderer_t *renderer = (Renderer_t*)malloc(sizeof(Renderer_t));
    renderer->window = window;
    renderer->program = -1;

    return renderer;
}

int renderer_load_shaders(Renderer_t *renderer, const char *vert, const char *frag) {
    char *vertex_shader_src = read_file(vert);
    if (vertex_shader_src == NULL) return 1;

    char *fragment_shader_src = read_file(frag);
    if (fragment_shader_src == NULL) return 1;

    GLuint vs = compile_shader(GL_VERTEX_SHADER, vertex_shader_src);
    GLuint fs = compile_shader(GL_FRAGMENT_SHADER, fragment_shader_src);
    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    GLint ok;
    glGetProgramiv(program, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetProgramInfoLog(renderer->program, 512, NULL, log);
        fprintf(stderr, "Link error: %s\n", log);
        return 1;
    }
    glDeleteShader(vs);
    glDeleteShader(fs);

    glUseProgram(program);

    renderer->program = program;

    free(vertex_shader_src);
    free(fragment_shader_src);

    // fullscreen quad
    //   -1     1
    // -1 +----+
    //    | \ b|
    //    |a \ |
    //  1 +----+
    float verts[] = {
        -1, -1,  1, -1,  -1, 1, // triangle a
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

    return 0;
}

GLint renderer_locate_frag(Renderer_t *renderer, const char *variable) {
    return glGetUniformLocation(renderer->program, variable);
}

void renderer_draw(Renderer_t *renderer, GLint resolution_loc) {
    int w, h;
    glfwGetFramebufferSize(renderer->window, &w, &h);
    glViewport(0, 0, w, h);
    glUniform2f(resolution_loc, (float)w, (float)h);

    // render
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glfwSwapBuffers(renderer->window);
}

void renderer_deinit(Renderer_t *renderer) {
    glfwDestroyWindow(renderer->window);
    glfwTerminate();
    free(renderer);
}

static char *read_file(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("failed to open `%s`!\n", filename);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    rewind(file);

    char *contents = (char*)malloc(length + 1);
    if (contents == NULL) {
        printf("malloc failed");
        return NULL;
    }

    long result = fread(contents, 1, length, file);
    if (result != length) {
        printf("error while reading file\n");
        return NULL;
    }

    contents[length] = '\0';

    fclose(file);

    return contents;
}

static GLuint compile_shader(GLenum type, const char *src) {
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
