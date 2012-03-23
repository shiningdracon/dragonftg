#ifndef _KEYFILTER_H_
#define _KEYFILTER_H_

#include <list>
#include <SDL/SDL.h>

using std::list;

namespace dragonfighting {

const size_t KEY_BUFFER_LEN = 128;


class Command {
public:
    char name[16];
    unsigned char ftgKeyArray[16];
    size_t length;

    Command(const char *name, const unsigned char *cmd, size_t length);
};


class KeyFilter {

    struct FTGKeyNode {
        unsigned char ftgkey;
        Uint32 numFrames;
    };
protected:
    list<Command*> commandTable;
    struct FTGKeyNode ftgKeyStateBuffer[KEY_BUFFER_LEN];
    int ftgKeyCurIndex;
    int ftgKeyPreIndex;
    int beginIndex;
    char curCommandName[16];

public:
    KeyFilter();
    ~KeyFilter();

    void addCommand(const char *name, const unsigned char *cmd, size_t length);

    void updateKeys(unsigned char currentFtgKeyState);//every frame
    bool pollCurrentCommandName(char *cmdname, size_t bufflen);
    Uint32 getKeyState(unsigned char key);

    // ugly way to handle charactor direction change
    void flipHorizontal();
};

}

#endif
