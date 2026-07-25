/*******************************************************************************************
*
*   raylib gamejam template
*
*   Code licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2022-2026 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>      // Emscripten library
#endif

#include <stdio.h>                          // Required for: printf()
#include <stdlib.h>                         // Required for: 
#include <string.h>                         // Required for:
#include <math.h>

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
// Simple log system to avoid printf() calls if required
// NOTE: Avoiding those calls, also avoids const strings memory usage
#define SUPPORT_LOG_INFO
#if defined(SUPPORT_LOG_INFO)
    #define LOG(...) printf(__VA_ARGS__)
#else
    #define LOG(...)
#endif

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef enum { 
    SCREEN_LOGO = 0, 
    SCREEN_TITLE, 
    SCREEN_GAMEPLAY, 
    SCREEN_ENDING
} GameScreen;

// TODO: Define your custom data types here

//----------------------------------------------------------------------------------
// Global Variables Definition (local to this module)
//----------------------------------------------------------------------------------
static const int virtualWidth = 128;
static const int virtualHeight = 128;
static const int windowScale = 6;
static const int screenWidth = virtualWidth * windowScale;
static const int screenHeight = virtualHeight * windowScale;

static RenderTexture2D target = { 0 };  // Render texture to render our game
static int frameCounter = 0;

// TODO: Define global variables here, recommended to make them static
static float countdown = 10.0f;
static bool shouldStartCountdown = false;
static float startTime;
static float enemyShootTime;


//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
static void UpdateDrawFrame(void);      // Update and Draw one frame

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
#if !defined(_DEBUG)
    SetTraceLogLevel(LOG_NONE);         // Disable raylib trace log messages
#endif

    // Initialization
    //--------------------------------------------------------------------------------------
    InitWindow(screenWidth, screenHeight, "raylib gamejam template");
    
    // TODO: Load resources / Initialize variables at this point
    enemyShootTime = 9.0f + ((float)GetRandomValue(0, 10000) / 10000.0f) * 2.0f;
    
    // Render texture to draw, enables screen scaling
    // NOTE: If screen is scaled, mouse input should be scaled proportionally
    target = LoadRenderTexture(virtualWidth, virtualHeight);
    SetTextureFilter(target.texture, TEXTURE_FILTER_POINT);

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 60, 1);
#else
    SetTargetFPS(60);     // Set our game frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button
    {
        UpdateDrawFrame();
    }
#endif

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadRenderTexture(target);
    
    // TODO: Unload all loaded resources at this point

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

//--------------------------------------------------------------------------------------------
// Module Functions Definition
//--------------------------------------------------------------------------------------------
// Update and draw frame
void UpdateDrawFrame(void)
{
    // Update
    //----------------------------------------------------------------------------------
    // TODO: Update variables / Implement example logic at this point
    float dt = GetFrameTime();

    if (IsKeyPressed(KEY_SPACE))
    {
        shouldStartCountdown = true;
        startTime = GetTime();
        LOG("Start time: %f\n", startTime);
    }

    if (IsKeyReleased(KEY_SPACE))
    {
        shouldStartCountdown = false;
        float shootTime = GetTime();
        float playerShootTime = shootTime - startTime;
        LOG("Shoot time: %f\n", shootTime);
        LOG("Player shoot time: %f\n", playerShootTime);
        LOG("Enemy shoot time: %f\n", enemyShootTime);

        float playerTimeDif = fabsf(10.0f - playerShootTime);
        float enemyTimeDif = fabsf(10.0f - enemyShootTime);
        LOG("player time difference: %f\n", playerTimeDif);
        LOG("enemy time dif: %f\n", enemyTimeDif);

        if (playerTimeDif < enemyTimeDif)
        {
            // change gamestate
        }
    }
    
    if (shouldStartCountdown)
    {
        countdown -= dt;
    }
   
    frameCounter++;
    //----------------------------------------------------------------------------------

    // Draw
    //----------------------------------------------------------------------------------
    // Render game screen to a texture, 
    // it could be useful for scaling or further shader postprocessing
    BeginTextureMode(target);
        ClearBackground(RAYWHITE);
        
        // TODO: Draw your game screen here
        DrawRectangle(2, 2, 124, 124, RED);
        if (shouldStartCountdown)
        {
            DrawText(TextFormat("%d", (int)ceil(countdown)), 50, 50, 20, BLACK);
        }
        
    EndTextureMode();
    
    // Render to screen (main framebuffer)
    BeginDrawing();
        ClearBackground(RAYWHITE);
        
        // Draw render texture to screen, scaled if required
        DrawTexturePro(target.texture, (Rectangle){ 0, 0, (float)target.texture.width, -(float)target.texture.height }, 
            (Rectangle){ 0, 0, (float)screenWidth, (float)screenHeight }, (Vector2){ 0, 0 }, 0.0f, WHITE);

        // TODO: Draw everything that requires to be drawn at this point, maybe UI?

    EndDrawing();
    //----------------------------------------------------------------------------------  
}
