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
static float countdown;
static float countdownMax;
static float timeToFullOpacity;
static float opacityCount;
static int countdownOpacity = 255;
static bool shouldStartCountdown = false;
static float startTime;
static float enemyShootTime;

static GameScreen gameScreen;

// Assets
static Texture2D grassImg;


//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
static void UpdateDrawFrame(void);      // Update and Draw one frame
static void UpdateGameLoop(float);

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
    countdownMax = (float)GetRandomValue(6, 12);
    countdown = countdownMax;
    timeToFullOpacity = 4.0f;
    opacityCount = countdownMax - 1.0f;
    enemyShootTime = (countdownMax - 1) + ((float)GetRandomValue(0, 10000) / 10000.0f) * 2.0f;
    grassImg = LoadTexture("resources/grass.png");
    gameScreen = SCREEN_TITLE;
    
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
    UnloadTexture(grassImg);
    
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

    switch (gameScreen)
    {
        case SCREEN_TITLE:
            if (IsKeyReleased(KEY_SPACE))
            {
                gameScreen = SCREEN_GAMEPLAY;
            }
            break;
        case SCREEN_GAMEPLAY:
            UpdateGameLoop(dt);
            break;
        case SCREEN_ENDING:
            break;
        default:
            LOG("default\n");
            break;
    }
    
    //----------------------------------------------------------------------------------

    // Draw
    //----------------------------------------------------------------------------------
    // Render game screen to a texture, 
    // it could be useful for scaling or further shader postprocessing
    BeginTextureMode(target);
        ClearBackground(ORANGE);
        
        // TODO: Draw your game screen here
        DrawRectangle(0, 96, 128, 32, BLACK);
        DrawTexture(grassImg, 0, 82, WHITE);
        if (shouldStartCountdown)
        {
            DrawText(TextFormat("%d", (int)ceil(countdown)), virtualWidth / 2 - MeasureText(TextFormat("%d", (int)ceil(countdown)), 20) / 2, 20, 20, (Color){0,0,0,countdownOpacity});
        }
        
    EndTextureMode();
    
    // Render to screen (main framebuffer)
    BeginDrawing();
        ClearBackground(RAYWHITE);
        
        // Draw render texture to screen, scaled if required
        DrawTexturePro(target.texture, (Rectangle){ 0, 0, (float)target.texture.width, -(float)target.texture.height }, 
            (Rectangle){ 0, 0, (float)screenWidth, (float)screenHeight }, (Vector2){ 0, 0 }, 0.0f, WHITE);

        // TODO: Draw everything that requires to be drawn at this point, maybe UI?
        switch (gameScreen)
        {
        case SCREEN_TITLE:
            DrawText("STANDOFF", screenWidth / 2 - MeasureText("STANDOFF", 100) / 2, screenHeight / 2 - 100 * 2, 100, BLACK);
            DrawText("press SPACE to play", screenWidth / 2 - MeasureText("press SPACE to start", 40) / 2, screenHeight / 2 - 90, 40, BLACK);
            DrawText("made by mysyq", screenWidth / 2 - MeasureText("made by mysyq", 40) / 2, 105 * windowScale, 40, ORANGE);
            DrawText("music by MurMurich", screenWidth / 2 - MeasureText("music by MurMurich", 40) / 2, 113 * windowScale, 40, ORANGE);
            break;

        case SCREEN_GAMEPLAY:
            if (!shouldStartCountdown)
            {
                DrawText("hold SPACE to countdown", screenWidth / 2 - MeasureText("hold SPACE to countdown", 40) / 2, screenHeight / 4, 40, BLACK);
            }

        default:
            break;
        }

    EndDrawing();
    //----------------------------------------------------------------------------------  
}

void UpdateGameLoop(float dt)
{
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

        float playerTimeDif = fabsf(countdownMax - playerShootTime);
        float enemyTimeDif = fabsf(countdownMax - enemyShootTime);
        LOG("player time difference: %f\n", playerTimeDif);
        LOG("enemy time dif: %f\n", enemyTimeDif);

        gameScreen = SCREEN_ENDING;
    }
    
    if (shouldStartCountdown)
    {
        countdown -= dt;
        if (countdown < opacityCount)
        {
            countdownOpacity -= 70;
            if (countdownOpacity < 0) countdownOpacity = 0;
            opacityCount--;
        }
    }
   
    frameCounter++;
}
