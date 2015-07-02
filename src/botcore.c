#include "Python.h"

#include "botcore.h"

#include "ircclient.h"

char **masters;
char *channel;

int running;

void
BOT_run(char *nick, char *addr, char *port, char *home, char **masters_arr)
{
    int msglen = 0;
    // char *ibuff;

    channel = home;
    masters = masters_arr;

    IRC_connect(addr, port, nick);

    running = 1;
    while (running) {
        // msglen = IRC_listen(&ibuff);
        if (msglen < 1) {
            running = 0;
        } else {
            // IRC_unpack(ibuff, msglen);
        }
    }
}

