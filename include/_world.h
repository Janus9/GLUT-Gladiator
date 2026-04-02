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

#include<_common.h>
#include<_texture.h>
#include<_benchmark.h>

/**
 * 
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
struct _chunk
{
    int chunkX;
    int chunkY;
    uint8_t tileData[256]; // 16x16 chunk
    GLuint tileVboID = 0;  // ID for the GPU memory of tiles
    GLuint lineVboID = 0;  // ID for the GPU memory of lines
    bool vboDirty = true;  // If dirty then we update the chunk (when tiles change)
};

class _world
{
    public:
        _world();
        virtual ~_world();
        
        // Initializes the world by loading tile textures and generating the initial chunks.
        void initWorld();

        // Draw function for world
        void drawWorld(float left, float right, float top, float bottom);
        
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
        Vec2i worldToChunkPos(const Vec2f &pos);

        /**
         * Retrieves a chunk from a given chunk position
         * 
         * @param chunkPos Position (in chunk coordinates)
         * 
         * @return Pointer to the chunk or nullptr if not found
         */
        _chunk* getChunkAt(const Vec2i &chunkPos);
        
        /**
         * Retrieves a chunk from a given world position
         * 
         * @param chunkPos Position (in world coordinates)
         * 
         * @return Pointer to the chunk or nullptr if not found
         */
        _chunk* getChunkAtWorld(const Vec2f &pos);

        /**
         * Returns a chunk's tile from a provided tile index
         * 
         * @param chunk Chunk to get tile from
         * @param index The tile's index in the chunk
         * 
         * @return Readonly pointer to a tile or nullptr if not found
         */
        const _tile* getTileFromChunkIndex(const _chunk* chunk, const uint8_t index) const;

        /**
         * Returns a tile from a given world position
         * 
         * @param pos Position (in world coordinates)
         * 
         * @return Readonly pointer to a tile or nullptr if out of range
         */
        const _tile* getTileAtWorld(const Vec2f &pos);

        bool DEBUG_displayChunkBorders = true; // When enabled puts a red border around chunks
    protected:
    private:
        /**
         * Enum mapped to TileId number as an unsigned 8 bit int. 
         * Naming goes: TILE_[TYPE]_[SUBTYPE]_[VARIANT]
         */
        const enum TileId : uint8_t {
            TILE_FLOOR_BLANK_1 = 0,
            TILE_FLOOR_CRACKED_1 = 1,
            TILE_FLOOR_CRACKED_2 = 2,
            TILE_FLOOR_SQUARE = 3,
            TILE_FLOOR_BLANK_2 = 4,
            TILE_WALL_CENTER = 5,
            TILE_WALL_LEFT = 6,
            TILE_WALL_RIGHT = 7,
            TILE_WALL_UP = 8,
            TILE_WALL_DOWN = 9,
            TILE_WALL_CORNER_TOPLEFT = 10,
            TILE_WALL_CORNER_TOPRIGHT = 11,
            TILE_WALL_CORNER_BOTTOMLEFT = 12,
            TILE_WALL_CORNER_BOTTOMRIGHT = 13,
            TILE_WALL_ISLAND = 14,
            TILE_WALL_PENINSULA_TOP = 15,
            TILE_WALL_PENINSULA_DOWN = 16,
            TILE_WALL_PENINSULA_LEFT = 17,
            TILE_WALL_PENINSULA_RIGHT = 18,
            TILE_WALL_COLUMN_UP = 19,
            TILE_WALL_COLUMN_SIDE = 20,
        };

        // -- RNG -- //

        // Using a current time for the seed is chose because the normal std::random_device doesnt work for some reason
        const int seed = (unsigned int)std::chrono::system_clock::now().time_since_epoch().count(); 
        std::mt19937 rng{seed}; 

        // -- WORLD DATA -- //

        _texture* tileAtlas = new _texture(); // Texture loader

        /**
         * This is the default number of renered chunks for the world. Starts at center and expands evenly outward as a cube 
         * Would be better done as a sphere but cube is easier -- gives warning if # doesnt make an even cube
         */
        const int numStartingChunks = 16384;


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

        // Finalizes the world generation binding the vector -> unordered map for rendering
        void finalizeWorld();

        // runs through all iterations of the world generation
        void runWorldGeneration(int iterations);

        // -- DEBUGGING -- //
        
        _benchmark* initBenchmark = new _benchmark();

};

#endif // _WORLD_H