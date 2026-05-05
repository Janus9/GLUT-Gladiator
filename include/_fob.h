#ifndef _FOB_H
#define _FOB_H

#include <_common.h>
#include <_unit.h>
#include <_player.h>
#include <_particleManager.h>
#include <_lightManager.h>

class _fob : public _unit {
    public:
        _fob();
        virtual ~_fob();

        // Initializes the given FOB
        void initFob(_player* currentPlayer, _lightManager* currentLightManager);

        /** Update loop */
        void updateFob(double dt);

        /** Draw function */
        void drawFob();
    protected:
    private:
        const float resupplyTime = 3.0; // Time for resupply event
        float resupplyDt = 0.0;

        _player* player = nullptr;                  // Pointer to existing player instance (non-owning)
        _lightManager* sceneLightManager = nullptr; // Pointer to existing light manager instance (non-owning)

        unique_ptr<_particleManager> particleManager = make_unique<_particleManager>();
};

#endif // _FOB_H