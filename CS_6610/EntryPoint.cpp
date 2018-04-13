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
#include "Graphics/Mesh.h"
#include "Graphics/MeshHelpers.h"
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
constexpr float SELECTION_RADIUS = 0.0005f;

const cy::Point3f YELLOW(1.0f, 1.0f, 0.0f);
const cy::Point3f WHITE(1.0f, 1.0f, 1.0f);
const cy::Point3f GREY(0.5f, 0.5f, 0.5f);

constexpr engine::animation::ESkeletonType SKELETON_TYPE = engine::animation::ESkeletonType::SimpleChain;


//~====================================================================================================
// Counters
std::chrono::time_point<std::chrono::steady_clock> LAST_DRAW_TIME_POINT;
bool g_left_mouse_button_pressed = false;
bool g_right_mouse_button_pressed = false;
bool g_control_pressed = false;
bool g_alt_pressed = false;
bool g_f_pressed = false;
bool g_r_pressed = false;
int g_curr_mouse_x = 0;
int g_curr_mouse_y = 0;
int g_curr_mouse_z = 0;
int g_prev_mouse_x = 0;
int g_prev_mouse_y = 0;
int g_prev_mouse_z = 0;
float g_mouse_screen_space_x = 0;
float g_mouse_screen_space_y = 0;
uint8_t g_selected_joint = 0;
uint8_t g_selected_end_effector = 0;


//~====================================================================================================
// Data

// Skeleton
engine::animation::Skeleton* g_skeleton = nullptr;

// Transforms
engine::math::Transform g_camera_transform;

// Matrices
cy::Matrix4f g_view;
cy::Matrix4f g_perspective_projection;

// Shader
cy::GLSLProgram g_GLProgram;

// Meshes
engine::graphics::Mesh g_plane_mesh;
engine::graphics::Mesh* g_joint_meshes;
engine::graphics::Mesh g_target_mesh;

// Misc
std::stringstream g_message_stream;


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
    const char* i_vertex_shader_path,
    const char* i_fragment_shader_path);
void InitMeshes();
void InitCamera();
void InitSkeleton();
void InitSkeletonTransforms();
void InitSkeletonMesh();

// Render functions
void Render();

// Update functions
void Update(float DeltaSeconds);
void UpdateRotationBasedOnInput(float i_delta_x, float i_delta_y);
void UpdatePositionBasedOnInput(float i_delta_x, float i_delta_y, float i_delta_mouse_z);
void UpdateSelection();
void UpdateSkeleton();

// Other functions
void SolveFABRIK();
void PrintMatrix(const cy::Matrix4f& i_matrix);
bool TestPointForSelection(const cy::Point3f& i_point_world_space, const cy::Matrix4f& i_screen);
void ScreenCoordsToWorldRay(
    float i_mouse_x, float i_mouse_y,
    cy::Matrix4f& i_view, cy::Matrix4f& i_projection,
    cy::Point3f& o_origin, cy::Point3f& o_direction
);
bool TestRayOBBIntersection(
    const cy::Point3f& i_ray_origin,
    const cy::Point3f& i_ray_direction,
    const cy::Point3f& i_aabb_min,
    const cy::Point3f& i_aabb_max,
    const cy::Matrix4f& i_model,
    float& o_intersection_distance
);


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
        g_joint_meshes[g_selected_joint].SetColor(engine::graphics::Color::TURQUOISE);
        g_selected_joint = g_selected_joint < g_skeleton->num_joints - 1 ? g_selected_joint + 1 : 0;
        g_joint_meshes[g_selected_joint].SetColor(engine::graphics::Color::CYAN);

    }
    else if (key == 'w')
    {
        g_joint_meshes[g_selected_joint].SetColor(engine::graphics::Color::TURQUOISE);
        g_selected_joint = g_selected_joint == 0 ? g_skeleton->num_joints - 1 : g_selected_joint - 1;
        g_joint_meshes[g_selected_joint].SetColor(engine::graphics::Color::CYAN);
    }
    else if (key == 'e')
    {
        g_joint_meshes[g_selected_end_effector].SetColor(engine::graphics::Color::TURQUOISE);
        g_selected_end_effector = g_selected_end_effector <= engine::animation::Skeleton::LEFT_FOOT ? engine::animation::Skeleton::RIGHT_HAND : g_selected_end_effector - 1;
        g_joint_meshes[g_selected_end_effector].SetColor(engine::graphics::Color::MAGENTA);
    }

    g_f_pressed = key == 'f';
    g_r_pressed = key == 'r';
}

void SpecialFunc(int key, int x, int y)
{
    if (key == GLUT_KEY_F6)
    {
        BuildShaders();
    }
    else
    {
        g_control_pressed = (key == CTRL_KEY);
        g_alt_pressed = (key == ALT_KEY);
    }
}

void SpecialUpFunc(int key, int x, int y)
{
    g_control_pressed = g_control_pressed ? !(key == CTRL_KEY) : g_control_pressed;
    g_alt_pressed = g_alt_pressed ? !(key == ALT_KEY) : g_alt_pressed;
}

void MouseFunc(int button, int state, int x, int y)
{
    g_left_mouse_button_pressed = button == GLUT_LEFT_BUTTON ? state == GLUT_DOWN : g_left_mouse_button_pressed;
    g_right_mouse_button_pressed = button == GLUT_RIGHT_BUTTON ? state == GLUT_DOWN : g_right_mouse_button_pressed;

    const bool buttonPressed = g_left_mouse_button_pressed || g_right_mouse_button_pressed;
    g_curr_mouse_x = buttonPressed ? x : g_curr_mouse_x;
    g_curr_mouse_y = buttonPressed ? y : g_curr_mouse_y;
    g_prev_mouse_x = buttonPressed ? x : g_prev_mouse_x;
    g_prev_mouse_y = buttonPressed ? y : g_prev_mouse_y;

    g_mouse_screen_space_x = float(g_curr_mouse_x - WINDOW_WIDTH / 2) / float(WINDOW_WIDTH / 2);
    g_mouse_screen_space_y = float(g_curr_mouse_y - WINDOW_HEIGHT / 2) / float(WINDOW_HEIGHT / 2);

    UpdateSelection();
}

void MotionFunc(int x, int y)
{
    g_curr_mouse_x = x;
    g_curr_mouse_y = y;

    g_mouse_screen_space_x = float(g_curr_mouse_x - WINDOW_WIDTH / 2) / float(WINDOW_WIDTH / 2);
    g_mouse_screen_space_y = float(g_curr_mouse_y - WINDOW_HEIGHT / 2) / float(WINDOW_HEIGHT / 2);
}

void MouseWheelFunc(int button, int dir, int x, int y)
{
    g_curr_mouse_z += dir * 3;
}


//~====================================================================================================
// Initialization functions

void BuildShaders()
{
    BuildShader(g_GLProgram, PLANE_VERTEX_SHADER_PATH, PLANE_FRAGMENT_SHADER_PATH);
}

bool BuildShader(cy::GLSLProgram& o_program, 
    const char* i_vertex_shader_path,
    const char* i_fragment_shader_path)
{
    // Compile Shaders
    g_message_stream.clear();
    if (o_program.BuildFiles(i_vertex_shader_path, i_fragment_shader_path, nullptr, nullptr, nullptr, &g_message_stream) == false)
    {
        LOG_ERROR("%s", g_message_stream.str().c_str());
        return false;
    }

    // Link Program
    g_message_stream.clear();
    if (o_program.Link(&g_message_stream) == false)
    {
        LOG_ERROR("%s", g_message_stream.str().c_str());
        return false;
    }

    return true;
}

void InitMeshes()
{
    // Init plane
    {
        engine::math::Transform transform;
        transform.rotation_ = engine::math::Quaternion::RIGHT;
        g_plane_mesh.SetTransform(transform);
        g_plane_mesh.SetColor(engine::graphics::Color::GRAY);

        constexpr float planeHalfWidth = 50.0f;
        engine::graphics::MeshHelpers::CreatePlaneMesh(g_plane_mesh, planeHalfWidth);
    }

    // Init target
    {
        engine::math::Transform transform;
        transform.rotation_ = engine::math::Quaternion::FORWARD;
        transform.position_.x_ = -30.0f;
        transform.position_.y_ = 30.0f;
        g_target_mesh.SetTransform(transform);
        g_target_mesh.SetColor(engine::graphics::Color::ORANGE);

        constexpr float halfWidth = 0.5f;
        engine::graphics::MeshHelpers::CreateBoxMesh(g_target_mesh, halfWidth);
    }
}

void InitCamera()
{
    // Initialize the transform
    {
        g_camera_transform.rotation_ = engine::math::Quaternion::UP;
        g_camera_transform.position_.y_ = -15.0f;
        g_camera_transform.position_.z_ = -100.0f;
    }

    // Initialize the perspective projection matrix
    {
        static constexpr float fov = M_PI * 0.25f;
        static constexpr float aspectRatio = 1.0f;
        static constexpr float zNear = 0.1f;
        static constexpr float zFar = 1000.0f;

        g_perspective_projection = cy::Matrix4f::MatrixPerspective(fov, aspectRatio, zNear, zFar);
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

    g_joint_meshes = new engine::graphics::Mesh[g_skeleton->num_joints];
    for (uint8_t i = 0; i < g_skeleton->num_joints; ++i)
    {
        g_joint_meshes[i].SetColor(engine::graphics::Color::TURQUOISE);
        engine::graphics::MeshHelpers::CreateBoxMesh(g_joint_meshes[i], halfWidth);
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
    g_GLProgram.Bind();

    // Set the view transformation
    cy::Matrix4f::GetMatrixFromTransform(g_view, g_camera_transform);
    g_GLProgram.SetUniformMatrix4("g_transform_view", g_view.data);

    // Set the projection matrix
    g_GLProgram.SetUniformMatrix4("g_transform_projection", g_perspective_projection.data);

    // Render the meshes
    g_plane_mesh.Render(g_GLProgram);
    g_target_mesh.Render(g_GLProgram);

    for (uint8_t i = 0; i < g_skeleton->num_joints; ++i)
    {
        g_joint_meshes[i].Render(g_GLProgram, g_skeleton->local_to_world_transforms[i].data);
    }
}

//~====================================================================================================
// Update functions

void Update(float DeltaSeconds)
{
    static constexpr float decrement = 0.1f;
    static float mouseZ = 0.0f;

    const float deltaMouseX = float(g_curr_mouse_x - g_prev_mouse_x);
    const float deltaMouseY = float(g_curr_mouse_y - g_prev_mouse_y);
    const float deltaMouseZ = float(g_curr_mouse_z - g_prev_mouse_z);

    mouseZ = fabs(float(deltaMouseZ)) > fabs(mouseZ) ? float(deltaMouseZ) : mouseZ;

    // Respond to input
    UpdateRotationBasedOnInput(deltaMouseX, deltaMouseY);
    UpdatePositionBasedOnInput(deltaMouseX, deltaMouseY, mouseZ);
    //UpdateSelection();

    // Reset skeleton
    if (g_r_pressed)
    {
        g_r_pressed = false;
        g_target_mesh.GetTransform().position_.x_ = -30.0f;
        g_target_mesh.GetTransform().position_.y_ = 30.0f;

        InitSkeletonTransforms();
        UpdateSkeleton();
    }

    g_prev_mouse_x = g_curr_mouse_x;
    g_prev_mouse_y = g_curr_mouse_y;
    g_prev_mouse_z = g_curr_mouse_z;
    mouseZ -= mouseZ * decrement;
}

void UpdateRotationBasedOnInput(float i_delta_x, float i_delta_y)
{
    if (g_left_mouse_button_pressed)
    {
        static constexpr float rotationDamping = DEGREES_TO_RADIANS(0.1f);

        if (g_control_pressed)
        {
            // Joint rotation
            if (abs(i_delta_x) > 0.0f)
            {
                const engine::math::Quaternion roll(i_delta_x * rotationDamping, engine::math::Vec3D::UNIT_Z);
                g_skeleton->joints[g_selected_joint].local_to_parent.rotation_ = roll * g_skeleton->joints[g_selected_joint].local_to_parent.rotation_;
                g_skeleton->joints[g_selected_joint].local_to_parent.rotation_.Normalize();
                UpdateSkeleton();
            }

            if (abs(i_delta_y) > 0.0f)
            {
                const engine::math::Quaternion pitch(i_delta_y * rotationDamping, engine::math::Vec3D::UNIT_X);
                g_skeleton->joints[g_selected_joint].local_to_parent.rotation_ = pitch * g_skeleton->joints[g_selected_joint].local_to_parent.rotation_;
                g_skeleton->joints[g_selected_joint].local_to_parent.rotation_.Normalize();
                UpdateSkeleton();
            }
        }
        else
        {
            // Camera rotation
            if (abs(i_delta_x) > 0.0f)
            {
                const engine::math::Quaternion yaw(i_delta_x * rotationDamping, engine::math::Vec3D::UNIT_Y);
                g_camera_transform.rotation_ = yaw * g_camera_transform.rotation_;
                g_camera_transform.rotation_.Normalize();
            }
        }
    }
}

void UpdatePositionBasedOnInput(float i_delta_x, float i_delta_y, float i_delta_mouse_z)
{
    if (g_right_mouse_button_pressed)
    {
        static constexpr float movementDamping = 0.05f;

        if (g_alt_pressed)
        {
            g_target_mesh.GetTransform().position_.x_ += i_delta_x * -movementDamping;
            g_target_mesh.GetTransform().position_.y_ += i_delta_y * -movementDamping;
            SolveFABRIK();
        }
        // Camera position
        else
        {
            g_camera_transform.position_.x_ += i_delta_x * movementDamping;
            g_camera_transform.position_.y_ += i_delta_y * -movementDamping;
        }
    }

    if (g_alt_pressed)
    {
        g_target_mesh.GetTransform().position_.z_ += i_delta_mouse_z * 0.25f;
        if (i_delta_mouse_z > 0.0f)
        {
            SolveFABRIK();
        }
    }
    else
    {
        g_camera_transform.position_.z_ += i_delta_mouse_z;
    }
}

void UpdateSelection()
{
    if (g_left_mouse_button_pressed == false)
    {
        return;
    }

    const cy::Matrix4f screen = g_perspective_projection * g_view;
    const cy::Point3f target_world_space(g_target_mesh.GetTransform().position_.x_, g_target_mesh.GetTransform().position_.y_, g_target_mesh.GetTransform().position_.z_);

    if (TestPointForSelection(target_world_space, screen))
    {
        LOG("Target selected!");
    }
    else
    {
        LOG("Target not found!");
    }

    //http://www.opengl-tutorial.org/miscellaneous/clicking-on-objects/picking-with-custom-ray-obb-function/

    //cy::Point3f ray_origin;
    //cy::Point3f ray_direction;
    //ScreenCoordsToWorldRay(
    //    g_mouseScreenSpaceX, g_mouseScreenSpaceY,
    //    g_view, g_perspectiveProjection,
    //    ray_origin, ray_direction
    //);

    //float intersection_distance = 0.0f;
    //const cy::Point3f aabb_min(-g_targetExtents.x_ * 2.0f, -g_targetExtents.y_ * 2.0f, -g_targetExtents.z_ * 2.0f);
    //const cy::Point3f aabb_max = -aabb_min;
    //cy::Matrix4f model;
    //cy::Matrix4f::GetMatrixFromTransform(model, g_targetTransform);

    //bool intersection_found = TestRayOBBIntersection(
    //    ray_origin,
    //    ray_direction,
    //    aabb_min,
    //    aabb_max,
    //    model,
    //    intersection_distance
    //);

    //if (intersection_found)
    //{
    //    LOG("Found intersection with target at %f units!", intersection_distance);
    //}
    //else
    //{
    //    LOG("Target not intersected!");
    //}
}

void UpdateSkeleton()
{
    // Update joint transforms
    for (uint8_t i = 0; i < g_skeleton->num_joints; ++i)
    {
        g_skeleton->UpdateJointTransform(i);
    }

    // Update world positions of all joints
    for (uint8_t i = 0; i < g_skeleton->num_joints; ++i)
    {
        const cy::Point3f joint_trans = g_skeleton->local_to_world_transforms[i].GetTrans();
        g_skeleton->joints_world_space[i].set(joint_trans.x, joint_trans.y, joint_trans.z);
    }
}

void SolveFABRIK()
{
    // Reset to bind pose
    InitSkeletonTransforms();
    UpdateSkeleton();

    // Prepare FABRIK parameters
    engine::animation::FABRIKParams params;
    params.target = g_target_mesh.GetTransform().position_;
    params.skeleton = g_skeleton;

    if (SKELETON_TYPE == engine::animation::ESkeletonType::SimpleChain)
    {
        params.root_joint_index = 0;
        params.end_joint_index = g_skeleton->num_joints - 1;
    }
    else if (SKELETON_TYPE == engine::animation::ESkeletonType::Humanoid)
    {
        if (g_selected_end_effector == engine::animation::Skeleton::LEFT_FOOT)
        {
            params.root_joint_index = engine::animation::Skeleton::UP_LEFT_LEG;
        }
        else if (g_selected_end_effector == engine::animation::Skeleton::RIGHT_FOOT)
        {
            params.root_joint_index = engine::animation::Skeleton::UP_RIGHT_LEG;
        }
        else if (g_selected_end_effector == engine::animation::Skeleton::LEFT_HAND)
        {
            params.root_joint_index = engine::animation::Skeleton::UP_LEFT_ARM;
        }
        else if (g_selected_end_effector == engine::animation::Skeleton::RIGHT_HAND)
        {
            params.root_joint_index = engine::animation::Skeleton::UP_RIGHT_ARM;
        }
        params.end_joint_index = g_selected_end_effector;
    }

    // Solve
    uint8_t num_iterations = engine::animation::FABRIK(params);
    LOG("NumIterations:%d", num_iterations);
}

void PrintMatrix(const cy::Matrix4f& i_matrix)
{
    LOG("%f %f %f %f", i_matrix.data[0], i_matrix.data[1], i_matrix.data[2], i_matrix.data[3]);
    LOG("%f %f %f %f", i_matrix.data[4], i_matrix.data[5], i_matrix.data[6], i_matrix.data[7]);
    LOG("%f %f %f %f", i_matrix.data[8], i_matrix.data[9], i_matrix.data[10], i_matrix.data[11]);
    LOG("%f %f %f %f", i_matrix.data[12], i_matrix.data[13], i_matrix.data[14], i_matrix.data[15]);
}

bool TestPointForSelection(const cy::Point3f& i_point_world_space, const cy::Matrix4f& i_screen)
{
    const cy::Point4f target_screen_space = i_screen * i_point_world_space;
    const cy::Point2f mouse_screen_space2d(g_mouse_screen_space_x, g_mouse_screen_space_y);
    const cy::Point2f target_screen_space2d(target_screen_space.x / target_screen_space.w, -target_screen_space.y / target_screen_space.w);
    const cy::Point2f delta = target_screen_space2d - mouse_screen_space2d;
    return delta.LengthSquared() < SELECTION_RADIUS;
}

void ScreenCoordsToWorldRay(
    float i_mouse_x, float i_mouse_y,
    cy::Matrix4f& i_view, cy::Matrix4f& i_projection,
    cy::Point3f& o_origin, cy::Point3f& o_direction
)
{
    const cy::Point4f ray_start_NDC_space(i_mouse_x, i_mouse_y, -1.0f, 1.0f);
    const cy::Point4f ray_end_NDC_space(i_mouse_x, i_mouse_y, 0.0f, 1.0f);

    const cy::Matrix4f inverse_projection = i_projection.GetInverse();
    const cy::Matrix4f inverse_view = i_view.GetInverse();

    cy::Point4f ray_start_camera_space = inverse_projection * ray_start_NDC_space;
    ray_start_camera_space /= ray_start_camera_space.w;
    cy::Point4f ray_start_world_space = inverse_view * ray_start_camera_space;
    ray_start_world_space /= ray_start_world_space.w;
    cy::Point4f ray_end_camera_space = inverse_projection * ray_end_NDC_space;
    ray_end_camera_space /= ray_end_camera_space.w;
    cy::Point4f ray_end_world_space = inverse_view * ray_end_camera_space;
    ray_end_world_space /= ray_end_world_space.w;

    const cy::Point3f ray_direction(ray_end_world_space - ray_start_world_space);
    o_origin = cy::Point3f(ray_start_world_space);
    o_direction = ray_direction.GetNormalized();
}

bool TestRayOBBIntersection(
    const cy::Point3f& i_ray_origin,
    const cy::Point3f& i_ray_direction,
    const cy::Point3f& i_aabb_min,
    const cy::Point3f& i_aabb_max,
    const cy::Matrix4f& i_model,
    float& o_intersection_distance
)
{
    // Intersection method from Real-Time Rendering and Essential Mathematics for Games

    float min = 0.0f;
    float max = 100000.0f;

    const cy::Point3f OBB_position_world_space = i_model.GetTrans();
    const cy::Point3f ray_to_OBB = OBB_position_world_space - i_ray_origin;

    // Test intersection with the two planes perpendicular to the OBB's X axis
    {
        const cy::Point3f x_axis(i_model.GetRow(0));
        float e = x_axis.Dot(ray_to_OBB);
        float f = i_ray_direction.Dot(x_axis);

        if (fabs(f) > 0.001f)
        {
            float t1 = (e + i_aabb_min.x) / f;
            float t2 = (e + i_aabb_max.x) / f;

            t1 = t1 > t2 ? t2 : t1;
            t2 = t1 > t2 ? t1 : t2;

            max = t2 < max ? t2 : max;
            min = t1 > min ? t1 : min;

            if (max < min)
            {
                return false;
            }
        }
        else if (-e + i_aabb_min.x > 0.0f || -e + i_aabb_max.x < 0.0f)
        {
            return false;
        }
    }

    // Test intersection with the two planes perpendicular to the OBB's Y axis
    {
        const cy::Point3f y_axis(i_model.GetRow(1));
        float e = y_axis.Dot(ray_to_OBB);
        float f = i_ray_direction.Dot(y_axis);

        if (fabs(f) > 0.001f)
        {
            float t1 = (e + i_aabb_min.y) / f;
            float t2 = (e + i_aabb_max.y) / f;

            t1 = t1 > t2 ? t2 : t1;
            t2 = t1 > t2 ? t1 : t2;

            max = t2 < max ? t2 : max;
            min = t1 > min ? t1 : min;

            if (max < min)
            {
                return false;
            }
        }
        else if (-e + i_aabb_min.y > 0.0f || -e + i_aabb_max.y < 0.0f)
        {
            return false;
        }
    }

    // Test intersection with the two planes perpendicular to the OBB's Z axis
    {
        const cy::Point3f z_axis(i_model.GetRow(2));
        float e = z_axis.Dot(ray_to_OBB);
        float f = i_ray_direction.Dot(z_axis);

        if (fabs(f) > 0.001f)
        {
            float t1 = (e + i_aabb_min.z) / f;
            float t2 = (e + i_aabb_max.z) / f;

            t1 = t1 > t2 ? t2 : t1;
            t2 = t1 > t2 ? t1 : t2;

            max = t2 < max ? t2 : max;
            min = t1 > min ? t1 : min;

            if (max < min)
            {
                return false;
            }
        }
        else if (-e + i_aabb_min.z > 0.0f || -e + i_aabb_max.z < 0.0f)
        {
            return false;
        }
    }

    o_intersection_distance = min;
    return true;
}
