/**
 *  _world.h
 * 
 *  There are a few different position systems, world position and chunk position.
 * 
 *  World position is use for OpenGL orthographic drawing, it starts at (0,0) where +X is right, -X is left
 *  +Y is up, and -Y is down. These units are in pixels.
 * 
 *  Chunk position is used for placement of chunks. It uses the same scheme for x,y and starts at (0,0). However, these 
 *  are integers. More detail in the "_chunk" class. 
 * 
 *  
 * 
 */
#ifndef _WORLD_H
#define _WORLD_H

#define TILE_W 16.0f   // Tile width in world units is ALWAYS the same of 16
#define TILE_H 16.0f   // Tile height in world units is ALWAYS the same of 16
#define NUM_CHUNKS 16384;

#include <_common.h>
#include <_texture.h>
#include <_benchmark.h>
#include <_particleManager.h>
#include <_shader.h>
#include <_lightManager.h>

#include <glm/glm.hpp>                      // Core library
#include <glm/gtc/matrix_transform.hpp>     // Matrix ops like transform, scale, ortho, etc
#include <glm/gtc/type_ptr.hpp>             // Send GLM datatypes (matrix) to GPU

class _chunk; // Forward declaration for cell

/**
 * This is for serializing data for world saving. It stores cell/tile data of a chunk.
 */
struct cell_serial_data {         // Offset 
    uint8_t tileID;               // 1 byte
    uint8_t outlined;             // 2 bytes
    uint16_t padding;             // 4 bytes (Do not change)   
    float health;                 // 8 bytes
};

/**
 * This is for serializing data for world saving. It stores chunk data and contains the cell data (see below).
 */
struct chunk_serial_data {
    int32_t chunkX;
    int32_t chunkY;
    cell_serial_data cell_data[256];
};

/**
 * Enum mapped to TileId number as an unsigned 8 bit int. 
 * Naming goes: TILE_[TYPE]_[SUBTYPE]_[VARIANT]
 */
enum TileId : uint8_t {
    // Floor //
    TILE_FLOOR_BLANK_1,
    TILE_FLOOR_CRACKED_1,
    TILE_FLOOR_CRACKED_2,
    TILE_FLOOR_SQUARE_1,
    TILE_FLOOR_SQUARE_2,
    TILE_FLOOR_BLANK_2,

    // Wall //
    TILE_WALL_CENTER,
    TILE_WALL_LEFT,
    TILE_WALL_RIGHT,
    TILE_WALL_UP,
    TILE_WALL_DOWN,
    TILE_WALL_CORNER_TOPLEFT,
    TILE_WALL_CORNER_TOPRIGHT,
    TILE_WALL_CORNER_BOTTOMLEFT,
    TILE_WALL_CORNER_BOTTOMRIGHT,
    TILE_WALL_ISLAND,
    TILE_WALL_PENINSULA_TOP,
    TILE_WALL_PENINSULA_DOWN,
    TILE_WALL_PENINSULA_LEFT,
    TILE_WALL_PENINSULA_RIGHT,
    TILE_WALL_COLUMN_UP,
    TILE_WALL_COLUMN_SIDE,

    // Special //
    TILE_NULL, // Special undefined tile
};

/**
 * A tileCell holds data individual to each tile instance. This allows for each tile to have a dynamic and local state. 
 * This is called shorthand a "cell"
 */
class _cell
{
    public:
        TileId tileId = TILE_NULL; // Defaults to undefined tile
        uint8_t index = 0; // Index cell lives in chunk data array

        Vec2f pos; // Position of the cell 
        _chunk* parentChunk = nullptr; // Who owns this cell

        /**
         * Sets cell's outline state and update's its owning chunk to redraw. 
         * 
         * @param state True to outline / False to remove outline
         * 
         * @return True if success
         */
        bool setOutline(bool state);

        // Returns true if the cell is outlined
        bool isOutlined() const;

        /**
         * Impulses an amount of HP onto the cell (positive for health / negative for damage)
         * Manages cell's alive state from impulse
         * 
         * @param amount Health to add (negative for damage)
         * 
         * @return True if impulse kills cell (always false for healing)
         */
        bool impluseHealth(float amount);
        
        // Set cell to an HP number (bounds to 0 if negative)
        void setHealth(float amount);

        // Get cell HP
        float getHealth() const;

        // Returns true of cell is alive
        bool isAlive() const;
    protected:
    private:
        bool outlined = false;
        float health = 100.0f;
};

/**
 * A tile holds consistent information. Chunks hold IDs that reference a tile in the tile table and not instances themselves.
 * This saves memory as many things like text coords and the name are static and will never change. 
 * 
 * This is called shorthand a "tile"
 * 
 * 
 * For per tile data we use tileCell
 */
struct _tile
{
    string name = "";
    bool hasCollision = true;
    float u0, v0, u1, v1; 
};

/**
 * For chunk generation the coordinates start at the bottom left of the chunk.
 * 
 * Thus chunk (0,0) is (0,0) coordinates at the bottom left but (256,256) at the top right.
 * 
 * Chunks are ordered by their own coordinate system of (X,Y) so the Center Chunk is (0,0)
 *  - (-1,0) is LEFT of Center Chunk
 *  - (1,0) is RIGHT of Center Chunk
 *  - (0,1) is ABOVE of Center Chunk
 *  - (0,-1) is BELOW of Center Chunk
 * 
 *  Tiles are held as IDs to a look up tables and not objects. They go bottom-left -> top-right
 */
class _chunk
{
    public:
        _chunk();
        virtual ~_chunk();

        int chunkX;
        int chunkY;

        // This should really be private info 
        GLuint tileVboID = 0;       // ID for the GPU vertex memory of tiles
        GLuint tileEboID = 0;       // ID for the GPU index memory of the tiles
        GLuint tileVaoID = 0;       // ID for the GPU array memory of the tiles
        bool vboDirty = true;       // If dirty then we update the chunk (when tiles change)

        /**
         * Gets a tile at the given index
         * 
         * @param index Index in tile data array
         * 
         * @return Copy of tile id (unsigned 8 bit int)
         */
        TileId getTileIdAt(int index) const;

        /**
         * Gets a cell at the given index
         * 
         * @param index Index in cell data array (bounds checking)
         * 
         * @return Pointer to the cell (nullptr if not found)
         */
        _cell* cellAt(int index);

        /**
         * Sets a tile at the given index with bounds checking
         * 
         * @param id TileId to set
         * 
         * @param index Index to set tile id at
         * 
         * @return True if operation succeeded 
         */
        bool setTileIdAt(TileId id, int index);

        // Returns all 256 cells stored in the chunk as an array (readonly)
        const _cell* getAllCells() const;

        // Returns all 256 tiles stored in the chunk as an array (readonly)
        const TileId* getAllTileIds() const;

        // Sets all 256 cells to the array passed in
        void setAllCells(const _cell* cells);

        // Sets all 256 tiles to the array passed in
        void setAllTiles(const TileId* tiles);

        chunk_serial_data serializeChunk() const;

        void loadSerializedChunk(const chunk_serial_data &chunk_data);
    protected:
    private:
        TileId tileData[256];  // 16x16 chunk
        _cell cellData[256];   // 16x16 chunk
};

class _world
{
    public:
        _world();
        virtual ~_world();
        
        /**
         * Initializes the world by loading tile textures and generating the initial chunks.
         * 
         * @param loadWorld If true world is loaded and NOT generated
         */
        void initWorld(bool loadWorld, _lightManager* lightManager);

        // Draw function for world
        void drawWorld(float left, float right, float top, float bottom);

        /**
         * Update loop for the world
         * 
         * @param dt Time (in seconds) for update loop
         */
        void updateWorld(double dt);
        
        // Checks if a given chunck is loaded by looking up the chunk in the unordered map of loaded chunks.
        bool isChunkLoaded(int chunkX, int chunkY);
        
        // Prints debugging information largely about storage and performance metrics of the world
        void debugPrint();
        
        /**
         * Converts a world position into a chunk position
         * 
         * @param pos Position (in world coordinates) to convert
         * 
         * @return Vec2i of chunk position (x,y)
         */
        Vec2i worldToChunkPos(const Vec2f &pos) const;

        /**
         * Retrieves a chunk from a given chunk position
         * 
         * @param chunkPos Position (in chunk coordinates)
         * 
         * @return Pointer to the chunk or nullptr if not found
         */
        _chunk* getChunkAt(const Vec2i &chunkPos) const;
        
        /**
         * Retrieves a chunk from a given world position
         * 
         * @param chunkPos Position (in world coordinates)
         * 
         * @return Pointer to the chunk or nullptr if not found
         */
        _chunk* getChunkAtWorld(const Vec2f &pos) const;

        /**
         * Returns a chunk's tile from a provided tile index
         * 
         * @param chunk Chunk to get tile from
         * @param index The tile's index in the chunk
         * 
         * @return Readonly pointer to a tile or nullptr if not found
         */
        const _tile* getTileFromChunkIndex(const _chunk* chunk, const int index) const;

        /**
         * Returns a tile from a given world position
         * 
         * @param pos Position (in world coordinates)
         * 
         * @return Readonly pointer to a tile or nullptr if out of range
         */
        const _tile* getTileAtWorld(const Vec2f &pos) const;


        /**
         * Returns a cell from a given world position
         * 
         * @param pos Position (in world coordinates)
         * 
         * @return Pointer to a cell or nullptr if out of range
         */
        _cell* getCellAtWorld(const Vec2f &pos) const;


        /**
         * Changes a cell to a different tile id. 
         * Handles directional tile changes for walls and redraw.
         * 
         * @param cell The cell that is attached to the tile 
         * 
         * @param id Tile ID to set
         * 
         * @return True if operation successfull
         */
        bool setCellTile(_cell* cell, TileId id);

        // Checks if a tile is a wall based on if its in the WALL group (be careful changing WALL tiles, maintain first/last)
        bool isTileWall(TileId tileId) const;

        /**
         * Checks if a given cell is of type Wall
         * 
         * @param cell Pointer to cell to check
         * 
         * @return True if Cell is of type Wall (false if nullptr)
         */
        bool isCellWall(const _cell* cell) const;
        
        /**
         * Applys an amount of damage onto a given cell.
         * Automatically handles cell death tile change and particle effect
         * 
         * @param cell Pointer to cell to damage
         * 
         * @param amount Damage to apply (negative for healing)
         * 
         * @return True if operation successfull
         */
        bool damageCell(_cell* cell, float amount);

        // -- World Saving -- //
        
        // Returns a vector of serialized chunk data for export
        vector<chunk_serial_data> exportSerializeWorld() const;

        // Reads a vector of serialized chunk data for import
        void importSerializeWorld(vector<chunk_serial_data> world_data);

        // Sets the seed for the world generation
        void setSeed(uint32_t _seed);

        // Sets the view projection matrix
        static void setViewProjectionMatrix(const glm::mat4 &_viewProjectionMatrix);

        // Sets the camera position
        static void setCameraPosition(const Vec2f &_cameraPosition);

        bool DEBUG_displayChunkBorders = false; // When enabled puts a red border around chunks
    protected:
    private:
        bool worldInitialized = false;

        // -- PARTICLE MANAGER -- //
        _particleManager* cellParticles = new _particleManager();
        particle_effect wall_break_effect;
        particle_effect wall_damage_effect;

        // -- RNG -- //
        uint32_t seed; 
        mt19937 rng; 

        // -- WORLD DATA -- //

        _texture* tileAtlas = new _texture(); // Texture loader

        /**
         * This is the default number of renered chunks for the world. Starts at center and expands evenly outward as a cube 
         * Would be better done as a sphere but cube is easier -- gives warning if # doesnt make an even cube
         */
        const int numStartingChunks = NUM_CHUNKS;
        const float worldBounds = sqrt(numStartingChunks) * 16 * 16 * 0.5;

        // List of all possible tile types. This is a lookup table NOT the main storage array
        _tile world_tiles[256];  
        
        // Called by initWorld as a section to load tile textures
        void initTiles();

        // Uses the tileNum to calculated the tex coords for each tile
        bool setTileInAtlas(int xIndex, int yIndex, _tile &tile);

        // Builds a VBO for each chunk of all 256 tiles
        void buildChunkVBO(_chunk* chunk);

        /*
            Main storage of world data. The world is made up of chunks (16x16 tiles) thus holding 256 tiles each.
            Each tile is represented by a single byte holding an ID that uses the world_tiles lookup to establish a texture and parameters for that tile.
        */
        vector<_chunk> worldChunks; 

        // Constant lookup time for a given chunk and is loaded (vector still used for iterating)
        unordered_map<pair<int,int>, _chunk*, PairHash> chunkLookup;
        unordered_map<pair<int,int>, bool, PairHash> loadedChunks;

        // -- World Generation -- //
        vector<uint8_t> world_noise;

        float noise_distribution = 0.60;  // 0-1 value for % of world that is walls as initial noise
        float generation_iterations = 7; // Number of iterations to run the algorithm

        // Converts an index into a coordinate position starting at TOP LEFT for (0,0) using grid
        Vec2i convertIndexToPos(int index, int width, int height);

        // Puts the world through post processing converting bools to tile IDs for texturing
        void postProcessWorld();

        // Determines the tile type for wall connections
        TileId determineTileType(const bool neighborTiles[9]) const;

        // Finalizes the world generation binding the vector -> unordered map for rendering
        void finalizeWorld();

        // runs through all iterations of the world generation
        void runWorldGeneration(int iterations);

        /**
         * Maps a 3x3 grid of neighbor cells around the given cell
         * 
         * 8 7 6
         * 5 4 3
         * 2 1 0
         * 
         * Where 4 is the host cell
         */
        void mapCellNeighbors(_cell* cell, _cell* outNeighbors[9]);

        // -- SHADERS -- //
        _lightManager* sceneLightManager = nullptr; // Pointer to scene lightManager (non-owning)

        static glm::mat4 viewProjectionMatrix;
        _shader shader;
        GLint u_viewProjectionMatrix = -1;
        GLint u_texture = -1;
        GLint u_cameraPos = -1;
        GLint u_time = -1;

        float time = 0.0f;
        static Vec2f cameraPosition; 

        // -- DEBUGGING -- //
        
        _benchmark* initBenchmark = new _benchmark();

};

#endif // _WORLD_H