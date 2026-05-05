#include <_world.h>

// -- CELL -- // 

bool _cell::setOutline(bool state) {
if (!this || !parentChunk) return false;
    outlined = state;
    parentChunk->vboDirty = true;
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

_chunk::_chunk() {
    // Creates the buffers for the chunk
    glGenBuffers(1, &tileVboID);
    glGenBuffers(1, &tileEboID);
    glGenVertexArrays(1, &tileVaoID);

    // CHUNK EBO SETUP //

    // 6 vertices * 6 floats * 256 tiles per chunk
    uint32_t tileEboData[6 * 256];
    int vertexOffset = 0;
    int eIndex = 0;
    for (int i = 0; i < 256; i++) {
        // Ebo (Two Triangles) //
        // Triangle 1
        tileEboData[eIndex++] = vertexOffset + 0; // BL   
        tileEboData[eIndex++] = vertexOffset + 1; // BR
        tileEboData[eIndex++] = vertexOffset + 2; // TR
        // Triangle 2
        tileEboData[eIndex++] = vertexOffset + 0; // BL
        tileEboData[eIndex++] = vertexOffset + 2; // TR
        tileEboData[eIndex++] = vertexOffset + 3; // TL

        vertexOffset += 4;
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tileEboID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(tileEboData), tileEboData, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);

    // CHUNK VAO SETUP //

    glBindVertexArray(tileVaoID);

    // Setup buffers
    glBindBuffer(GL_ARRAY_BUFFER,tileVboID);            // VBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,tileEboID);    // EBO
    
    // 6 floats per vertex 
    GLsizei stride = 7 * sizeof(float);

    // Setup attributes
    glEnableVertexAttribArray(0);       // Size (vec2)
    glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,stride,(void*)(0 * sizeof(float)));
    glEnableVertexAttribArray(1);       // Texture Coords (vec2)
    glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,stride,(void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(2);       // Position (vec2)
    glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,stride,(void*)(4 * sizeof(float)));
    glEnableVertexAttribArray(3);       // Tile Outlined (float)
    glVertexAttribPointer(3,1,GL_FLOAT,GL_FALSE,stride,(void*)(6 * sizeof(float)));

    glBindVertexArray(0);
}

_chunk::~_chunk() {
    if (tileVboID != 0) {
        glDeleteBuffers(1,&tileVboID); 
        tileVboID = 0;
    }
    if (tileEboID != 0) {
        glDeleteBuffers(1,&tileEboID); 
        tileEboID = 0;
    }
    if (tileVaoID != 0) {
        glDeleteVertexArrays(1,&tileVaoID); 
        tileVaoID = 0;
    }
}

TileId _chunk::getTileIdAt(int index) const {
    if (!this) return TILE_NULL;
    if (index < 0 || index > 255) return TILE_NULL;
    return tileData[index];
}

_cell* _chunk::cellAt(int index) {
    if (index < 0 || index > 255) return nullptr;
    return &cellData[index];
}

bool _chunk::setTileIdAt(TileId id, int index) {
    if (!this) return false;
    if (index < 0 || index > 255) return false;
    
    tileData[index] = id;
    cellData[index].tileId = id;
    vboDirty = true;

    return true;
}

const _cell* _chunk::getAllCells() const {
    return cellData;
}

const TileId* _chunk::getAllTileIds() const {
    return tileData;
}

void _chunk::setAllCells(const _cell* cells) {
    if (!cells) return;
    memcpy(cellData,cells,256 * sizeof(_cell));
    vboDirty = true;
}

void _chunk::setAllTiles(const TileId* tiles) {
    if (!tiles) return;
    memcpy(tileData,tiles,256 * sizeof(TileId));

    // Set all cell IDs to match
    _cell cellData[256];
    for (int i = 0; i < 256; i++) {
        cellData[i].tileId = tiles[i];
    }
    setAllCells(cellData);
    vboDirty = true;
}

chunk_serial_data _chunk::serializeChunk() const {
    chunk_serial_data chunk_data;
    chunk_data.chunkX = chunkX;
    chunk_data.chunkY = chunkY;
    for (int i = 0; i < 256; i++) {
        const _cell* cell = &cellData[i]; 
        chunk_data.cell_data[i].tileID = cell->tileId;
        chunk_data.cell_data[i].outlined = static_cast<uint8_t>(cell->isOutlined());
        chunk_data.cell_data[i].padding = 0;   // Padding, does nothing.
        chunk_data.cell_data[i].health = cell->getHealth();
    }
    return chunk_data;
}

void _chunk::loadSerializedChunk(const chunk_serial_data &chunk_data) {
    chunkX = chunk_data.chunkX;
    chunkY = chunk_data.chunkY;
    for (int i = 0; i < 256; i++) {
        // Serialized Data //
        tileData[i] = static_cast<TileId>(chunk_data.cell_data[i].tileID);
        cellData[i].tileId = static_cast<TileId>(chunk_data.cell_data[i].tileID);
        cellData[i].setOutline(static_cast<bool>(chunk_data.cell_data[i].outlined));
        cellData[i].setHealth(chunk_data.cell_data[i].health);

        // Non Serialized Data //
        // Handled by VBO setup -- should be changed later
        vboDirty = true;
    }
}

// -- WORLD -- //

// STATIC MEMBERS //

glm::mat4 _world::viewProjectionMatrix;
Vec2f _world::cameraPosition = {0.0f, 0.0f};

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

void _world::initWorld(bool loadWorld, _lightManager* lightManager)
{
    sceneLightManager = lightManager;

    if (worldInitialized) {
        cout << "WARNING: World has already been initialized, skipping\n";
        return;
    }

    initBenchmark->startBenchmark();

    // Logger.LogInfo("Initializing world for seed " + to_string(seed), LOG_BOTH);
    Logger.LogInfo("World has " + to_string(numStartingChunks) + " starting chunks.", LOG_BOTH);

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
    
    double sqrtNumChunks = sqrt(numStartingChunks);
    // This checks if a decimal (like 1.3) is equal to its floor (1.0) which indicates the sqrt wasn't perfect
    if (sqrtNumChunks != floor(sqrtNumChunks)) {
        Logger.LogWarning("numStartingChunks is not a perfect square. This may lead to an uneven distribution of chunks around the center.", LOG_BOTH);
    }

    // Only run generation when we dont load the world  
    if (!loadWorld) {
        runWorldGeneration(generation_iterations); 
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

    // Outer Wall //
    setTileInAtlas(22,16, world_tiles[TILE_WALL_OUTER_CENTER]);       
    world_tiles[TILE_WALL_OUTER_CENTER].name = "wall_outer_center";

    setTileInAtlas(21,16, world_tiles[TILE_WALL_OUTER_LEFT]);        
    world_tiles[TILE_WALL_OUTER_LEFT].name = "wall_outer_left";
    
    setTileInAtlas(23,16, world_tiles[TILE_WALL_OUTER_RIGHT]);        
    world_tiles[TILE_WALL_OUTER_RIGHT].name = "wall_outer_right";
    
    setTileInAtlas(22,15, world_tiles[TILE_WALL_OUTER_UP]);        
    world_tiles[TILE_WALL_OUTER_UP].name = "wall_outer_up";
    
    setTileInAtlas(22,17, world_tiles[TILE_WALL_OUTER_DOWN]);       
    world_tiles[TILE_WALL_OUTER_DOWN].name = "wall_outer_down";

    setTileInAtlas(21,15, world_tiles[TILE_WALL_OUTER_CORNER_TOPLEFT]);       
    world_tiles[TILE_WALL_OUTER_CORNER_TOPLEFT].name = "wall_outer_corner_top_left";
    
    setTileInAtlas(23,15, world_tiles[TILE_WALL_OUTER_CORNER_TOPRIGHT]);        
    world_tiles[TILE_WALL_OUTER_CORNER_TOPRIGHT].name = "wall_outer_corner_top_right";
    
    setTileInAtlas(21,17, world_tiles[TILE_WALL_OUTER_CORNER_BOTTOMLEFT]);        
    world_tiles[TILE_WALL_OUTER_CORNER_BOTTOMLEFT].name = "wall_outer_corner_bottom_left";
    
    setTileInAtlas(23,17, world_tiles[TILE_WALL_OUTER_CORNER_BOTTOMRIGHT]);        
    world_tiles[TILE_WALL_OUTER_CORNER_BOTTOMRIGHT].name = "wall_outer_corner_bottom_right";

    setTileInAtlas(25,16, world_tiles[TILE_WALL_OUTER_ISLAND]);        
    world_tiles[TILE_WALL_OUTER_ISLAND].name = "wall_outer_island";

    setTileInAtlas(25,15, world_tiles[TILE_WALL_OUTER_PENINSULA_TOP]);        
    world_tiles[TILE_WALL_OUTER_PENINSULA_TOP].name = "wall_outer_peninsula_top";
    
    setTileInAtlas(25,17, world_tiles[TILE_WALL_OUTER_PENINSULA_DOWN]);        
    world_tiles[TILE_WALL_OUTER_PENINSULA_DOWN].name = "wall_outer_peninsula_down";
    
    setTileInAtlas(24,16, world_tiles[TILE_WALL_OUTER_PENINSULA_LEFT]);        
    world_tiles[TILE_WALL_OUTER_PENINSULA_LEFT].name = "wall_outer_peninsula_left";
    
    setTileInAtlas(26,16, world_tiles[TILE_WALL_OUTER_PENINSULA_RIGHT]);        
    world_tiles[TILE_WALL_OUTER_PENINSULA_RIGHT].name = "wall_outer_peninsula_right";

    setTileInAtlas(23,18, world_tiles[TILE_WALL_OUTER_COLUMN_UP]);        
    world_tiles[TILE_WALL_OUTER_COLUMN_UP].name = "wall_outer_column_up";
    
    setTileInAtlas(22,18, world_tiles[TILE_WALL_OUTER_COLUMN_SIDE]);        
    world_tiles[TILE_WALL_OUTER_COLUMN_SIDE].name = "wall_outer_column_side";

    // Middle Wall //
    setTileInAtlas(15,16, world_tiles[TILE_WALL_MIDDLE_CENTER]);       
    world_tiles[TILE_WALL_MIDDLE_CENTER].name = "wall_middle_center";

    setTileInAtlas(14,16, world_tiles[TILE_WALL_MIDDLE_LEFT]);        
    world_tiles[TILE_WALL_MIDDLE_LEFT].name = "wall_middle_left";
    
    setTileInAtlas(16,16, world_tiles[TILE_WALL_MIDDLE_RIGHT]);        
    world_tiles[TILE_WALL_MIDDLE_RIGHT].name = "wall_middle_right";
    
    setTileInAtlas(15,15, world_tiles[TILE_WALL_MIDDLE_UP]);        
    world_tiles[TILE_WALL_MIDDLE_UP].name = "wall_middle_up";
    
    setTileInAtlas(15,17, world_tiles[TILE_WALL_MIDDLE_DOWN]);       
    world_tiles[TILE_WALL_MIDDLE_DOWN].name = "wall_middle_down";

    setTileInAtlas(14,15, world_tiles[TILE_WALL_MIDDLE_CORNER_TOPLEFT]);       
    world_tiles[TILE_WALL_MIDDLE_CORNER_TOPLEFT].name = "wall_middle_corner_top_left";
    
    setTileInAtlas(16,15, world_tiles[TILE_WALL_MIDDLE_CORNER_TOPRIGHT]);        
    world_tiles[TILE_WALL_MIDDLE_CORNER_TOPRIGHT].name = "wall_middle_corner_top_right";
    
    setTileInAtlas(14,17, world_tiles[TILE_WALL_MIDDLE_CORNER_BOTTOMLEFT]);        
    world_tiles[TILE_WALL_MIDDLE_CORNER_BOTTOMLEFT].name = "wall_middle_corner_bottom_left";
    
    setTileInAtlas(16,17, world_tiles[TILE_WALL_MIDDLE_CORNER_BOTTOMRIGHT]);        
    world_tiles[TILE_WALL_MIDDLE_CORNER_BOTTOMRIGHT].name = "wall_middle_corner_bottom_right";

    setTileInAtlas(18,16, world_tiles[TILE_WALL_MIDDLE_ISLAND]);        
    world_tiles[TILE_WALL_MIDDLE_ISLAND].name = "wall_middle_island";

    setTileInAtlas(18,15, world_tiles[TILE_WALL_MIDDLE_PENINSULA_TOP]);        
    world_tiles[TILE_WALL_MIDDLE_PENINSULA_TOP].name = "wall_middle_peninsula_top";
    
    setTileInAtlas(18,17, world_tiles[TILE_WALL_MIDDLE_PENINSULA_DOWN]);        
    world_tiles[TILE_WALL_MIDDLE_PENINSULA_DOWN].name = "wall_middle_peninsula_down";
    
    setTileInAtlas(17,16, world_tiles[TILE_WALL_MIDDLE_PENINSULA_LEFT]);        
    world_tiles[TILE_WALL_MIDDLE_PENINSULA_LEFT].name = "wall_middle_peninsula_left";
    
    setTileInAtlas(19,16, world_tiles[TILE_WALL_MIDDLE_PENINSULA_RIGHT]);        
    world_tiles[TILE_WALL_MIDDLE_PENINSULA_RIGHT].name = "wall_middle_peninsula_right";

    setTileInAtlas(16,18, world_tiles[TILE_WALL_MIDDLE_COLUMN_UP]);        
    world_tiles[TILE_WALL_MIDDLE_COLUMN_UP].name = "wall_middle_column_up";
    
    setTileInAtlas(15,18, world_tiles[TILE_WALL_MIDDLE_COLUMN_SIDE]);        
    world_tiles[TILE_WALL_MIDDLE_COLUMN_SIDE].name = "wall_middle_column_side";

    // Middle Wall //
    setTileInAtlas(15,16, world_tiles[TILE_WALL_MIDDLE_CENTER]);       
    world_tiles[TILE_WALL_MIDDLE_CENTER].name = "wall_middle_center";

    setTileInAtlas(14,16, world_tiles[TILE_WALL_MIDDLE_LEFT]);        
    world_tiles[TILE_WALL_MIDDLE_LEFT].name = "wall_middle_left";
    
    setTileInAtlas(16,16, world_tiles[TILE_WALL_MIDDLE_RIGHT]);        
    world_tiles[TILE_WALL_MIDDLE_RIGHT].name = "wall_middle_right";
    
    setTileInAtlas(15,15, world_tiles[TILE_WALL_MIDDLE_UP]);        
    world_tiles[TILE_WALL_MIDDLE_UP].name = "wall_middle_up";
    
    setTileInAtlas(15,17, world_tiles[TILE_WALL_MIDDLE_DOWN]);       
    world_tiles[TILE_WALL_MIDDLE_DOWN].name = "wall_middle_down";

    setTileInAtlas(14,15, world_tiles[TILE_WALL_MIDDLE_CORNER_TOPLEFT]);       
    world_tiles[TILE_WALL_MIDDLE_CORNER_TOPLEFT].name = "wall_middle_corner_top_left";
    
    setTileInAtlas(16,15, world_tiles[TILE_WALL_MIDDLE_CORNER_TOPRIGHT]);        
    world_tiles[TILE_WALL_MIDDLE_CORNER_TOPRIGHT].name = "wall_middle_corner_top_right";
    
    setTileInAtlas(14,17, world_tiles[TILE_WALL_MIDDLE_CORNER_BOTTOMLEFT]);        
    world_tiles[TILE_WALL_MIDDLE_CORNER_BOTTOMLEFT].name = "wall_middle_corner_bottom_left";
    
    setTileInAtlas(16,17, world_tiles[TILE_WALL_MIDDLE_CORNER_BOTTOMRIGHT]);        
    world_tiles[TILE_WALL_MIDDLE_CORNER_BOTTOMRIGHT].name = "wall_middle_corner_bottom_right";

    setTileInAtlas(18,16, world_tiles[TILE_WALL_MIDDLE_ISLAND]);        
    world_tiles[TILE_WALL_MIDDLE_ISLAND].name = "wall_middle_island";

    setTileInAtlas(18,15, world_tiles[TILE_WALL_MIDDLE_PENINSULA_TOP]);        
    world_tiles[TILE_WALL_MIDDLE_PENINSULA_TOP].name = "wall_middle_peninsula_top";
    
    setTileInAtlas(18,17, world_tiles[TILE_WALL_MIDDLE_PENINSULA_DOWN]);        
    world_tiles[TILE_WALL_MIDDLE_PENINSULA_DOWN].name = "wall_middle_peninsula_down";
    
    setTileInAtlas(17,16, world_tiles[TILE_WALL_MIDDLE_PENINSULA_LEFT]);        
    world_tiles[TILE_WALL_MIDDLE_PENINSULA_LEFT].name = "wall_middle_peninsula_left";
    
    setTileInAtlas(19,16, world_tiles[TILE_WALL_MIDDLE_PENINSULA_RIGHT]);        
    world_tiles[TILE_WALL_MIDDLE_PENINSULA_RIGHT].name = "wall_middle_peninsula_right";

    setTileInAtlas(16,18, world_tiles[TILE_WALL_MIDDLE_COLUMN_UP]);        
    world_tiles[TILE_WALL_MIDDLE_COLUMN_UP].name = "wall_middle_column_up";
    
    setTileInAtlas(15,18, world_tiles[TILE_WALL_MIDDLE_COLUMN_SIDE]);        
    world_tiles[TILE_WALL_MIDDLE_COLUMN_SIDE].name = "wall_middle_column_side";

    // Inner Wall //
    setTileInAtlas(8,16, world_tiles[TILE_WALL_INNER_CENTER]);       
    world_tiles[TILE_WALL_INNER_CENTER].name = "wall_inner_center";

    setTileInAtlas(7,16, world_tiles[TILE_WALL_INNER_LEFT]);        
    world_tiles[TILE_WALL_INNER_LEFT].name = "wall_inner_left";
    
    setTileInAtlas(9,16, world_tiles[TILE_WALL_INNER_RIGHT]);        
    world_tiles[TILE_WALL_INNER_RIGHT].name = "wall_inner_right";
    
    setTileInAtlas(8,15, world_tiles[TILE_WALL_INNER_UP]);        
    world_tiles[TILE_WALL_INNER_UP].name = "wall_inner_up";
    
    setTileInAtlas(8,17, world_tiles[TILE_WALL_INNER_DOWN]);       
    world_tiles[TILE_WALL_INNER_DOWN].name = "wall_inner_down";

    setTileInAtlas(7,15, world_tiles[TILE_WALL_INNER_CORNER_TOPLEFT]);       
    world_tiles[TILE_WALL_INNER_CORNER_TOPLEFT].name = "wall_inner_corner_top_left";
    
    setTileInAtlas(9,15, world_tiles[TILE_WALL_INNER_CORNER_TOPRIGHT]);        
    world_tiles[TILE_WALL_INNER_CORNER_TOPRIGHT].name = "wall_inner_corner_top_right";
    
    setTileInAtlas(7,17, world_tiles[TILE_WALL_INNER_CORNER_BOTTOMLEFT]);        
    world_tiles[TILE_WALL_INNER_CORNER_BOTTOMLEFT].name = "wall_inner_corner_bottom_left";
    
    setTileInAtlas(9,17, world_tiles[TILE_WALL_INNER_CORNER_BOTTOMRIGHT]);        
    world_tiles[TILE_WALL_INNER_CORNER_BOTTOMRIGHT].name = "wall_inner_corner_bottom_right";

    setTileInAtlas(11,16, world_tiles[TILE_WALL_INNER_ISLAND]);        
    world_tiles[TILE_WALL_INNER_ISLAND].name = "wall_inner_island";

    setTileInAtlas(11,15, world_tiles[TILE_WALL_INNER_PENINSULA_TOP]);        
    world_tiles[TILE_WALL_INNER_PENINSULA_TOP].name = "wall_inner_peninsula_top";
    
    setTileInAtlas(11,17, world_tiles[TILE_WALL_INNER_PENINSULA_DOWN]);        
    world_tiles[TILE_WALL_INNER_PENINSULA_DOWN].name = "wall_inner_peninsula_down";
    
    setTileInAtlas(10,16, world_tiles[TILE_WALL_INNER_PENINSULA_LEFT]);        
    world_tiles[TILE_WALL_INNER_PENINSULA_LEFT].name = "wall_inner_peninsula_left";
    
    setTileInAtlas(12,16, world_tiles[TILE_WALL_INNER_PENINSULA_RIGHT]);        
    world_tiles[TILE_WALL_INNER_PENINSULA_RIGHT].name = "wall_inner_peninsula_right";

    setTileInAtlas(9,18, world_tiles[TILE_WALL_INNER_COLUMN_UP]);        
    world_tiles[TILE_WALL_INNER_COLUMN_UP].name = "wall_inner_column_up";
    
    setTileInAtlas(8,18, world_tiles[TILE_WALL_INNER_COLUMN_SIDE]);        
    world_tiles[TILE_WALL_INNER_COLUMN_SIDE].name = "wall_inner_column_side";
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
    // 4 Verticies of 7 floats and 256 total
    float tileVboData[4 * 7 * 256];
    int vIndex = 0;

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

            float halfWidth = TILE_W * 0.5f;
            float halfHeight = TILE_H * 0.5f;

            float worldXCenter = (chunk->chunkX * 16 + x) * TILE_W + halfWidth;
            float worldYCenter = (chunk->chunkY * 16 + y) * TILE_H + halfHeight;

            cell->pos = {worldXCenter, worldYCenter};

            float cellOutlined = cell->isOutlined() ? 1.0f : 0.0f;  // <= 0.0 is false > 0.0 is true

            // Tile VBO Setup //
            // The VBO is set up identical to how we would do glVertex2f and glTexCoord2f
            
            // Bottom-left
            tileVboData[vIndex++] = -halfWidth;
            tileVboData[vIndex++] = -halfHeight;
            tileVboData[vIndex++] = tile->u0;
            tileVboData[vIndex++] = tile->v1;
            tileVboData[vIndex++] = worldXCenter;
            tileVboData[vIndex++] = worldYCenter;
            tileVboData[vIndex++] = cellOutlined;
            
            // Bottom-right
            tileVboData[vIndex++] = halfWidth;
            tileVboData[vIndex++] = -halfHeight;
            tileVboData[vIndex++] = tile->u1;
            tileVboData[vIndex++] = tile->v1;
            tileVboData[vIndex++] = worldXCenter;
            tileVboData[vIndex++] = worldYCenter;
            tileVboData[vIndex++] = cellOutlined;
            
            // Top-right
            tileVboData[vIndex++] = halfWidth;
            tileVboData[vIndex++] = halfHeight;
            tileVboData[vIndex++] = tile->u1;
            tileVboData[vIndex++] = tile->v0;
            tileVboData[vIndex++] = worldXCenter;
            tileVboData[vIndex++] = worldYCenter;
            tileVboData[vIndex++] = cellOutlined;
            
            // Top-left
            tileVboData[vIndex++] = -halfWidth;
            tileVboData[vIndex++] = halfHeight;
            tileVboData[vIndex++] = tile->u0;
            tileVboData[vIndex++] = tile->v0;  
            tileVboData[vIndex++] = worldXCenter;
            tileVboData[vIndex++] = worldYCenter;
            tileVboData[vIndex++] = cellOutlined;
        }
    }
    // Tile Data VBO //
    glBindBuffer(GL_ARRAY_BUFFER, chunk->tileVboID); // Working with this specific buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(tileVboData), tileVboData, GL_STATIC_DRAW); // Copy the system memory buffer (tileVboData) into a GPU memory buffer
    glBindBuffer(GL_ARRAY_BUFFER, 0); // Make sure to set vbo to 0 (nothing) to signal we're done working with this tileVboID

    chunk->vboDirty = false;
}

void _world::drawWorld(float left, float right, float top, float bottom)
{
    // Since its an atlas we only need one text bind. Each tile takes a snippit of the atlas
    glUseProgram(shader.getProgram());
    
    tileAtlas->bindTexture();

    glUniformMatrix4fv(u_viewProjectionMatrix, 1, GL_FALSE, glm::value_ptr(viewProjectionMatrix));
    glUniform1i(u_texture, 0); 
    glUniform2f(u_cameraPos, cameraPosition.x, cameraPosition.y);
    glUniform1f(u_time,time);

    sceneLightManager->applyLights(shader.getProgram());

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

            // Draw Tiles  //
            glBindVertexArray(chunk->tileVaoID);
            glDrawElements(GL_TRIANGLES, 256 * 7, GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
        }
    }

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

    const int worldWidth = (int)sqrt(numStartingChunks)*16;

    vector<uint8_t> world_noise_copy(world_noise);
    uniform_int_distribution<uint8_t> outer_dist(TILE_FLOOR_OUTER_BLANK_1, TILE_FLOOR_OUTER_BLANK_2); 
    uniform_int_distribution<uint8_t> middle_dist(TILE_FLOOR_OUTER_DEFAULT_1, TILE_FLOOR_OUTER_DEFAULT_2); 
    uniform_int_distribution<uint8_t> inner_dist(TILE_FLOOR_INNER_DEFAULT_1, TILE_FLOOR_INNER_DEFAULT_2); 

    for (int i = 0; i < world_noise.size(); i++) {
        const int col = i % worldWidth;                                 // Which column
        const int row = i / worldWidth;                                 // Which row
        
        const float tilePosX = (-worldWidth * 0.5f + col) * 16.0f;      // Get world pos X
        const float tilePosY = (worldWidth * 0.5f - row) * 16.0f;       // Get world pos Y

        Vec2f tilePos = {tilePosX, tilePosY};
        level_pos level = getLevelFromPos(tilePos);

        // Floor tile
        if (!world_noise_copy[i]) {
            switch (level) {
                case LEVEL_INNER:
                    world_noise[i] = inner_dist(rng);
                    break;
                case LEVEL_MIDDLE:
                    world_noise[i] = middle_dist(rng);
                    break;
                case LEVEL_OUTER:
                    world_noise[i] = outer_dist(rng);
                    break;
            }
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
            if (index < 0 || index >= world_noise.size()) { // If index is out of bounds, treat as wall
                neighborTiles[j] = true;
                continue;
            }
            if (world_noise_copy[index]) {  // Check index, true means wall
                neighborTiles[j] = true;
            }
        }
        world_noise[i] = determineTileType(level, neighborTiles);
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

    switch (level) {
        case LEVEL_OUTER:
            // Island Check //
            if (!N && !W && !E && !S) return TILE_WALL_OUTER_ISLAND;
            
            // Peninsula Checks //
            if (!N && !W && !S && E) return TILE_WALL_OUTER_PENINSULA_LEFT;
            if (!N && !E && !S && W) return TILE_WALL_OUTER_PENINSULA_RIGHT;
            if (N && !W && !E && !S) return TILE_WALL_OUTER_PENINSULA_DOWN;
            if (!W && !E && S && !N) return TILE_WALL_OUTER_PENINSULA_TOP;

            // Column Checks //
            if (N && S && !W && !E) return TILE_WALL_OUTER_COLUMN_UP;
            if (!N && !S && W && E) return TILE_WALL_OUTER_COLUMN_SIDE;

            // Wall Checks //
            if (!W && N && S && E) return TILE_WALL_OUTER_LEFT;
            if (!E && N && S && W) return TILE_WALL_OUTER_RIGHT;
            if (N && W && E && !S) return TILE_WALL_OUTER_DOWN;
            if (!N && W && E && S) return TILE_WALL_OUTER_UP;

            // Wall Corners //
            if (!W && !S && N && E) return TILE_WALL_OUTER_CORNER_BOTTOMLEFT;
            if (!N && !W && E && S) return TILE_WALL_OUTER_CORNER_TOPLEFT;
            if (!E && !S && N && W) return TILE_WALL_OUTER_CORNER_BOTTOMRIGHT;
            if (!N && !E && W && S) return TILE_WALL_OUTER_CORNER_TOPRIGHT;
            if (N && E && S && W) return TILE_WALL_OUTER_CENTER;
            break;
        case LEVEL_MIDDLE:
            // Island Check //
            if (!N && !W && !E && !S) return TILE_WALL_MIDDLE_ISLAND;
            
            // Peninsula Checks //
            if (!N && !W && !S && E) return TILE_WALL_MIDDLE_PENINSULA_LEFT;
            if (!N && !E && !S && W) return TILE_WALL_MIDDLE_PENINSULA_RIGHT;
            if (N && !W && !E && !S) return TILE_WALL_MIDDLE_PENINSULA_DOWN;
            if (!W && !E && S && !N) return TILE_WALL_MIDDLE_PENINSULA_TOP;

            // Column Checks //
            if (N && S && !W && !E) return TILE_WALL_MIDDLE_COLUMN_UP;
            if (!N && !S && W && E) return TILE_WALL_MIDDLE_COLUMN_SIDE;

            // Wall Checks //
            if (!W && N && S && E) return TILE_WALL_MIDDLE_LEFT;
            if (!E && N && S && W) return TILE_WALL_MIDDLE_RIGHT;
            if (N && W && E && !S) return TILE_WALL_MIDDLE_DOWN;
            if (!N && W && E && S) return TILE_WALL_MIDDLE_UP;

            // Wall Corners //
            if (!W && !S && N && E) return TILE_WALL_MIDDLE_CORNER_BOTTOMLEFT;
            if (!N && !W && E && S) return TILE_WALL_MIDDLE_CORNER_TOPLEFT;
            if (!E && !S && N && W) return TILE_WALL_MIDDLE_CORNER_BOTTOMRIGHT;
            if (!N && !E && W && S) return TILE_WALL_MIDDLE_CORNER_TOPRIGHT;
            if (N && E && S && W) return TILE_WALL_MIDDLE_CENTER;
            break;
        case LEVEL_INNER:
            // Island Check //
            if (!N && !W && !E && !S) return TILE_WALL_INNER_ISLAND;
            
            // Peninsula Checks //
            if (!N && !W && !S && E) return TILE_WALL_INNER_PENINSULA_LEFT;
            if (!N && !E && !S && W) return TILE_WALL_INNER_PENINSULA_RIGHT;
            if (N && !W && !E && !S) return TILE_WALL_INNER_PENINSULA_DOWN;
            if (!W && !E && S && !N) return TILE_WALL_INNER_PENINSULA_TOP;

            // Column Checks //
            if (N && S && !W && !E) return TILE_WALL_INNER_COLUMN_UP;
            if (!N && !S && W && E) return TILE_WALL_INNER_COLUMN_SIDE;

            // Wall Checks //
            if (!W && N && S && E) return TILE_WALL_INNER_LEFT;
            if (!E && N && S && W) return TILE_WALL_INNER_RIGHT;
            if (N && W && E && !S) return TILE_WALL_INNER_DOWN;
            if (!N && W && E && S) return TILE_WALL_INNER_UP;

            // Wall Corners //
            if (!W && !S && N && E) return TILE_WALL_INNER_CORNER_BOTTOMLEFT;
            if (!N && !W && E && S) return TILE_WALL_INNER_CORNER_TOPLEFT;
            if (!E && !S && N && W) return TILE_WALL_INNER_CORNER_BOTTOMRIGHT;
            if (!N && !E && W && S) return TILE_WALL_INNER_CORNER_TOPRIGHT;
            if (N && E && S && W) return TILE_WALL_INNER_CENTER;
            break;
    }

    

    return TILE_NULL;
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

        worldChunks.emplace_back();

        _chunk* newChunk = &worldChunks.back();
        newChunk->chunkX = new_chunkX;
        newChunk->chunkY = new_chunkY;

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

                newChunk->setTileIdAt(newId,chunk_tile_index);
            }
        }

        loadedChunks[{new_chunkX, new_chunkY}] = true;
        chunkLookup[{new_chunkX, new_chunkY}] = newChunk;
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
    if (pos.x > worldBounds || pos.y > worldBounds || pos.x < -worldBounds || pos.y < -worldBounds) {
        // Cell out of bounds
        return nullptr;
    }

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

bool _world::setCellTile(_cell* cell, TileId id) {
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
            TileId localTileId = determineTileType(getLevelFromPos(cell->pos), neighborTiles);
            localCell->parentChunk->setTileIdAt(localTileId, localCell->index);
        }
        
        cell->parentChunk->vboDirty = true;  // Mark chunk for rebuild
    }
    return success;
}

bool _world::isTileWall(TileId tileId) const {
    return (tileId >= TILE_WALL_OUTER_CENTER && tileId <= TILE_WALL_INNER_COLUMN_SIDE);
}

bool _world::isCellWall(const _cell* cell) const {
    if (!cell) return false;
    return (cell->tileId >= TILE_WALL_OUTER_CENTER && cell->tileId <= TILE_WALL_INNER_COLUMN_SIDE);
}

bool _world::damageCell(_cell* cell, float amount) {
    if (!cell) return false;
    cell->impluseHealth(-amount); // Reverse sign since function expects healing
    cellParticles->spawnEffect(cell->pos, wall_damage_effect);
    if (!cell->isAlive()) {
        setCellTile(cell,TILE_NULL);
        cellParticles->spawnEffect(cell->pos,wall_break_effect);
    }
}

vector<chunk_serial_data> _world::exportSerializeWorld() const {
    vector<chunk_serial_data> world_data;
    for (int i = 0; i < numStartingChunks; i++) {
        const _chunk* chunk = &worldChunks[i];
        world_data.push_back(chunk->serializeChunk());
    }
    return world_data;
}

void _world::importSerializeWorld(vector<chunk_serial_data> world_data) {
    for (int i = 0; i < world_data.size(); i++) {
        // Build chunk
        worldChunks.reserve(numStartingChunks);
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

    if (distance > 0.0f && distance < 4000.0f) {
        return LEVEL_INNER;
    } else if (distance >= 4000.0f && distance < 8000.0f) {
        return LEVEL_MIDDLE;
    } else {
        return LEVEL_OUTER;
    }
}


// -- PRIVATE -- //

void _world::runWorldGeneration(int iterations) {
    worldChunks.reserve(numStartingChunks); // Resize the vector to hold numStartingChunks chunks
    
    // Setup seed + rng engine 
    seed = std::chrono::system_clock::now().time_since_epoch().count(); 
    rng = mt19937(seed);

    // Setup world_noise
    world_noise.resize(numStartingChunks*256);  // 256 tiles per chunk
    
    Logger.LogInfo("Running world generation for parameters: ");
    Logger.LogInfo(" - Noise Density: " + to_string(noise_distribution*100.0f) + "%");
    Logger.LogInfo(" - Generation Iterations: " + to_string(iterations));
    Logger.LogInfo(" - Seed: " + to_string(seed));

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

    // Clean up data once world gen is done since no longer used
    world_noise.clear();
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