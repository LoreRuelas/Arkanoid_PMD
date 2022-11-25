#include "raylib.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

/* REQUERIMIENTOS DEL JUEGO
 * :) - Al menos 2 tipos diferentes de bricks (i.e uno que se rompe con un solo impacto, otro que requiere 2 impactos)
- Al menos 3 niveles diferentes, que aumenten en complejidad y cambie la configuración y orden de los bricks.
:) - El jugador tiene 3 vidas, si pierde todas : GAME_OVER
- El jugador tiene 2 poderes que puede usar UNA vez en CUALQUIER momento
                                       --- MultiBall, donde la bola se divide en 3 (alterandose su angulo de salida), es suficiente que el JUGADOR evite que al menos una de las bolas se pierda
                                       ---- PowerBall, donde la bola destruye todos los BRICKS de un solo golpe y solo rebota contra las paredes - y hasta tocar el PALLET.
 */

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

//----------------------------------------------------------------------------------
// Some
//----------------------------------------------------------------------------------
#define PLAYER_MAX_LIFE         3
#define LINES_OF_BRICKS         1
#define BRICKS_PER_LINE         2
//#define NIVEL                   3

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef struct Player {
    Vector2 position;
    Vector2 size;
    int life;
} Player;

typedef struct Ball {
    Vector2 position;
    Vector2 speed;
    int radius;
    bool active;
} Ball;

typedef struct Brick {
    Vector2 position;
    bool active;
    int numHits;
} Brick;

//------------------------------------------------------------------------------------
// Global Variables Declaration
//------------------------------------------------------------------------------------
static const int screenWidth = 800;
static const int screenHeight = 450;

static bool gameOver = false;
static bool pause = false;

static Player player = { 0 };
static Ball ball = { 0 };

static Brick brick[LINES_OF_BRICKS][BRICKS_PER_LINE] = { 0 };
static Vector2 brickSize = { 0 };

//------------------------------------------------------------------------------------
// Module Functions Declaration (local)
//------------------------------------------------------------------------------------
int* InitGame(int NIVEL);         // Initialize game
static void UpdateGame(int* arrayPowers, int NIVEL); // Update game (one frame)
int DrawGame(int NIVEL);         // Draw game (one frame)
static void UnloadGame(void);       // Unload game


static void UpdateDrawFrame(int* arrayPowers, int NIVEL);  // Update and Draw (one frame)

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    // Initialization (Note windowTitle is unused on Android)
    //---------------------------------------------------------
    InitWindow(screenWidth, screenHeight, "classic game: arkanoid");

    int NIVEL = 1;
    int *arrayPowers = InitGame(NIVEL);


#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 60, 1);
#else
    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update and Draw
        //----------------------------------------------------------------------------------
        UpdateDrawFrame(arrayPowers, NIVEL);
        //----------------------------------------------------------------------------------
    }
#endif
    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadGame();         // Unload loaded data (textures, sounds, models...)

    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}

//------------------------------------------------------------------------------------
// Module Functions Definitions (local)
//------------------------------------------------------------------------------------

// Initialize game variables
int* InitGame(int NIVEL)
{
    brickSize = (Vector2){ GetScreenWidth()/BRICKS_PER_LINE, 40 };

    // Initialize player
    player.position = (Vector2){ screenWidth/2, screenHeight*7/8 };
    player.size = (Vector2){ screenWidth/10, 20 };
    player.life = PLAYER_MAX_LIFE;
/*
     Ball ball2;
   if (IsKeyDown('A') == 1) { //&& arrayPowers[0] == 0){
        ball2.position = (Vector2){ screenWidth/2, screenHeight*7/8 - 30 };
        ball2.speed = (Vector2){ 10, 10 };
        ball2.radius = 10;
        ball2.active = true;
    }
*/
    ball.position = (Vector2){ screenWidth/2, screenHeight*7/8 - 30 };
    //ball.speed = (Vector2){ 5, 5 };
    //ball.radius = 10;
    ball.active = false;


    // Initialize Powers lrg
    int *arrayPowers = malloc(3*sizeof(int));
    arrayPowers[0] = 1;
    arrayPowers[1] = 1;
    arrayPowers[2] = 1;

   // Initialize bricks
    int initialDownPosition = 50;

    for (int i = 0; i < LINES_OF_BRICKS; i++)
    {
        for (int j = 0; j < BRICKS_PER_LINE; j++)
        {
            //brick[i][j].numHits = 2;
            brick[i][j].position = (Vector2){ j*brickSize.x + brickSize.x/2, i*brickSize.y + initialDownPosition };
            brick[i][j].active = true;

            // 2 diferent bricks lrg
            if ((i + j) % 2 == 0) // bricks oscuros mas resistentes
                    brick[i][j].numHits = 1;
            else //bricks de color claro mas vulnerables
                brick[i][j].numHits = 2;

            switch(NIVEL)
            {
                case 1:
                    ball.speed = (Vector2){ 5, 5 };
                    ball.radius = 10;
                    break;
                case 2:
                    ball.speed = (Vector2){ -1000,-1000  };
                    ball.radius = 40;
                    break;

                case 3:
                    ball.radius = 100;
                    if (j % 2 == 0)
                        brick[i][j].numHits = rand() % 2;
                    else
                        brick[i][j].numHits = rand() % 2 ;

            }
        }
            /*
            // nivel 2 lrg
             if (j % 2 == 0)
                brick[i][j].numHits = rand() % 2;
            else
                brick[i][j].numHits = rand() % 2 ;
                }
                */
    }
    return arrayPowers; //lrg
}

// Update game (one frame)
void UpdateGame(int* arrayPowers, int NIVEL)
{
    if (!gameOver)
    {
        if (IsKeyPressed('P')) pause = !pause;

        if (!pause)
        {
            // SprPoder PowerBall - Se activa con la Ss lrg
            float Num = -1;
            if (IsKeyDown('S') == 1 && arrayPowers[0] == 1)
                Num = 1;
            if (IsKeyReleased('S') == 1) {
                printf("REALEASED");
                arrayPowers[0] = 0;
            }
            
            // Player movement logics
            if (IsKeyDown(KEY_LEFT)) player.position.x -= 5;
            if ((player.position.x - player.size.x/2) <= 0) player.position.x = player.size.x/2;
            if (IsKeyDown(KEY_RIGHT)) player.position.x += 5;
            if ((player.position.x + player.size.x/2) >= screenWidth) player.position.x = screenWidth - player.size.x/2;

            // Ball launching logic
            if (!ball.active)
            {
                if (IsKeyPressed(KEY_SPACE))
                {
                    ball.active = true;
                    ball.speed = (Vector2){ 0, -5 };
                }
            }

            // Ball movement logic
            if (ball.active)
            {
                ball.position.x += ball.speed.x;
                ball.position.y += ball.speed.y;
            }
            else
            {
                ball.position = (Vector2){ player.position.x, screenHeight*7/8 - 30 };
            }

            // Collision logic: ball vs walls
            if (((ball.position.x + ball.radius) >= screenWidth) || ((ball.position.x - ball.radius) <= 0)) ball.speed.x *= -1;
            if ((ball.position.y - ball.radius) <= 0) ball.speed.y *= -1;
            if ((ball.position.y + ball.radius) >= screenHeight)
            {
                ball.speed = (Vector2){ 0, 0 };
                ball.active = false;

                player.life--;
            }

            // Collision logic: ball vs player
            if (CheckCollisionCircleRec(ball.position, ball.radius,
                                        (Rectangle){ player.position.x - player.size.x/2, player.position.y - player.size.y/2, player.size.x, player.size.y}))
            {
                if (ball.speed.y > 0)
                {
                    ball.speed.y *= -1;
                    ball.speed.x = (ball.position.x - player.position.x)/(player.size.x/2)*5;
                }
            }


            // Collision logic: ball vs bricks
            for (int i = 0; i < LINES_OF_BRICKS; i++)
            {
                for (int j = 0; j < BRICKS_PER_LINE; j++)
                {
                    if (brick[i][j].active)
                    {
                        // Hit below
                        if (((ball.position.y - ball.radius) <= (brick[i][j].position.y + brickSize.y/2)) &&
                            ((ball.position.y - ball.radius) > (brick[i][j].position.y + brickSize.y/2 + ball.speed.y)) &&
                            ((fabs(ball.position.x - brick[i][j].position.x)) < (brickSize.x/2 + ball.radius*2/3)) && (ball.speed.y < 0))
                        {
                            brick[i][j].numHits --;
                            //brick[i][j].active = false; lrg
                            ball.speed.y *= Num;
                        }
                            // Hit above
                        else if (((ball.position.y + ball.radius) >= (brick[i][j].position.y - brickSize.y/2)) &&
                                 ((ball.position.y + ball.radius) < (brick[i][j].position.y - brickSize.y/2 + ball.speed.y)) &&
                                 ((fabs(ball.position.x - brick[i][j].position.x)) < (brickSize.x/2 + ball.radius*2/3)) && (ball.speed.y > 0))
                        {
                            brick[i][j].numHits --;
                            //brick[i][j].active = false;
                            ball.speed.y *= Num;
                        }
                            // Hit left
                        else if (((ball.position.x + ball.radius) >= (brick[i][j].position.x - brickSize.x/2)) &&
                                 ((ball.position.x + ball.radius) < (brick[i][j].position.x - brickSize.x/2 + ball.speed.x)) &&
                                 ((fabs(ball.position.y - brick[i][j].position.y)) < (brickSize.y/2 + ball.radius*2/3)) && (ball.speed.x > 0))
                        {
                            brick[i][j].numHits --;
                            //brick[i][j].active = false;
                            ball.speed.x *= Num;
                        }
                            // Hit right
                        else if (((ball.position.x - ball.radius) <= (brick[i][j].position.x + brickSize.x/2)) &&
                                 ((ball.position.x - ball.radius) > (brick[i][j].position.x + brickSize.x/2 + ball.speed.x)) &&
                                 ((fabs(ball.position.y - brick[i][j].position.y)) < (brickSize.y/2 + ball.radius*2/3)) && (ball.speed.x < 0))
                        {
                            brick[i][j].numHits --;
                            //brick[i][j].active = false;
                            ball.speed.x *= Num;
                        }
                    }
                    // Doble Hit to break brick lrg (checar que cumpla condiciones y no se buggue con el PowerBall)
                    if (brick[i][j].numHits == 0)
                        brick[i][j].active = false;
                }
            }

            // Game over logic
            if (player.life <= 0) gameOver = true;
            else
            {
                gameOver = true;

                for (int i = 0; i < LINES_OF_BRICKS; i++)
                {
                    for (int j = 0; j < BRICKS_PER_LINE; j++)
                    {
                        if (brick[i][j].active) gameOver = false;
                    }
                }
            }
        }
    }
    else
    {
        if (IsKeyPressed(KEY_ENTER))
        {
            InitGame(NIVEL);
            gameOver = false;
        }
    }
}

// Draw game (one frame)
int DrawGame(int NIVEL)
{
    BeginDrawing();

    ClearBackground(RAYWHITE);

    if (!gameOver)
    {
        // Draw player bar
        DrawRectangle(player.position.x - player.size.x/2, player.position.y - player.size.y/2, player.size.x, player.size.y, BLACK);

        // Draw player lives
        for (int i = 0; i < player.life; i++) DrawRectangle(20 + 40*i, screenHeight - 30, 35, 10, DARKPURPLE);

        // Draw ball
        DrawCircleV(ball.position, ball.radius, MAROON);

        // Draw bricks
        for (int i = 0; i < LINES_OF_BRICKS; i++)
        {
            for (int j = 0; j < BRICKS_PER_LINE; j++)
            {
                if (brick[i][j].active)
                {
                    if ((i + j) % 2 == 0) DrawRectangle(brick[i][j].position.x - brickSize.x/2, brick[i][j].position.y - brickSize.y/2, brickSize.x, brickSize.y, PINK);
                    else DrawRectangle(brick[i][j].position.x - brickSize.x/2, brick[i][j].position.y - brickSize.y/2, brickSize.x, brickSize.y, DARKPURPLE);
                }
            }
        }

        if (pause) DrawText("GAME PAUSED", screenWidth/2 - MeasureText("GAME PAUSED", 40)/2, screenHeight/2 - 40, 40, PINK);
    }
    else {
        DrawText("PRESS [ENTER] TO NEXT LEVEL",
                 GetScreenWidth() / 2 - MeasureText("PRESS [ENTER] TO PLAY AGAIN", 20) / 2, GetScreenHeight() / 2 - 50,
                 20, PINK);
        NIVEL ++;

    }

    EndDrawing();
    return NIVEL;
}

// Unload game variables
void UnloadGame(void)
{
    // TODO: Unload all dynamic loaded data (textures, sounds, models...)
}

// Update and Draw (one frame)
void UpdateDrawFrame(int* arrayPowers, int NIVEL)
{
    UpdateGame(arrayPowers, NIVEL);
    DrawGame(NIVEL);
}
