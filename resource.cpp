#include <libxml/tree.h>
#include <SDL/SDL_image.h>
#include <assert.h>
#include "resource.h"

namespace dragonfighting {

Sprite *SpriteFactory::loadSprite(const char *basedir, const char *spritename)
{
    char animationFilename[256];
    char collisionFilename[256];
    Sprite *sprite = new Sprite();
    if (sprite == NULL) {
        return NULL;
    }

    snprintf(animationFilename, sizeof(animationFilename), "%s.xml", spritename);
    snprintf(collisionFilename, sizeof(collisionFilename), "%s_c.xml", spritename);
    try {
        loadSpriteAnimation(sprite, basedir, animationFilename);
        loadSpriteCollision(sprite, basedir, collisionFilename);
    } catch (const char *e) {
        fprintf(stderr, "Error: %s\n", e);
        freeSprite(sprite);
        return NULL;
    }

    return sprite;
}

void SpriteFactory::freeSprite(Sprite *sprite)
{
    if (sprite->getFullImage() != NULL) {
        SDL_FreeSurface(sprite->getFullImage());
    }
    delete sprite;
}

void SpriteFactory::loadSpriteAnimation(Sprite *sprite, const char *basedir, const char *filename)
{
    char filepathbuff[2048];

    xmlDoc         *doc = NULL;
    xmlNode        *rootnode = NULL;
    xmlNode        *curanimation = NULL;
    xmlNode        *curcut = NULL;
    xmlChar        *text = NULL;

    snprintf(filepathbuff, sizeof(filepathbuff), "%s/%s", basedir, filename);
    doc = xmlReadFile(filepathbuff, NULL, 0);
    if (doc == NULL) {
        fprintf(stderr, "Unable to open %s\n", filepathbuff);
        throw "Unable to open";
    }

    rootnode = xmlDocGetRootElement(doc);

    text = xmlGetProp(rootnode, (const xmlChar*)"image");
    if (text == NULL) {
        fprintf(stderr, "Unable to find image attribute\n");
        throw "Unable to find image attribute";
    }
    snprintf(filepathbuff, sizeof(filepathbuff), "%s/%s", basedir, text);
    SDL_Surface *imgloaded = IMG_Load((const char *)filepathbuff);
    if (imgloaded == NULL) {
        fprintf(stderr, "Unable to open %s\n", text);
        throw "Unable to open image file";
    }
    xmlFree(text);

    text = xmlGetProp(rootnode, BAD_CAST "transparentColor");
    Uint32 colorkey = 0;
    if (text) {
        sscanf((const char *)text, "%x", &colorkey);
        xmlFree(text);
    }

    SDL_Surface *pImage = SDL_DisplayFormat(imgloaded);
    SDL_SetColorKey( pImage, SDL_SRCCOLORKEY, colorkey );
    SDL_FreeSurface(imgloaded);
    sprite->setFullImage(pImage);

    int frameSum = 0;
    curanimation = rootnode->xmlChildrenNode;
    while (curanimation != NULL) {
        int delay = 0;
        char title[32];
        int indexs[64];
        if (xmlStrcmp(curanimation->name, BAD_CAST "animation") != 0) {
            curanimation = curanimation->next;
            continue;
        }

        text = xmlGetProp(curanimation, BAD_CAST "delay");
        if (text) {
            delay = atoi((const char *)text);
            // translate from msec to frame
            delay = delay * 60 / 1000;
            xmlFree(text);
        }
        text = xmlGetProp(curanimation, BAD_CAST "title");
        if (text) {
            strncpy(title, (const char *)text, sizeof(title));
            xmlFree(text);
        }

        int curCutNum = 0;
        curcut = curanimation->xmlChildrenNode;
        while (curcut != NULL) {
            SDL_Rect rect = {0,0,0,0};
            SDL_Rect anchorpoint = {0,0,0,0};

            if (xmlStrcmp(curcut->name, BAD_CAST "cut") != 0) {
                curcut = curcut->next;
                continue;
            }

            text = xmlGetProp(curcut, BAD_CAST "x");
            if (text) {
                rect.x = atoi((const char *)text);
                xmlFree(text);
            }
            text = xmlGetProp(curcut, BAD_CAST "y");
            if (text) {
                rect.y = atoi((const char *)text);
                xmlFree(text);
            }
            text = xmlGetProp(curcut, BAD_CAST "w");
            if (text) {
                rect.w = atoi((const char *)text);
                xmlFree(text);
            }
            text = xmlGetProp(curcut, BAD_CAST "h");
            if (text) {
                rect.h = atoi((const char *)text);
                xmlFree(text);
            }
            text = xmlGetProp(curcut, BAD_CAST "anchor_x");
            if (text) {
                anchorpoint.x = atoi((const char *)text);
                xmlFree(text);
            }
            text = xmlGetProp(curcut, BAD_CAST "anchor_y");
            if (text) {
                anchorpoint.y = atoi((const char *)text);
                xmlFree(text);
            }

            sprite->addFrame(rect, anchorpoint);
            indexs[curCutNum] = frameSum;
            frameSum ++;
            curCutNum ++;
            curcut = curcut->next;
        }

        sprite->addSequence(title, delay, AnimationSequence::LOOP, indexs, curCutNum);
        curanimation = curanimation->next;
    }

    xmlFreeDoc(doc);

    xmlCleanupParser();
}

void SpriteFactory::loadSpriteCollision(Sprite *sprite, const char *basedir, const char *filename)
{
    enum {
        NONE,
        HITRECT,
        ATTACKRECT,
    };
    char filepathbuff[2048];

    xmlDoc         *doc = NULL;
    xmlNode        *rootnode = NULL;
    xmlNode        *curseq = NULL;
    xmlNode        *curframe = NULL;
    xmlNode        *currect = NULL;
    xmlChar        *text = NULL;

    snprintf(filepathbuff, sizeof(filepathbuff), "%s/%s", basedir, filename);
    doc = xmlReadFile(filepathbuff, NULL, 0);
    if (doc == NULL) {
        fprintf(stderr, "Unable to open %s\n", filepathbuff);
        throw "Unable to open";
    }

    rootnode = xmlDocGetRootElement(doc);

    int frameSum = 0;
    curseq = rootnode->xmlChildrenNode;
    while (curseq != NULL) {
        if (xmlStrcmp(curseq->name, BAD_CAST "sequence") != 0) {
            curseq = curseq->next;
            continue;
        }

        int delay = 0;
        char title[32];
        int indexs[64];

        text = xmlGetProp(curseq, BAD_CAST "delay");
        if (text) {
            delay = atoi((const char *)text);
            // translate from msec to frame
            delay = delay * 60 / 1000;
            xmlFree(text);
        }
        text = xmlGetProp(curseq, BAD_CAST "title");
        if (text) {
            strncpy(title, (const char *)text, sizeof(title));
            xmlFree(text);
        }

        int curFrameNum = 0;
        curframe = curseq->xmlChildrenNode;
        while (curframe != NULL) {
            if (xmlStrcmp(curframe->name, BAD_CAST "frame") != 0) {
                curframe = curframe->next;
                continue;
            }

            SDL_Rect hitrects[30];
            SDL_Rect attackrects[30];
            int curHitRectNum = 0;
            int curAttackRectNum = 0;
            currect = curframe->xmlChildrenNode;
            while (currect != NULL) {
                if (xmlStrcmp(currect->name, BAD_CAST "rect") != 0) {
                    currect = currect->next;
                    continue;
                }

                int recttype = NONE;

                text = xmlGetProp(currect, BAD_CAST "type");
                if (text) {
                    recttype = (xmlStrcmp(BAD_CAST "hit", text) == 0 ? HITRECT : ATTACKRECT);
                    xmlFree(text);
                } else {
                    throw "Node rect missing attribute: type";
                }

                if (recttype == HITRECT) {
                    text = xmlGetProp(currect, BAD_CAST "x");
                    if (text) {
                        hitrects[curHitRectNum].x = atoi((const char *)text);
                        xmlFree(text);
                    }
                    text = xmlGetProp(currect, BAD_CAST "y");
                    if (text) {
                        hitrects[curHitRectNum].y = atoi((const char *)text);
                        xmlFree(text);
                    }
                    text = xmlGetProp(currect, BAD_CAST "w");
                    if (text) {
                        hitrects[curHitRectNum].w = atoi((const char *)text);
                        xmlFree(text);
                    }
                    text = xmlGetProp(currect, BAD_CAST "h");
                    if (text) {
                        hitrects[curHitRectNum].h = atoi((const char *)text);
                        xmlFree(text);
                    }
                    curHitRectNum ++;
                } else if (recttype == ATTACKRECT) {
                    text = xmlGetProp(currect, BAD_CAST "x");
                    if (text) {
                        attackrects[curAttackRectNum].x = atoi((const char *)text);
                        xmlFree(text);
                    }
                    text = xmlGetProp(currect, BAD_CAST "y");
                    if (text) {
                        attackrects[curAttackRectNum].y = atoi((const char *)text);
                        xmlFree(text);
                    }
                    text = xmlGetProp(currect, BAD_CAST "w");
                    if (text) {
                        attackrects[curAttackRectNum].w = atoi((const char *)text);
                        xmlFree(text);
                    }
                    text = xmlGetProp(currect, BAD_CAST "h");
                    if (text) {
                        attackrects[curAttackRectNum].h = atoi((const char *)text);
                        xmlFree(text);
                    }
                    curAttackRectNum ++;
                }

                currect = currect->next;
            }

            sprite->addCollisionRects(hitrects, curHitRectNum, attackrects, curAttackRectNum);
            indexs[curFrameNum] = frameSum;
            frameSum ++;
            curFrameNum ++;
            curframe = curframe->next;
        }

        sprite->addCollisionSequence(title, delay, indexs, curFrameNum);
        curseq = curseq->next;
    }

    xmlFreeDoc(doc);

    xmlCleanupParser();
}

}

