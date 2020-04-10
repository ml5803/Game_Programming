#include "Level1.h"

#define LEVEL1_WIDTH 16
#define LEVEL1_HEIGHT 8

#define ENEMY_COUNT 1

unsigned int level1_data[] =
{
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 3,
    3, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 3,
    3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3,
};


void Level1::Initialize() {
    state.nextScene = -1;
    
    GLuint mapTextureID = Util::LoadTexture("tileset.png");
    state.map = new Map(LEVEL1_WIDTH, LEVEL1_HEIGHT, level1_data, mapTextureID, 1.0f, 4, 1);

    // Initialize Game Objects
    //make player
    state.player = new Entity();
    state.player->position = glm::vec3(1,-2,0);
    state.player->movement = glm::vec3(0);
    state.player->acceleration = glm::vec3(0,-10.0f,0);
    state.player->speed = 1.75f;
    state.player->textureID = Util::LoadTexture("george_0.png");
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
    state.player->animCols = 4;
    state.player->animRows = 4;
    
    //make map
    state.map = new Map(LEVEL1_WIDTH, LEVEL1_HEIGHT, level1_data, mapTextureID, 1.0f, 4, 1);
    
    //make enemies
    GLuint goombaTextureID = Util::LoadTexture("goomba.png");
    state.enemies = new Entity[ENEMY_COUNT];
    
    //patrol
    state.enemies[0].position = glm::vec3(7,-3,0);
    state.enemies[0].aiType = PATROL;
    state.enemies[0].aiState = WALKING;
    state.enemies[0].movement = glm::vec3(-1,0,0);
    
    for (int i = 0; i < ENEMY_COUNT; i++){
        state.enemies[i].acceleration = glm::vec3(0,-10.0f,0);
        state.enemies[i].height = 1.0f;
        state.enemies[i].width = 0.8f;
        state.enemies[i].speed = 1.0f;
        state.enemies[i].textureID = goombaTextureID;
        state.enemies[i].animWalk = new int[2] {0,1};
        state.enemies[i].animIndices = state.enemies[0].animWalk;
        state.enemies[i].animFrames = 2;
        state.enemies[i].animIndex = 0;
        state.enemies[i].animTime = 0;
        state.enemies[i].animCols = 3;
        state.enemies[i].animRows = 1;
        
        state.enemies[i].entityType = ENEMY;
    }
    
    fontTextureID = Util::LoadTexture("font.png");
}
    
void Level1::Update(float deltaTime) {
    state.player->Update(deltaTime, state.player, state.enemies, ENEMY_COUNT, state.map, &state.lives);
    
    for (int i = 0; i < ENEMY_COUNT; i++){
        state.enemies[i].Update(deltaTime, state.player, state.enemies, ENEMY_COUNT, state.map, &state.lives);
    }
    
    viewMatrix = glm::mat4(1.0f);
    if (state.player->position.x < 5){
        viewMatrix = glm::translate(viewMatrix, glm::vec3(-5, 3.75, 0));
    }else if (state.player->position.x >= 5 && state.player->position.x <= 10){
        viewMatrix = glm::translate(viewMatrix, glm::vec3(-state.player->position.x, 3.75, 0));
    }else{
        viewMatrix = glm::translate(viewMatrix, glm::vec3(-10, 3.75, 0));
    }
    
    //if entered cave, next level
    if (state.player->position.x >= 15){
        state.nextScene = 2;
    }
}
void Level1::Render(ShaderProgram *program) {
    state.map->Render(program);
    state.player->Render(program);
    
    for (int i = 0; i < ENEMY_COUNT; i++){
        state.enemies[i].Render(program);
    }
    
    float textOffset;
    if (state.player->position.x < 5){
        textOffset = 0;
    }else if (state.player->position.x >= 5 && state.player->position.x <= 10){
        textOffset = state.player->position.x-5;
    }else{
        textOffset = 5;
    }
    
    Util::DrawText(program, fontTextureID,"Lives: " + std::to_string(state.lives),0.3f, 0.0f, glm::vec3(0.75+textOffset,-.5,0));
    
    if (state.lives <= 0){
        Util::DrawText(program, fontTextureID,"You lose :(..." ,0.45f, 0.0f, glm::vec3(state.player->position.x-2,-2.5,0));
    }
}
