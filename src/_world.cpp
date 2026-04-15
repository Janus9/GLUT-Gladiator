#include <_world.h>

// -- CHUNK -- //

TileId _chunk::getTileIdAt(int index) const {
    return tileData[index];
}

_cell* _chunk::cellAt(int index) {
    if (index < 0 || index > 255) return nullptr;
    return &cellData[index];
}

bool _chunk::setTileIdAt(TileId id, int index) {
    if (index < 0 || index > 255) return false;
    
    tileData[index] = id;
    cellData[index].tileId = id;
    vboDirty = true;

    return true;
}

// -- WORLD -- //

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
    setTileInAtlas(0,0, world_tiles[TILE_NULL]);       // Undefined Tile
    
    setTileInAtlas(8,13, world_tiles[TILE_FLOOR_BLANK_1]);       // Blank Floor
    setTileInAtlas(9,12, world_tiles[TILE_FLOOR_CRACKED_1]);       // Slightly Cracked Floor
    setTileInAtlas(10,13, world_tiles[TILE_FLOOR_CRACKED_2]);      // Medium Cracked Floor
    setTileInAtlas(8,12, world_tiles[TILE_FLOOR_SQUARE]);       // Square outlined floor
    setTileInAtlas(9,13, world_tiles[TILE_FLOOR_BLANK_2]);       // Blank Floor 2

    world_tiles[TILE_NULL].hasCollision = false;
    world_tiles[TILE_FLOOR_BLANK_1].hasCollision = false;
    world_tiles[TILE_FLOOR_CRACKED_1].hasCollision = false;
    world_tiles[TILE_FLOOR_CRACKED_2].hasCollision = false;
    world_tiles[TILE_FLOOR_SQUARE].hasCollision = false;
    world_tiles[TILE_FLOOR_BLANK_2].hasCollision = false;

    world_tiles[TILE_NULL].name = "null";
    world_tiles[TILE_FLOOR_BLANK_1].name = "blank_floor";
    world_tiles[TILE_FLOOR_CRACKED_1].name = "slightly_cracked_floor";
    world_tiles[TILE_FLOOR_CRACKED_2].name = "medium_cracked_floor";
    world_tiles[TILE_FLOOR_SQUARE].name = "square_outlined_floor";
    world_tiles[TILE_FLOOR_BLANK_2].name = "blank_floor_2";

    // WALL //
    setTileInAtlas(22,16, world_tiles[TILE_WALL_CENTER]);        // Wall Center

    setTileInAtlas(21,16, world_tiles[TILE_WALL_LEFT]);        // Wall Left
    setTileInAtlas(23,16, world_tiles[TILE_WALL_RIGHT]);        // Wall Right
    setTileInAtlas(22,15, world_tiles[TILE_WALL_UP]);        // Wall Up
    setTileInAtlas(22,17, world_tiles[TILE_WALL_DOWN]);        // Wall Down

    setTileInAtlas(21,15, world_tiles[TILE_WALL_CORNER_TOPLEFT]);        // Wall Corner Top Left
    setTileInAtlas(23,15, world_tiles[TILE_WALL_CORNER_TOPRIGHT]);        // Wall Corner Top Right
    setTileInAtlas(21,17, world_tiles[TILE_WALL_CORNER_BOTTOMLEFT]);        // Wall Corner Bottom Left
    setTileInAtlas(23,17, world_tiles[TILE_WALL_CORNER_BOTTOMRIGHT]);        // Wall Corner Bottom Right

    setTileInAtlas(25,16, world_tiles[TILE_WALL_ISLAND]);        // Wall Island

    setTileInAtlas(25,15, world_tiles[TILE_WALL_PENINSULA_TOP]);        // Wall Peninsula Top
    setTileInAtlas(25,17, world_tiles[TILE_WALL_PENINSULA_DOWN]);        // Wall Peninsula Down
    setTileInAtlas(24,16, world_tiles[TILE_WALL_PENINSULA_LEFT]);        // Wall Peninsula Left
    setTileInAtlas(26,16, world_tiles[TILE_WALL_PENINSULA_RIGHT]);        // Wall Peninsula Right

    setTileInAtlas(23,18, world_tiles[TILE_WALL_COLUMN_UP]);        // Wall Column Up
    setTileInAtlas(22,18, world_tiles[TILE_WALL_COLUMN_SIDE]);        // Wall Column Side

    world_tiles[TILE_WALL_CENTER].name = "wall_center";

    world_tiles[TILE_WALL_LEFT].name = "wall_left";
    world_tiles[TILE_WALL_RIGHT].name = "wall_right";
    world_tiles[TILE_WALL_UP].name = "wall_up";
    world_tiles[TILE_WALL_DOWN].name = "wall_down";

    world_tiles[TILE_WALL_CORNER_TOPLEFT].name = "wall_corner_top_left";
    world_tiles[TILE_WALL_CORNER_TOPRIGHT].name = "wall_corner_top_right";
    world_tiles[TILE_WALL_CORNER_BOTTOMLEFT].name = "wall_corner_bottom_left";
    world_tiles[TILE_WALL_CORNER_BOTTOMRIGHT].name = "wall_corner_bottom_right";

    world_tiles[TILE_WALL_ISLAND].name = "wall_island";

    world_tiles[TILE_WALL_PENINSULA_TOP].name = "wall_peninsula_top";
    world_tiles[TILE_WALL_PENINSULA_DOWN].name = "wall_peninsula_down";
    world_tiles[TILE_WALL_PENINSULA_LEFT].name = "wall_peninsula_left";
    world_tiles[TILE_WALL_PENINSULA_RIGHT].name = "wall_peninsula_right";

    world_tiles[TILE_WALL_COLUMN_UP].name = "wall_column_up";
    world_tiles[TILE_WALL_COLUMN_SIDE].name = "wall_column_side";
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
    
    // Vbo for line outline data
    vector<float> tileOutlineVboData;

    // For each tile of the chunk //
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            int tileIndex = y * 16 + x;

            TileId tileId = chunk->getTileIdAt(tileIndex);
            _cell* cell = chunk->cellAt(tileIndex);
            const _tile* tile = &world_tiles[tileId];
            
            cell->tileId = tileId;
            cell->index = tileIndex; // Match every draw cycle
            cell->parentChunk = chunk;

            float worldX = (chunk->chunkX * 16 + x) * TILE_W;
            float worldY = (chunk->chunkY * 16 + y) * TILE_H;

            cell->pos = {worldX + TILE_W/2.0f, worldY + TILE_H/2.0f};

            // Outline VBO Setup //

            // Only outlined tiles have vbo data pushed
            if (cell->outlined) {
                // Bottom-Left -> Bottom-Right //
                // bottom-left
                tileOutlineVboData.push_back(worldX);
                tileOutlineVboData.push_back(worldY);
                // bottom-right
                tileOutlineVboData.push_back(worldX + TILE_W);
                tileOutlineVboData.push_back(worldY);
    
                // Bottom-Right -> Top-Right //
                // bottom-right
                tileOutlineVboData.push_back(worldX + TILE_W);
                tileOutlineVboData.push_back(worldY);
                // top-right
                tileOutlineVboData.push_back(worldX + TILE_W);
                tileOutlineVboData.push_back(worldY + TILE_H);
    
                // Top-Right -> Top-Left //
                // top-right
                tileOutlineVboData.push_back(worldX + TILE_W);
                tileOutlineVboData.push_back(worldY + TILE_H);
                // top-left
                tileOutlineVboData.push_back(worldX);
                tileOutlineVboData.push_back(worldY + TILE_H);

                // Top-Left - Bottom-Left //
                // top-left
                tileOutlineVboData.push_back(worldX);
                tileOutlineVboData.push_back(worldY + TILE_H);
                // bottom-left
                tileOutlineVboData.push_back(worldX);
                tileOutlineVboData.push_back(worldY);
            }

            // Tile VBO Setup //
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

    // VBO uninitialized if 0 -- creates an ID for the GPU buffer (only done once)
    if (chunk->tileVboID == 0) {
        glGenBuffers(1, &chunk->tileVboID);
    }

    if (chunk->chunkLineVboID == 0) {
        glGenBuffers(1, &chunk->chunkLineVboID); 
    }

    if (chunk->tileLineVboID == 0) {
        glGenBuffers(1, &chunk->tileLineVboID); 
    }

    // Tile Data VBO //
    glBindBuffer(GL_ARRAY_BUFFER, chunk->tileVboID); // Working with this specific buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(tileVboData), tileVboData, GL_STATIC_DRAW); // Copy the system memory buffer (tileVboData) into a GPU memory buffer
    glBindBuffer(GL_ARRAY_BUFFER, 0); // Make sure to set vbo to 0 (nothing) to signal we're done working with this tileVboID

    // Chunk Line Data VBO //
    glBindBuffer(GL_ARRAY_BUFFER, chunk->chunkLineVboID); // Working with chunkLineVboID
    glBufferData(GL_ARRAY_BUFFER, sizeof(lineVboData), lineVboData, GL_STATIC_DRAW); // Copy the lineVboData into a GPU buffer. GL_STATIC_DRAW because the lines will never change
    glBindBuffer(GL_ARRAY_BUFFER, 0); // Make sure to set vbo to 0 (nothing) to signal we're done working with this chunkLineVboID

    // Tile Line Data VBO //
    glBindBuffer(GL_ARRAY_BUFFER, chunk->tileLineVboID);
    glBufferData(GL_ARRAY_BUFFER, tileOutlineVboData.size() * sizeof(float), tileOutlineVboData.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    chunk->tileLineVboSize = tileOutlineVboData.size();

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
            _chunk* chunk = getChunkAt(Vec2i(chunkX, chunkY));
            if (chunk == nullptr) { continue; }

            // If chunk is dirty (so tiles changed) call a rebuild
            if (chunk->vboDirty) {
                buildChunkVBO(chunk);
            }

            // Draw Tile VBO //
            glColor3f(1.0f,1.0f,1.0f); // Reset color to white for blank canvas

            glBindBuffer(GL_ARRAY_BUFFER, chunk->tileVboID);
            glVertexPointer(2, GL_FLOAT, 4 * sizeof(float), (void*)0);
            glTexCoordPointer(2, GL_FLOAT, 4 * sizeof(float), (void*)(2 * sizeof(float)));
            glDrawArrays(GL_QUADS, 0, 256 * 4);  // 256 tiles * 4 vertices

            // Draw Tile Line VBO //
            glBindBuffer(GL_ARRAY_BUFFER, chunk->tileLineVboID);
            glVertexPointer(2, GL_FLOAT, 0, (void*)0); // We use 0 because its tightly packed data

            glDisable(GL_TEXTURE_2D); // Textures will disrupt the lines
            glColor3f(0.0f,0.0f,1.0f);
            glLineWidth(2.0f);
            
            glDrawArrays(GL_LINES, 0, chunk->tileLineVboSize/2); // Two floats per vertex so divide by 2
            
            glEnable(GL_TEXTURE_2D);

            // Draw Chunk Line VBO //
            if (DEBUG_displayChunkBorders) {
                glBindBuffer(GL_ARRAY_BUFFER, chunk->chunkLineVboID);
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
        8 7 6
        5 4 3
        2 1 0

        0 1 2
        3 4 5
        6 7 8

        Where 4 is ourselves
        */
        bool neighborTiles[9] = { false };
        
        for (int j = 0; j < 9; j++) {
            int xOffset = j % 3 - 1;    // Gets xOffset for tiles [-1,1]
            int yOffset = (8-j) / 3 - 1;    // Gets yOffset for tiles [-1,1] -- Applies worldWidth later
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
        world_noise[i] = determineTileType(neighborTiles);
    }
    Logger.LogInfo("Finishing post processing of world");
}

TileId _world::determineTileType(const bool neighborTiles[9]) const {
    // Island Check //
    if (!neighborTiles[1] && !neighborTiles[3] && !neighborTiles[5] && !neighborTiles[7]) 
    {
        return TILE_WALL_ISLAND; // Island
    }
    // Peninsula Checks //

    // Peninsula Left
    if (!neighborTiles[1] && !neighborTiles[3] && !neighborTiles[7] && neighborTiles[5]) 
    {
        return TILE_WALL_PENINSULA_LEFT; // Peninsula Left
    }

    // Peninsula Right
    if (!neighborTiles[1] && !neighborTiles[5] && !neighborTiles[7] && neighborTiles[3]) 
    {
        return TILE_WALL_PENINSULA_RIGHT; // Peninsula Right
    }

    // Peninsula Bottom
    if (neighborTiles[1] && !neighborTiles[3] && !neighborTiles[5] && !neighborTiles[7]) 
    {
        return TILE_WALL_PENINSULA_DOWN; // Peninsula Bottom
    }

    // Peninsula Top
    if (!neighborTiles[3] && !neighborTiles[5] && neighborTiles[7] && !neighborTiles[1]) 
    {
        return TILE_WALL_PENINSULA_TOP; // Peninsula Top
    }

    // Column Checks //

    // Column Up
    if (neighborTiles[1] && neighborTiles[7] && !neighborTiles[3] && !neighborTiles[5]) {
        return TILE_WALL_COLUMN_UP; // Column Up

    }

    // Column Side
    if (!neighborTiles[1] && !neighborTiles[7] && neighborTiles[3] && neighborTiles[5]) {
        return TILE_WALL_COLUMN_SIDE; // Column Side
    }

    // Wall Checks //

    // Wall Left
    if (!neighborTiles[3] && neighborTiles[1] && neighborTiles[7] && neighborTiles[5]) {
        return TILE_WALL_LEFT; // Wall Left 
    }

    // Wall Right
    if (!neighborTiles[5] && neighborTiles[1] && neighborTiles[7] && neighborTiles[3]) {
        return TILE_WALL_RIGHT; // Wall Right 
    }

    // Wall Down
    if (neighborTiles[1] && neighborTiles[3] && neighborTiles[5] && !neighborTiles[7]) {
        return TILE_WALL_DOWN; // Wall Down 
    }

    // Wall Up
    if (!neighborTiles[1] && neighborTiles[3] && neighborTiles[5] && neighborTiles[7]) {
        return TILE_WALL_UP; // Wall Up 
    }

    // Wall Corners //

    // Wall Corner Bottom Left
    if (!neighborTiles[3] && !neighborTiles[7] && neighborTiles[1] && neighborTiles[5]) {
        return TILE_WALL_CORNER_BOTTOMLEFT; // Wall Corner Bottom Left 
    }

    // Wall Corner Top Left
    if (!neighborTiles[1] && !neighborTiles[3] && neighborTiles[5] && neighborTiles[7]) {
        return TILE_WALL_CORNER_TOPLEFT; // Wall Corner Top Left 
    }

    // Wall Corner Bottom Right
    if (!neighborTiles[5] && !neighborTiles[7] && neighborTiles[1] && neighborTiles[3]) {
        return TILE_WALL_CORNER_BOTTOMRIGHT; // Wall Corner Bottom Right 
    }

    // Wall Corner Top Right
    if (!neighborTiles[1] && !neighborTiles[5] && neighborTiles[3] && neighborTiles[7]) {
        return TILE_WALL_CORNER_TOPRIGHT; // Wall Corner Top Right
    }

    return TILE_WALL_CENTER;
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
                
                TileId newId = static_cast<TileId>(world_noise[world_noise_index]);

                newChunk.setTileIdAt(newId,chunk_tile_index);
            }
        }

        worldChunks.push_back(newChunk);

        loadedChunks[{new_chunkX, new_chunkY}] = true;
        chunkLookup[{new_chunkX, new_chunkY}] = &worldChunks.back();
    }
    Logger.LogDebug("World noise has been mapped to tiles and has been finalized!");
}

Vec2i _world::worldToChunkPos(const Vec2f &pos) const {
    // Get chunk position (coordinates)
    Vec2i chunkPos;
    chunkPos.x = (int)floor(pos.x / (16 * TILE_W));
    chunkPos.y = (int)floor(pos.y / (16 * TILE_H));
    return chunkPos;
}

_chunk* _world::getChunkAt(const Vec2i &chunkPos) const {
    auto it = chunkLookup.find({chunkPos.x,chunkPos.y});
    if (it != chunkLookup.end()) {
        return it->second;
    }
    return nullptr;
}

_chunk* _world::getChunkAtWorld(const Vec2f &pos) const {
    // Just a wrapper of the two functions
    return getChunkAt(worldToChunkPos(pos));
}


const _tile* _world::getTileFromChunkIndex(const _chunk* chunk, const int index) const {
    if (index < 0 || index > 255) return nullptr;
    return &world_tiles[chunk->getTileIdAt(index)];
}

// https://www.desmos.com/calculator/x5cyeg8q8s
const _tile* _world::getTileAtWorld(const Vec2f &pos) const {
    // Convert floats to ints by truncation
    int posX = pos.x;
    int posY = pos.y;

    // The adjust pos is 0-255 for x/y (pos within the chunk). This works with negatives as well.
    Vec2i adjustedPos(modFloor(posX,256),modFloor(posY,256));
    
    // Get chunk present at position
    const _chunk* chunk = getChunkAtWorld(pos);
    
    // Get an index in the flat array for the tile
    uint8_t tileIndex = (int)floor(adjustedPos.y/16)*16 + (int)floor(adjustedPos.x/16);

    // Map id -> tile and return it
    return &world_tiles[chunk->getTileIdAt(tileIndex)];
}

_cell* _world::getCellAtWorld(const Vec2f &pos) const {
     // Convert floats to ints by truncation
    int posX = pos.x;
    int posY = pos.y;

    // The adjust pos is 0-255 for x/y (pos within the chunk). This works with negatives as well.
    Vec2i adjustedPos(modFloor(posX,256),modFloor(posY,256));
    
    // Get chunk present at position
    _chunk* chunk = getChunkAtWorld(pos);
    
    // Get an index in the flat array for the tile
    uint8_t tileIndex = (int)floor(adjustedPos.y/16)*16 + (int)floor(adjustedPos.x/16);

    // Get the id stored in the chunk
    return chunk->cellAt(tileIndex);
}

bool _world::setTileAtChunk(_cell* cell, TileId id) {
    if (!cell || cell->parentChunk == nullptr) return false;
    
    bool success = cell->parentChunk->setTileIdAt(id, cell->index);
    if (success) {
        _cell* neighborCells[9];
        mapCellNeighbors(cell,neighborCells); // Gets a 3x3 map of cell pointers around the center

        // For each cell, rerun the post-processing tile type (requires checking all 8 around it)
        for (int i = 0; i < 9; i++) {
            if (i == 4 || !isTileWall(neighborCells[i]->tileId)) continue; // Skip center cell and floors
            _cell* localCell = neighborCells[i];
            // We need all 9 cells around each cell we check
            _cell* localNeighborCells[9];
            mapCellNeighbors(localCell,localNeighborCells);

            // Map the cells to booleans for tiles
            bool neighborTiles[9];
            for (int j = 0; j < 9; j++) {
                if (neighborCells[j]) {
                    neighborTiles[j] = isTileWall(localNeighborCells[j]->tileId);
                } else {
                    // nullptr (out of bounds treat as wall)
                    neighborTiles[j] = true;
                }
            }
            TileId localTileId = determineTileType(neighborTiles);
            localCell->parentChunk->setTileIdAt(localTileId, localCell->index);
        }
        
        cell->parentChunk->vboDirty = true;  // Mark chunk for rebuild
    }
    return success;
}

bool _world::setCellOutined(_cell* cell, bool state) {
    if (!cell || !cell->parentChunk) return false;
    cell->outlined = state;
    cell->parentChunk->vboDirty = true;
}   

bool _world::isTileWall(TileId tileId) const {
    return (tileId >= TILE_WALL_CENTER && tileId <= TILE_WALL_COLUMN_SIDE);
}

bool _world::isCellWall(const _cell* cell) const {
    if (!cell) return false;
    return (cell->tileId >= TILE_WALL_CENTER && cell->tileId <= TILE_WALL_COLUMN_SIDE);
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

void _world::mapCellNeighbors(_cell* cell, _cell* outNeighbors[9]) {
    if (!cell) return;
    
    outNeighbors[0] = getCellAtWorld({cell->pos.x - TILE_W, cell->pos.y + TILE_H}); // top-left
    outNeighbors[1] = getCellAtWorld({cell->pos.x,          cell->pos.y + TILE_H}); // top
    outNeighbors[2] = getCellAtWorld({cell->pos.x + TILE_W, cell->pos.y + TILE_H}); // top-right
    
    outNeighbors[3] = getCellAtWorld({cell->pos.x - TILE_W, cell->pos.y});          // left
    outNeighbors[4] = cell;                                                           // center
    outNeighbors[5] = getCellAtWorld({cell->pos.x + TILE_W, cell->pos.y});          // right

    outNeighbors[6] = getCellAtWorld({cell->pos.x - TILE_W, cell->pos.y - TILE_H}); // bottom-left
    outNeighbors[7] = getCellAtWorld({cell->pos.x,          cell->pos.y - TILE_H}); // bottom
    outNeighbors[8] = getCellAtWorld({cell->pos.x + TILE_W, cell->pos.y - TILE_H}); // bottom-right

    cout << "Neighbors: \n" 
     << outNeighbors[0]->tileId << ", " << outNeighbors[1]->tileId << ", " << outNeighbors[2]->tileId << "\n"
     << outNeighbors[3]->tileId << ", " << outNeighbors[4]->tileId << ", " << outNeighbors[5]->tileId << "\n"
     << outNeighbors[6]->tileId << ", " << outNeighbors[7]->tileId << ", " << outNeighbors[8]->tileId << "\n";
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