// Library includes
#include <chrono>
#include <math.h>
#include <sstream>
#include <string>
#include <vector>

// GL includes
#include <GL/glew.h>
#include <GL/freeglut.h>

// Math includes
#include "Math/Transform.h"

// Util includes
#include "Utils/cyGL.h"
#include "Utils/cyMatrix.h"
#include "Utils/cyTriMesh.h"
#include "Utils/lodepng.h"
#include "Utils/logger.h"


//~====================================================================================================
// Helpers
#define DEGREES_TO_RADIANS(deg) ((deg) * M_PI / 180.0f)
#define RADIANS_TO_DEGREES(rad) ((rad) * 180.0f / M_PI)


//~====================================================================================================
// Constants
constexpr char* WINDOW_TITLE = "Karan's CS_6610 Playground";
constexpr char* CONTENT_PATH = "..\\CS_6610\\Content\\";

constexpr char* PLANE_VERTEX_SHADER_PATH = "..\\CS_6610\\Content\\plane_vertex_shader.glsl";
constexpr char* PLANE_FRAGMENT_SHADER_PATH = "..\\CS_6610\\Content\\plane_fragment_shader.glsl";

constexpr uint8_t CONTENT_PATH_LENGTH = 22;
constexpr uint16_t MAX_PATH_LENGTH = 1024;
constexpr uint16_t WINDOW_WIDTH = 1024;
constexpr uint16_t WINDOW_HEIGHT = 1024;
constexpr unsigned char CTRL_KEY = 114;
constexpr unsigned char ALT_KEY = 116;

constexpr GLuint INVALID_INDEX = -1;
constexpr double FRAME_RATE = 1.0 / 60.0;
constexpr float DISTANCE_FROM_MESH = 100.0f;

const cy::Point3f YELLOW(1.0f, 1.0f, 0.0f);
const cy::Point3f WHITE(1.0f, 1.0f, 1.0f);
const cy::Point3f GREY(0.5f, 0.5f, 0.5f);



//~====================================================================================================
// Enums
enum class SceneType
{
    ERegularScene,
    EReflectiveScene,
    EShadowedScene
};

//~====================================================================================================
// Structures

struct BufferIdGroup
{
    GLuint vertexArrayId = INVALID_INDEX;
    GLuint vertexBufferId = INVALID_INDEX;
    GLuint normalBufferId = INVALID_INDEX;
    GLuint texCoordId = INVALID_INDEX;
    GLuint indexBufferId = INVALID_INDEX;
};


//~====================================================================================================
// Counters
std::chrono::time_point<std::chrono::steady_clock> LAST_DRAW_TIME_POINT;
bool g_leftMouseButtonPressed = false;
bool g_rightMouseButtonPressed = false;
bool g_controlPressed = false;
bool g_altPressed = false;
int g_currMouseX = 0;
int g_currMouseY = 0;
int g_currMouseZ = 0;
int g_prevMouseX = 0;
int g_prevMouseY = 0;
int g_prevMouseZ = 0;


//~====================================================================================================
// Data

// Transforms
engine::math::Transform g_cameraTransform;
engine::math::Transform g_planeTransform;

// Matrices
cy::Matrix4f g_perspectiveProjection;

// Shader
cy::GLSLProgram g_planeGLProgram;

// Buffer IDs
// Teapot
BufferIdGroup g_planeBufferIds;

// Misc
std::stringstream g_messageStream;


//~====================================================================================================
// Function declarations

// Callback functions
void DisplayFunc();
void IdleFunc();
void KeyboardFunc(unsigned char key, int x, int y);
void SpecialFunc(int key, int x, int y);
void SpecialUpFunc(int key, int x, int y);
void MouseFunc(int button, int state, int x, int y);
void MotionFunc(int x, int y);
void MouseWheelFunc(int button, int dir, int x, int y);

// Initialization functions
void BuildShaders();
bool BuildShader(cy::GLSLProgram& o_program,
    const char* i_vertexShaderPath,
    const char* i_fragmentShaderPath);
void InitMeshes();
void InitCamera();

// Render functions
void Render();

// Update functions
void Update(float DeltaSeconds);
void GetMatrixFromTransform(cy::Matrix4f& o_matrix,
    const engine::math::Transform& i_transform);


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

        glutInitWindowPosition(screen_width / 2 - WINDOW_WIDTH / 2, screen_height / 2 - WINDOW_HEIGHT / 2);
        glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
        glutInitContextVersion(3, 3);
        glutInitContextProfile(GLUT_CORE_PROFILE);
        glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
        glutCreateWindow(WINDOW_TITLE);
        glEnable(GL_DEPTH_TEST);
    }

    // initialize GLEW
    {
        GLenum err = glewInit();
        if (GLEW_OK != err)
        {
            LOG_ERROR("glewInit error:%s", glewGetErrorString(err));
            return 0;
        }
    }

    // register necessary callbacks
    {
        glutDisplayFunc(&DisplayFunc);
        glutIdleFunc(&IdleFunc);
        glutKeyboardFunc(&KeyboardFunc);
        glutSpecialFunc(&SpecialFunc);
        glutSpecialUpFunc(&SpecialUpFunc);
        glutMouseFunc(&MouseFunc);
        glutMotionFunc(&MotionFunc);
        glutMouseWheelFunc(&MouseWheelFunc);
    }

    // initialize content
    {
        BuildShaders();
        {
            InitMeshes();
        }
        InitCamera();
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
    Render();
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
    static constexpr unsigned char ESC_KEY = 27;

    if (key == ESC_KEY)
    {
        glutLeaveMainLoop();
    }
}

void SpecialFunc(int key, int x, int y)
{
    if (key == GLUT_KEY_F6)
    {
        BuildShaders();
    }
    else
    {
        g_controlPressed = (key == CTRL_KEY);
        g_altPressed = (key == ALT_KEY);
    }
}

void SpecialUpFunc(int key, int x, int y)
{
    g_controlPressed = g_controlPressed ? !(key == CTRL_KEY) : g_controlPressed;
    g_altPressed = g_altPressed ? !(key == ALT_KEY) : g_altPressed;
}

void MouseFunc(int button, int state, int x, int y)
{
    g_leftMouseButtonPressed = button == GLUT_LEFT_BUTTON ? state == GLUT_DOWN : g_leftMouseButtonPressed;
    g_rightMouseButtonPressed = button == GLUT_RIGHT_BUTTON ? state == GLUT_DOWN : g_rightMouseButtonPressed;

    const bool buttonPressed = g_leftMouseButtonPressed || g_rightMouseButtonPressed;
    g_currMouseX = buttonPressed ? x : g_currMouseX;
    g_currMouseY = buttonPressed ? y : g_currMouseY;
    g_prevMouseX = buttonPressed ? x : g_prevMouseX;
    g_prevMouseY = buttonPressed ? y : g_prevMouseY;
}

void MotionFunc(int x, int y)
{
    g_currMouseX = x;
    g_currMouseY = y;
}

void MouseWheelFunc(int button, int dir, int x, int y)
{
    g_currMouseZ += dir * 3;
}


//~====================================================================================================
// Initialization functions

void BuildShaders()
{
    BuildShader(g_planeGLProgram, PLANE_VERTEX_SHADER_PATH, PLANE_FRAGMENT_SHADER_PATH);
}

bool BuildShader(cy::GLSLProgram& o_program, 
    const char* i_vertexShaderPath,
    const char* i_fragmentShaderPath)
{
    // Compile Shaders
    g_messageStream.clear();
    if (o_program.BuildFiles(i_vertexShaderPath, i_fragmentShaderPath, nullptr, nullptr, nullptr, &g_messageStream) == false)
    {
        LOG_ERROR("%s", g_messageStream.str().c_str());
        return false;
    }

    // Link Program
    g_messageStream.clear();
    if (o_program.Link(&g_messageStream) == false)
    {
        LOG_ERROR("%s", g_messageStream.str().c_str());
        return false;
    }

    return true;
}

void InitMeshes()
{
    //================================================
    // Floor

    g_planeTransform.rotation_ = engine::math::Quaternion::RIGHT;

    // Create a vertex array object and make it active
    {
        constexpr GLsizei arrayCount = 1;
        glGenVertexArrays(arrayCount, &g_planeBufferIds.vertexArrayId);
        glBindVertexArray(g_planeBufferIds.vertexArrayId);
    }

    // Create a vertex buffer object and make it active
    {
        constexpr GLsizei bufferCount = 1;
        glGenBuffers(bufferCount, &g_planeBufferIds.vertexBufferId);
        glBindBuffer(GL_ARRAY_BUFFER, g_planeBufferIds.vertexBufferId);
    }

    // Assign data to the vertex buffer
    {
        constexpr uint8_t numVertices = 4;
        constexpr float halfSize = 50.0f;
        const cy::Point3f vertices[numVertices] = {
            { -halfSize, 0.0f, halfSize },     // bottom-left
            { halfSize, 0.0f, halfSize },      // bottom-right
            { halfSize, 0.0f, -halfSize },       // top-right
            { -halfSize, 0.0f, -halfSize }       // top-left
        };

        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    }

    // Initialize vertex position attribute
    {
        constexpr GLuint vertexElementLocation = 0;
        constexpr GLuint elementCount = 3;
        glVertexAttribPointer(vertexElementLocation, elementCount, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(vertexElementLocation);
    }

    // Create an index buffer object and make it active
    {
        constexpr GLsizei bufferCount = 1;
        glGenBuffers(bufferCount, &g_planeBufferIds.indexBufferId);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_planeBufferIds.indexBufferId);
    }

    // Assign data to the index buffer
    {
        constexpr uint8_t numIndices = 6;
        constexpr uint8_t indices[numIndices] = { 0, 1, 2, 0, 2, 3 };
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    }
}

void InitCamera()
{
    // Initialize the transform
    {
        engine::math::Quaternion initialRotation = engine::math::Quaternion::RIGHT;
        initialRotation.w_ = DEGREES_TO_RADIANS(-5.0f);

        g_cameraTransform.rotation_ = engine::math::Quaternion::UP;
        g_cameraTransform.rotation_ = initialRotation * g_cameraTransform.rotation_;
        g_cameraTransform.rotation_.Normalize();

        g_cameraTransform.position_.z_ = -200.0f;
    }

    // Initialize the perspective projection matrix
    {
        static constexpr float fov = M_PI * 0.25f;
        static constexpr float aspectRatio = 1.0f;
        static constexpr float zNear = 0.1f;
        static constexpr float zFar = 1000.0f;

        g_perspectiveProjection = cy::Matrix4f::MatrixPerspective(fov, aspectRatio, zNear, zFar);
    }
}

//~====================================================================================================
// Render functions

void Render()
{
    // Clear
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

    // Bind the shaders
    g_planeGLProgram.Bind();

    // Set the model transformation
    cy::Matrix4f model;
    GetMatrixFromTransform(model, g_planeTransform);
    g_planeGLProgram.SetUniformMatrix4("g_transform_model", model.data);

    // Set the view transformation
    cy::Matrix4f view;
    GetMatrixFromTransform(view, g_cameraTransform);
    g_planeGLProgram.SetUniformMatrix4("g_transform_view", view.data);

    // Set the projection matrix
    g_planeGLProgram.SetUniformMatrix4("g_transform_projection", g_perspectiveProjection.data);

    // Set the color
    g_planeGLProgram.SetUniform("g_color", 0.4f, 0.4f, 0.4f);

    // Draw the mesh
    {
        glBindVertexArray(g_planeBufferIds.vertexArrayId);
        static constexpr uint8_t indexCount = 6;
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_BYTE, 0);
    }
}

//~====================================================================================================
// Update functions

void Update(float DeltaSeconds)
{
    static constexpr float decrement = 0.1f;
    static float mouseZ = 0.0f;

    const int deltaMouseX = g_currMouseX - g_prevMouseX;
    const int deltaMouseY = g_currMouseY - g_prevMouseY;
    const int deltaMouseZ = g_currMouseZ - g_prevMouseZ;

    mouseZ = fabs(float(deltaMouseZ)) > fabs(mouseZ) ? float(deltaMouseZ) : mouseZ;

    // Rotation
    if (g_leftMouseButtonPressed)
    {
        static constexpr float rotationDamping = DEGREES_TO_RADIANS(0.05f);

        // Camera
        if (abs(deltaMouseX) > 0.0f)
        {
            engine::math::Quaternion yaw = engine::math::Quaternion::UP;
            yaw.w_ = float(deltaMouseX) * rotationDamping;

            g_cameraTransform.rotation_ = yaw * g_cameraTransform.rotation_;
            g_cameraTransform.rotation_.Normalize();
        }
    }

    // Location
    if (g_rightMouseButtonPressed)
    {
        static constexpr float movementDamping = 0.05f;

        g_cameraTransform.position_.x_ += float(deltaMouseX) * movementDamping;
        g_cameraTransform.position_.y_ += float(deltaMouseY) * -movementDamping;
    }

    g_cameraTransform.position_.z_ += mouseZ;

    g_prevMouseX = g_currMouseX;
    g_prevMouseY = g_currMouseY;
    g_prevMouseZ = g_currMouseZ;
    mouseZ -= mouseZ * decrement;
}

void GetMatrixFromTransform(cy::Matrix4f& o_matrix, const engine::math::Transform& i_transform)
{
    o_matrix.data[3] = 0.0f;
    o_matrix.data[7] = 0.0f;
    o_matrix.data[11] = 0.0f;
    o_matrix.data[12] = i_transform.position_.x_;
    o_matrix.data[13] = i_transform.position_.y_;
    o_matrix.data[14] = i_transform.position_.z_;
    o_matrix.data[15] = 1.0f;

    const auto _2x = i_transform.rotation_.x_ + i_transform.rotation_.x_;
    const auto _2y = i_transform.rotation_.y_ + i_transform.rotation_.y_;
    const auto _2z = i_transform.rotation_.z_ + i_transform.rotation_.z_;
    const auto _2xx = i_transform.rotation_.x_ * _2x;
    const auto _2xy = _2x * i_transform.rotation_.y_;
    const auto _2xz = _2x * i_transform.rotation_.z_;
    const auto _2xw = _2x * i_transform.rotation_.w_;
    const auto _2yy = _2y * i_transform.rotation_.y_;
    const auto _2yz = _2y * i_transform.rotation_.z_;
    const auto _2yw = _2y * i_transform.rotation_.w_;
    const auto _2zz = _2z * i_transform.rotation_.z_;
    const auto _2zw = _2z * i_transform.rotation_.w_;

    //o_matrix.data[0] = 1.0f - _2yy - _2zz;
    //o_matrix.data[4] = _2xy - _2zw;
    //o_matrix.data[8] = _2xz + _2yw;

    //o_matrix.data[1] = _2xy + _2zw;
    //o_matrix.data[5] = 1.0f - _2xx - _2zz;
    //o_matrix.data[9] = _2yz - _2xw;

    //o_matrix.data[2] = _2xz - _2yw;
    //o_matrix.data[6] = _2yz + _2xw;
    //o_matrix.data[10] = 1.0f - _2xx - _2yy;

    o_matrix.data[0] = 1.0f - _2yy - _2zz;
    o_matrix.data[1] = _2xy - _2zw;
    o_matrix.data[2] = _2xz + _2yw;

    o_matrix.data[4] = _2xy + _2zw;
    o_matrix.data[5] = 1.0f - _2xx - _2zz;
    o_matrix.data[6] = _2yz - _2xw;

    o_matrix.data[8] = _2xz - _2yw;
    o_matrix.data[9] = _2yz + _2xw;
    o_matrix.data[10] = 1.0f - _2xx - _2yy;
}
