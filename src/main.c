#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ircclient.h"

struct config {
    char *nick;
    char *addr;
    char *port;
    char *home;
    char **masters;
};

char DEFNICK[] = "miyabot";
char DEFADDR[] = "embyr.fyrechat.net";
char DEFPORT[] = "6667";
char DEFHOME[] = "#str8c";
char DEFMSTR[] = "miyabe";

#define CNICK 0x1
#define CADDR 0x2
#define CPORT 0x4
#define CHOME 0x8
#define CMSTR 0x10
#define CFULL 0x1f

struct config *gendefault(void);

struct config*
gen_default(void)
{
    struct config *defconf = malloc((5 * sizeof(char*)) + (2 * sizeof(char*)));
    defconf->nick = DEFNICK;
    defconf->addr = DEFADDR;
    defconf->port = DEFPORT;
    defconf->home = DEFHOME;
    defconf->masters = (char **) (defconf + 5);
    defconf->masters[0] = DEFMSTR;
    defconf->masters[1] = NULL;

    return defconf;
}

struct config*
parse_args(int argc, char *argv[])
{
    if (argc < 3) {
        return gen_default();
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

    printf("%d == %d\n", j, masters_count);

    if (j == masters_count) {
        conflags |= CMSTR;
    }

    if (conflags != CFULL) {
        return gen_default();
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

int
main(int argc, char *argv[])
{
    struct config *conf = parse_args(argc, argv);

    printf("NICK: %s\nADDR: %s:%s\nHOME: %s\nMTRS: %p\n",
            conf->nick,
            conf->addr,
            conf->port,
            conf->home,
            (void*) conf->masters
            );

    IRC_run(conf->nick, conf->addr, conf->port, conf->home, conf->masters);

    free(conf);

    return 0;
}

