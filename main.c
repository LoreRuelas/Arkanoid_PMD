#include "Library.h."


//------------------------------------------------------------------------------------
int main(void)
{
    // Se inicia la ventana
    InitWindow(screenWidth, screenHeight, "classic game: PROYECTO FINAL");

    // Se inician variables
    int NIVEL = 1;
    int *arrayPowers = InitGame(NIVEL);


#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 60, 1);
#else
    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // El loop principal
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Se actualiza el juego - se van cargando los diferntes "estados" del juego
        NIVEL = UpdateDrawFrame(arrayPowers, NIVEL);
        //----------------------------------------------------------------------------------
    }
#endif
    //--------------------------------------------------------------------------------------

    CloseWindow();        // Se cierra la ventana
    //--------------------------------------------------------------------------------------

    return 0;
}
