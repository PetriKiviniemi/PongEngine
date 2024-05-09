#include <raylib.h>

#include <iostream>
#include <PongEngineUDPClient.hpp>

int main()
{
    InitWindow(WINDOW_HEIGHT, WINDOW_WIDTH, "PongClient");
    SetTargetFPS(60);

    PongEngineUDPClient::GetInstance()->runClient();

    while(WindowShouldClose() == false)
    {
        if(IsKeyPressed(KEY_ESCAPE))
        {
            CloseWindow();
        }
    }

    std::cout << "Client initialized!" << std::endl;
    return 0;
}