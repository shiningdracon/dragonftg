#ifndef _MENUS_H_
#define _MENUS_H_

#include <SDL/SDL.h>
#include "widget.h"
#include "keystream.h"

namespace dragonfighting {

class MainMenu : public Widget
{
    public:
        MainMenu();
        ~MainMenu();

        void update(Uint32 frameStamp);
        void draw(SDL_Surface *dst);
        void setInputer(CtrlKeyReader *inputer);

    private:
        SDL_Surface *bkImage;
        CtrlKeyReader *keyInputer;
        unsigned char current_key_state;
        Uint32 repeatTimer;
};

}

#endif
