#include "SDL.h"
#include <gl/glew.h>
#include <SDL_opengl.h>
#include <gl/glu.h>
#include <config.hpp>

void drawTri() {
    glBegin(GL_QUADS);

    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(-1.0f, -1.0f, 1.0f);
    glVertex3f(1.0f, -1.0f, 1.0f);
    glVertex3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-1.0f, 1.0f, 1.0f);

    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-1.0f, -1.0f, -1.0f);
    glVertex3f(1.0f, -1.0f, -1.0f);
    glVertex3f(1.0f, 1.0f, -1.0f);
    glVertex3f(-1.0f, 1.0f, -1.0f);

    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(1.0f, -1.0f, -1.0f);
    glVertex3f(1.0f, 1.0f, -1.0f);
    glVertex3f(1.0f, 1.0f, 1.0f);
    glVertex3f(1.0f, -1.0f, 1.0f);

    glColor3f(1.0f, 1.0f, 0.0f);
    glVertex3f(-1.0f, -1.0f, -1.0f);
    glVertex3f(-1.0f, 1.0f, -1.0f);
    glVertex3f(-1.0f, 1.0f, 1.0f);
    glVertex3f(-1.0f, -1.0f, 1.0f);

    glEnd();
}


int initGL(SDL_Window *window) {
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_GLContext context = SDL_GL_CreateContext(window);
    if (!context) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create OpenGL context: %s", SDL_GetError());
        return -1;
    }
    SDL_GL_MakeCurrent(window, context);

    GLenum glewError = glewInit();
    if (glewError != GLEW_OK) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize GLEW: %s", glewGetErrorString(glewError));
        return -1;
    }

    return 0;
}

int main(int argc, char *argv[])
{

    if (0 > SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow(
        "LowLevelGame attempt 2 (million)",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        config::window_size[0],
        config::window_size[1],
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL
    );
    if (!window) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create window: %s", SDL_GetError());
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED
    );
    if (!renderer) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create renderer: %s", SDL_GetError());
        return 1;
    }

    if (0 != initGL(window)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize OpenGL: %s", SDL_GetError());
        return 1;
    }


    SDL_Surface *surface;
    SDL_Event event;
    while (true) {
        SDL_PollEvent(&event);
        if (event.type == SDL_QUIT) {
            break;
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        const float aspectRatio = static_cast<float>(config::window_size[0]) / static_cast<float>(config::window_size[1]);
        const float top = 0.1f * std::tan(45.0f * 0.5f * 3.14159265f / 180.0f);
        const float bottom = -top;
        const float right = top * aspectRatio;
        const float left = -right;
        glFrustum(left, right, bottom, top, 0.1f, 100.0f);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glTranslatef(0.0f, 0.0f, -5.0f);

        glEnable(GL_DEPTH_TEST);

        drawTri();


        SDL_SetRenderDrawColor(renderer, 24, 27, 32, 255);
        SDL_RenderClear(renderer);
        // SDL_RenderPresent(renderer);
        SDL_GL_SwapWindow(window);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}

