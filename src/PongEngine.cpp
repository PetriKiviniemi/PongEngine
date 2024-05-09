#include <raylib.h>
#include <iostream>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <raymath.h>
#include <math.h>
#include <string>
#include <vector>
#include <random>
#include <FrameEncoder.hpp>
#include <GL/gl.h>
#include <common.hpp>

#define MAX_SCORE 1

typedef struct Pad{
    Vector2 pos;
    int w,h;
    Color color;
};

typedef struct Ball{
    Vector2 pos;
    int r;
    Color color;
    Vector2 dir;
    float vel;
};

bool checkCollision(Pad &player, Ball &ball)
{
    // AABB Collision
    float playerLeft = player.pos.x - player.w / 2.0f;
    float playerRight = player.pos.x + player.w / 2.0f;
    float playerTop = player.pos.y - player.h / 2.0f;
    float playerBottom = player.pos.y + player.h / 2.0f;

    float ballLeft = ball.pos.x - ball.r;
    float ballRight = ball.pos.x + ball.r;
    float ballTop = ball.pos.y - ball.r;
    float ballBottom = ball.pos.y + ball.r;

    bool collisionX = playerRight >= ballLeft && ballRight >= playerLeft;
    bool collisionY = playerBottom >= ballTop && ballBottom >= playerTop;

    return collisionX && collisionY;
}

Vector2 calculateHorReflection(Vector2 dir)
{
    Vector2 wallN{0.0, 1.0};
    float dotP = Vector2DotProduct(dir, wallN);
    //Prevet the ball from going straight up or down
    if(dotP > 0.92)
        dotP -= 0.05;
    else if(dotP < -0.92)
        dotP += 0.05;
    return Vector2Subtract(dir, Vector2Scale(wallN, (2 * dotP)));
}

Vector2 calculateVerReflection(Vector2 dir)
{
    //TODO:: We could add the velocity of the wall here
    //So that we can bounce the ball in steeper angle when the wall is moving
    Vector2 wallN{1.0, 0.0};
    float dotP = Vector2DotProduct(dir, wallN);
    return Vector2Subtract(dir, Vector2Scale(wallN, (2 * dotP)));
}

Vector2 getRandomBallDir()
{
    std::random_device rd;
    std::mt19937 gen(rd());    
    std::uniform_real_distribution<float> dist(-180.0f * DEG2RAD, 180.0f * DEG2RAD);


    float angle = dist(gen);

    float dx = std::cos(angle);
    float dy = std::sin(angle);

    return Vector2Normalize(Vector2{ dx, dy });
}

enum GameState{
    MENU,
    GAMEOVER,
    PLAYING
};

int main()
{
    //Lets make pong
    InitWindow(WINDOW_HEIGHT, WINDOW_WIDTH, "Pong");
    SetTargetFPS(60);

    //Random starting direction for ball
    srand(time(NULL));

    //Initialize the entities
    Pad player{40, 400, 15, 200, WHITE};
    Pad computer{760, 400, 15, 200, WHITE};
    Ball ball{400, 400, 20, WHITE, getRandomBallDir(), 6};

    //UI things
    int playerScore, computerScore;
    playerScore = 0;
    computerScore = 0;
    GameState state = MENU;
    
    std::vector<std::string> menuElements{"Start", "Quit"};
    int selectedMenuElemIdx = 0;

    FrameEncoder* frameEncoder = FrameEncoder::GetInstance();
    RenderTexture2D target = LoadRenderTexture(WINDOW_WIDTH, WINDOW_HEIGHT);

    while(WindowShouldClose() == false)
    {
        if(playerScore == MAX_SCORE || computerScore == MAX_SCORE)
        {
            playerScore = 0;
            computerScore = 0;
            state = GAMEOVER;
        }
        //Event handling
        if(state == MENU)
        {
            if(IsKeyPressed(KEY_W))
            {
                selectedMenuElemIdx += 1;
                if (selectedMenuElemIdx >= menuElements.size())
                    selectedMenuElemIdx = 0;

            }

            if(IsKeyPressed(KEY_S))
            {
                selectedMenuElemIdx -= 1;
                if (selectedMenuElemIdx < 0)
                    selectedMenuElemIdx = menuElements.size() - 1;
            }

            if(IsKeyPressed(KEY_ENTER))
            {
                if(menuElements[selectedMenuElemIdx] == "Start")
                {
                    state = PLAYING;
                }

                if(menuElements[selectedMenuElemIdx] == "Quit")
                {
                    break;
                }

            }
        }
        else if (state == PLAYING)
        {
            if(IsKeyDown(KEY_S))
            {
                if(player.pos.y < WINDOW_HEIGHT - player.h / 2)
                {
                    player.pos.y += 4;
                }
            }

            if(IsKeyDown(KEY_W))
            {
                if(player.pos.y > 0 + player.h / 2)
                {
                    player.pos.y -= 4;
                }
            }

            if(IsKeyPressed(KEY_ESCAPE))
            {
                break;
            }

            //Update positions
            //Collision for walls
            if(ball.pos.x < ball.r / 2) 
            {
                computerScore += 1;
                ball.pos = Vector2{400, 400};
                ball.dir = getRandomBallDir();
            }

            if(ball.pos.x >= WINDOW_WIDTH - ball.r / 2)
            {
                playerScore += 1;
                ball.pos = Vector2{400, 400};
                ball.dir = getRandomBallDir();
            }

            if(ball.pos.y < ball.r / 2 || ball.pos.y >= WINDOW_HEIGHT - ball.r / 2)
            {
                ball.dir = calculateHorReflection(ball.dir);
            }

            //Collisition for the pads
            //Have to make a collision box for the pads
            if(checkCollision(player, ball))
            {
                ball.dir = calculateVerReflection(ball.dir);
                //Add a small offset
                ball.pos.x += 4;
            }

            if(checkCollision(computer, ball))
            {
                ball.dir = calculateVerReflection(ball.dir);
                //Add a small offset
                ball.pos.x -= 4;
            }

            ball.pos = Vector2Add(ball.pos, Vector2Scale(ball.dir, ball.vel));

            //Update the computer pad position to the direction the ball is coming
            if (computer.pos.y > ball.pos.y)
            {
                computer.pos.y -= 1;
            }

            else if(computer.pos.y < ball.pos.y)
            {
                computer.pos.y += 1;
            }
        }
        else if(state == GAMEOVER)
        {
            if(IsKeyPressed(KEY_ENTER))
            {
                state = MENU;
            }
        }

        //Draw stuff 
        //First draw everything onto a texture, then draw the texture onto screen
        //Since that's the only way to access the raw pixel data

        //TODO:: We have to draw another texture but flipped
        BeginTextureMode(target);
        BeginDrawing();
        ClearBackground(BLACK);

        if(state == MENU)
        {
            int x = 370;
            int startY = 300;
            for (int i = 0; i < menuElements.size(); i++)
            {
                if(i == selectedMenuElemIdx)
                {
                    DrawText(menuElements.at(i).c_str(), x, startY, 32, RED);
                }
                else
                {
                    DrawText(menuElements.at(i).c_str(), x, startY, 32, WHITE);
                }

                startY += 200;
            }
        }
        else if(state == PLAYING)
        {
            //Player and computer
            DrawCube(Vector3{player.pos.x, player.pos.y, 0}, player.w, player.h, 0, player.color);
            DrawCube(Vector3{computer.pos.x, computer.pos.y, 0}, computer.w, computer.h, 0, computer.color);
            DrawCircle(ball.pos.x, ball.pos.y, ball.r, ball.color);
            //UI
            DrawText(std::to_string(playerScore).c_str(), 70, 20, 32, WHITE);
            DrawText(std::to_string(computerScore).c_str(), 730, 20, 32, WHITE);
        }
        else if (state == GAMEOVER)
        {
            std::string gameOverText = "GameOver! Winner: ";
            gameOverText += playerScore == MAX_SCORE ? "Player" : "Computer";
            gameOverText += "\n Press ENTER to continue";
            DrawText(gameOverText.c_str(), 180, 400, 32, WHITE);
        }

        //Send the Image data to the encoder
        EndTextureMode();
        int posX = (WINDOW_WIDTH - target.texture.width) / 2;
        int posY = (WINDOW_HEIGHT - target.texture.height) / 2;

        DrawTexturePro(target.texture,
                       (Rectangle){0, 0, target.texture.width, -target.texture.height}, // Source rectangle (flip vertically)
                       (Rectangle){posX, posY, target.texture.width, target.texture.height}, // Destination rectangle
                       Vector2Zero(),
                       0.0f,
                       WHITE);

        //Get the raw pixel data and send it to our encoder
        unsigned int textureID = target.texture.id;
        uint32_t textureSize =  target.texture.width * target.texture.height * 4; //RGBA
        uint8_t *pixelData = new uint8_t[textureSize];

        // Using raw OpenGL functions to bind the texture 
        // and get the pixel data out of it

        glBindTexture(GL_TEXTURE_2D, textureID);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelData);
        glBindTexture(GL_TEXTURE_2D, 0);

        flipPixelsVertically(pixelData, WINDOW_WIDTH, WINDOW_HEIGHT);

        //NOTE:: REMEMBER TO FREE THE PIXELDATA IN THE FrameEncoder
        frameEncoder->encodeAndAddToQueue(pixelData, textureSize);

        EndDrawing();

    }

    frameEncoder->cleanUp();
    UnloadRenderTexture(target);
    CloseWindow();
    return 0;
}