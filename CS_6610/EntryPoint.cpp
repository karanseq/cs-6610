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
constexpr char* VERTEX_SHADER_PATH = "..\\CS_6610\\Content\\default_vertex_shader.glsl";
constexpr char* FRAGMENT_SHADER_PATH = "..\\CS_6610\\Content\\default_fragment_shader.glsl";
constexpr uint16_t MAX_PATH_LENGTH = 1024;

constexpr double FRAME_RATE = 1.0 / 60.0;
constexpr unsigned char CTRL_KEY = 114;
constexpr float DISTANCE_FROM_MESH = 100.0f;


//~====================================================================================================
// Counters
std::chrono::time_point<std::chrono::steady_clock> LAST_DRAW_TIME_POINT;
bool g_leftMouseButtonPressed = false;
bool g_rightMouseButtonPressed = false;
bool g_controlPressed = false;
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
cy::TriMesh g_triMesh;

// Texture Data


// Light Data
Transform g_lightTransform;
cy::Point3f g_ambientLightIntensity(0.2f, 0.2f, 0.2f);
cy::Point3f g_ambient(0.0f, 0.0f, 0.0f);
cy::Point3f g_diffuse(0.0f, 0.0f, 0.0f);
cy::Point3f g_specular(0.75f, 0.75f, 0.75f);
float g_shininess = 100.0f;

// GL Data
cy::GLSLProgram g_GLProgramDefault;
GLuint g_vertexArrayId = 0;
GLuint g_vertexBufferId = 0;
GLuint g_normalBufferId = 0;
GLuint g_texCoordBufferId = 0;
GLuint g_indexBufferId = 0;
GLuint g_diffuseTextureId = 0;
GLuint g_specularTextureId = 0;

// Misc Data
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
            // Set the model transformation
            cy::Matrix4f model;
            GetMatrixFromTransform(model, g_meshTransform);
            g_GLProgramDefault.SetUniformMatrix4("g_transform_model", model.data);

            // Set the view transformation
            cy::Matrix4f view;
            GetMatrixFromTransform(view, g_cameraTransform);
            g_GLProgramDefault.SetUniformMatrix4("g_transform_view", view.data);

            // Set the projection transformation
            g_GLProgramDefault.SetUniformMatrix4("g_transform_projection", g_perspectiveProjection.data);

            // Set the camera position
            g_GLProgramDefault.SetUniform("g_cameraPosition", g_cameraTransform.Position.x, g_cameraTransform.Position.y, g_cameraTransform.Position.z);

            // Set the light parameters
            {
                cy::Matrix4f light;
                GetMatrixFromTransform(light, g_lightTransform);
                cy::Point4f lightPosition = model * light * g_lightTransform.Position;

                g_GLProgramDefault.SetUniform("g_lightPosition", lightPosition.x, lightPosition.y, lightPosition.z);

                g_GLProgramDefault.SetUniform("g_ambientLightIntensity", g_ambientLightIntensity.x, g_ambientLightIntensity.y, g_ambientLightIntensity.z);
                
                g_GLProgramDefault.SetUniform("g_ambient", g_ambient.x, g_ambient.y, g_ambient.z);                
                g_GLProgramDefault.SetUniform("g_diffuse", g_diffuse.x, g_diffuse.y, g_diffuse.z);
                g_GLProgramDefault.SetUniform("g_specular", g_specular.x, g_specular.y, g_specular.z);
                g_GLProgramDefault.SetUniform("g_shininess", g_shininess);
            }
        }

        // Attach and bind textures
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, g_diffuseTextureId);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, g_specularTextureId);

        // Draw the mesh
        glDrawElements(GL_TRIANGLES, g_triMesh.NF() * 3, GL_UNSIGNED_INT, 0);
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
    }
}

void SpecialUpFunc(int key, int x, int y)
{
    g_controlPressed = g_controlPressed ? !(key == CTRL_KEY) : g_controlPressed;
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
    if (g_GLProgramDefault.BuildFiles(VERTEX_SHADER_PATH, FRAGMENT_SHADER_PATH, nullptr, nullptr, nullptr, &g_messageStream) == false)
    {
        LOG_ERROR("%s", g_messageStream.str().c_str());
        return;
    }

    // Link Program
    g_messageStream.clear();
    if (g_GLProgramDefault.Link(&g_messageStream) == false)
    {
        LOG_ERROR("%s", g_messageStream.str().c_str());
        return;
    }
}

void InitMeshes(const char* i_meshPath)
{
    // Load the mesh from disk
    if (g_triMesh.LoadFromFileObj(i_meshPath) == false)
    {
        LOG_ERROR("Couldn't load mesh file:%s", i_meshPath);
        return;
    }

    // Compute additional geometry
    g_triMesh.ComputeNormals();
    if (g_triMesh.IsBoundBoxReady() == false)
    {
        g_triMesh.ComputeBoundingBox();
    }

    // Initialize the mesh transform
    {
        g_meshTransform.Orientation.Zero();
        g_meshTransform.Orientation.x = -90.0f;
        g_meshTransform.Position.Zero();
        g_meshTransform.Position.y -= (g_triMesh.GetBoundMax().z + g_triMesh.GetBoundMin().z) * 0.5f;
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
        const size_t bufferSize = sizeof(cy::Point3f) * g_triMesh.NV();
        glBufferData(GL_ARRAY_BUFFER, bufferSize, &g_triMesh.V(0), GL_STATIC_DRAW);
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
        const size_t bufferSize = sizeof(cy::Point3f) * g_triMesh.NVN();
        glBufferData(GL_ARRAY_BUFFER, bufferSize, &g_triMesh.VN(0), GL_STATIC_DRAW);
        const GLenum errorCode = glGetError();
        if (errorCode != GL_NO_ERROR)
        {
            LOG_ERROR("OpenGL failed to allocate the normal buffer!");
        }
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

    // Create a texture coordinate buffer object and make it active
    {
        constexpr GLsizei bufferCount = 1;
        glGenBuffers(bufferCount, &g_texCoordBufferId);
        const GLenum errorCode = glGetError();
        if (errorCode == GL_NO_ERROR)
        {
            glBindBuffer(GL_ARRAY_BUFFER, g_texCoordBufferId);
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

    // Assign data to the texture coordinate buffer
    {
        const size_t bufferSize = sizeof(cy::Point2f) * g_triMesh.NV();

        cy::Point2f* texCoords = reinterpret_cast<cy::Point2f*>(malloc(bufferSize));
        for (int i = 0; i < g_triMesh.NF(); ++i)
        {
            const cy::TriMesh::TriFace& vertexFace = g_triMesh.F(i);
            const cy::TriMesh::TriFace& texCoordFace = g_triMesh.FT(i);

            texCoords[vertexFace.v[0]] = cy::Point2f(g_triMesh.VT(texCoordFace.v[0]));
            texCoords[vertexFace.v[1]] = cy::Point2f(g_triMesh.VT(texCoordFace.v[1]));
            texCoords[vertexFace.v[2]] = cy::Point2f(g_triMesh.VT(texCoordFace.v[2]));
        }

        glBufferData(GL_ARRAY_BUFFER, bufferSize, texCoords, GL_STATIC_DRAW);
        const GLenum errorCode = glGetError();
        if (errorCode != GL_NO_ERROR)
        {
            LOG_ERROR("OpenGL failed to allocate the normal buffer!");
        }

        free(texCoords);
        texCoords = nullptr;
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
        const size_t bufferSize = sizeof(cy::TriMesh::TriFace) * g_triMesh.NF();
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, bufferSize, &g_triMesh.F(0), GL_STATIC_DRAW);
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
    const int numMaterials = g_triMesh.NM();
    if (numMaterials <= 0)
    {
        return;
    }

    // Extract the first material from the mesh
    constexpr uint8_t firstMaterialIndex = 0;
    const cy::TriMesh::Mtl& material = g_triMesh.M(firstMaterialIndex);

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
        glGenTextures(1, &g_diffuseTextureId);
        // Bind the texture
        glBindTexture(GL_TEXTURE_2D, g_diffuseTextureId);
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
        glGenTextures(1, &g_specularTextureId);
        // Bind the texture
        glBindTexture(GL_TEXTURE_2D, g_specularTextureId);
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
