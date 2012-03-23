#ifndef _COLLISTION_DETECT_H_
#define _COLLISTION_DETECT_H_

#include <SDL/SDL.h>

bool rectCollide(SDL_Rect rect1, SDL_Rect rect2);
bool areaCollide(const SDL_Rect *rects1, int size1, const SDL_Rect *rects2, int size2);

#endif

