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
    //can't collide with self or something that isn't active
    if (isActive == false || other->isActive == false || other == this){
        return false;
    }
    
    //we don't want to check for collision from a wall, will check from player or enemy
    if (entityType == PLATFORM || entityType == DUMMY){
        return false;
    }
    
    float xdist = fabs(position.x - other->position.x) - ((width + other->width)/2.0f);
    float ydist = fabs(position.y - other->position.y) - ((height + other->height)/2.0f);
    
    if (xdist <= 0.05 && ydist <= 0.5){
        //detected a collision
        lastCollided = other->entityType;
        return true;
    }
    
    return false;
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

void Entity::CheckCollisionsX(Map *map)
{
    // Probes for tiles
    glm::vec3 left = glm::vec3(position.x - (width / 2), position.y, position.z);
    glm::vec3 right = glm::vec3(position.x + (width / 2), position.y, position.z);

    float penetration_x = 0;
    float penetration_y = 0;
    if (map->IsSolid(left, &penetration_x, &penetration_y) && velocity.x < 0) {
        position.x += penetration_x;
        velocity.x = 0;
        collidedLeft = true;
    }

    if (map->IsSolid(right, &penetration_x, &penetration_y) && velocity.x > 0) {
        position.x -= penetration_x;
        velocity.x = 0;
        collidedRight = true;
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

void Entity::CheckCollisionsY(Map *map)
{
    // Probes for tiles
    glm::vec3 top = glm::vec3(position.x, position.y + (height / 2), position.z);
    glm::vec3 top_left = glm::vec3(position.x - (width / 2), position.y + (height / 2), position.z);
    glm::vec3 top_right = glm::vec3(position.x + (width / 2), position.y + (height / 2), position.z);

    glm::vec3 bottom = glm::vec3(position.x, position.y - (height / 2), position.z);
    glm::vec3 bottom_left = glm::vec3(position.x - (width / 2), position.y - (height / 2), position.z);
    glm::vec3 bottom_right = glm::vec3(position.x + (width / 2), position.y - (height / 2), position.z);

    float penetration_x = 0;
    float penetration_y = 0;
    if (map->IsSolid(top, &penetration_x, &penetration_y) && velocity.y > 0) {
        position.y -= penetration_y;
        velocity.y = 0;
        collidedTop = true;
        lastCollided = PLATFORM;
    }
    else if (map->IsSolid(top_left, &penetration_x, &penetration_y) && velocity.y > 0) {
        position.y -= penetration_y;
        velocity.y = 0;
        collidedTop = true;
        collidedTopLeft = true;
        lastCollided = PLATFORM;
    }
    else if (map->IsSolid(top_right, &penetration_x, &penetration_y) && velocity.y > 0) {
        position.y -= penetration_y;
        velocity.y = 0;
        collidedTop = true;
        collidedTopRight = true;
        lastCollided = PLATFORM;
    }
    
    if (map->IsSolid(bottom, &penetration_x, &penetration_y) && velocity.y < 0) {
        position.y += penetration_y;
        velocity.y = 0;
        collidedBottom = true;
        lastCollided = PLATFORM;
    }
    else if (map->IsSolid(bottom_left, &penetration_x, &penetration_y) && velocity.y < 0) {
        position.y += penetration_y;
        velocity.y = 0;
        collidedBottom = true;
        collidedBottomLeft = true;
        lastCollided = PLATFORM;
    }
    else if (map->IsSolid(bottom_right, &penetration_x, &penetration_y) && velocity.y < 0) {
        position.y += penetration_y;
        velocity.y = 0;
        collidedBottom = true;
        collidedBottomRight = true;
        lastCollided = PLATFORM;
    }
}

void Entity::Update(float deltaTime, Entity* player, Entity* objects, int objectCount, Map* map, int *lives)
{
    if (isActive == false){
        return;
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
    
    collidedTop = false;
    collidedTopLeft = false;
    collidedTopRight = false;
    collidedBottom = false;
    collidedBottomLeft = false;
    collidedBottomRight = false;
    collidedLeft = false;
    collidedRight = false;
    
    velocity.x = movement.x * speed;
    //only count acceleration when you in air aka collidedBottom is false
    if (!collidedBottom){
        velocity += acceleration * deltaTime;
    }
    
    position.y += velocity.y * deltaTime; // Move on Y
    CheckCollisionsY(map);
    CheckCollisionsY(objects, objectCount);// Fix if needed
    
    position.x += velocity.x * deltaTime; // Move on X
    CheckCollisionsX(map);
    CheckCollisionsX(objects, objectCount);// Fix if needed
    
    if (entityType == ENEMY){
//        std::cout << "top    " << collidedTopLeft << collidedTop << collidedTopRight << std::endl;
//        std::cout << "middle " << collidedLeft << collidedRight << std::endl;
//        std::cout << "bottom " << collidedBottomLeft << collidedBottom << collidedBottomRight << std::endl;
//        std::cout << "last collided " << lastCollided << std::endl;
        AI(player);
    }
    
    //if enemy check collisions
    if (entityType == ENEMY){
        if(CheckCollision(player) && player->position.y >= position.y + 0.5f){ //if I touch the player and he is higher, he stepped on my head
            player->velocity.y = 5;
            isActive = false;
        }else if (CheckCollision(player)){
            *lives -= 1;
            isActive = false;
        }
    }
    
    //if player runs out of lives, deactivate player
    if (entityType == PLAYER && *lives <= 0){
        isActive = false;
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
    //wall on left
    if(collidedLeft){
        movement = glm::vec3(1,0,0);
    }
    //wall on right
    if(collidedRight){
        movement = glm::vec3(-1,0,0);
    }
}

void Entity::AIChase(Entity* player){
    
    if (glm::distance(position, player->position) < 4.0f){ //don't move until player is close enough
        aiState = WALKING;
    }else{
        aiState = IDLE;
    }
    
    if (aiState == WALKING){
        //jump if player on a higher platform and you are a platform,
        //either you touched a wall or you approached a gap
        if ((player->position.y > position.y && collidedBottom) && (collidedLeft || collidedRight)){
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
