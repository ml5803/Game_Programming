#include "Entity.h"

Entity::Entity()
{
    position = glm::vec3(0);
    movement = glm::vec3(0);
    acceleration = glm::vec3(0);
    velocity = glm::vec3(0);
    speed = 0;
    lastCollided = DUMMY;
    modelMatrix = glm::mat4(1.0f);
}

bool Entity::CheckCollision(Entity *other)
{
    if (isActive == false || other->isActive == false){
        return false;
    }
    
    //we don't want to check for collision from a wall or landingpad view, will check from player
    if (entityType == PLATFORM){
        return false;
    }
    
    float xdist = fabs(position.x - other->position.x) - ((width + other->width)/2.0f);
    float ydist = fabs(position.y - other->position.y) - ((height + other->height)/2.0f);
    
    if (xdist < 0 && ydist < 0){
        //detected a collision
        lastCollided = other->entityType;
        return true;
    }
    
    return false;
}

void Entity::CheckCollisionPointLeft(Entity* objects, int objectCount){
    float leftCornerX;
    float leftCornerY;
    
    leftCornerX = position.x - (width/2) - offset;
    leftCornerY = position.y - (height/2) - offset;
    
    for (int i = 0; i < objectCount; i++)
    {
        Entity *object = &objects[i];
        float maxX = object->position.x + (object->width)/2;
        float minX = object->position.x - (object->width)/2;
        float maxY = object->position.y + (object->height)/2;
        float minY = object->position.y - (object->height)/2;
    
        //if leftCornerX is between X's and leftCornerY is between Y's, that point is in bounds
        
        if (leftCornerX >= minX && leftCornerX <= maxX){
            if (leftCornerY >= minY && leftCornerY <= maxY){
                leftCornerContact = true;
            }
        }
    }
}

void Entity::CheckCollisionPointRight(Entity* objects, int objectCount){
    float rightCornerX;
    float rightCornerY;
    
    rightCornerX = position.x + (width/2) + offset;
    rightCornerY = position.y - (height/2) - offset;
    for (int i = 0; i < objectCount; i++)
    {
        Entity *object = &objects[i];
        float maxX = object->position.x + (object->width)/2;
        float minX = object->position.x - (object->width)/2;
        float maxY = object->position.y + (object->height)/2;
        float minY = object->position.y - (object->height)/2;
    
        //if rightCornerX is between X's and rightCornerY is between Y's, that point is in bounds
        if (rightCornerX >= minX && rightCornerX <= maxX){
            if (rightCornerY >= minY && rightCornerY <= maxY){
                rightCornerContact = true;
            }
        }
    }
}

void Entity::CheckCollisionsX(Entity *objects, int objectCount)
{
     for (int i = 0; i < objectCount; i++)
     {
         Entity *object = &objects[i];

             if (CheckCollision(object))
             {
                 float xdist = fabs(position.x - object->position.x);
                 float penetrationX = fabs(xdist - (width / 2.0f) - (object->width / 2.0f));
                 if (velocity.x > 0) {
                 position.x -= penetrationX;
                 velocity.x = 0;
                 collidedRight = true;
             }
                 else if (velocity.x < 0) {
                 position.x += penetrationX;
                 velocity.x = 0;
                 collidedLeft = true;
             }
         }
     }
}

void Entity::CheckCollisionsY(Entity *objects, int objectCount)
{
     for (int i = 0; i < objectCount; i++)
     {
         Entity *object = &objects[i];
         if (CheckCollision(object))
         {
             float ydist = fabs(position.y - object->position.y);
             float penetrationY = fabs(ydist - (height / 2.0f) - (object->height / 2.0f));
             if (velocity.y > 0) {
                 position.y -= penetrationY;
                 velocity.y = 0;
                 collidedTop = true;
             }
             else if (velocity.y < 0) {
                 position.y += penetrationY;
                 velocity.y = 0;
                 collidedBottom = true;
             }
         }
     }
}

void Entity::Update(float deltaTime, Entity* player, Entity* enemies, Entity* platforms, int enemyCount, int platformCount)
{
    if (isActive == false){
        return;
    }
    
    //check if approaching a cliff
    CheckCollisionPointLeft(platforms, platformCount);
    CheckCollisionPointRight(platforms, platformCount);
    
    if (entityType == ENEMY){
        AI(player);
    }
    
    //animations if needed
    if (animIndices != NULL) {
       if (glm::length(movement) != 0) {
           animTime += deltaTime;

           if (animTime >= 0.25f)
           {
               animTime = 0.0f;
               animIndex++;
               if (animIndex >= animFrames)
               {
                   animIndex = 0;
               }
           }
       } else {
           animIndex = 0;
       }
   }
    
    //if player
    collidedTop = false;
    collidedBottom = false;
    collidedLeft = false;
    collidedRight = false;
    leftCornerContact = false;
    rightCornerContact = false;
    
    velocity.x = movement.x * speed;
    //only count acceleration when you in air aka collidedBottom is false
    if (!collidedBottom){
        velocity += acceleration * deltaTime;
    }
    
    position.y += velocity.y * deltaTime; // Move on Y
    CheckCollisionsY(platforms, platformCount);// Fix if needed
    
    position.x += velocity.x * deltaTime; // Move on X
    CheckCollisionsX(platforms, platformCount);// Fix if needed

    //fix bounds to never leave the screen...
    if (position.x > 4.7){
        position.x = 4.7;
    }else if (position.x < -4.7){
        position.x = -4.7;
    }
    
    //if enemy check collisions
    if (entityType == ENEMY){
        if(CheckCollision(player) && player->position.y >= position.y +0.3){ //if I touch the player and he is higher, he stepped on my head
            isActive = false;
            player->velocity.y = 3.0f;
        }else if (CheckCollision(player)){
            player->isActive = false;
        }
    }
    
    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, position);
}

void Entity::DrawSpriteFromTextureAtlas(ShaderProgram *program, GLuint textureID, int index)
{
    float u = (float)(index % animCols) / (float)animCols;
    float v = (float)(index / animCols) / (float)animRows;
    
    float width = 1.0f / (float)animCols;
    float height = 1.0f / (float)animRows;
    
    float texCoords[] = { u, v + height, u + width, v + height, u + width, v,
        u, v + height, u + width, v, u, v};
    
    float vertices[]  = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
    
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->positionAttribute);
    
    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
    glEnableVertexAttribArray(program->texCoordAttribute);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}

void Entity::Render(ShaderProgram *program) {
    
    if (isActive == false){
        return;
    }
    
    program->SetModelMatrix(modelMatrix);
    
    if (animIndices != NULL) {
        DrawSpriteFromTextureAtlas(program, textureID, animIndices[animIndex]);
        return;
    }
    
    float vertices[]  = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
    float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
    
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program->positionAttribute);
    
    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
    glEnableVertexAttribArray(program->texCoordAttribute);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}

void Entity::AI(Entity* player){
    switch (aiType){
        case PATROL:
            AIPatrol(player);
            break;
        case CHASE:
            AIChase(player);
            break;
        case RUN:
            AIRunner(player);
            break;
    }
}

void Entity::AIPatrol(Entity* player){
    //gap on the left side
    if(!leftCornerContact){
        movement = glm::vec3(1,0,0);
    }
    
    //gap on the right side
    if(!rightCornerContact){
        movement = glm::vec3(-1,0,0);
    }
}

void Entity::AIChase(Entity* player){
    if (aiState == WALKING){
        //jump if player on a higher platform and you are a platform,
        //either you touched a wall or you approached a gap
        if ((player->position.y > position.y && collidedBottom) && (collidedLeft || collidedRight || !leftCornerContact || !rightCornerContact)){
            velocity.y = 7.0f;
        }
        //temporarily increase in speed while in air
        if (velocity.y > 0.0f){
            speed = 1.0f;
        }else{
            speed = 1.0f;
        }

        
        if (player->position.x > position.x){ // player on right
            movement = glm::vec3(1,0,0);
        }else if (player->position.x < position.x){
            movement = glm::vec3(-1,0,0);
        }else{
            movement.x = 0;
        }
    }
}
    

void Entity::AIRunner(Entity* player){
    if (glm::distance(position, player->position) < 2.5f){ //don't move until player is close enough
        aiState = WALKING;
    }
    
    if (aiState == WALKING){
        if (player->position.x > position.x){//player on right
            movement = glm::vec3(-1,0,0);
        }else{
            movement = glm::vec3(1,0,0);
        }
        
        if(glm::distance(position, player->position) > 3.0f){
            movement = glm::vec3(0);
            aiState = IDLE;
        }
    }
}
