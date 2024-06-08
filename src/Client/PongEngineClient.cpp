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
    Texture2D texture = { 0 };



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
            // Save the first 10 frames, the frame is converted to RGBA by the encoder
            if(frameDecoder->getFrameIndex() <= 10)
            {
                frameDecoder->saveFrametoPng(frame, AV_PIX_FMT_RGBA);
            }

            // TODO:: Somehow this does not work, yet the frames are saved to png succesfully
            // It could be about saveFramePng converting the frame again
            // with SwsContext and sws_scale
            Image image = {
                .data = frame->data[0],
                .width = frame->width,
                .height = frame->height,
                .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
            };
            texture = LoadTextureFromImage(image);

            DrawTexture(texture, 0, 0, WHITE);

            av_frame_free(&frame);
        }
        EndDrawing();
    }

    UnloadTexture(texture);

    return 0;
}