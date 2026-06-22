#ifndef _PICKUP_MANAGER_H
#define _PICKUP_MANAGER_H

#include <_common.h>
#include <_player.h>
#include <_lightManager.h>
#include <_texture.h>
#include <_shader.h>

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
    float value;    
    int32_t type;
    float xPos;
    float yPos;               
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
         */
        void initPickupManager(const std::string& fileName, int imageWidth, _player* currentPlayer, _lightManager* currentLightManager);
        
        /** Draw function */
        void drawPickups();

        /** Update function */
        void updatePickups(const double dt);

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
         * Returns a vector of serialized pickup data for saving
         */
        std::vector<pickup_serial_data> exportSerializedPickups() const;

        /**
         * Imports a vector of serialized pickup data for loading
         * @param pickup_data Vector of data
         * @return True if operation was successfull
         */
        bool importSerializedPickups(const std::vector<pickup_serial_data> &pickup_data);

        static void setViewProjectionMatrix(const glm::mat4& _viewProjectionMatrix); 
    protected:
    private:
        const int maxPickups = 5000;    // Maxmimum pickups world allows
        int alivePickups;
        int numImages;

        bool initialized = false;       // Blocks operations until initialized

        struct _pickup {
            Vec2f pos;   
            Vec2f vel;           
            Vec2f acc;
            
            pickup_type type;
            
            float size;             
            float value;

            bool alive;
        };

        std::vector<_pickup> pickupList;

        pickup_serial_data serializePickup(const _pickup &pickup) const;

        _player* player = nullptr;  // Pointer to player instance in scene (non-owning) 

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

        _lightManager* sceneLightManager;   // Pointer to light manager in scene (non-owning)

        // Rng machine
        std::mt19937 rng;   
};

#endif // _PICKUP_MANAGER_H