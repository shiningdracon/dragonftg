#include <SDL/SDL.h>
#include <assert.h>

#include "keystream.h"

namespace dragonfighting {


CtrlKeyReaderWriter::CtrlKeyReaderWriter() :
    isEmpty(true)
{
    memset(&keyEvent, 0, sizeof(struct Ctrl_KeyEvent));
}

CtrlKeyReaderWriter::~CtrlKeyReaderWriter()
{
}

void CtrlKeyReaderWriter::writeEvent(struct Ctrl_KeyEvent *event)
{
    assert(isEmpty == true);

    this->keyEvent = *event;
    isEmpty = false;
}

int CtrlKeyReaderWriter::readEvent(struct Ctrl_KeyEvent *event, Uint32 frameStamp)
{
    if (isEmpty) return 0;

    if (frameStamp == this->keyEvent.frameStamp) {
        *event = this->keyEvent;
        isEmpty = true;
        return 1;
    }

    return 0;
}


ReplayWriter::ReplayWriter() :
    index(0)
{
    buffer = (struct Ctrl_KeyEvent *)malloc(4096);
}

void ReplayWriter::writeEvent(struct Ctrl_KeyEvent *event)
{
    buffer[index++] = *event;
}


SDLKeyConverter::SDLKeyConverter()
{
    memset(keyMap, 0, sizeof(keyMap));
}

void SDLKeyConverter::registerKey(unsigned char sdlKey, unsigned char ctrlKey)
{
    keyMap[sdlKey] = ctrlKey;
}

unsigned char SDLKeyConverter::convert(unsigned char sdlKey)
{
    return keyMap[sdlKey];
}

}
