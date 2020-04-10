#include "Menu.h"

#define MENU_WIDTH 11
#define MENU_HEIGHT 8

#define ENEMY_COUNT 0

unsigned int menu_data[] =
{
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3,
    3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3,
    3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3,
};

void Menu::Initialize() {
    state.nextScene = -1;
    
    GLuint mapTextureID = Util::LoadTexture("tileset.png");
    state.map = new Map(MENU_WIDTH, MENU_HEIGHT, menu_data, mapTextureID, 1.0f, 4, 1);

    // Initialize Game Objects
    //make player
    state.player = new Entity();
    state.player->position = glm::vec3(5,0,0);
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
    state.player->animCols =4;
    state.player->animRows = 4;
    
    //make map
    state.map = new Map(MENU_WIDTH, MENU_HEIGHT, menu_data, mapTextureID, 1.0f, 4, 1);
    
    //load font
    fontTextureID = Util::LoadTexture("font.png");
}
void Menu::Update(float deltaTime) {
    state.player->Update(deltaTime, state.player, state.enemies, ENEMY_COUNT, state.map, &state.lives);
    
    //check if enter is pressed
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    if (keys[SDL_SCANCODE_RETURN]) {
        state.nextScene = 1;
    }
    
    viewMatrix = glm::mat4(1.0f);
    viewMatrix = glm::translate(viewMatrix, glm::vec3(-5, 3.75, 0));
}
void Menu::Render(ShaderProgram *program) {
    state.map->Render(program);
    state.player->Render(program);
    
    Util::DrawText(program, fontTextureID,"Adventure Time!",0.6f, 0.0f, glm::vec3(0.75,-.5,0));
    Util::DrawText(program, fontTextureID,"Press Enter to Begin",0.45f, 0.0001f, glm::vec3(0.75,-1.5,0));
}
