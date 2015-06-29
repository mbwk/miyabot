#include <stdio.h>
#include "ircclient.h"

int
main()
{
    IRC_run("miyabot", "irc.fyrechat.net", "6667", "#sandbox", "miyabe");
}

