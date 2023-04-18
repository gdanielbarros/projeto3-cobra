/*******************************************************************************************
*
*   raylib - game: snake
*   Giovanna Andrade - Daniel Alves de Barros
*
********************************************************************************************/

#include "raylib.h"

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

// Some Defines
#define SNAKE_LENGTH   256
#define SQUARE_SIZE     31

// Types and Structures Definition
typedef struct Snake {
    Vector2 position;
    Vector2 size;
    Vector2 speed;
    Color color;
} Snake;

typedef struct Food {
    Vector2 position;
    Vector2 size;
    bool active;
    Color color;
} Food;

// Global Variables Declaration
static const int screenWidth = 800;
static const int screenHeight = 450;

static int framesCounter = 0;
static bool gameOver = false;
static bool pause = false;

static Food fruit = { 0 };
static Snake snake[SNAKE_LENGTH] = { 0 };
static Vector2 snakePosition[SNAKE_LENGTH] = { 0 };
static bool allowMove = false;
static Vector2 offset = { 0 };
static int counterTail = 0;
static Sound overSound;
static Sound eatSound;

// Module Functions Declaration (local)
static void InitGame(void);         // Initialize game
static void UpdateGame(void);       // Update game (one frame)
static void DrawGame(void);         // Draw game (one frame)
static void LoadGame(void);         // Load game
static void UnloadGame(void);       // Unload game
static void UpdateDrawFrame(void);  // Update and Draw (one frame)

// Program main entry point
int main(void)
{
    //Variables Declaration
    int option = 0;
    char *menuOptions[] = {"NOVO JOGO", "CARREGAR JOGO"};

    // Initialization (Note windowTitle is unused on Android)
    InitWindow(screenWidth, screenHeight, "classic game: snake");
    InitGame();
    LoadGame();
    
#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 60, 1);
#else
    SetTargetFPS(60);

    // Main game loop
    while (!WindowShouldClose())      // Detect window close button or ESC key
    {  
        DrawMenu(menuOptions,option); // Draw menu
        option = Menu(option);        // Get the user option of the menu 
        Game(option);                 // Game begin
    }
#endif
    // De-Initialization
    UnloadGame();                     // Unload loaded data (textures, sounds, models...)
    CloseWindow();                    // Close window and OpenGL context
    return 0;
}

// Module Functions Definitions (local)
// Initialize game variables
void InitGame(void)
{
    framesCounter = 0;
    gameOver = false;
    pause = false;

    counterTail = 1;
    allowMove = false;

    offset.x = screenWidth%SQUARE_SIZE;
    offset.y = screenHeight%SQUARE_SIZE;

    for (int i = 0; i < SNAKE_LENGTH; i++)
    {
        snake[i].position = (Vector2){ offset.x/2, offset.y/2 };
        if (i == 0) snake[i].size = (Vector2){ SQUARE_SIZE, SQUARE_SIZE };
        else snake[i].size = (Vector2){ SQUARE_SIZE, SQUARE_SIZE };
        snake[i].speed = (Vector2){ SQUARE_SIZE, 0 };

        if (i == 0) snake[i].color = DARKBLUE;
        else snake[i].color = BLUE;
    }

    for (int i = 0; i < SNAKE_LENGTH; i++)
    {
        snakePosition[i] = (Vector2){ 0.0f, 0.0f };
    }

    fruit.size = (Vector2){ SQUARE_SIZE, SQUARE_SIZE };
    fruit.color = RED;
    fruit.active = false;
}

// Update game (one frame)
void UpdateGame(void)
{
    if (!gameOver)
    {
        if (IsKeyPressed('P')) pause = !pause;

        if (!pause)
        {
            // Player control
            if (IsKeyPressed(KEY_RIGHT) && (snake[0].speed.x == 0) && allowMove)
            {
                snake[0].speed = (Vector2){ SQUARE_SIZE, 0 };
                allowMove = false;
            }
            if (IsKeyPressed(KEY_LEFT) && (snake[0].speed.x == 0) && allowMove)
            {
                snake[0].speed = (Vector2){ -SQUARE_SIZE, 0 };
                allowMove = false;
            }
            if (IsKeyPressed(KEY_UP) && (snake[0].speed.y == 0) && allowMove)
            {
                snake[0].speed = (Vector2){ 0, -SQUARE_SIZE };
                allowMove = false;
            }
            if (IsKeyPressed(KEY_DOWN) && (snake[0].speed.y == 0) && allowMove)
            {
                snake[0].speed = (Vector2){ 0, SQUARE_SIZE };
                allowMove = false;
            }

            // Snake movement
            for (int i = 0; i < counterTail; i++) snakePosition[i] = snake[i].position;

            if ((framesCounter%5) == 0)
            {
                for (int i = 0; i < counterTail; i++)
                {
                    if (i == 0)
                    {
                        snake[0].position.x += snake[0].speed.x;
                        snake[0].position.y += snake[0].speed.y;
                        allowMove = true;
                    }
                    else snake[i].position = snakePosition[i-1];
                }
            }

            // Wall behaviour
            if (((snake[0].position.x) > (screenWidth - offset.x)) ||
                ((snake[0].position.y) > (screenHeight - offset.y)) ||
                (snake[0].position.x < 0) || (snake[0].position.y < 0))
            {
                gameOver = true;
                PlaySound(overSound);
            }

            // Collision with yourself
            for (int i = 1; i < counterTail; i++)
            {
                if ((snake[0].position.x == snake[i].position.x) && (snake[0].position.y == snake[i].position.y))
                {
                    gameOver = true;
                    PlaySound(overSound);
                } 
            }

            // Fruit position calculation
            if (!fruit.active)
            {
                fruit.active = true;
                fruit.position = (Vector2){ GetRandomValue(0, (screenWidth/SQUARE_SIZE) - 1)*SQUARE_SIZE + offset.x/2, GetRandomValue(0, (screenHeight/SQUARE_SIZE) - 1)*SQUARE_SIZE + offset.y/2 };

                for (int i = 0; i < counterTail; i++)
                {
                    while ((fruit.position.x == snake[i].position.x) && (fruit.position.y == snake[i].position.y))
                    {
                        fruit.position = (Vector2){ GetRandomValue(0, (screenWidth/SQUARE_SIZE) - 1)*SQUARE_SIZE + offset.x/2, GetRandomValue(0, (screenHeight/SQUARE_SIZE) - 1)*SQUARE_SIZE + offset.y/2 };
                        i = 0;
                    }
                }
            }

            // Collision
            if ((snake[0].position.x < (fruit.position.x + fruit.size.x) && (snake[0].position.x + snake[0].size.x) > fruit.position.x) &&
                (snake[0].position.y < (fruit.position.y + fruit.size.y) && (snake[0].position.y + snake[0].size.y) > fruit.position.y))
            {
                snake[counterTail].position = snakePosition[counterTail - 1];
                counterTail += 1;
                fruit.active = false;
                PlaySound(eatSound);
                //score++;
            }
            framesCounter++;
        }
    }
    else
    {  
        if (IsKeyPressed(KEY_ENTER))
        {
            InitGame();
            gameOver = false;
        }
    }
}

void DrawMenu(char **menuOptions, int option)
{
    BeginDrawing();
        ClearBackground(BLACK);
        DrawText("SNAKE",230,130,100,RED);
        for (int i = 0; i < 2; i++)
        {
            if (i == option) DrawText(menuOptions[i], screenWidth / 2 - MeasureText(menuOptions[i], 20) / 2, 260 + i * 50, 20, YELLOW);
            else DrawText(menuOptions[i], screenWidth / 2 - MeasureText(menuOptions[i], 20) / 2, 260 + i * 50, 20, GRAY);
        }
    EndDrawing();
}

int Menu(int option)
{
    // Get user option
    if (IsKeyPressed(KEY_UP))
    {
        option--;
        if (option < 0) option = 1;
    }
    else if (IsKeyPressed(KEY_DOWN))
    {
        option++;
        if (option > 1) option = 0;
    }
    return option;
}

void Game(option)
{
    if(option==0&&IsKeyPressed(KEY_ENTER)) // if novo jogo it was select the game begin from the top
    {
        while(!WindowShouldClose())
        {
            // Update and Draw
            UpdateDrawFrame(); 
        }
    } 
    else if(option==1&&IsKeyPressed(KEY_ENTER)) // if carregar jogo it was select you can choose the saved game to continue
    {
        
    }
}
// Draw game (one frame)
void DrawGame(void)
{
    BeginDrawing();
        ClearBackground(BLACK);

        if (!gameOver)
        {
            // Draw grid lines
            for (int i = 0; i < screenWidth/SQUARE_SIZE + 1; i++)
            {
                DrawLineV((Vector2){SQUARE_SIZE*i + offset.x/2, offset.y/2}, (Vector2){SQUARE_SIZE*i + offset.x/2, screenHeight - offset.y/2}, LIGHTGRAY);
            }

            for (int i = 0; i < screenHeight/SQUARE_SIZE + 1; i++)
            {
                DrawLineV((Vector2){offset.x/2, SQUARE_SIZE*i + offset.y/2}, (Vector2){screenWidth - offset.x/2, SQUARE_SIZE*i + offset.y/2}, LIGHTGRAY);
            }

            // Draw snake
            for (int i = 0; i < counterTail; i++) DrawRectangleV(snake[i].position, snake[i].size, snake[i].color);

            // Draw fruit to pick
            DrawRectangleV(fruit.position, fruit.size, fruit.color);

            if (pause) DrawText("GAME PAUSED", screenWidth/2 - MeasureText("GAME PAUSED", 40)/2, screenHeight/2 - 40, 40, GRAY);
        }
        else 
        {
            //DrawText("SCORE: ",GetScreenWidth()/10 - 50,GetScreenHeight()-20, 15,YELLOW);
            DrawText("GAME OVER\n\n",GetScreenWidth()/3 - 20,GetScreenHeight()/2.5 - 20,50,YELLOW);
            DrawText("PRESS [ENTER] TO PLAY AGAIN", GetScreenWidth()/2 - MeasureText("PRESS [ENTER] TO PLAY AGAIN", 20)/2, GetScreenHeight()/1.75 - 20, 20, GRAY);
        }
    EndDrawing();
}

// Load all dynamic loaded data (textures, sounds, models...)
void LoadGame(void)
{
    InitAudioDevice();
    overSound = LoadSound("resources/gameover.mp3");
    eatSound = LoadSound("resources/eatSound.mp3");
}

// Unload game variables
void UnloadGame(void)
{
    UnloadSound(overSound);
    UnloadSound(eatSound);
    CloseAudioDevice();
    // TODO: Unload all dynamic loaded data (textures, sounds, models...)
}

// Update and Draw (one frame)
void UpdateDrawFrame(void)
{
    UpdateGame();
    DrawGame();
}