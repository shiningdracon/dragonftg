#include "healthbar.h"

namespace dragonfighting {

HealthBar::HealthBar() :
    leftToRight(true),
    max(0),
    current(0)
{
    geometry = {0, 0, 0, 0};
    geometry2 = {0, 0, 0, 0};
}

HealthBar::~HealthBar()
{
}

void HealthBar::setLeftToRight(bool leftToRight)
{
    this->leftToRight = leftToRight;
}

void HealthBar::setGeometry(int x, int y, int w, int h)
{
    this->geometry = {(Sint16)x, (Sint16)y, (Uint16)w, (Uint16)h};
    this->geometry2 = {(Sint16)(x+1), (Sint16)(y+1), (Uint16)(w-2), (Uint16)(h-2)};
}

void HealthBar::setMax(int max)
{
    this->max = max;
}

void HealthBar::setCurrent(int current)
{
    this->current = current;
}

void HealthBar::draw(SDL_Surface *dst)
{
    if (max == 0) {
        return;
    }
    Uint16 width = geometry2.w * current / max;
    SDL_Rect r1;
    SDL_Rect r2;
    if (leftToRight) {
        r1 = {(Sint16)(geometry2.x), (Sint16)(geometry2.y), (Uint16)(width), (Uint16)(geometry2.h)};
        r2 = {(Sint16)(geometry2.x+width), (Sint16)(geometry2.y), (Uint16)(geometry2.w-width), (Uint16)(geometry2.h)};
    } else {
        r1 = {(Sint16)(geometry2.x+geometry2.w-width), (Sint16)(geometry2.y), (Uint16)(width), (Uint16)(geometry2.h)};
        r2 = {(Sint16)(geometry2.x), (Sint16)(geometry2.y), (Uint16)(geometry2.w-width), (Uint16)(geometry2.h)};
    }
    SDL_FillRect(dst, &geometry, SDL_MapRGB(dst->format, 213, 213, 213));
    SDL_FillRect(dst, &r1, SDL_MapRGB(dst->format, 255, 240, 0));
    SDL_FillRect(dst, &r2, SDL_MapRGB(dst->format, 194, 0, 22));

}

void HealthBar::update(Uint32 frameStamp)
{
}



}

