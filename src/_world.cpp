#include <_world.h>

_world::_world()
{
    //ctor
}

_world::~_world()
{
    //dtor
    delete tileTexture;
    tileTexture = nullptr;
}

void _world::initWorld()
{
    // Initialize world tiles and chunks here
    for (int i = 0; i < 9; i++) {
        worldChunks[i].chunkX = (i % 3) - 1; // -1, 0, 1
        worldChunks[i].chunkY = (i / 3) - 1; // -1, 0, 1

        Logger.LogInfo("Initialized chunk at (" + std::to_string(worldChunks[i].chunkX) + ", " + std::to_string(worldChunks[i].chunkY) + ")", LOG_CONSOLE);
    }
    
    world_tiles[0].textureId = tileTexture->loadTexture("images/grass_variant_one.png");   
    world_tiles[1].textureId = tileTexture->loadTexture("images/grass_variant_two.png");   
    
    for (int i = 0; i < 256; i++) {
        for(int chunkNum = 0; chunkNum < 9; chunkNum++) {
            worldChunks[chunkNum].tileData[i] = i % 2; // Alternate between tile type 0 and 1 for testing
        }
        world_tiles[i].pixelW = 16; // Assuming each tile is 16x16 pixels in the atlas
        world_tiles[i].pixelH = 16;
    }
}

void _world::drawWorld()
{
    glEnable(GL_TEXTURE_2D);
    
    for(int chunkNum = 0; chunkNum < 9; chunkNum++) {
        _chunk* chunk = &worldChunks[chunkNum];
        for (int y = 0; y < 16; y++) {
            for (int x = 0; x < 16; x++) {
                int index = y * 16 + x;
                uint8_t tileType = chunk->tileData[index];
                
                // Get the tile definition
                _tile& tile = world_tiles[tileType];
                
                // Calculate world position
                float worldX = (chunk->chunkX * 16 + x) * TILE_W;
                float worldY = (chunk->chunkY * 16 + y) * TILE_H;
                
                // Bind texture
                glBindTexture(GL_TEXTURE_2D, tile.textureId);
                
                // Draw the tile quad
                glBegin(GL_QUADS);
                    glTexCoord2f(0.0f, 0.0f); glVertex2f(worldX, worldY);
                    glTexCoord2f(1.0f, 0.0f); glVertex2f(worldX + TILE_W, worldY);
                    glTexCoord2f(1.0f, 1.0f); glVertex2f(worldX + TILE_W, worldY + TILE_H);
                    glTexCoord2f(0.0f, 1.0f); glVertex2f(worldX, worldY + TILE_H);
                glEnd();
            }
        }
    }
    glDisable(GL_TEXTURE_2D);
}

