#ifndef _FOB_H
#define _FOB_H

#include <_common.h>
#include <_unit.h>
#include <_player.h>
#include <_particleManager.h>

class _fob : public _unit {
    public:
        _fob();
        virtual ~_fob();

        // Initializes the given FOB
        void initFob(_player* currentPlayer);
    protected:
    private:
        _player* player = nullptr; // Pointer to existing player instance (non-owning)
};

#endif // _FOB_H