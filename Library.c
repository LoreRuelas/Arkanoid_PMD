
#include "Library.h"

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

//----------------------------------------------------------------------------------
// Some
//----------------------------------------------------------------------------------
#define PLAYER_MAX_LIFE         3
#define LINES_OF_BRICKS         5
#define BRICKS_PER_LINE         20

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
static bool gameOver = false;
static bool pause = false;

static Player player = { 0 };
static Ball ball = { 0 };
static Ball ball2 = { 0 };
static Ball ball3 = { 0 };

static Brick brick[LINES_OF_BRICKS][BRICKS_PER_LINE] = { 0 };
static Vector2 brickSize = { 0 };


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

    ball.position = (Vector2){ screenWidth/2, screenHeight*7/8 - 30 };
    ball.speed = (Vector2){ 10, 10 };
    ball.radius = 10;
    ball.active = false;

    // Se inicializan balls para el poder MultiBall
    ball2.position = (Vector2){ 700, screenHeight - 30};
    ball2.speed = (Vector2){ 10, 10 };
    ball2.radius = 10;
    ball2.active = false;

    ball3.position = (Vector2){ 730, screenHeight - 30};
    ball3.speed = (Vector2){ 10, 10 };
    ball3.radius = 10;
    ball3.active = false;


    // Initialize Powers lrg
    int *arrayPowers = malloc(3*sizeof(int));
    arrayPowers[0] = 1;
    arrayPowers[1] = 1;
    // arrayPowers[2] = 1;

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
            //printf("Check Nivel = " + NIVEL);

            switch(NIVEL)
            {
                case 1:
                    break;
                case 2:
                    // Se modifica el "mundo de bricks"
                    brick[i][j].numHits = rand() % 2;
                    break;

                case 3:
                    if ((i + j) % 2 == 0) // bricks oscuros mas resistentes
                        brick[i][j].numHits = 2;
                    else //bricks de color claro mas vulnerables
                        brick[i][j].numHits = 3;
                default:
                    player.size = (Vector2){ screenWidth/14, 20 };

            }
        }
    }
    return arrayPowers; //lrg
}


// Update game (one frame)
int UpdateGame(int* arrayPowers, int NIVEL, int numBall)
{
    Ball * ballptr = &ball;
    if ( numBall == 2 ) {
        ballptr = &ball2;
    } else if ( numBall == 3 )
        ballptr = &ball3;


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
                //printf("REALEASED");
                arrayPowers[0] = 0;
            }

            // Player movement logics
            if (IsKeyDown(KEY_LEFT)) player.position.x -= 5;
            if ((player.position.x - player.size.x/2) <= 0) player.position.x = player.size.x/2;
            if (IsKeyDown(KEY_RIGHT)) player.position.x += 5;
            if ((player.position.x + player.size.x/2) >= screenWidth) player.position.x = screenWidth - player.size.x/2;

            // Ball launching logic
            if (!ballptr->active)
            {
                if (IsKeyPressed(KEY_SPACE))
                {
                    ballptr->active = true;
                    ballptr->speed = (Vector2){ 0, -5 };
                }
            }

            // Ball movement logic
            if (ballptr->active)
            {
                ballptr->position.x += ballptr->speed.x;
                ballptr->position.y += ballptr->speed.y;
            }
            else
            {
                // se modifica la posicion inicial de las pelotas lrg
                if ( numBall == 2 ) {
                    ballptr->position = (Vector2){ player.position.x + 20 , screenHeight*7/8 - 20 };
                } else if ( numBall == 3 ) {
                    ballptr->position = (Vector2){ player.position.x - 10 , screenHeight*7/8 - 20 };
                } else {
                    ballptr->position = (Vector2){ player.position.x, screenHeight*7/8 - 30 };
                }

            }

            // Collision logic: ball vs walls
            if (((ballptr->position.x + ballptr->radius) >= screenWidth) || ((ballptr->position.x - ballptr->radius) <= 0))
                ballptr->speed.x *= -1;
            if ((ballptr->position.y - ballptr->radius) <= 0)
                ballptr->speed.y *= -1;
            if ((ballptr->position.y + ballptr->radius) >= screenHeight)
            {
                if (numBall == 1) {
                    ballptr->speed = (Vector2){ 0, 0 };
                    ballptr->active = false;
                    player.life--;
                } else {
                    //ballptr->speed.y *= -1;
                    ballptr->active = false;
                    ballptr->radius = 0;

                }

            }

            // Collision logic: ball vs player
            if (CheckCollisionCircleRec(ballptr->position, ballptr->radius,
                                        (Rectangle){ player.position.x - player.size.x/2, player.position.y - player.size.y/2, player.size.x, player.size.y}))
            {
                if (ballptr->speed.y > 0)
                {
                    ballptr->speed.y *= -1;
                    ballptr->speed.x = (ballptr->position.x - player.position.x)/(player.size.x/2)*5;
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
                        if (((ballptr->position.y - ballptr->radius) <= (brick[i][j].position.y + brickSize.y/2)) &&
                            ((ballptr->position.y - ballptr->radius) > (brick[i][j].position.y + brickSize.y/2 + ballptr->speed.y)) &&
                            ((fabs(ballptr->position.x - brick[i][j].position.x)) < (brickSize.x/2 + ballptr->radius*2/3)) && (ballptr->speed.y < 0))
                        {
                            brick[i][j].numHits --;
                            //brick[i][j].active = false; lrg
                            ballptr->speed.y *= Num;
                        }
                            // Hit above
                        else if (((ballptr->position.y + ballptr->radius) >= (brick[i][j].position.y - brickSize.y/2)) &&
                                 ((ballptr->position.y + ballptr->radius) < (brick[i][j].position.y - brickSize.y/2 + ballptr->speed.y)) &&
                                 ((fabs(ballptr->position.x - brick[i][j].position.x)) < (brickSize.x/2 + ballptr->radius*2/3)) && (ballptr->speed.y > 0))
                        {
                            brick[i][j].numHits --;
                            //brick[i][j].active = false;
                            ballptr->speed.y *= Num;
                        }
                            // Hit left
                        else if (((ballptr->position.x + ballptr->radius) >= (brick[i][j].position.x - brickSize.x/2)) &&
                                 ((ballptr->position.x + ballptr->radius) < (brick[i][j].position.x - brickSize.x/2 + ballptr->speed.x)) &&
                                 ((fabs(ballptr->position.y - brick[i][j].position.y)) < (brickSize.y/2 + ballptr->radius*2/3)) && (ballptr->speed.x > 0))
                        {
                            brick[i][j].numHits --;
                            //brick[i][j].active = false;
                            ballptr->speed.x *= Num;
                        }
                            // Hit right
                        else if (((ballptr->position.x - ballptr->radius) <= (brick[i][j].position.x + brickSize.x/2)) &&
                                 ((ballptr->position.x - ballptr->radius) > (brick[i][j].position.x + brickSize.x/2 + ballptr->speed.x)) &&
                                 ((fabs(ballptr->position.y - brick[i][j].position.y)) < (brickSize.y/2 + ballptr->radius*2/3)) && (ballptr->speed.x < 0))
                        {
                            brick[i][j].numHits --;
                            //brick[i][j].active = false;
                            ballptr->speed.x *= Num;
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
            NIVEL++;
            InitGame(NIVEL);
            gameOver = false;
        }
    }
    return NIVEL;
}


// Draw game (one frame)
void DrawGame(int *arrayPowers, int NIVEL)
{
    BeginDrawing();

    ClearBackground(RAYWHITE);

    if (!gameOver)
    {
        // Draw player bar
        DrawRectangle(player.position.x - player.size.x/2, player.position.y - player.size.y/2, player.size.x, player.size.y, BLACK);

        // Draw player lives
        for (int i = 0; i < player.life; i++) DrawRectangle(20 + 40*i, screenHeight - 30, 35, 10, PURPLE);

        // Draw ball
        DrawCircleV(ball.position, ball.radius, PURPLE);

        // lrg se dibujan balls para el MultiBall // se dejan de dibujar si ya estÃ¡ inactivo el poder
        if (arrayPowers[1] == 1)
        {
            DrawCircleV(ball2.position, ball2.radius, PURPLE);
            DrawCircleV(ball3.position, ball3.radius, PURPLE);
        }


        // Draw bricks
        for (int i = 0; i < LINES_OF_BRICKS; i++)
        {
            for (int j = 0; j < BRICKS_PER_LINE; j++)
            {
                if (brick[i][j].active)
                {
                    if ((i + j) % 2 == 0) DrawRectangle(brick[i][j].position.x - brickSize.x/2, brick[i][j].position.y - brickSize.y/2, brickSize.x, brickSize.y, PINK);
                    else DrawRectangle(brick[i][j].position.x - brickSize.x/2, brick[i][j].position.y - brickSize.y/2, brickSize.x, brickSize.y, PURPLE);
                }
            }
        }

        if (pause) DrawText("GAME PAUSED", screenWidth/2 - MeasureText("GAME PAUSED", 40)/2, screenHeight/2 - 40, 40, PINK);
    }
    else {
        DrawText("PRESS [ENTER] TO NEXT LEVEL",
                 GetScreenWidth() / 2 - MeasureText("PRESS [ENTER] TO PLAY AGAIN", 20) / 2, GetScreenHeight() / 2 - 50,
                 20, PINK);
        printf("NIVEL INCREMENTED");

    }

    DrawText(TextFormat("NIVEL: %i", NIVEL), 10, 10, 20, DARKPURPLE);
    EndDrawing();
}

// Update and Draw (one frame)
int UpdateDrawFrame(int* arrayPowers, int NIVEL)
{
    NIVEL = UpdateGame(arrayPowers, NIVEL, 1 );
    if (IsKeyDown('A') == 1 && arrayPowers[1] == 1) {
        UpdateGame(arrayPowers, NIVEL, 2);
        UpdateGame(arrayPowers, NIVEL, 3);
    }
    if (IsKeyReleased('A') == 1) {
        //printf("REALEASED");
        arrayPowers[1] = 0;
    }

    DrawGame(arrayPowers, NIVEL);
    return NIVEL;
}
