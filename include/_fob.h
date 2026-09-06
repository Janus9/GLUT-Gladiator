#ifndef _FOB_H
#define _FOB_H

#include <_common.h>
#include <_unit.h>
#include <_player.h>
#include <_lightManager.h>
#include <SoundEngine.h>
#include <ParticleEngine.h>

struct FobContext {
    _player &player;
    _lightManager &lights;
    sound::Engine &sounds;
    particles::Engine &particles;
};

class _fob : public _unit {
    public:
        _fob();
        virtual ~_fob();

        // Initializes the given FOB
        void initFob(const FobContext& context);

        /** Update loop */
        void updateFob(double dt);

        /** Tells the fob to check the player class for changes such as lives lost or death  */
        void evaluatePlayer();

        /** Draw function */
        void drawFob();
    protected:
    private:
        bool alive = true;
        const float resupplyTime = 3.0; // Time for resupply event
        float resupplyDt = 0.0;

        _player* player;                      // Pointer to existing player instance (non-owning)
        _lightManager* lightManager;          // Pointer to existing light manager instance (non-owning)
        sound::Engine* soundEngine;           // Pointer to existing sound manager instance (non-owning)
        particles::Engine* particleEngine;    // Pointer to existing particle engine instance (non-owning)
};

#endif // _FOB_H