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
#include "Math/Vec2D.h"

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

constexpr uint8_t MAX_JOINT_INDEX = 255;
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
bool g_left_mouse_dragging = false;
bool g_right_mouse_dragging = false;
bool g_control_pressed = false;
bool g_alt_pressed = false;
bool g_r_pressed = false;
bool g_target_updated = false;
int g_curr_mouse_x = 0;
int g_curr_mouse_y = 0;
int g_curr_mouse_z = 0;
int g_prev_mouse_x = 0;
int g_prev_mouse_y = 0;
int g_prev_mouse_z = 0;
float g_curr_mouse_screen_space_x = 0;
float g_curr_mouse_screen_space_y = 0;
float g_prev_mouse_screen_space_x = 0;
float g_prev_mouse_screen_space_y = 0;
uint8_t g_selected_joint = MAX_JOINT_INDEX;
uint8_t g_selected_end_effector = 0;


//~====================================================================================================
// Data

// Skeleton
engine::animation::Skeleton* g_skeleton = nullptr;

// Transforms
engine::math::Transform g_camera_transform;

// Matrices
cy::Matrix4f g_view;
cy::Matrix4f g_projection;

// Shader
cy::GLSLProgram g_GLProgram;

// Meshes
engine::graphics::Mesh g_plane_mesh;
engine::graphics::Mesh* g_joint_meshes = nullptr;
engine::graphics::Mesh g_target_mesh;
engine::graphics::Mesh* g_selected_mesh = nullptr;

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
void Reset();

// Mouse functions
void HandleMouseDown();
void HandleMouseUp();
void HandleMouseDrag();

// Other functions
void SolveFABRIK();
void PrintMatrix(const cy::Matrix4f& i_matrix);
bool TestMeshesForSelection(const engine::math::Vec2D& i_mouse_screen_space2d, const cy::Matrix4f& i_screen);
bool TestPointForSelection(const cy::Point3f& i_point_world_space, const cy::Matrix4f& i_screen);


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
    // Update mouse coordinates
    g_curr_mouse_x = x;
    g_curr_mouse_y = y;
    g_prev_mouse_x = x;
    g_prev_mouse_y = y;
    g_curr_mouse_screen_space_x = float(g_curr_mouse_x - WINDOW_WIDTH / 2) / float(WINDOW_WIDTH / 2);
    g_curr_mouse_screen_space_y = float(g_curr_mouse_y - WINDOW_HEIGHT / 2) / float(WINDOW_HEIGHT / 2);
    g_prev_mouse_screen_space_x = g_curr_mouse_screen_space_x;
    g_prev_mouse_screen_space_y = g_curr_mouse_screen_space_y;

    if (state == GLUT_DOWN)
    {
        // Update flags
        g_left_mouse_button_pressed = button == GLUT_LEFT_BUTTON;
        g_right_mouse_button_pressed = button == GLUT_RIGHT_BUTTON;

        HandleMouseDown();
    }
    else if (state == GLUT_UP)
    {
        HandleMouseUp();

        // Update flags
        g_left_mouse_button_pressed = g_right_mouse_button_pressed = false;
        g_left_mouse_dragging = g_left_mouse_button_pressed;
        g_right_mouse_dragging = g_right_mouse_button_pressed;
    }
}

void MotionFunc(int x, int y)
{
    // Update mouse coordinates
    g_curr_mouse_x = x;
    g_curr_mouse_y = y;
    g_prev_mouse_screen_space_x = g_curr_mouse_screen_space_x;
    g_prev_mouse_screen_space_y = g_curr_mouse_screen_space_y;
    g_curr_mouse_screen_space_x = float(g_curr_mouse_x - WINDOW_WIDTH / 2) / float(WINDOW_WIDTH / 2);
    g_curr_mouse_screen_space_y = float(g_curr_mouse_y - WINDOW_HEIGHT / 2) / float(WINDOW_HEIGHT / 2);

    // Update flags
    g_left_mouse_dragging = g_left_mouse_button_pressed;
    g_right_mouse_dragging = g_right_mouse_button_pressed;

    HandleMouseDrag();
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

        g_target_mesh.InitSelection(engine::graphics::EMeshSelectionType::Editable);
        g_target_mesh.SetSelectionDrawType(engine::graphics::EMeshSelectionDrawType::DrawAlways);
        g_target_mesh.SetSelectedColor(engine::graphics::Color::YELLOW);

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

        g_projection = cy::Matrix4f::MatrixPerspective(fov, aspectRatio, zNear, zFar);
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

    g_skeleton->UpdateChain();
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
        g_joint_meshes[i].InitSelection(engine::graphics::EMeshSelectionType::Editable);
        g_joint_meshes[i].SetSelectionDrawType(engine::graphics::EMeshSelectionDrawType::DrawAlways);
        g_joint_meshes[i].SetSelectedColor(engine::graphics::Color::YELLOW);

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
    g_GLProgram.SetUniformMatrix4("g_transform_projection", g_projection.data);

    // Render the meshes
    g_plane_mesh.Render(g_GLProgram);
    g_target_mesh.Render(g_GLProgram);

    for (uint8_t i = 0; i < g_skeleton->num_joints; ++i)
    {
        g_joint_meshes[i].Render(g_GLProgram, g_skeleton->local_to_world_transforms[i]);
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

    if (g_target_updated)
    {
        g_target_updated = false;
        SolveFABRIK();
    }

    // Reset skeleton
    if (g_r_pressed)
    {
        g_r_pressed = false;
        Reset();
    }

    g_prev_mouse_x = g_curr_mouse_x;
    g_prev_mouse_y = g_curr_mouse_y;
    g_prev_mouse_z = g_curr_mouse_z;
    mouseZ -= mouseZ * decrement;
}

void UpdateRotationBasedOnInput(float i_delta_x, float i_delta_y)
{
    static constexpr float rotationDamping = DEGREES_TO_RADIANS(0.1f);

    if (g_left_mouse_dragging)
    {
        // First dibs to skeleton
        if (g_selected_joint < MAX_JOINT_INDEX)
        {
            // Joint rotation
            if (abs(i_delta_x) > 0.0f)
            {
                const engine::math::Quaternion roll(i_delta_x * rotationDamping, engine::math::Vec3D::UNIT_Z);
                g_skeleton->joints[g_selected_joint].local_to_parent.rotation_ = roll * g_skeleton->joints[g_selected_joint].local_to_parent.rotation_;
                g_skeleton->joints[g_selected_joint].local_to_parent.rotation_.Normalize();
                g_skeleton->UpdateChain();
            }

            if (abs(i_delta_y) > 0.0f)
            {
                const engine::math::Quaternion pitch(i_delta_y * rotationDamping, engine::math::Vec3D::UNIT_X);
                g_skeleton->joints[g_selected_joint].local_to_parent.rotation_ = pitch * g_skeleton->joints[g_selected_joint].local_to_parent.rotation_;
                g_skeleton->joints[g_selected_joint].local_to_parent.rotation_.Normalize();
                g_skeleton->UpdateChain();
            }
        }
        // Second dibs to camera
        else if (g_selected_mesh == nullptr)
        {
            // Camera rotation
            if (abs(i_delta_x) > 0.0f)
            {
                const engine::math::Quaternion yaw(i_delta_x * rotationDamping, engine::math::Vec3D::UNIT_Y);
                g_camera_transform.rotation_ = yaw * g_camera_transform.rotation_;
                g_camera_transform.rotation_.Normalize();
            }

            //if (abs(i_delta_y) > 0.0f)
            //{
            //    const engine::math::Quaternion pitch(i_delta_y * rotationDamping, engine::math::Vec3D::UNIT_X);
            //    g_camera_transform.rotation_ = pitch * g_camera_transform.rotation_;
            //    g_camera_transform.rotation_.Normalize();
            //}
        }
    }
}

void UpdatePositionBasedOnInput(float i_delta_x, float i_delta_y, float i_delta_mouse_z)
{
    if (g_right_mouse_button_pressed)
    {
        static constexpr float movementDamping = 0.05f;

        g_camera_transform.position_.x_ += i_delta_x * movementDamping;
        g_camera_transform.position_.y_ += i_delta_y * -movementDamping;
    }

    g_camera_transform.position_.z_ += i_delta_mouse_z;
}

void Reset()
{
    g_target_mesh.GetTransform().position_.x_ = -30.0f;
    g_target_mesh.GetTransform().position_.y_ = 30.0f;

    InitSkeletonTransforms();
    g_skeleton->UpdateChain();
}

void HandleMouseDown()
{
    bool click_consumed = false;

    const cy::Matrix4f screen = g_projection * g_view;
    const engine::math::Vec2D mouse_screen_space2d(g_curr_mouse_screen_space_x, g_curr_mouse_screen_space_y);

    if (g_selected_mesh != nullptr)
    {
        click_consumed = TestMeshesForSelection(mouse_screen_space2d, screen);
    }
}

void HandleMouseUp()
{
    const cy::Matrix4f screen = g_projection * g_view;
    const engine::math::Vec2D mouse_screen_space2d(g_curr_mouse_screen_space_x, g_curr_mouse_screen_space_y);

    bool click_consumed = false;

    // Don't do any selection if the mouse was being dragged
    if (!(g_left_mouse_dragging ||
        g_right_mouse_dragging))
    {
        click_consumed = TestMeshesForSelection(mouse_screen_space2d, screen);
    }
}

void HandleMouseDrag()
{
    // Translate/rotate the selected mesh
    // Meshes can only be transformed using the left mouse button
    if (g_left_mouse_dragging &&
        g_selected_mesh != nullptr)
    {
        cy::Matrix4f view;
        cy::Matrix4f::GetMatrixFromTransform(view, g_camera_transform);

        const cy::Point4f camera_forward = view.GetColumn(2);
        const cy::Point4f camera_right = view.GetColumn(0);

        const float delta_mouse_x = g_curr_mouse_screen_space_x - g_prev_mouse_screen_space_x;
        const float delta_mouse_y = g_curr_mouse_screen_space_y - g_prev_mouse_screen_space_y;

        constexpr float input_multiplier = 40.0f;
        g_selected_mesh->GetTransform().position_.x_ += camera_right.x * delta_mouse_x * input_multiplier;
        g_selected_mesh->GetTransform().position_.y_ += delta_mouse_y * -input_multiplier;
        g_selected_mesh->GetTransform().position_.z_ += camera_right.z * delta_mouse_x * -input_multiplier;

        g_target_updated = g_selected_mesh == &g_target_mesh;

        //cy::Matrix4f model;
        //cy::Matrix4f::GetMatrixFromTransform(model, g_selected_mesh->GetTransform());

        //const cy::Point3f camera_to_mesh_world_space = model.GetTrans() - view.GetTrans();
        //LOG("CameraToMesh:%f", camera_to_mesh_world_space.Length());
    }
}

void SolveFABRIK()
{
    // Reset to bind pose
    g_skeleton->ResetToCachedPose();
    g_skeleton->UpdateChain();
    g_skeleton->UpdateJointWorldSpacePositions();

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
#if 0
    LOG("NumIterations:%d", num_iterations);
#endif
}

void PrintMatrix(const cy::Matrix4f& i_matrix)
{
    LOG("%f %f %f %f", i_matrix.data[0], i_matrix.data[1], i_matrix.data[2], i_matrix.data[3]);
    LOG("%f %f %f %f", i_matrix.data[4], i_matrix.data[5], i_matrix.data[6], i_matrix.data[7]);
    LOG("%f %f %f %f", i_matrix.data[8], i_matrix.data[9], i_matrix.data[10], i_matrix.data[11]);
    LOG("%f %f %f %f", i_matrix.data[12], i_matrix.data[13], i_matrix.data[14], i_matrix.data[15]);
}

bool TestMeshesForSelection(const engine::math::Vec2D& i_mouse_screen_space2d, const cy::Matrix4f& i_screen)
{
    bool click_consumed = false;

    // Deselect the previously selected mesh, if any
    if (g_selected_mesh != nullptr)
    {
        g_selected_mesh->SetIsSelected(false);
        g_selected_mesh = false;
        g_selected_joint = MAX_JOINT_INDEX;
    }

    // Offer the target mesh first dibs
    if (g_target_mesh.TestMouseClick(i_mouse_screen_space2d, i_screen))
    {
        click_consumed = true;
        g_selected_mesh = &g_target_mesh;
    }
    // Joint meshes get second dibs
    else
    {
        for (uint8_t i = 0; i < g_skeleton->num_joints; ++i)
        {
            if (TestPointForSelection(g_skeleton->local_to_world_transforms[i].GetTrans(), i_screen))
            {
                g_joint_meshes[i].SetIsSelected(true);
                click_consumed = true;
                g_selected_mesh = &g_joint_meshes[i];
                g_selected_joint = i;
                break;
            }
        }
    }

    return click_consumed;
}

bool TestPointForSelection(const cy::Point3f& i_point_world_space, const cy::Matrix4f& i_screen)
{
    const cy::Point4f point_screen_space = i_screen * i_point_world_space;
    const cy::Point2f mouse_screen_space2d(g_curr_mouse_screen_space_x, g_curr_mouse_screen_space_y);
    const cy::Point2f mesh_screen_space2d(point_screen_space.x / point_screen_space.w, -point_screen_space.y / point_screen_space.w);
    const cy::Point2f delta = mesh_screen_space2d - mouse_screen_space2d;
    return delta.LengthSquared() < SELECTION_RADIUS;
}
