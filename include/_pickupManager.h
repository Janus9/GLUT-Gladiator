#ifndef _PICKUP_MANAGER_H
#define _PICKUP_MANAGER_H

#include <_common.h>
#include <_player.h>
#include <_lightManager.h>
#include <_texture.h>
#include <_shader.h>
/** A serialization class should be made at some point */
#include <_world.h>         // For the world generation config + serial data types

// https://docs.google.com/spreadsheets/d/125jQdeutCYkeY5MRzY0q0YjmVbVyGVxeAAeK7RvzJqU/edit?usp=sharing // Drive Link

struct enemy_serial_data;

/**
 * These must be in order with image
 */
enum pickup_type : int32_t {
    PICKUP_HEALTH,
    PICKUP_AMMO,
    PICKUP_SPEED,
    PICKUP_MAX_HEALTH,
    PICKUP_XP,
    PICKUP_FIRERATE
};

struct pickup_serial_data {
    uint32_t id;
    float value;    
    int32_t type;
    float xPos;
    float yPos;   
    uint32_t alive;            
};

class _pickupManager {
    public:
        _pickupManager();
        virtual ~_pickupManager();

        /**
         * Initilization function
         * 
         * @param fileName Image file source 
         * @param imageWidth Number of images in the horizontal image strip
         * @param currentPlayer Player owned by scene to apply pickup attributes
         * @param currentLightManager Light manager owned by scene to apply lighting effects
         * @param currentWorld World owned by scene to check collision with
         */
        void initPickupManager(
            const std::string& fileName, 
            int imageWidth, 
            _player* currentPlayer, 
            _lightManager* currentLightManager,
            _world* currentWorld
        );
        
        /** Draw function */
        void drawPickups();

        /** Update function */
        void updatePickups(const double dt);

        /** 
         * Background Update Function 
         * 
         * Always runs regardless of game state. Does not run at a fixed framerate.
        */
        void updateBackground();

        /**
         * Adds a pickup to the system. (Size is determined by value)
         * 
         * The image is automatically assigned through the type
         * 
         * @param pos Position to spawn the pickup
         * @param type Enum pickup type
         * @param value Value of the pickup 
         * 
         * @return True if operation succeeded (False can be caused by full pickup manager)
         */
        bool addPickup(const Vec2f &pos, pickup_type type, float value);

        /**
         * Generates pickups from the configuration file into the game's save file.
         * 
         * @param config Configuration file of the world generation
         */
        bool generateToFile(const world_config &config);

        /** 
         * Reads the current world file and writes the contents into the memory
         * @return True if opperation succeeded
         */
        bool readFromFile();

        /**
         * Writes the contents of the mutation map into disk
         * 
         * Clears out the mutation map
         * 
         * Opens in a new thread
         * @return True if operation succeeded
         */
        bool writeToFile();

        /** 
         * Creates a "pickup.log" file which contains a detailed printout of the save file pickup contents.
         * 
         * This function does nothing if no save file is found.
         * */
        void logDisk() const;

        static void setViewProjectionMatrix(const glm::mat4& _viewProjectionMatrix); 
        static void setCameraPosition(const Vec2f &_cameraPosition);
    protected:
    private:
        static constexpr int MAX_RENDER_PICKUPS = 65536;     // Maxmimum render pickups visible
        static constexpr float VIEW_RANGE = 1096.0f;
        static constexpr float PICKUP_SAVE_INTERVAL = 30.0f; // In seconds

        float pickupSaveElapsedTime = 0.0f;
        
        int alivePickups;
        int numImages;

        bool initialized = false;       // Blocks operations until initialized

        struct _pickup {
            unsigned int id;

            Vec2f pos;   
            Vec2f vel;           
            Vec2f acc;
            
            pickup_type type;
            
            float size;             
            float value;

            bool alive;
        };
        
        std::vector<_pickup> dataBuffer1;
        std::vector<_pickup> dataBuffer2;

        std::unordered_map<uint32_t, _pickup> mutationMap;
        
        std::vector<_pickup>* readBuffer = nullptr;
        std::vector<_pickup>* writeBuffer = nullptr;

        std::mutex m_mm; // Mutation Map Mutex
        std::atomic<bool> writeBufferCompleted;
        std::atomic<bool> writeBufferInProgress;
        std::atomic<bool> writeDiskCompleted;
        std::atomic<bool> writeDiskInProgress;
        std::atomic<uint32_t> nextID;

        std::thread writeBufferThread;      // Thread for Write Buffer (Memory)
        std::thread writeDiskThread;        // Thread for Disk

        Vec2f prevWritePos = {0.0f, 0.0f};
        
        _world* world = nullptr;  // Pointer to world instance in scene (non-owning) 
        _player* player = nullptr;  // Pointer to player instance in scene (non-owning) 
        
        void writeToBuffer();   // Write pickups from disk into memory
        void emptyMutationMap(); // Empties the contents of mutationMap into disk
        bool generatePickup(std::fstream &file, const pickup_config &config, float numChunks, pickup_type type, int &ID); // Generates a given pickup
        
        // Serialization + Disk Access //

        pickup_serial_data serializePickup(const _pickup &pickup) const;
        
        // Verifies existing save file is valid, sets the pickup count.
        bool verifyFile(std::fstream &file, uint32_t &pickup_count) const;

        // Rendering //
        void buildVBO();
        void buildEBO();
        void buildVAO();

        GLuint vboID = 0;
        GLuint eboID = 0;
        GLuint vaoID = 0;

        _texture textureLoader;
        _shader pickupShader;

        GLint u_viewProjectionMatrix = -1;
        GLint u_texture = -1;
        GLint u_t = -1;
        
        float t_value = 0.0f;
        static glm::mat4 viewProjectionMatrix;
        static Vec2f cameraPosition; 

        _lightManager* sceneLightManager;   // Pointer to light manager in scene (non-owning)

        // Rng machine
        std::mt19937 rng;   
};

#endif // _PICKUP_MANAGER_H