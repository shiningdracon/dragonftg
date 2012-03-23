#include <SDL/SDL_image.h>
#include <assert.h>
#include "menus.h"

namespace dragonfighting {

MainMenu::MainMenu() :
    bkImage(NULL),
    keyInputer(NULL),
    current_key_state(0),
    repeatTimer(0)
{
    // Background
    SDL_Surface *imgloaded = IMG_Load("./kingdragonstitle.png");
    bkImage = SDL_DisplayFormat(imgloaded);
    SDL_FreeSurface(imgloaded);
}

MainMenu::~MainMenu()
{
    SDL_FreeSurface(bkImage);
}

void MainMenu::update(Uint32 frameStamp)
{
    assert(keyInputer != NULL);

    struct Ctrl_KeyEvent event;
    if (this->keyInputer->readEvent(&event, frameStamp) == 1) {
        if (event.type == Ctrl_KEYDOWN) {
            current_key_state |= event.key;
        } else if (event.type == Ctrl_KEYUP) {
            current_key_state &= ~event.key;
        }
    }

    if (repeatTimer == 0) {
    } else {
        repeatTimer --;
    }
}

void MainMenu::draw(SDL_Surface *dst)
{
    SDL_Rect screenposition = getPositionScreenCoor();
    SDL_BlitSurface(bkImage, NULL, dst, &screenposition);
}

void MainMenu::setInputer(CtrlKeyReader *inputer)
{
    this->keyInputer = inputer;
}

}
