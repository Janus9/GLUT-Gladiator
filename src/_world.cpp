#include <_world.h>

_world::_world()
{
    //ctor
}

_world::~_world()
{
    //dtor
    delete chunk;
    chunk = nullptr;

    delete tileTexture;
    tileTexture = nullptr;
}

void _world::initWorld()
{
    // Initialize world tiles and chunks here
    chunk->chunkX = 0;
    chunk->chunkY = 0;
    
    world_tiles[0].textureId = tileTexture->loadTexture("images/missing_texture.png");   
    world_tiles[1].textureId = tileTexture->loadTexture("images/orange.png");   
    
    for (int i = 0; i < 256; i++) {
        chunk->tileData[i] = i % 2; // Alternate between tile type 0 and 1 for testing

        world_tiles[i].pixelW = 16; // Assuming each tile is 16x16 pixels in the atlas
        world_tiles[i].pixelH = 16;
    }
}

void _world::drawWorld()
{
    glEnable(GL_TEXTURE_2D);
    
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

    glDisable(GL_TEXTURE_2D);
}

