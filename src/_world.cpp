#include <_world.h>

// -- CELL -- // 

_cell::_cell() {
    for (int i = 0; i < NUM_LAYERS; i++) {
        tileIDs[i] = TILE_NULL;
    }
}

_cell::~_cell() {

}

bool _cell::setOutline(bool state) {
    if (!this || !parentChunk) return false;
        outlined = state;
        parentChunk->setChunkDirty();
    return true;
}   

bool _cell::isOutlined() const {
    return outlined;
}

bool _cell::impluseHealth(float amount) {
    health += amount;
    if (health <= 0) {
        health = 0; // Bound health to 0
        return true;
    }
    return false;
}

void _cell::setHealth(float amount) {
    if (amount < 0) amount = 0;
    health = amount;
}

float _cell::getHealth() const {
    return health;
}

bool _cell::isAlive() const {
    return health > 0.0f;
}

// -- CHUNK -- //

// Static //

int _chunk::nextIndex = 0;

// Public //

_chunk::_chunk() {
    vboIndex = nextIndex;
    nextIndex++;
}

_chunk::~_chunk() {
}

TileId _chunk::getTileIdAt(int index) const {
    if (!this) return TILE_NULL;
    if (index < 0 || index > 255) return TILE_NULL;
    return cellData[index].tileIDs[LAYER_PRIMARY];       // Change later only returns "primary" tile
}

_cell* _chunk::cellAt(int index) {
    if (index < 0 || index > 255) return nullptr;
    return &cellData[index];
}

bool _chunk::setTileIdAt(TileId id, int index) {
    if (!this) return false;
    if (index < 0 || index > 255) return false;
    
    cellData[index].tileIDs[LAYER_PRIMARY] = id;         // Change later only sets "primary" tile
    vboDirty = true;

    return true;
}

const _cell* _chunk::getAllCells() const {
    return cellData;
}

void _chunk::setAllCells(const _cell* cells) {
    if (!cells) return;
    memcpy(cellData,cells,256 * sizeof(_cell));
    vboDirty = true;
}

chunk_serial_data _chunk::serializeChunk() const {
    chunk_serial_data chunk_data;
    chunk_data.chunkX = chunkX;
    chunk_data.chunkY = chunkY;
    for (int i = 0; i < 256; i++) {
        const _cell* cell = &cellData[i];
        memcpy(chunk_data.cell_data[i].tileIDs, cell->tileIDs, NUM_LAYERS * sizeof(TileId)); 
        chunk_data.cell_data[i].outlined = static_cast<uint8_t>(cell->isOutlined());
        chunk_data.cell_data[i].padding1 = 0;   // Padding, does nothing.
        chunk_data.cell_data[i].padding2 = 0;   // Padding, does nothing.
        chunk_data.cell_data[i].health = cell->getHealth();
    }
    return chunk_data;
}

void _chunk::loadSerializedChunk(const chunk_serial_data &chunk_data) {
    chunkX = chunk_data.chunkX;
    chunkY = chunk_data.chunkY;
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            const int tileIndex = y * 16 + x;

            const float halfWidth = TILE_W * 0.5f;
            const float halfHeight = TILE_H * 0.5f;
        
            const float worldXCenter = (chunkX * 16 + x) * TILE_W + halfWidth;
            const float worldYCenter = (chunkY * 16 + y) * TILE_H + halfHeight;

            _cell& chunkCell = cellData[tileIndex];

            // Serialized Data //
            chunkCell.setHealth(chunk_data.cell_data[tileIndex].health);
            chunkCell.setOutline(static_cast<bool>(chunk_data.cell_data[tileIndex].outlined));
            chunkCell.parentChunk = this;
            memcpy(chunkCell.tileIDs, chunk_data.cell_data[tileIndex].tileIDs, NUM_LAYERS * sizeof(TileId));
            chunkCell.index = tileIndex;
            chunkCell.pos = {worldXCenter, worldYCenter};

            // Non Serialized Data //
            // Handled by VBO setup -- should be changed later
            vboDirty = true;
        }
    }
}

int _chunk::getVboIndex() const {
    return vboIndex;
}

bool _chunk::isChunkDirty() const {
    return vboDirty;
}

void _chunk::setChunkDirty() {
    vboDirty = true;
}

void _chunk::setChunkClean() {
    vboDirty = false;
}

// -- WORLD -- //

// STATIC MEMBERS //

glm::mat4 _world::viewProjectionMatrix;
Vec2f _world::cameraPosition = {0.0f, 0.0f};

// -- PUBLIC -- //

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

    Logger.LogInfo("Tiles to Draw: " + to_string(tilesToDraw),LOG_CONSOLE);
    
    Logger.LogInfo("------------------------", LOG_CONSOLE);
}

void _world::setViewProjectionMatrix(const glm::mat4 &_viewProjectionMatrix) {
    viewProjectionMatrix = _viewProjectionMatrix;
}

void _world::setCameraPosition(const Vec2f &_cameraPosition) {
    cameraPosition = _cameraPosition;
}

_world::_world()
{
    //ctor
}

_world::~_world()
{
    if (vboID != 0) {
        glDeleteBuffers(1,&vboID); // tell the GPU to delete the vertex buffer
        vboID = 0;
    }
    if (eboID != 0) {
        glDeleteBuffers(1,&eboID); // tell the GPU to delete the index buffer
        eboID = 0;
    }
    if (vaoID != 0) {
        glDeleteVertexArrays(1,&vaoID); // tell the GPU to delete the array buffer
        vaoID = 0;
    }

    //dtor
    delete tileAtlas;
    tileAtlas = nullptr;

    delete initBenchmark;
    initBenchmark = nullptr;

    delete cellParticles;
    cellParticles = nullptr;

    chunkLookup.clear();
    loadedChunks.clear();

    worldChunks.clear();
}

void _world::initWorld(bool loadWorld, const world_config &_configuration, _lightManager* lightManager)
{
    sceneLightManager = lightManager;

    configuration = _configuration;

    if (worldInitialized) {
        cout << "WARNING: World has already been initialized, skipping\n";
        return;
    }

    initBenchmark->startBenchmark();

    // Chunk width/height * 16 tiles wide * 16 world units per tile / 2 
    worldBounds = sqrt(configuration.num_chunks) * 16.0f * 16.0f * 0.5f;

    // Logger.LogInfo("Initializing world for seed " + to_string(seed), LOG_BOTH);
    Logger.LogInfo("World has " + to_string(configuration.num_chunks) + " starting chunks.", LOG_BOTH);

    tileAtlas->loadTexture("images/set_1.png"); // Load the tile atlas texture
    // Reserve allocates memory but does not instantiate it -- resize allocates AND instantiates it (dont want that)

    // -- SHADER SETUP -- //
    shader.initShader("shaders/world/vertex.vs","shaders/world/fragment.fs");
    uint32_t program = shader.getProgram();

    sceneLightManager->addProgram(program);

    // Uniforms
    u_viewProjectionMatrix = glGetUniformLocation(program,"u_viewProjectionMatrix");
    u_texture = glGetUniformLocation(program,"u_texture");
    u_cameraPos = glGetUniformLocation(program,"u_cameraPos");
    u_time = glGetUniformLocation(program,"u_time");

    initTiles(); // Setup tiles
    
    const double sqrtNumChunks = sqrt(configuration.num_chunks);
    // This checks if a decimal (like 1.3) is equal to its floor (1.0) which indicates the sqrt wasn't perfect
    if (sqrtNumChunks != floor(sqrtNumChunks)) {
        Logger.LogWarning("numStartingChunks is not a perfect square. This may lead to an uneven distribution of chunks around the center.", LOG_BOTH);
    }

    // Only run generation when we dont load the world  
    if (!loadWorld) {
        runWorldGeneration(); 
    }

    // PARTICLE EFFECTS //

    cellParticles->initParticleManager("images/particle.png",1, sceneLightManager, 10000); // Particles for cell usage
    // wall_break_effect
    wall_break_effect.amount = 100;

    wall_break_effect.minVelX = -3.0f;
    wall_break_effect.maxVelX = 3.0f;
    wall_break_effect.minVelY = 5.0f;
    wall_break_effect.maxVelY = 20.0f;

    wall_break_effect.minRadius = 1.0f;
    wall_break_effect.maxRadius = 3.0f;

    wall_break_effect.minLifeTime = 0.6f;
    wall_break_effect.maxLifeTime = 1.3f;

    wall_break_effect.minSpawnOffsetX = -8.0f;
    wall_break_effect.maxSpawnOffsetX = 8.0f;
    wall_break_effect.minSpawnOffsetY = -8.0f;
    wall_break_effect.maxSpawnOffsetY = 8.0f;

    // _wall_damage_effect
    wall_damage_effect.amount = 10;

    wall_damage_effect.minVelX = -3.0f;
    wall_damage_effect.maxVelX = 3.0f;
    wall_damage_effect.minVelY = 5.0f;
    wall_damage_effect.maxVelY = 15.0f;

    wall_damage_effect.minRadius = 1.0f;
    wall_damage_effect.maxRadius = 3.0f;

    wall_damage_effect.minLifeTime = 0.4f;
    wall_damage_effect.maxLifeTime = 1.1f;

    wall_damage_effect.minSpawnOffsetX = -4.0f;
    wall_damage_effect.maxSpawnOffsetX = 4.0f;
    wall_damage_effect.minSpawnOffsetY = -4.0f;
    wall_damage_effect.maxSpawnOffsetY = 4.0f;

    // -- BUFFER SETUP -- //
    glGenBuffers(1, &vboID); 
    glGenBuffers(1, &eboID); 
    glGenVertexArrays(1, &vaoID);

    // VBO //
    // Number of total chunks * number of tiles per chunk * number of tile layers * 4 vertices per tile * 7 floats per vertex * bytes per float 
    const int maxSizeBytes = configuration.num_chunks * NUM_TILES_CHUNK * NUM_LAYERS * 4 * 7 * sizeof(float);

    glBindBuffer(GL_ARRAY_BUFFER, vboID);
    glBufferData(GL_ARRAY_BUFFER,maxSizeBytes,nullptr,GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    GLenum errVbo = glGetError();
    if (errVbo != GL_NO_ERROR) {
        cout << "ERROR: OpenGL error on world VBO: " << errVbo << "\n";
    }

    // EBO //
    // (Number of total chunks) * (256 tiles per chunk) * (number of layers) * (6 indicies per tile)
    vector<uint32_t> eboData(configuration.num_chunks * NUM_TILES_CHUNK * NUM_LAYERS * 6);
    int vertexOffset = 0;
    int eIndex = 0;
    for (int i = 0; i < configuration.num_chunks * NUM_TILES_CHUNK * NUM_LAYERS; i++) {
        // Ebo (Two Triangles) //
        // Triangle 1
        eboData[eIndex++] = vertexOffset + 0; // BL   
        eboData[eIndex++] = vertexOffset + 1; // BR
        eboData[eIndex++] = vertexOffset + 2; // TR
        // Triangle 2
        eboData[eIndex++] = vertexOffset + 0; // BL
        eboData[eIndex++] = vertexOffset + 2; // TR
        eboData[eIndex++] = vertexOffset + 3; // TL

        vertexOffset += 4;
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, eboData.size() * sizeof(uint32_t), eboData.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
    
    GLenum errEbo = glGetError();
    if (errEbo != GL_NO_ERROR) {
        cout << "ERROR: OpenGL error on world EBO: " << errEbo << "\n";
    }

    // VAO //
    glBindVertexArray(vaoID);
    
    glBindBuffer(GL_ARRAY_BUFFER,vboID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,eboID);
    
    const GLsizei stride = 7 * sizeof(float);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,stride,(void*)(0 * sizeof(float)));     // Size (vec2)
    
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,stride,(void*)(2 * sizeof(float)));     // Texture Position (vec2)
    
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,stride,(void*)(4 * sizeof(float)));     // Position (vec2)
    
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3,1,GL_FLOAT,GL_FALSE,stride,(void*)(6 * sizeof(float)));     // Outlined (int)

    glBindVertexArray(0);

    // BENCHMARK //

    initBenchmark->clickBenchmark();
    double time = initBenchmark->getAverageResult();

    worldInitialized = true;
    Logger.LogInfo("World initialization for " + to_string(worldChunks.size()) + "chunks took " + to_string(time) + "ms");
}

void _world::initTiles() {
    // NULL //
    setTileInAtlas(0,0, world_tiles[TILE_NULL]);       // Undefined Tile
    world_tiles[TILE_NULL].hasCollision = false;
    world_tiles[TILE_NULL].name = "null";

    // Boss Floor //
    setTileInAtlas(8,13, world_tiles[TILE_FLOOR_BOSS_BLANK_1]);      
    world_tiles[TILE_FLOOR_BOSS_BLANK_1].hasCollision = false;
    world_tiles[TILE_FLOOR_BOSS_BLANK_1].name = "blank_floor_boss";

    setTileInAtlas(9,12, world_tiles[TILE_FLOOR_BOSS_CRACKED_1]);       
    world_tiles[TILE_FLOOR_BOSS_CRACKED_1].hasCollision = false;
    world_tiles[TILE_FLOOR_BOSS_CRACKED_1].name = "slightly_cracked_floor_boss";

    setTileInAtlas(10,13, world_tiles[TILE_FLOOR_BOSS_CRACKED_2]);      
    world_tiles[TILE_FLOOR_BOSS_CRACKED_2].hasCollision = false;
    world_tiles[TILE_FLOOR_BOSS_CRACKED_2].name = "medium_cracked_floor_boss";

    setTileInAtlas(8,12, world_tiles[TILE_FLOOR_BOSS_SQUARE_1]);       
    world_tiles[TILE_FLOOR_BOSS_SQUARE_1].hasCollision = false;
    world_tiles[TILE_FLOOR_BOSS_SQUARE_1].name = "square_outlined_floor_1_boss";

    setTileInAtlas(10,12, world_tiles[TILE_FLOOR_BOSS_SQUARE_2]);       
    world_tiles[TILE_FLOOR_BOSS_SQUARE_2].hasCollision = false;
    world_tiles[TILE_FLOOR_BOSS_SQUARE_2].name = "square_outlined_floor_2_boss";

    setTileInAtlas(9,13, world_tiles[TILE_FLOOR_BOSS_BLANK_2]);       
    world_tiles[TILE_FLOOR_BOSS_BLANK_2].hasCollision = false;
    world_tiles[TILE_FLOOR_BOSS_BLANK_2].name = "blank_floor_2_boss";
    
    // Outer Floor //
    setTileInAtlas(22,11, world_tiles[TILE_FLOOR_OUTER_BLANK_1]);      
    world_tiles[TILE_FLOOR_OUTER_BLANK_1].hasCollision = false;
    world_tiles[TILE_FLOOR_OUTER_BLANK_1].name = "blank_floor";

    setTileInAtlas(23,10, world_tiles[TILE_FLOOR_OUTER_CRACKED_1]);       
    world_tiles[TILE_FLOOR_OUTER_CRACKED_1].hasCollision = false;
    world_tiles[TILE_FLOOR_OUTER_CRACKED_1].name = "slightly_cracked_floor";

    setTileInAtlas(24,11, world_tiles[TILE_FLOOR_OUTER_CRACKED_2]);      
    world_tiles[TILE_FLOOR_OUTER_CRACKED_2].hasCollision = false;
    world_tiles[TILE_FLOOR_OUTER_CRACKED_2].name = "medium_cracked_floor";

    setTileInAtlas(22,10, world_tiles[TILE_FLOOR_OUTER_SQUARE_1]);       
    world_tiles[TILE_FLOOR_OUTER_SQUARE_1].hasCollision = false;
    world_tiles[TILE_FLOOR_OUTER_SQUARE_1].name = "square_outlined_floor_1";

    setTileInAtlas(24,10, world_tiles[TILE_FLOOR_OUTER_SQUARE_2]);       
    world_tiles[TILE_FLOOR_OUTER_SQUARE_2].hasCollision = false;
    world_tiles[TILE_FLOOR_OUTER_SQUARE_2].name = "square_outlined_floor_2";

    setTileInAtlas(23,11, world_tiles[TILE_FLOOR_OUTER_BLANK_2]);       
    world_tiles[TILE_FLOOR_OUTER_BLANK_2].hasCollision = false;
    world_tiles[TILE_FLOOR_OUTER_BLANK_2].name = "blank_floor_2";

    // Middle Floor //
    setTileInAtlas(22,13, world_tiles[TILE_FLOOR_OUTER_DEFAULT_1]);       
    world_tiles[TILE_FLOOR_OUTER_DEFAULT_1].hasCollision = false;
    world_tiles[TILE_FLOOR_OUTER_DEFAULT_1].name = "default_floor_middle_1";
    
    setTileInAtlas(23,13, world_tiles[TILE_FLOOR_OUTER_DEFAULT_2]);       
    world_tiles[TILE_FLOOR_OUTER_DEFAULT_2].hasCollision = false;
    world_tiles[TILE_FLOOR_OUTER_DEFAULT_2].name = "default_floor_middle_2";

    // Inner Floor //
    setTileInAtlas(25,13, world_tiles[TILE_FLOOR_INNER_DEFAULT_1]);       
    world_tiles[TILE_FLOOR_INNER_DEFAULT_1].hasCollision = false;
    world_tiles[TILE_FLOOR_INNER_DEFAULT_1].name = "default_floor_inner_1";
    
    setTileInAtlas(26,13, world_tiles[TILE_FLOOR_INNER_DEFAULT_2]);       
    world_tiles[TILE_FLOOR_INNER_DEFAULT_2].hasCollision = false;
    world_tiles[TILE_FLOOR_INNER_DEFAULT_2].name = "default_floor_inner_2";

    // Floor Broken //
    setTileInAtlas(24,8, world_tiles[TILE_FLOOR_BROKEN_INNER]);       
    world_tiles[TILE_FLOOR_BROKEN_INNER].hasCollision = false;
    world_tiles[TILE_FLOOR_BROKEN_INNER].name = "broken_floor_inner";
    
    setTileInAtlas(23,8, world_tiles[TILE_FLOOR_BROKEN_MIDDLE]);       
    world_tiles[TILE_FLOOR_BROKEN_MIDDLE].hasCollision = false;
    world_tiles[TILE_FLOOR_BROKEN_MIDDLE].name = "broken_floor_middle";

    setTileInAtlas(22,8, world_tiles[TILE_FLOOR_BROKEN_OUTER]);       
    world_tiles[TILE_FLOOR_BROKEN_OUTER].hasCollision = false;
    world_tiles[TILE_FLOOR_BROKEN_OUTER].name = "broken_floor_outer";

    // Outer Wall //
    setTileInAtlas(22,16, world_tiles[TILE_WALL_CENTER]);       
    world_tiles[TILE_WALL_CENTER].name = "wall_center";

    setTileInAtlas(21,16, world_tiles[TILE_WALL_LEFT]);        
    world_tiles[TILE_WALL_LEFT].name = "wall_left";
    
    setTileInAtlas(23,16, world_tiles[TILE_WALL_RIGHT]);        
    world_tiles[TILE_WALL_RIGHT].name = "wall_right";
    
    setTileInAtlas(22,15, world_tiles[TILE_WALL_UP]);        
    world_tiles[TILE_WALL_UP].name = "wall_up";
    
    setTileInAtlas(22,17, world_tiles[TILE_WALL_DOWN]);       
    world_tiles[TILE_WALL_DOWN].name = "wall_down";

    setTileInAtlas(21,15, world_tiles[TILE_WALL_CORNER_TOPLEFT]);       
    world_tiles[TILE_WALL_CORNER_TOPLEFT].name = "wall_corner_top_left";
    
    setTileInAtlas(23,15, world_tiles[TILE_WALL_CORNER_TOPRIGHT]);        
    world_tiles[TILE_WALL_CORNER_TOPRIGHT].name = "wall_corner_top_right";
    
    setTileInAtlas(21,17, world_tiles[TILE_WALL_CORNER_BOTTOMLEFT]);        
    world_tiles[TILE_WALL_CORNER_BOTTOMLEFT].name = "wall_corner_bottom_left";
    
    setTileInAtlas(23,17, world_tiles[TILE_WALL_CORNER_BOTTOMRIGHT]);        
    world_tiles[TILE_WALL_CORNER_BOTTOMRIGHT].name = "wall_corner_bottom_right";

    setTileInAtlas(25,16, world_tiles[TILE_WALL_ISLAND]);        
    world_tiles[TILE_WALL_ISLAND].name = "wall_island";

    setTileInAtlas(25,15, world_tiles[TILE_WALL_PENINSULA_TOP]);        
    world_tiles[TILE_WALL_PENINSULA_TOP].name = "wall_peninsula_top";
    
    setTileInAtlas(25,17, world_tiles[TILE_WALL_PENINSULA_DOWN]);        
    world_tiles[TILE_WALL_PENINSULA_DOWN].name = "wall_peninsula_down";
    
    setTileInAtlas(24,16, world_tiles[TILE_WALL_PENINSULA_LEFT]);        
    world_tiles[TILE_WALL_PENINSULA_LEFT].name = "wall_peninsula_left";
    
    setTileInAtlas(26,16, world_tiles[TILE_WALL_PENINSULA_RIGHT]);        
    world_tiles[TILE_WALL_PENINSULA_RIGHT].name = "wall_peninsula_right";

    setTileInAtlas(23,18, world_tiles[TILE_WALL_COLUMN_UP]);        
    world_tiles[TILE_WALL_COLUMN_UP].name = "wall_column_up";
    
    setTileInAtlas(22,18, world_tiles[TILE_WALL_COLUMN_SIDE]);        
    world_tiles[TILE_WALL_COLUMN_SIDE].name = "wall_column_side";

    setTileInAtlas(26,10, world_tiles[TILE_COSMETIC_ROCK_1]);        
    world_tiles[TILE_COSMETIC_ROCK_1].name = "rock_1";
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

void _world::drawWorld(float left, float right, float top, float bottom)
{
    // Build VBO data for drawing culled chunks in viewport range
    buildWorldVBO(left,right,top,bottom);
   
    // Since its an atlas we only need one text bind. Each tile takes a snippit of the atlas
    glUseProgram(shader.getProgram());
    
    tileAtlas->bindTexture();

    glUniformMatrix4fv(u_viewProjectionMatrix, 1, GL_FALSE, glm::value_ptr(viewProjectionMatrix));
    glUniform1i(u_texture, 0); 
    glUniform2f(u_cameraPos, cameraPosition.x, cameraPosition.y);
    glUniform1f(u_time,time);

    sceneLightManager->applyLights(shader.getProgram());

    // Calculate which chunks are visible
    const int minChunkX = (int)floor(left / (16 * TILE_W));
    const int maxChunkX = (int)ceil(right / (16 * TILE_W));
    const int minChunkY = (int)floor(bottom / (16 * TILE_H));
    const int maxChunkY = (int)ceil(top / (16 * TILE_H));

    constexpr GLsizei indicesPerChunk = NUM_TILES_CHUNK * 6;                        // How many indicies a chunk takes up
    const GLsizei indicesPerLayer = indicesPerChunk * configuration.num_chunks;     // How many indicies an entire layer takes up 
    
    glBindVertexArray(vaoID);
    
    for (int chunkY = minChunkY; chunkY < maxChunkY; chunkY++) {
        for (int chunkX = minChunkX; chunkX < maxChunkX; chunkX++) {
            // Uses unordered map to get a reference to the chunk (since we arent iterating through entire vector)
            const _chunk* chunk = getChunkAt(Vec2i(chunkX, chunkY));
            if (chunk == nullptr) { continue; }
            
            // Draw Tiles Per Chunk //
            const GLsizei chunkIndexByteOffset = chunk->getVboIndex() * indicesPerChunk * sizeof(uint32_t);   // Which byte index to start reading from
            
            for (int layer = 0; layer < NUM_LAYERS; layer++) {
                const GLsizei layerIndexByteOffset = layer * indicesPerLayer * sizeof(uint32_t);
                const GLsizei drawOffsetBytes = layerIndexByteOffset + chunkIndexByteOffset;
                /**
                 * Draw per layer, layer 0 is the LAYER_FLOOR and top layer is LAYER_PRIMARY
                 * Each layer is in the same buffer, but just one length over
                 */
                glDrawElements(GL_TRIANGLES, indicesPerChunk, GL_UNSIGNED_INT, (void*)(drawOffsetBytes));
            }
        }
    }
    
    glBindVertexArray(0);
    
    glUseProgram(0);

    // Draw everything else before image bind of world
    cellParticles->drawParticleManager();
}

void _world::updateWorld(double dt) {
    time += dt;
    cellParticles->updateParticleManger(dt);
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

    const int worldWidth = (int)sqrt(configuration.num_chunks)*16;

    vector<uint8_t> world_noise_primary_copy(world_noise[LAYER_PRIMARY]);

    uniform_int_distribution<uint8_t> boss_dist(TILE_FLOOR_BOSS_BLANK_1, TILE_FLOOR_BOSS_BLANK_2); 
    uniform_int_distribution<uint8_t> outer_dist(TILE_FLOOR_OUTER_BLANK_1, TILE_FLOOR_OUTER_BLANK_2); 
    uniform_int_distribution<uint8_t> middle_dist(TILE_FLOOR_OUTER_DEFAULT_1, TILE_FLOOR_OUTER_DEFAULT_2); 
    uniform_int_distribution<uint8_t> inner_dist(TILE_FLOOR_INNER_DEFAULT_1, TILE_FLOOR_INNER_DEFAULT_2); 
    uniform_real_distribution<float> dist(0.0f,1.0f);

    // Positions (world units) where one biome ends and other begins
    const float innerCutoff = configuration.inner_cutoff * worldBounds;
    const float middleCutoff = configuration.middle_cutoff * worldBounds;
    const float outerCutoff = configuration.outer_cutoff * worldBounds;

    const float innerBlendRadius = configuration.inner_biome_blend_radius * TILE_W;
    const float middleBlendRadius = configuration.middle_biome_blend_radius * TILE_W;
    const float outerBlendRadius = configuration.outer_biome_blend_radius * TILE_W;
    
    // FLOOR TILE //
    for (int i = 0; i < world_noise[LAYER_FLOOR].size(); i++) {
        const int col = i % worldWidth;                                 // Which column
        const int row = i / worldWidth;                                 // Which row
        
        const float tilePosX = (-worldWidth * 0.5f + col) * 16.0f;      // Get world pos X
        const float tilePosY = (worldWidth * 0.5f - row) * 16.0f;       // Get world pos Y

        Vec2f tilePos = {tilePosX, tilePosY};
        const float distance = tilePos.distance({0.0f,0.0f});
        level_pos level = getLevelFromPos(tilePos);
        
        switch (level) {
            case LEVEL_BOSS: 
                world_noise[LAYER_FLOOR][i] = boss_dist(rng);
                break;
            case LEVEL_INNER:
                if (distance < innerCutoff + innerBlendRadius) {
                    // Boss Room //
                    float transitionProgress = (distance - innerCutoff) / innerBlendRadius; // 0.0 at innerCutoff, 1.0 at innerCutoff + innerBlendRadius
                    transitionProgress = glm::clamp(transitionProgress, 0.0f, 1.0f);

                    if (dist(rng) > transitionProgress) {
                        world_noise[LAYER_FLOOR][i] = boss_dist(rng);
                    } else {
                        world_noise[LAYER_FLOOR][i] = inner_dist(rng);
                    }
                } else {
                    world_noise[LAYER_FLOOR][i] = inner_dist(rng);
                }
                break;
            case LEVEL_MIDDLE:
                if (distance < middleCutoff + middleBlendRadius) {
                    // Transition period between INNER and MIDDLE
                    float transitionProgress = (distance - middleCutoff) / middleBlendRadius; // 0.0 at middleCutoff, 1.0 at middleCutoff + middleBlendRadius
                    transitionProgress = glm::clamp(transitionProgress, 0.0f, 1.0f);
                    
                    if (dist(rng) > transitionProgress) {
                        world_noise[LAYER_FLOOR][i] = inner_dist(rng); // Blend toward middle tiles
                    } else {
                        world_noise[LAYER_FLOOR][i] = middle_dist(rng);
                    }
                } else {
                    world_noise[LAYER_FLOOR][i] = middle_dist(rng);
                }
                break;
            case LEVEL_OUTER:
                if (distance < outerCutoff + outerBlendRadius) {
                    // Transition period between MIDDLE and OUTER
                    float transitionProgress = (distance - outerCutoff) / outerBlendRadius; // 0.0 at outerCutoff, 1.0 at outerCutoff + outerBlendRadius
                    transitionProgress = glm::clamp(transitionProgress, 0.0f, 1.0f);
                    
                    if (dist(rng) > transitionProgress) {
                        world_noise[LAYER_FLOOR][i] = middle_dist(rng); // Blend toward middle tiles
                    } else {
                        world_noise[LAYER_FLOOR][i] = outer_dist(rng);
                    }
                } else {
                    world_noise[LAYER_FLOOR][i] = outer_dist(rng);
                }
                break;
        }
    }

    // COSMETIC 1 //
    for (int i = 0; i < world_noise[LAYER_COSMETIC_1].size(); i++) {
        if (dist(rng) > 0.95f) {
            world_noise[LAYER_COSMETIC_1][i] = TILE_COSMETIC_ROCK_1;
        }
    }

    // PRIMARY TILE //
    for (int i = 0; i < world_noise[LAYER_PRIMARY].size(); i++) {
        const int col = i % worldWidth;                                 // Which column
        const int row = i / worldWidth;                                 // Which row
        
        const float tilePosX = (-worldWidth * 0.5f + col) * 16.0f;      // Get world pos X
        const float tilePosY = (worldWidth * 0.5f - row) * 16.0f;       // Get world pos Y

        Vec2f tilePos = {tilePosX, tilePosY};
        const float distance = tilePos.distance({0.0f,0.0f});
        level_pos level = getLevelFromPos(tilePos);

        if (!world_noise_primary_copy[i]) {
            world_noise[LAYER_PRIMARY][i] = TILE_NULL;
            continue;
        }
        /*
        Wall Tile

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
            if (index < 0 || index >= world_noise[LAYER_PRIMARY].size()) { // If index is out of bounds, treat as wall
                neighborTiles[j] = true;
                continue;
            }
            if (world_noise_primary_copy[index]) {  // Check index, true means wall
                neighborTiles[j] = true;
            }
        }
        world_noise[LAYER_PRIMARY][i] = determineTileType(level, neighborTiles);
    }
    Logger.LogInfo("Finishing post processing of world");
}

// This is awful and should be changed to layered at some point
TileId _world::determineTileType(level_pos level, const bool neighborTiles[9]) const {
    // It is not elegant but it is fast and easy to understand
    // The neighborTiles array is a 3x3 grid of booleans where true means there is a wall
    // 0 1 2
    // 3 4 5
    // 6 7 8

    bool N = neighborTiles[1];
    bool S = neighborTiles[7];
    bool E = neighborTiles[5];
    bool W = neighborTiles[3];
    bool NW = neighborTiles[0];
    bool NE = neighborTiles[2];
    bool SW = neighborTiles[6];
    bool SE = neighborTiles[8];

    if (!N && !W && !E && !S) return TILE_WALL_ISLAND;
    
    // Peninsula Checks //
    if (!N && !W && !S && E) return TILE_WALL_PENINSULA_LEFT;
    if (!N && !E && !S && W) return TILE_WALL_PENINSULA_RIGHT;
    if (N && !W && !E && !S) return TILE_WALL_PENINSULA_DOWN;
    if (!W && !E && S && !N) return TILE_WALL_PENINSULA_TOP;

    // Column Checks //
    if (N && S && !W && !E) return TILE_WALL_COLUMN_UP;
    if (!N && !S && W && E) return TILE_WALL_COLUMN_SIDE;

    // Wall Checks //
    if (!W && N && S && E) return TILE_WALL_LEFT;
    if (!E && N && S && W) return TILE_WALL_RIGHT;
    if (N && W && E && !S) return TILE_WALL_DOWN;
    if (!N && W && E && S) return TILE_WALL_UP;

    // Wall Corners //
    if (!W && !S && N && E) return TILE_WALL_CORNER_BOTTOMLEFT;
    if (!N && !W && E && S) return TILE_WALL_CORNER_TOPLEFT;
    if (!E && !S && N && W) return TILE_WALL_CORNER_BOTTOMRIGHT;
    if (!N && !E && W && S) return TILE_WALL_CORNER_TOPRIGHT;
    if (N && E && S && W) return TILE_WALL_CENTER;
    
    return TILE_NULL;
}

/*
When tiles are made from noise its flat but since we load chunk by chunk we have to convert this flat array into 
a coordinate system for chunks
*/
void _world::finalizeWorld() {
    Logger.LogDebug("Mapping world noise into tiles");
    
    const int worldWidth = (int)sqrt(configuration.num_chunks) * 16;
    const int worldHeight = (int)sqrt(configuration.num_chunks) * 16;

    for (int i = 0; i < configuration.num_chunks; i++) {
        const int new_chunkX = i % (int)sqrt(configuration.num_chunks) - floor(sqrt(configuration.num_chunks) / 2);
        const int new_chunkY = i / (int)sqrt(configuration.num_chunks) - floor(sqrt(configuration.num_chunks) / 2);

        worldChunks.emplace_back();

        _chunk* newChunk = &worldChunks.back();
        newChunk->chunkX = new_chunkX;
        newChunk->chunkY = new_chunkY;

        // Calculate the starting position of this chunk in the world grid
        const int chunkStartX = (new_chunkX + (int)floor(sqrt(configuration.num_chunks) / 2)) * 16;
        const int chunkStartY = (new_chunkY + (int)floor(sqrt(configuration.num_chunks) / 2)) * 16;

        // Extract the 16x16 tile section for this chunk from world_noise
        for (int tileY = 0; tileY < 16; tileY++) {
            for (int tileX = 0; tileX < 16; tileX++) {
                // Calculate position in world grid
                const int worldX = chunkStartX + tileX;
                const int worldY = chunkStartY + tileY;
                
                // Convert to flat array index
                const int world_noise_index = worldY * worldWidth + worldX;
                
                // Convert to chunk tile index (tileY * 16 + tileX gives position in chunk's 16x16 grid)
                const int chunk_tile_index = tileY * 16 + tileX;
                
                const float halfWidth = TILE_W * 0.5f;
                const float halfHeight = TILE_H * 0.5f;

                // Positions of the tiles/cells
                const float worldXCenter = (newChunk->chunkX * 16 + tileX) * TILE_W + halfWidth;
                const float worldYCenter = (newChunk->chunkY * 16 + tileY) * TILE_H + halfHeight;
                
                _cell* cell = newChunk->cellAt(chunk_tile_index);
                
                // Apply tile IDs from the noise to each chunk
                for (uint8_t layer = 0; layer < NUM_LAYERS; layer++) {
                    const TileId newId = static_cast<TileId>(world_noise[layer][world_noise_index]);
                    cell->tileIDs[layer] = newId;   
                }
                
                // Set Cell Data //
                cell->setHealth(100.0f);
                cell->setOutline(false);
                cell->index = chunk_tile_index; // Match every draw cycle
                cell->parentChunk = newChunk;

                cell->pos = {worldXCenter, worldYCenter};
            }
        }

        loadedChunks[{new_chunkX, new_chunkY}] = true;
        chunkLookup[{new_chunkX, new_chunkY}] = newChunk;
    }

    // Clear the world generation memory 
    for (int layer = 0; layer < NUM_LAYERS; layer++) {
        world_noise[layer].clear();
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
    // Floor (not truncate) so negative coords land in the correct chunk-local cell.
    int posX = (int)floor(pos.x);
    int posY = (int)floor(pos.y);

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
    if (pos.x > worldBounds || pos.y > worldBounds || pos.x < -worldBounds || pos.y < -worldBounds) {
        // Cell out of bounds
        return nullptr;
    }

    // Floor (not truncate) so negative coords land in the correct chunk-local cell.
    int posX = (int)floor(pos.x);
    int posY = (int)floor(pos.y);

    // The adjust pos is 0-255 for x/y (pos within the chunk). This works with negatives as well.
    Vec2i adjustedPos(modFloor(posX,256),modFloor(posY,256));
    
    // Get chunk present at position
    _chunk* chunk = getChunkAtWorld(pos);
    
    // Get an index in the flat array for the tile
    uint8_t tileIndex = (int)floor(adjustedPos.y/16)*16 + (int)floor(adjustedPos.x/16);

    // Get the id stored in the chunk
    return chunk->cellAt(tileIndex);
}

bool _world::setCellTile(_cell* cell, TileId id) {
    if (!cell || cell->parentChunk == nullptr) return false;
    
    bool success = cell->parentChunk->setTileIdAt(id, cell->index);
    if (success) {
        _cell* neighborCells[9];
        mapCellNeighbors(cell,neighborCells); // Gets a 3x3 map of cell pointers around the center

        // For each cell, rerun the post-processing tile type (requires checking all 8 around it)
        for (int i = 0; i < 9; i++) {
            if (i == 4 || !isCellWall(neighborCells[i])) continue; // Skip center cell and floors
            _cell* localCell = neighborCells[i];
            // We need all 9 cells around each cell we check
            _cell* localNeighborCells[9];
            mapCellNeighbors(localCell,localNeighborCells);

            // Map the cells to booleans for tiles
            bool neighborTiles[9];
            for (int j = 0; j < 9; j++) {
                if (neighborCells[j]) {
                    neighborTiles[j] = isCellWall(localNeighborCells[j]);
                } else {
                    // nullptr (out of bounds treat as wall)
                    neighborTiles[j] = true;
                }
            }
            TileId localTileId = determineTileType(getLevelFromPos(cell->pos), neighborTiles);
            localCell->parentChunk->setTileIdAt(localTileId, localCell->index);
        }
        
        cell->parentChunk->setChunkDirty();  // Mark chunk for rebuild
    }
    return success;
}

bool _world::isTileWall(TileId tileId) const {
    return (tileId >= TILE_WALL_CENTER && tileId <= TILE_WALL_COLUMN_SIDE);
}

// This name should be changed to be something like "cellHasCollision" for when we things beyond walls
bool _world::isCellWall(const _cell* cell) const {
    if (!cell) return false;
    // Any tile that contains a primary layer has collision and is thus a "wall"
    const _tile collisionTile = world_tiles[cell->tileIDs[LAYER_PRIMARY]];
    if (cell->tileIDs[LAYER_PRIMARY] == TILE_NULL) return false;
    return (collisionTile.hasCollision);
}

bool _world::damageCell(_cell* cell, float amount) {
    if (!cell) return false;
    cell->impluseHealth(-amount); // Reverse sign since function expects healing
    cellParticles->spawnEffect(cell->pos, wall_damage_effect);
    if (!cell->isAlive()) {
        level_pos level = getLevelFromPos(cell->pos);
        switch (level) {
            case LEVEL_INNER:
                setCellTile(cell,TILE_FLOOR_BROKEN_INNER);
                break;
            case LEVEL_MIDDLE:
                setCellTile(cell,TILE_FLOOR_BROKEN_MIDDLE);
                break;
            case LEVEL_OUTER:
                setCellTile(cell,TILE_FLOOR_BROKEN_OUTER);
                break;
        }
        cellParticles->spawnEffect(cell->pos,wall_break_effect);
    }
}

vector<chunk_serial_data> _world::exportSerializeWorld() const {
    vector<chunk_serial_data> world_data;
    for (int i = 0; i < configuration.num_chunks; i++) {
        const _chunk* chunk = &worldChunks[i];
        world_data.push_back(chunk->serializeChunk());
    }
    return world_data;
}

void _world::importSerializeWorld(vector<chunk_serial_data> world_data) {
    for (int i = 0; i < world_data.size(); i++) {
        // Build chunk
        worldChunks.reserve(configuration.num_chunks);
        worldChunks.emplace_back();
        _chunk* chunk = &worldChunks.back();
        chunk->loadSerializedChunk(world_data[i]);

        int chunkX = world_data[i].chunkX;
        int chunkY = world_data[i].chunkY;

        loadedChunks[{chunkX, chunkY}] = true;
        chunkLookup[{chunkX, chunkY}] = chunk;
    }
}

void _world::setSeed(uint32_t _seed) {
    seed = _seed;
}

level_pos _world::getLevelFromPos(const Vec2f &pos) const {
    const float distance = pos.distance({0.0f,0.0f});           // How far from center?

    // Positions (world units) where one biome ends and other begins
    const float innerCutoff = configuration.inner_cutoff * worldBounds;
    const float middleCutoff = configuration.middle_cutoff * worldBounds;
    const float outerCutoff = configuration.outer_cutoff * worldBounds;

    if (distance > 0.0f && distance < innerCutoff) {
        return LEVEL_BOSS;
    } else if (distance >= innerCutoff && distance < middleCutoff) {
        return LEVEL_INNER;
    } else if (distance >= middleCutoff && distance < outerCutoff) {
        return LEVEL_MIDDLE;
    } else {
        return LEVEL_OUTER;
    }
}

void _world::importWorldConfiguration(const world_config &_world_config) {
    configuration = _world_config;
}

// -- PRIVATE -- //

void _world::buildWorldVBO(float left, float right, float top, float bottom) {
    // Calculate which chunks are visible
    const int minChunkX = (int)floor(left / (16 * TILE_W));
    const int maxChunkX = (int)ceil(right / (16 * TILE_W));
    const int minChunkY = (int)floor(bottom / (16 * TILE_H));
    const int maxChunkY = (int)ceil(top / (16 * TILE_H));

    const int numChunksToRender = (maxChunkX - minChunkX) * (maxChunkY - minChunkY);    // Total chunks in visible range

    // (Number of tiles in a chunk) * (7 floats per vertex) * (4 verticies per tile) * (bytes per float)
    constexpr GLsizei bytesPerChunk = NUM_TILES_CHUNK * 7 * 4 * sizeof(float); 
    const GLsizei bytesPerLayer = configuration.num_chunks * bytesPerChunk; // How many bytes an entire layer takes up (sizeof chunk * how many)

    glBindBuffer(GL_ARRAY_BUFFER, vboID);

    for (int chunkY = minChunkY; chunkY < maxChunkY; chunkY++) {
        for (int chunkX = minChunkX; chunkX < maxChunkX; chunkX++) {
            const Vec2i chunkPos(chunkX,chunkY);
            _chunk* chunk = getChunkAt(chunkPos);

            if (!chunk) {
                cout << "ERROR: Could not find chunk at (" << chunkX << ", " << chunkY << ")\n";
                continue;
            }
            
            if (!chunk->isChunkDirty()) continue; // Skip chunks with unchanged data

            const _cell* cellList = chunk->getAllCells();
            
            for (uint8_t layer = 0; layer < NUM_LAYERS; layer++) {

                // 256 tiles * 4 verticies * 7 floats per vertex
                vector<float> chunkVboData(NUM_TILES_CHUNK * 4 * 7);
                int vIndex = 0;

                // For each tile of the chunk //
                for (int y = 0; y < 16; y++) {
                    for (int x = 0; x < 16; x++) {
                        const int tileIndex = y * 16 + x;
    
                        const _cell& cell = cellList[tileIndex];
                        const _tile& tile = world_tiles[cell.tileIDs[layer]];

                        bool drawTile = (cell.tileIDs[layer] != TILE_NULL);
                        
                        const float halfWidth = drawTile ? TILE_W * 0.5f : 0.0f;
                        const float halfHeight = drawTile ? TILE_H * 0.5f : 0.0f;
    
                        const float worldXCenter = (chunk->chunkX * 16 + x) * TILE_W + halfWidth;
                        const float worldYCenter = (chunk->chunkY * 16 + y) * TILE_H + halfHeight;
    
                        const float cellOutlined = cell.isOutlined() ? 1.0f : 0.0f;  // <= 0.0 is false > 0.0 is true
    
                        // Tile VBO Setup //
                        // The VBO is set up identical to how we would do glVertex2f and glTexCoord2f
                        
                        // Bottom-left
                        chunkVboData[vIndex++] = -halfWidth;
                        chunkVboData[vIndex++] = -halfHeight;
                        chunkVboData[vIndex++] = tile.u0;
                        chunkVboData[vIndex++] = tile.v1;
                        chunkVboData[vIndex++] = worldXCenter;
                        chunkVboData[vIndex++] = worldYCenter;
                        chunkVboData[vIndex++] = cellOutlined;
                        
                        // Bottom-right
                        chunkVboData[vIndex++] = halfWidth;
                        chunkVboData[vIndex++] = -halfHeight;
                        chunkVboData[vIndex++] = tile.u1;
                        chunkVboData[vIndex++] = tile.v1;
                        chunkVboData[vIndex++] = worldXCenter;
                        chunkVboData[vIndex++] = worldYCenter;
                        chunkVboData[vIndex++] = cellOutlined;
                        
                        // Top-right
                        chunkVboData[vIndex++] = halfWidth;
                        chunkVboData[vIndex++] = halfHeight;
                        chunkVboData[vIndex++] = tile.u1;
                        chunkVboData[vIndex++] = tile.v0;
                        chunkVboData[vIndex++] = worldXCenter;
                        chunkVboData[vIndex++] = worldYCenter;
                        chunkVboData[vIndex++] = cellOutlined;
                        
                        // Top-left
                        chunkVboData[vIndex++] = -halfWidth;
                        chunkVboData[vIndex++] = halfHeight;
                        chunkVboData[vIndex++] = tile.u0;
                        chunkVboData[vIndex++] = tile.v0;  
                        chunkVboData[vIndex++] = worldXCenter;
                        chunkVboData[vIndex++] = worldYCenter;
                        chunkVboData[vIndex++] = cellOutlined;
                    }
                }
                const GLsizei offset = (chunk->getVboIndex() * bytesPerChunk) + (bytesPerLayer * layer);

                glBufferSubData(GL_ARRAY_BUFFER, offset, bytesPerChunk, chunkVboData.data()); 
                
                GLenum err = glGetError();
                if (err != GL_NO_ERROR) {
                    std::cout << "OpenGL error after tile glBufferData: " << err << "\n";
                }
            }
            chunk->setChunkClean(); // Mark chunk as "clean" to stop rebuilding buffer until dirty again
        }
    }
    
    glBindBuffer(GL_ARRAY_BUFFER,0);
    
    // How many total tiles for draw command to draw
    tilesToDraw = numChunksToRender * NUM_TILES_CHUNK;
}

void _world::runWorldGeneration() {
    worldChunks.reserve(configuration.num_chunks); // Resize the vector to hold configuration.num_chunks chunks
    
    // Setup seed + rng engine 
    seed = std::chrono::system_clock::now().time_since_epoch().count(); 
    rng = mt19937(seed);

    // Setup world_noise 
    world_noise[LAYER_FLOOR].resize(configuration.num_chunks*256);         // Floor tiles
    world_noise[LAYER_COSMETIC_1].resize(configuration.num_chunks*256);    // Cosmetic Tiles (1st layer)
    world_noise[LAYER_COSMETIC_2].resize(configuration.num_chunks*256);    // Cosmetic Tiles (2nd layer)
    world_noise[LAYER_PRIMARY].resize(configuration.num_chunks*256);       // Wall tiles (run cellular automata w/ moore neighborhood)
    
    Logger.LogInfo("Running world generation for parameters: ");
    Logger.LogInfo(" - Wall Density: " + to_string(configuration.wall_distribution*100.0f) + "%");
    Logger.LogInfo(" - Wall Generation Iterations: " + to_string(configuration.wall_generation_iterations));
    Logger.LogInfo(" - Wet Density: " + to_string(configuration.wet_distribution*100.0f) + "%");
    Logger.LogInfo(" - Wet Generation Iterations: " + to_string(configuration.wet_generation_iterations));
    Logger.LogInfo(" - Seed: " + to_string(seed));

    uniform_real_distribution<float> dist(0.0f,1.0f);
    
    Logger.LogInfo("Establishing world noise for a ratio of " + to_string(configuration.wall_distribution));
    
      // World width/height in tiles
    int worldWidth = (int)sqrt(configuration.num_chunks)*16;
    int worldHeight = (int)sqrt(configuration.num_chunks)*16;

    for (int i = 0; i < world_noise[LAYER_PRIMARY].size(); i++) {
        world_noise[LAYER_FLOOR][i] = TILE_NULL;
        world_noise[LAYER_COSMETIC_1][i] = TILE_NULL;
        world_noise[LAYER_COSMETIC_2][i] = TILE_NULL;
        world_noise[LAYER_PRIMARY][i] = (dist(rng) < configuration.wall_distribution);    // Randomly assigns 0 or 1 based on noise_distribution
    }

    Logger.LogInfo("Finished generating noise of " + to_string(world_noise[LAYER_PRIMARY].size()) + "tiles");

    Logger.LogInfo("Starting cellular automata algorithm for a world of Width: " + to_string(worldWidth) + "and Height: " + to_string(worldHeight) + " tiles");
    
    // Run cellular automata algorithm on walls //
    for (int iteration = 0; iteration < configuration.wall_generation_iterations; iteration++) {
        const vector<uint8_t> world_noise_copy(world_noise[LAYER_PRIMARY]);
        for (int i = 0; i < world_noise[LAYER_PRIMARY].size(); i++) {
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
                if (index < 0 || index >= world_noise[LAYER_PRIMARY].size()) { // If index is out of bounds, treat as wall
                    num_neighbors++;
                    continue;
                }
                if (world_noise_copy[index]) {  // Check index, true means wall
                    num_neighbors++;
                }
            }
            
            // Moore Neighborhood //
            if (num_neighbors > 4) {
                world_noise[LAYER_PRIMARY][i] = true;
            } else {
                world_noise[LAYER_PRIMARY][i] = false;
            }
        }
        Logger.LogDebug(" -- Wall Iteration: " + to_string(iteration) + " completed!");
    }

    // Run cellular automata algorithm on wet //
    for (int iteration = 0; iteration < configuration.wet_generation_iterations; iteration++) {
        const vector<uint8_t> wet_noise_copy(wet_noise);
        for (int i = 0; i < wet_noise.size(); i++) {
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
                if (index < 0 || index >= wet_noise.size()) { // If index is out of bounds, treat as wall
                    num_neighbors++;
                    continue;
                }
                if (wet_noise_copy[index]) {  // Check index, true means wall
                    num_neighbors++;
                }
            }
            
            // Moore Neighborhood //
            if (num_neighbors > 4) {
                wet_noise[i] = true;
            } else {
                wet_noise[i] = false;
            }
        }
        Logger.LogDebug(" -- Wet Iteration: " + to_string(iteration) + " completed!");
    }

    // World modifications -- clear space in center for the boss //
    vector<uint8_t> world_noise_copy(world_noise[LAYER_PRIMARY]);
    for (int i = 0; i < world_noise[LAYER_PRIMARY].size(); i++) {
        const int col = i % worldWidth;                                 // Which column
        const int row = i / worldWidth;                                 // Which row
        
        const float tilePosX = (-worldWidth * 0.5f + col) * 16.0f;      // Get world pos X
        const float tilePosY = (worldWidth * 0.5f - row) * 16.0f;       // Get world pos Y

        Vec2f tilePos = {tilePosX, tilePosY};
        const float distance = tilePos.distance({0.0f,0.0f});

        // Wall tile
        if (world_noise_copy[i] && distance < 400.0f) {
            // Clear out all tiles of distance from center
            world_noise[LAYER_PRIMARY][i] = static_cast<uint8_t>(false);
        }
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

    // cout << "Neighbors: \n" 
    //  << outNeighbors[0]->tileId << ", " << outNeighbors[1]->tileId << ", " << outNeighbors[2]->tileId << "\n"
    //  << outNeighbors[3]->tileId << ", " << outNeighbors[4]->tileId << ", " << outNeighbors[5]->tileId << "\n"
    //  << outNeighbors[6]->tileId << ", " << outNeighbors[7]->tileId << ", " << outNeighbors[8]->tileId << "\n";
}
