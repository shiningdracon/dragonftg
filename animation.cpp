#include <assert.h>
#include <SDL/SDL.h>
#include "sprite.h"

namespace dragonfighting {

/*
 * Return the pixel value at (x, y)
 * NOTE: The surface must be locked before calling this!
 */
Uint32 getpixel(SDL_Surface *surface, int x, int y)
{
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to retrieve */
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch(bpp) {
    case 1:
        return *p;

    case 2:
        return *(Uint16 *)p;

    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
            return p[0] << 16 | p[1] << 8 | p[2];
        else
            return p[0] | p[1] << 8 | p[2] << 16;

    case 4:
        return *(Uint32 *)p;

    default:
        return 0;       /* shouldn't happen, but avoids warnings */
    }
}

/*
 * Set the pixel at (x, y) to the given value
 * NOTE: The surface must be locked before calling this!
 */
void putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to set */
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch(bpp) {
    case 1:
        *p = pixel;
        break;

    case 2:
        *(Uint16 *)p = pixel;
        break;

    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
            p[0] = (pixel >> 16) & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = pixel & 0xff;
        } else {
            p[0] = pixel & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = (pixel >> 16) & 0xff;
        }
        break;

    case 4:
        *(Uint32 *)p = pixel;
        break;
    }
}


AnimationSequence::AnimationSequence(const char *name) :
    name(name),
    animRateFrame(0),
    curIndex(0),
    ended(false),
    playStyle(ONCE),
    indexArray(NULL),
    size(0)
{
}

AnimationSequence::~AnimationSequence()
{
    if (indexArray != NULL) {
        free(indexArray);
    }
}

bool AnimationSequence::nameCompare(const char *name)
{
    return this->name.compare(name) == 0;
}

void AnimationSequence::setFrameRate(Uint32 frame)
{
    this->animRateFrame = frame;
}

Uint32 AnimationSequence::getFrameRate()
{
    return this->animRateFrame;
}

void AnimationSequence::setPlayStyle(enum AnimationStyle style)
{
    this->playStyle = style;
}

AnimationSequence::AnimationStyle AnimationSequence::getPlayStyle()
{
    return this->playStyle;
}

void AnimationSequence::setIndexArray(int indexarray[], int length)
{
    indexArray = (int *)malloc(sizeof(int) * length);
    assert(indexArray != NULL);
    memcpy(this->indexArray, indexarray, sizeof(int) * length);
    size = length;
}

int AnimationSequence::getCurrentFrameIndex()
{
    return indexArray[curIndex];
}

int AnimationSequence::getNextFrameIndex()
{
    curIndex ++;
    if (curIndex >= size) {
        switch (playStyle) {
            case ONCE:
                curIndex = size - 1;
                ended = true;
                break;
            case LOOP:
                curIndex = 0;
                break;
            default:
                break;
        }
    }
    return indexArray[curIndex];
}

int AnimationSequence::getPrevFrameIndex()
{
    curIndex --;
    if (curIndex < 0) {
        switch (playStyle) {
            case ONCE:
                curIndex = 0;
                ended = true;
                break;
            case LOOP:
                curIndex = size - 1;
                break;
            default:
                break;
        }
    }
    return indexArray[curIndex];
}

bool AnimationSequence::isEnd()
{
    return ended;
}

void AnimationSequence::reset()
{
    curIndex = 0;
    ended = false;
}


Animation::Animation() :
    fullImage(NULL),
    flipedFullImage(NULL),
    sequences(),
    currentFrame(0),
    currentSequence(-1),
    defaultSequence(-1),
    oldFrameStamp(0),
    flipHorizontal(false)
{
}

Animation::~Animation()
{
    for (vector<AnimationSequence*>::iterator i = sequences.begin(); i != sequences.end(); ++i ){
        delete *i;
    }
    // free fliped image
    if (flipedFullImage != NULL) {
        SDL_FreeSurface(flipedFullImage);
    }
}

void Animation::setFullImage(SDL_Surface *img)
{
    this->fullImage = img;
    if (flipedFullImage != NULL) {
        SDL_FreeSurface(flipedFullImage);
    }
    this->flipedFullImage = SDL_CreateRGBSurface(img->flags, img->w, img->h, img->format->BitsPerPixel,
        img->format->Rmask, img->format->Gmask, img->format->Bmask, img->format->Amask);
    SDL_LockSurface(img);
    SDL_LockSurface(flipedFullImage);
    for(int y = 0; y < img->h; y++) {
        for(int x = 0; x < img->w; x++) {
            putpixel(flipedFullImage, img->w - 1 - x, y, getpixel(img, x, y));
        }
    }
    SDL_UnlockSurface(img);
    SDL_UnlockSurface(flipedFullImage);

    flipedFullImage->flags = img->flags;
    flipedFullImage->format->colorkey = img->format->colorkey;
}

SDL_Surface *Animation::getFullImage()
{
    return fullImage;
}

void Animation::addFrame(SDL_Rect rect, SDL_Rect anchorpoint)
{
    frameRects.push_back(rect);
    frameAnchorPoints.push_back(anchorpoint);
}

void Animation::addSequence(const char *name, Uint32 framerate, AnimationSequence::AnimationStyle style, int indexarray[], int length)
{
    AnimationSequence *seq = new AnimationSequence(name);
    seq->setFrameRate(framerate);
    seq->setPlayStyle(style);
    for (int i=0; i<length; i++) {
        assert(indexarray[i] < (int)frameRects.size());
    }
    seq->setIndexArray(indexarray, length);
    this->sequences.push_back(seq);
}

void Animation::setFlipHorizontal(bool b)
{
    flipHorizontal = b;
}

void Animation::setDefaultSequence(const char *name)
{
    int index = 0;
    for (vector<AnimationSequence*>::iterator i = sequences.begin(); i != sequences.end(); ++i ){
        if ( (*i)->nameCompare(name) ) {
            defaultSequence = index;
            break;
        }
        index ++;
    }
}

SDL_Rect Animation::getCurAnchor()
{
    return frameAnchorPoints[currentFrame];
}

void Animation::playSequence(const char *name)
{
    int index = 0;
    for (vector<AnimationSequence*>::iterator i = sequences.begin(); i != sequences.end(); ++i ){
        if ( (*i)->nameCompare(name) ) {
            currentSequence = index;
            sequences[currentSequence]->reset();
            currentFrame = sequences[currentSequence]->getCurrentFrameIndex();
            oldFrameStamp = 0;
            break;
        }
        index ++;
    }
    backorder = false;
}

void Animation::playSequenceBackorder(const char *name)
{
    int index = 0;
    for (vector<AnimationSequence*>::iterator i = sequences.begin(); i != sequences.end(); ++i ){
        if ( (*i)->nameCompare(name) ) {
            currentSequence = index;
            sequences[currentSequence]->reset();
            currentFrame = sequences[currentSequence]->getPrevFrameIndex();
            break;
        }
        index ++;
    }
    backorder = true;
}

void Animation::update(Uint32 frameStamp)
{
    assert(currentSequence >= 0);
    if (oldFrameStamp == 0) {
        // is the first frame
        oldFrameStamp = frameStamp;
    } else if (oldFrameStamp + sequences[currentSequence]->getFrameRate() <= frameStamp) {
        oldFrameStamp = frameStamp;
        if (backorder) {
            currentFrame = sequences[currentSequence]->getPrevFrameIndex();
        } else {
            currentFrame = sequences[currentSequence]->getNextFrameIndex();
        }
        if (sequences[currentSequence]->isEnd()) {
            currentSequence = defaultSequence;
            sequences[currentSequence]->reset();
            currentFrame = sequences[currentSequence]->getCurrentFrameIndex();
        }
    }
}

void Animation::draw(SDL_Surface *dst)
{
    SDL_Rect screenposition = getPositionScreenCoor();
    if (flipHorizontal) {
        SDL_Rect srcrect = {(Sint16)(flipedFullImage->w - frameRects[currentFrame].x - frameRects[currentFrame].w),
                            frameRects[currentFrame].y,
                            frameRects[currentFrame].w,
                            frameRects[currentFrame].h};
        SDL_Rect dstrect = {(Sint16)(screenposition.x - frameRects[currentFrame].w + frameAnchorPoints[currentFrame].x),
            (Sint16)(screenposition.y - frameAnchorPoints[currentFrame].y), 0, 0};
        SDL_BlitSurface(flipedFullImage, &srcrect, dst, &dstrect);
    } else {
        SDL_Rect dstrect = {(Sint16)(screenposition.x - frameAnchorPoints[currentFrame].x),
            (Sint16)(screenposition.y - frameAnchorPoints[currentFrame].y), 0, 0};
        SDL_BlitSurface(fullImage, &frameRects[currentFrame], dst, &dstrect);
    }
}

}
