#ifndef _MODEL_H
#define _MODEL_H

#include "_common.h"

class _model {
    private:
    protected:
    public:
        _model();
        virtual ~_model();

        void initModel();
        void drawModel();

        Vec3f position;
        Vec3f rotation;
        Vec3f scale;
        Col3f color;
};

#endif // _MODEL_H