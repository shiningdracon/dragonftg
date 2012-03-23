#ifndef _AI_H_
#define _AI_H_

#include "keystream.h"
#include "sprite.h"

namespace dragonfighting {

class AI
{
    private:
        struct Ctrl_KeyEvent ctrlevent;
        int direction;
        Sprite *player;
        Sprite *enemy;

        bool positionXEqual(float x, float dst);

    public:
        AI(Sprite *player, Sprite *enemy);
        ~AI();

        void update(Uint32 frameStamp);
        bool pollEvent(struct Ctrl_KeyEvent *event);
};

}

#endif
