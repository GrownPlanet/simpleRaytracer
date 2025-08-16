#ifndef renderer_h_INCLUDED
#define renderer_h_INCLUDED

typedef struct {
    GLFWwindow *window;
    GLint program;
} Renderer_t;

// initialize the renderer
Renderer_t *renderer_init(int window_width, int window_height, const char* name);

// add shaders
int renderer_load_shaders(Renderer_t *renderer, const char *vert, const char *frag);

// locate variable in the fragment shader
GLint renderer_locate_frag(Renderer_t *renderer, const char *variable);

// draw the current screen
void renderer_draw(Renderer_t *renderer, GLint resolution_loc);

// deinitialize
void renderer_deinit(Renderer_t *renderer);

// check if the window should close
#define renderer_should_close(renderer) glfwWindowShouldClose(renderer->window)

// check if a key is down
#define renderer_key_down(renderer, scancode) (int)(glfwGetKey(renderer->window, scancode) == GLFW_PRESS)

#endif // renderer_h_INCLUDED
