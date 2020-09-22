#include <stdio.h>
#include <raylib.h>
#include <raymath.h>

#ifdef OS_WEB
#include <emscripten/emscripten.h>
#endif

#define WIDTH 800
#define HEIGHT 600

#if defined(OS_WEB)
#define GLSL_VERSION            100
#else   // PLATFORM_WEB
#define GLSL_VERSION            330
#endif

#define RLIGHTS_IMPLEMENTATION
#include "rlights.h"

Camera3D camera = {0};
Camera3D camera_shadow_map = {0};
Vector3 cube_position = {0.0f, 1.0f, 0.0f};
Model quad = {0};
Model cube = {0};
RenderTexture2D render_texture = {0};
Vector3 phase = {0};
Shader shader = {0};
Shader shader_default = {0};

void shader_init(){
    shader = LoadShader(FormatText("./assets/shaders/glsl%i/base_lighting.vs", GLSL_VERSION),
                            FormatText("./assets/shaders/glsl%i/lighting.fs", GLSL_VERSION));
    shader.locs[LOC_MATRIX_MODEL] = GetShaderLocation(shader, "matModel");
    shader.locs[LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");

    int ambientLoc = GetShaderLocation(shader, "ambient");
    SetShaderValue(shader, ambientLoc, (float[4]){ 0.2f, 0.2f, 0.2f, 1.0f }, UNIFORM_VEC4);

    Light light_1 = CreateLight(LIGHT_POINT, (Vector3){ 0, 15.f, 0}, Vector3Zero(), PURPLE, shader);
    // Light light_1 = CreateLight(LIGHT_POINT, (Vector3){ -105, 5, -105 }, Vector3Zero(), RED, shader);
    // Light light_2 = CreateLight(LIGHT_POINT, (Vector3){ -105, 5, 105 }, Vector3Zero(), BLUE, shader);
    // Light light_3 = CreateLight(LIGHT_POINT, (Vector3){ 105, 5, -105 }, Vector3Zero(), PURPLE, shader);
    UpdateLightValues(shader, light_1);
    // UpdateLightValues(shader, light_2);
    // UpdateLightValues(shader, light_3);

}

void update_frame()
{
    phase = Vector3Add(phase, (Vector3){0.01f, 0.02f, 0.03f});
    cube.transform = MatrixRotateXYZ(phase);
    UpdateCamera(&camera);

    cube.materials[0].shader = shader_default;

    BeginTextureMode(render_texture);{
        ClearBackground(GRAY);
        BeginMode3D(camera_shadow_map);
        {
            DrawModel(cube, cube_position, 1.f, DARKGRAY);
            DrawCubeWires((Vector3){2.f, 2.f, 1.f}, 1, 1, 1, DARKGRAY);
        }
        EndMode3D();
    }EndTextureMode();

    cube.materials[0].shader = shader;

    BeginDrawing();
    {

        ClearBackground(WHITE);
        DrawFPS(10, 10);

        BeginMode3D(camera);
        {
            DrawModel(cube, cube_position, 1.f, RED);
            DrawCubeWires((Vector3){2.f, 2.f, 1.f}, 1, 1, 1, BLUE);
            DrawModel(quad, (Vector3){0, -1.f, 0}, 1.f, GREEN);
        }
        EndMode3D();

        if (IsKeyDown(KEY_KP_ADD))
            camera.fovy += 1.0f;
        if (IsKeyDown(KEY_KP_SUBTRACT))
            camera.fovy -= 1.0f;

        if (IsKeyPressed(KEY_LEFT))
            cube_position.x -= 1.0f;
        if (IsKeyPressed(KEY_RIGHT))
            cube_position.x += 1.0f;
        if (IsKeyPressed(KEY_UP))
            cube_position.z -= 1.0f;
        if (IsKeyPressed(KEY_DOWN))
            cube_position.z += 1.0f;
    }
    EndDrawing();
}

int main(void)
{
#ifdef OS_Windows_NT
    printf("Windows dettected\n");
#elif defined OS_Linux
    printf("LINUS dettected\n");
#elif defined OS_Darwin
    printf("MacOS dettected\n");
#endif

    InitWindow(WIDTH, HEIGHT, "This is a dynamic shadow test");
    SetTargetFPS(60);

    shader_init();

    camera.fovy = 45.0f;
    camera.target = (Vector3){.0f, .0f, .0f};
    camera.position = (Vector3){0.0f, 10.0f, 10.0f};
    camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    camera.type = CAMERA_PERSPECTIVE;
    SetCameraMode(camera, CAMERA_ORBITAL);

    camera_shadow_map.fovy = 45.0f;
    camera_shadow_map.target = (Vector3){.0f, .0f, .0f};
    camera_shadow_map.position = (Vector3){0.0f, 10.0f, 0.0f};
    camera_shadow_map.up = (Vector3){0.0f, 0.0f,-1.0f};
    camera_shadow_map.type = CAMERA_PERSPECTIVE;


    cube = LoadModelFromMesh(GenMeshTorus(.3f, 2.f, 20, 20));
    shader_default = cube.materials[0].shader;
    Mesh plane_mesh = GenMeshCube(10.f, .1f, 10.f);
    quad = LoadModelFromMesh(plane_mesh);
    render_texture = LoadRenderTexture(160, 100);

    quad.materials[0].maps[MAP_DIFFUSE].texture = render_texture.texture;

#ifdef OS_WEB
    emscripten_set_main_loop(update_frame, 0, 1);
#else
    while (!WindowShouldClose())
    {
        update_frame();
    }
#endif
    UnloadModel(quad);
    UnloadModel(cube);
    CloseWindow();

    return 0;
}