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

    protected:
    private:
        _texture* tileTexture = new _texture(); // Texture loader

        _tile world_tiles[256]; // 256 unique tile types 
        //_chunk* chunk = new _chunk(); // example chunk

        _chunk worldChunks[9]; // 3x3 grid around the player
};

#endif // _WORLD_H