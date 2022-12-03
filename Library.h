#ifndef SU_NOMBRE_DE_PROYECTO_LIBRERIA_H
#define SU_NOMBRE_DE_PROYECTO_LIBRERIA_H

#include "raylib.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <ctype.h>

static const int screenWidth = 800;
static const int screenHeight = 450;

//------------------------------------------------------------------------------------
// Module Functions Declaration (local)
//------------------------------------------------------------------------------------
int* InitGame(int NIVEL);         // Initialize game
int UpdateGame(int* arrayPowers, int NIVEL,  int numBall); // Update game (one frame)
static void DrawGame(int *arrayPowers, int NIVEL);         // Draw game (one frame)
int UpdateDrawFrame(int* arrayPowers, int NIVEL);  // Update and Draw (one frame)


#endif //SU_NOMBRE_DE_PROYECTO_LIBRERIA_H

