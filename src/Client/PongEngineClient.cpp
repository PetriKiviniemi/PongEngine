#include <raylib.h>

#include <iostream>
#include <PongEngineUDPClient.hpp>
#include <FrameDecoder.hpp>

int main()
{
    InitWindow(WINDOW_HEIGHT, WINDOW_WIDTH, "PongClient");
    SetTargetFPS(60);

    FrameDecoder::GetInstance();
    PongEngineUDPClient::GetInstance()->runClient();

    while(WindowShouldClose() == false)
    {
        if(IsKeyPressed(KEY_ESCAPE))
        {
            CloseWindow();
        }

        BeginDrawing();
        ClearBackground(BLACK);
        //Get the frames from FrameDecoder queue
        //Map them into textures and draw
        EndDrawing();
    }

    std::cout << "Client initialized!" << std::endl;
    return 0;
}