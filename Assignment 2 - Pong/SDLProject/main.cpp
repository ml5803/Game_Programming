#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

SDL_Window* displayWindow;
bool gameIsRunning = true;
bool takingInput = true;
bool gameStarted = false;

ShaderProgram program;
glm::mat4 viewMatrix, projectionMatrix, player1Matrix, player2Matrix, ballMatrix;

GLuint player1TextureID, player2TextureID, ballTextureID;

float player1y = 0;
float player2y = 0;

glm::vec3 player1Movement = glm::vec3(0);
glm::vec3 player2Movement = glm::vec3(0);
glm::vec3 ballMovement = glm::vec3(0);
float step = 5;

glm::vec3 ballPosition;

GLuint LoadTexture(const char* filepath){
    int w, h, n;
    unsigned char* image = stbi_load(filepath, &w, &h, &n, STBI_rgb_alpha);
    
    if (image == NULL){
        std::cout << "Unable to load image. Make sure path is correct \n";
        assert(false);
    }
    
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE,image);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    stbi_image_free(image);
    return textureID;
}

void Initialize() {
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("Pong!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(0, 0, 640*2, 480*2);
    
    program.Load("shaders/vertex_textured.glsl", "shaders/fragment_textured.glsl");
    
    viewMatrix = glm::mat4(1.0f);
    projectionMatrix = glm::ortho(-5.0f,5.0f,-3.75f,3.75f,-1.0f,1.0f);
    
    player1Matrix = glm::mat4(1.0f);
    player1Matrix = glm::translate(player1Matrix, glm::vec3(-4.75f, 0.0f,0.0f));
    player2Matrix = glm::mat4(1.0f);
    player2Matrix = glm::translate(player2Matrix, glm::vec3(4.75f, 0.0f,0.0f));
    ballMatrix = glm::mat4(1.0f);
    
    program.SetProjectionMatrix(projectionMatrix);
    program.SetViewMatrix(viewMatrix);
    
    glUseProgram(program.programID);
    
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    
    //blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    player1TextureID = LoadTexture("red_player.png");
    player2TextureID = LoadTexture("blue_player.png");
    ballTextureID = LoadTexture("ball.png");
}

void ProcessInput() {
    player1Movement = glm::vec3(0);
    player2Movement = glm::vec3(0);
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type){
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                gameIsRunning = false;
                break;
            
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym){
                    //player 1 movement
                    case SDLK_w:
                        player1Movement.y = 1.0f;
                        break;
                    case SDLK_s:
                        player1Movement.y = -1.0f;
                        break;
                    case SDLK_UP:
                        player2Movement.y = 1.0f;
                        break;
                    case SDLK_DOWN:
                        player2Movement.y = -1.0f;
                        break;
                    case SDLK_SPACE:
                        if (!gameStarted){
                            ballMovement = glm::vec3(1,1,0);
                            gameStarted = true;
                            break;
                        }
                }
                break;
        }
    }
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    if (keys[SDL_SCANCODE_W]){
        player1Movement.y = 1.0f;
    }
    else if (keys[SDL_SCANCODE_S]){
        player1Movement.y = -1.0f;
    }
    
    if (keys[SDL_SCANCODE_UP]){
        player2Movement.y = 1.0f;
    }
    else if (keys[SDL_SCANCODE_DOWN]){
        player2Movement.y = -1.0f;
    }
    
    if (glm::length(player1Movement) > 1.0f){
        player1Movement = glm::normalize(player1Movement);
    }
    
    if (glm::length(player2Movement) > 1.0f){
        player2Movement = glm::normalize(player2Movement);
    }
}

bool detectBoxCollision(const glm::mat4& player, const glm::mat4& ball){
    //xdiff = abs(x2-x1), ydiff = abs(y2-y1)
    //xdist = xdiff - (w1+w2)/2, ydist = ydiff - (h1+h2)/2
    //if xdist <0 and ydist < 0 then colliding, else no.
        
    // [3][0] is x, [3][1] is y
    float x1 = player[3][0];
    float x2 = ball[3][0];
    float y1 = player[3][1];
    float y2 = ball[3][1];
    
    float w1 = 0.1f;
    float w2 = 0.25f;
    float h1 = 0.5f;
    float h2 = 0.5f;
    
    float xdist = fabs(x2 - x1) - ((w1+w2)/2.0f);
    float ydist = fabs(y2 - y1) - ((h1+h2)/2.0f);

    std::cout << "player:" << x1 << "," << y1 << std::endl;
    std::cout << "ball:" << x2 << "," << y2 << std::endl;
    
    if (xdist < 0 && ydist < 0){
        return true;
    }else{
        return false;
    }
}

bool detectWallCollision(const glm::mat4& obj){
    float upper, lower;
    upper = 3.6;
    lower = -3.6;
    if (obj[3][1] >= upper || obj[3][1] <= lower){
        return true;
    }else{
        return false;
    }
}

bool detectGameOver(){
    if (ballMatrix[3][0] < -4.70 || ballMatrix[3][0] > 4.70){
        return true;
    }else{
        return false;
    }
}

float lastTicks = 0.0f;

void Update() {
    float ticks = (float)SDL_GetTicks() / 1000.0f;
    float deltaTime = ticks - lastTicks;
    lastTicks = ticks;
    
    player1Matrix = glm::translate(player1Matrix,player1Movement * step * deltaTime);
    player2Matrix = glm::translate(player2Matrix,player2Movement * step * deltaTime);
    
    //if player out of bound, bring them back!
    if (player1Matrix[3][1] > 3.5f){
        player1Matrix[3][1] = 3.5f;
    }
    if (player1Matrix[3][1] < -3.5f){
        player1Matrix[3][1] = -3.5f;
    }
    if (player2Matrix[3][1] > 3.5f){
        player2Matrix[3][1] = 3.5f;
    }
    if (player2Matrix[3][1] < -3.5f){
        player2Matrix[3][1] = -3.5f;
    }
    
    ballMatrix = glm::translate(ballMatrix, ballMovement * (step/3) * deltaTime);
    
    //if collide with either paddle, flip x velocity
    if (detectBoxCollision(player1Matrix,ballMatrix) || detectBoxCollision(player2Matrix,ballMatrix)){
        ballMovement.x *= -1;
    }
    // if collide with wall, flip y velocity
    if (detectWallCollision(ballMatrix)){
        ballMovement.y *= -1;
    }
    // if leave screen, ball stops moving and goes to center, game over, restart
    if (detectGameOver()){
        ballMatrix = glm::mat4(1.0f);
        ballMovement = glm::vec3(0,0,0);
        gameStarted = false;
    }
    
}

void Render() {
    glClear(GL_COLOR_BUFFER_BIT);
    
    //player 1
    program.SetModelMatrix(player1Matrix);
    
    float player1Vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5};
    float player1TexCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0};
    
    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, player1Vertices);
    glEnableVertexAttribArray(program.positionAttribute);
    glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, player1TexCoords);
    glEnableVertexAttribArray(program.texCoordAttribute);
    
    glBindTexture(GL_TEXTURE_2D, player1TextureID);
    glDrawArrays(GL_TRIANGLES,0,6);
    
    //player 2
    program.SetModelMatrix(player2Matrix);
    
    float player2Vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5};
    float player2TexCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0};
    
    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, player2Vertices);
    glEnableVertexAttribArray(program.positionAttribute);
    glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, player2TexCoords);
    glEnableVertexAttribArray(program.texCoordAttribute);
    
    glBindTexture(GL_TEXTURE_2D, player2TextureID);
    glDrawArrays(GL_TRIANGLES,0,6);
    
    //ball
    program.SetModelMatrix(ballMatrix);
    
    float ballVertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5};
    float ballTexCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0};
    
    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, ballVertices);
    glEnableVertexAttribArray(program.positionAttribute);
    glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, ballTexCoords);
    glEnableVertexAttribArray(program.texCoordAttribute);
    
    glBindTexture(GL_TEXTURE_2D, ballTextureID);
    glDrawArrays(GL_TRIANGLES,0,6);
    
    SDL_GL_SwapWindow(displayWindow);
}

void Shutdown() {
    SDL_Quit();
}

int main(int argc, char* argv[]) {
    Initialize();
    
    while (gameIsRunning) {
        ProcessInput();
        Update();
        Render();
    }
    
    Shutdown();
    return 0;
}
