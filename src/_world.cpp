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

        tileAtlas->loadTexture("images/set_1.png"); // Load the tile atlas texture
        // Reserve allocates memory but does not instantiate it -- resize allocates AND instantiates it (dont want that)
        worldChunks.reserve(numStartingChunks); // Resize the vector to hold numStartingChunks chunks
        world_noise.resize(numStartingChunks*256);  // 256 tiles per chunk

        initTiles(); // Setup tiles
        
        double sqrtNumChunks = sqrt(numStartingChunks);
        // This checks if a decimal (like 1.3) is equal to its floor (1.0) which indicates the sqrt wasn't perfect
        if (sqrtNumChunks != floor(sqrtNumChunks)) {
            Logger.LogWarning("numStartingChunks is not a perfect square. This may lead to an uneven distribution of chunks around the center.", LOG_BOTH);
        }

        // Initialize world tiles and chunks here
        runWorldGeneration(generation_iterations); 

    initBenchmark->clickBenchmark();
    double time = initBenchmark->getAverageResult();
    Logger.LogInfo("World initialization for " + to_string(worldChunks.size()) + "chunks took " + to_string(time) + "ms");
}

void _world::initTiles() {
    // FLOOR //
    setTileInAtlas(8,13, world_tiles[0]);       // Blank Floor
    setTileInAtlas(9,12, world_tiles[1]);       // Slightly Cracked Floor
    setTileInAtlas(10,13, world_tiles[2]);      // Medium Cracked Floor
    setTileInAtlas(8,12, world_tiles[3]);       // Square outlined floor
    setTileInAtlas(9,13, world_tiles[4]);       // Blank Floor 2
    // WALL //
    setTileInAtlas(22,16, world_tiles[5]);        // Wall Center

    setTileInAtlas(21,16, world_tiles[6]);        // Wall Left
    setTileInAtlas(23,16, world_tiles[7]);        // Wall Right
    setTileInAtlas(22,15, world_tiles[8]);        // Wall Up
    setTileInAtlas(22,17, world_tiles[9]);        // Wall Down

    setTileInAtlas(21,15, world_tiles[10]);        // Wall Corner Top Left
    setTileInAtlas(23,15, world_tiles[11]);        // Wall Corner Top Right
    setTileInAtlas(21,17, world_tiles[12]);        // Wall Corner Bottom Left
    setTileInAtlas(23,17, world_tiles[13]);        // Wall Corner Bottom Right

    setTileInAtlas(25,16, world_tiles[14]);        // Wall Island

    setTileInAtlas(25,15, world_tiles[15]);        // Wall Peninsula Top
    setTileInAtlas(25,17, world_tiles[16]);        // Wall Peninsula Down
    setTileInAtlas(24,16, world_tiles[17]);        // Wall Peninsula Left
    setTileInAtlas(26,16, world_tiles[18]);        // Wall Peninsula Right

    setTileInAtlas(22,18, world_tiles[19]);        // Wall Column Up
    setTileInAtlas(23,18, world_tiles[20]);        // Wall Column Side
}

bool _world::setTileInAtlas(int xIndex, int yIndex, _tile &tile) {
    int numTilesPerRow = 448 / TILE_W;     // This gives us how many tiles are in a single row
    int numTilesPerCol = 320 / TILE_H;     // This gives us how many tiles are in a single column

    // Error check for mod by 0 
    if (numTilesPerRow == 0) {
        Logger.LogError("Number of tiles per row cannot be zero. Check tile pixel width.", LOG_BOTH);
        return false;
    }

    /*
        All of these values have to be between 0 and 1. This is because for glTexCoord2f we use this to assign an image from 
        0 (start of image) to 1 (end of image). Since we have 16 tiles per row/column we divide by 16.

        ex/ to get tileNum 0 (the very first tile) we need coordinates from 0-0.0625 as 0.0625 is 1/16
    */
    // Left (X)
    float u0 = (xIndex * TILE_W) / 448.0f;
    // Left (Y)
    float v0 = (yIndex * TILE_H) / 320.0f;
    // Right (X)
    float u1 = u0 + (TILE_W / 448.0f);
    // Right (Y)
    float v1 = v0 + (TILE_H / 320.0f);

    Logger.LogDebug("Tile (" + to_string(xIndex) + ", " + to_string(yIndex) + " atlas coordinates: (" + to_string(u0) + ", " + to_string(v0) + ") to (" + to_string(u1) + ", " + to_string(v1) + ")", LOG_CONSOLE);

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

    uniform_int_distribution<int> dist(4, 5); 

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
    float tileVboData[4096];
    int index = 0;

    // Line VBO Data //
    float x0 = (chunk->chunkX * 16) * TILE_W; 
    float y0 = (chunk->chunkY * 16) * TILE_H;
    
    float x1 = (chunk->chunkX * 16 + 15) * TILE_W + TILE_W;
    float y1 = (chunk->chunkY * 16 + 15) * TILE_H + TILE_H;
    
    // The lines only require 2 values (x,y) per w/ no texCoords and 4 total = 8 values
    float lineVboData[8] {
        x0, y1, // Top Left
        x1, y1, // Top Right
        x1, y0, // Bottom Right
        x0, y0, // Bottom Left 
    };

    // Tile VBO Data //
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            int tileIndex = y * 16 + x;
            uint8_t tileType = chunk->tileData[tileIndex];
            _tile* tile = &world_tiles[tileType];
            
            float worldX = (chunk->chunkX * 16 + x) * TILE_W;
            float worldY = (chunk->chunkY * 16 + y) * TILE_H;

            // The VBO is set up identical to how we would do glVertex2f and glTexCoord2f

            // Bottom-left
            tileVboData[index++] = worldX;
            tileVboData[index++] = worldY;
            tileVboData[index++] = tile->u0;
            tileVboData[index++] = tile->v1;
            
            // Bottom-right
            tileVboData[index++] = worldX + TILE_W;
            tileVboData[index++] = worldY;
            tileVboData[index++] = tile->u1;
            tileVboData[index++] = tile->v1;
            
            // Top-right
            tileVboData[index++] = worldX + TILE_W;
            tileVboData[index++] = worldY + TILE_H;
            tileVboData[index++] = tile->u1;
            tileVboData[index++] = tile->v0;
            
            // Top-left
            tileVboData[index++] = worldX;
            tileVboData[index++] = worldY + TILE_H;
            tileVboData[index++] = tile->u0;
            tileVboData[index++] = tile->v0;
        }
    }

    // VBO uninitialized if 0
    if (chunk->tileVboID == 0) {
        glGenBuffers(1, &chunk->tileVboID); // Create an ID for the GPU buffer for the tiles (only done once)
    }

    if (chunk->lineVboID == 0) {
        glGenBuffers(1, &chunk->lineVboID); // Create an ID for the GPU buffer for the lines
    }

    glBindBuffer(GL_ARRAY_BUFFER, chunk->tileVboID); // Working with this specific buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(tileVboData), tileVboData, GL_STATIC_DRAW); // Copy the system memory buffer (tileVboData) into a GPU memory buffer
    glBindBuffer(GL_ARRAY_BUFFER,0); // Make sure to set vbo to 0 (nothing) to signal we're done working with this tileVboID

    glBindBuffer(GL_ARRAY_BUFFER, chunk->lineVboID); // Working with lineVboID
    glBufferData(GL_ARRAY_BUFFER, sizeof(lineVboData), lineVboData, GL_STATIC_DRAW); // Copy the lineVboData into a GPU buffer. GL_STATIC_DRAW because the lines will never change
    glBindBuffer(GL_ARRAY_BUFFER,0); // Make sure to set vbo to 0 (nothing) to signal we're done working with this lineVboID

    chunk->vboDirty = false;
}

void _world::drawWorld(float left, float right, float top, float bottom)
{
    // Since its an atlas we only need one text bind. Each tile takes a snippit of the atlas
    tileAtlas->bindTexture();

    glColor4f(1.0f,1.0f,1.0f,1.0f);

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
            glColor3f(1.0f,1.0f,1.0f); // Reset color to white for blank canvas

            glBindBuffer(GL_ARRAY_BUFFER, chunk->tileVboID);
            glVertexPointer(2, GL_FLOAT, 4 * sizeof(float), (void*)0);
            glTexCoordPointer(2, GL_FLOAT, 4 * sizeof(float), (void*)(2 * sizeof(float)));
            glDrawArrays(GL_QUADS, 0, 256 * 4);  // 256 tiles * 4 vertices

            if (displayChunkBorders) {
                glBindBuffer(GL_ARRAY_BUFFER, chunk->lineVboID);
                glVertexPointer(2, GL_FLOAT, 0, (void*)0); // We use 0 because its tightly packed data

                glDisable(GL_TEXTURE_2D); // Textures will disrupt the lines
                glColor3f(1.0f,0.0f,0.0f);
                glLineWidth(3.0f);
             
                glDrawArrays(GL_LINE_LOOP, 0, 4); // Draw a GL_LINE_LOOP of 4 points so 8 values total
             
                glEnable(GL_TEXTURE_2D);
            }
        }
    }

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
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

/* -- >> WORLD GENERATION << -- */

Vec2i _world::convertIndexToPos(int index, int width, int height) {
    int xPos = index % width;
    int yPos = index / height;
}

void _world::postProcessWorld() {
    Logger.LogInfo("Starting post processing of world");

    int worldWidth = (int)sqrt(numStartingChunks)*16;

    vector<uint8_t> world_noise_copy(world_noise);
    uniform_int_distribution<uint8_t> dist(0, 4); 

    for (int i = 0; i < world_noise.size(); i++) {
        if (!world_noise_copy[i]) {
            // Empty Tile applys random floor tile 
            world_noise[i] = dist(rng);
            continue;;
        }
        /*
        0 1 2
        3 4 5
        6 7 8

        Where 4 is ourselves
        */
        bool neighborTiles[9] = { false };
        
        for (int j = 0; j < 9; j++) {
            int xOffset = j % 3 - 1;    // Gets xOffset for tiles [-1,1]
            int yOffset = j / 3 - 1;    // Gets yOffset for tiles [-1,1] -- Applies worldWidth later
            int index = i + xOffset + yOffset * worldWidth; // Gets the given index to check
            if (index == i) continue;   // Skip checking ourselves
            if (index < 0 || index >= world_noise.size()) { // If index is out of bounds, treat as wall
                neighborTiles[j] = true;
                continue;
            }
            if (world_noise_copy[index]) {  // Check index, true means wall
                neighborTiles[j] = true;
            }
        }
        // Island Check //
        if (!neighborTiles[1] && !neighborTiles[3] && !neighborTiles[5] && !neighborTiles[7]) 
        {
            world_noise[i] = 14; // Island
            continue;
        }
        // Peninsula Checks //

        // Peninsula Right
        if (!neighborTiles[0] && !neighborTiles[1] && !neighborTiles[3] && !neighborTiles[6] && !neighborTiles[7]) 
        {
            world_noise[i] = 17; // Peninsula Right
            continue;
        }

        // Peninsula Left
        if (!neighborTiles[2] && !neighborTiles[1] && !neighborTiles[5] && !neighborTiles[7] && !neighborTiles[8]) 
        {
            world_noise[i] = 18; // Peninsula Left
            continue;
        }

        // Peninsula Bottom
        if (!neighborTiles[0] && !neighborTiles[1] && !neighborTiles[2] && !neighborTiles[3] && !neighborTiles[5]) 
        {
            world_noise[i] = 16; // Peninsula Bottom
            continue;
        }

        // Peninsula Top
        if (!neighborTiles[3] && !neighborTiles[5] && !neighborTiles[6] && !neighborTiles[7] && !neighborTiles[8]) 
        {
            world_noise[i] = 15; // Peninsula Top
            continue;
        }

        // Column Checks //

        // Column Up
        if (!neighborTiles[1] && !neighborTiles[7] && neighborTiles[3] && neighborTiles[5]) {
            world_noise[i] = 19; // Peninsula Up
            continue;

        }

        // Column Side
        if (neighborTiles[1] && neighborTiles[7] && !neighborTiles[3] && !neighborTiles[5]) {
            world_noise[i] = 20; // Peninsula Side
            continue;
        }

        // Wall Checks //

        // Wall Left
        if (!neighborTiles[3] && neighborTiles[1] && neighborTiles[7]) {
            world_noise[i] = 6; // Wall Left 
            continue;
        }

        // Wall Right
        if (!neighborTiles[5] && neighborTiles[1] && neighborTiles[7]) {
            world_noise[i] = 7; // Wall Right 
            continue;
        }

        // Wall Down
        if (!neighborTiles[1] && neighborTiles[3] && neighborTiles[5]) {
            world_noise[i] = 9; // Wall Down 
            continue;
        }

        // Wall Up
        if (!neighborTiles[7] && neighborTiles[3] && neighborTiles[5]) {
            world_noise[i] = 8; // Wall Up 
            continue;
        }

        // Wall Corners //

        // Wall Corner Bottom Left
        if (neighborTiles[5] && neighborTiles[7] && !neighborTiles[1] && !neighborTiles[3]) {
            world_noise[i] = 12; // Wall Corner Bottom Left 
            continue;
        }

        // Wall Corner Top Right
        if (neighborTiles[1] && neighborTiles[3] && !neighborTiles[5] && !neighborTiles[7]) {
            world_noise[i] = 11; // Wall Corner Top Right 
            continue;
        }

        // Wall Corner Bottom Right
        if (neighborTiles[3] && neighborTiles[7] && !neighborTiles[1] && !neighborTiles[5]) {
            world_noise[i] = 13; // Wall Corner Bottom Right 
            continue;
        }

        // Wall Corner Top Left
        if (neighborTiles[1] && neighborTiles[5] && !neighborTiles[3] && !neighborTiles[7]) {
            world_noise[i] = 10; // Wall Corner Top Left
            continue;
        }

        world_noise[i] = 5;
    }
    Logger.LogInfo("Finishing post processing of world");
}

/*
When tiles are made from noise its flat but since we load chunk by chunk we have to convert this flat array into 
a coordinate system for chunks
*/
void _world::finalizeWorld() {
    Logger.LogDebug("Mapping world noise into tiles");
    
    int worldWidth = (int)sqrt(numStartingChunks) * 16;
    int worldHeight = (int)sqrt(numStartingChunks) * 16;
    
    for (int i = 0; i < numStartingChunks; i++) {
        int new_chunkX = i % (int)sqrt(numStartingChunks) - floor(sqrt(numStartingChunks) / 2);
        int new_chunkY = i / (int)sqrt(numStartingChunks) - floor(sqrt(numStartingChunks) / 2);

        _chunk newChunk;
        newChunk.chunkX = new_chunkX;
        newChunk.chunkY = new_chunkY;

        // Calculate the starting position of this chunk in the world grid
        int chunkStartX = (new_chunkX + (int)floor(sqrt(numStartingChunks) / 2)) * 16;
        int chunkStartY = (new_chunkY + (int)floor(sqrt(numStartingChunks) / 2)) * 16;

        // Extract the 16x16 tile section for this chunk from world_noise
        for (int tileY = 0; tileY < 16; tileY++) {
            for (int tileX = 0; tileX < 16; tileX++) {
                // Calculate position in world grid
                int worldX = chunkStartX + tileX;
                int worldY = chunkStartY + tileY;
                
                // Convert to flat array index
                int world_noise_index = worldY * worldWidth + worldX;
                
                // Convert to chunk tile index (tileY * 16 + tileX gives position in chunk's 16x16 grid)
                int chunk_tile_index = tileY * 16 + tileX;
                
                newChunk.tileData[chunk_tile_index] = world_noise[world_noise_index];
            }
        }

        worldChunks.push_back(newChunk);

        loadedChunks[{new_chunkX, new_chunkY}] = true;
        chunkLookup[{new_chunkX, new_chunkY}] = &worldChunks.back();
    }
    Logger.LogDebug("World noise has been mapped to tiles and has been finalized!");
}

void _world::runWorldGeneration(int iterations) {
    Logger.LogInfo("Running world generation for parameters: ");
    Logger.LogInfo(" - Noise Density: " + to_string(noise_distribution*100.0f) + "%");
    Logger.LogInfo(" - Generation Iterations: " + to_string(iterations));

    uniform_real_distribution<float> dist(0.0f,1.0f);
    
    Logger.LogInfo("Establishing world noise for a ratio of " + to_string(noise_distribution));
    
      // World width/height in tiles
    int worldWidth = (int)sqrt(numStartingChunks)*16;
    int worldHeight = (int)sqrt(numStartingChunks)*16;

    for (int i = 0; i < world_noise.size(); i++) {
        world_noise[i] = (dist(rng) < noise_distribution);    // Randomly assigns 0 or 1 based on noise_distribution
    }

    Logger.LogInfo("Finished generating noise of " + to_string(world_noise.size()) + "tiles");

    Logger.LogInfo("Starting cellular automata algorithm for a world of Width: " + to_string(worldWidth) + "and Height: " + to_string(worldHeight) + " tiles");
    
    // Run cellular automata algorithm
    for (int iteration = 0; iteration < iterations; iteration++) {
        vector<uint8_t> world_noise_copy(world_noise);
        for (int i = 0; i < world_noise.size(); i++) {
            /*
            Each cell must check eight neighbors total (9 including itself). Each neighbor is checked to see if it is of type
            Wall = true or type Floor = false
            We do this by finding the index arround the element for efficiency
            */
            int num_neighbors = 0;
            for (int j = 0; j < 9; j++) {
                int xOffset = j % 3 - 1;    // Gets xOffset for tiles [-1,1]
                int yOffset = j / 3 - 1;    // Gets yOffset for tiles [-1,1] -- Applies worldWidth later
                int index = i + xOffset + yOffset * worldWidth; // Gets the given index to check
                if (index == i) continue;   // Skip checking ourselves
                if (index < 0 || index >= world_noise.size()) { // If index is out of bounds, treat as wall
                    num_neighbors++;
                    continue;
                }
                if (world_noise_copy[index]) {  // Check index, true means wall
                    num_neighbors++;
                }
            }
            
            // Moore Neighborhood //
            if (num_neighbors > 4) {
                world_noise[i] = true;
            } else {
                world_noise[i] = false;
            }
        }
        Logger.LogDebug(" -- Iteration: " + to_string(iteration) + " completed!");
    }

    Logger.LogDebug("World generation completed! Post processing now ...");
    postProcessWorld();
    Logger.LogDebug("Post processing completed! Finalizing world now ...");
    finalizeWorld();
}

/* -- >> DEBUGING << -- */

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