This document details how a world save file is structured and saved.

File: ".gg_world"


       -- META DATA --  
| Offset | Size | Data Type      | Name    
|--------|------|----------------|----------- 
| 0      | 2    | char[2]        | Header ("GG")
| 2      | 4    | uint32_t       | Seed 
| 6      | 8    | uint64_t       | Time Stamp
| 14     | 4    | uint32_t       | Version ID (ex/ "v0.11" is 11 and "v1.12" is 112) 
| 18     | 4    | int32_t        | Chunk Count
| 22     | 4    | char[4]        | Chunk Data Header ("WRLD")
|--------|------|----------------|----------- (This is repeated for chunk count amount) (26 offset)
|      -- CHUNK DATA --          |
| 0      | 4    | int32_t        | Chunk Position X
| 4      | 4    | int32_t        | Chunk Position Y 
| 8      | 256  | uint8_t[256]   | Chunk Tile IDs
|--------|------|----------------|----------- (End of File) (26 + (264 * Chunk Count) offset)
|        | 3    | char[3]        | End Stamp ("END")