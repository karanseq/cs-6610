// Library includes
#include <chrono>
#include <math.h>
#include <sstream>
#include <string>
#include <vector>

// GL includes
#include <GL/glew.h>
#include <GL/freeglut.h>

// Engine includes
#include "Animation/FABRIK.h"
#include "Animation/Skeleton.h"
#include "Math/Transform.h"

// Util includes
#include "Utils/cyGL.h"
#include "Utils/cyMatrix.h"
#include "Utils/cyTriMesh.h"
#include "Utils/lodepng.h"
#include "Utils/logger.h"
#include "Utils/MeshHelpers.h"


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

constexpr engine::animation::ESkeletonType SKELETON_TYPE = engine::animation::ESkeletonType::SimpleChain;


//~====================================================================================================
// Counters
std::chrono::time_point<std::chrono::steady_clock> LAST_DRAW_TIME_POINT;
bool g_leftMouseButtonPressed = false;
bool g_rightMouseButtonPressed = false;
bool g_controlPressed = false;
bool g_altPressed = false;
bool g_fPressed = false;
bool g_rPressed = false;
int g_currMouseX = 0;
int g_currMouseY = 0;
int g_currMouseZ = 0;
int g_prevMouseX = 0;
int g_prevMouseY = 0;
int g_prevMouseZ = 0;
uint8_t g_selectedJoint = 0;
uint8_t g_selectedEndEffector = 0;


//~====================================================================================================
// Data

// Transforms
engine::math::Transform g_cameraTransform;
engine::math::Transform g_planeTransform;
engine::math::Transform g_rootBoneTransform;
engine::math::Transform g_targetTransform;

// Matrices
cy::Matrix4f g_perspectiveProjection;

// Shader
cy::GLSLProgram g_planeGLProgram;

// Buffer IDs
BufferIdGroup g_planeBufferIds;
BufferIdGroup* g_skeletonBufferIds;
BufferIdGroup g_targetBufferIds;

// Misc
std::stringstream g_messageStream;

// Skeleton
engine::animation::Skeleton* g_skeleton = nullptr;


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
void InitSkeleton();
void InitSkeletonTransforms();
void InitSkeletonMesh();

// Render functions
void Render();

// Update functions
void Update(float DeltaSeconds);
void UpdateSkeleton();
void UpdateRoot();
void UpdateJoint(const uint8_t i_jointIndex);

// Other functions
void SolveFABRIK();
void GetMatrixFromTransform(cy::Matrix4f& o_matrix,
    const engine::math::Transform& i_transform);
void PrintMatrix(const cy::Matrix4f& i_matrix);


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
        InitMeshes();
        InitCamera();
        InitSkeleton();
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
    else if (key == 'q')
    {
        g_selectedJoint = g_selectedJoint < g_skeleton->num_joints - 1 ? g_selectedJoint + 1 : 0;
    }
    else if (key == 'w')
    {
        g_selectedJoint = g_selectedJoint == 0 ? g_skeleton->num_joints - 1 : g_selectedJoint - 1;
    }
    else if (key == 'e')
    {
        g_selectedEndEffector = g_selectedEndEffector <= engine::animation::Skeleton::LEFT_FOOT ? engine::animation::Skeleton::RIGHT_HAND : g_selectedEndEffector - 1;
    }

    g_fPressed = key == 'f';
    g_rPressed = key == 'r';
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
    constexpr float planeHalfWidth = 50.0f;
    MeshHelpers::CreatePlaneMesh(g_planeBufferIds, planeHalfWidth);

    g_targetTransform.rotation_ = engine::math::Quaternion::RIGHT;
    g_targetTransform.position_.x_ = 20.0f;
    //g_targetTransform.position_.y_ = 15.0f;
    constexpr float halfWidth = 0.5f;
    MeshHelpers::CreateBoxMesh(g_targetBufferIds, halfWidth);
}

void InitCamera()
{
    // Initialize the transform
    {
        g_cameraTransform.rotation_ = engine::math::Quaternion::UP;
        g_cameraTransform.position_.y_ = -15.0f;
        g_cameraTransform.position_.z_ = -100.0f;
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

void InitSkeleton()
{
    g_skeleton = new engine::animation::Skeleton;

    if (SKELETON_TYPE == engine::animation::ESkeletonType::SimpleChain)
    {
        g_skeleton->num_joints = 10;
        g_skeleton->bone_length = 3.0f;
    }
    else if (SKELETON_TYPE == engine::animation::ESkeletonType::Humanoid)
    {
        g_skeleton->num_joints = 15;
        g_skeleton->bone_length = 5.0f;
    }

    engine::animation::Skeleton::CreateSkeleton(g_skeleton, SKELETON_TYPE);

    UpdateSkeleton();
    InitSkeletonMesh();
}

void InitSkeletonTransforms()
{
    if (SKELETON_TYPE == engine::animation::ESkeletonType::SimpleChain)
    {
        engine::animation::Skeleton::InitSimpleChain(g_skeleton);
    }
    else if (SKELETON_TYPE == engine::animation::ESkeletonType::Humanoid)
    {
        engine::animation::Skeleton::InitHumanoid(g_skeleton);
    }
}

void InitSkeletonMesh()
{
    constexpr float halfWidth = 0.5f;

    g_skeletonBufferIds = static_cast<BufferIdGroup*>(malloc(sizeof(BufferIdGroup) * g_skeleton->num_joints));
    for (uint8_t i = 0; i < g_skeleton->num_joints; ++i)
    {
        MeshHelpers::CreateBoxMesh(g_skeletonBufferIds[i], halfWidth);
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

    // Set the view transformation
    cy::Matrix4f view;
    GetMatrixFromTransform(view, g_cameraTransform);
    g_planeGLProgram.SetUniformMatrix4("g_transform_view", view.data);

    // Set the projection matrix
    g_planeGLProgram.SetUniformMatrix4("g_transform_projection", g_perspectiveProjection.data);

    // Draw the floor
    {
        // Set the model transformation
        cy::Matrix4f model;
        GetMatrixFromTransform(model, g_planeTransform);
        g_planeGLProgram.SetUniformMatrix4("g_transform_model", model.data);

        // Set the color
        g_planeGLProgram.SetUniform("g_color", 0.4f, 0.4f, 0.4f);

        glBindVertexArray(g_planeBufferIds.vertexArrayId);
        glDrawElements(GL_TRIANGLES, MeshHelpers::NUM_INDICES_IN_PLANE, GL_UNSIGNED_BYTE, 0);
    }

    // Draw the skeleton
    {
        const cy::Point3f jointOffset = cy::Point3f(0.0f, g_skeleton->bone_length, 0.0f);

        for (uint8_t i = 0; i < g_skeleton->num_joints; ++i)
        {
            // Set the model transformation
            //g_planeGLProgram.SetUniformMatrix4("g_transform_model", g_skeleton->joint_to_world_transforms[i].data);

            const engine::math::Vec3D& joint_world_position = g_skeleton->solved_joints[i];
            cy::Matrix4f model = cy::Matrix4f::MatrixTrans(cy::Point3f(joint_world_position.x_, joint_world_position.y_, joint_world_position.z_));
            g_planeGLProgram.SetUniformMatrix4("g_transform_model", model.data);

            // Set the color
            if (i == g_selectedJoint)
            {
                g_planeGLProgram.SetUniform("g_color", 0.7f, 0.7f, 0.0f);
            }
            else if (i == g_selectedEndEffector)
            {
                g_planeGLProgram.SetUniform("g_color", 0.8f, 0.2f, 0.0f);
            }
            else
            {
                g_planeGLProgram.SetUniform("g_color", 0.2f, 0.5f, 1.0f);
            }

            glBindVertexArray(g_skeletonBufferIds[i].vertexArrayId);
            glDrawElements(GL_TRIANGLES, MeshHelpers::NUM_INDICES_IN_BOX, GL_UNSIGNED_BYTE, 0);
        }
    }

    // Draw the target
    {
        // Set the model transformation
        cy::Matrix4f model;
        GetMatrixFromTransform(model, g_targetTransform);
        g_planeGLProgram.SetUniformMatrix4("g_transform_model", model.data);

        // Set the color
        g_planeGLProgram.SetUniform("g_color", 0.75f, 0.5f, 0.0f);

        glBindVertexArray(g_targetBufferIds.vertexArrayId);
        glDrawElements(GL_TRIANGLES, MeshHelpers::NUM_INDICES_IN_BOX, GL_UNSIGNED_BYTE, 0);
    }
}

//~====================================================================================================
// Update functions

void Update(float DeltaSeconds)
{
    bool mustUpdateSkeleton = false;

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

        if (g_controlPressed)
        {
            // Joint rotation
            if (abs(deltaMouseX) > 0.0f)
            {
                engine::math::Quaternion roll = engine::math::Quaternion::FORWARD;
                roll.w_ = float(-deltaMouseX) * rotationDamping;

                engine::math::Quaternion& jointRotation = g_skeleton->joints[g_selectedJoint].local_to_parent.rotation_;
                jointRotation = roll * roll * jointRotation;
                jointRotation.Normalize();

                mustUpdateSkeleton = true;
            }

            if (abs(deltaMouseY) > 0.0f)
            {
                engine::math::Quaternion pitch = engine::math::Quaternion::RIGHT;
                pitch.w_ = float(-deltaMouseY) * rotationDamping;

                engine::math::Quaternion& jointRotation = g_skeleton->joints[g_selectedJoint].local_to_parent.rotation_;
                jointRotation = pitch * pitch * jointRotation;
                jointRotation.Normalize();

                mustUpdateSkeleton = true;
            }
        }
        else
        {
            // Camera rotation
            if (abs(deltaMouseX) > 0.0f)
            {
                engine::math::Quaternion yaw = engine::math::Quaternion::UP;
                yaw.w_ = float(-deltaMouseX) * rotationDamping;
                yaw.Normalize();

                g_cameraTransform.rotation_ = yaw * yaw * g_cameraTransform.rotation_;
                g_cameraTransform.rotation_.Normalize();
            }
        }
    }

    // Position
    if (g_rightMouseButtonPressed)
    {
        static constexpr float movementDamping = 0.05f;

        // Joint position
        if (g_controlPressed)
        {
            engine::math::Vec3D& jointPosition = g_skeleton->joints[g_selectedJoint].local_to_parent.position_;
            jointPosition.x_ += float(deltaMouseX) * -movementDamping;
            jointPosition.y_ += float(deltaMouseY) * -movementDamping;
            mustUpdateSkeleton = true;
        }
        // Target position
        else if (g_altPressed)
        {
            g_targetTransform.position_.x_ += float(deltaMouseX) * -movementDamping;
            g_targetTransform.position_.y_ += float(deltaMouseY) * -movementDamping;
        }
        // Camera position
        else
        {
            g_cameraTransform.position_.x_ += float(deltaMouseX) * movementDamping;
            g_cameraTransform.position_.y_ += float(deltaMouseY) * -movementDamping;
        }
    }

    if (g_altPressed)
    {
        g_targetTransform.position_.z_ += mouseZ * 0.25f;
    }
    else
    {
        g_cameraTransform.position_.z_ += mouseZ;
    }

    // Solve FABRIK
    if (g_fPressed)
    {
        g_fPressed = false;
        SolveFABRIK();
    }

    // Reset skeleton
    if (g_rPressed)
    {
        g_rPressed = false;
        InitSkeletonTransforms();
        mustUpdateSkeleton = true;
    }

    if (mustUpdateSkeleton)
    {
        UpdateSkeleton();
    }

    g_prevMouseX = g_currMouseX;
    g_prevMouseY = g_currMouseY;
    g_prevMouseZ = g_currMouseZ;
    mouseZ -= mouseZ * decrement;
}

void UpdateSkeleton()
{
    // Update all the transforms
    UpdateRoot();
    for (uint8_t i = 1; i < g_skeleton->num_joints; ++i)
    {
        UpdateJoint(i);
    }

    // Update world positions of all joints
    for (uint8_t i = 0; i < g_skeleton->num_joints; ++i)
    {
        const cy::Point3f joint_trans = g_skeleton->local_to_world_transforms[i].GetTrans();
        g_skeleton->solved_joints[i].set(joint_trans.x, joint_trans.y, joint_trans.z);
    }
}

void UpdateRoot()
{
    GetMatrixFromTransform(g_skeleton->local_to_world_transforms[0], g_skeleton->joints[0].local_to_parent);
    g_skeleton->world_to_local_transforms[0] = g_skeleton->local_to_world_transforms[0].GetInverse();
    g_skeleton->local_to_world_rotations[0] = g_skeleton->joints[0].local_to_parent.rotation_;
    g_skeleton->world_to_local_rotations[0] = g_skeleton->local_to_world_rotations[0].GetInverse();
}

void UpdateJoint(const uint8_t i_jointIndex)
{
    static const cy::Point3f jointOffset(0.0f, g_skeleton->bone_length, 0.0f);

    // Get the parent's index
    const uint8_t& parent_index = g_skeleton->joints[i_jointIndex].parent_index;

    // Update matrices
    {
        // Get local to parent transformation matrix
        cy::Matrix4f local_to_parent_transform;
        GetMatrixFromTransform(local_to_parent_transform, g_skeleton->joints[i_jointIndex].local_to_parent);

        // Get parent to world transformation matrix
        cy::Matrix4f& parent_to_world_transform = g_skeleton->local_to_world_transforms[parent_index];

        // Set local to world & world to local transformation matrix
        g_skeleton->local_to_world_transforms[i_jointIndex] = parent_to_world_transform * local_to_parent_transform;
        g_skeleton->world_to_local_transforms[i_jointIndex] = g_skeleton->local_to_world_transforms[i_jointIndex].GetInverse();
    }

    // Update rotations
    {
        // Get parent to world rotation
        const engine::math::Quaternion& parent_to_world_rotation = g_skeleton->local_to_world_rotations[parent_index];

        // Set the local to world & world to local rotations
        g_skeleton->local_to_world_rotations[i_jointIndex] = parent_to_world_rotation * g_skeleton->joints[i_jointIndex].local_to_parent.rotation_;
        g_skeleton->local_to_world_rotations[i_jointIndex].Normalize();
        g_skeleton->world_to_local_rotations[i_jointIndex] = g_skeleton->local_to_world_rotations[i_jointIndex].GetInverse();
        g_skeleton->world_to_local_rotations[i_jointIndex].Normalize();
    }
}

void SolveFABRIK()
{
    // Prepare FABRIK parameters
    engine::animation::FABRIKParams params;
    params.target = g_targetTransform.position_;
    params.skeleton = g_skeleton;

    if (SKELETON_TYPE == engine::animation::ESkeletonType::SimpleChain)
    {
        params.root_joint_index = 0;
        params.end_joint_index = g_skeleton->num_joints - 1;
    }
    else if (SKELETON_TYPE == engine::animation::ESkeletonType::Humanoid)
    {
        if (g_selectedEndEffector == engine::animation::Skeleton::LEFT_FOOT)
        {
            params.root_joint_index = engine::animation::Skeleton::UP_LEFT_LEG;
        }
        else if (g_selectedEndEffector == engine::animation::Skeleton::RIGHT_FOOT)
        {
            params.root_joint_index = engine::animation::Skeleton::UP_RIGHT_LEG;
        }
        else if (g_selectedEndEffector == engine::animation::Skeleton::LEFT_HAND)
        {
            params.root_joint_index = engine::animation::Skeleton::UP_LEFT_ARM;
        }
        else if (g_selectedEndEffector == engine::animation::Skeleton::RIGHT_HAND)
        {
            params.root_joint_index = engine::animation::Skeleton::UP_RIGHT_ARM;
        }
        params.end_joint_index = g_selectedEndEffector;
    }

    // Solve
    engine::animation::FABRIK(params);
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

    o_matrix.data[0] = 1.0f - _2yy - _2zz;
    o_matrix.data[4] = _2xy - _2zw;
    o_matrix.data[8] = _2xz + _2yw;

    o_matrix.data[1] = _2xy + _2zw;
    o_matrix.data[5] = 1.0f - _2xx - _2zz;
    o_matrix.data[9] = _2yz - _2xw;

    o_matrix.data[2] = _2xz - _2yw;
    o_matrix.data[6] = _2yz + _2xw;
    o_matrix.data[10] = 1.0f - _2xx - _2yy;
}

void PrintMatrix(const cy::Matrix4f& i_matrix)
{
    LOG("%f %f %f %f", i_matrix.data[0], i_matrix.data[1], i_matrix.data[2], i_matrix.data[3]);
    LOG("%f %f %f %f", i_matrix.data[4], i_matrix.data[5], i_matrix.data[6], i_matrix.data[7]);
    LOG("%f %f %f %f", i_matrix.data[8], i_matrix.data[9], i_matrix.data[10], i_matrix.data[11]);
    LOG("%f %f %f %f", i_matrix.data[12], i_matrix.data[13], i_matrix.data[14], i_matrix.data[15]);
}
