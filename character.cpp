#include <assert.h>
#include "character.h"

namespace dragonfighting {

Character::Character() :
    keyFilter(NULL),
    keyInputer(NULL),
    state(STAND),
    facing(LEFT),
    forward(true),
    jumpingDirection(0),
    stateTimer(0),
    stateAllow(0),
    invincible(false),
    current_key_state(0)
{
}

Character::~Character()
{
}

void Character::setName(const char *name)
{
    unsigned int i;
    for (i=0; i<sizeof(this->name)-1 && name[i]!='\0'; i++) {
        this->name[i] = name[i];
    }
    this->name[i] = '\0';
}

const char *Character::getName()
{
    return this->name;
}

void Character::setKeyFilter(KeyFilter *filter)
{
    this->keyFilter = filter;
}

void Character::setInputer(CtrlKeyReader *inputer)
{
    this->keyInputer = inputer;
}

void Character::setFacing(enum Facing facing)
{
    if (this->facing != facing) {
        // ugly way to handle charactor direction change
        bool key1down = false, key2down = false;
        if ( (current_key_state & FTGKEY_6) != 0 ) {
            key1down = true;
        }
        if ( (current_key_state & FTGKEY_4) != 0 ) {
            key2down = true;
        }

        if (key1down) {
            current_key_state |= FTGKEY_4;
        } else {
            current_key_state &= ~FTGKEY_4;
        }
        if (key2down) {
            current_key_state |= FTGKEY_6;
        } else {
            current_key_state &= ~FTGKEY_6;
        }

        this->keyFilter->flipHorizontal();
        this->facing = facing;
    }
}

enum Character::Facing Character::getFacing()
{
    return this->facing;
}

enum Character::State Character::getState()
{
    return state;
}

Uint32 Character::getStateAllow()
{
    return stateAllow;
}

bool Character::isGuard()
{
    return state == GUARD || state == SQUATGUARD || state == JUMPGUARD;
}

bool Character::isInvincible()
{
    return invincible;
}

void Character::update(Uint32 frameStamp)
{
    assert(keyFilter != NULL);
    assert(keyInputer != NULL);

    struct Ctrl_KeyEvent event;
    if (this->keyInputer->readEvent(&event, frameStamp) == 1) {
        if (event.type == Ctrl_KEYDOWN) {
            current_key_state |= ctrlkey2ftgkey(event.key, facing == LEFT);
        } else if (event.type == Ctrl_KEYUP) {
            current_key_state &= ~ctrlkey2ftgkey(event.key, facing == LEFT);
        }
    }
    this->keyFilter->updateKeys(current_key_state);

    updateStateMachine();
}

void Character::updateStateMachine()
{
    bool iscmd = false;
    char commandname[16];

    if (stateTimer == 0) {
        if (state == JUMP || state == JUMPATTACK || state == JUMPGUARD) {
            state = LAND;
            stateAllow = ALLOW_NONE;
            stateTimer = 8;
        } else if (state == FALL) {
            state = LIE;
            stateAllow = ALLOW_NONE;
            stateTimer = 60;
        } else if (state == LIE) {
            state = RAISE;
            stateAllow = ALLOW_NONE;
            stateTimer = 16;
        } else {
            state = STAND;
            stateAllow = ALLOW_ALL;
            invincible = false;
        }
    }

    iscmd = this->keyFilter->pollCurrentCommandName(commandname, sizeof(commandname));

    if (!iscmd) {
        if (state != WALK && (stateAllow & ALLOW_WALK) && this->keyFilter->getKeyState(FTGKEY_6)) {
            state = WALK;
            stateAllow = ALLOW_ALL;
            stateTimer = 2;
            forward = true;
        } else if (state != WALK && (stateAllow & ALLOW_WALK) && this->keyFilter->getKeyState(FTGKEY_4)) {
            state = WALK;
            stateAllow = ALLOW_ALL;
            stateTimer = 2;
            forward = false;
        } else if (state != ATTACK && (stateAllow & ALLOW_ATTACK) && this->keyFilter->getKeyState(FTGKEY_A)
                && this->keyFilter->getKeyState(FTGKEY_A) < 10) {
            state = ATTACK;
            stateAllow = ALLOW_NONE;
            stateTimer = 30;
        } else if (state != JUMP && (stateAllow & ALLOW_JUMP) && this->keyFilter->getKeyState(FTGKEY_8)) {
            state = JUMP;
            stateAllow = ALLOW_JUMPATTACK | ALLOW_JUMPGUARD;
            stateTimer = -1;
            if (this->keyFilter->getKeyState(FTGKEY_6)) {
                jumpingDirection = 1;
            } else if (this->keyFilter->getKeyState(FTGKEY_4)) {
                jumpingDirection = 2;
            } else {
                jumpingDirection = 0;
            }
        } else if (state == JUMP && (stateAllow & ALLOW_JUMPATTACK) && this->keyFilter->getKeyState(FTGKEY_A)) {
            state = JUMPATTACK;
            stateAllow = ALLOW_NONE;
            stateTimer = stateTimer;
        }
    } else {
        if (state != ATTACK3 && (stateAllow & ALLOW_ATTACK3) && strcmp(commandname, "6323A") == 0) {
            state = ATTACK3;
            stateAllow = ALLOW_NONE;
            stateTimer = 30;
        }
    }
    stateTimer --;
    moveTimer --;
}

void Character::underAttack(enum HitType hittype)
{
    if (state == GUARD || state == SQUATGUARD || state == JUMPGUARD) {
        stateTimer = 30;
    } else if ((stateAllow & ALLOW_GUARD) &&
            (stateAllow & ALLOW_SQUATGUARD) &&
            (stateAllow & ALLOW_JUMPGUARD) &&
            this->keyFilter->getKeyState(FTGKEY_4)) {
        if (state == JUMP || state == JUMPATTACK) {
            state = JUMPGUARD;
            stateAllow = ALLOW_JUMPGUARD;
            stateTimer = 30;
        } else {
            if (this->keyFilter->getKeyState(FTGKEY_2)) {
                state = SQUATGUARD;
                stateAllow = ALLOW_GUARD;
            } else {
                state = GUARD;
                stateAllow = ALLOW_SQUATGUARD;
            }
            stateTimer = 30;
            moveTimer = 10;
        }
    } else {
        if (hittype == NORMAL) {
            state = HIT;
            stateAllow = ALLOW_NONE;
            stateTimer = 15;
            moveTimer = 10;
            invincible = true;
        } else {
            state = FALL;
            stateAllow = ALLOW_NONE;
            stateTimer = -1;
            invincible = true;
        }
    }
}


}



