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
// Constants
constexpr char* WINDOW_TITLE = "Karan's CS_6610 Playground";
constexpr uint8_t CONTENT_PATH_LENGTH = 22;
constexpr char* CONTENT_PATH = "..\\CS_6610\\Content\\";
constexpr char* SKYBOX_OBJ_PATH = "..\\CS_6610\\Content\\cube.obj";
constexpr char* MESH_VERTEX_SHADER_PATH = "..\\CS_6610\\Content\\mesh_vertex_shader.glsl";
constexpr char* MESH_FRAGMENT_SHADER_PATH = "..\\CS_6610\\Content\\mesh_fragment_shader.glsl";
constexpr char* PLANE_VERTEX_SHADER_PATH = "..\\CS_6610\\Content\\plane_vertex_shader.glsl";
constexpr char* PLANE_FRAGMENT_SHADER_PATH = "..\\CS_6610\\Content\\plane_fragment_shader.glsl";
constexpr char* SKYBOX_VERTEX_SHADER_PATH = "..\\CS_6610\\Content\\skybox_vertex_shader.glsl";
constexpr char* SKYBOX_FRAGMENT_SHADER_PATH = "..\\CS_6610\\Content\\skybox_fragment_shader.glsl";
constexpr uint16_t MAX_PATH_LENGTH = 1024;
constexpr uint16_t WINDOW_WIDTH = 512;
constexpr uint16_t WINDOW_HEIGHT = 512;

constexpr GLuint INVALID_INDEX = -1;
constexpr double FRAME_RATE = 1.0 / 60.0;
constexpr unsigned char CTRL_KEY = 114;
constexpr unsigned char ALT_KEY = 116;
constexpr float DISTANCE_FROM_MESH = 100.0f;


//~====================================================================================================
// Structures
struct Transform
{
    cy::Point3f position;
    cy::Point3f orientation;
};

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

// Camera
Transform g_cameraTransform;
cy::Matrix4f g_perspectiveProjection;

// Meshes
cy::TriMesh g_teapotMesh;
cy::TriMesh g_skyboxMesh;

// Transforms
Transform g_teapotTransform;
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
cy::GLSLProgram g_skyboxGLProgram;

// Buffer IDs
// Teapot
BufferIdGroup g_teapotBufferIds;
BufferIdGroup g_skyboxBufferIds;
BufferIdGroup g_planeBufferIds;

GLuint g_skyboxTextureId = 0;
GLuint g_teapotDiffuseTextureId = 0;
GLuint g_teapotSpecularTextureId = 0;

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
void InitMeshes(const char* i_meshPath);
void InitMeshFromObj(cy::TriMesh& o_mesh,
    BufferIdGroup& o_bufferIds,
    const char* i_objPath,
    bool i_loadNormals = false,
    bool i_loadTexCoords = false);
void InitTextures();
void InitSkyboxTexture(GLenum i_side, const char* i_texturePath);
void InitTeapotTexture(GLuint &o_textureId, const char* i_texturePath);
void InitCamera();
void InitLights();

// Render functions
void RenderSkybox();
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

    RenderSkybox();
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
    BuildShader(g_teapotGLProgram, MESH_VERTEX_SHADER_PATH, MESH_FRAGMENT_SHADER_PATH);
    BuildShader(g_planeGLProgram, PLANE_VERTEX_SHADER_PATH, PLANE_FRAGMENT_SHADER_PATH);
    BuildShader(g_skyboxGLProgram, SKYBOX_VERTEX_SHADER_PATH, SKYBOX_FRAGMENT_SHADER_PATH);
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

void InitMeshes(const char* i_meshPath)
{
    //================================================
    // Teapot
    {
        constexpr bool loadNormals = true;
        constexpr bool loadTexCoords = true;
        InitMeshFromObj(g_teapotMesh,
            g_teapotBufferIds,
            i_meshPath,
            loadNormals,
            loadTexCoords);

        // Initialize the mesh transform
        g_teapotTransform.orientation.Zero();
        g_teapotTransform.orientation.x = -90.0f;
        g_teapotTransform.position.Zero();
        g_teapotTransform.position.y -= (g_teapotMesh.GetBoundMax().z + g_teapotMesh.GetBoundMin().z) * 0.5f;
    }

    //================================================
    // Skybox
    {
        InitMeshFromObj(g_skyboxMesh,
            g_skyboxBufferIds,
            SKYBOX_OBJ_PATH
        );
    }

    //================================================
    // Render Texture Plane

    // Initialize the plane's transform
    g_planeTransform.position.z = -55.0f;

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
        constexpr float halfSize = 17.5f;
        const cy::Point3f vertices[numVertices] = {
            { -halfSize, -halfSize, 0.0f },     // bottom-left
            { halfSize, -halfSize, 0.0f },      // bottom-right
            { halfSize, halfSize, 0.0f },       // top-right
            { -halfSize, halfSize, 0.0f }       // top-left
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

    // Create a texture coordinate buffer object and make it active
    {
        constexpr GLsizei bufferCount = 1;
        glGenBuffers(bufferCount, &g_planeBufferIds.texCoordId);
        glBindBuffer(GL_ARRAY_BUFFER, g_planeBufferIds.texCoordId);
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
    }

    // Initialize the texture coordinate attribute
    {
        constexpr GLuint vertexElementLocation = 1;
        constexpr GLuint elementCount = 2;
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

void InitMeshFromObj(cy::TriMesh& o_mesh,
    BufferIdGroup& o_bufferIds,
    const char* i_objPath,
    bool i_loadNormals/* = false*/,
    bool i_loadTexCoords/* = false*/)
{
    // Load the mesh from disk
    if (o_mesh.LoadFromFileObj(i_objPath) == false)
    {
        LOG_ERROR("Couldn't load mesh file:%s", i_objPath);
        return;
    }

    // Check if normals need to be computed
    if (i_loadNormals)
    {
        o_mesh.ComputeNormals();
    }

    // Compute additional geometry
    if (o_mesh.IsBoundBoxReady() == false)
    {
        o_mesh.ComputeBoundingBox();
    }

    // Create a vertex array object and make it active
    {
        constexpr GLsizei arrayCount = 1;
        glGenVertexArrays(arrayCount, &o_bufferIds.vertexArrayId);
        glBindVertexArray(o_bufferIds.vertexArrayId);
    }

    // Create a vertex buffer object and make it active
    {
        constexpr GLsizei bufferCount = 1;
        glGenBuffers(bufferCount, &o_bufferIds.vertexBufferId);
        glBindBuffer(GL_ARRAY_BUFFER, o_bufferIds.vertexBufferId);
    }

    // Assign data to the vertex buffer
    {
        const size_t bufferSize = sizeof(cy::Point3f) * o_mesh.NF() * 3;
        cy::Point3f* vertices = (cy::Point3f*) malloc(bufferSize);

        for (unsigned int i = 0; i < o_mesh.NF(); ++i)
        {
            const cy::TriMesh::TriFace& triFace = o_mesh.F(i);
            vertices[i * 3] = o_mesh.V(triFace.v[0]);
            vertices[i * 3 + 1] = o_mesh.V(triFace.v[1]);
            vertices[i * 3 + 2] = o_mesh.V(triFace.v[2]);
        }

        glBufferData(GL_ARRAY_BUFFER, bufferSize, vertices, GL_STATIC_DRAW);
        free(vertices);
    }

    // Initialize vertex position attribute
    {
        constexpr GLuint vertexElementLocation = 0;
        constexpr GLuint elementCount = 3;
        glVertexAttribPointer(vertexElementLocation, elementCount, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(vertexElementLocation);
    }

    if (i_loadNormals)
    {
        // Create a normal buffer object and make it active
        {
            constexpr GLsizei bufferCount = 1;
            glGenBuffers(bufferCount, &o_bufferIds.normalBufferId);
            glBindBuffer(GL_ARRAY_BUFFER, o_bufferIds.normalBufferId);
        }

        // Assign data to the normal buffer
        {
            const size_t bufferSize = sizeof(cy::Point3f) * o_mesh.NF() * 3;
            cy::Point3f* normals = (cy::Point3f*) malloc(bufferSize);

            for (unsigned int i = 0; i < o_mesh.NF(); ++i)
            {
                const cy::TriMesh::TriFace& triFace = o_mesh.FN(i);
                normals[i * 3] = o_mesh.VN(triFace.v[0]);
                normals[i * 3 + 1] = o_mesh.VN(triFace.v[1]);
                normals[i * 3 + 2] = o_mesh.VN(triFace.v[2]);
            }

            glBufferData(GL_ARRAY_BUFFER, bufferSize, normals, GL_STATIC_DRAW);
            free(normals);
        }

        // Initialize vertex normal attribute
        {
            constexpr GLuint vertexElementLocation = 1;
            constexpr GLuint elementCount = 3;
            glVertexAttribPointer(vertexElementLocation, elementCount, GL_FLOAT, GL_FALSE, 0, 0);
            glEnableVertexAttribArray(vertexElementLocation);
        }
    }

    if (i_loadTexCoords)
    {
        // Create a texture coordinate buffer object and make it active
        {
            constexpr GLsizei bufferCount = 1;
            glGenBuffers(bufferCount, &o_bufferIds.texCoordId);
            glBindBuffer(GL_ARRAY_BUFFER, o_bufferIds.texCoordId);
        }

        // Assign data to the texture coordinate buffer
        {
            const size_t bufferSize = sizeof(cy::Point2f) * o_mesh.NF() * 3;
            cy::Point2f* uvs = (cy::Point2f*) malloc(bufferSize);

            for (unsigned int i = 0; i < o_mesh.NF(); ++i)
            {
                const cy::TriMesh::TriFace& triFace = o_mesh.FT(i);
                uvs[i * 3] = cy::Point2f(o_mesh.VT(triFace.v[0]));
                uvs[i * 3 + 1] = cy::Point2f(o_mesh.VT(triFace.v[1]));
                uvs[i * 3 + 2] = cy::Point2f(o_mesh.VT(triFace.v[2]));
            }

            glBufferData(GL_ARRAY_BUFFER, bufferSize, uvs, GL_STATIC_DRAW);
            free(uvs);
        }

        // Initialize the texture coordinate attribute
        {
            constexpr GLuint vertexElementLocation = 2;
            constexpr GLuint elementCount = 2;
            glVertexAttribPointer(vertexElementLocation, elementCount, GL_FLOAT, GL_FALSE, 0, 0);
            glEnableVertexAttribArray(vertexElementLocation);
        }
    }

    // Create an index buffer object and make it active
    {
        constexpr GLsizei bufferCount = 1;
        glGenBuffers(bufferCount, &o_bufferIds.indexBufferId);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, o_bufferIds.indexBufferId);
    }

    // Assign data to the index buffer
    {
        const size_t bufferSize = sizeof(cy::TriMesh::TriFace) * o_mesh.NF();
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, bufferSize, &o_mesh.F(0), GL_STATIC_DRAW);
    }
}

void InitTextures()
{
    //================================================
    // Render Texture

    // Initialize the render texture
    {
        constexpr bool useDepthBuffer = true;
        constexpr uint8_t numChannels = 3;
        g_renderTexture.Initialize(useDepthBuffer, numChannels, WINDOW_WIDTH, WINDOW_HEIGHT);
        g_renderTexture.SetTextureFilteringMode(GL_LINEAR, 0);
        g_renderTexture.SetTextureMaxAnisotropy();
        g_renderTexture.BuildTextureMipmaps();
    }

    //================================================
    // Skybox

    {
        // Get a texture id
        glGenTextures(1, &g_skyboxTextureId);        

        InitSkyboxTexture(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, "negz.png");
        InitSkyboxTexture(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, "posz.png");
        InitSkyboxTexture(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, "posy.png");
        InitSkyboxTexture(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, "negy.png");
        InitSkyboxTexture(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, "negx.png");
        InitSkyboxTexture(GL_TEXTURE_CUBE_MAP_POSITIVE_X, "posx.png");

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    //================================================
    // Teapot

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

    // Load diffuse & specular textures
    {
        constexpr bool generateMipMap = true;
        InitTeapotTexture(g_teapotDiffuseTextureId, material.map_Kd.data);
        InitTeapotTexture(g_teapotSpecularTextureId, material.map_Ks.data);
    }
}

void InitSkyboxTexture(GLenum i_side, const char* i_texturePath)
{
    // Texture loading parameters
    unsigned char* textureData = nullptr;
    unsigned int width = 0, height = 0;

    // Load texture
    char fullTexturePath[MAX_PATH_LENGTH];
    sprintf_s(fullTexturePath, "%s%s", CONTENT_PATH, i_texturePath);
    if (unsigned int error = lodepng_decode24_file(&textureData, &width, &height, fullTexturePath))
    {
        LOG_ERROR("Error while loading texture %s:%s", fullTexturePath, lodepng_error_text(error));
    }
    else
    {
        // Bind the texture
        glBindTexture(GL_TEXTURE_CUBE_MAP, g_skyboxTextureId);
        // Set the texture data
        glTexImage2D(i_side, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData);
        // Free the texture
        free(textureData);
    }
}

void InitTeapotTexture(GLuint &o_textureId, const char* i_texturePath)
{
    // Texture loading parameters
    unsigned char* textureData = nullptr;
    unsigned int width = 0, height = 0;

    // Load texture
    char fullTexturePath[MAX_PATH_LENGTH];
    sprintf_s(fullTexturePath, "%s%s", CONTENT_PATH, i_texturePath);
    if (unsigned int error = lodepng_decode24_file(&textureData, &width, &height, fullTexturePath))
    {
        LOG_ERROR("Error while loading texture %s:%s", fullTexturePath, lodepng_error_text(error));
    }
    else
    {
        // Get a texture id
        glGenTextures(1, &o_textureId);
        // Bind the texture
        glBindTexture(GL_TEXTURE_2D, o_textureId);
        // Set the texture data
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData);

        // Set filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

        glGenerateMipmap(GL_TEXTURE_2D);

        // Free the texture
        free(textureData);
    }
}

void InitCamera()
{
    // Initialize the transform
    {
        g_cameraTransform.orientation.Zero();
        g_cameraTransform.position.Zero();
        g_cameraTransform.position.z = -50.0f;
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
        g_lightTransform.orientation.Zero();
        g_lightTransform.position.Zero();
        g_lightTransform.position.z = 75;
    }
}

//~====================================================================================================
// Render functions

void RenderSkybox()
{
    glDepthMask(GL_FALSE);

    g_skyboxGLProgram.Bind();
    {
        // Set the view transformation
        cy::Matrix4f view;
        
        Transform cameraTransform_noTranslation = g_cameraTransform;
        cameraTransform_noTranslation.position.Zero();
        GetMatrixFromTransform(view, cameraTransform_noTranslation);

        g_skyboxGLProgram.SetUniformMatrix4("g_transform_view", view.data);

        // Set the projection transformation
        g_skyboxGLProgram.SetUniformMatrix4("g_transform_projection", g_perspectiveProjection.data);
    }

    // Attach and bind textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, g_skyboxTextureId);

    // Draw the mesh
    {
        glBindVertexArray(g_skyboxBufferIds.vertexArrayId);
        glDrawArrays(GL_TRIANGLES, 0, g_skyboxMesh.NF() * 3);
    }

    glDepthMask(GL_TRUE);
}

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
        g_teapotGLProgram.SetUniform("g_cameraPosition", g_cameraTransform.position.x, g_cameraTransform.position.y, g_cameraTransform.position.z);

        // Set the light parameters
        {
            cy::Matrix4f light;
            GetMatrixFromTransform(light, g_lightTransform);
            cy::Point4f lightPosition = model * light * g_lightTransform.position;

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
        glBindVertexArray(g_teapotBufferIds.vertexArrayId);
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
        glBindVertexArray(g_planeBufferIds.vertexArrayId);

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
            g_lightTransform.orientation.y += float(deltaMouseX) * rotationDamping;
            g_lightTransform.orientation.x += float(deltaMouseY) * rotationDamping;
        }
        else if (g_altPressed)
        {
            g_planeTransform.orientation.y += float(deltaMouseX) * rotationDamping;
            g_planeTransform.orientation.x += float(deltaMouseY) * rotationDamping;
        }
        else
        {
            g_cameraTransform.orientation.y += float(deltaMouseX) * rotationDamping;
            g_cameraTransform.orientation.x += float(deltaMouseY) * rotationDamping;
        }
    }

    // Update camera location
    if (g_rightMouseButtonPressed)
    {
        static constexpr float movementDamping = 0.05f;

        g_cameraTransform.position.x += float(deltaMouseX) * movementDamping;
        g_cameraTransform.position.y += float(deltaMouseY) * -movementDamping;
    }

    if (g_altPressed)
    {
        g_planeTransform.position.z += float(deltaMouseZ);
    }
    else
    {
        g_cameraTransform.position.z += float(deltaMouseZ);        
    }

    g_prevMouseX = g_currMouseX;
    g_prevMouseY = g_currMouseY;
    g_prevMouseZ = g_currMouseZ;
}

void GetMatrixFromTransform(cy::Matrix4f& o_matrix, const Transform& i_transform)
{
    const cy::Matrix4f matRotationX = cy::Matrix4f::MatrixRotationX(DEGREES_TO_RADIANS(i_transform.orientation.x));
    const cy::Matrix4f matRotationY = cy::Matrix4f::MatrixRotationY(DEGREES_TO_RADIANS(i_transform.orientation.y));
    const cy::Matrix4f matRotationZ = cy::Matrix4f::MatrixRotationZ(DEGREES_TO_RADIANS(i_transform.orientation.z));
    const cy::Matrix4f matTranslation = cy::Matrix4f::MatrixTrans(i_transform.position);
    o_matrix = matTranslation * matRotationX * matRotationY * matRotationZ;
}
