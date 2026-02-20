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
    Logger.LogInfo("Initializing world for seed " + to_string(seed), LOG_BOTH);
    worldChunks.resize(9); // Resize the vector to hold 9 chunks (3x3 grid around the player)

    uniform_int_distribution<int> dist(0, 2); 

    // Initialize world tiles and chunks here
    for (int i = 0; i < worldChunks.size(); i++) {
        worldChunks[i].chunkX = (i % 3) - 1; // -1, 0, 1
        worldChunks[i].chunkY = (i / 3) - 1; // -1, 0, 1

        loadedChunks.insert({ChunkPosToKey(worldChunks[i].chunkX, worldChunks[i].chunkY),true});

        Logger.LogInfo("Initialized chunk at (" + std::to_string(worldChunks[i].chunkX) + ", " + std::to_string(worldChunks[i].chunkY) + ")", LOG_CONSOLE);
    } 

    generateChunk(); // generate test chunk
    
    for (int i = 0; i < 256; i++) {
        for(int chunkNum = 0; chunkNum < 9; chunkNum++) {
            worldChunks[chunkNum].tileData[i] = dist(rng); // Randomly assign tile type 0 or 1 
        }
        world_tiles[i].pixelW = 16; // Assuming each tile is 16x16 pixels in the atlas
        world_tiles[i].pixelH = 16;
    }
        
    world_tiles[0].textureId = tileTexture->loadTexture("images/grass_variant_one.png");   
    world_tiles[1].textureId = tileTexture->loadTexture("images/grass_variant_two.png");   
    world_tiles[2].textureId = tileTexture->loadTexture("images/grass_variant_three.png");  
}

void _world::generateChunk() 
{
    //todo -- test add chunk to right
    uniform_int_distribution<int> dist(0, 2); 

    int new_chunkX = 2;
    int new_chunky = 0;

    loadedChunks.insert({ChunkPosToKey(new_chunkX, new_chunky),true});

    Logger.LogDebug("Generating new chunk at (" + std::to_string(new_chunkX) + ", " + std::to_string(new_chunky) + ")", LOG_BOTH);

    _chunk newChunk;
    newChunk.chunkX = new_chunkX;
    newChunk.chunkY = new_chunky;

    worldChunks.push_back(newChunk);

    for (int i = 0; i < 256; i++) {
        worldChunks.back().tileData[i] = dist(rng); // Randomly assign tile type 0, 1, or 2
    }
}

void _world::drawWorld()
{
    glEnable(GL_TEXTURE_2D);
    
    for(int chunkNum = 0; chunkNum < worldChunks.size(); chunkNum++) {
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
                
                // Draws from bottom left
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

    // Displays chunk borders in red when enabled
    if (displayChunkBorders) {
        glLineWidth(2.0f); // Thicker lines for visibility
        glColor3f(1.0f, 0.0f, 0.0f); // Red color for borders
        for(int chunkNum = 0; chunkNum < worldChunks.size(); chunkNum++) {
            _chunk& chunk = worldChunks[chunkNum];
            float left = chunk.chunkX * 16 * TILE_W;
            float right = left + 16 * TILE_W;
            float top = chunk.chunkY * 16 * TILE_H;
            float bottom = top + 16 * TILE_H;
            
            glBegin(GL_LINE_LOOP);
                glVertex2f(left, top);
                glVertex2f(right, top);
                glVertex2f(right, bottom);
                glVertex2f(left, bottom);
            glEnd();
        }
        glLineWidth(1.0f); // Reset line width
    }   
}

bool _world::isChunkLoaded(int chunkX, int chunkY) {
    string key = ChunkPosToKey(chunkX, chunkY);
    return loadedChunks.find(key) != loadedChunks.end();
}