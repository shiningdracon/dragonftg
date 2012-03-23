#include <assert.h>
#include "keyfilter.h"
#include "ftgkeys.h"

namespace dragonfighting {

Command::Command(const char *name, const unsigned char *cmd, size_t length)
{
    assert(length <= 16);
    unsigned int i=0;
    for (i=0; i<sizeof(this->name)-1 && name[i]!='\0'; i++) {
        this->name[i] = name[i];
    }
    this->name[i] = '\0';
    for (i=0; i<length; i++) {
        this->ftgKeyArray[i] = cmd[i];
    }
    this->length = length;
}

static bool commandCompare(Command *c1, Command *c2)
{
    if (c1->length == c2->length) {
        // because: 3A must be parsed before 2A or 6A
        return c1->ftgKeyArray[c1->length-2] > c2->ftgKeyArray[c2->length-2];
    } else {
        return c1->length > c2->length;
    }
}

KeyFilter::KeyFilter() :
    commandTable(),
    ftgKeyCurIndex(0),
    ftgKeyPreIndex(sizeof(ftgKeyStateBuffer) - 1),
    beginIndex(0)
{
    memset(ftgKeyStateBuffer, 0, sizeof(ftgKeyStateBuffer));
    memset(curCommandName, 0, sizeof(curCommandName));
}

KeyFilter::~KeyFilter()
{
    for (list<Command*>::iterator i = commandTable.begin(); i != commandTable.end(); ++i ){
        delete *i;
    }
}

void KeyFilter::addCommand(const char *name, const unsigned char *cmd, size_t length)
{
    if (length < 2) {
        throw "command length must at lest 2";
    }
    Command *command = new Command(name, cmd, length);
    commandTable.push_back(command);
    commandTable.sort(commandCompare);

    //debug
    /*
    for (list<Command*>::iterator i = commandTable.begin(); i != commandTable.end(); ++i ){
        printf("%s\n", (*i)->name);
    }
    printf("--------------\n");
    */
}

void KeyFilter::updateKeys(unsigned char currentFtgKeyState)
{
    // save key in buffer
    if (ftgKeyStateBuffer[ftgKeyCurIndex].ftgkey == currentFtgKeyState) {
        ftgKeyStateBuffer[ftgKeyCurIndex].numFrames += 1;
    } else {
        ftgKeyPreIndex = ftgKeyCurIndex;
        ftgKeyCurIndex = (ftgKeyCurIndex + 1) % KEY_BUFFER_LEN;
        ftgKeyStateBuffer[ftgKeyCurIndex].ftgkey = currentFtgKeyState;
        ftgKeyStateBuffer[ftgKeyCurIndex].numFrames = 1;
    }

    // if not a 'command end', skip parse
    // A 'command end' is: current is a button key and pre is not the same button key
    if ((ftgKeyStateBuffer[ftgKeyCurIndex].ftgkey & FTG_BUTTON_KEYS) == 0) {
        return;
    } else if ((ftgKeyStateBuffer[ftgKeyPreIndex].ftgkey & ftgKeyStateBuffer[ftgKeyCurIndex].ftgkey & FTG_BUTTON_KEYS) != 0) {
        return;
    }

    // parse command
    list<Command*>::iterator i;
    for (i = commandTable.begin(); i != commandTable.end(); ++i ){
        int j;
        int temp_p_cur = ftgKeyCurIndex;
        for (j=(*i)->length-1; j>=0; j--) {
            if (temp_p_cur == beginIndex) {
                break;
            }

            // faulttolerant
            if (ftgKeyStateBuffer[temp_p_cur].ftgkey == 0 && ftgKeyStateBuffer[temp_p_cur].numFrames < 8) {
                temp_p_cur--;
                if (temp_p_cur == -1)
                    temp_p_cur += KEY_BUFFER_LEN;
            }

            // if interval too long
            if (j != 0 && ftgKeyStateBuffer[temp_p_cur].numFrames > 8) {
                break;
            }

            if (j == (int)(*i)->length-1) {
                // if 'command end' mismatch
                if ( (ftgKeyStateBuffer[temp_p_cur].ftgkey & FTG_BUTTON_KEYS) != (*i)->ftgKeyArray[j] ) {
                    break;
                }
            } else {
                bool passed = true;
                int loop = 2;
                // test if mismatch
                while ( loop > 0 && (ftgKeyStateBuffer[temp_p_cur].ftgkey != (*i)->ftgKeyArray[j]) ) {
                    // faulttolerant
                    unsigned char keytest = (*i)->ftgKeyArray[j] & ftgKeyStateBuffer[temp_p_cur].ftgkey;
                    if ( keytest != 0 && (keytest == ftgKeyStateBuffer[temp_p_cur].ftgkey || keytest == (*i)->ftgKeyArray[j]) ) {
                        // skip one key and test again
                        temp_p_cur--;
                        if (temp_p_cur == -1) {
                            temp_p_cur += KEY_BUFFER_LEN;
                        }
                        loop --;
                    } else {
                        passed = false;
                        break;
                    }
                }
                if (!passed) {
                    break;
                }
            }

            temp_p_cur--;
            if (temp_p_cur == -1) {
                temp_p_cur += KEY_BUFFER_LEN;
            }
        }
        if (j < 0) {
            break;
        }
    }
    if (i != commandTable.end()) {
        strncpy(curCommandName, (*i)->name, sizeof(curCommandName));
        beginIndex = ftgKeyCurIndex;

        //debug
        
        static int hitnumber = 0;
        printf("%s hit   %d\n", (*i)->name, hitnumber++);
        /*int i;
        for (i=0; i<KEY_BUFFER_LEN; i++) {
            if (i == ftgKeyCurIndex) {
                printf("*");
            }
            printf("%d,", ftgKeyStateBuffer[i].ftgkey);
        }
        printf("\n");
*/ 
    }
}

bool KeyFilter::pollCurrentCommandName(char *cmdname, size_t bufflen)
{
    assert(cmdname != NULL && bufflen > 0);
    if (curCommandName[0] != '\0') {
        strncpy(cmdname, curCommandName, bufflen);
        memset(curCommandName, 0, sizeof(curCommandName));
        return true;
    } else {
        return false;
    }
}

Uint32 KeyFilter::getKeyState(unsigned char key)
{
    if ( (ftgKeyStateBuffer[ftgKeyCurIndex].ftgkey & key) != 0 ) {
        return ftgKeyStateBuffer[ftgKeyCurIndex].numFrames;
    }
    return 0;
}

void KeyFilter::flipHorizontal()
{
    bool key1down = false, key2down = false;
    if ( (ftgKeyStateBuffer[ftgKeyCurIndex].ftgkey & FTGKEY_6) != 0 ) {
        key1down = true;
    }
    if ( (ftgKeyStateBuffer[ftgKeyCurIndex].ftgkey & FTGKEY_4) != 0 ) {
        key2down = true;
    }

    if (key1down) {
        ftgKeyStateBuffer[ftgKeyCurIndex].ftgkey |= FTGKEY_4;
    } else {
        ftgKeyStateBuffer[ftgKeyCurIndex].ftgkey &= ~FTGKEY_4;
    }
    if (key2down) {
        ftgKeyStateBuffer[ftgKeyCurIndex].ftgkey |= FTGKEY_6;
    } else {
        ftgKeyStateBuffer[ftgKeyCurIndex].ftgkey &= ~FTGKEY_6;
    }
}


}


#ifdef FTG_TEST

#include <stdlib.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <math.h>

#include "keyfilter.h"

using namespace dragonfighting;

int main(int argc, char **argv)
{
    int exited = 0;
    SDL_Surface *screen = NULL;
    Uint32 interval = 1000/60;
    Uint32 delay = 0;
    Uint32 frame = 0;
    Uint32 oldtime = SDL_GetTicks();
    Uint32 newtime = oldtime;
    SDL_Init(SDL_INIT_EVERYTHING);
    atexit(SDL_Quit);

    screen = SDL_SetVideoMode(640, 480, 32, SDL_SWSURFACE);
    if (screen == NULL)
    {
        fprintf(stderr, "error set video mode.\n");
        exit(1);
    }

    // Init key map
    unsigned char key_map[SDLK_LAST];
    memset(key_map, 0, sizeof(key_map));
    key_map[SDLK_w] = FTGKEY_8;
    key_map[SDLK_s] = FTGKEY_2;
    key_map[SDLK_a] = FTGKEY_4;
    key_map[SDLK_d] = FTGKEY_6;
    key_map[SDLK_j] = FTGKEY_A;
    key_map[SDLK_k] = FTGKEY_B;
    key_map[SDLK_l] = FTGKEY_C;
    key_map[SDLK_i] = FTGKEY_D;

    // Init key filter
    KeyFilter keyFilter;
    unsigned char cmd[][8] = {
        {FTGKEY_6, FTGKEY_3, FTGKEY_2, FTGKEY_3, FTGKEY_A},
        {FTGKEY_2, FTGKEY_3, FTGKEY_6, FTGKEY_A},
        {FTGKEY_2, FTGKEY_3, FTGKEY_6, FTGKEY_B},
        {FTGKEY_2, FTGKEY_2, FTGKEY_A},
        {FTGKEY_6, FTGKEY_A},
        {FTGKEY_2, FTGKEY_A},
        {FTGKEY_6, FTGKEY_3, FTGKEY_2, FTGKEY_3, FTGKEY_6, FTGKEY_A},
        {FTGKEY_6, FTGKEY_2, FTGKEY_3, FTGKEY_A},
    };
    //keyFilter.addCommand("63236A", cmd[6], 6);
    keyFilter.addCommand("6323A", cmd[0], 5);
    //keyFilter.addCommand("623A", cmd[7], 4);
    keyFilter.addCommand("236A", cmd[1], 4);
    keyFilter.addCommand("236B", cmd[2], 4);
    keyFilter.addCommand("22A", cmd[3], 3);
    keyFilter.addCommand("6A", cmd[4], 2);
    keyFilter.addCommand("2A", cmd[5], 2);

    unsigned char current_key_state = 0;
    while(exited==0)
    {
        SDL_Event event;
        if (SDL_PollEvent(&event)==1){
            if((event.type==SDL_KEYDOWN && event.key.keysym.sym==SDLK_ESCAPE) || (event.type==SDL_QUIT)) exited=1;
            else if (event.type==SDL_KEYDOWN)
            {
                current_key_state |= key_map[event.key.keysym.sym];
            }
            else if (event.type==SDL_KEYUP)
            {
                current_key_state &= ~key_map[event.key.keysym.sym];
            }
        }

        keyFilter.updateKeys(current_key_state);

        SDL_FillRect( screen, NULL, 0 );
        //SDL_FillRect( screen, &rect, color );

        frame++;
        SDL_Flip(screen);

        newtime = SDL_GetTicks();
        if (newtime - oldtime < interval)
            SDL_Delay(interval - newtime + oldtime);
        oldtime = SDL_GetTicks();
    }

    SDL_Quit();
    return 0;
}


#endif
