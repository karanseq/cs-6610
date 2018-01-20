// Library includes
#include <chrono>
#include <iostream>

// GL includes
#include <GL/freeglut.h>


//~====================================================================================================
// Constants
constexpr char* WINDOW_TITLE = "Karan's CS_6610 Playground";
constexpr double FRAME_RATE = 1.0 / 60.0;


//~====================================================================================================
// Counters
std::chrono::time_point<std::chrono::steady_clock> LAST_DRAW_TIME_POINT;
float r = 0.3f;
float g = 0.9f;
float b = 0.4f;


//~====================================================================================================
// Function declarations

// Callback functions
void DisplayFunc();
void IdleFunc();
void KeyboardFunc(unsigned char key, int x, int y);

// Update functions
void Update(float DeltaSeconds);
void AnimateBackgroundColor();


//~====================================================================================================
// MAIN

int main(int argcp, char** argv)
{
    // initialize GLUT
    {
        glutInit(&argcp, argv);
        glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
    }

    // initialize & create the window and the OpenGL context
    {
        const int screen_width = glutGet(GLUT_SCREEN_WIDTH);
        const int screen_height = glutGet(GLUT_SCREEN_HEIGHT);
        const int window_width = 512;
        const int window_height = 512;

        glutInitWindowPosition(screen_width / 2 - window_width / 2, screen_height / 2 - window_height / 2);
        glutInitWindowSize(window_width, window_height);
        glutCreateWindow(WINDOW_TITLE);
    }

    // register necessary callbacks
    {
        glutDisplayFunc(&DisplayFunc);
        glutIdleFunc(&IdleFunc);
        glutKeyboardFunc(&KeyboardFunc);
    }

    LAST_DRAW_TIME_POINT = std::chrono::steady_clock::now();

    // run!
    glutMainLoop();

    return 0;
}


//~====================================================================================================
// Callback functions

void DisplayFunc()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(r, g, b, 1.0f);

    // draw stuff here!

    glutSwapBuffers();
}

void IdleFunc()
{
    const auto now = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed_time = now - LAST_DRAW_TIME_POINT;

    if (const double elapsed_time_count = elapsed_time.count() > FRAME_RATE)
    {
        LAST_DRAW_TIME_POINT = now;

        Update(float(elapsed_time_count));

        glutPostRedisplay();
    }
}

void KeyboardFunc(unsigned char key, int x, int y)
{
    static const unsigned char ESC_KEY = 27;

    if (key == ESC_KEY)
    {
        glutLeaveMainLoop();
    }
}


//~====================================================================================================
// Update functions

void Update(float DeltaSeconds)
{
    AnimateBackgroundColor();
}

void AnimateBackgroundColor()
{
    static bool r_up = false;
    static bool g_up = false;
    static bool b_up = false;

    r += r_up ? 0.005f : -0.005f;
    g += g_up ? 0.003f : -0.003f;
    b += b_up ? 0.004f : -0.004f;

    r_up = (r < 0.0f || r > 1.0f) ? !r_up : r_up;
    g_up = (g < 0.0f || g > 1.0f) ? !g_up : g_up;
    b_up = (b < 0.0f || b > 1.0f) ? !b_up : b_up;
}
