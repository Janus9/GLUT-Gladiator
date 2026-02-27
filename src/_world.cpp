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

    delete initBenchmark;
    initBenchmark = nullptr;
}

void _world::initWorld()
{
    initBenchmark->startBenchmark();

        Logger.LogInfo("Initializing world for seed " + to_string(seed), LOG_BOTH);
        Logger.LogInfo("World has " + to_string(numStartingChunks) + " starting chunks.", LOG_BOTH);

        tileAtlas->loadTexture("images/world_tiles_atlas.png"); // Load the tile atlas texture
        // Reserve allocates memory but does not instantiate it -- resize allocates AND instantiates it (dont want that)
        worldChunks.reserve(numStartingChunks); // Resize the vector to hold numStartingChunks chunks
        
        initTiles(); // Setup tiles
        
        uniform_int_distribution<int> dist(0, 3); 

        double sqrtNumChunks = sqrt(numStartingChunks);
        // This checks if a decimal (like 1.3) is equal to its floor (1.0) which indicates the sqrt wasn't perfect
        if (sqrtNumChunks != floor(sqrtNumChunks)) {
            Logger.LogWarning("numStartingChunks is not a perfect square. This may lead to an uneven distribution of chunks around the center.", LOG_BOTH);
        }

        // Initialize world tiles and chunks here
        for (int i = 0; i < numStartingChunks; i++) {

            int chunkX = i % (int)sqrt(numStartingChunks) - floor(sqrt(numStartingChunks) / 2); // Calculate chunkX based on index
            int chunkY = i / (int)sqrt(numStartingChunks) - floor(sqrt(numStartingChunks) / 2); // Calculate chunkY based on index

            generateChunk(chunkX, chunkY); // Generate the chunk at the calculated coordinates
        } 

    initBenchmark->clickBenchmark();
    double time = initBenchmark->getAverageResult();
    Logger.LogInfo("World initialization for " + to_string(worldChunks.size()) + "chunks took " + to_string(time) + "ms");
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
    int numTilesPerRow = 256 / TILE_W;     // This gives us how many tiles are in a single row
    int numTilesPerCol = 256 / TILE_H;     // This gives us how many tiles are in a single column

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
    float u0 = (tileNumX * TILE_W) / 256.0f;
    // Left (Y)
    float v0 = (tileNumY * TILE_H) / 256.0f;
    // Right (X)
    float u1 = u0 + (TILE_W / 256.0f);
    // Right (Y)
    float v1 = v0 + (TILE_H / 256.0f);

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

    loadedChunks[{new_chunkX, new_chunkY}] = true;  // Using pair directly

    //Logger.LogDebug("Generating new chunk at (" + std::to_string(new_chunkX) + ", " + std::to_string(new_chunkY) + ")", LOG_BOTH);

    _chunk newChunk;
    newChunk.chunkX = new_chunkX;
    newChunk.chunkY = new_chunkY;

    worldChunks.push_back(newChunk);

    for (int i = 0; i < 256; i++) {
        worldChunks.back().tileData[i] = dist(rng); // Randomly assign tile type 0, 1, 2, or 3
    }

    chunkLookup[{new_chunkX, new_chunkY}] = &worldChunks.back();  // Using pair directly
}

void _world::buildChunkVBO(_chunk* chunk) {
    // 1 tile = 4 vertices, 1 vertex = 4 floats (2 for vertex, 2 for texture) = 16
    // 256 tiles * 16 from above gives 4096
    float vboData[4096];
    int index = 0;

    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            int tileIndex = y * 16 + x;
            uint8_t tileType = chunk->tileData[tileIndex];
            _tile* tile = &world_tiles[tileType];
            
            float worldX = (chunk->chunkX * 16 + x) * TILE_W;
            float worldY = (chunk->chunkY * 16 + y) * TILE_H;

            // The VBO is set up identical to how we would do glVertex2f and glTexCoord2f

            // Bottom-left
            vboData[index++] = worldX;
            vboData[index++] = worldY;
            vboData[index++] = tile->u0;
            vboData[index++] = tile->v1;
            
            // Bottom-right
            vboData[index++] = worldX + TILE_W;
            vboData[index++] = worldY;
            vboData[index++] = tile->u1;
            vboData[index++] = tile->v1;
            
            // Top-right
            vboData[index++] = worldX + TILE_W;
            vboData[index++] = worldY + TILE_H;
            vboData[index++] = tile->u1;
            vboData[index++] = tile->v0;
            
            // Top-left
            vboData[index++] = worldX;
            vboData[index++] = worldY + TILE_H;
            vboData[index++] = tile->u0;
            vboData[index++] = tile->v0;
        }
    }

    // VBO uninitialized if 0
    if (chunk->vboID == 0) {
        glGenBuffers(1, &chunk->vboID); // Create an ID for the GPU buffer (only done once)
    }

    glBindBuffer(GL_ARRAY_BUFFER, chunk->vboID); // Working with this specific buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(vboData), vboData, GL_STATIC_DRAW); // Copy the system memory buffer (vboData) into a GPU memory buffer
    glBindBuffer(GL_ARRAY_BUFFER,0); // Make sure to set vbo to 0 (nothing) to signal we're done working with this vboID

    chunk->vboDirty = false;
}

void _world::drawWorld(float left, float right, float top, float bottom)
{
    // Since its an atlas we only need one text bind. Each tile takes a snippit of the atlas
    tileAtlas->bindTexture();

    // Tell OpenGL were using VBOs
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    // Calculate which chunks are visible
    int minChunkX = (int)floor(left / (16 * TILE_W));
    int maxChunkX = (int)ceil(right / (16 * TILE_W));
    int minChunkY = (int)floor(bottom / (16 * TILE_H));
    int maxChunkY = (int)ceil(top / (16 * TILE_H));

    for (int chunkY = minChunkY; chunkY < maxChunkY; chunkY++) {
        for (int chunkX = minChunkX; chunkX < maxChunkX; chunkX++) {
            // Check if we loaded the given chunk
            if (!isChunkLoaded(chunkX, chunkY)) { continue; }
            
            // Uses unordered map to get a reference to the chunk (since we arent iterating through entire vector)
            _chunk* chunk = getChunk(chunkX, chunkY);
            if (chunk == nullptr) { continue; }

            // If chunk is dirty (so tiles changed) call a rebuild
            if (chunk->vboDirty) {
                buildChunkVBO(chunk);
            }

            // Draw VBO
            glBindBuffer(GL_ARRAY_BUFFER, chunk->vboID);
            glVertexPointer(2, GL_FLOAT, 4 * sizeof(float), (void*)0);
            glTexCoordPointer(2, GL_FLOAT, 4 * sizeof(float), (void*)(2 * sizeof(float)));
            glDrawArrays(GL_QUADS, 0, 256 * 4);  // 256 tiles * 4 vertices
        }
    }

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    // Displays chunk borders in red when enabled -- kind of broken
    if (displayChunkBorders) {
        glPushAttrib(GL_CURRENT_BIT | GL_LINE_BIT); // saves current GL state for color/line wdidth
            glColor3f(1.0f, 0.0f, 0.0f); // Red color for borders
            glLineWidth(2.0f); // Thicker lines for visibility
            glBegin(GL_LINE_LOOP);
                for(int chunkNum = 0; chunkNum < worldChunks.size(); chunkNum++) {
                    _chunk* chunk = &worldChunks[chunkNum];

                    float left = chunk->chunkX * 16 * TILE_W;
                    float right = left + 16 * TILE_W;
                    float top = chunk->chunkY * 16 * TILE_H;
                    float bottom = top + 16 * TILE_H;
                    
                    glVertex2f(left, top);
                    glVertex2f(right, top);
                    glVertex2f(right, bottom);
                    glVertex2f(left, bottom);
                }
            glEnd();
        glPopAttrib();
    }
}

bool _world::isChunkLoaded(int chunkX, int chunkY) {
    return loadedChunks.find({chunkX, chunkY}) != loadedChunks.end();
}

_chunk* _world::getChunk(int chunkX, int chunkY) {
    auto it = chunkLookup.find({chunkX,chunkY});
    if (it != chunkLookup.end()) {
        return it->second;
    }
    return nullptr;
}

void _world::debugPrint() {

    size_t ChunkBytes = sizeof(_chunk) * worldChunks.size();
    size_t TileBytes = sizeof(uint8_t) * worldChunks.size() * 256;

    Logger.LogInfo(" -- World Debug Print -- ", LOG_CONSOLE);
    Logger.LogInfo(
        "Chunks Loaded: " 
        + std::to_string(worldChunks.size()) 
        + " (" + std::to_string(ChunkBytes) + " B)" 
        + " (" + std::to_string(ChunkBytes/1000000) + "MB)", 
        LOG_CONSOLE
    );
    Logger.LogInfo(
        "Tiles Loaded: " 
        + std::to_string(worldChunks.size() * 256) 
        + " (" + std::to_string(TileBytes) + " B)" 
        + " (" + std::to_string(TileBytes/1000000) + "MB)", 
        LOG_CONSOLE);
    Logger.LogInfo("------------------------", LOG_CONSOLE);
}