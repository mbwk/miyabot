#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "ircclient.h"

int
main(int argc, char *argv[])
{
    struct config *conf = parse_args(argc, argv);

    if (!conf) {
        fprintf(stderr, "please configure miyabot -- flags:\n\t%s\n\t%s\n\t%s\n\t%s\n\t%s\n",
                "-n -- NICK",
                "-a -- ADDRESS",
                "-p -- PORT",
                "-h -- HOME",
                "-m -- MASTERS (may supply more than one)");
        return -1;
    }

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

