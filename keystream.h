#ifndef _KEY_STREAM_H_
#define _KEY_STREAM_H_

#include <stdio.h>
#include <SDL/SDL.h>
#include "ftgkeys.h"

namespace dragonfighting {


enum CtrlKey {
    Ctrl_KEYNONE,
    Ctrl_KEYDOWN,
    Ctrl_KEYUP,
};

struct Ctrl_KeyEvent {
    char controler;
    unsigned char type;
    unsigned char key;
    Uint32 frameStamp;
};

class SDLKeyConverter {
protected:
    unsigned char keyMap[SDLK_LAST];

public:
    SDLKeyConverter();
    void registerKey(unsigned char sdlKey, unsigned char ctrlKey);
    unsigned char convert(unsigned char sdlKey);
};

class CtrlKeyWriter
{
public:
    CtrlKeyWriter() {}
    virtual ~CtrlKeyWriter() {}

    virtual void writeEvent(struct Ctrl_KeyEvent *event) = 0;

};

class CtrlKeyReader
{
public:
    CtrlKeyReader() {}
    virtual ~CtrlKeyReader() {}

    virtual int readEvent(struct Ctrl_KeyEvent *event, Uint32 frameStamp) = 0;
};

class CtrlKeyReaderWriter : public CtrlKeyReader, public CtrlKeyWriter
{
protected:
    struct Ctrl_KeyEvent keyEvent;
    bool isEmpty;

public:
    CtrlKeyReaderWriter();
    virtual ~CtrlKeyReaderWriter();

    virtual void writeEvent(struct Ctrl_KeyEvent *event);
    virtual int readEvent(struct Ctrl_KeyEvent *event, Uint32 frameStamp);
};

class NetReader : public CtrlKeyReader
{
public:
    NetReader();
    virtual ~NetReader();

    virtual int readEvent(struct Ctrl_KeyEvent *event, Uint32 frameStamp);
};

class NetWriter : public CtrlKeyWriter
{
public:
    virtual void writeEvent(struct Ctrl_KeyEvent *event);
};

class ReplayReader : public CtrlKeyReader
{
public:
    ReplayReader();
    virtual ~ReplayReader();

    virtual int readEvent(struct Ctrl_KeyEvent *event);
};

class ReplayWriter : public CtrlKeyWriter
{
protected:
    struct Ctrl_KeyEvent *buffer;
    unsigned long index;

public:
    ReplayWriter();
    virtual void writeEvent(struct Ctrl_KeyEvent *event);
};

class DebugReader : public CtrlKeyReader
{
protected:
    struct Ctrl_KeyEvent *eventArray;

public:
    DebugReader(struct Ctrl_KeyEvent *events, int len);

};

}

#endif
