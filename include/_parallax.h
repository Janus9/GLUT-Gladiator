#ifndef _PARALLAX_H
#define _PARALLAX_H

#include<_common.h>
#include<_texture.h>

class _parallax
{
    public:
        _parallax();
        virtual ~_parallax();

        enum {
            LEFT = 0,
            RIGHT = 1,
            UP = 2,
            DOWN = 3
        };

        float xMax, xMin, yMax, yMin; // boundaries for parallax scrolling

        void initParallax(const std::string &fileName);
        void scrollParallax(bool, int, float);
        void drawParallax(float, float);

    protected:

    private:
        _texture* parallaxLoader = new _texture();
};

#endif // _PARALLAX_H