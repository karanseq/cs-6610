// Library includes
#include <chrono>
#include <math.h>
#include <sstream>
#include <string>
#include <vector>

// GL includes
#include <GL/glew.h>
#include <GL/freeglut.h>

// Windows includes

// Util includes
#include "Utils/cyGL.h"
#include "Utils/cyMatrix.h"
#include "Utils/cyTriMesh.h"
#include "Utils/lodepng.h"
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
constexpr uint8_t CONTENT_PATH_LENGTH = 22;
constexpr char* CONTENT_PATH = "..\\CS_6610\\Content\\";
constexpr char* MESH_VERTEX_SHADER_PATH = "..\\CS_6610\\Content\\mesh_vertex_shader.glsl";
constexpr char* MESH_FRAGMENT_SHADER_PATH = "..\\CS_6610\\Content\\mesh_fragment_shader.glsl";
constexpr char* PLANE_VERTEX_SHADER_PATH = "..\\CS_6610\\Content\\plane_vertex_shader.glsl";
constexpr char* PLANE_FRAGMENT_SHADER_PATH = "..\\CS_6610\\Content\\plane_fragment_shader.glsl";
constexpr uint16_t MAX_PATH_LENGTH = 1024;
constexpr uint16_t WINDOW_WIDTH = 512;
constexpr uint16_t WINDOW_HEIGHT = 512;

constexpr double FRAME_RATE = 1.0 / 60.0;
constexpr unsigned char CTRL_KEY = 114;
constexpr unsigned char ALT_KEY = 116;
constexpr float DISTANCE_FROM_MESH = 100.0f;


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

// Camera
Transform g_cameraTransform;
cy::Matrix4f g_perspectiveProjection;

// Meshes
// Teapot
Transform g_teapotTransform;
cy::TriMesh g_teapotMesh;

// Render Texture Plane
Transform g_planeTransform;

// Light
Transform g_lightTransform;
cy::Point3f g_ambientLightIntensity(0.2f, 0.2f, 0.2f);
cy::Point3f g_ambient(0.0f, 0.0f, 0.0f);
cy::Point3f g_diffuse(0.0f, 0.0f, 0.0f);
cy::Point3f g_specular(0.75f, 0.75f, 0.75f);
float g_shininess = 100.0f;

// Render Texture
cy::GLRenderTexture2D g_renderTexture;

// Shader
cy::GLSLProgram g_teapotGLProgram;
cy::GLSLProgram g_planeGLProgram;

// Buffer IDs
// Teapot
GLuint g_teapotVertexArrayId = 0;
GLuint g_teapotVertexBufferId = 0;
GLuint g_teapotNormalBufferId = 0;
GLuint g_teapotTexCoordBufferId = 0;
GLuint g_teapotIndexBufferId = 0;
GLuint g_teapotDiffuseTextureId = 0;
GLuint g_teapotSpecularTextureId = 0;

// Rneder Texture Plane
GLuint g_planeVertexArrayId = 0;
GLuint g_planeVertexBufferId = 0;
GLuint g_planeTexCoordBufferId = 0;
GLuint g_planeIndexBufferId = 0;

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
void InitMeshes(const char* i_meshPath);
void InitTextures();
void InitCamera();
void InitLights();

// Render functions
void RenderTeapot();
void RenderTexture();

// Update functions
void Update(float DeltaSeconds);
void GetMatrixFromTransform(cy::Matrix4f& o_Matrix, const Transform& i_transform);


//~====================================================================================================
// MAIN

int main(int argcp, char** argv)
{
    if (argcp < 2)
    {
        LOG_ERROR("Insufficient arguments passed to executable!");
        return 0;
    }

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
            char meshPath[MAX_PATH_LENGTH];
            //strncpy_s(meshPath, argv[1], strlen(argv[1]));
            sprintf_s(meshPath, "%s", argv[1]);

            InitMeshes(meshPath);
        }
        InitTextures();
        InitCamera();
        InitLights();
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
    g_renderTexture.Bind();
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        RenderTeapot();
    }
    g_renderTexture.Unbind();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.15f, 0.15f, 0.15f, 1.0f);

    RenderTexture();

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
    // Compile Shaders
    g_messageStream.clear();
    if (g_teapotGLProgram.BuildFiles(MESH_VERTEX_SHADER_PATH, MESH_FRAGMENT_SHADER_PATH, nullptr, nullptr, nullptr, &g_messageStream) == false)
    {
        LOG_ERROR("%s", g_messageStream.str().c_str());
        return;
    }

    // Link Program
    g_messageStream.clear();
    if (g_teapotGLProgram.Link(&g_messageStream) == false)
    {
        LOG_ERROR("%s", g_messageStream.str().c_str());
        return;
    }

    // Compile shaders
    g_messageStream.clear();
    if (g_planeGLProgram.BuildFiles(PLANE_VERTEX_SHADER_PATH, PLANE_FRAGMENT_SHADER_PATH, nullptr, nullptr, nullptr, &g_messageStream) == false)
    {
        LOG_ERROR("%s", g_messageStream.str().c_str());
        return;
    }

    // Link Program
    g_messageStream.clear();
    if (g_planeGLProgram.Link(&g_messageStream) == false)
    {
        LOG_ERROR("%s", g_messageStream.str().c_str());
        return;
    }
}

void InitMeshes(const char* i_meshPath)
{
    //================================================
    // Teapot

    // Load the mesh from disk
    if (g_teapotMesh.LoadFromFileObj(i_meshPath) == false)
    {
        LOG_ERROR("Couldn't load mesh file:%s", i_meshPath);
        return;
    }

    // Compute additional geometry
    g_teapotMesh.ComputeNormals();
    if (g_teapotMesh.IsBoundBoxReady() == false)
    {
        g_teapotMesh.ComputeBoundingBox();
    }

    // Initialize the mesh transform
    {
        g_teapotTransform.Orientation.Zero();
        g_teapotTransform.Orientation.x = -90.0f;
        g_teapotTransform.Position.Zero();
        g_teapotTransform.Position.y -= (g_teapotMesh.GetBoundMax().z + g_teapotMesh.GetBoundMin().z) * 0.5f;
    }

    // Create a vertex array object and make it active
    {
        constexpr GLsizei arrayCount = 1;
        glGenVertexArrays(arrayCount, &g_teapotVertexArrayId);
        const GLenum errorCode = glGetError();
        if (errorCode == GL_NO_ERROR)
        {
            glBindVertexArray(g_teapotVertexArrayId);
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
        glGenBuffers(bufferCount, &g_teapotVertexBufferId);
        const GLenum errorCode = glGetError();
        if (errorCode == GL_NO_ERROR)
        {
            glBindBuffer(GL_ARRAY_BUFFER, g_teapotVertexBufferId);
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
        const size_t bufferSize = sizeof(cy::Point3f) * g_teapotMesh.NF() * 3;
        cy::Point3f* vertices = (cy::Point3f*) malloc(bufferSize);

        for (unsigned int i = 0; i < g_teapotMesh.NF(); ++i)
        {
            const cy::TriMesh::TriFace& triFace = g_teapotMesh.F(i);
            vertices[i * 3] = g_teapotMesh.V(triFace.v[0]);
            vertices[i * 3 + 1] = g_teapotMesh.V(triFace.v[1]);
            vertices[i * 3 + 2] = g_teapotMesh.V(triFace.v[2]);
        }

        glBufferData(GL_ARRAY_BUFFER, bufferSize, vertices, GL_STATIC_DRAW);
        const GLenum errorCode = glGetError();
        if (errorCode != GL_NO_ERROR)
        {
            LOG_ERROR("OpenGL failed to allocate the vertex buffer!");
        }

        free(vertices);
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
                LOG_ERROR("OpenGL failed to allocate the vertex attrib pointer!");
            }
        }
    }

    // Create a normal buffer object and make it active
    {
        constexpr GLsizei bufferCount = 1;
        glGenBuffers(bufferCount, &g_teapotNormalBufferId);
        const GLenum errorCode = glGetError();
        if (errorCode == GL_NO_ERROR)
        {
            glBindBuffer(GL_ARRAY_BUFFER, g_teapotNormalBufferId);
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
        const size_t bufferSize = sizeof(cy::Point3f) * g_teapotMesh.NF() * 3;
        cy::Point3f* normals = (cy::Point3f*) malloc(bufferSize);

        for (unsigned int i = 0; i < g_teapotMesh.NF(); ++i)
        {
            const cy::TriMesh::TriFace& triFace = g_teapotMesh.FN(i);
            normals[i * 3] = g_teapotMesh.VN(triFace.v[0]);
            normals[i * 3 + 1] = g_teapotMesh.VN(triFace.v[1]);
            normals[i * 3 + 2] = g_teapotMesh.VN(triFace.v[2]);
        }

        glBufferData(GL_ARRAY_BUFFER, bufferSize, normals, GL_STATIC_DRAW);
        const GLenum errorCode = glGetError();
        if (errorCode != GL_NO_ERROR)
        {
            LOG_ERROR("OpenGL failed to allocate the normal buffer!");
        }

        free(normals);
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
                LOG_ERROR("OpenGL failed to allocate the vertex attrib pointer!");
            }
        }
    }

    // Create a texture coordinate buffer object and make it active
    {
        constexpr GLsizei bufferCount = 1;
        glGenBuffers(bufferCount, &g_teapotTexCoordBufferId);
        const GLenum errorCode = glGetError();
        if (errorCode == GL_NO_ERROR)
        {
            glBindBuffer(GL_ARRAY_BUFFER, g_teapotTexCoordBufferId);
            const GLenum errorCode = glGetError();
            if (errorCode != GL_NO_ERROR)
            {
                LOG_ERROR("OpenGL failed to bind a new texture coordinate buffer!");
            }
        }
        else
        {
            LOG_ERROR("OpenGL failed to get an unused texture coordinate buffer!");
        }
    }

    // Assign data to the texture coordinate buffer
    {
        const size_t bufferSize = sizeof(cy::Point2f) * g_teapotMesh.NF() * 3;
        cy::Point2f* uvs = (cy::Point2f*) malloc(bufferSize);

        for (unsigned int i = 0; i < g_teapotMesh.NF(); ++i)
        {
            const cy::TriMesh::TriFace& triFace = g_teapotMesh.FT(i);
            uvs[i * 3] = cy::Point2f(g_teapotMesh.VT(triFace.v[0]));
            uvs[i * 3 + 1] = cy::Point2f(g_teapotMesh.VT(triFace.v[1]));
            uvs[i * 3 + 2] = cy::Point2f(g_teapotMesh.VT(triFace.v[2]));
        }

        glBufferData(GL_ARRAY_BUFFER, bufferSize, uvs, GL_STATIC_DRAW);
        const GLenum errorCode = glGetError();
        if (errorCode != GL_NO_ERROR)
        {
            LOG_ERROR("OpenGL failed to allocate the texture coordinate buffer!");
        }

        free(uvs);
    }

    // Initialize the texture coordinate attribute
    {
        constexpr GLuint vertexElementLocation = 2;
        constexpr GLuint elementCount = 2;
        glVertexAttribPointer(vertexElementLocation, elementCount, GL_FLOAT, GL_FALSE, 0, 0);
        const GLenum errorCode = glGetError();
        if (errorCode == GL_NO_ERROR)
        {
            glEnableVertexAttribArray(vertexElementLocation);
            if (errorCode != GL_NO_ERROR)
            {
                LOG_ERROR("OpenGL failed to allocate the vertex attrib pointer!");
            }
        }
    }

    // Create an index buffer object and make it active
    {
        constexpr GLsizei bufferCount = 1;
        glGenBuffers(bufferCount, &g_teapotIndexBufferId);
        const GLenum errorCode = glGetError();
        if (errorCode == GL_NO_ERROR)
        {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_teapotIndexBufferId);
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
        const size_t bufferSize = sizeof(cy::TriMesh::TriFace) * g_teapotMesh.NF();
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, bufferSize, &g_teapotMesh.F(0), GL_STATIC_DRAW);
        const GLenum errorCode = glGetError();
        if (errorCode != GL_NO_ERROR)
        {
            LOG_ERROR("OpenGL failed to allocate the index buffer!");
        }
    }

    //================================================
    // Render Texture Plane

    // Initialize the plane's transform
    g_planeTransform.Position.z = -55.0f;

    // Create a vertex array object and make it active
    {
        constexpr GLsizei arrayCount = 1;
        glGenVertexArrays(arrayCount, &g_planeVertexArrayId);
        const GLenum errorCode = glGetError();
        if (errorCode == GL_NO_ERROR)
        {
            glBindVertexArray(g_planeVertexArrayId);
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
        glGenBuffers(bufferCount, &g_planeVertexBufferId);
        const GLenum errorCode = glGetError();
        if (errorCode == GL_NO_ERROR)
        {
            glBindBuffer(GL_ARRAY_BUFFER, g_planeVertexBufferId);
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
        constexpr uint8_t numVertices = 4;
        constexpr float halfSize = 17.5f;
        const cy::Point3f vertices[numVertices] = {
            { -halfSize, -halfSize, 0.0f },     // bottom-left
            { halfSize, -halfSize, 0.0f },      // bottom-right
            { halfSize, halfSize, 0.0f },       // top-right
            { -halfSize, halfSize, 0.0f }       // top-left
        };

        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
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
                LOG_ERROR("OpenGL failed to allocate the vertex attrib pointer!");
            }
        }
    }

    // Create a texture coordinate buffer object and make it active
    {
        constexpr GLsizei bufferCount = 1;
        glGenBuffers(bufferCount, &g_planeTexCoordBufferId);
        const GLenum errorCode = glGetError();
        if (errorCode == GL_NO_ERROR)
        {
            glBindBuffer(GL_ARRAY_BUFFER, g_planeTexCoordBufferId);
            const GLenum errorCode = glGetError();
            if (errorCode != GL_NO_ERROR)
            {
                LOG_ERROR("OpenGL failed to bind a new texture coordinate buffer!");
            }
        }
        else
        {
            LOG_ERROR("OpenGL failed to get an unused texture coordinate buffer!");
        }
    }

    // Assign data to the texture coordinate buffer
    {
        constexpr uint8_t numUVs = 4;
        const cy::Point2f uvs[numUVs] = {
            { 0.0f, 0.0f },     // bottom-left
            { 1.0f, 0.0f },     // bottom-right
            { 1.0f, 1.0f },     // top-right
            { 0.0f, 1.0f }      // top-left
        };

        glBufferData(GL_ARRAY_BUFFER, sizeof(uvs), uvs, GL_STATIC_DRAW);
        const GLenum errorCode = glGetError();
        if (errorCode != GL_NO_ERROR)
        {
            LOG_ERROR("OpenGL failed to allocate the texture coordinate buffer!");
        }
    }

    // Initialize the texture coordinate attribute
    {
        constexpr GLuint vertexElementLocation = 1;
        constexpr GLuint elementCount = 2;
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
        glGenBuffers(bufferCount, &g_planeIndexBufferId);
        const GLenum errorCode = glGetError();
        if (errorCode == GL_NO_ERROR)
        {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_planeIndexBufferId);
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
        constexpr uint8_t numIndices = 6;
        constexpr uint8_t indices[numIndices] = {
            0, 1, 2, 0, 2, 3
        };
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
        const GLenum errorCode = glGetError();
        if (errorCode != GL_NO_ERROR)
        {
            LOG_ERROR("OpenGL failed to allocate the index buffer!");
        }
    }
}

void InitTextures()
{
    // Check if the mesh has materials
    const int numMaterials = g_teapotMesh.NM();
    if (numMaterials <= 0)
    {
        return;
    }

    // Extract the first material from the mesh
    constexpr uint8_t firstMaterialIndex = 0;
    const cy::TriMesh::Mtl& material = g_teapotMesh.M(firstMaterialIndex);

    // Set lighting parameters
    g_ambient.Set(material.Ka[0], material.Ka[1], material.Ka[2]);
    g_diffuse.Set(material.Kd[0], material.Kd[1], material.Kd[2]);
    g_specular.Set(material.Ks[0], material.Ks[1], material.Ks[2]);
    g_shininess = material.Ns;

    // Texture loading parameters
    unsigned char* textureData = nullptr;
    unsigned int width = 0, height = 0;

    // Load diffuse texture
    char diffuseTexturePath[MAX_PATH_LENGTH];
    sprintf_s(diffuseTexturePath, "%s%s", CONTENT_PATH, material.map_Kd.data);
    if (unsigned int error = lodepng_decode24_file(&textureData, &width, &height, diffuseTexturePath))
    {
        LOG_ERROR("Error while loading texture %s:%s", diffuseTexturePath, lodepng_error_text(error));
    }
    else
    {
        // Get a texture id
        glGenTextures(1, &g_teapotDiffuseTextureId);
        // Bind the texture
        glBindTexture(GL_TEXTURE_2D, g_teapotDiffuseTextureId);
        // Set the texture data
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData);
        // Set parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glGenerateMipmap(GL_TEXTURE_2D);

        // Free the texture
        free(textureData);
    }

    // Reset texture loading parameters
    textureData = nullptr;
    width = 0;
    height = 0;

    // Load specular texture
    char specularTexturePath[MAX_PATH_LENGTH];
    sprintf_s(specularTexturePath, "%s%s", CONTENT_PATH, material.map_Ks.data);
    if (unsigned int error = lodepng_decode24_file(&textureData, &width, &height, specularTexturePath))
    {
        LOG_ERROR("Error while loading texture %s:%s", specularTexturePath, lodepng_error_text(error));
    }
    else
    {
        // Get a texture id
        glGenTextures(1, &g_teapotSpecularTextureId);
        // Bind the texture
        glBindTexture(GL_TEXTURE_2D, g_teapotSpecularTextureId);
        // Set the texture data
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData);
        // Set trilinear filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glGenerateMipmap(GL_TEXTURE_2D);

        // Free the texture
        free(textureData);
    }

    // Initialize the render texture
    {
        constexpr bool useDepthBuffer = true;
        constexpr uint8_t numChannels = 3;
        g_renderTexture.Initialize(useDepthBuffer, numChannels, WINDOW_WIDTH, WINDOW_HEIGHT);
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

void InitLights()
{
    // Initialize the transform
    {
        g_lightTransform.Orientation.Zero();
        g_lightTransform.Position.Zero();
        g_lightTransform.Position.z = 75;
    }
}

//~====================================================================================================
// Render functions

void RenderTeapot()
{
    g_teapotGLProgram.Bind();
    {
        // Set the model transformation
        cy::Matrix4f model;
        GetMatrixFromTransform(model, g_teapotTransform);
        g_teapotGLProgram.SetUniformMatrix4("g_transform_model", model.data);

        // Set the view transformation
        cy::Matrix4f view;
        GetMatrixFromTransform(view, g_cameraTransform);
        g_teapotGLProgram.SetUniformMatrix4("g_transform_view", view.data);

        // Set the projection transformation
        g_teapotGLProgram.SetUniformMatrix4("g_transform_projection", g_perspectiveProjection.data);

        // Set the camera position
        g_teapotGLProgram.SetUniform("g_cameraPosition", g_cameraTransform.Position.x, g_cameraTransform.Position.y, g_cameraTransform.Position.z);

        // Set the light parameters
        {
            cy::Matrix4f light;
            GetMatrixFromTransform(light, g_lightTransform);
            cy::Point4f lightPosition = model * light * g_lightTransform.Position;

            g_teapotGLProgram.SetUniform("g_lightPosition", lightPosition.x, lightPosition.y, lightPosition.z);
            g_teapotGLProgram.SetUniform("g_ambientLightIntensity", g_ambientLightIntensity.x, g_ambientLightIntensity.y, g_ambientLightIntensity.z);
            g_teapotGLProgram.SetUniform("g_ambient", g_ambient.x, g_ambient.y, g_ambient.z);
            g_teapotGLProgram.SetUniform("g_diffuse", g_diffuse.x, g_diffuse.y, g_diffuse.z);
            g_teapotGLProgram.SetUniform("g_specular", g_specular.x, g_specular.y, g_specular.z);
            g_teapotGLProgram.SetUniform("g_shininess", g_shininess);
        }
    }

    // Attach and bind textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, g_teapotDiffuseTextureId);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, g_teapotSpecularTextureId);

    // Draw the mesh
    {
        glBindVertexArray(g_teapotVertexArrayId);
        glDrawArrays(GL_TRIANGLES, 0, g_teapotMesh.NF() * 3);
    }
}

void RenderTexture()
{
    g_planeGLProgram.Bind();
    {
        // Set the model transformation
        cy::Matrix4f model;
        GetMatrixFromTransform(model, g_planeTransform);
        g_planeGLProgram.SetUniformMatrix4("g_transform_model", model.data);

        // Set the view transformation
        cy::Matrix4f view;
        GetMatrixFromTransform(view, g_cameraTransform);
        g_planeGLProgram.SetUniformMatrix4("g_transform_view", view.data);

        // Set the projection transformation
        g_planeGLProgram.SetUniformMatrix4("g_transform_projection", g_perspectiveProjection.data);
    }

    // Attach and bind textures
    glActiveTexture(GL_TEXTURE0);
    g_renderTexture.BindTexture();

    // Draw the mesh
    {
        glBindVertexArray(g_planeVertexArrayId);

        static constexpr uint8_t indexCount = 6;
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_BYTE, 0);
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

        if (g_controlPressed)
        {
            g_lightTransform.Orientation.y += float(deltaMouseX) * rotationDamping;
            g_lightTransform.Orientation.x += float(deltaMouseY) * rotationDamping;
        }
        else if (g_altPressed)
        {
            g_planeTransform.Orientation.y += float(deltaMouseX) * rotationDamping;
            g_planeTransform.Orientation.x += float(deltaMouseY) * rotationDamping;
        }
        else
        {
            g_cameraTransform.Orientation.y += float(deltaMouseX) * rotationDamping;
            g_cameraTransform.Orientation.x += float(deltaMouseY) * rotationDamping;
        }
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

void GetMatrixFromTransform(cy::Matrix4f& o_matrix, const Transform& i_transform)
{
    const cy::Matrix4f matRotationX = cy::Matrix4f::MatrixRotationX(DEGREES_TO_RADIANS(i_transform.Orientation.x));
    const cy::Matrix4f matRotationY = cy::Matrix4f::MatrixRotationY(DEGREES_TO_RADIANS(i_transform.Orientation.y));
    const cy::Matrix4f matRotationZ = cy::Matrix4f::MatrixRotationZ(DEGREES_TO_RADIANS(i_transform.Orientation.z));
    const cy::Matrix4f matTranslation = cy::Matrix4f::MatrixTrans(i_transform.Position);
    o_matrix = matTranslation * matRotationX * matRotationY * matRotationZ;
}
