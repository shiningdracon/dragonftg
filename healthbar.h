#ifndef _HEALTHBAR_H_
#define _HEALTHBAR_H_

#include "widget.h"

namespace dragonfighting {

class HealthBar : public Widget
{
    protected:
        bool leftToRight;
        SDL_Rect geometry;
        SDL_Rect geometry2;
        int max;
        int current;

    public:
        HealthBar();
        virtual ~HealthBar();
        void setLeftToRight(bool leftToRight);
        void setGeometry(int x, int y, int w, int h);
        void setMax(int max);
        void setCurrent(int current);
        virtual void draw(SDL_Surface *dst);
        virtual void update(Uint32 frameStamp);
};


}

#endif
