#ifndef _STAGE_H_
#define _STAGE_H_

#include "widget.h"
#include "sprite.h"
#include "collisiondetect.h"
#include "healthbar.h"

namespace dragonfighting {

class Stage : public Widget
{
    public:
        Stage(Sprite *p1, Sprite *p2);
        ~Stage();

        void update(Uint32 frameStamp);
        void draw(SDL_Surface *dst);

    private:
        Sprite *player1;
        Sprite *player2;
        float groundline;
        SDL_Surface *bkImage;
        SDL_Rect bkRect;

        int p1Health;
        int p2Health;
        HealthBar healthbarP1;
        HealthBar healthbarP2;
};


}

#endif
