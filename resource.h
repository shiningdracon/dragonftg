#ifndef _RESOURCE_
#define _RESOURCE_

#include "sprite.h"

namespace dragonfighting {

class SpriteFactory
{
public:
    static Sprite *loadSprite(const char *basedir, const char *filename);
    static void freeSprite(Sprite *sprite);

private:
    static void loadSpriteAnimation(Sprite *sprite, const char *basedir, const char *filename);
    static void loadSpriteCollision(Sprite *sprite, const char *basedir, const char *filename);
};

}

#endif
