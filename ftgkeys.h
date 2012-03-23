#ifndef _FTGKEYS_H_
#define _FTGKEYS_H_

namespace dragonfighting {

const unsigned char FTGKEY_8 = 128;
const unsigned char FTGKEY_2 = 64;
const unsigned char FTGKEY_4 = 32;
const unsigned char FTGKEY_6 = 16;
const unsigned char FTGKEY_A = 8;
const unsigned char FTGKEY_B = 4;
const unsigned char FTGKEY_C = 2;
const unsigned char FTGKEY_D = 1;

const unsigned char FTGKEY_3 = FTGKEY_2 | FTGKEY_6;
const unsigned char FTGKEY_1 = FTGKEY_2 | FTGKEY_4;
const unsigned char FTGKEY_9 = FTGKEY_8 | FTGKEY_6;
const unsigned char FTGKEY_7 = FTGKEY_8 | FTGKEY_4;

const unsigned char FTGKEY_5 = 0;

const unsigned char FTG_ARROW_KEYS = FTGKEY_8 | FTGKEY_2 | FTGKEY_4 | FTGKEY_6;
const unsigned char FTG_BUTTON_KEYS = FTGKEY_A | FTGKEY_B | FTGKEY_C | FTGKEY_D;



const unsigned char CTRLKEY_UP = 1;
const unsigned char CTRLKEY_DOWN = 2;
const unsigned char CTRLKEY_LEFT = 3;
const unsigned char CTRLKEY_RIGHT = 4;
const unsigned char CTRLKEY_A = 5;
const unsigned char CTRLKEY_B = 6;
const unsigned char CTRLKEY_C = 7;
const unsigned char CTRLKEY_D = 8;

inline unsigned char ctrlkey2ftgkey(unsigned char ctrlkey, bool flip)
{
    static unsigned char ctrlkeymap[9] = {0, FTGKEY_8, FTGKEY_2, FTGKEY_4, FTGKEY_6, FTGKEY_A, FTGKEY_B, FTGKEY_C, FTGKEY_D};
    static unsigned char ctrlkeymapflip[9] = {0, FTGKEY_8, FTGKEY_2, FTGKEY_6, FTGKEY_4, FTGKEY_A, FTGKEY_B, FTGKEY_C, FTGKEY_D};
    if (flip) {
        return ctrlkeymapflip[ctrlkey];
    } else {
        return ctrlkeymap[ctrlkey];
    }
}

}

#endif
