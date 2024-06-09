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

    FrameDecoder* frameDecoder = FrameDecoder::GetInstance();
    PongEngineUDPClient::GetInstance()->runVideoStreamClient();
    PongEngineUDPServer* server = PongEngineUDPServer::GetInstance();
    server->setAddrAndPort("localhost", 9091);
    server->restartServer();
    Texture2D texture = { 0 };


    while(WindowShouldClose() == false)
    {
        if(IsKeyPressed(KEY_ESCAPE))
        {
            server->sendUserInput(KEY_ESCAPE);
            CloseWindow();
        }

        if(IsKeyPressed(KEY_W))
        {
            server->sendUserInput(KEY_W);
        }

        if(IsKeyPressed(KEY_S))
        {
            server->sendUserInput(KEY_S);
        }
        if(IsKeyPressed(KEY_ENTER))
        {
            server->sendUserInput(KEY_ENTER);
        }

        BeginDrawing();
        ClearBackground(BLACK);
        //Get the frames from FrameDecoder queue
        //Map them into textures and draw
        AVFrame* frame = frameDecoder->getFrameFromQueue();

        if(frame)
        {
            // Save the first 10 frames, the frame is converted to RGBA by the encoder
            //if(frameDecoder->getFrameIndex() <= 10)
            //{
            //    frameDecoder->saveFrametoPng(frame, AV_PIX_FMT_RGBA);
            //}

            // TODO:: Somehow this does not work, yet the frames are saved to png succesfully
            // It could be about saveFramePng converting the frame again
            // with SwsContext and sws_scale

            Image image = {
                .data = frame->data[0],
                .width = frame->width,
                .height = frame->height,
                .mipmaps = 1,
                .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
            };

            // Check if LoadTextureFromImage returns a valid texture
            texture = LoadTextureFromImage(image);
        }

        int posX = (WINDOW_WIDTH - texture.width) / 2;
        int posY = (WINDOW_HEIGHT - texture.height) / 2;

        DrawTexturePro(texture,
                    (Rectangle){0, 0, texture.width, texture.height},
                    (Rectangle){posX, posY, texture.width, texture.height},
                    Vector2Zero(),
                    0.0f,
                    WHITE);

        av_frame_free(&frame);
        EndDrawing();
    }

    UnloadTexture(texture);

    return 0;
}