// Library includes
#include <chrono>
#include <math.h>
#include <sstream>

// GL includes
#include <GL/glew.h>
#include <GL/freeglut.h>

// Windows includes

// Util includes
#include "Utils/cyGL.h"
#include "Utils/cyMatrix.h"
#include "Utils/cyTriMesh.h"
#include "Utils/logger.h"


//~====================================================================================================
// Helpers
#define M_PI 3.14159265359f
#define DEGREES_TO_RADIANS(deg) ((deg) * M_PI / 180.0f)
#define RADIANS_TO_DEGREES(rad) ((rad) * 180.0f / M_PI)


//~====================================================================================================
// Structures
struct Transform
{
    cy::Point3f Position;
    cy::Point3f Orientation;
};


//~====================================================================================================
// Constants
constexpr char* WINDOW_TITLE = "Karan's CS_6610 Playground";
char DEFAULT_MESH_PATH[1024] = {0};
constexpr char* DEFAULT_VERTEX_SHADER_PATH = "..\\CS_6610\\Content\\default_vertex_shader.glsl";
constexpr char* DEFAULT_FRAGMENT_SHADER_PATH = "..\\CS_6610\\Content\\default_fragment_shader.glsl";
constexpr double FRAME_RATE = 1.0 / 60.0;


//~====================================================================================================
// Counters
std::chrono::time_point<std::chrono::steady_clock> LAST_DRAW_TIME_POINT;
bool g_leftMouseButtonPressed = false;
bool g_rightMouseButtonPressed = false;
int g_currMouseX = 0;
int g_currMouseY = 0;
int g_currMouseZ = 0;
int g_prevMouseX = 0;
int g_prevMouseY = 0;
int g_prevMouseZ = 0;


//~====================================================================================================
// Data
// Camera Data
Transform g_cameraTransform;
cy::Matrix4f g_perspectiveProjection;

// Mesh Data
Transform g_meshTransform;
cy::TriMesh g_TriMeshDefault;

// GL Data
cy::GLSLProgram g_GLProgramDefault;
GLuint g_vertexArrayId = 0;
GLuint g_vertexBufferId = 0;
GLuint g_normalBufferId = 0;
GLuint g_indexBufferId = 0;
GLuint g_uniformModelViewProjection = 0;

// Misc Data
std::stringstream g_messageStream;


//~====================================================================================================
// Function declarations

// Callback functions
void DisplayFunc();
void IdleFunc();
void KeyboardFunc(unsigned char key, int x, int y);
void SpecialFunc(int key, int x, int y);
void MouseFunc(int button, int state, int x, int y);
void MotionFunc(int x, int y);
void MouseWheelFunc(int button, int dir, int x, int y);

// Initialization functions
void InitMeshes();
void BuildShaders();
void InitCamera();

// Update functions
void Update(float DeltaSeconds);
void GetMatrixFromTransform(cy::Matrix4f& o_Matrix, const Transform& i_transform);


//~====================================================================================================
// MAIN

int main(int argcp, char** argv)
{
    LOG("Init!");

    if (argcp < 2)
    {
        LOG_ERROR("Insufficient arguments passed to executable!");
        return 0;
    }

    // save off the mesh path passed in as command-line argument
    strncpy_s(DEFAULT_MESH_PATH, argv[1], strlen(argv[1]));

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
        glutMouseFunc(&MouseFunc);
        glutMotionFunc(&MotionFunc);
        glutMouseWheelFunc(&MouseWheelFunc);
    }

    // initialize content
    {
        BuildShaders();
        InitMeshes();
        InitCamera();
    }

    LAST_DRAW_TIME_POINT = std::chrono::steady_clock::now();

    // run!
    glutMainLoop();

    LOG("Shutdown!");

    return 0;
}


//~====================================================================================================
// Callback functions

void DisplayFunc()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // draw stuff here!
    {
        g_GLProgramDefault.Bind();
        {
            cy::Matrix4f model;
            GetMatrixFromTransform(model, g_meshTransform);

            cy::Matrix4f view;
            GetMatrixFromTransform(view, g_cameraTransform);

            cy::Matrix4f modelViewProjection = g_perspectiveProjection * view * model;

            g_GLProgramDefault.SetUniformMatrix4("g_transform_modelViewProjection", modelViewProjection.data);

            cy::Matrix3f modelView_inverseTranspose(view * model);
            modelView_inverseTranspose.Invert();
            modelView_inverseTranspose.Transpose();
            g_GLProgramDefault.SetUniformMatrix3("g_transform_modelView_inverseTranspose", modelView_inverseTranspose.data);
        }

        glDrawElements(GL_TRIANGLES, g_TriMeshDefault.NF() * 3, GL_UNSIGNED_INT, 0);
    }

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

void SpecialFunc(int key, int x, int y)
{
    if (key == GLUT_KEY_F6)
    {
        BuildShaders();
    }
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
    // Compile Shaders
    g_messageStream.clear();
    if (g_GLProgramDefault.BuildFiles(DEFAULT_VERTEX_SHADER_PATH, DEFAULT_FRAGMENT_SHADER_PATH, nullptr, nullptr, nullptr, &g_messageStream) == false)
    {
        LOG_ERROR("%s", g_messageStream.str().c_str());
        return;
    }
    else
    {
        LOG("Built shaders successfully!");
    }

    // Link Program
    g_messageStream.clear();
    if (g_GLProgramDefault.Link(&g_messageStream) == false)
    {
        LOG_ERROR("%s", g_messageStream.str().c_str());
        return;
    }
    else
    {
        LOG("Linked shaders successfully!");
    }

}

void InitMeshes()
{
    // Load the mesh from disk
    g_TriMeshDefault.LoadFromFileObj(DEFAULT_MESH_PATH);
    if (g_TriMeshDefault.IsBoundBoxReady() == false)
    {
        g_TriMeshDefault.ComputeBoundingBox();
    }

    // Initialize the mesh transform
    {
        g_meshTransform.Orientation.Zero();
        g_meshTransform.Orientation.x = -90.0f;
        g_meshTransform.Position.Zero();
        g_meshTransform.Position.y -= (g_TriMeshDefault.GetBoundMax().z + g_TriMeshDefault.GetBoundMin().z) * 0.5f;
    }

    // Create a vertex array object and make it active
    {
        constexpr GLsizei arrayCount = 1;
        glGenVertexArrays(arrayCount, &g_vertexArrayId);
        const GLenum errorCode = glGetError();
        if (errorCode == GL_NO_ERROR)
        {
            glBindVertexArray(g_vertexArrayId);
            const GLenum errorCode = glGetError();
            if (errorCode != GL_NO_ERROR)
            {
                LOG_ERROR("OpenGL failed to bind a new vertex array!");
            }
        }
        else
        {
            LOG_ERROR("OpenGL failed to get an unused vertex array!");
        }
    }

    // Create a vertex buffer object and make it active
    {
        constexpr GLsizei bufferCount = 1;
        glGenBuffers(bufferCount, &g_vertexBufferId);
        const GLenum errorCode = glGetError();
        if (errorCode == GL_NO_ERROR)
        {
            glBindBuffer(GL_ARRAY_BUFFER, g_vertexBufferId);
            const GLenum errorCode = glGetError();
            if (errorCode != GL_NO_ERROR)
            {
                LOG_ERROR("OpenGL failed to bind a new vertex buffer!");
            }
        }
        else
        {
            LOG_ERROR("OpenGL failed to get an unused vertex buffer!");
        }
    }

    // Assign data to the vertex buffer
    {
        const size_t bufferSize = sizeof(cy::Point3f) * g_TriMeshDefault.NV();
        glBufferData(GL_ARRAY_BUFFER, bufferSize, &g_TriMeshDefault.V(0), GL_STATIC_DRAW);
        const GLenum errorCode = glGetError();
        if (errorCode != GL_NO_ERROR)
        {
            LOG_ERROR("OpenGL failed to allocate the vertex buffer!");
        }
    }

    // Initialize vertex position attribute
    {
        constexpr GLuint vertexElementLocation = 0;
        constexpr GLuint elementCount = 3;
        glVertexAttribPointer(vertexElementLocation, elementCount, GL_FLOAT, GL_FALSE, 0, 0);
        const GLenum errorCode = glGetError();
        if (errorCode == GL_NO_ERROR)
        {
            glEnableVertexAttribArray(vertexElementLocation);
            if (errorCode != GL_NO_ERROR)
            {
                LOG_ERROR("OpenGL failed to allocate the index buffer!");
            }
        }
    }

    // Create a normal buffer object and make it active
    {
        constexpr GLsizei bufferCount = 1;
        glGenBuffers(bufferCount, &g_normalBufferId);
        const GLenum errorCode = glGetError();
        if (errorCode == GL_NO_ERROR)
        {
            glBindBuffer(GL_ARRAY_BUFFER, g_normalBufferId);
            const GLenum errorCode = glGetError();
            if (errorCode != GL_NO_ERROR)
            {
                LOG_ERROR("OpenGL failed to bind a new normal buffer!");
            }
        }
        else
        {
            LOG_ERROR("OpenGL failed to get an unused normal buffer!");
        }
    }

    // Assign data to the normal buffer
    {
        cy::Point3f* combinedNormals = reinterpret_cast<cy::Point3f*>(malloc(sizeof(cy::Point3f) * g_TriMeshDefault.NVN()));

        const size_t bufferSize = sizeof(cy::Point3f) * g_TriMeshDefault.NVN();
        glBufferData(GL_ARRAY_BUFFER, bufferSize, &g_TriMeshDefault.VN(0), GL_STATIC_DRAW);
        const GLenum errorCode = glGetError();
        if (errorCode != GL_NO_ERROR)
        {
            LOG_ERROR("OpenGL failed to allocate the normal buffer!");
        }

        free(combinedNormals);
        combinedNormals = nullptr;
    }

    // Initialize vertex normal attribute
    {
        constexpr GLuint vertexElementLocation = 1;
        constexpr GLuint elementCount = 3;
        glVertexAttribPointer(vertexElementLocation, elementCount, GL_FLOAT, GL_FALSE, 0, 0);
        const GLenum errorCode = glGetError();
        if (errorCode == GL_NO_ERROR)
        {
            glEnableVertexAttribArray(vertexElementLocation);
            if (errorCode != GL_NO_ERROR)
            {
                LOG_ERROR("OpenGL failed to allocate the index buffer!");
            }
        }
    }

    // Create an index buffer object and make it active
    {
        constexpr GLsizei bufferCount = 1;
        glGenBuffers(bufferCount, &g_indexBufferId);
        const GLenum errorCode = glGetError();
        if (errorCode == GL_NO_ERROR)
        {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_indexBufferId);
            const GLenum errorCode = glGetError();
            if (errorCode != GL_NO_ERROR)
            {
                LOG_ERROR("OpenGL failed to bind a new index buffer!");
            }
        }
        else
        {
            LOG_ERROR("OpenGL failed to get an unused index buffer!");
        }
    }

    // Assign data to the index buffer
    {
        const size_t bufferSize = sizeof(cy::TriMesh::TriFace) * g_TriMeshDefault.NF();
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, bufferSize, &g_TriMeshDefault.F(0), GL_STATIC_DRAW);
        const GLenum errorCode = glGetError();
        if (errorCode != GL_NO_ERROR)
        {
            LOG_ERROR("OpenGL failed to allocate the index buffer!");
        }
    }
}

void InitCamera()
{
    // Initialize the transform
    {
        g_cameraTransform.Orientation.Zero();
        g_cameraTransform.Position.Zero();
        g_cameraTransform.Position.z = -50.0f;
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
// Update functions

void Update(float DeltaSeconds)
{
    const int deltaMouseX = g_currMouseX - g_prevMouseX;
    const int deltaMouseY = g_currMouseY - g_prevMouseY;
    const int deltaMouseZ = g_currMouseZ - g_prevMouseZ;

    // Update camera rotation
    if (g_leftMouseButtonPressed)
    {
        static constexpr float rotationDamping = DEGREES_TO_RADIANS(10.0f);
        g_cameraTransform.Orientation.y += float(deltaMouseX) * rotationDamping;
        g_cameraTransform.Orientation.x += float(deltaMouseY) * rotationDamping;
    }

    // Update camera location
    if (g_rightMouseButtonPressed)
    {
        static constexpr float movementDamping = 0.05f;
        g_cameraTransform.Position.x += float(deltaMouseX) * movementDamping;
        g_cameraTransform.Position.y += float(deltaMouseY) * -movementDamping;
    }
    g_cameraTransform.Position.z += float(deltaMouseZ);

    g_prevMouseX = g_currMouseX;
    g_prevMouseY = g_currMouseY;
    g_prevMouseZ = g_currMouseZ;
}

void GetMatrixFromTransform(cy::Matrix4f& o_Matrix, const Transform& i_transform)
{
    const cy::Matrix4f matRotationX = cy::Matrix4f::MatrixRotationX(DEGREES_TO_RADIANS(i_transform.Orientation.x));
    const cy::Matrix4f matRotationY = cy::Matrix4f::MatrixRotationY(DEGREES_TO_RADIANS(i_transform.Orientation.y));
    const cy::Matrix4f matRotationZ = cy::Matrix4f::MatrixRotationZ(DEGREES_TO_RADIANS(i_transform.Orientation.z));
    const cy::Matrix4f matTranslation = cy::Matrix4f::MatrixTrans(i_transform.Position);
    o_Matrix = matTranslation * matRotationX * matRotationY * matRotationZ;
}
