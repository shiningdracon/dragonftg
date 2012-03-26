#include <SDL/SDL_image.h>
#include <math.h>
#include "stage.h"

namespace dragonfighting {

Stage::Stage(Sprite *player1, Sprite *player2) :
    player1(player1),
    player2(player2),
    groundline(0),
    bkImage(NULL),
    bkRect(),
    p1Health(0),
    p2Health(0),
    healthbarP1(),
    healthbarP2()
{
    addChild(player1);
    addChild(player2);

    player1->setPosition(275.0f, 200.0f);
    player1->setFacing(Character::RIGHT);
    player1->setFlipHorizontal(true);

    player2->setPosition(475.0f, 200.0f);
    player2->setFacing(Character::LEFT);
    player2->setFlipHorizontal(false);

    groundline = 200.0f;

    // Background
    SDL_Surface *imgloaded = IMG_Load("./data/stage2.png");
    bkImage = SDL_DisplayFormat(imgloaded);
    SDL_FreeSurface(imgloaded);
    bkRect = {0, 0, 750, 224};

    // Health bar
    p1Health = 10000;
    healthbarP1.setGeometry(4, 4, 190, 15);
    healthbarP1.setMax(p1Health);
    healthbarP1.setCurrent(p1Health);
    healthbarP1.setLeftToRight(false);
    p2Health = 10000;
    healthbarP2.setGeometry(226, 4, 190, 15);
    healthbarP2.setMax(p2Health);
    healthbarP2.setCurrent(p2Health);
}

Stage::~Stage()
{
    SDL_FreeSurface(bkImage);
}

void Stage::update(Uint32 frameStamp)
{
    player1->update(frameStamp);
    player2->update(frameStamp);

    float p1vx = player1->getVelocityX();
    float p1vy = player1->getVelocityY();
    float p1x = player1->getPositionX();
    float p1y = player1->getPositionY();
    float p2vx = player2->getVelocityX();
    float p2vy = player2->getVelocityY();
    float p2x = player2->getPositionX();
    float p2y = player2->getPositionY();

    int edgeleft = std::max(p1x, p2x) - 380;
    int edgeright = std::min(p1x, p2x) + 380;
    edgeleft = std::max(edgeleft, 20);
    edgeright = std::min(edgeright, 730);

    int rectsize1 = 0, rectsize2 = 0;
    const SDL_Rect *area1 = NULL, *area2 = NULL;
    bool p1hit = false, p2hit = false;

    if (player1->getState() == Character::STAND || player1->getState() == Character::WALK) {
        if (p1x > p2x) {
            player1->setFacing(Character::LEFT);
            player1->setFlipHorizontal(false);
        } else if (p1x < p2x) {
            player1->setFacing(Character::RIGHT);
            player1->setFlipHorizontal(true);
        }
    }
    if (player2->getState() == Character::STAND || player2->getState() == Character::WALK) {
        if (p2x > p1x) {
            player2->setFacing(Character::LEFT);
            player2->setFlipHorizontal(false);
        } else if (p2x < p1x) {
            player2->setFacing(Character::RIGHT);
            player2->setFlipHorizontal(true);
        }
    }

    if (!player2->isInvincible()) {
        area1 = player1->getCurRealAttackCollisionRects(rectsize1);
        area2 = player2->getCurRealHitCollisionRects(rectsize2);
        p2hit = areaCollide(area1, rectsize1, area2, rectsize2);
    }

    if (!player1->isInvincible()) {
        area1 = player1->getCurRealHitCollisionRects(rectsize1);
        area2 = player2->getCurRealAttackCollisionRects(rectsize2);
        p1hit = areaCollide(area1, rectsize1, area2, rectsize2);
    }

    if (p1hit && !player1->isGuard()) {
        p1Health -= 1000;
        if (p1Health < 0) p1Health = 0;
        player1->underAttack( (player2->getState() == Character::ATTACK || player2->getState() == Character::JUMPATTACK) ? Character::HitType::NORMAL : Character::HitType::THUMP);
        healthbarP1.setCurrent(p1Health);
        printf("p1hit\n");
    }
    if (p2hit && !player2->isGuard()) {
        p2Health -= 1000;
        if (p2Health < 0) p2Health = 0;
        player2->underAttack( (player1->getState() == Character::ATTACK || player1->getState() == Character::JUMPATTACK) ? Character::HitType::NORMAL : Character::HitType::THUMP);
        healthbarP2.setCurrent(p2Health);
        printf("p2hit\n");
    }

    if ((player1->getState() == Character::STAND || player1->getState() == Character::WALK) && (player2->getState() == Character::STAND || player2->getState() == Character::WALK)) {
        if (fabsf(p1x - p2x) < 40) {
            if (p1x > p2x) {
                p1vx = 1;
                p2vx = -1;
            } else {
                p1vx = -1;
                p2vx = 1;
            }
            /*if (abs(p1vx) < 0.001 && abs(p2vx) < 0.001) {
                if (p1x > p2x) {
                    p1vx = 1;
                    p2vx = -1;
                } else {
                    p1vx = -1;
                    p2vx = 1;
                }
            } else if (p1x < p2x && p1vx > 0 && p2vx < 0) {
                p1vx = 0;
                p2vx = 0;
            } else if (p1x > p2x && p1vx < 0 && p2vx > 0) {
                p1vx = 0;
                p2vx = 0;
            } else if (p1x < p2x && p1vx > 0 && p2vx >= 0) {
                p1vx = 1;
                if (p2vx < 0.001) {
                    p2vx = 1;
                }
            } else if (p1x > p2x && p1vx < 0 && p2vx <= 0) {
                p1vx = -1;
                if (p2vx > -0.001) {
                    p2vx = -1;
                }
            } else if (p2x < p1x && p2vx > 0 && p1vx >= 0) {
                p2vx = 1;
                if (p1vx < 0.001) {
                    p1vx = 1;
                }
            } else if (p2x > p1x && p2vx < 0 && p1vx <= 0) {
                p2vx = -1;
                if (p1vx > -0.001) {
                    p1vx = -1;
                }
            }*/
        }
    }
    p1x += p1vx;
    p2x += p2vx;
    p1y += p1vy;
    p2y += p2vy;

    if (p1x < 20) {
        p1x = 20;
    }
    if (p1x > 730) {
        p1x = 730;
    }
    if (p2x < 20) {
        p2x = 20;
    }
    if (p2x > 730) {
        p2x = 730;
    }
    if (p1x < edgeleft) {
        p1x = edgeleft;
    }
    if (p1x > edgeright) {
        p1x = edgeright;
    }
    if (p2x < edgeleft) {
        p2x = edgeleft;
    }
    if (p2x > edgeright) {
        p2x = edgeright;
    }

    if (p1y > groundline) {
        p1y = groundline;
        player1->hitGround();
    }
    if (p2y > groundline) {
        p2y = groundline;
        player2->hitGround();
    }

    player1->setPosition(p1x, p1y);
    player2->setPosition(p2x, p2y);

    // scroll
    int p1ScreenX = player1->getPositionScreenCoor().x;
    int p2ScreenX = player2->getPositionScreenCoor().x;
    int distance = abs(p1ScreenX - p2ScreenX);

    if (distance < 240) {
        if (p1ScreenX < 90) {
            position.x = (position.x + 90 - p1ScreenX);
        }
        if (p1ScreenX > 330) {
            position.x = (position.x + 330 - p1ScreenX);
        }
        if (p2ScreenX < 90) {
            position.x = (position.x + 90 - p2ScreenX);
        }
        if (p2ScreenX > 330) {
            position.x = (position.x + 330 - p2ScreenX);
        }
    } else if (distance <= 380) {
        position.x = - ((p1x + p2x) / 2 - 210);
    } else {
        printf("Can't be here!\n");
    }

    if (position.x > 0) {
        position.x = 0;
    }
    if (position.x < -330) {
        position.x = -330;
    }
}

void Stage::draw(SDL_Surface *dst)
{
    SDL_Rect screenposition = getPositionScreenCoor();
    SDL_BlitSurface(bkImage, &bkRect, dst, &screenposition);
    player1->draw(dst);
    player2->draw(dst);
    healthbarP1.draw(dst);
    healthbarP2.draw(dst);
}


}

