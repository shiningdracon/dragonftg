#ifndef _ANIMATION_H_
#define _ANIMATION_H_

#include <SDL/SDL.h>
#include <vector>
#include <string>
#include "widget.h"

using std::vector;
using std::string;

namespace dragonfighting {

class AnimationSequence
{
public:
    enum AnimationStyle {
        LOOP,
        ONCE,
    };

protected:
    string name;
    Uint32 animRateFrame;
    int curIndex;
    bool ended;
    enum AnimationStyle playStyle;
    int *indexArray;
    int size;

public:
    AnimationSequence(const char *name);
    ~AnimationSequence();

    bool nameCompare(const char *name);
    void setFrameRate(Uint32 frame);
    Uint32 getFrameRate();
    void setPlayStyle(enum AnimationStyle style);
    enum AnimationStyle getPlayStyle();
    void setIndexArray(int indexarray[], int length);
    int getCurrentFrameIndex();
    int getNextFrameIndex();
    int getPrevFrameIndex();
    bool isEnd();
    void reset();

};

class Animation : public Widget
{
protected:
    SDL_Surface *fullImage;
    SDL_Surface *flipedFullImage;
    vector<SDL_Rect> frameRects;
    vector<SDL_Rect> frameAnchorPoints;
    vector<AnimationSequence *> sequences;
    int currentFrame;
    int currentSequence;
    int defaultSequence;
    Uint32 oldFrameStamp;
    bool flipHorizontal;
    bool backorder;

public:
    Animation();
    virtual ~Animation();
    void setFullImage(SDL_Surface *img);
    SDL_Surface *getFullImage();
    void addFrame(SDL_Rect rect, SDL_Rect anchorpoint);
    void addSequence(const char *name, Uint32 framerate, AnimationSequence::AnimationStyle style, int indexarray[], int length);
    void setFlipHorizontal(bool b);
    void setDefaultSequence(const char *name);
    SDL_Rect getCurAnchor();
    void playSequence(const char *name);
    void playSequenceBackorder(const char *name);
    virtual void update(Uint32 frameStamp);
    virtual void draw(SDL_Surface *dst);
};


}

#endif
