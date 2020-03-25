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

#include "Entity.h"
#include <vector>

#define PLATFORM_COUNT 21
#define ENEMY_COUNT 3
struct GameState {
    Entity *player;
    Entity *platforms;
    Entity *enemies;
    GLuint font1TextureID;
    int gameState = 0; //-1 for lose, 1 for win, 0 for ongoing
};

GameState state;

SDL_Window* displayWindow;
bool gameIsRunning = true;

ShaderProgram program;
glm::mat4 viewMatrix, modelMatrix, projectionMatrix;

GLuint LoadTexture(const char* filePath) {
    int w, h, n;
    unsigned char* image = stbi_load(filePath, &w, &h, &n, STBI_rgb_alpha);
    
    if (image == NULL) {
        std::cout << "Unable to load image. Make sure the path is correct\n" << filePath;
        //assert(false);
    }
    
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    stbi_image_free(image);
    return textureID;
}

void DrawText(ShaderProgram *program, GLuint fontTextureID, std::string text, float size, float spacing, glm::vec3 position)
{
    float width = 1.0f / 16.0f;
    float height = 1.0f / 16.0f;

    std::vector<float> vertices;
    std::vector<float> texCoords;

    for(int i = 0; i < text.size(); i++) {
        int index = (int)text[i];
        float offset = (size + spacing) * i;
        
        float u = (float)(index % 16) / 16.0f;
        float v = (float)(index / 16) / 16.0f;
        
        vertices.insert(vertices.end(), {
            offset + (-0.5f * size), 0.5f * size,
            offset + (-0.5f * size), -0.5f * size,
            offset + (0.5f * size), 0.5f * size,
            offset + (0.5f * size), -0.5f * size,
            offset + (0.5f * size), 0.5f * size,
            offset + (-0.5f * size), -0.5f * size,
        });
        texCoords.insert(texCoords.end(), {
            u, v,
            u, v + height,
            u + width, v,
            u + width, v + height,
            u + width, v,
            u, v + height,
        });

    } // end of for loop
    glBindTexture(GL_TEXTURE_2D, fontTextureID);
    
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, position);
    program->SetModelMatrix(modelMatrix);

    glUseProgram(program->programID);

    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices.data());
    glEnableVertexAttribArray(program->positionAttribute);

    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords.data());
    glEnableVertexAttribArray(program->texCoordAttribute);

    glBindTexture(GL_TEXTURE_2D, fontTextureID);
    glDrawArrays(GL_TRIANGLES, 0, (int)(text.size() * 6));

    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}

bool AllEnemiesDead(Entity* enemies){
    for (int i = 0; i < ENEMY_COUNT; i++){
        if (enemies[i].isActive){
            return false;
        }
    }
    return true;
}

void Initialize() {
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("AI Platformer!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(0, 0, 640*2, 480*2);
    
    program.Load("shaders/vertex_textured.glsl", "shaders/fragment_textured.glsl");
    
    viewMatrix = glm::mat4(1.0f);
    modelMatrix = glm::mat4(1.0f);
    projectionMatrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    
    program.SetProjectionMatrix(projectionMatrix);
    program.SetViewMatrix(viewMatrix);
    
    glUseProgram(program.programID);
    
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glEnable(GL_BLEND);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
   
    // Initialize Game Objects
    
    //make player
    state.player = new Entity();
    state.player->position = glm::vec3(-4,-2,0);
    state.player->movement = glm::vec3(0);
    state.player->acceleration = glm::vec3(0,-10.0f,0);
    state.player->speed = 1.75f;
    state.player->textureID = LoadTexture("george_0.png");
    state.player->height = 0.8f;
    state.player->width = 0.60f;
    state.player->entityType = PLAYER;
    //player animations
    state.player->animRight = new int[4] {3,7,11,15};
    state.player->animLeft = new int[4] {1,5,9,13};
    state.player->animIndices = state.player->animRight;
    state.player->animFrames = 4;
    state.player->animIndex = 0;
    state.player->animTime = 0;
    state.player->animCols =4;
    state.player->animRows = 4;
    
    //make enemies
    state.enemies = new Entity[ENEMY_COUNT];
    
    //patrol
    state.enemies[0].position = glm::vec3(3.5,2,0);
    state.enemies[0].aiType = PATROL;
    state.enemies[0].aiState = WALKING;
    
    //chaser
    state.enemies[1].position = glm::vec3(-3.5,1,0);
    state.enemies[1].aiType = CHASE;
    state.enemies[1].aiState = WALKING;
    
    //runner
    state.enemies[2].position = glm::vec3(-2,4.5,0);
    state.enemies[2].aiType = RUN;
    state.enemies[2].aiState = IDLE;
    
    
    GLuint goombaTextureID = LoadTexture("goomba.png");
    for (int i = 0; i < ENEMY_COUNT; i++){
        state.enemies[i].acceleration =glm::vec3(0,-10.0f,0);
        state.enemies[i].width = 1.0f;
        state.enemies[i].speed = 1.0f;
        state.enemies[i].textureID = goombaTextureID;
        state.enemies[i].animWalk = new int[2] {0,1};
        state.enemies[i].animIndices = state.enemies[0].animWalk;
        state.enemies[i].animFrames = 2;
        state.enemies[i].animIndex = 0;
        state.enemies[i].animTime = 0;
        state.enemies[i].animCols = 3;
        state.enemies[i].animRows = 1;
    }
    
    //make stage
    GLuint platformTextureID = LoadTexture("platformPack_tile001.png");
    GLuint platform2TextureID = LoadTexture("platformPack_tile004.png");
    state.font1TextureID = LoadTexture("font.png");
    state.platforms = new Entity[PLATFORM_COUNT];
    
    //base ground
    for (int i = 0; i < 10; i++){
        if (i == 5){
            state.platforms[i].textureID = platform2TextureID;
        }
        else{
            state.platforms[i].textureID = platformTextureID;
        }
        state.platforms[i].position = glm::vec3(-4.5f+i,-3.25,0);
    }
    state.platforms[20].position = glm::vec3(0.5,-2.25,0);
    state.platforms[20].textureID = platformTextureID;
    
    
    //platform 1
    for (int i = 10; i < 13; i++){
        state.platforms[i].position = glm::vec3(-4.5+i-10,-1.0,0);
        state.platforms[i].textureID = platformTextureID;
    }
    
    //platform 2
    for (int i = 13; i < 18; i++){
        state.platforms[i].position = glm::vec3(1.5+i-13,0,0);
        state.platforms[i].textureID = platformTextureID;
    }
    
    //platform 3
    for (int i = 18; i < 20; i++){
        state.platforms[i].position = glm::vec3(-2.5+i-18,1.5,0);
        state.platforms[i].textureID = platformTextureID;
    }
    
    //update locations
    for (int i = 0; i < PLATFORM_COUNT; i++){
        state.platforms[i].entityType = PLATFORM;
        state.platforms[i].Update(0,state.player,NULL,NULL,0,0);
    }
    
    //update enemies
    for (int i = 0; i < ENEMY_COUNT; i++){
        state.enemies[i].entityType = ENEMY;
        state.enemies[i].width = 0.8f;
        state.enemies[i].Update(0, state.player, state.enemies, state.platforms, ENEMY_COUNT, PLATFORM_COUNT);
    }
}

void ProcessInput() {
    //reset player movement
    state.player->movement.x = 0.0f;
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                gameIsRunning = false;
                break;
                
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_LEFT:
                        // Move the player left
                        state.player->animIndices = state.player->animLeft;
                        break;
                    case SDLK_RIGHT:
                        // Move the player right
                        state.player->animIndices = state.player->animRight;
                        break;
                    case SDLK_SPACE:
                        //only jump if on a platform
                        if (state.player->collidedBottom){
                            state.player->velocity.y = 7.0f;
                        }
                        break;
                }
                break; // SDL_KEYDOWN
        }
    }
    
    const Uint8 *keys = SDL_GetKeyboardState(NULL);

    if (keys[SDL_SCANCODE_LEFT]) {
        state.player->movement.x = -1.0f;
    }
    else if (keys[SDL_SCANCODE_RIGHT]) {
       state.player->movement.x = 1.0f;
    }
}

#define FIXED_TIMESTEP 0.0166666f
float lastTicks = 0;
float accumulator = 0.0f;
void Update() {
    float ticks = (float)SDL_GetTicks() / 1000.0f;
    float deltaTime = ticks - lastTicks;
    lastTicks = ticks;

    deltaTime += accumulator;
    if (deltaTime < FIXED_TIMESTEP) {
        accumulator = deltaTime;
        return;
    }

    while (deltaTime >= FIXED_TIMESTEP) {
        // Update. Notice it's FIXED_TIMESTEP. Not deltaTime
        state.player->Update(FIXED_TIMESTEP,state.player, state.enemies, state.platforms,ENEMY_COUNT, PLATFORM_COUNT);
        
        for (int i = 0; i < ENEMY_COUNT; i++){
            state.enemies[i].Update(FIXED_TIMESTEP,state.player, state.enemies, state.platforms,ENEMY_COUNT, PLATFORM_COUNT);
        }
        deltaTime -= FIXED_TIMESTEP;
    }

    accumulator = deltaTime;
    
    if (!state.player->isActive){
        state.gameState = -1;
    }else if (AllEnemiesDead(state.enemies)){
        state.gameState = 1;
    }
}

void Render() {
    glClear(GL_COLOR_BUFFER_BIT);

    for (int i = 0; i < PLATFORM_COUNT; i++){
        state.platforms[i].Render(&program);
    }
    
    for (int i = 0; i < ENEMY_COUNT; i++){
        state.enemies[i].Render(&program);
    }
    state.player->Render(&program);
    
    if (state.gameState == -1){
        DrawText(&program, state.font1TextureID,"Defeat",0.45f, 0.0001f, glm::vec3(-1.25,1,0));
        DrawText(&program, state.font1TextureID,"Try again?",0.45f, 0.0001f, glm::vec3(-2,0,0));
    }else if (state.gameState == 1){
        DrawText(&program, state.font1TextureID,"You win!",0.45f, 0.0001f, glm::vec3(-1.5,0,0));
    }
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
