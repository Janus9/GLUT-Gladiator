This document details how a world save file is structured and saved.

File: ".gg_world"


       -- META DATA --  
| Offset | Size | Data Type      | Name    
|--------|------|----------------|----------- 
| 0      | 2    | char[2]        | Header ("GG")
| 2      | 4    | uint32_t       | Seed 
| 6      | 8    | uint64_t       | Time Stamp
| 14     | 4    | uint32_t       | Version ID (not game version, whole numbers only) 
| 18     | 4    | int32_t        | Chunk Count
| 22     | 4    | char[4]        | Chunk Data Header ("WRLD")
|--------|------|----------------|----------- (This is repeated for chunk count amount) (26 offset)
|      -- CHUNK DATA --          |
| 0      | 4    | int32_t        | Chunk Position X
| 4      | 4    | int32_t        | Chunk Position Y 
| 8      | 1536 | cell_aata[256] | Chunk Cell/Tile Data (See Below)
|--------|------|----------------|----------- (Data for "Cell Data" above)
|       -- CELL DATA --          |
| 0      | 1    | uint8_t        | Tile ID
| 1      | 1    | uint8_t        | Outlined (Boolean)
| 2      | 4    | float          | Health
|--------|------|----------------|----------- (End of File) 