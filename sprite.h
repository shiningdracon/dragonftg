#ifndef _SPRITE_H_
#define _SPRITE_H_

#include "animation.h"
#include "character.h"

namespace dragonfighting {


class AI;

class Sprite : public Character, public Animation
{
    struct CollisionArea
    {
        SDL_Rect *hitRectArray;
        int sizeHit;
        SDL_Rect *attackRectArray;
        int sizeAttack;
    };

    struct CollisionAreaSequence
    {
        string name;
        Uint32 rateFrame;
        int *indexArray;
        int size;
        int curIndex;

        void reset();
        int getCurrentAreaIndex();
        int getNextAreaIndex();
    };

    private:
        float speed;
        float velocity_x;
        float velocity_y;
        float x;
        float y;
        float accx;
        float accy;
        enum State oldstate;
        bool oldforward;
        vector<struct CollisionArea> collisionAreas;
        vector<struct CollisionAreaSequence *> collisionAreaSequences;
        struct CollisionAreaSequence *curAreaSequence;
        int curAreaIndex;
        Uint32 oldFrameStamp;
        SDL_Rect *realHitRectArray;
        SDL_Rect *realAttackRectArray;

        void resetPhysic();

    public:
        Sprite();
        virtual ~Sprite();
        void setPosition(float x, float y);
        void setPositionX(float x);
        float getPositionX();
        float getPositionY();
        void setSpeed(float speed);
        float getVelocityX();
        float getVelocityY();
        void setVelocityX(float vx);
        void setVelocityY(float vy);
        void setVelocity(float vx, float vy);
        void hitGround();
        void addCollisionRects(SDL_Rect *hitrects, int hitsize, SDL_Rect *attackrects, int attacksize);
        void addCollisionSequence(const char *name, Uint32 framerate, int indexarray[], int length);
        void useCollisionSequence(const char *name);
        const SDL_Rect *getCurRealHitCollisionRects(int &size /*out*/);
        const SDL_Rect *getCurRealAttackCollisionRects(int &size /*out*/);
        virtual void update(Uint32 frameStamp);
        void updateCollisionArea(Uint32 frameStamp);

        //debug
        void draw(SDL_Surface *dst);

        friend class AI;
};

}


#endif
