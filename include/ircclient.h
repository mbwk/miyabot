#ifndef IRCCLIENT_H
#define IRCCLIENT_H

void IRC_run(char *nick, char *addr, char *port, char *channel, char *master);
void IRC_quit();

#endif /* IRCCLIENT_H */

