/**
 *  _world.h
 * 
 *  Holds helpers/containers and classes needed for rendering a world of tiles. World is broken
 *  up into chunks of (W x H) tiles. 
 * 
 */

#ifndef _WORLD_H
#define _WORLD_H

#define TILE_W 16.0f   // Tile width in world units is ALWAYS the same of 16
#define TILE_H 16.0f   // Tile height in world units is ALWAYS the same of 16

#include<_common.h>
#include<_texture.h>
#include<_benchmark.h>

struct _tile
{
    float u0, v0, u1, v1; // if using atlas; otherwise 0..1
};


/*
* For chunk generation the coordinates start at the bottom left of the chunk.
*
* Thus chunk (0,0) is (0,0) coordinates at the bottom left but (256,256) at the top right.
*
* Chunks are ordered by their own coordinate system of (X,Y) so the Center Chunk is (0,0)
*   (-1,0) is LEFT of Center Chunk
*   (1,0) is RIGHT of Center Chunk
*   (0,1) is ABOVE of Center Chunk
*   (0,-1) is BELOW of Center Chunk
*
*/
struct _chunk
{
    uint8_t tileData[256]; // 16x16 chunk
    int chunkX;
    int chunkY;
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

        // Sets the debug option to display chunk borders
        void SET_DisplayChunkBorders(bool value) { displayChunkBorders = value; }

        // Generates a chunk at the given chunk coordinates (chunkX, chunkY) and adds it to the worldChunks vector.
        void generateChunk(int chunkX, int chunkY);

        // Checks if a given chunck is loaded by looking up the chunk in the unordered map of loaded chunks.
        bool isChunkLoaded(int chunkX, int chunkY);

        // Prints debugging information largely about storage and performance metrics of the world
        void debugPrint();

    protected:
    private:
        // -- RNG -- //

        // Using a current time for the seed is chose because the normal std::random_device doesnt work for some reason
        unsigned int seed = (unsigned int)std::chrono::system_clock::now().time_since_epoch().count(); 
        std::mt19937 rng{seed}; 

        // -- WORLD DATA -- //

        // This is the default number of renered chunks for the world. Starts at center and expands evenly outward as a cube 
        // Would be better done as a sphere but cube is easier -- gives warning if # doesnt make an even cube
        int numStartingChunks = 256;

        _texture* tileAtlas = new _texture(); // Texture loader

        // List of all possible tile types. This is a lookup table NOT the main storage array
        _tile world_tiles[256];  
        
        // Called by initWorld as a section to load tile textures
        void initTiles();

        // Uses the tileNum to calculated the tex coords for each tile
        bool setTileInAtlas(int tileNum, _tile &tile);

        /*
            Main storage of world data. The world is made up of chunks (16x16 tiles) thus holding 256 tiles each.
            Each tile is represented by a single byte holding an ID that uses the world_tiles lookup to establish a texture and parameters for that tile.
        */
        vector<_chunk> worldChunks; 

        // Unordered map to keep track of which chunks are loaded. The key is a string of the format "chunkX,chunkY" and the value is a boolean indicating whether the chunk is loaded or not.
        // This key should be changed at some point, using a string conversion is expensive.
        unordered_map<string,bool> loadedChunks;

        // -- DEBUGGING -- //
        
        _benchmark* initBenchmark = new _benchmark();

        bool displayChunkBorders = false; // When enabled puts a red border around chunks (may drop performance)
};

#endif // _WORLD_H