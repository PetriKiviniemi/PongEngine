#include <raylib.h>

#include <iostream>
#include <PongEngineUDPClient.hpp>
#include <PongEngineServer.hpp>
#include <FrameDecoder.hpp>
#include <common.hpp>
#include <raymath.h>

int main()
{
    InitWindow(WINDOW_HEIGHT, WINDOW_WIDTH, "PongClient");
    SetTargetFPS(60);
    SetTraceLogLevel(LOG_NONE);

    FrameDecoder* frameDecoder = FrameDecoder::GetInstance();
    PongEngineUDPClient::GetInstance()->runVideoStreamClient();
    PongEngineUDPServer* server = PongEngineUDPServer::GetInstance();
    server->setAddrAndPort("localhost", 9091);
    server->restartServer();
    Texture2D texture = { 0 };
    bool textureLoaded = false;

    while(WindowShouldClose() == false)
    {
        if(IsKeyPressed(KEY_ESCAPE))
        {
            server->sendUserInput(KEY_ESCAPE);
            CloseWindow();
        }

        if(IsKeyDown(KEY_W))
        {
            server->sendUserInput(KEY_W);
        }

        if(IsKeyDown(KEY_S))
        {
            server->sendUserInput(KEY_S);
        }
        if(IsKeyPressed(KEY_ENTER))
        {
            server->sendUserInput(KEY_ENTER);
        }

        //Get the frames from FrameDecoder queue
        //Map them into textures and draw
        AVFrame* frame = frameDecoder->getFrameFromQueue();

        if(frame)
        {
            if(textureLoaded)
            {
                UnloadTexture(texture);
            }
            // Save the first 10 frames, the frame is converted to RGBA by the encoder
            //if(frameDecoder->getFrameIndex() <= 10)
            //{
            //    frameDecoder->saveFrametoPng(frame, AV_PIX_FMT_RGBA);
            //}

            Image image = {
                .data = frame->data[0],
                .width = frame->width,
                .height = frame->height,
                .mipmaps = 1,
                .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
            };

            // Check if LoadTextureFromImage returns a valid texture
            texture = LoadTextureFromImage(image);
            textureLoaded = true;
            av_frame_free(&frame);
        }

        BeginDrawing();
        ClearBackground(BLACK);

        if(textureLoaded)
        {

            int posX = (WINDOW_WIDTH - texture.width) / 2;
            int posY = (WINDOW_HEIGHT - texture.height) / 2;

            DrawTexturePro(texture,
                        (Rectangle){0, 0, texture.width, texture.height},
                        (Rectangle){posX, posY, texture.width, texture.height},
                        Vector2Zero(),
                        0.0f,
                        WHITE);

        }

        EndDrawing();
    }

    UnloadTexture(texture);

    return 0;
}