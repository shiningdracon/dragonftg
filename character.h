#ifndef _CHARACTER_H_
#define _CHARACTER_H_

#include "keyfilter.h"
#include "keystream.h"

namespace dragonfighting {

class AI;
class Character {
public:
    enum Facing {
        LEFT,
        RIGHT,
    };

    enum State {
        NONE = 0,
        STAND = 1,
        ATTACK,
        ATTACK2,
        ATTACK3,
        ATTACK4,
        GUARD,
        HIT,
        FALL,
        LIE,
        RAISE,
        WALK,
        RUN,
        DASH,
        SQUAT,
        SQUATATTACK,
        SQUATGUARD,
        JUMP,
        JUMP2,
        JUMPATTACK,
        JUMPGUARD,
        JUMPHIT,
        LAND,
        COUNTERED,
        GUARDBREAK,
        // max = 32
    };
    const static Uint32 ALLOW_ALL           = 0xFFFFFFFF;
    const static Uint32 ALLOW_NONE          = 0x00000000;
    const static Uint32 ALLOW_STAND         = STAND; // 0x00000001
    const static Uint32 ALLOW_ATTACK        = ALLOW_STAND << ATTACK;
    const static Uint32 ALLOW_ATTACK3       = ALLOW_STAND << ATTACK3;
    const static Uint32 ALLOW_GUARD         = ALLOW_STAND << GUARD;
    const static Uint32 ALLOW_SQUAT         = ALLOW_STAND << SQUAT;
    const static Uint32 ALLOW_SQUATGUARD    = ALLOW_STAND << SQUATGUARD;
    const static Uint32 ALLOW_WALK          = ALLOW_STAND << WALK;
    const static Uint32 ALLOW_RUN           = ALLOW_STAND << RUN;
    const static Uint32 ALLOW_DASH          = ALLOW_STAND << DASH;
    const static Uint32 ALLOW_JUMP          = ALLOW_STAND << JUMP;
    const static Uint32 ALLOW_JUMPATTACK    = ALLOW_STAND << JUMPATTACK;
    const static Uint32 ALLOW_JUMPGUARD     = ALLOW_STAND << JUMPGUARD;

    enum HitType {
        NORMAL,
        THUMP,
    };

protected:
    char name[16];
    KeyFilter *keyFilter;
    CtrlKeyReader *keyInputer;
    enum State state;
    enum Facing facing;  // facing left or right
    bool forward;  // is moving forward or backward
    int jumpingDirection;

    // action last, return to STAND when timeout
    Uint32 stateTimer;
    Uint32 moveTimer;

private:
    Uint32 stateAllow;
    bool invincible;
    unsigned char current_key_state;

    void updateStateMachine();

public:
    Character();
    virtual ~Character();
    void setName(const char *name);
    const char *getName();
    void setKeyFilter(KeyFilter *filter);
    void setInputer(CtrlKeyReader *inputer);
    virtual void update(Uint32 frameStamp);

    void setFacing(enum Facing facing);
    enum Facing getFacing();
    enum State getState();
    Uint32 getStateAllow();
    bool isGuard();
    bool isInvincible();

    void underAttack(enum HitType hittype);
};

}

#endif
