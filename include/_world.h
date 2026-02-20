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

struct _tile
{
    GLuint textureId;
    //float u0, v0, u1, v1; // if using atlas; otherwise 0..1
    int pixelW;
    int pixelH;
    //bool isSolid;  
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
        
        void initWorld();
        void drawWorld();

        void SET_DisplayChunkBorders(bool value) { displayChunkBorders = value; }

        void generateChunk();

        bool isChunkLoaded(int chunkX, int chunkY);

    protected:
    private:
        // Using a current time for the seed is chose because the normal std::random_device doesnt work for some reason
        unsigned int seed = (unsigned int)std::chrono::system_clock::now().time_since_epoch().count(); 
        std::mt19937 rng{seed}; 

        _texture* tileTexture = new _texture(); // Texture loader

        _tile world_tiles[256]; // 256 unique tile types 
        //_chunk* chunk = new _chunk(); // example chunk

        vector<_chunk> worldChunks; 

        unordered_map<string,bool> loadedChunks;

        // DEBUGGING //
        bool displayChunkBorders = true; // When enabled puts a red border around chunks (may drop performance)
};

#endif // _WORLD_H