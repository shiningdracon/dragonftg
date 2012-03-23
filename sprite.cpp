#include <assert.h>
#include "sprite.h"

namespace dragonfighting {

static const int MAX_COLLISION_RECT_ARRAY_LEN = 10;

Sprite::Sprite() :
    Character(),
    Animation(),
    speed(0),
    velocity_x(0),
    velocity_y(0),
    x(0),
    y(0),
    accx(0),
    accy(0.1f),
    oldstate(STAND),
    oldforward(-1),
    curAreaSequence(NULL),
    curAreaIndex(0),
    oldFrameStamp(0)
{
    realHitRectArray = (SDL_Rect *)malloc(sizeof(SDL_Rect) * MAX_COLLISION_RECT_ARRAY_LEN);
    realAttackRectArray = (SDL_Rect *)malloc(sizeof(SDL_Rect) * MAX_COLLISION_RECT_ARRAY_LEN);
}

Sprite::~Sprite()
{
    free(realHitRectArray);
    free(realAttackRectArray);
    for (vector<struct CollisionArea>::iterator i = collisionAreas.begin(); i != collisionAreas.end(); ++i ) {
        if ( (*i).hitRectArray) {
            free( (*i).hitRectArray );
        }
        if ( (*i).attackRectArray) {
            free( (*i).attackRectArray );
        }
    }

    for (vector<struct CollisionAreaSequence *>::iterator i = collisionAreaSequences.begin(); i != collisionAreaSequences.end(); i++) {
        if ( (*i)->indexArray) {
            free( (*i)->indexArray );
        }
        delete *i;
    }
}

void Sprite::setPosition(float x, float y)
{
    this->x = x;
    this->y = y;
    Animation::setPosition((int)x, (int)y);
}

void Sprite::setPositionX(float x)
{
    setPosition(x, this->y);
}

float Sprite::getPositionX()
{
    return x;
}

float Sprite::getPositionY()
{
    return y;
}

void Sprite::setSpeed(float speed)
{
    this->speed = speed;
}

void Sprite::resetPhysic()
{
    velocity_x = 0;
    velocity_y = 0;
    accx = 0;
    accy = 0;
}

void Sprite::hitGround()
{
    resetPhysic();
    stateTimer = 0;
}

float Sprite::getVelocityX()
{
    return velocity_x;
}

float Sprite::getVelocityY()
{
    return velocity_y;
}

void Sprite::setVelocityX(float vx)
{
    this->velocity_x = vx;
}

void Sprite::setVelocityY(float vy)
{
    this->velocity_y = vy;
}

void Sprite::setVelocity(float vx, float vy)
{
    this->velocity_x = vx;
    this->velocity_y = vy;
}

void Sprite::addCollisionRects(SDL_Rect *hitrects, int hitsize, SDL_Rect *attackrects, int attacksize)
{
    struct CollisionArea area;

    if (hitrects != NULL && hitsize > 0) {
        area.hitRectArray = (SDL_Rect *)malloc(sizeof(struct SDL_Rect) * hitsize);
        assert(area.hitRectArray != NULL);
        memcpy(area.hitRectArray, hitrects, sizeof(struct SDL_Rect) * hitsize);
    } else {
        area.hitRectArray = NULL;
    }
    area.sizeHit = hitsize;

    if (attackrects != NULL && attacksize > 0) {
        area.attackRectArray = (SDL_Rect *)malloc(sizeof(struct SDL_Rect) * attacksize);
        assert(area.attackRectArray != NULL);
        memcpy(area.attackRectArray, attackrects, sizeof(struct SDL_Rect) * attacksize);
    } else {
        area.attackRectArray = NULL;
    }
    area.sizeAttack = attacksize;

    this->collisionAreas.push_back(area);
}

void Sprite::addCollisionSequence(const char *name, Uint32 framerate, int indexarray[], int length)
{
    struct CollisionAreaSequence *sequence = new CollisionAreaSequence;
    assert(sequence);
    sequence->indexArray = (int *)malloc(sizeof(int) * length);
    assert(sequence->indexArray);

    sequence->name = name;
    sequence->rateFrame = framerate;
    memcpy(sequence->indexArray, indexarray, sizeof(int) * length);
    sequence->size = length;

    this->collisionAreaSequences.push_back(sequence);
}

void Sprite::useCollisionSequence(const char *name)
{
    for (vector<struct CollisionAreaSequence *>::iterator i = collisionAreaSequences.begin(); i != collisionAreaSequences.end(); i++) {
        if ( (*i)->name.compare(name) == 0 ) {
            curAreaSequence = *i;
            curAreaSequence->reset();
            curAreaIndex = curAreaSequence->getCurrentAreaIndex();
            oldFrameStamp = 0;
            break;
        }
    }
}

const SDL_Rect *Sprite::getCurRealHitCollisionRects(int &size)
{
    if (collisionAreas.size() == 0) {
        return NULL;
    }

    struct CollisionArea *curarea = &collisionAreas.at(curAreaIndex);
    size = curarea->sizeHit;
    assert(size <= MAX_COLLISION_RECT_ARRAY_LEN);

    SDL_Rect screenposition = getPositionScreenCoor();
    SDL_Rect realrect;
    for (int i=0; i<curarea->sizeHit; i++) {
        if (flipHorizontal) {
            realrect = {(Sint16)(screenposition.x + frameAnchorPoints[currentFrame].x - curarea->hitRectArray[i].w - curarea->hitRectArray[i].x),
                (Sint16)(screenposition.y + curarea->hitRectArray[i].y - frameAnchorPoints[currentFrame].y),
                curarea->hitRectArray[i].w, curarea->hitRectArray[i].h};
        } else {
            realrect = {(Sint16)(screenposition.x - frameAnchorPoints[currentFrame].x + curarea->hitRectArray[i].x),
                (Sint16)(screenposition.y + curarea->hitRectArray[i].y - frameAnchorPoints[currentFrame].y),
                curarea->hitRectArray[i].w, curarea->hitRectArray[i].h};
        }
        realHitRectArray[i] = realrect;
    }
    return realHitRectArray;
}

const SDL_Rect *Sprite::getCurRealAttackCollisionRects(int &size)
{
    if (collisionAreas.size() == 0) {
        return NULL;
    }

    struct CollisionArea *curarea = &collisionAreas.at(curAreaIndex);
    size = curarea->sizeAttack;
    assert(size <= MAX_COLLISION_RECT_ARRAY_LEN);

    SDL_Rect screenposition = getPositionScreenCoor();
    SDL_Rect realrect;
    for (int i=0; i<curarea->sizeAttack; i++) {
        if (flipHorizontal) {
            realrect = {(Sint16)(screenposition.x + frameAnchorPoints[currentFrame].x - curarea->attackRectArray[i].w - curarea->attackRectArray[i].x),
                (Sint16)(screenposition.y + curarea->attackRectArray[i].y - frameAnchorPoints[currentFrame].y),
                curarea->attackRectArray[i].w, curarea->attackRectArray[i].h};
        } else {
            realrect = {(Sint16)(screenposition.x + curarea->attackRectArray[i].x - frameAnchorPoints[currentFrame].x),
                (Sint16)(screenposition.y + curarea->attackRectArray[i].y - frameAnchorPoints[currentFrame].y),
                curarea->attackRectArray[i].w, curarea->attackRectArray[i].h};
        }
        realAttackRectArray[i] = realrect;
    }
    return realAttackRectArray;
}

void Sprite::update(Uint32 frameStamp)
{
    Character::update(frameStamp);
    Animation::update(frameStamp);
    updateCollisionArea(frameStamp);

    if (state == WALK) {
        if (oldstate != state || oldforward != forward) {
            if (forward) {
                playSequence("walk");
                useCollisionSequence("walk");
            } else {
                playSequenceBackorder("walk");
                useCollisionSequence("walk");
            }
            resetPhysic();
            velocity_x = speed;
            if (facing == LEFT && forward) {
                velocity_x = -velocity_x;
            } else if (facing == RIGHT && !forward) {
                velocity_x = -velocity_x;
            }
        }
    }

    if (state == ATTACK) {
        if (oldstate != state) {
            playSequence("attack");
            useCollisionSequence("attack");
            resetPhysic();
        }
    }

    if (state == ATTACK3) {
        if (oldstate != state) {
            playSequence("rush");
            useCollisionSequence("rush");
            velocity_x = speed * 3;
            if (facing == LEFT && forward) {
                velocity_x = -velocity_x;
            } else if (facing == RIGHT && !forward) {
                velocity_x = -velocity_x;
            }
        }
    }

    if (state == STAND) {
        if (oldstate != state) {
            playSequence("stand");
            useCollisionSequence("stand");
            resetPhysic();
        }
    }

    if (state == GUARD) {
        if (oldstate != state) {
            playSequence("guard");
            useCollisionSequence("guard");
            resetPhysic();
            velocity_x = speed * 1.5;
            if (facing == RIGHT) {
                velocity_x = -velocity_x;
            }
        }

        if (moveTimer == 0) {
            resetPhysic();
        }
    }

    if (state == SQUATGUARD) {
        if (oldstate != state) {
            playSequence("guardsquat");
            useCollisionSequence("guardsquat");
            resetPhysic();
            velocity_x = speed * 1.5;
            if (facing == RIGHT) {
                velocity_x = -velocity_x;
            }
        }
        if (moveTimer == 0) {
            resetPhysic();
        }
    }

    if (state == HIT) {
        if (oldstate != state) {
            playSequence("hit");
            useCollisionSequence("hit");
            resetPhysic();
            velocity_x = speed * 1.5;
            if (facing == RIGHT) {
                velocity_x = -velocity_x;
            }
        }
        if (moveTimer == 0) {
            resetPhysic();
        }
    }

    if (state == FALL) {
        if (oldstate != state) {
            playSequence("fall");
            useCollisionSequence("fall");
            resetPhysic();
            velocity_x = speed * 1.5;
            if (facing == RIGHT) {
                velocity_x = -velocity_x;
            }
            velocity_y = -speed * 3;
            accy = speed * 3 / 11;
        }
    }

    if (state == LIE) {
        if (oldstate != state) {
            playSequence("lie");
            useCollisionSequence("lie");
            resetPhysic();
        }
    }

    if (state == RAISE) {
        if (oldstate != state) {
            playSequence("raise");
            useCollisionSequence("raise");
            resetPhysic();
        }
    }

    if (state == JUMP) {
        if (oldstate != state) {
            playSequence("jump");
            useCollisionSequence("jump");
            resetPhysic();
            if (jumpingDirection != 0) {
                velocity_x = speed * 1.5;
                if (facing == LEFT && jumpingDirection == 1) {
                    velocity_x = -velocity_x;
                } else if (facing == RIGHT && jumpingDirection == 2) {
                    velocity_x = -velocity_x;
                }
            }
            velocity_y = -speed * 6;
            accy = speed * 6 / 20;
        }
    }

    if (state == LAND) {
        if (oldstate != state) {
            playSequence("land");
            useCollisionSequence("land");
            resetPhysic();
        }
    }

    if (state == JUMPATTACK) {
        if (oldstate != state) {
            playSequence("jumpattack");
            useCollisionSequence("jumpattack");
            if (jumpingDirection != 0) {
                velocity_x = speed * 1.5;
                if (facing == LEFT && jumpingDirection == 1) {
                    velocity_x = -velocity_x;
                } else if (facing == RIGHT && jumpingDirection == 2) {
                    velocity_x = -velocity_x;
                }
            }
        }
    }

    // physic
    velocity_x = velocity_x + accx;
    velocity_y = velocity_y + accy;
    /*velocity_x = velocity_x + accx;
    x += velocity_x;

    velocity_y = velocity_y + accy;
    y += velocity_y;
    if (y > groundline) {
        y = groundline;
        stateTimer = 0;
        resetPhysic();
    }

    this->position.x = x;
    this->position.y = y;
    */


    oldstate = state;
    oldforward = forward;
}

void Sprite::CollisionAreaSequence::reset()
{
    curIndex = 0;
}

int Sprite::CollisionAreaSequence::getCurrentAreaIndex()
{
    return indexArray[curIndex];
}

int Sprite::CollisionAreaSequence::getNextAreaIndex()
{
    curIndex ++;
    if (curIndex == size) {
        //TODO: do we need to consider animation play style (ONEC, LOOP) ?
        curIndex = size - 1;
    }
    return indexArray[curIndex];
}

void Sprite::updateCollisionArea(Uint32 frameStamp)
{
    if (curAreaSequence == NULL) {
        return;
    }
    if (oldFrameStamp == 0) {
        // is the first frame
        oldFrameStamp = frameStamp;
    } else if (oldFrameStamp + curAreaSequence->rateFrame <= frameStamp) {
        oldFrameStamp = frameStamp;
        curAreaIndex = curAreaSequence->getNextAreaIndex();
    }
}

void Sprite::draw(SDL_Surface *dst)
{
    Animation::draw(dst);

#ifdef DEBUG
    if (collisionAreas.size() == 0) {
        return;
    }

    SDL_Rect screenposition = getPositionScreenCoor();

    SDL_Surface *surface = NULL;//SDL_CreateRGBSurface(SDL_SWSURFACE, 200, 200, 32, rmask, gmask, bmask, amask);
    surface = SDL_CreateRGBSurface(fullImage->flags, fullImage->w, fullImage->h, fullImage->format->BitsPerPixel,
        fullImage->format->Rmask, fullImage->format->Gmask, fullImage->format->Bmask, fullImage->format->Amask);
    SDL_SetAlpha(surface, SDL_RLEACCEL | SDL_SRCALPHA, 0x80);

    struct CollisionArea *curarea = &collisionAreas.at(curAreaIndex);
    SDL_FillRect( surface, NULL, SDL_MapRGBA(dst->format, 64, 200, 64, 0));
    for (int i=0; i<curarea->sizeHit; i++) {
        SDL_Rect srcrect = {0, 0, curarea->hitRectArray[i].w, curarea->hitRectArray[i].h};
        if (flipHorizontal) {
            SDL_Rect dstrect = {(Sint16)(screenposition.x + frameAnchorPoints[currentFrame].x - curarea->hitRectArray[i].w - curarea->hitRectArray[i].x),
                (Sint16)(screenposition.y + curarea->hitRectArray[i].y - frameAnchorPoints[currentFrame].y),
                0, 0};
            SDL_BlitSurface(surface, &srcrect, dst, &dstrect);
        } else {
            SDL_Rect dstrect = {(Sint16)(screenposition.x - frameAnchorPoints[currentFrame].x + curarea->hitRectArray[i].x),
                (Sint16)(screenposition.y + curarea->hitRectArray[i].y - frameAnchorPoints[currentFrame].y),
                0, 0};
            SDL_BlitSurface(surface, &srcrect, dst, &dstrect);
        }
    }
    SDL_FillRect( surface, NULL, SDL_MapRGBA(dst->format, 200, 64, 64, 0));
    for (int i=0; i<curarea->sizeAttack; i++) {
        SDL_Rect srcrect = {0, 0, curarea->attackRectArray[i].w, curarea->attackRectArray[i].h};
        if (flipHorizontal) {
            SDL_Rect dstrect = {(Sint16)(screenposition.x + frameAnchorPoints[currentFrame].x - curarea->attackRectArray[i].w - curarea->attackRectArray[i].x),
                (Sint16)(screenposition.y + curarea->attackRectArray[i].y - frameAnchorPoints[currentFrame].y),
                curarea->attackRectArray[i].w, curarea->attackRectArray[i].h};
            SDL_BlitSurface(surface, &srcrect, dst, &dstrect);
        } else {
            SDL_Rect dstrect = {(Sint16)(screenposition.x + curarea->attackRectArray[i].x - frameAnchorPoints[currentFrame].x),
                (Sint16)(screenposition.y + curarea->attackRectArray[i].y - frameAnchorPoints[currentFrame].y),
                curarea->attackRectArray[i].w, curarea->attackRectArray[i].h};
            SDL_BlitSurface(surface, &srcrect, dst, &dstrect);
        }
    }
    SDL_FreeSurface(surface);
#endif
}


}
