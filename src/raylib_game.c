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

typedef enum {
    STANDING,
    SHOT,
    FALL,
} CowboyState;

// TODO: Define your custom data types here
typedef struct Animation {
    int firstFrame;
    int lastFrame;
    int frameSpeed;
    bool isLooping;
} Animation;

typedef struct AnimationComponent {
    Animation animation;
    int frameCounter;
    int currentFrame;
} AnimationComponent;

typedef struct Cowboy {
    Vector2 position;
    Rectangle frameRec;
    CowboyState state;
    Animation animations[3];
    int frameCounter;
    int currentFrame;
    float shootTime;
} Cowboy;

typedef struct Entity {
    int id;
} Entity;

//----------------------------------------------------------------------------------
// Global Variables Definition (local to this module)
//----------------------------------------------------------------------------------
static const int virtualWidth = 128;
static const int virtualHeight = 128;
static const int windowScale = 6;
static const int screenWidth = virtualWidth * windowScale;
static const int screenHeight = virtualHeight * windowScale;

static RenderTexture2D target = { 0 };  // Render texture to render our game

// TODO: Define global variables here, recommended to make them static
static float countdown;
static float countdownMax;
static bool isCountingDown;

static float timeToFullOpacity;
static float opacityCount;
static int countdownOpacity;

static float startTime;
static float timer;

static float playerTimeDif;
static float enemyTimeDif;

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
static Cowboy player;
static Cowboy enemy;

static Sound music;
static Sound victorySound;
static Sound defeatSound;
static Sound shootSound1;
static Sound shootSound2;

static Animation standingAnim;
static Animation shotAnim;
static Animation fallAnim;

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
static void UpdateDrawFrame(void);      // Update and Draw one frame
static void UpdateGameLoop(float);
static void ResetGame(void);
static void AnimateGrass(void);
static void AnimateCowboy(Cowboy* cowboy);

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
    timeToFullOpacity = 4.0f; // Not opacity, but transparency 
    gameScreen = SCREEN_TITLE;

    // Grass init
    grassSheet = LoadTexture("resources/grassSheet.png");
    grassPosition = (Vector2){0, 80};
    grassFramesCounter = 0;
    grassFramesSpeed = 2;
    grassGoesBack = false;
    grassCurrentFrame = 0;
    grassFrameRec = (Rectangle){0, 0, grassSheet.width, grassSheet.height/3.0f};

    standingAnim = (Animation){0, 1, 2, true};
    shotAnim = (Animation){2, 3, 40, false};
    fallAnim = (Animation){4, 7, 2, false};

    // Cowboys init
    cowboysSheet = LoadTexture("resources/cowboysSheet.png");
    player.position = (Vector2){17,64};
    player.frameRec = (Rectangle){0, 0, 48, cowboysSheet.height/8.0f};
    player.animations[0] = standingAnim;
    player.animations[1] = shotAnim;
    player.animations[2] = fallAnim;

    enemy.position = (Vector2){65, 64};
    enemy.frameRec = (Rectangle){48, 0, 42, cowboysSheet.height/8.0f};
    enemy.animations[0] = standingAnim;
    enemy.animations[1] = shotAnim;
    enemy.animations[2] = fallAnim;

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
            AnimateCowboy(&player);
            AnimateCowboy(&enemy);
            break;

        case SCREEN_ENDING:
            AnimateCowboy(&player);
            AnimateCowboy(&enemy);
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

        if (gameScreen != SCREEN_TITLE)
        {
            DrawTextureRec(cowboysSheet, player.frameRec, player.position, WHITE);
            DrawTextureRec(cowboysSheet, enemy.frameRec, enemy.position, WHITE);
        }

        if (isCountingDown)
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
            if (!isCountingDown && player.state != SHOT)
            {
                DrawText("hold SPACE to countdown", screenWidth / 2 - MeasureText("hold SPACE to countdown", 40) / 2, screenHeight / 4, 40, BLACK);
            }
            break;
        
        case SCREEN_ENDING:
            if (playerTimeDif > 0) DrawText(TextFormat("+%.3f", playerTimeDif), 100, screenHeight / 4, 40, BLACK);
            else DrawText(TextFormat("%.3f", playerTimeDif), 100, screenHeight / 4, 40, BLACK);
            if (enemyTimeDif > 0) DrawText(TextFormat("+%.3f", enemyTimeDif), 520, screenHeight / 4, 40, BLACK);
            else DrawText(TextFormat("%.3f", enemyTimeDif), 520, screenHeight / 4, 40, BLACK);
            DrawText("press SPACE to restart", screenWidth / 2 - MeasureText("press SPACE to restart", 40) / 2, screenHeight / 7, 40, BLACK);

        default:
            break;
        }

    EndDrawing();
    //----------------------------------------------------------------------------------  
}

void UpdateGameLoop(float dt)
{
    if (IsKeyPressed(KEY_SPACE) && !isCountingDown)
    {
        isCountingDown = true;
        startTime = GetTime();
        LOG("Start time: %f\n", startTime);
    }

    if (IsKeyReleased(KEY_SPACE) && player.state != SHOT)
    {
        player.shootTime = GetTime() - startTime;
        
        PlaySound(shootSound2);
        player.state = SHOT;
        LOG("Player SHOT\n");
    }
    
    if (isCountingDown)
    {
        countdown -= dt;
        if (countdown < opacityCount)
        {
            countdownOpacity -= 70;
            if (countdownOpacity < 0) countdownOpacity = 0;
            opacityCount--; // WHY?
        }

        if (enemy.state != SHOT)
        {
            if (enemy.shootTime <= (float)GetTime() - startTime)
            {
                enemy.state = SHOT;
                LOG("ENEMY SHOT\n");
                PlaySound(shootSound1);
            }
        }
    }

    if (player.state == SHOT && enemy.state == SHOT)
    {
        timer += dt;
        isCountingDown = false;
        if (timer >= 2.0f)
        {
            LOG("Player shoot time: %f\n", player.shootTime);
            LOG("Enemy shoot time: %f\n", enemy.shootTime);

            playerTimeDif = -(countdownMax - player.shootTime);
            enemyTimeDif = -(countdownMax - enemy.shootTime);
            LOG("player time difference: %f\n", playerTimeDif);
            LOG("enemy time dif: %f\n", enemyTimeDif);
            
            bool isPlayerWinner = (fabsf(playerTimeDif) < fabsf(enemyTimeDif)) ? true : false;
            if (isPlayerWinner) 
            {
                PlaySound(victorySound);
                enemy.state = FALL;
            }
            else 
            {
                player.state = FALL;
                PlaySound(defeatSound);
            }

            LOG("Switch to SCREEN_ENDING\n");
            gameScreen = SCREEN_ENDING;
        }
    }
}

void ResetGame(void)
{
    countdownMax = (float)GetRandomValue(6, 12);
    countdown = countdownMax;
    enemy.shootTime = (countdownMax - 1) + ((float)GetRandomValue(0, 10000) / 10000.0f) * 2.0f;

    countdownOpacity = 255;
    opacityCount = countdownMax - 1.0f;

    player.state = STANDING;
    player.frameCounter = 0;

    enemy.state = STANDING;
    enemy.frameCounter = 0;

    isCountingDown = false;
    timer = 0.0f;
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

void AnimateCowboy(Cowboy* cowboy)
{
    cowboy->frameCounter++;

    if (cowboy->frameCounter >= 60/cowboy->animations[cowboy->state].frameSpeed)
    {
        cowboy->currentFrame++;

        if (cowboy->currentFrame > cowboy->animations[cowboy->state].lastFrame)
        {
            if (cowboy->animations[cowboy->state].isLooping)
            {
                cowboy->currentFrame = cowboy->animations[cowboy->state].firstFrame;
            }
            else
            {
                cowboy->currentFrame = cowboy->animations[cowboy->state].lastFrame;
            }
        }

        cowboy->frameCounter = 0;
    }

    cowboy->frameRec.y = (float)cowboy->currentFrame*(float)cowboysSheet.height/8;
}
