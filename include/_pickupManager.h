#ifndef _PICKUP_MANAGER_H
#define _PICKUP_MANAGER_H

#include <_common.h>
#include <_particleManager.h>
#include <_player.h>
#include <_lightManager.h>
#include <_texture.h>
#include <_shader.h>

struct pickup_config {
    int imageIndex = 0;     /// Which image to apply as the texture (0 is a missing texture)
    float size;             /// Size of the image (1.0 is 100% size, 2.0 is 200% etc)

    float health;           /// Gives the player HP 
    float maxHealth;        /// Increases the player's max HP
    float ammo;             /// Gives the player ammo
    float speed;            /// Increases the player's movement speed
    float fireRate;         /// Increases player's rate of weapon fire
    float xp;               /// XP to reward to player 
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
        void initPickupManager(const string& fileName, int imageWidth, _player* currentPlayer, _lightManager* currentLightManager);
        
        /** Draw function */
        void drawPickups();

        /** Update function */
        void updatePickups(const double dt);

        /**
         * Adds a pickup to the system
         * 
         * @param pos Position to spawn the pickup
         * @param config Pickup configuration class 
         * 
         * @return True if operation succeeded (False can be caused by full pickup manager)
         */
        bool addPickup(const Vec2f &pos, const pickup_config& config);

        static void setViewProjectionMatrix(const glm::mat4& _viewProjectionMatrix); 
    protected:
    private:
        const int maxPickups = 5000;    // Maxmimum pickups world allows
        int alivePickups;
        int numImages;

        bool initialized = false;       // Blocks operations until initialized

        struct _pickup {
            int imageIndex = 0;     
            Vec2f pos;   
            Vec2f vel;           
            Vec2f acc;
            float size;             

            float health;           
            float maxHealth;        
            float ammo;             
            float speed;            
            float fireRate;         
            float xp;

            bool alive;
        };

        vector<_pickup> pickupList;

        _player* player = nullptr;  // Pointer to player instance in scene (non-owning) 
        unique_ptr<_particleManager> particleManager = make_unique<_particleManager>();

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
        mt19937 rng;   
};

#endif // _PICKUP_MANAGER_H