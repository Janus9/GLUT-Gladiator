This document details how a world save file is structured and saved.

File: ".gg_world"

       -- META DATA --  
| Offset | Size | Data Type      | Name    
|--------|------|----------------|----------- 
| 0      | 2    | char[2]        | Header ("GG")
| 2      | 4    | uint32_t       | Seed 
| 6      | 8    | uint64_t       | Time Stamp
| 14     | 4    | uint32_t       | Save System Version ID
| 18     | 4    | float          | Game Version 
| 22     | 4    | int32_t        | Chunk Count
| 26     | 4    | char[4]        | Chunk Data Header ("WRLD")
|--------|------|----------------|----------- (This is repeated for chunk count amount) (26 offset)
|      -- CHUNK DATA (28 byte offset) --          
| 0      | 4    | int32_t        | Chunk Position X
| 4      | 4    | int32_t        | Chunk Position Y 
| 8      | 2048 | cell_data[256] | Chunk Cell/Tile Data (See Below)
|--------|------|----------------|----------- 
|      -- ENEMY DATA (XXX byte offset) --          
| 0      | 4    | char[4]        | Enemy Data Header ("ENMY")
| 0      | 4    | uint32_t       | Number of enemies
| 0      |      | enemy_data[x]  | Enemy Data (Where "x" is number of enemies) (See Below)
|--------|------|----------------|-----------  
|       -- CELL DATA --          |
| 0      | 1    | uint8_t        | Tile ID
| 1      | 1    | uint8_t        | Outlined (Boolean)
| 2      | 2    | uint16_t       | Padding
| 4      | 4    | float          | Health
|--------|------|----------------|-----------  
|      -- ENEMY DATA --          |
| 0      | 1    | uint8_t        | Enemy Type (enum)
| 1      | 1    | uint8_t        | Enemy Team (enum)
| 2      | 4    | float          | Current Health
| 6      | 4    | float          | Max Health
| 10     | 4    | float          | Position X
| 12     | 4    | float          | Position Y
| 14     | 4    | float          | Position Z
| 18     | 2    | uint16_t       | Padding
|--------|------|----------------|----------- (End of File) 
