#include <_world.h>

_world::_world()
{
    //ctor
}

_world::~_world()
{
    //dtor
    delete tileAtlas;
    tileAtlas = nullptr;
}

void _world::initWorld()
{
    Logger.LogInfo("Initializing world for seed " + to_string(seed), LOG_BOTH);

    tileAtlas->loadTexture("images/world_tiles_atlas.png"); // Load the tile atlas texture
    worldChunks.resize(9); // Resize the vector to hold 9 chunks (3x3 grid around the player)

    uniform_int_distribution<int> dist(0, 3); 

    // Initialize world tiles and chunks here
    for (int i = 0; i < worldChunks.size(); i++) {
        worldChunks[i].chunkX = (i % 3) - 1; // -1, 0, 1
        worldChunks[i].chunkY = (i / 3) - 1; // -1, 0, 1

        loadedChunks.insert({ChunkPosToKey(worldChunks[i].chunkX, worldChunks[i].chunkY),true});

        Logger.LogInfo("Initialized chunk at (" + std::to_string(worldChunks[i].chunkX) + ", " + std::to_string(worldChunks[i].chunkY) + ")", LOG_CONSOLE);
    } 

    for (int i = 0; i < 256; i++) {
        for(int chunkNum = 0; chunkNum < 9; chunkNum++) {
            worldChunks[chunkNum].tileData[i] = dist(rng); // Randomly assign tile type 0 or 1 or 2 or 3                                    
        }
        world_tiles[i].pixelW = 16; // Assuming each tile is 16x16 pixels in the atlas
        world_tiles[i].pixelH = 16;
    }

    initTiles(); // Setup tiles
}

void _world::initTiles() {
    // Grass Variants //
    setTileInAtlas(0, world_tiles[0]); 
    setTileInAtlas(1, world_tiles[1]); 
    setTileInAtlas(2, world_tiles[2]); 
    setTileInAtlas(3, world_tiles[3]); 
    // Empty Tiles //
}

bool _world::setTileInAtlas(int tileNum, _tile &tile) {
        
    // Will (for now) give us 16x16 tiles

    // Error check for divide by 0
    if (tile.pixelW == 0 || tile.pixelH == 0) 
    {
        Logger.LogError("Tile pixel width and height must be greater than zero. Check tile initialization.", LOG_BOTH);
        return false;
    }

    int numTilesPerRow = 256 / tile.pixelW;     // This gives us how many tiles are in a single row
    int numTilesPerCol = 256 / tile.pixelH;     // This gives us how many tiles are in a single column

    // Error check for mod by 0 
    if (numTilesPerRow == 0) {
        Logger.LogError("Number of tiles per row cannot be zero. Check tile pixel width.", LOG_BOTH);
        return false;
    }

    int tileNumX = tileNum % numTilesPerRow;    // This gives us which column the tile is in
    int tileNumY = tileNum / numTilesPerRow;    // This gives us which row the tile is in (using integer division to round down)
    /*
        All of these values have to be between 0 and 1. This is because for glTexCoord2f we use this to assign an image from 
        0 (start of image) to 1 (end of image). Since we have 16 tiles per row/column we divide by 16.

        ex/ to get tileNum 0 (the very first tile) we need coordinates from 0-0.0625 as 0.0625 is 1/16
    */

    // Left (X)
    float u0 = (tileNumX * tile.pixelW) / 256.0f;
    // Left (Y)
    float v0 = (tileNumY * tile.pixelH) / 256.0f;
    // Right (X)
    float u1 = u0 + (tile.pixelW / 256.0f);
    // Right (Y)
    float v1 = v0 + (tile.pixelH / 256.0f);

    Logger.LogDebug("Tile " + std::to_string(tileNum) + " atlas coordinates: (" + std::to_string(u0) + ", " + std::to_string(v0) + ") to (" + std::to_string(u1) + ", " + std::to_string(v1) + ")", LOG_CONSOLE);

    tile.u0 = u0;
    tile.v0 = v0;
    tile.u1 = u1;
    tile.v1 = v1;

    return true;
}

void _world::generateChunk(int new_chunkX, int new_chunkY) 
{
    if (isChunkLoaded(new_chunkX, new_chunkY)) {
        Logger.LogWarning("Chunk at (" + std::to_string(new_chunkX) + ", " + std::to_string(new_chunkY) + ") is already loaded. Skipping generation.", LOG_BOTH);
        return; // Chunk is already loaded, no need to generate
    }

    uniform_int_distribution<int> dist(0, 3); 

    loadedChunks.insert({ChunkPosToKey(new_chunkX, new_chunkY),true});

    Logger.LogDebug("Generating new chunk at (" + std::to_string(new_chunkX) + ", " + std::to_string(new_chunkY) + ")", LOG_BOTH);

    _chunk newChunk;
    newChunk.chunkX = new_chunkX;
    newChunk.chunkY = new_chunkY;

    worldChunks.push_back(newChunk);

    for (int i = 0; i < 256; i++) {
        worldChunks.back().tileData[i] = dist(rng); // Randomly assign tile type 0, 1, 2, or 3
    }
}

void _world::drawWorld()
{
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
                tileAtlas->bindTexture();

                // Draws from bottom left
                glBegin(GL_QUADS);
                    // bottom-left
                    glTexCoord2f(tile.u0, tile.v1); glVertex2f(worldX, worldY);
                    // bottom-right
                    glTexCoord2f(tile.u1, tile.v1); glVertex2f(worldX + TILE_W, worldY);
                    // top-right
                    glTexCoord2f(tile.u1, tile.v0); glVertex2f(worldX + TILE_W, worldY + TILE_H);
                    // top-left
                    glTexCoord2f(tile.u0, tile.v0); glVertex2f(worldX, worldY + TILE_H);
                glEnd();
            }
        }
    }
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

void _world::debugPrint() {
    Logger.LogInfo(" -- World Debug Print -- ", LOG_CONSOLE);
    Logger.LogInfo("Chunks Loaded: " + std::to_string(worldChunks.size()) + " (" + std::to_string(sizeof(_chunk) * worldChunks.size()) + " B)", LOG_CONSOLE);
    Logger.LogInfo("Tiles Loaded: " + std::to_string(worldChunks.size() * 256) + " (" + std::to_string(sizeof(uint8_t) * worldChunks.size() * 256) + " B)", LOG_CONSOLE);
    Logger.LogInfo("------------------------", LOG_CONSOLE);
}