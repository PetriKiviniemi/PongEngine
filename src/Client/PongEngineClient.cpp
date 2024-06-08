#include <raylib.h>

#include <iostream>
#include <PongEngineUDPClient.hpp>
#include <FrameDecoder.hpp>
#include <common.hpp>

int main()
{
    InitWindow(WINDOW_HEIGHT, WINDOW_WIDTH, "PongClient");
    SetTargetFPS(60);

    FrameDecoder* frameDecoder = FrameDecoder::GetInstance();
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
        AVFrame* frame = frameDecoder->getFrameFromQueue();
        if(frame)
        {
            //We are receiving YUV420 pixel format frames, so we have to convert
            Image image = {
                .data = frame->data[0],
                .width = frame->width,
                .height = frame->height,
                .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
            };
            Texture2D texture = LoadTextureFromImage(image);
            DrawTexture(texture, 0, 0, WHITE);

            UnloadTexture(texture);
            av_frame_free(&frame);
        }
        EndDrawing();
    }

    return 0;
}