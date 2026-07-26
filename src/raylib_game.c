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
static int countdownOpacity;

static bool shouldStartCountdown;
static float startTime;
static float enemyShootTime;
static float enemyCountdown;

static float playerTimeDif;
static float enemyTimeDif;
static bool didEnemyShoot;
static bool isPlayerWinner;

static float timer;
static bool didPlayerShoot;

static float shootTime;
static float playerShootTime;

static GameScreen gameScreen;

// Assets
static Texture2D grassSheet;
static Vector2 grassPosition;
static int grassFramesCounter;
static int grassFramesSpeed;
static bool grassGoesBack;
static int grassCurrentFrame;
static Rectangle grassFrameRec;


static Texture2D cowboysSheet;
static int cowboysStandingFrameSpeed;
static int cowboysShootingFrameSpeed;
static int cowboysFallingFrameSpeed;

static Vector2 cowboyLeftPosition;
static Rectangle cowboyLeftFrameRec;
static int cowboyLeftCurrentFrame;
static int cowboyLeftFrameCounter;

static Vector2 cowboyRightPosition;
static Rectangle cowboyRightFrameRec;
static int cowboyRightCurrentFrame;
static int cowboyRightFrameCounter;


static Sound music;
static Sound victorySound;
static Sound defeatSound;
static Sound shootSound1;
static Sound shootSound2;


//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
static void UpdateDrawFrame(void);      // Update and Draw one frame
static void UpdateGameLoop(float);
static void ResetGame(void);
static void AnimateGrass(void);
static void AnimateCowboys(void);

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
    InitWindow(screenWidth, screenHeight, "STANDOFF");
    InitAudioDevice();
    
    // TODO: Load resources / Initialize variables at this point
    ResetGame();
    timeToFullOpacity = 4.0f;
    gameScreen = SCREEN_TITLE;

    grassSheet = LoadTexture("resources/grassSheet.png");
    grassPosition = (Vector2){0, 80};
    grassFramesCounter = 0;
    grassFramesSpeed = 2;
    grassGoesBack = false;
    grassCurrentFrame = 0;
    grassFrameRec = (Rectangle){0, 0, grassSheet.width, grassSheet.height/3};

    cowboysSheet = LoadTexture("resources/cowboysSheet.png");
    cowboyLeftPosition = (Vector2){17,64};
    cowboyRightPosition = (Vector2){65, 64};
    cowboyLeftFrameCounter = 0;
    cowboyRightFrameCounter = 0;
    cowboysStandingFrameSpeed = 2;
    cowboyLeftCurrentFrame = 0;
    cowboyRightCurrentFrame = 0;
    cowboyLeftFrameRec = (Rectangle){0, 0, 48, cowboysSheet.height/8};
    cowboyRightFrameRec = (Rectangle){48, 0, 42, cowboysSheet.height/8};

    music = LoadSound("resources/music.mp3");
    victorySound = LoadSound("resources/victory.mp3");
    defeatSound = LoadSound("resources/defeat.mp3");
    shootSound1 = LoadSound("resources/explosion1.wav");
    shootSound2 = LoadSound("resources/explosion2.wav");

    PlaySound(music);
    
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
    UnloadTexture(grassSheet);
    UnloadTexture(cowboysSheet);

    UnloadSound(music);
    UnloadSound(victorySound);
    UnloadSound(defeatSound);
    UnloadSound(shootSound1);
    UnloadSound(shootSound2);

    CloseAudioDevice();
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

    AnimateGrass();
    AnimateCowboys();

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
            if (IsKeyReleased(KEY_SPACE))
            {
                ResetGame();
                gameScreen = SCREEN_GAMEPLAY;
            }
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
        DrawTextureRec(grassSheet, grassFrameRec, grassPosition, WHITE);

        // DrawTexture(cowboyR1, 21, 64, WHITE);
        // DrawTexture(cowboyL1, 88, 64, WHITE);

        if (gameScreen != SCREEN_TITLE)
        {
            DrawTextureRec(cowboysSheet, cowboyLeftFrameRec, cowboyLeftPosition, WHITE);
            DrawTextureRec(cowboysSheet, cowboyRightFrameRec, cowboyRightPosition, WHITE);
        }

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
            if (!shouldStartCountdown && !didPlayerShoot)
            {
                DrawText("hold SPACE to countdown", screenWidth / 2 - MeasureText("hold SPACE to countdown", 40) / 2, screenHeight / 4, 40, BLACK);
            }
            break;
        
        case SCREEN_ENDING:
            if (playerTimeDif > 0) DrawText(TextFormat("+%.3f", playerTimeDif), 100, screenHeight / 4, 40, BLACK);
            else DrawText(TextFormat("%.3f", playerTimeDif), 100, screenHeight / 4, 40, BLACK);
            if (enemyTimeDif > 0) DrawText(TextFormat("+%.3f", enemyTimeDif), 520, screenHeight / 4, 40, BLACK);
            else DrawText(TextFormat("%.3f", enemyTimeDif), 520, screenHeight / 4, 40, BLACK);

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
        shootTime = GetTime();
        playerShootTime = shootTime - startTime;
        

        PlaySound(shootSound2);
        didPlayerShoot = true;
        LOG("Player SHOT\n");
    }

    if (didPlayerShoot && didEnemyShoot)
    {
        timer += dt;
        shouldStartCountdown = false;
        if (timer >= 2.0f)
        {
            LOG("Shoot time: %f\n", shootTime);
            LOG("Player shoot time: %f\n", playerShootTime);
            LOG("Enemy shoot time: %f\n", enemyShootTime);

            playerTimeDif = countdownMax - playerShootTime;
            enemyTimeDif = countdownMax - enemyShootTime;
            isPlayerWinner = (fabsf(playerTimeDif) < fabsf(enemyTimeDif)) ? true : false;
            LOG("player time difference: %f\n", playerTimeDif);

            LOG("enemy time dif: %f\n", enemyTimeDif);
            if (isPlayerWinner) PlaySound(victorySound);
            else PlaySound(defeatSound);
            LOG("Switch to SCREEN_ENDING\n");
            gameScreen = SCREEN_ENDING;
        }
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

        if (!didEnemyShoot)
        {
            enemyCountdown -= dt;
            if (enemyCountdown <= 0.0f)
            {
                LOG("ENEMY SHOT\n");
                PlaySound(shootSound1);
                didEnemyShoot = true;
            }
        }
    }
   
    frameCounter++;
}

void ResetGame(void)
{
    countdownMax = (float)GetRandomValue(6, 12);
    countdown = countdownMax;
    enemyShootTime = (countdownMax - 1) + ((float)GetRandomValue(0, 10000) / 10000.0f) * 2.0f;
    enemyCountdown = enemyShootTime;
    didEnemyShoot = false;
    countdownOpacity = 255;
    opacityCount = countdownMax - 1.0f;

    shouldStartCountdown = false;
    timer = 0.0f;

    isPlayerWinner = false;
    didPlayerShoot = false;
}

void AnimateGrass(void)
{
    grassFramesCounter++;
    if (grassFramesCounter >= (60/grassFramesSpeed))
    {
        grassFramesCounter = 0;

        if (grassGoesBack) grassCurrentFrame--;
        else grassCurrentFrame++;

        if (grassCurrentFrame == 2) grassGoesBack = true;
        else if (grassCurrentFrame == 0) grassGoesBack = false;
        

        grassFrameRec.y = (float)grassCurrentFrame*(float)grassSheet.height/3;
    }
}

void AnimateCowboys(void)
{
    cowboyLeftFrameCounter++;
    cowboyRightFrameCounter++;

    if (didPlayerShoot)
    {
        cowboyLeftCurrentFrame = 2;
        if (cowboyLeftFrameCounter >= (60/cowboysShootingFrameSpeed))
        {
            cowboyLeftFrameCounter = 0;
            cowboyLeftCurrentFrame = 3;
        }
    }

    if (didEnemyShoot)
    {
        cowboyRightCurrentFrame = 2;
        if (cowboyRightFrameCounter >= (60/cowboysShootingFrameSpeed))
        {
            cowboyRightFrameCounter = 0;
            cowboyRightCurrentFrame = 3;
        }
    }

    if (cowboyLeftFrameCounter >= (60/cowboysStandingFrameSpeed))
    {
        cowboyLeftFrameCounter = 0;
        cowboyRightFrameCounter = 0;

        cowboyLeftCurrentFrame++;
        cowboyRightCurrentFrame++;

        if (cowboyLeftCurrentFrame > 1 && cowboyRightCurrentFrame > 1)
        {
            cowboyLeftCurrentFrame = 0;
            cowboyRightCurrentFrame = 0;
        }
    }

    cowboyLeftFrameRec.y = (float)cowboyLeftCurrentFrame*(float)cowboysSheet.height/8;
    cowboyRightFrameRec.y = (float)cowboyRightCurrentFrame*(float)cowboysSheet.height/8;
}