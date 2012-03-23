#include <stdlib.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <math.h>

#include "keyfilter.h"
#include "ftgkeys.h"
#include "keystream.h"
#include "sprite.h"
#include "collisiondetect.h"
#include "resource.h"
#include "healthbar.h"
#include "ai.h"
#include "stage.h"

using namespace dragonfighting;


int main(int argc, char **argv)
{
    int exited = 0;
    SDL_Surface *screen = NULL;
    Uint32 interval = 1000/60;
    Uint32 frame = 0;
    Uint32 oldtime = SDL_GetTicks();
    Uint32 newtime = oldtime;
    SDL_Init(SDL_INIT_EVERYTHING);
    atexit(SDL_Quit);

    screen = SDL_SetVideoMode(420, 224, 32, SDL_SWSURFACE);
    if (screen == NULL)
    {
        fprintf(stderr, "error set video mode.\n");
        exit(1);
    }

    // Init key map
    SDLKeyConverter keyconv;
    keyconv.registerKey(SDLK_w, CTRLKEY_UP);
    keyconv.registerKey(SDLK_s, CTRLKEY_DOWN);
    keyconv.registerKey(SDLK_a, CTRLKEY_LEFT);
    keyconv.registerKey(SDLK_d, CTRLKEY_RIGHT);
    keyconv.registerKey(SDLK_j, CTRLKEY_A);
    keyconv.registerKey(SDLK_k, CTRLKEY_B);
    keyconv.registerKey(SDLK_l, CTRLKEY_C);
    keyconv.registerKey(SDLK_i, CTRLKEY_D);

    CtrlKeyReaderWriter sdlkeyrw1;
    CtrlKeyReaderWriter sdlkeyrw2;

    // Init key filter
    KeyFilter keyFilter1;
    KeyFilter keyFilter2;
    unsigned char cmd[][8] = {
        {FTGKEY_6, FTGKEY_3, FTGKEY_2, FTGKEY_3, FTGKEY_A},
        {FTGKEY_2, FTGKEY_3, FTGKEY_6, FTGKEY_A},
        {FTGKEY_2, FTGKEY_3, FTGKEY_6, FTGKEY_B},
        {FTGKEY_2, FTGKEY_2, FTGKEY_A},
        {FTGKEY_6, FTGKEY_A},
        {FTGKEY_2, FTGKEY_A},
        {FTGKEY_3, FTGKEY_A},
    };
    keyFilter1.addCommand("6323A", cmd[0], 5);
    keyFilter1.addCommand("236A", cmd[1], 4);
    keyFilter1.addCommand("236B", cmd[2], 4);
    keyFilter1.addCommand("22A", cmd[3], 3);
    keyFilter1.addCommand("6A", cmd[4], 2);
    keyFilter1.addCommand("2A", cmd[5], 2);
    keyFilter1.addCommand("3A", cmd[6], 2);

    keyFilter2.addCommand("6323A", cmd[0], 5);
    keyFilter2.addCommand("236A", cmd[1], 4);
    keyFilter2.addCommand("236B", cmd[2], 4);
    keyFilter2.addCommand("22A", cmd[3], 3);
    keyFilter2.addCommand("6A", cmd[4], 2);
    keyFilter2.addCommand("2A", cmd[5], 2);
    keyFilter2.addCommand("3A", cmd[6], 2);


#if 0
    SDL_Surface *imgloaded = NULL;
    // Init charactor
    //Character p1;
    Sprite p1;
    p1.setName("p1");
    SDL_Surface *p1Image = NULL;
    int indexs_p1[30];
    for (unsigned int i=0; i<sizeof(indexs_p1)/sizeof(int); i++) {
        indexs_p1[i] = i;
    }
    SDL_Rect rect = {19, 21, 119, 90};
    SDL_Rect anchorpoint = {0,0,0,0};
    SDL_Rect hitrects[30];
    SDL_Rect attackrects[30];

    p1.setKeyFilter(&keyFilter1);
    p1.setInputer(&sdlkeyrw1);

    imgloaded = IMG_Load("./firedragon.png");
    p1Image = SDL_DisplayFormat(imgloaded);
    //SDL_SetAlpha(p1Image, SDL_SRCALPHA, 0x80);
    SDL_SetColorKey( p1Image, SDL_SRCCOLORKEY, SDL_MapRGB( p1Image->format, 0, 0, 0 ) );
    SDL_FreeSurface(imgloaded);

    p1.setFullImage(p1Image);

    rect.x = 19; rect.y = 21; rect.w = 119; rect.h = 90;
    anchorpoint.x = 64; anchorpoint.y = 87;
    p1.addFrame(rect, anchorpoint);
    rect.x += rect.w;
    rect.w = 117;
    anchorpoint.x = 64; anchorpoint.y = 87;
    p1.addFrame(rect, anchorpoint);
    rect.x += rect.w;
    rect.w = 120;
    anchorpoint.x = 64; anchorpoint.y = 87;
    p1.addFrame(rect, anchorpoint);
    rect.x += rect.w;
    anchorpoint.x = 64; anchorpoint.y = 87;
    p1.addFrame(rect, anchorpoint);
    p1.addSequence("stand", 20, AnimationSequence::LOOP, indexs_p1, 4);

    //TODO
    hitrects[0] = {6,4,73,32};
    hitrects[1] = {34,36,54,48};
    p1.addCollisionRects(hitrects, 2, NULL, 0);
    p1.addCollisionSequence("stand", 20, indexs_p1, 1);

    rect.x = 27; rect.y = 120; rect.w = 109; rect.h = 108;
    anchorpoint.x = 68; anchorpoint.y = 106;
    p1.addFrame(rect, anchorpoint);
    rect.x = 145; rect.y = 140; rect.w = 132; rect.h = 89;
    anchorpoint.x = 99; anchorpoint.y = 87;
    p1.addFrame(rect, anchorpoint);
    rect.x = 296; rect.y = 144; rect.w = 120; rect.h = 85;
    anchorpoint.x = 87; anchorpoint.y = 83;
    p1.addFrame(rect, anchorpoint);
    p1.addSequence("attack", 10, AnimationSequence::ONCE, indexs_p1+4, 3);

    //TODO
    hitrects[0] = {27,26,42,72};
    hitrects[1] = {69,48,13,52};
    attackrects[0] = {0,0,31,38};
    p1.addCollisionRects(hitrects, 2, attackrects, 1);
    hitrects[0] = {33,14,59,29};
    hitrects[1] = {51,37,59,47};
    attackrects[0] = {0,49,27,20};
    attackrects[1] = {17,31,21,19};
    p1.addCollisionRects(hitrects, 2, attackrects, 2);
    hitrects[0] = {33,14,59,29};
    hitrects[1] = {51,37,59,47};
    attackrects[0] = {0,49,27,20};
    attackrects[1] = {18,31,19,19};
    p1.addCollisionRects(hitrects, 2, attackrects, 2);
    p1.addCollisionSequence("attack", 10, indexs_p1+1, 3);

    rect.x = 27; rect.y = 477; rect.w = 115; rect.h = 84;
    anchorpoint.x = 80; anchorpoint.y = 83;
    p1.addFrame(rect, anchorpoint);
    rect.x = 148; rect.y = 478; rect.w = 115; rect.h = 84;
    anchorpoint.x = 80; anchorpoint.y = 83;
    p1.addFrame(rect, anchorpoint);
    rect.x = 267; rect.y = 480; rect.w = 115; rect.h = 77;
    anchorpoint.x = 80; anchorpoint.y = 83;
    p1.addFrame(rect, anchorpoint);
    rect.x = 389; rect.y = 484; rect.w = 115; rect.h = 76;
    anchorpoint.x = 80; anchorpoint.y = 83;
    p1.addFrame(rect, anchorpoint);
    rect.x = 506; rect.y = 481; rect.w = 115; rect.h = 84;
    anchorpoint.x = 80; anchorpoint.y = 83;
    p1.addFrame(rect, anchorpoint);
    rect.x = 33; rect.y = 577; rect.w = 115; rect.h = 84;
    anchorpoint.x = 80; anchorpoint.y = 83;
    p1.addFrame(rect, anchorpoint);
    p1.addSequence("walk", 8, AnimationSequence::LOOP, indexs_p1+7, 5);

    p1.addSequence("hit", 8, AnimationSequence::ONCE, indexs_p1, 1);
    p1.addCollisionSequence("hit", 10, indexs_p1, 1);

    p1.setDefaultSequence("stand");
    p1.playSequenceBackorder("stand");
    p1.useCollisionSequence("stand");
    p1.setPosition(200.0f, 400.0f);
    p1.setFlipHorizontal(true);
    p1.setFacing(Character::RIGHT);
    p1.setSpeed(2.0f);
#endif

    //Character p1;
    Sprite *p1 = SpriteFactory::loadSprite("data", "minotaur");
    if (p1 == NULL) {
        exit(1);
    }
    p1->setName("p1");
    p1->setKeyFilter(&keyFilter1);
    p1->setInputer(&sdlkeyrw1);
    p1->playSequence("stand");
    p1->useCollisionSequence("stand");
    p1->setSpeed(2.0f);

    //Character p2;
    Sprite *p2 = SpriteFactory::loadSprite("data", "minotaur");
    if (p2 == NULL) {
        exit(1);
    }
    p2->setName("p2");
    p2->setKeyFilter(&keyFilter2);
    p2->setInputer(&sdlkeyrw2);
    p2->playSequence("stand");
    p2->useCollisionSequence("stand");
    p2->setSpeed(2.0f);

    // Init Stage
    Stage firstStage = Stage(p1, p2);
    firstStage.setPosition(-165, 0);

    // Init AI
    AI ai2 = AI(p2, p1);

    while(exited==0)
    {
        // AI
        ai2.update(frame);

        //----input----
        struct Ctrl_KeyEvent ctrlevent;
        SDL_Event event;
        if (SDL_PollEvent(&event) == 1) {
            ctrlevent.frameStamp = frame;
            ctrlevent.controler = 1;
            if((event.type==SDL_KEYDOWN && event.key.keysym.sym==SDLK_ESCAPE) || (event.type==SDL_QUIT)) exited=1;
            else if (event.type==SDL_KEYDOWN)
            {
                ctrlevent.type = Ctrl_KEYDOWN;
                //ctrlevent.key = keyconv.convert(event.key.keysym.sym, p1->getFacing() == Character::LEFT);
                ctrlevent.key = keyconv.convert(event.key.keysym.sym);
                if (ctrlevent.key != 0) {
                    sdlkeyrw1.writeEvent(&ctrlevent);
                }
                //ctrlevent.key = keyconv.convert(event.key.keysym.sym, p2->getFacing() == Character::LEFT);
                //sdlkeyrw2.writeEvent(&ctrlevent);
            }
            else if (event.type==SDL_KEYUP)
            {
                ctrlevent.type = Ctrl_KEYUP;
                //ctrlevent.key = keyconv.convert(event.key.keysym.sym, p1->getFacing() == Character::LEFT);
                ctrlevent.key = keyconv.convert(event.key.keysym.sym);
                if (ctrlevent.key != 0) {
                    sdlkeyrw1.writeEvent(&ctrlevent);
                }
                //ctrlevent.key = keyconv.convert(event.key.keysym.sym, p2->getFacing() == Character::LEFT);
                //sdlkeyrw2.writeEvent(&ctrlevent);
            }
        }

        if (ai2.pollEvent(&ctrlevent)) {
            ctrlevent.frameStamp = frame;
            ctrlevent.controler = 2;
            sdlkeyrw2.writeEvent(&ctrlevent);
        }

        // ----logic----
        firstStage.update(frame);

        // ----draw----
        SDL_FillRect( screen, NULL, 0x00008080 );
        //SDL_FillRect( screen, &rect, color );
        firstStage.draw(screen);
        SDL_Flip(screen);

        // ----frame control----
        frame++;
        newtime = SDL_GetTicks();
        if (newtime - oldtime < interval)
            SDL_Delay(interval - newtime + oldtime);
        oldtime = SDL_GetTicks();
    }

    SpriteFactory::freeSprite(p1);
    SpriteFactory::freeSprite(p2);

    SDL_Quit();
    return 0;
}


