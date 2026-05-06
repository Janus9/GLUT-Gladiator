#ifndef _FOB_H
#define _FOB_H

#include <_common.h>
#include <_unit.h>
#include <_player.h>
#include <_particleManager.h>
#include <_lightManager.h>
#include <_sounds.h>

class _fob : public _unit {
    public:
        _fob();
        virtual ~_fob();

        // Initializes the given FOB
        void initFob(_player* currentPlayer, _lightManager* currentLightManager, _sounds* currentSoundManager);

        /** Update loop */
        void updateFob(double dt);

        /** Tells the fob to check the player class for changes such as lives lost or death  */
        void evaluatePlayer();

        /** Draw function */
        void drawFob();
    protected:
    private:
        const float resupplyTime = 3.0; // Time for resupply event
        float resupplyDt = 0.0;

        _player* player = nullptr;                  // Pointer to existing player instance (non-owning)
        _lightManager* sceneLightManager = nullptr; // Pointer to existing light manager instance (non-owning)
        _sounds* sceneSoundManager = nullptr;            // Pointer to existing sound maanger instance (non-owning)

        unique_ptr<_particleManager> particleManager = make_unique<_particleManager>();
        particle_effect death_1_effect;
        particle_effect death_2_effect;
        particle_effect death_3_effect;
        particle_effect death_4_effect;
};

#endif // _FOB_H