#ifndef _PLAYER_H
#define _PLAYER_H

#include <_common.h>
#include <_unit.h>
#include <_particleManager.h>
#include <_lightManager.h>

struct player_serial_data {
    uint8_t team;

    float maxHP;
    float currentHP;

    float posX;
    float posY;

    float fireRate;
    float respawnTime;

    float spawnPosX;
    float spawnPosY;

    float movementSpeed;
    int32_t numDeaths;

    int32_t magCapacity;
    int32_t magLevel;
    int32_t reserveCapacity;
    int32_t reserveLevel;
    float reloadSpeed;

    uint8_t padding1;
    uint16_t padding2;
};

enum player_face {
    PLAYER_FACE_NULL,
    PLAYER_FACE_N,
    PLAYER_FACE_NE,
    PLAYER_FACE_E,
    PLAYER_FACE_SE,
    PLAYER_FACE_S,
    PLAYER_FACE_SW,
    PLAYER_FACE_W,
    PLAYER_FACE_NW,
    // KEEP AT BACK //
    PLAYER_FACE_COUNT 
};

enum player_action {
    PLAYER_ACTION_NULL,
    PLAYER_ACTION_WALK,
    PLAYER_ACTION_WALK_GUN,
    PLAYER_ACTION_WALK_SHOOT,
    PLAYER_ACTION_WALK_RELOAD,
    PLAYER_ACTION_IDLE,
    PLAYER_ACTION_IDLE_GUN,
    PLAYER_ACTION_IDLE_SHOOT,
    PLAYER_ACTION_IDLE_RELOAD,
    PLAYER_ACTION_DEATH_GUN,
    // KEEP AT BACK //
    PLAYER_ACTION_COUNT 
};

enum player_enterered_level_event {
    PLAYER_EVENT_LEVEL_OUTER,
    PLAYER_EVENT_LEVEL_MIDDLE,
    PLAYER_EVENT_LEVEL_CENTER,
    PLAYER_EVENT_LEVEL_BOSS,
    PLAYER_EVENT_LEVEL_NONE
};

class _player : public _unit {
    public:
        _player();
        virtual ~_player();

        /**
         * Sets up player sprites, animations, etc.
         */
        void initPlayer(_lightManager* lightManager);

        /**
         * Update loop for player including inputs/animations/movement/etc
         * 
         * @param dt Delta time (in seconds)
         */
        void updatePlayer(double dt);

        // Draw function for set action corresponding sprite/animation
        void drawPlayer();

        // Sets action for player (walk,run,shoot, etc)
        void setAction(player_action action, player_face face);

        // Stops current action
        void stopAction(player_action action);

        // Sets the animation FPS for the player's animations
        void setAnimationFPS(int _FPS);

        // Wrapper to handle player death animation and actions
        void handlePlayerDeath(player_face face);

        // Exports a serialized data package of the player for export
        player_serial_data exportSerializedPlayer() const;

        // Imports a serialized data package of the player for import
        void importSerializedPlayer(const player_serial_data &player_data);

        /** 
         * Resupplies the player with health/ammo.
         * 
         * Prevents overflow and spawns effect
         */
        void resupply(float health, int ammo);

        // Applies reload event to the player, safe to apply constantly as the state doesnt change
        void procReload();

        // Returns true if player is activley reloading
        bool isReloading() const;

        // Player Variables //
        int lives = 3;
        float respawnTime = 5.0f; // Seconds to respawn.
        float movementSpeed = 120.0f; // World units / second
        Vec2f spawnPos = {0.0f, 0.0f};
        
        // Weapon Variables //
        float fireRate;          // RPM
        int magCapacity;         // Maximum bullets in magazine
        int magLevel;            // Current bullets in magazine
        int reserveCapacity;     // Maximum bullets in reserve             
        int reserveLevel;        // Current bullets in reserve
        float reloadSpeed;       // How fast weapon reloads (in seconds)

        bool isMoving = false;      // Is player actively moving
        bool hasGun = false;        // Does player have gun equipped
        bool isShooting = false;    // Is the player actively shooting
        bool isRealDead = false;    // If true, game ends

        double deathTimeElapsed = 0.0;  // Time elapsed since death occured
        bool inDeathAnimation = false;  // Is player in the animation 
   
        // Procedure Events (Fire when true) //
        bool playerTookDamage = false;
        bool playerShootEvent = false;
        bool playerRespawnedEvent = false;

        player_enterered_level_event playerLevelEvent = PLAYER_EVENT_LEVEL_NONE;
    protected:
    private:
        Vec2f previousPos = {0.0f, 0.0f};
        _lightManager* sceneLightManager = nullptr; // Pointer to light manager instantiated in scne (non-owning)

        int FPS = 12;

        bool reloading = false;
        double reloadTimeElapsed = 0.0;

        double shootTimeElapsed = 0.0;

        struct PlayerAnimationResult {
            string sprite = ""; // Which sprite
            string action = ""; // Which action (direction facing)
            Vec2i idleFrame = {0,0};
            bool valid = true;  // Wether action is valid (not null)
            
            bool operator==(const PlayerAnimationResult &other) const {
                return sprite == other.sprite && action == other.action;
            }
        };
        
        PlayerAnimationResult animationTable[PLAYER_ACTION_COUNT][PLAYER_FACE_COUNT];
        
        PlayerAnimationResult currentResult;

        // Gets animation with error checking + NULL handling
        PlayerAnimationResult getAnimationResult(player_action action, player_face face);

        unique_ptr<_particleManager> particleManger = make_unique<_particleManager>();
        particle_effect player_hit_effect;
        particle_effect resupply_health_effect;
        particle_effect resupply_ammo_effect;

        unique_ptr<_sprite> navSprite = make_unique<_sprite>();
};

#endif // _PLAYER_H