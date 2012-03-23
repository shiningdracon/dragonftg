#ifndef _WIDGET_H_
#define _WIDGET_H_

#include <SDL/SDL.h>

namespace dragonfighting {

class Widget
{
    public:
        Widget();
        virtual ~Widget() {};
        virtual void draw(SDL_Surface *dst) = 0;
        virtual void update(Uint32 frameStamp) = 0;

        virtual void setPosition(int x, int y);
        virtual void setPosition(SDL_Rect point);
        SDL_Rect getPositionScreenCoor();
        void setLevel(Sint16 val);
        Sint16 getLevel();
        inline void addBrotherBefore(Widget *node);
        inline void addBrotherAfter(Widget *node);
        void addChild(Widget *child);
        void splitFromParent();
        void breakTree();
        static void wtree_levelorder_traversal(Widget *node,
                void (*callback)(Widget *node, void *param, void * param2),
                void *param, void *param2);
        static void wtree_postorder_traversal(Widget *node,
                void (*callback)(Widget *node, void *param, void * param2),
                void *param, void *param2);

    protected:
        SDL_Rect position;

    private:
        Widget *parent;
        Widget *left_child;
        Widget *next_brother;
        Widget *prio_brother;

        Sint16      level;          //display order
};




}

#endif
