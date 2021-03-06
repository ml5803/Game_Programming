#pragma once
#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "Util.h"
#include "Entity.h"
#include "Map.h"

struct GameState {
    Map *map;
    Entity *player;
    Entity *enemies;
    int lives = 3;
    int nextScene = -1;
    bool win;
    Mix_Music *musicList[3];
    Mix_Chunk *soundList[2];
};

class Scene {
public:
    GameState state;
    glm::mat4 viewMatrix;
    GLuint fontTextureID;
    virtual void Initialize() = 0;
    virtual void Update(float deltaTime) = 0;
    virtual void Render(ShaderProgram *program) = 0;
};
