#include <assert.h>
#include <math.h>
#include "ai.h"


namespace dragonfighting {

AI::AI(Sprite *player, Sprite *enemy) :
    direction(0),
    player(player),
    enemy(enemy)
{
    memset(&ctrlevent, 0, sizeof(ctrlevent));
    ctrlevent.type = Ctrl_KEYNONE;
}

AI::~AI()
{
}

bool AI::positionXEqual(float x, float dst)
{
    return fabs(dst - x) < player->speed + 0.00001f;
}

bool AI::pollEvent(struct Ctrl_KeyEvent *event)
{
    assert(event != NULL);

    if (ctrlevent.type == Ctrl_KEYNONE) {
        return false;
    }
    event->type = ctrlevent.type;
    event->key = ctrlevent.key;

    ctrlevent.type = Ctrl_KEYNONE;

    return true;
}


void AI::update(Uint32 frameStamp)
{
    if (player->getState() != Character::WALK) {
        if (!positionXEqual(player->x, 475)) {
            if (player->getStateAllow() & Character::ALLOW_WALK) {
                if (player->x > 475 && !positionXEqual(player->x, 475)) {
                    ctrlevent.type = Ctrl_KEYDOWN;
                    ctrlevent.key = CTRLKEY_LEFT;
                    direction = 1;
                } else if (player->x < 475 && !positionXEqual(player->x, 475)) {
                    ctrlevent.type = Ctrl_KEYDOWN;
                    ctrlevent.key = CTRLKEY_RIGHT;
                    direction = 2;
                }
            }
        }
    } else {
        if (positionXEqual(player->x, 475)) {
            ctrlevent.type = Ctrl_KEYUP;
            if (direction == 1) {
                ctrlevent.key = CTRLKEY_LEFT;
            } else if (direction == 2) {
                ctrlevent.key = CTRLKEY_RIGHT;
            }
            direction = 0;
        }
    }
}


}

