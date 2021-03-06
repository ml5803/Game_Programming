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

enum EntityType {DUMMY,PLAYER,PLATFORM,ENEMY};
enum AIType {PATROL, CHASE, RUN};
enum AIState {IDLE, WALKING};

class Entity {
public:
    EntityType entityType;
    AIType aiType;
    AIState aiState;
    
    EntityType lastCollided;
    
    glm::vec3 position;
    glm::vec3 movement;
    glm::vec3 acceleration;
    glm::vec3 velocity;
    
    float width = 1.0f;
    float height = 1.0f;
    float offset = 0.2;
    
    float speed;
    
    GLuint textureID;
    
    glm::mat4 modelMatrix;
    
    int *animRight = NULL;
    int *animLeft = NULL;
    int *animUp = NULL;
    int *animDown = NULL;
    int *animWalk = NULL;

    int *animIndices = NULL;
    int animFrames = 0;
    int animIndex = 0;
    float animTime = 0;
    int animCols = 0;
    int animRows = 0;
    
    bool isActive = true;
    
    bool collidedTop = false;
    bool collidedBottom=  false;
    bool collidedLeft = false;
    bool collidedRight = false;
    bool leftCornerContact = false;
    bool rightCornerContact = false;
    
    Entity();
    
    bool CheckCollision(Entity *other);
    void CheckCollisionsX(Entity *objects, int objectCount);
    void CheckCollisionsY(Entity *objects, int objectCount);
    void CheckCollisionPointLeft(Entity* objects, int objectCount);
    void CheckCollisionPointRight(Entity* objects, int objectCount);
    void Update(float deltaTime, Entity* player, Entity* enemies, Entity* platforms, int enemyCount, int platformCount);
    void Render(ShaderProgram *program);
    void DrawSpriteFromTextureAtlas(ShaderProgram *program, GLuint textureID, int index);
    
    void AI(Entity* player);
    void AIPatrol(Entity* player);
    void AIChase(Entity* player);
    void AIRunner(Entity* player);
}; 
