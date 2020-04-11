#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1

#include <SDL_mixer.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"

#include <vector>

#include "Scene.h"
#include "Menu.h"
#include "Level1.h"
#include "Level2.h"
#include "Level3.h"

SDL_Window* displayWindow;
ShaderProgram program;
glm::mat4 modelMatrix, projectionMatrix;

GameState state;
int lives = 3;
bool gameIsRunning = true;

Scene *currentScene;
Scene *sceneList[4];

void SwitchToScene(Scene *scene) {
     currentScene = scene;
     currentScene->Initialize();
}

void SwitchMusic(int sceneNum){
    switch (sceneNum){
        case 0:
            Mix_PlayMusic(state.musicList[0],-1);
            break;
        case 1:
            Mix_PlayMusic(state.musicList[1],-1);
            break;
        case 2:
            //Mix_PlayMusic(state.musicList[1],-1);
            break;
        case 3:
            Mix_PlayMusic(state.musicList[2],-1);
            break;
    }
}

void Initialize() {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    displayWindow = SDL_CreateWindow("Platformer!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(0, 0, 640*2, 480*2);
    
    program.Load("shaders/vertex_textured.glsl", "shaders/fragment_textured.glsl");
    
    modelMatrix = glm::mat4(1.0f);
    projectionMatrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    
    program.SetProjectionMatrix(projectionMatrix);
    
    glUseProgram(program.programID);
    
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glEnable(GL_BLEND);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 4096);
    state.musicList[0] = Mix_LoadMUS("New Hero in Town.mp3");
    state.musicList[1] = Mix_LoadMUS("Blippy Trance.mp3");
    state.musicList[2] = Mix_LoadMUS("Circus of Freaks.mp3");
    
    Mix_VolumeMusic(MIX_MAX_VOLUME / 3);
    
    state.soundList[0] = Mix_LoadWAV("jump.wav");
    state.soundList[1] = Mix_LoadWAV("hurt.wav");
    
    sceneList[0] = new Menu();
    sceneList[1] = new Level1();
    sceneList[2] = new Level2();
    sceneList[3] = new Level3();
    SwitchToScene(sceneList[0]);
    SwitchMusic(0);
}

void ProcessInput() {
    currentScene->state.player->movement.x = 0.0f;
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                gameIsRunning = false;
                break;
                
            //only allow movement if game is running
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_LEFT:
                        // Move the player left
                        if (!currentScene->state.win){
                           currentScene->state.player->animIndices = currentScene->state.player->animLeft;
                        }
                        break;
                    case SDLK_RIGHT:
                        // Move the player right
                        if (!currentScene->state.win){
                          currentScene->state.player->animIndices = currentScene->state.player->animRight;
                        }
                        break;
                    case SDLK_SPACE:
                        //only jump if on a platform
                        if (!currentScene->state.win){
                            if (currentScene->state.player->collidedBottom){
                                Mix_PlayChannel(-1, state.soundList[0], 0);
                                currentScene->state.player->velocity.y = 7.0f;
                            }
                            break;
                        }
                }
                break; // SDL_KEYDOWN
        }
    }
    
    //only allow movement if I didn't win yet.
    if (currentScene->state.win){
        return;
    }
    
    //play sound if damaged
    if (lives != currentScene->state.lives){
        Mix_PlayChannel(-1,state.soundList[1], 0);
        lives = currentScene->state.lives;
    }
    
    const Uint8 *keys = SDL_GetKeyboardState(NULL);

    if (keys[SDL_SCANCODE_LEFT]) {
        currentScene->state.player->movement.x = -1.0f;
    }
    else if (keys[SDL_SCANCODE_RIGHT]) {
        currentScene->state.player->movement.x = 1.0f;
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
        currentScene->Update(FIXED_TIMESTEP);
        deltaTime -= FIXED_TIMESTEP;
    }

    accumulator = deltaTime;
}

void Render() {
    glClear(GL_COLOR_BUFFER_BIT);
    
    program.SetViewMatrix(currentScene->viewMatrix);
    
    currentScene->Render(&program);

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
        
        if (currentScene->state.nextScene >= 0){
            //lives shared across scenes
            sceneList[currentScene->state.nextScene]->state.lives = currentScene->state.lives;
            SwitchMusic(currentScene->state.nextScene);
            SwitchToScene(sceneList[currentScene->state.nextScene]);
        }
        
        Render();
    }
    
    Shutdown();
    return 0;
}
