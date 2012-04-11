#include <stdlib.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <math.h>

#include "keyfilter.h"
#include "ftgkeys.h"
#include "keystream.h"
#include "sprite.h"
#include "collisiondetect.h"
#include "resource.h"
#include "healthbar.h"
#include "ai.h"
#include "stage.h"
#include "netudp.h"

using namespace dragonfighting;

int main(int argc, char **argv)
{
    int exited = 0;
    SDL_Surface *screen = NULL;
    bool paused = false;
    Uint32 interval = 1000/60;
    Uint32 frame = 0;
    Uint32 oldtime = SDL_GetTicks();
    Uint32 newtime = oldtime;
    SDL_Init(SDL_INIT_EVERYTHING);
    atexit(SDL_Quit);

    screen = SDL_SetVideoMode(420, 224, 32, SDL_SWSURFACE);
    if (screen == NULL)
    {
        fprintf(stderr, "error set video mode.\n");
        exit(1);
    }

    // Init key map
    SDLKeyConverter keyconv;
    keyconv.registerKey(SDLK_w, CTRLKEY_UP);
    keyconv.registerKey(SDLK_s, CTRLKEY_DOWN);
    keyconv.registerKey(SDLK_a, CTRLKEY_LEFT);
    keyconv.registerKey(SDLK_d, CTRLKEY_RIGHT);
    keyconv.registerKey(SDLK_j, CTRLKEY_A);
    keyconv.registerKey(SDLK_k, CTRLKEY_B);
    keyconv.registerKey(SDLK_l, CTRLKEY_C);
    keyconv.registerKey(SDLK_i, CTRLKEY_D);

    CtrlKeyReaderWriter sdlkeyrw1;
    CtrlKeyReaderWriter sdlkeyrw2;
    CtrlKeyReaderWriter *localKeyReaderWriter = NULL;
    CtrlKeyReaderWriter *remoteKeyReaderWriter = NULL;

    // Init key filter
    KeyFilter keyFilter1;
    KeyFilter keyFilter2;
    unsigned char cmd[][8] = {
        {FTGKEY_6, FTGKEY_3, FTGKEY_2, FTGKEY_3, FTGKEY_A},
        {FTGKEY_2, FTGKEY_3, FTGKEY_6, FTGKEY_A},
        {FTGKEY_2, FTGKEY_3, FTGKEY_6, FTGKEY_B},
        {FTGKEY_2, FTGKEY_2, FTGKEY_A},
        {FTGKEY_6, FTGKEY_A},
        {FTGKEY_2, FTGKEY_A},
        {FTGKEY_3, FTGKEY_A},
    };
    keyFilter1.addCommand("6323A", cmd[0], 5);
    keyFilter1.addCommand("236A", cmd[1], 4);
    keyFilter1.addCommand("236B", cmd[2], 4);
    keyFilter1.addCommand("22A", cmd[3], 3);
    keyFilter1.addCommand("6A", cmd[4], 2);
    keyFilter1.addCommand("2A", cmd[5], 2);
    keyFilter1.addCommand("3A", cmd[6], 2);

    keyFilter2.addCommand("6323A", cmd[0], 5);
    keyFilter2.addCommand("236A", cmd[1], 4);
    keyFilter2.addCommand("236B", cmd[2], 4);
    keyFilter2.addCommand("22A", cmd[3], 3);
    keyFilter2.addCommand("6A", cmd[4], 2);
    keyFilter2.addCommand("2A", cmd[5], 2);
    keyFilter2.addCommand("3A", cmd[6], 2);

    //Init network
    enum Mode
    {
        AIcontrol,
        Client,
        Server
    };

    Mode mode = AIcontrol;
    Address address;

    if ( argc >= 2 ) {
        if (strcmp(argv[1], "server") == 0) {
            mode = Server;
            address = Address(127,0,0,1,26801);
        } else if (strcmp(argv[1], "client") == 0) {
            mode = Client;
            address = Address(127,0,0,1,26800);
        } else {
            printf("Usage:\n");
            return 1;
        }
    } else {
        mode = AIcontrol;
    }

    if ( !InitializeSockets() ) {
        printf( "failed to initialize sockets\n" );
        return 1;
    }
    const int ProtocolId = 0x11223344;
    const float TimeOut = 5.0f;
    const float DeltaTime = interval / 1000;
    bool connected = false;
    const int maxFrameDis = 30; // 30f = 0.5sec
    const int minFrameDis = 5;

    ReliableConnection connection(ProtocolId, TimeOut);

    if (!connection.Start(mode == Server ? 26800 : 26801)) {
        printf( "failed to start\n" );
        return -1;
    }

    if (mode == Server) {
        connection.Listen();
        printf("Listening...\n");
    } else if (mode == Client) {
        connection.Connect(address);
        printf("Connecting...\n");
    }

    //Character p1;
    Sprite *p1 = SpriteFactory::loadSprite("data", "minotaur");
    if (p1 == NULL) {
        exit(1);
    }
    p1->setName("p1");
    p1->setKeyFilter(&keyFilter1);
    p1->setInputer(&sdlkeyrw1);
    p1->playSequence("stand");
    p1->useCollisionSequence("stand");
    p1->setSpeed(2.0f);

    //Character p2;
    Sprite *p2 = SpriteFactory::loadSprite("data", "minotaur");
    if (p2 == NULL) {
        exit(1);
    }
    p2->setName("p2");
    p2->setKeyFilter(&keyFilter2);
    p2->setInputer(&sdlkeyrw2);
    p2->playSequence("stand");
    p2->useCollisionSequence("stand");
    p2->setSpeed(2.0f);

    // Init Stage
    Stage firstStage = Stage(p1, p2);
    firstStage.setPosition(-165, 0);

    // Init AI
    AI ai2 = AI(p2, p1);

    if (mode == Server) {
        localKeyReaderWriter = &sdlkeyrw1;
        remoteKeyReaderWriter = &sdlkeyrw2;
    } else if (mode == Client) {
        localKeyReaderWriter = &sdlkeyrw2;
        remoteKeyReaderWriter = &sdlkeyrw1;
    } else {
        localKeyReaderWriter = &sdlkeyrw1;
        remoteKeyReaderWriter = &sdlkeyrw2;
    }

    struct Ctrl_KeyEvent ctrlevent;
    // trying connection
    while(mode != AIcontrol && exited==0) {
        SDL_Event event;
        if (SDL_PollEvent(&event) == 1) {
            if((event.type==SDL_KEYDOWN && event.key.keysym.sym==SDLK_ESCAPE) || (event.type==SDL_QUIT)) exited=1;
        }
        if ( !connected && connection.IsConnected() ) {
            printf( "client connected to server\n" );
            connected = true;
            break;
        }

        if ( !connected && connection.IsConnectFailed() ) {
            printf( "connection failed\n" );
            exited = 1;
            break;
        }

        memset(&ctrlevent, 0, sizeof(ctrlevent));
        if (!connection.SendPacket(&ctrlevent, sizeof(ctrlevent))) {
        }
        memset(&ctrlevent, 0, sizeof(ctrlevent));
        if (connection.ReceivePacket(&ctrlevent, sizeof(ctrlevent)) > 0) {
            printf("recved\n");
        }

        connection.Update(DeltaTime);
        SDL_Delay(interval);
    }

    // main loop
    struct PendingAckNode {
        struct Ctrl_KeyEvent event;
        unsigned int sequence;
    };
    struct PendingAckNode pendingAck;
    memset(&pendingAck, 0, sizeof(pendingAck));
    unsigned int localSequence = 0;
    while(exited==0)
    {
        memset(&ctrlevent, 0, sizeof(ctrlevent));
        // Net
        if ( mode == Server && connected && !connection.IsConnected() ) {
            printf( "reset flow control\n" );
            connected = false;
        }

        if (maxFrameDis - minFrameDis) {}
        if (mode == Client) {
            static Uint32 serverFrame = 0;
            if (connection.ReceivePacket(&ctrlevent, sizeof(ctrlevent)) > 0) {
                if (ctrlevent.frameStamp > serverFrame) {
                    serverFrame = ctrlevent.frameStamp;
                }
                if (ctrlevent.type != Ctrl_KEYNONE && ctrlevent.frameStamp == serverFrame) {
                    printf("recv key: %d, packetframe: %d, localframe: %d\n", ctrlevent.key, ctrlevent.frameStamp, frame);
                    remoteKeyReaderWriter->writeEvent(&ctrlevent);
                }
            } else {
                printf("recv error\n");
            }

            int ack_count = 0;
            unsigned int * acks = NULL;
            connection.GetReliabilitySystem().GetAcks( &acks, ack_count );
            for (int i=0; i<ack_count; i++) {
                if (acks[i] == pendingAck.sequence) {
                    // last event acked
                    printf("sequence %d adked\n", pendingAck.sequence);
                    pendingAck.sequence = 0;
                }
            }

            // if no pending ack
            if (pendingAck.sequence == 0) {
                //----input----
                SDL_Event event;
                if (SDL_PollEvent(&event) == 1) {
                    localSequence = connection.GetReliabilitySystem().GetLocalSequence();
                    ctrlevent.frameStamp = frame;
                    ctrlevent.controler = 1;
                    if((event.type==SDL_KEYDOWN && event.key.keysym.sym==SDLK_ESCAPE) || (event.type==SDL_QUIT)) exited=1;
                    else if (event.type==SDL_KEYDOWN)
                    {
                        ctrlevent.type = Ctrl_KEYDOWN;
                        ctrlevent.key = keyconv.convert(event.key.keysym.sym);
                        if (ctrlevent.key != 0) {
                            localKeyReaderWriter->writeEvent(&ctrlevent);
                            pendingAck = {ctrlevent, localSequence};
                        }
                    }
                    else if (event.type==SDL_KEYUP)
                    {
                        ctrlevent.type = Ctrl_KEYUP;
                        ctrlevent.key = keyconv.convert(event.key.keysym.sym);
                        if (ctrlevent.key != 0) {
                            localKeyReaderWriter->writeEvent(&ctrlevent);
                            pendingAck = {ctrlevent, localSequence};
                        }
                    }
                } else {
                    ctrlevent.type = Ctrl_KEYNONE;
                }
            }

            if (!connection.SendPacket(&ctrlevent, sizeof(ctrlevent))) {
                printf("send error\n");
            }
        } else if (mode == Server) {
            // if no pending ack
            if (pendingAck.sequence == 0) {
                //----input----
                SDL_Event event;
                if (SDL_PollEvent(&event) == 1) {
                    localSequence = connection.GetReliabilitySystem().GetLocalSequence();
                    ctrlevent.frameStamp = frame;
                    ctrlevent.controler = 1;
                    if((event.type==SDL_KEYDOWN && event.key.keysym.sym==SDLK_ESCAPE) || (event.type==SDL_QUIT)) exited=1;
                    else if (event.type==SDL_KEYDOWN)
                    {
                        ctrlevent.type = Ctrl_KEYDOWN;
                        ctrlevent.key = keyconv.convert(event.key.keysym.sym);
                        if (ctrlevent.key != 0) {
                            localKeyReaderWriter->writeEvent(&ctrlevent);
                            pendingAck = {ctrlevent, localSequence};
                        }
                    }
                    else if (event.type==SDL_KEYUP)
                    {
                        ctrlevent.type = Ctrl_KEYUP;
                        ctrlevent.key = keyconv.convert(event.key.keysym.sym);
                        if (ctrlevent.key != 0) {
                            localKeyReaderWriter->writeEvent(&ctrlevent);
                            pendingAck = {ctrlevent, localSequence};
                        }
                    }
                } else {
                    ctrlevent.type = Ctrl_KEYNONE;
                }
            }

            if (!connection.SendPacket(&ctrlevent, sizeof(ctrlevent))) {
                printf("send error\n");
            }
            
            static Uint32 clientFrame = 0;
            if (connection.ReceivePacket(&ctrlevent, sizeof(ctrlevent)) > 0) {
                if (ctrlevent.frameStamp > clientFrame) {
                    clientFrame = ctrlevent.frameStamp;
                }
                if (ctrlevent.type != Ctrl_KEYNONE && ctrlevent.frameStamp == clientFrame) {
                    printf("recv key: %d, packetframe: %d, localframe: %d\n", ctrlevent.key, ctrlevent.frameStamp, frame);
                    ctrlevent.frameStamp = frame;
                    remoteKeyReaderWriter->writeEvent(&ctrlevent);
                }
            } else {
                printf("recv error\n");
            }

            int ack_count = 0;
            unsigned int * acks = NULL;
            connection.GetReliabilitySystem().GetAcks( &acks, ack_count );
            for (int i=0; i<ack_count; i++) {
                if (acks[i] == pendingAck.sequence) {
                    // last event acked
                    printf("sequence %d adked\n", pendingAck.sequence);
                    pendingAck.sequence = 0;
                }
            }
        } else if (mode == AIcontrol) {
            //----input----
            SDL_Event event;
            if (SDL_PollEvent(&event) == 1) {
                ctrlevent.frameStamp = frame;
                ctrlevent.controler = 1;
                if((event.type==SDL_KEYDOWN && event.key.keysym.sym==SDLK_ESCAPE) || (event.type==SDL_QUIT)) exited=1;
                else if (event.type==SDL_KEYDOWN)
                {
                    ctrlevent.type = Ctrl_KEYDOWN;
                    ctrlevent.key = keyconv.convert(event.key.keysym.sym);
                    if (ctrlevent.key != 0) {
                        localKeyReaderWriter->writeEvent(&ctrlevent);
                    }
                }
                else if (event.type==SDL_KEYUP)
                {
                    ctrlevent.type = Ctrl_KEYUP;
                    ctrlevent.key = keyconv.convert(event.key.keysym.sym);
                    if (ctrlevent.key != 0) {
                        localKeyReaderWriter->writeEvent(&ctrlevent);
                    }
                }
            } else {
                ctrlevent.type = Ctrl_KEYNONE;
            }

            if (ai2.pollEvent(&ctrlevent)) {
                ctrlevent.frameStamp = frame;
                ctrlevent.controler = 2;
                remoteKeyReaderWriter->writeEvent(&ctrlevent);
            }
        }

        if (!paused) {
            // ----logic----
            firstStage.update(frame);
            // AI
            if (mode == AIcontrol) {
                ai2.update(frame);
            }
            // ----frame control----
            frame++;
        }

        connection.Update(DeltaTime);

        // ----draw----
        SDL_FillRect( screen, NULL, 0x00008080 );
        //SDL_FillRect( screen, &rect, color );
        firstStage.draw(screen);
        SDL_Flip(screen);


        newtime = SDL_GetTicks();
        if (newtime - oldtime < interval) {
            SDL_Delay(interval - newtime + oldtime);
        }
        oldtime = SDL_GetTicks();
    }

    SpriteFactory::freeSprite(p1);
    SpriteFactory::freeSprite(p2);

    SDL_Quit();
    return 0;
}


