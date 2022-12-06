
#include "Library.h"

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

//----------------------------------------------------------------------------------
// Se definen constantes
//----------------------------------------------------------------------------------
#define PLAYER_MAX_LIFE         3
#define LINES_OF_BRICKS         6
#define BRICKS_PER_LINE         6

//----------------------------------------------------------------------------------
// Se definen los structs
//----------------------------------------------------------------------------------
typedef struct Player { // barra negra que controla el jugador
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
// Se definen variables globales
//------------------------------------------------------------------------------------
static bool gameOver = false;
static bool pause = false;

static Player player = { 0 };
static Ball ball = { 0 };
static Ball ball2 = { 0 };
static Ball ball3 = { 0 };
float Num = -1; // Se usa para activar el PowerBall (En un inicio desactivado)
int  BanderaMultiBall = 1; // Se usa para activar el poder MultiBall

static Brick brick[LINES_OF_BRICKS][BRICKS_PER_LINE] = { 0 };
static Vector2 brickSize = { 0 };


//------------------------------------------------------------------------------------

// Se inicilizan variables (Carcateristicas de los bricks/player/ball)
int* InitGame(int NIVEL)
{
    brickSize = (Vector2){ GetScreenWidth()/BRICKS_PER_LINE, 40 };

    // Se inicializa el jugador
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


    // Se inicializa array que "controla" los poderes
    int *arrayPowers = malloc(3*sizeof(int));
    arrayPowers[0] = 1; // PoderBall
    arrayPowers[1] = 1; // MultiBall
    // arrayPowers[2] = 1;

    // Se inicializan los bricks
    int initialDownPosition = 50;

    for (int i = 0; i < LINES_OF_BRICKS; i++)
    {
        for (int j = 0; j < BRICKS_PER_LINE; j++)
        {
            //brick[i][j].numHits = 2;
            brick[i][j].position = (Vector2){ j*brickSize.x + brickSize.x/2, i*brickSize.y + initialDownPosition };
            brick[i][j].active = true;

            // 2 diferent bricks lrg
            if ((i + j) % 2 == 0) // bricks de color claro mas vulnerables 1 hit
                brick[i][j].numHits = 1;
            else // bricks oscuros mas resistentes 2 hits
                brick[i][j].numHits = 2;

            switch(NIVEL)
            {
                case 1: // Nivel 1 Es el basico y común
                    break;
                case 2: // Nivel 2 Se modifica el "mundo de bricks"
                    brick[i][j].numHits = rand() % 2;
                    break;

                case 3: // Nivel 3 Unos bricks mucho más resistentes (3 hits) y otros (2 hits)
                    if ((i + j) % 2 == 0) // bricks oscuros mas resistentes
                        brick[i][j].numHits = 2; // 2 hits
                    else //bricks de color claro mas vulnerables
                        brick[i][j].numHits = 3; // 3 hits
                default:
                    player.size = (Vector2){ screenWidth/14, 20 };

            }
        }
    }
    return arrayPowers; //lrg
}

// Controla la "imagen" que se muestra al ususario / Se actualiza el juego
int UpdateGame(int* arrayPowers, int NIVEL, int numBall)
{
    // Se escoge el ball a usar, dependiendo a "numBall"
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

            if (IsKeyPressed('S') == 1 && arrayPowers[0] == 1) {
                Num = 1; // Variable clave para que funcione PowerBall
                // Se cambia la resistencia de los bricks a 1 hit
                for (int i = 0; i < LINES_OF_BRICKS; i++)
                {
                    for (int j = 0; j < LINES_OF_BRICKS; j++)
                    {
                        brick[i][j].numHits = 1;
                    }
                }
            }

            // Control del movimiento del jugador
            if (IsKeyDown(KEY_LEFT)) player.position.x -= 5;
            if ((player.position.x - player.size.x/2) <= 0) player.position.x = player.size.x/2;
            if (IsKeyDown(KEY_RIGHT)) player.position.x += 5;
            if ((player.position.x + player.size.x/2) >= screenWidth) player.position.x = screenWidth - player.size.x/2;

            // Se inicia la pelota/  Se suelta hacia el jugador (plataforma negra)
            if (!ballptr->active)
            {
                if (IsKeyPressed(KEY_SPACE))
                {
                    ballptr->active = true;
                    ballptr->speed = (Vector2){ 0, -5 };
                }
            }

            // Control del movimiento de la pelota
            if (ballptr->active)
            {
                ballptr->position.x += ballptr->speed.x;
                ballptr->position.y += ballptr->speed.y;
            }
            else
            {
                // se modifica la posicion inicial de las pelotas (Considerando las del MultiBall)
                if ( numBall == 2 ) {
                    ballptr->position = (Vector2){ player.position.x + 20 , screenHeight*7/8 - 20 };
                } else if ( numBall == 3 ) {
                    ballptr->position = (Vector2){ player.position.x - 10 , screenHeight*7/8 - 20 };
                } else {
                    ballptr->position = (Vector2){ player.position.x, screenHeight*7/8 - 30 };
                }

            }

            // Colisión de la ball contra las "paredes"
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
                    ballptr->active = false;
                    ballptr->radius = 0;
                    BanderaMultiBall = 1;
                    arrayPowers[1] = 0;

                }

            }

            // Colisión de ball contra el jugador(barra negra)
            if (CheckCollisionCircleRec(ballptr->position, ballptr->radius,
                                        (Rectangle){ player.position.x - player.size.x/2, player.position.y - player.size.y/2, player.size.x, player.size.y}))
            {
                if (ballptr->speed.y > 0)
                {
                    ballptr->speed.y *= -1;
                    ballptr->speed.x = (ballptr->position.x - player.position.x)/(player.size.x/2)*5;
                }
                if (Num == 1) // Checa si PowerBall está activo
                {
                    // Se desactiva el poder /  Num vuelve a su valor original
                    Num = -1;
                    arrayPowers[0] = 0;
                    // Los hits de los bricks vuelven a su normalidad
                    for (int i = 0; i < LINES_OF_BRICKS; i++)
                    {
                        for (int j = 0; j < LINES_OF_BRICKS; j++)
                        {
                            if ((i + j) % 2 == 0) // bricks de color claro mas vulnerables
                                brick[i][j].numHits = 1;
                            else // bricks oscuros mas resistentes
                                brick[i][j].numHits = 2;
                        }
                    }
                }
            }


            // Colisión de ball contra bricks
            //  Al detectar una colisión (con los if) se decrementa el num de hits y se modifica la velocidad
            //  Velocidad*= : (-1 : rebota, 1 : continua (PowerBall)
            for (int i = 0; i < LINES_OF_BRICKS; i++)
            {
                for (int j = 0; j < BRICKS_PER_LINE; j++)
                {
                    if (brick[i][j].active)
                    {
                        // Golpe abajo
                        if (((ballptr->position.y - ballptr->radius) <= (brick[i][j].position.y + brickSize.y/2)) &&
                            ((ballptr->position.y - ballptr->radius) > (brick[i][j].position.y + brickSize.y/2 + ballptr->speed.y)) &&
                            ((fabs(ballptr->position.x - brick[i][j].position.x)) < (brickSize.x/2 + ballptr->radius*2/3)) && (ballptr->speed.y < 0))
                        {
                            brick[i][j].numHits --;
                            ballptr->speed.y *= Num;
                        }
                            // Golpe Arriba
                        else if (((ballptr->position.y + ballptr->radius) >= (brick[i][j].position.y - brickSize.y/2)) &&
                                 ((ballptr->position.y + ballptr->radius) < (brick[i][j].position.y - brickSize.y/2 + ballptr->speed.y)) &&
                                 ((fabs(ballptr->position.x - brick[i][j].position.x)) < (brickSize.x/2 + ballptr->radius*2/3)) && (ballptr->speed.y > 0))
                        {
                            brick[i][j].numHits --;
                            ballptr->speed.y *= Num;
                        }
                            // Golpe a la izquierda
                        else if (((ballptr->position.x + ballptr->radius) >= (brick[i][j].position.x - brickSize.x/2)) &&
                                 ((ballptr->position.x + ballptr->radius) < (brick[i][j].position.x - brickSize.x/2 + ballptr->speed.x)) &&
                                 ((fabs(ballptr->position.y - brick[i][j].position.y)) < (brickSize.y/2 + ballptr->radius*2/3)) && (ballptr->speed.x > 0))
                        {
                            brick[i][j].numHits --;
                            ballptr->speed.x *= Num;
                        }
                            // Golpe a la derecha
                        else if (((ballptr->position.x - ballptr->radius) <= (brick[i][j].position.x + brickSize.x/2)) &&
                                 ((ballptr->position.x - ballptr->radius) > (brick[i][j].position.x + brickSize.x/2 + ballptr->speed.x)) &&
                                 ((fabs(ballptr->position.y - brick[i][j].position.y)) < (brickSize.y/2 + ballptr->radius*2/3)) && (ballptr->speed.x < 0))
                        {
                            brick[i][j].numHits --;
                            ballptr->speed.x *= Num;
                        }
                    }
                    // Doble Hit to break brick lrg (checar que cumpla condiciones y no se buggue con el PowerBall)
                    if (brick[i][j].numHits == 0)
                        brick[i][j].active = false;
                }
            }

            // Game Over
            if (player.life <= 0) gameOver = true;
            else
            {
                gameOver = true;

                // Se desactivan los bricks
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
        if (IsKeyPressed(KEY_ENTER)) // Empieza el juego
        {
            NIVEL++; //Sube de NIVEL
            // Se vuelven a inicializar variables
            Num = -1;
            BanderaMultiBall = 1;
            InitGame(NIVEL);
            gameOver = false;
        }
    }
    return NIVEL;
}


// Dibuja los frames
void DrawGame(int *arrayPowers, int NIVEL)
{
    BeginDrawing();

    ClearBackground(RAYWHITE);

    if (!gameOver)
    {
        // Dibuja jugador (barra negra)
        DrawRectangle(player.position.x - player.size.x/2, player.position.y - player.size.y/2, player.size.x, player.size.y, BLACK);

        // Dibuja barras de vida (izq abajo)
        for (int i = 0; i < player.life; i++) DrawRectangle(20 + 40*i, screenHeight - 30, 35, 10, PURPLE);

        // Dibuja ball
        DrawCircleV(ball.position, ball.radius, PURPLE);

        // Se dibujan balls para el MultiBall // se dejan de dibujar si ya está inactivo el poder
        if (arrayPowers[1] == 1)
        {
            DrawCircleV(ball2.position, ball2.radius, PURPLE);
            DrawCircleV(ball3.position, ball3.radius, PURPLE);
        }


        // Se dibujan los bricks
        for (int i = 0; i < LINES_OF_BRICKS; i++)
        {
            for (int j = 0; j < BRICKS_PER_LINE; j++)
            {
                if (brick[i][j].active)
                {
                    if ((i + j) % 2 == 0 || brick[i][j].numHits == 1) // si es brick de solo un hit o le queda solo un hit será brick PINK
                        DrawRectangle(brick[i][j].position.x - brickSize.x/2, brick[i][j].position.y - brickSize.y/2, brickSize.x, brickSize.y, PINK);
                    else
                        DrawRectangle(brick[i][j].position.x - brickSize.x/2, brick[i][j].position.y - brickSize.y/2, brickSize.x, brickSize.y, PURPLE);
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

// Se actualiza el juego (UpdateGame)  y se dibuja (DrawGame)
int UpdateDrawFrame(int* arrayPowers, int NIVEL)
{
    NIVEL = UpdateGame(arrayPowers, NIVEL, 1 );

    // Poder: MultiBall -
    if (IsKeyPressed('A') &&  BanderaMultiBall == 1)
    {   // Se activa bandera para MultiBall
        BanderaMultiBall = 0;
    }
    // Si se activa se manda llamar la funcion con las diferentes balls
    if ( BanderaMultiBall == 0 && arrayPowers[1] == 1 ) {
        UpdateGame(arrayPowers, NIVEL, 2);
        UpdateGame(arrayPowers, NIVEL, 3);
    }

    // Se dibuja el estado del juego
    DrawGame(arrayPowers, NIVEL);

    return NIVEL;
}
