#include "collisiondetect.h"

bool rectCollide(SDL_Rect rect1, SDL_Rect rect2)
{
    int left1 = rect1.x;
    int right1 = rect1.x + rect1.w;
    int top1 = rect1.y;
    int bottom1 = rect1.y + rect1.h;
    int left2 = rect2.x;
    int right2 = rect2.x + rect2.w;
    int top2 = rect2.y;
    int bottom2 = rect2.y + rect2.h;

    //printf("left1=%d,right1=%d,top1=%d,bottom1=%d,left2=%d,right2=%d,top2=%d,bottom2=%d\n",left1,right1,top1,bottom1,left2,right2,top2,bottom2);
    if (bottom1 < top2) return false;
    if (top1 > bottom2) return false;
    if (right1 < left2) return false;
    if (left1 > right2) return false;

    return true;
}

bool areaCollide(const SDL_Rect *rects1, int size1, const SDL_Rect *rects2, int size2)
{
    if (rects1 != NULL && rects2 != NULL)
    {
        for (int i=0; i<size1; i++) {
            for (int j=0; j<size2; j++) {
                if (rectCollide(rects1[i], rects2[j])) {
                    return true;
                }
            }
        }
    }

    return false;
}

