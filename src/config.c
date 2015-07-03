#include <stdlib.h>
#include <string.h>

#include "config.h"

struct config*
parse_args(int argc, char *argv[])
{
    if (argc < 3) {
        return NULL;
    }

    int conflags = 0;
    char *nick, *addr, *port, *home;
    nick = addr = port = home = NULL;
    int masters_count = 0;

    for (int i = 1; i < argc - 1; ++i) {
        char *thisarg = argv[i], *nextarg = argv[i + 1];
        if (thisarg[0] == '-' && nextarg[0] != '-') {
            switch (argv[i][1]) {
                case 'n':
                    nick = nextarg;
                    conflags |= CNICK;
                    break;
                case 'a':
                    addr = nextarg;
                    conflags |= CADDR;
                    break;
                case 'p':
                    port = nextarg;
                    conflags |= CPORT;
                    break;
                case 'h':
                    home = nextarg;
                    conflags |= CHOME;
                    break;
                case 'm':
                    ++masters_count;
                    break;
            }
            ++i;
        }
    }

    char *masters[masters_count + 1];
    masters[masters_count] = NULL;
    int j = 0;

    for (int i = 0; i < argc - 1; ++i) {
        char *thisarg = argv[i], *nextarg = argv[i + 1];
        if (!strncmp(thisarg, "-m", 2) && nextarg[0] != '-') {
            masters[j] = nextarg;
            ++j;
            ++i;
        }
    }

    if (j == masters_count) {
        conflags |= CMSTR;
    }

    if (conflags != CFULL) {
        return NULL;
    }

    // allocate storage for the struct and an array of pointers immediately after it
    int ptrcount = 5 + masters_count + 1;
    struct config *conf = malloc(ptrcount * sizeof(char*));

    // point to program args
    conf->nick = nick;
    conf->addr = addr;
    conf->port = port;
    conf->home = home;

    // copy array of pointers to location after struct and point to it
    conf->masters = (char**) conf + 5;
    memcpy(conf->masters, masters, (masters_count + 1) * sizeof (char *));

    return conf;
}

