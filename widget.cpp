#include <list>
#include "widget.h"

using std::list;
namespace dragonfighting {

Widget::Widget() :
    parent(NULL),
    left_child(NULL),
    next_brother(NULL),
    prio_brother(NULL),
    level(0)
{
    position = {0, 0, 0, 0};
}

void Widget::setLevel(Sint16 val)
{
    if (level == val) {
        return;
    }
    this->level = val;
    Widget *ptr = NULL;
    if (next_brother != NULL && next_brother->getLevel() < level) {
        ptr = next_brother;
        while (ptr != NULL) {
            if (ptr->getLevel() >= level) {
                splitFromParent();
                ptr->addBrotherBefore(this);
                return;
            }
            ptr = ptr->next_brother;
        }
    }

    if (prio_brother != NULL && prio_brother->getLevel() > level) {
        ptr = prio_brother;
        while (ptr != NULL) {
            if (ptr->getLevel() <= level) {
                splitFromParent();
                ptr->addBrotherAfter(this);
                return;
            }
            ptr = ptr->prio_brother;
        }
    }
}

void Widget::setPosition(int x, int y)
{
    position.x = x;
    position.y = y;
}

void Widget::setPosition(SDL_Rect point)
{
    setPosition(point.x, point.y);
}

SDL_Rect Widget::getPositionScreenCoor()
{
    SDL_Rect screenposition = position;
    SDL_Rect parentposition = {0, 0, 0, 0};
    if (parent != NULL) {
        parentposition = parent->getPositionScreenCoor();
        screenposition.x += parentposition.x;
        screenposition.y += parentposition.y;
    }
    return screenposition;
}

Sint16 Widget::getLevel()
{
    return level;
}

void Widget::addBrotherBefore(Widget *node)
{
    node->next_brother = this;
    node->prio_brother = this->prio_brother;
    if (this->prio_brother != NULL) {
        this->prio_brother->next_brother = node;
    }
    this->prio_brother = node;
    node->parent = this->parent;
    if (this->parent != NULL) {
        if (this->parent->left_child == this) {
            this->parent->left_child = node;
        }
    }
}

void Widget::addBrotherAfter(Widget *node)
{
    node->next_brother = this->next_brother;
    node->prio_brother = this;
    if (this->next_brother != NULL) {
        this->next_brother->prio_brother = node;
    }
    this->next_brother = node;
    node->parent = this->parent;
}

void Widget::addChild(Widget *child)
{
    Widget *ptr = NULL;

    child->parent = this;
    if (this->left_child == NULL) {
        this->left_child = child;
        return;
    }

    //already have children, find a right place to insert (depend on level)
    ptr = this->left_child;
    while (child->getLevel() >= ptr->getLevel()) {
        if (ptr->next_brother != NULL) {
            ptr = ptr->next_brother;
        } else {
            break;
        }
    }

    if (child->getLevel() < ptr->getLevel()) {
        //add child before ptr
        ptr->addBrotherBefore(child);
    } else {
        //add child as the last child of the list
        ptr->addBrotherAfter(child);
    }
}

void Widget::splitFromParent()
{
    if (this == NULL) {
        return;
    }
    if (this->parent != NULL) {
        if (this->parent->left_child == this) {
            //this is the first child of its parent
            this->parent->left_child = this->next_brother;
        }
        this->parent = NULL;
    }
    if (this->next_brother != NULL) {
        this->next_brother->prio_brother = this->prio_brother;
        this->next_brother = NULL;
    }
    if (this->prio_brother != NULL) {
        this->prio_brother->next_brother = this->next_brother;
        this->prio_brother = NULL;
    }
}

static inline void cb_break_tree(Widget *node, void *param, void *param2)
{
    list<Widget *> *visitlist = (list<Widget *> *)param;
    visitlist->push_back(node);
}

/**
 * split current tree from parent and break this tree
 */

void Widget::breakTree()
{
    list<Widget *> visitlist;
    Widget *n = NULL;

    this->splitFromParent();
    wtree_levelorder_traversal(this, cb_break_tree, &visitlist, NULL);
    while (!visitlist.empty()) {
        n = visitlist.back();
        visitlist.pop_back();
        n->left_child = NULL;
        n->next_brother = NULL;
        n->prio_brother = NULL;
        n->parent = NULL;
    }
}

void Widget::wtree_levelorder_traversal(Widget *node,
        void (*callback)(Widget *node, void *param, void * param2),
        void *param, void *param2)
{
    list<Widget *> nodelist;
    Widget *n = NULL;

    nodelist.push_front(node);
    while (!nodelist.empty()) {
        n = nodelist.front();
        nodelist.pop_front();
        if (callback != NULL) {
            //visit node
            callback(n, param, param2);
        }
        if (n->left_child != NULL) {
            nodelist.push_back(n->left_child);
        }
        if (n->next_brother != NULL) {
            nodelist.push_front(n->next_brother);
        }
    }
}

//TODO: not tested
void Widget::wtree_postorder_traversal(Widget *node,
        void (*callback)(Widget *node, void *param, void * param2),
        void *param, void *param2)
{
    bool backtracking = false;
    list<Widget *> nodelist;
    Widget *n = NULL;

    nodelist.push_front(node);
    while (!nodelist.empty()) {
        n = nodelist.front();
        if (n->left_child != NULL && !backtracking) {
            nodelist.push_front(n->left_child);
            continue;
        }

        if (callback != NULL) {
            //visit node
            callback(n, param, param2);
        }
        nodelist.pop_front();

        if (n->next_brother != NULL) {
            nodelist.push_front(n->next_brother);
            backtracking = false;
            continue;
        }

        backtracking = true;
    }
}


}

