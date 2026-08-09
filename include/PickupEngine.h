#ifndef PICKUP_ENGINE_H
#define PICKUP_ENGINE_H

#include <_common.h>
#include <_player.h>
#include <_lightManager.h>
#include <_texture.h>
#include <_shader.h>
/** A serialization class should be made at some point */
#include <_world.h>         // For the world generation config + serial data types

// https://docs.google.com/spreadsheets/d/125jQdeutCYkeY5MRzY0q0YjmVbVyGVxeAAeK7RvzJqU/edit?usp=sharing // Drive Link

struct enemy_serial_data;

namespace pickups {
    /**
     * These must be in order with image
     */
    enum type : int32_t {
        PICKUP_HEALTH,
        PICKUP_AMMO,
        PICKUP_SPEED,
        PICKUP_MAX_HEALTH,
        PICKUP_XP,
        PICKUP_FIRERATE
    };

    struct serial_data {
        uint32_t id;
        float value;    
        int32_t type;
        float xPos;
        float yPos;   
        uint32_t alive;            
    };

    class Engine {
        public:
            Engine();
            virtual ~Engine();

            /**
             * Initilization function
             * 
             * @param fileName Image file source 
             * @param imageWidth Number of images in the horizontal image strip
             * @param currentPlayer Player owned by scene to apply pickup attributes
             * @param currentLightManager Light manager owned by scene to apply lighting effects
             * @param currentWorld World owned by scene to check collision with
             */
            void init(
                const std::string& fileName, 
                int imageWidth, 
                _player* currentPlayer, 
                _lightManager* currentLightManager,
                _world* currentWorld
            );
            
            /** Draw function */
            void draw();

            /** Update function */
            void update(const double dt);

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
            bool add(const Vec2f &pos, pickups::type type, float value);

            /**
             * Generates pickups from the configuration file into the game's save file.
             * 
             * @param config Configuration file of the world generation
             */
            bool generateToFile(const world_config &config);

            /** 
             * Reads the current world file and writes the contents into the memory
             * 
             * Runs asynchronously on a worker thread.
             */
            void readFromFileAsync();

            /**
             * Writes the contents of the mutation map into disk.
             * 
             * Runs asynchronously on a worker thread.
             */
            void writeToFileAsync();

            /**
             * Removes dead pickups from the save file.
             * 
             * Runs asynchronously on a worker thread.
             */
            void cleanDeadFromFileAsync();

            /**
             * Checks if any async tasks are actively runnning.
             * 
             * @return True if all async functions have completed.
             */
            bool areAsyncTasksCompleted() const;

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
            static constexpr float PICKUP_SAVE_INTERVAL = 30.0f; // Delay in seconds until pickups from memory are saved to disk
            static constexpr float DISK_CLEAN_INTERVAL = 60.0f;  // Delay in seconds until dead pickups in disk are cleaned

            // PHYSICS VARIABLES //
            static constexpr float PHYS_AIR_RESISTANCE = 0.75f;        // % Vel lost per second
            static constexpr float PHYS_VEL_ZERO = 0.01f;              // Velocity magnitude lower than this results in zero velocity
            static constexpr float PHYS_PICKUP_DISTANCE = 48.0f;       // Distance for pickups to begin movement towards player
            static constexpr float PHYS_ATTRACT_SPEED = 35.0f;         // Overall multiplier for pickup attraction speed
            static constexpr float PHYS_MIN_SPEED = 15.0f;             // Minimum speed pickup can be (forces instant velocity)
            static constexpr float PHYS_MAX_SPEED = 120.0f;             // Maximum speed pickup can achieve

            float pickupSaveElapsedTime = 0.0f;
            float cleanDiskElapsedTime = 0.0f;
            
            int alivePickups;
            int numImages;

            bool initialized = false;       // Blocks operations until initialized

            struct _pickup {
                unsigned int id;

                Vec2f pos;   
                Vec2f vel;           
                Vec2f acc;
                
                pickups::type type;
                
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
            // Add Pickups From Disk Into Memory //
            std::atomic<bool> writeBufferCompleted;
            std::atomic<bool> writeBufferInProgress;
            std::thread writeBufferThread;      // Thread for Write Buffer (Memory)
            // Add Pickups To Disk //
            std::atomic<bool> writeDiskCompleted;
            std::atomic<bool> writeDiskInProgress;
            std::thread writeDiskThread;        // Thread for Disk Write
            // Remove Dead Pickups From Disk //
            std::atomic<bool> cleanDiskCompleted;   
            std::atomic<bool> cleanDiskInProgress;
            std::thread cleanDiskThread;        // Thread for Disk Clean

            std::atomic<uint32_t> nextID;

            Vec2f prevWritePos = {0.0f, 0.0f};
            
            _world* world = nullptr;  // Pointer to world instance in scene (non-owning) 
            _player* player = nullptr;  // Pointer to player instance in scene (non-owning) 

            // Async Worker Functions //        
            void readFromFileWorker();      // Write pickups from disk into memory
            void saveToFileWorker();        // Empties the contents of mutationMap into disk
            void cleanDeadFromFileWorker(); // Removes dead pickups from disk

            bool generatePickup(std::fstream &file, const pickup_config &config, float numChunks, pickups::type type, int &ID); // Generates a given pickup

            // Serialization + Disk Access //

            serial_data serializePickup(const _pickup &pickup) const;
            
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
            _shader shader;

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
}
#endif // PICKUP_ENGINE_H